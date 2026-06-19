#ifndef BANG_BANG_H
#define BANG_BANG_H

#include <Arduino.h>

struct BangBangConfig
{
    float upperTemp;
    float bottomTemp;
    float upperHum;
    float bottomHum;
};

struct ActuatorConfig
{
    uint8_t pinExhaustFan;
    uint8_t pinMistMaker;
    uint8_t pinHeater;
};

class BangBangController
{
private:
    BangBangConfig _configSetPoint;
    ActuatorConfig _configActuator;

public:
    // Lazy separate body file sorry
    BangBangController(const BangBangConfig setPointConfig,
                       const ActuatorConfig actuatorConfig)
    {
        _configSetPoint = setPointConfig;
        _configActuator = actuatorConfig;
    }
    ~BangBangController() {}

    void control(float temperature, float humidity) const
    {
        if (temperature > _configSetPoint.upperTemp)
        {
            // Turn off exhaust fan
            digitalWrite(_configActuator.pinExhaustFan, HIGH);

            Serial.print("FAN IS ON  pin: ");
            Serial.print(_configActuator.pinExhaustFan);
        }
        else if (temperature < _configSetPoint.bottomTemp)
        {
            // Turn on exhaust fan
            digitalWrite(_configActuator.pinExhaustFan, LOW);
            Serial.println("FAN IS OFF pin: ");
            Serial.print(_configActuator.pinExhaustFan);
        }

        if (humidity > _configSetPoint.upperHum)
        {
            // Turn off mist maker
            digitalWrite(_configActuator.pinMistMaker, LOW);
            Serial.print("MIST IS OFF  pin: ");
            Serial.print(_configActuator.pinMistMaker);
        }
        else if (humidity < _configSetPoint.bottomHum)
        {
            // Turn on mist maker
            digitalWrite(_configActuator.pinMistMaker, HIGH);
            Serial.print("MIST IS ON  pin: ");
            Serial.print(_configActuator.pinMistMaker);
        }
    }
};

#endif