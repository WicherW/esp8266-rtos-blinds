# ESP8266 RTOS-SDK Blinds Controller

### Warning: This project is under active development and may not be stable yet.

A simple Wi-Fi controlled blinds driver based on ESP8266 using RTOS SDK.  
This project uses Docker to simplify the development environment setup.

There are two blinds controlled in this project: one larger and one smaller. The sizes are fixed and hardcoded in the firmware.

## WiFi Credentials Setup

1. Make a file named `wifi_data.h` in `/main` folder.
2. Fill it with the template below and enter your WiFi credentials: 
```c 
#ifndef WIFI_DATA_H
#define WIFI_DATA_H

#define WIFI_SSID "SSID"
#define WIFI_PASS "PASS"

#endif

```

## Pinout 
TODO :)

## Docker - Windows
This project uses Docker, so make sure you have Docker and WSL2 installed.
You also need to download **usbipd** from GitHub.

` https://github.com/dorssel/usbipd-win `

Before each upload or after a USB/PC reset on Windows:

` usbipd list `

Read the device ID and then run:

` usbipd attach --wsl --busid <busid> `


## Docker
To build the container:

` docker build -t esp8266_blinds . `

To run it with the serial port:

` docker run --rm -it --device=/dev/ttyUSB0 esp8266_blinds `

Finally, click on the bottom-left corner (in VS Code) to open the `connection menu` and select `Reopen in Container`


## SPIFFS
Generate the image:

` mkspiffs -c /workspace/data -b 4096 -p 256 -s 0xF0000 /workspace/data_images/image_to_flash.img `

And then flash:

` /opt/sdk/components/esptool_py/esptool/esptool.py -p /dev/ttyUSB0 write_flash 0x210000 /workspace/data_images/image_to_flash.img `


## Serial Monitor
To launch the monitor:

` idf.py monitor `

If it doesn’t work, run:

` lsof /dev/ttyUSB0 `

And then:

` kill <PID> `

To check if the port is visible inside the container:

` ls -l /dev/ttyUSB0 `

## Known Issues

Currently, there is a problem with the LED constantly being on because there are not enough GPIO pins available to control the blinds and the LED separately.