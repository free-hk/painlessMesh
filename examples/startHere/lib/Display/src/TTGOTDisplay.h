
#pragma once

#ifndef TTGOTDisplay_h
#define TTGOTDisplay_h

/////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include "esp_adc_cal.h"

class TTGOTDisplay
{
    public:
        TTGOTDisplay() = default;
        TTGOTDisplay(int width, int height) : _tft{width, height} {};
        ~TTGOTDisplay();
        TFT_eSPI getTFT();
    
    private:
        TFT_eSPI _tft = {135, 240};
};

#endif