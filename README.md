# SIP Phone Example

This example allows users to make calls over the internet. The project is basic, but you are free to evolute.

## Compatibility

ESP32-ADF
https://www.olimex.com/Products/IoT/ESP32/ESP32-ADF/open-source-hardware

![ESP32-ADF](https://olimex.files.wordpress.com/2019/08/esp32-adf-sip1.jpg?w=535)

# Get ESP-ADF
```bash
cd ~/
git clone https://github.com/espressif/esp-adf.git
cd esp-adf
git checkout tags/v2.0-beta1-55-g9bcaec2
git submodule update --init
export ADF_PATH=~/esp-adf
```


## Usage

Prepare the audio board:

- Connect speakers or headphones to the board.

Load the example:
```bash
git clone --recursive https://github.com/d3v1c3nv11/sip_phone_example.git
cd  sip_phone_example
git submodule update --init
cp lvgl_component.mk components/lvgl/component.mk
make menuconfig
```

Configure the example:

- Select compatible audio board in `menuconfig` > `Audio board select`.
- Set up Wi-Fi connection by running `menuconfig` > `VOIP App Configuration` and filling in `WiFi SSID` and `WiFi Password`.
- Select compatible audio codec in `menuconfig` > `VOIP App Configuration` > `SIP Codec`.
- Create the SIP extension, ex: 100 (see below)
- Set up SIP URI in `menuconfig` > `VOIP App Configuration` > `SIP_URI`.

Upload the example:
```bash
make flash monitor
```

Configure external application:

 Setup the PBX Server like Yet Another Telephony Engine (FreePBX/FreeSwitch or any other PBXs)
 http://docs.yate.ro/wiki/Beginners_in_Yate

## Features
- Lightweight
- Support multiple transports for SIP (UDP, TCP, TLS)
- Support G711A/8000 & G711U/8000 Audio Codec
- Easy setting up by using URI

## Reference
http://www.yate.ro/
https://www.tutorialspoint.com/session_initiation_protocol/index.htm
https://tools.ietf.org/html/rfc3261
