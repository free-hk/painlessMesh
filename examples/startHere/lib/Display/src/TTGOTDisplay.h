
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

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23 

#define TFT_BL          4  // Display backlight control pin
#define ADC_EN          14
#define ADC_PIN         34

class TTGOTDisplay
{
    // protected:
    //     /* data */
    //     TFT_eSPI _tft;
    
    public:
        TTGOTDisplay() = default;
        TTGOTDisplay(int width, int height) : _tft{width, height} {};
        ~TTGOTDisplay();
        void initTFT();
        TFT_eSPI getTFT();
    
    private:
        TFT_eSPI _tft = {135, 240};
};

#endif