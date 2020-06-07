
# Command API Interface

API WRITE CHARACTERISTIC

```6E400006-B5A3-F393-E0A9-E50E24DCCA9E```

API READ CHARACTERISTIC

```6E400007-B5A3-F393-E0A9-E50E24DCCA9E```

API NOTIFY CHARACTERISTIC

```6E400005-B5A3-F393-E0A9-E50E24DCCA9E```

# Command API Example

## Read Version information

Send Command
```
{"type": "check_version"}
```

Respond in Read characteristic

```
{"type":"check_version","result":"1.3.3"}
```

## Read OTA Mode

Send Command
```
{"type": "read_ota_mode"}
```

Respond in Read characteristic

```
{"type":"read_ota_mode","result":"true"}
```

or 
```
{"type":"read_ota_mode","result":"false"}
```

## Set OTA Mode

Send Command
```
{"type": "set_ota_mode", "status": "true"}
```

or 

```
{"type": "set_ota_mode", "status": "false"}
```

``` Device will restart after set the OTA mode, so no respond will be given. Application need to reconnect to the device once the device is restarted.```


# Bluetooth change mac address ( TODO )

```
#include "esp_system.h"
uint8_t mac[] = {0x98, 0xB6, 0xE9, 0x43, 0x26, 0xCC};
esp_base_mac_addr_set(mac);
```

# Reference:

https://github.com/Xinyuan-LilyGO/TTGO-T-Display
https://github.com/LennartHennigs/Button2
https://github.com/Mystereon/TTGO-T-DISPLAY-LIFE-/tree/master
https://www.tindie.com/products/ttgo/lilygor-ttgo-t-display-esp32-wifibluetooth-module/


# Development:

### For Troubleshoot
```
open .pio/libdeps/esp32/TFT_eSPI_ID1559/User_Setup_Select.h

// need comment this line
// #include <User_Setup.h>           // Default setup is root library folder

// need uncomment this line
#include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT
```
