# SIP Phone Example

This example allows users to make calls over the internet. The project is basic, but you are free to evolute.

## Compatibility

ESP32-ADF
https://www.olimex.com/Products/IoT/ESP32/ESP32-ADF/open-source-hardware

![ESP32-ADF](https://olimex.files.wordpress.com/2019/08/esp32-adf-sip1.jpg?w=535)

# Get ESP-ADF

Install requared packages:

```bash
sudo apt-get install git wget flex bison gperf python python-pip python-setuptools python-serial python-click python-cryptography python-future python-pyparsing python-pyelftools cmake ninja-build ccache libffi-dev libssl-dev
```

Download and install esp-adf:

```bash
cd ~/
git clone --recursive https://github.com/espressif/esp-adf.git
cd esp-adf
git submodule update --init
export ADF_PATH=$PWD
cd esp-idf
./install.sh
. ./export.sh
```

## Usage

Prepare the audio board:

- Connect speakers or headphones to the board.

Load the example:
```bash
mkdir ~/espwork
cd ~/espwork
git clone --recursive https://github.com/OLIMEX/sip_phone_example.git
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
