# WisBlock Soil Moisture Sensor
| <img src="./assets/RAK-Whirls.png" alt="Modules" width="150"> | <img src="./assets/WisBlock.png" alt="WisBlock" width="150"> | <img src="./assets/rakstar.jpg" alt="RAKstar" width="100"> | <img src="./assets/RAK12023.png" alt="RAK12023" width="100"> | <img src="./assets/RAK12035.png" alt="RAK12035" height="300"> | <img src="./assets/beegee.png" alt="Bernd" width="100"> |    
| :-: | :-: | :-: | :-: | :-: | :-: |      

This solution is for a soil moisture sensor based on the [WisBlock RAK4631 Core module](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK4631/Overview/) :arrow_heading_up: and the [RAK12023/RAK12035 soil moisture sensor](https://docs.rakwireless.com/Product-Categories/WisBlock/#wisblock-sensor) :arrow_heading_up:.

----

# Hardware used
- [RAK4631](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK4631/Overview/) :arrow_heading_up: WisBlock Core module
- [RAK5005-O](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK5005-O/Overview/) :arrow_heading_up: WisBlock Base board
- [RAK12023/RAK12035 soil moisture sensor](https://docs.rakwireless.com/Product-Categories/WisBlock/#wisblock-sensor) :arrow_heading_up:
- Optional [RAK1904 ACC sensor](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1904/Overview/) :arrow_heading_up:

## Power consumption
The application sleeps around between each measurement and puts the soil moisture sensor into sleep as well. But even this solution switches off the power of the sensor, the sleep current is still ~190uA. Still looking into a better solution.

----

# Software used
- [PlatformIO](https://platformio.org/install) :arrow_heading_up:
- alternative [ArduinoIDE](https://www.arduino.cc/en/software) :arrow_heading_up:
- [Adafruit nRF52 BSP](https://docs.platformio.org/en/latest/boards/nordicnrf52/adafruit_feather_nrf52832.html) :arrow_heading_up:
- [Patch to use RAK4631 with PlatformIO](https://github.com/RAKWireless/WisBlock/blob/master/PlatformIO/RAK4630/README.md) :arrow_heading_up:
- [WisBlock API](https://github.com/beegee-tokyo/WisBlock-API) :arrow_heading_up:
- [RAK12035 Soil Moisture library](https://github.com/RAKWireless/RAK12035_SoilMoisture) :arrow_heading_up:
- [Sparkfun LIS3DH library](https://platformio.org/lib/show/1401/SparkFun%20LIS3DH%20Arduino%20Library)

## _REMARK_     
The project was developed using Platform IO. But for the users that _**still**_ stick to Arduino IDE, an Arduino IDE compatible copy of the sources is in the ArduinoIDE folder.    
The complete project for PIO is in the PlatformIO folder.    

## _REMARK_
The libraries are all listed in the **`platformio.ini`** and are automatically installed when the project is compiled. 

## _REMARK_    
The RAK1904 ACC sensor is used as a **`virtual`** switch. BLE is only active after a power up or reset. If the sensor is in the field and parameters needs to be changed, it would be inconvenient to always open the enclosure. If you tap on the enclosure, the module will wake up and activate the BLE for 15 seconds so that a mobile phone can connect. This way it is very simple to change parameters even on deployed sensors somewhere in the field.    

----

# How to use it

#### Compile the firmware and flash it on a WisBlock with RAK4631 and RAK12023 modules installed.

#### A Setup the LPWAN credentials with one of the options:
1) Connect over USB to setup the LPWAN credentials. Use the DevEUI printed on the RAK4631, use the AppEUI and AppKey from your LPWAN server. Do _**NOT**_ activate automatic join yet. As soil moisture levels are not changing very fast, it might be sufficient to set the send frequency to every 3 hours. The send frequency is set in seconds, so the value would be 3 * 60 * 60 ==> 10800    
Example AT commands:
```
AT+NWM=1
AT+NJM=1
AT+BAND=10
AT+DEVEUI=1000000000000001
AT+APPEUI=AB00AB00AB00AB00
AT+APPKEY=AB00AB00AB00AB00AB00AB00AB00AB00
AT+SENDFREQ=3600
```

| Command | Explanation |
| --- | --- |
| **AT+NWM=1**  |  set the node into LoRaWAN mode |
| **AT+NJM=1**  |  set network join method to OTAA |
| **AT+BAND=10**  |  set LPWAN region (here AS923-3) see [AT Command Manual](./AT-Commands.md) :arrow_heading_up: for all regions |
| **AT+DEVEUI=1000000000000001**  |  set the device EUI, best to use the DevEUI that is printed on the label of your WisBlock Core module |
| **AT+APPEUI=AB00AB00AB00AB00**  |  set the application EUI, required on the LoRaWAN server  |
| **AT+APPKEY=AB00AB00AB00AB00AB00AB00AB00AB00**  |  set the application Key, used to encrypt the data packet during network join |
| **AT+SENDFREQ=3600**  |  set the frequency the sensor node will send data packets. 3600 == 60 x 60 seconds == 1 hour |

## _REMARK_
The manual for all AT commands can be found here: [AT-Commands.md](./AT-Commands.md) :arrow_heading_up:    

2) Use the [WisBlock Toolbox](https://play.google.com/store/apps/details?id=tk.giesecke.wisblock_toolbox) :arrow_heading_up:, connect over Bluetooth with the Soil Sensor and setup the credentials. Do _**NOT**_ activate automatic join yet.

#### B Calibrate the sensor
As each sensor has a different sensitivity, the sensor needs to be calibrated. Calibration can be done over the AT command interface. The calibration requires two steps, first measure with the sensor in dry air, second measure with the sensor submerged in water (only the tip up to the white line on the sensor).    
The calibration needs to be done only once. The calibration values are stored in the RAK12035 sensor. That means if you calibrate the sensor on one RAK4631, you do not need to do it again if you change the RAK4631. The firmware will read the calibration data from the sensor.    
1) Dry calibration.    
Have the sensor in dry air.    
Submit AT command **`AT+DRY`** to start the calibration. After the calibration is done, the value is saved in the sensor and the result will be printed in the terminal screen.
2) Wet calibration.    
Have the sensor submerged in water (only the tip up to the white line on the sensor).    
Submit AT command **`AT+WET`** to start the calibration. After the calibration is done, the value is saved in the sensor and the result will be printed in the terminal screen.

Above calibration is a very basic one, if you need better accuracy you need to calibrate the sensor in the soil itself. Which is not a simple thing, as the moisture measurement will be affected by the density of the soil, the amount of fertilizer in the soil, temperature and other factors.     
For a calibration in soil, you need to prepare two pots with the same soil (best taken from the field/garden where the sensors will be used later). 
1) Dry calibration:
Let one pot dry up complete, make sure the soil is not clumped together and the complete soil in the pot is dried up. Put the sensor complete into the dry soil, make sure it is complete under the surface, only the wire sticking out.    
Submit AT command **`AT+DRY`** to start the calibration. After the calibration is done, the value is saved in the sensor and the result will be printed in the terminal screen.    
2) Wet calibration:
Poor water into the second pot. Make sure the soil is saturated with water, mix it up well, make sure the soil has everywhere the same level of water saturation. Put the sensor complete into the wet soil, make sure it is complete under the surface, only the wire sticking out.    
Submit AT command **`AT+WET`** to start the calibration. After the calibration is done, the value is saved in the sensor and the result will be printed in the terminal screen.

#### C Setup your LPWAN server application.    
Please check tutorials how to setup an LPWAN server application.     
Example for Loriot: [RAK7258 Loriot](https://docs.rakwireless.com/Product-Categories/WisGate/RAK7258/Supported-LoRa-Network-Servers/#loriot) :arrow_heading_up:    
Example for AWS: [RAK7258 AWS](https://docs.rakwireless.com/Product-Categories/WisGate/RAK7258/Supported-LoRa-Network-Servers/#amazon-web-services-aws) :arrow_heading_up:    
Example for TheThingsNetwork: [RAK7258 TTN](https://docs.rakwireless.com/Product-Categories/WisGate/RAK7258/Supported-LoRa-Network-Servers/#the-things-network-ttn) :arrow_heading_up:    
Example for Chirpstack: [RAK7244 Chirpstack](https://docs.rakwireless.com/Product-Categories/WisGate/RAK7244/Quickstart/#connect-the-gateway-with-chirpstack) :arrow_heading_up:

#### D Connect the Soil Sensor to the LPWAN server
To enable connection to the LPWAN server, the sensor node must be set to _**auto join**_ mode either with AT command over USB or with the WisBlock Toolbox over BLE.    
1) With AT command:    
Reconnect over USB to the sensor node and send the command **`AT+JOIN=1,1,10,10`** to the sensor node. Then restart the node to start the join process.    
2) Over BLE:    
Connect the WisBlock Toolbox to the sensor node. Check the _**Auto join**_ checkbox, then push the _**Send**_ button.

----

# Setting up LoRaWAN credentials
The LoRaWAN credentials are defined in [include/main.h](./include/main.h). But this code supports 2 other methods to change the LoRaWAN credentials on the fly:

## 1) Setup over BLE
Using the [WisBlock Toolbox](https://play.google.com/store/apps/details?id=tk.giesecke.wisblock_toolbox) :arrow_heading_up: you can connect to the WisBlock over BLE and setup all LoRaWAN parameters like
- Region
- OTAA/ABP
- Confirmed/Unconfirmed message
- ...

More details can be found in the [WisBlock Toolbox repo](https://github.com/beegee-tokyo/WisBlock-Toolbox) :arrow_heading_up:

The device is advertising over BLE only the first 30 seconds after power up and then again for 15 seconds after wakeup for measurements. The device is advertising as **`RAK-SOIL-xx`** where xx is the BLE MAC address of the device.

## 2) Setup over USB port
Using the AT command interface the WisBlock can be setup over the USB port.

A detailed manual for the AT commands are in [AT-Commands.md](./AT-Commands.md) :arrow_heading_up:

Example setup for LoRaWAN, OTAA join mode, sending data every 60 minutes. DevEUI, AppEUI and AppKey in this example are random choosen and need to be changed to your setup. You need to have a matching device setup in the LPWAN server.

```
AT+NWM=1
AT+NJM=1
AT+BAND=10
AT+DEVEUI=1000000000000001
AT+APPEUI=AB00AB00AB00AB00
AT+APPKEY=AB00AB00AB00AB00AB00AB00AB00AB00
AT+SENDFREQ=3600
AT+JOIN=1,1,10,10
```

| Command | Explanation |
| --- | --- |
| **AT+NWM=1**  |  set the node into LoRaWAN mode |
| **AT+NJM=1**  |  set network join method to OTAA |
| **AT+BAND=10**  |  set LPWAN region (here AS923-3) see [AT Command Manual](./AT-Commands.md) :arrow_heading_up: for all regions |
| **AT+DEVEUI=1000000000000001**  |  set the device EUI, best to use the DevEUI that is printed on the label of your WisBlock Core module |
| **AT+APPEUI=AB00AB00AB00AB00**  |  set the application EUI, required on the LoRaWAN server  |
| **AT+APPKEY=AB00AB00AB00AB00AB00AB00AB00AB00**  |  set the application Key, used to encrypt the data packet during network join |
| **AT+SENDFREQ=3600**  |  set the frequency the sensor node will send data packets. 3600 == 60 x 60 seconds == 1 hour |
| **AT+JOIN=1,1,10,10**  |  start to join the network, enables as well auto join after a power up or a device reset |

----

# Custom AT commands
The application supports two custom AT commands for the calibration and reading the calibration values.

## AT+DRY

Description: Custom AT command to initialize dry calibration and read the dry calibration value

This command is used to start an dry calibration. The result of the calibration is stored in the EEPROM of the sensor. This command is used as well to read back the calibrarion value.

| Command                    | Input Parameter | Return Value                | Return Code |
| -------------------------- | --------------- | --------------------------- | ----------- |
| AT+DRY?                    | -               | *`Get/Set dry calibration value` | `OK`        |
| AT+DRY=?                    | -               | *`Dry calibration value` | `OK`        |
| AT+DRY   | -   | -                       | `OK`        |

**Examples**:

```
AT+DRY=?
Dry Calibration Value: 450
OK

AT+DRY
Start Dry Calibration
New Dry Calibration Value: 465
OK
```
_**REMARK**_
- The calibration value is stored on the sensor, not in the WisBlock Core module. This makes it possible to calibrate the sensor and then use it with another WisBlock Core module without the need to repeat the calibration.

[Back](#content)    

## AT+WET

Description: Custom AT command to initialize wet calibration and read the wet calibration value

This command is used to start an wet calibration. The result of the calibration is stored in the EEPROM of the sensor. This command is used as well to read back the calibrarion value.

| Command                    | Input Parameter | Return Value                | Return Code |
| -------------------------- | --------------- | --------------------------- | ----------- |
| AT+WET?                    | -               | *`Get/Set wet calibration value` | `OK`        |
| AT+WET=?                    | -               | *`Wet calibration value` | `OK`        |
| AT+WET   | -   | -                       | `OK`        |

**Examples**:

```
AT+WET=?
Wet Calibration Value: 660
OK

AT+WET
Start Wet Calibration
New Wet Calibration Value: 685
OK
```
_**REMARK**_
- The calibration value is stored on the sensor, not in the WisBlock Core module. This makes it possible to calibrate the sensor and then use it with another WisBlock Core module without the need to repeat the calibration.

[Back](#content)    

----

# Packet data format
The packet data is made compatible with the extended Cayenne LPP encoding from [ElectronicCats/CayenneLPP](https://github.com/ElectronicCats/CayenneLPP) ??????.  

The packet data is using 5 channels:     

| Data            | Channel # | Channel ID | Length  | Comment        |
| --              | --        | --         | --      | --             |
| Humidity        | 1         | 104        | 2 bytes | in %RH         |
| Temperature     | 2         | 103        | 1 bytes | in ??C          |
| Battery         | 3         | 2          | 2 bytes | in V           |
| Validity        | 4         | 102        | 2 bytes | true or false  |
| Raw capacitance | 5         | 2          | 2 bytes | raw value      |

Example decoders for TTN, Chirpstack, Helium and Datacake can be found in [https://github.com/RAKWireless/WisBlock/Cayenne_Ext_LPP] ??????

----

# Compiled output
The compiled files are located in the [./Generated](./Generated) folder. Each successful compiled version is named as      
**`WisBlock_SOIL_Vx.y.z_YYYYMMddhhmmss`**    
x.y.z is the version number. The version number is setup in the [./platformio.ini](./platformio.ini) file.    
YYYYMMddhhmmss is the timestamp of the compilation.

The generated **`.hex`** file can be used as well to update the device over BLE using either [WisBlock Toolbox](https://play.google.com/store/apps/details?id=tk.giesecke.wisblock_toolbox) :arrow_heading_up: or [Nordic nRF Toolbox](https://play.google.com/store/apps/details?id=no.nordicsemi.android.nrftoolbox) :arrow_heading_up: or [nRF Connect](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) :arrow_heading_up:

----

# Debug options 
Debug output can be controlled by defines in the **`platformio.ini`**    
_**LIB_DEBUG**_ controls debug output of the SX126x-Arduino LoRaWAN library
 - 0 -> No debug output
 - 1 -> Library debug output (not recommended, can have influence on timing)    

_**API_DEBUG**_ controls debug output of WisBlock API
 - 0 -> No debug output    
 - 1 -> WisBlock API debug output    

_**MY_DEBUG**_ controls debug output of the application itself    
 - 0 -> No debug output    
 - 1 -> Application debug output    

_**NO_BLE_LED**_ controls the BLE status LED
- 0 -> LED active, blinks while advertising, on when connected
- 1 -> LED inactive

_**CFG_DEBUG**_ controls the debug output of the nRF52 BSP. It is recommended to keep it off

## Example for no debug output and maximum power savings:

```ini
[env:wiscore_rak4631]
platform = nordicnrf52
board = wiscore_rak4631
framework = arduino
build_flags = 
    ; -DCFG_DEBUG=2
	-DSW_VERSION_1=1 ; major version increase on API change / not backwards compatible
	-DSW_VERSION_2=0 ; minor version increase on API change / backward compatible
	-DSW_VERSION_3=0 ; patch version increase on bugfix, no affect on API
	-DLIB_DEBUG=0    ; 0 Disable LoRaWAN debug output
	-DAPI_DEBUG=0    ; 0 Disable WisBlock API debug output
	-DMY_DEBUG=0     ; 0 Disable application debug output
	-DNO_BLE_LED=1   ; 1 Disable blue LED as BLE notificator
lib_deps = 
	beegee-tokyo/WisBlock API
	beegee-tokyo/RAK12035_SoilMoisture
	sparkfun/SparkFun LIS3DH Arduino Library
extra_scripts = pre:rename.py
```

----

# Changelog

## V1.0.2
  - Smaller changes to match with the lastest version of WisBlock-API

## V1.0.1
  - First release under the RAKWireless Github account

## V1.0.0
  - First code, abandoned now.