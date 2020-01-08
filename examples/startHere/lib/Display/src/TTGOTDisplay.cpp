#include "Arduino.h"
#include "TTGOTDisplay.h"

TTGOTDisplay::~TTGOTDisplay()
{
}

TFT_eSPI TTGOTDisplay::getTFT()
{
    _tft.init();
    _tft.setRotation(1);
    _tft.fillScreen(TFT_BLACK);
    _tft.setTextSize(2);
    _tft.setTextColor(TFT_WHITE);
    _tft.setCursor(0, 0);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextSize(2);

    _tft.fillScreen(TFT_BLACK);
    _tft.setTextDatum(MC_DATUM);
    return _tft;
}