#ifndef SOIL_HUM_READING_H
#define SOIL_HUM_READING_H

#include <Arduino.h>
#include "ads_wrapper.h"

/**
    No need for begin here,
    begin done in main directly using wrapper
**/

float soilHumidityReading(ADSWrapper &ads)
{
    static float smoothedHumidity = -1; // -1 = not initialized yet
    const float ALPHA = 0.2;            // smoothing factor: lower = smoother but slower to react

    int raw = ads.adsReading(1);
#if defined(SIBOB_1)
    const int16_t DRY_VALUE = 4566; // no water reading
    const int16_t WET_VALUE = 4639; // water reading
#else defined(SIBOB_2)
    const int16_t DRY_VALUE = 3145; // no water reading
    const int16_t WET_VALUE = 3227; // water reading
#endif

    float humidity = (float)(raw - DRY_VALUE) / (WET_VALUE - DRY_VALUE) * 100.0;

    // clamp to 0-100% in case of noise or out-of-range readings
    if (humidity < 0)
        humidity = 0;
    if (humidity > 100)
        humidity = 100;

    if (smoothedHumidity < 0)
    {
        smoothedHumidity = humidity; // first reading, no history yet
    }
    else
    {
        smoothedHumidity = ALPHA * humidity + (1 - ALPHA) * smoothedHumidity;
    }

    return raw;
}

#endif