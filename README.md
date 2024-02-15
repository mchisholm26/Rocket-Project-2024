# Rocket-Project-2024

The purpose of this code is to control the rocket being launched by Engineering Club at Cornell College in April 2024.

## Hardware

* [Teensy® 4.1](https://www.pjrc.com/store/teensy41.html)
* [SparkFun Triple Axis Accelerometer Breakout - KX134 (Qwiic)](https://www.sparkfun.com/products/17589)
* [MPL3115A2 - I2C Barometric Pressure/Altitude/Temperature Sensor](https://www.adafruit.com/product/1893)

## Software (on Rocket)
~~* [CircutPython](https://circuitpython.org/board/teensy41/)~~
* C++

## Software (on Ground)
* TBD

## Objectives:

PROVE SUPERSONIC
1) detect lauch
2) Alter Sampling Rate upon descend
3) write all data to SD card for later calculation

Teensy reads data from accelerometer and barometer, writes to SD Card
desired data from barometer: altitude
desired data from accelerometer: acceleration (all 3 axes)

## Variables Required to Achieve

1) Detect Lauch - acceleration and altitude 
2) Alter Sampling rate - detect descend when acceleration value=0 and/or altitude begins decreasing
3) Write all data to SD card when landed - when landed or right before in case of damage? either way use 
