# ESP-IDF project for CrankshaftNG media control using CAN inputs

This ESP-IDF (developed on ESP32S3) project emulates a keyboard based on messages received over CAN bus. If plugged into the right CAN line in a car it can read button presses eg. on the steering wheel and, for example, skip a song.

## Functions
- Virtually pressing buttons based on received CAN messages

## How to use
Your ESP32 chip must have tinyusb support to register as a device (eg. ESP32 S3). It also has to have a TWAI controller on board. You will need an external transceiver module such as SN65HVD230. You will need to set in menuconfig the Tinyusb hid device count to 1 from 0 in order to compile it successfully.

You will need to modify the conditions for the received messages. To know the correct ID and bits of the message you will need to sniff the messages. You can do it using this other repo and a program from adamtheone 
https://github.com/DamianK77/can_obd_reader
