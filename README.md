# Humid Matic

A humidifier "brain". It pulls humidity data from an InfluxDB and based on it controll a humidifier so a specific relative humidity level is reached.

It has 1 input from a sensor for the humidifier water level. And there are two outputs - PWM for the piezo element driver and for the humidifier fan.

Circuit and PCB is available at https://easyeda.com/funnybrum/humidifier-brain. A primitive schema that may never evolve if there is no need for that.

## Building the project

The project uses a common set of tools that are availabe in another repo - https://github.com/funnybrum/esp8266-base. Clone the esp8266-base repo in the lib folder:

```
cd lib
git clone git@github.com:funnybrum/esp8266-base.git
```

Actually the changes should become part of the esp8266-base project. This introduces only the required files to enable the development of the POC. Once done all details about this will be documented in the esp8266-base project.