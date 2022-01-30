/**
 * @file app.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Application specific functions. Mandatory to have init_app(), 
 *        app_event_handler(), ble_data_handler(), lora_data_handler()
 *        and lora_tx_finished()
 * @version 0.1
 * @date 2021-04-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "app.h"

/** Set the device name, max length is 10 characters */
char g_ble_dev_name[10] = "RAK-SOIL";

/** Battery level uinion */
batt_s batt_level;

/** Send Fail counter **/
uint8_t send_fail = 0;

/** Flag for low battery protection */
bool low_batt_protection = false;

/** Flag if the acc sensor is available */
bool has_acc = false;

/** Initialization result */
bool init_result = true;

/**
 * @brief Application specific setup functions
 * 
 */
void setup_app(void)
{
#if API_DEBUG == 0
	// Initialize Serial for debug output
	Serial.begin(115200);

	time_t serial_timeout = millis();
	// On nRF52840 the USB serial is not available immediately
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
		}
		else
		{
			break;
		}
	}
#endif

	// Enable BLE
	g_enable_ble = true;
}

/**
 * @brief Application specific initializations
 * 
 * @return true Initialization success
 * @return false Initialization failure
 */
bool init_app(void)
{
	MYLOG("APP", "init_app");

	api_set_version(SW_VERSION_1, SW_VERSION_2, SW_VERSION_3);

	// Initialize Serial for debug output
	Serial.begin(115200);

	time_t serial_timeout = millis();
	// On nRF52840 the USB serial is not available immediately
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		}
		else
		{
			break;
		}
	}

	Serial.printf("============================\n");
	Serial.printf("Soil Moisture Sensor Solution\n");
	Serial.printf("Built with RAK's WisBlock\n");
	Serial.printf("SW Version %d.%d.%d\n", g_sw_ver_1, g_sw_ver_2, g_sw_ver_3);
	Serial.printf("LoRa(R) is a registered trademark or service mark of Semtech Corporation or its affiliates.\nLoRaWAN(R) is a licensed mark.\n");
	Serial.printf("============================\n");
	at_settings();
	Serial.printf("============================\n");

	Wire.begin();

	if (init_acc())
	{
		has_acc = true;
	}
	// Initialize Soil module
	init_result = init_soil();
	return init_result;
}

/**
 * @brief Application specific event handler
 *        Requires as minimum the handling of STATUS event
 *        Here you handle as well your application specific events
 */
void app_event_handler(void)
{
	// To lower power consumption, switch off the LED immediately
	digitalWrite(LED_GREEN, LOW);
	// Timer triggered event
	if ((g_task_event_type & STATUS) == STATUS)
	{
		g_task_event_type &= N_STATUS;
		MYLOG("APP", "Timer wakeup");

		// Initialization failed, report error over AT interface */
		if (!init_result)
		{
			AT_PRINTF("+EVT:HW_FAILURE\n");
		}

		// If BLE is enabled, restart Advertising
		if (g_enable_ble)
		{
			restart_advertising(15);
		}

		if (!low_batt_protection)
		{
			if (init_result)
			{
				// Get soil sensor values
				read_soil();
			}
		}

		// Get battery level
		float temp_batt = read_batt() / 10;
		for (int num = 0; num < 10; num++)
		{
			temp_batt += read_batt() / 10;
			temp_batt /= 2;
		}

		batt_level.batt16 = (uint16_t)temp_batt;
		g_soil_data.batt_1 = batt_level.batt8[1];
		g_soil_data.batt_2 = batt_level.batt8[0];

#if MY_DEBUG > 0
		Serial.printf("%02X", g_soil_data.data_flag0);
		Serial.printf("%02X", g_soil_data.data_flag1);
		Serial.printf("%02X", g_soil_data.humid_1);
		Serial.printf("%02X", g_soil_data.data_flag2);
		Serial.printf("%02X", g_soil_data.data_flag3);
		Serial.printf("%02X", g_soil_data.temp_1);
		Serial.printf("%02X", g_soil_data.temp_2);
		Serial.printf("%02X", g_soil_data.data_flag4);
		Serial.printf("%02X", g_soil_data.data_flag5);
		Serial.printf("%02X", g_soil_data.batt_1);
		Serial.printf("%02X", g_soil_data.batt_2);
		Serial.printf("%02X", g_soil_data.data_flag6);
		Serial.printf("%02X", g_soil_data.data_flag7);
		Serial.printf("%02X\n", g_soil_data.valid);
#endif

		// Protection against battery drain
		if (batt_level.batt16 < 290)
		{
			// Battery is very low, change send time to 1 hour to protect battery
			low_batt_protection = true; // Set low_batt_protection active
			api_timer_restart(6 * 60 * 60 * 1000);
			MYLOG("APP", "Battery protection activated");
		}
		else if ((batt_level.batt16 > 410) && low_batt_protection)
		{
			// Battery is higher than 4V, change send time back to original setting
			low_batt_protection = false;
			api_timer_restart(g_lorawan_settings.send_repeat_time);
			MYLOG("APP", "Battery protection deactivated");
		}

		lmh_error_status result;
		if (low_batt_protection || !init_result)
		{
			MYLOG("APP", "Sending short packet");
			g_soil_data.valid = 0;
			result = send_lora_packet((uint8_t *)&g_soil_data.data_flag4, 7);
		}
		else
		{
			MYLOG("APP", "Sending full packet");
			result = send_lora_packet((uint8_t *)&g_soil_data, SOIL_DATA_LEN);
		}
		switch (result)
		{
		case LMH_SUCCESS:
			MYLOG("APP", "Packet enqueued");
			break;
		case LMH_BUSY:
			MYLOG("APP", "LoRa transceiver is busy");
			break;
		case LMH_ERROR:
			MYLOG("APP", "Packet error, too big to send with current DR");
			break;
		}
	}

	// ACC trigger event
	if ((g_task_event_type & ACC_TRIGGER) == ACC_TRIGGER)
	{
		g_task_event_type &= N_ACC_TRIGGER;
		MYLOG("APP", "ACC triggered");

		if (has_acc)
		{
			read_acc();
			clear_acc_int();
			restart_advertising(15);
		}
	}
}

