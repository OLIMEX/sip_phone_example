# SIP Phone Example

This example allows users to make calls over the internet.

## Compatibility

ESP32-ADF
https://www.olimex.com/Products/IoT/ESP32/ESP32-ADF/open-source-hardware

# Get ESP-ADF
```bash
cd ~/
git clone --recursive https://github.com/espressif/esp-adf.git
cd esp-adf
git submodule update --init
export ADF_PATH=~/esp-adf
```


## Usage

Prepare the audio board:

- Connect speakers or headphones to the board.

Load the example:
```bash
git clone TO BE FILLED
cd  TO BE FILLED
./configure
make menuconfig
```

Configure the example:

- Select compatible audio board in `menuconfig` > `Audio board select`.
- Set up Wi-Fi connection by running `menuconfig` > `VOIP App Configuration` and filling in `WiFi SSID` and `WiFi Password`.
- Select compatible audio codec in `menuconfig` > `VOIP App Configuration` > `SIP Codec`.
- Create the SIP extension, ex: 100 (see below)
- Set up SIP URI in `menuconfig` > `VOIP App Configuration` > `SIP_URI`.

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
