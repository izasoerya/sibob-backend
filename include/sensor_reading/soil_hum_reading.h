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
    return ads.adsReading(1);
}

#endif