/**
 * @brief Handle BLE UART data
 * 
 */
void ble_data_handler(void)
{
	// To lower power consumption, switch off the LED immediately
	digitalWrite(LED_GREEN, LOW);
	if (g_enable_ble)
	{
		// BLE UART data handling
		if ((g_task_event_type & BLE_DATA) == BLE_DATA)
		{
			MYLOG("AT", "RECEIVED BLE");
			/** BLE UART data arrived */
			g_task_event_type &= N_BLE_DATA;

			while (g_ble_uart.available() > 0)
			{
				at_serial_input(uint8_t(g_ble_uart.read()));
				delay(5);
			}
			at_serial_input(uint8_t('\n'));
		}
	}
}

/**
 * @brief Handle received LoRa Data
 * 
 */
void lora_data_handler(void)
{
	// To lower power consumption, switch off the LED immediately
	digitalWrite(LED_GREEN, LOW);

	// LoRa Join finished handling
	if ((g_task_event_type & LORA_JOIN_FIN) == LORA_JOIN_FIN)
	{
		g_task_event_type &= N_LORA_JOIN_FIN;
		if (g_join_result)
		{
			MYLOG("APP", "Successfully joined network");
			AT_PRINTF("+EVT:JOINED\n");
		}
		else
		{
			MYLOG("APP", "Join network failed");
			AT_PRINTF("+EVT:JOIN FAILED\n");
			// Restart Join Request
			lmh_join();
		}
	}

	// LoRa TX finished handling
	if ((g_task_event_type & LORA_TX_FIN) == LORA_TX_FIN)
	{
		g_task_event_type &= N_LORA_TX_FIN;

		MYLOG("APP", "LPWAN TX cycle %s", g_rx_fin_result ? "finished ACK" : "failed NAK");

		if ((g_lorawan_settings.confirmed_msg_enabled) && (g_lorawan_settings.lorawan_enable))
		{
			AT_PRINTF("+EVT:SEND CONFIRMED %s\n", g_rx_fin_result ? "SUCCESS" : "FAIL");
		}
		else
		{
			AT_PRINTF("+EVT:SEND OK\n");
		}

		if (!g_rx_fin_result)
		{
			// Increase fail send counter
			send_fail++;

			if (send_fail == 10)
			{
				// Too many failed sendings, reset node and try to rejoin
				delay(100);
				api_reset();
			}
		}
	}

	// LoRa data handling
	if ((g_task_event_type & LORA_DATA) == LORA_DATA)
	{
		/**************************************************************/
		/**************************************************************/
		/// LoRa data arrived
		/// 0x01 0x66 0xnn uplink is used to switch on / off the ble
		/// nn = 0 => switch off BLE after 5 seconds
		/// nn = 1 => switch on BLE permanently
		/**************************************************************/
		/**************************************************************/
		g_task_event_type &= N_LORA_DATA;
		MYLOG("APP", "Received package over LoRa");

		if (g_lorawan_settings.lorawan_enable)
		{
			AT_PRINTF("+EVT:RX_1, RSSI %d, SNR %d\n", g_last_rssi, g_last_snr);
			AT_PRINTF("+EVT:%d:", g_last_fport);
			for (int idx = 0; idx < g_rx_data_len; idx++)
			{
				AT_PRINTF("%02X", g_rx_lora_data[idx]);
			}
			AT_PRINTF("\n");
		}
		else
		{
			AT_PRINTF("+EVT:RXP2P, RSSI %d, SNR %d\n", g_last_rssi, g_last_snr);
			AT_PRINTF("+EVT:");
			for (int idx = 0; idx < g_rx_data_len; idx++)
			{
				AT_PRINTF("%02X", g_rx_lora_data[idx]);
			}
			AT_PRINTF("\n");
		}

		char log_buff[g_rx_data_len * 3] = {0};
		uint8_t log_idx = 0;
		for (int idx = 0; idx < g_rx_data_len; idx++)
		{
			sprintf(&log_buff[log_idx], "%02X ", g_rx_lora_data[idx]);
			log_idx += 3;
		}
		MYLOG("APP", "%s", log_buff);

		if (g_rx_data_len > 2)
		{
			if ((g_rx_lora_data[0] == 0x01) && (g_rx_lora_data[1] == 0x66))
			{
				// Received BLE command
				if (g_rx_lora_data[2] == 1)
				{
					// Command to switch on BLE
					restart_advertising(0);
				}
				else if (g_rx_lora_data[2] == 0)
				{
					// Command to switch off BLE (after 5 seconds)
					restart_advertising(5);
				}
			}
		}
	}
}
