# Battery driven wireless temperature sensor

Arduino based battery driven wireless temperature sensor.

## Introduction


## ATmega328 on a breadboard

In order to save battery the ATmega328P is set to use its internal 8 MHz RC oscillator as a clock. This way we can set up a minimal circuit, eliminating the need for extra components like the the 16 MHz crystal. See the [From Arduino to a Microcontroller on a Breadboard](https://docs.arduino.cc/built-in-examples/arduino-isp/ArduinoToBreadboard) tutorial for instructions on how to burn a bootloader and setting up the minimal ATmega328 circuit on a breadboard.


## Hardware

* ATmega328P
* HopeRF RFM69CW 13DBM 433Mhz RF Transceiver Module
* DS18B20 Digital temperature sensor


## Arduino sketch dependencies

* [Arduino AVR Boards](https://github.com/arduino/ArduinoCore-avr)
* [RadioHead Packet Radio library for embedded microprocessors](http://www.airspayce.com/mikem/arduino/RadioHead/)
* [One Wire](https://github.com/PaulStoffregen/OneWire)
* [Arduino Library for Maxim Temperature Integrated Circuits](https://github.com/milesburton/Arduino-Temperature-Control-Library)
* [Low-Power](https://github.com/rocketscream/Low-Power)


## Compile, upload and monitor ATmega328p using the Arduino CLI

### Add secrets
Copy `arduino_secrets.h.example` to `arduino_secrets.h` and change the example secret values.

### Download and install Arduino IDE and/or CLI
[Arduino CLI](https://arduino.github.io/arduino-cli/) is an all-in-one solution that provides Boards/Library Managers, sketch builder, board detection, uploader, and many other tools needed to use any Arduino compatible board and platform from command line or machine interfaces.

## Download and burn bootloader
Download the [ATmega328 on a breadboard bootloader](https://www.arduino.cc/en/uploads/Tutorial/breadboard-1-6-x.zip) and follow the instructions in the [From Arduino to a Microcontroller on a Breadboard](https://docs.arduino.cc/built-in-examples/arduino-isp/ArduinoToBreadboard) tutorial.

### Install library dependecies
Install the library dependencies listet above.

### Compile sketch
`arduino-cli compile -b breadboard:avr:atmega328bb -v rfm69-client`

### Upload sketch to ATmega328P using e.g. a FTD-232R-3V3 USB to TTL Serial Cable:
`arduino-cli upload -p /dev/cu.usbserial-FTC0DLEV -b breadboard:avr:atmega328bb -v rfm69-client`

### Open the serial monitor
`arduino-cli monitor -p /dev/cu.usbserial-FTC0DLEV -c baudrate=115200`