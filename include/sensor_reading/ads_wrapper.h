#ifndef ADS_WRAPPER_H
#define ADS_WRAPPER_H

#include <Arduino.h>
#include "ADS1X15.h"

class ADSWrapper
{
public:
    ADSWrapper(ADS1115 &ads) : _ads(ads) {}
    ~ADSWrapper() {}

    int8_t adsBegin()
    {
        if (!_ads.begin())
        {
            Serial.println("ADS not found!");
            return 0;
        }
        _ads.setGain(1);
        _ads.setMode(0);
        _ads.readADC(0);

        return 1;
    }

    float adsReading(uint8_t channel)
    {
        return _ads.readADC(channel);
    }

private:
    ADS1115 &_ads;
};

#endif