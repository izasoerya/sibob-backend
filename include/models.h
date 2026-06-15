#ifndef MODELS_H
#define MODELS_H

#include <Arduino.h>
#include <ArduinoJson.h>

struct SensorProp
{
    float value;
    int8_t status;
};

struct SensorData
{
    SensorProp temperature_soil;
    SensorProp humidity_soil;
    SensorProp ph;
    SensorProp weight_breed;
    SensorProp weight_yield;
    SensorProp temperature_air;
    SensorProp humidity_air;
    unsigned long created_at;

    JsonDocument toJsonDocument() const
    {
        JsonDocument doc;
        doc["temperature_soil"] = temperature_soil.value;
        doc["humidity_soil"] = humidity_soil.value;
        doc["ph"] = ph.value;
        doc["weight"] = weight_breed.value * 0.5 + weight_yield.value * 0.5;
        doc["temperature_air"] = temperature_air.value;
        doc["humidity_air"] = humidity_air.value;

        String status = "";
        if (temperature_soil.status != 0)
            status += "T";
        if (humidity_soil.status != 0)
            status += "H";
        if (ph.status != 0)
            status += "P";
        if (weight_breed.status != 0)
            status += "W";
        if (temperature_air.status != 0)
            status += "A";
        if (humidity_air.status != 0)
            status += "U";
        doc["status"] = status;
        return doc;
    }
};

struct ActuatorData
{
    uint16_t exhaust_fan;
    uint16_t mist_maker;
    uint16_t heater;
    unsigned long created_at;

    JsonDocument toJsonDocument() const
    {
        JsonDocument doc;
        doc["exhaust_fan"] = exhaust_fan;
        doc["mist_maker"] = mist_maker;
        doc["heater"] = heater;
        return doc;
    }
};

#endif // MODELS_H