#ifndef PH_READING_H
#define PH_READING_H

#include <Arduino.h>
#include "ads_wrapper.h"

/**
    No need for begin here,
    begin done in main directly using wrapper
**/

class PHWrapper
{
public:
    PHWrapper(const uint8_t pinEN) : _pinEN(pinEN) {}
    ~PHWrapper() {}

    float phReading(ADSWrapper &ads)
    {
        digitalWrite(_pinEN, HIGH);
        float x = ads.adsReading(0);
        digitalWrite(_pinEN, LOW);
        return x;
    }

private:
    const uint8_t _pinEN;
};

#endif