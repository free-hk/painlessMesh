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

void TTGOTDisplay::loop(uint32_t nodeId, int node_list_count, String version, int unread_count)
{
    _tft.fillScreen(TFT_BLACK);
    _tft.setTextDatum(MC_DATUM);

    _tft.setTextSize(2);
    _tft.setTextColor(TFT_WHITE);
    String out = "WiFi Mesh v";
    out += version;
    _tft.drawString(out,  _tft.width() / 2, 10 );

    String node_id = "Node ID - ";
    node_id += nodeId;

    _tft.drawString(node_id,  _tft.width() / 2, 30 );  

    String node_count = "";
    node_count += node_list_count;
    node_count += " Node connected";

    _tft.drawString(node_count,  _tft.width() / 2, 50 );  

    _tft.setTextDatum(MC_DATUM);
    _tft.setTextSize(2);
    _tft.setTextColor(TFT_RED);

    if (unread_count > 0) {
      String unread_message = "";
      unread_message += String(unread_count);
      unread_message += " Unread Messages";

      _tft.drawString(unread_message, _tft.width() / 2, 70 );  
    }

    _tft.setTextSize(1);
    _tft.setTextColor(TFT_LIGHTGREY);
    _tft.drawString("Long press any button to show menu", _tft.width() / 2, 130);
    _tft.setTextSize(2);
}