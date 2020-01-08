
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
        void loop(uint32_t nodeId, int node_list_count, String version, int unread_count);
    
    private:
        TFT_eSPI _tft = {135, 240};
};

#endif