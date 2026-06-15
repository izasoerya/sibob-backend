#ifndef DS18_READING_H
#define DS18_READING_H

#include <Arduino.h>
#include "DS18B20.h"

int8_t soilTemperatureBegin(DS18B20 &ds)
{
    if (!ds.begin())
    {
        Serial.println("DS18 Sensor begin failed!");
        return 0;
    }
    ds.setResolution(12);
    return 1;
}

float soilTemperatureReading(DS18B20 &ds)
{
    ds.requestTemperatures();
    while (!ds.isConversionComplete())
        delay(1); // Stupid lib blocking shit
    return ds.getTempC();
}

#endif