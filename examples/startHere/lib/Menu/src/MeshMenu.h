  #include <menu.h>
  #include <menuIO/serialIO.h>
  #include <menuIO/TFT_eSPIOut.h>
  #include <menuIO/esp8266Out.h>//must include this even if not doing web output...
  
  // ---- define menu color 
  #define Black RGB565(0,0,0)
  #define Red	RGB565(255,0,0)
  #define Green RGB565(0,255,0)
  #define Blue RGB565(0,0,255)
  #define Gray RGB565(128,128,128)
  #define LighterRed RGB565(255,150,150)
  #define LighterGreen RGB565(150,255,150)
  #define LighterBlue RGB565(150,150,255)
  #define DarkerRed RGB565(150,0,0)
  #define DarkerGreen RGB565(0,150,0)
  #define DarkerBlue RGB565(0,0,150)
  #define Cyan RGB565(0,255,255)
  #define Magenta RGB565(255,0,255)
  #define Yellow RGB565(255,255,0)
  #define White RGB565(255,255,255)

  #define MAX_DEPTH 5

  #define GFX_WIDTH 240
  #define GFX_HEIGHT 135
  #define fontW 12
  #define fontH 18