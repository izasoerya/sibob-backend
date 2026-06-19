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
        static float smoothedPH = -1; // -1 = not initialized yet
        const float ALPHA = 0.2;      // smoothing factor

        const float BASELINE_RAW = 17660.0; // "in air" reading, treated as pH 7 reference
        const float COUNTS_PER_PH = 1440.0; // raw count change per pH unit (derived below)

        digitalWrite(_pinEN, LOW);
        delay(15); // let sensor stabilize after enabling
        int16_t raw = ads.adsReading(0);
        digitalWrite(_pinEN, HIGH);

        float ph = 7.0 + (BASELINE_RAW - raw) / COUNTS_PER_PH;

        if (smoothedPH < 0)
            smoothedPH = ph;
        else
            smoothedPH = ALPHA * ph + (1 - ALPHA) * smoothedPH;

        return smoothedPH;
    }

private:
    const uint8_t _pinEN;
};

#endif