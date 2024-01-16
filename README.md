# A collection of Arduino projects

## Temperature Monitor (tempmon)
Measures temperature and humidity from a sensor and send results to the cloud

### Features
* Read temperature sensor
* Send data to cloud
* Deep sleep for battery power
* Http firmware update
* Coded to run on ESP 8266 Nodemcu

## Basic Wifi (basicwifi)
A template for a basic Wifi setup and connect

### Features
* Supports ESP8266 and ESP32
* Displays MAC address
* Set hostname
* Optional fixed static IP
* Optional multi SSID config

## Current Monitor (currentmon-ina219)
A basic current monitor using an INA219 mondule e.g. https://www.adafruit.com/product/904
Either prints to the serial port or if a TFT display is connected will display values there.

### Features
* For ESP32
* Designed for a INA219 connected to the I2C bus
* Prints values to serial port every 2s, uses Adafruit library (Adafruit_INA219)
* Prints values to a TFT display (optional), uses TFT_eSPI library (https://github.com/Bodmer/TFT_eSPI)