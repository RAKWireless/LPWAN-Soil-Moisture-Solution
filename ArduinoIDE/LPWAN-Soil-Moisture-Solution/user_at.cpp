/**
 * @file user_at.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Handle user defined AT commands
 * @version 0.1
 * @date 2021-11-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "app.h"

#define AT_PRINTF(...)                  \
	Serial.printf(__VA_ARGS__);         \
	if (g_ble_uart_is_connected)        \
	{                                   \
		g_ble_uart.printf(__VA_ARGS__); \
	}

/**
 * @brief Start dry calibration
 * 
 * @return int 0
 */
static int at_exec_dry()
{
	// Dry calibration requested
	AT_PRINTF("Start Dry Calibration\n");
	uint16_t new_val = start_calib(true);
	if (new_val == 0xFFFF)
	{
		AT_PRINTF("Calibration failed, please try again");
	}
	else
	{
	AT_PRINTF("New Dry Calibration Value: %d", new_val);
	}
	return 0;
}

static int at_set_dry(char *str)
{
	long dry_val = strtol(str, NULL, 0);
	if ((dry_val < 0) || (dry_val > 1000))
	{
		return AT_ERRNO_PARA_VAL;
	}
	set_calib(true, dry_val);
	return 0;
}

/**
 * @brief Query dry calibration value
 * 
 * @return int 0
 */
static int at_query_dry()
{
	// Dry calibration value query
	AT_PRINTF("Dry Calibration Value: %d", get_calib(true));
	return 0;
}

/**
 * @brief Start wet calibration
 * 
 * @return int 0
 */
static int at_exec_wet()
{
	// Dry calibration requested
	AT_PRINTF("Start Wet Calibration\n");
	uint16_t new_val = start_calib(false);
	if (new_val == 0xFFFF)
	{
		AT_PRINTF("Calibration failed, please try again");
	}
	else
	{
	AT_PRINTF("New Wet Calibration Value: %d", new_val);
	}
	return 0;
}

static int at_set_wet(char *str)
{
	long wet_val = strtol(str, NULL, 0);
	if ((wet_val < 0) || (wet_val > 1000))
	{
		return AT_ERRNO_PARA_VAL;
	}
	set_calib(false, wet_val);
	return 0;
}

/**
 * @brief Query wet calibration value
 * 
 * @return int 0
 */
static int at_query_wet()
{
	// Wet calibration value query
	AT_PRINTF("Wet Calibration Value: %d", get_calib(false));
	return 0;
}

atcmd_t g_user_at_cmd_list[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// GNSS commands
	{"+DRY", "Get/Set dry calibration value", at_query_dry, at_set_dry, at_exec_dry},
	{"+WET", "Get/Set wet calibration value", at_query_wet, at_set_wet, at_exec_wet},
};

/** Number of user defined AT commands */
uint8_t g_user_at_cmd_num = sizeof(g_user_at_cmd_list) / sizeof(atcmd_t);
