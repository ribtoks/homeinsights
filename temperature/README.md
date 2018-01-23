Objective is to report temperature from multiple Arduinos to small server in Raspberry Pi which will show local website with schemas and temperatures.

Manual currently for Arduino Uno + Raspberry Pi

# Arduino Uno

## Programming

* install Arduino IDE
* install libraries `DHT` v1.2.3, `rc-switch` latest
* flush sketch in `arduino/` dir onto board

## Wiring

* solder antenna to 433MHz transmitter
* connect as in manual

![Arduino Uno wiring](https://github.com/ribtoks/homeinsights/raw/master/temperature/assets/20180122_125103.jpg)

&nbsp;

&nbsp;

# Raspberry Pi

## Programming

* disto Raspbian with `gcc`, `git`, `libsqlite3-dev`, `sqlite3`
* install `gpio` library
* install `wiringPi` library
* install `433Utils` library (you will need `rc-switch` from that repo)
* copy and build `tempserver` in `rpi/` directory on RaspberryPi

## Wiring

* solder antenna to 433MHz receiver
* connect as in manual (connect data to GPIO pin 21/27)

![Raspberry Pi wiring](https://github.com/ribtoks/homeinsights/raw/master/temperature/assets/20180122_125214.jpg)
