#ifndef DHT_READING_H
#define DHT_READING_H

#include <Arduino.h>
#include <DHT11.h>

class DHTWrapper
{
public:
    DHTWrapper(const uint8_t pinDHT) : _pinDHT(pinDHT), _dht11(DHT11(pinDHT)) {}
    ~DHTWrapper() {}

    uint8_t begin()
    {
        int result = _dht11.readTemperature();
        if (result == 0)
        {
            return 1;
        }
        else
        {
            Serial.println(DHT11::getErrorString(result));
            return 0;
        }
    }

    float getTemperature()
    {
        return _dht11.readTemperature();
    }
    float getHumidity()
    {
        return _dht11.readHumidity();
    }

private:
    const uint8_t _pinDHT;
    DHT11 _dht11;
};

#endif