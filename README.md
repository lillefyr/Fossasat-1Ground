# Fossasat-1 Groundstation
Fossasat-1 Ground station Arduino Uno R3 and Dragino Lora Shield

Spain's first picosatellite (FOSSASAT-1) is set to be launched in Q3 of 2019.

To prepare for this event I have created a ground station based on information at http://fossa.systems/ and the official git repo at https://github.com/Bambofy/FOSSASAT-1

## Hardware

The groundstation is based on 2 Arduino UNO R3. One has a Dragino Lora Shield the second a 2.4 inch TFT LCD Touch Screen Shield for arduino UNO R3.
The micro-controllers are connected via I2C.

## Software

Software is developen with the Arduino GUI and consist of 3 repositories.
https://github.com/lillefyr/Fossasat-1Screen contains the display code, and can be utilised with other micro-controllers via I2C.

https://github.com/lillefyr/Fossasat-1Ground contains the ground station code.

https://github.com/lillefyr/Fossasat-1Beacon433 contains a satellite simulator (based on SX1275 and ESP8266)

## Other details

Earlier code for the groundstation was developed on RFM95 (868Mhz) and ESP8266 (Wemos Mini D1)
