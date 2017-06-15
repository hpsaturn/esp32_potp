/*
  Morse.h - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef WifiManager_h
#define WifiManager_h

#include "Arduino.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

#define MAX_RECONNECT 1

class WiFiManager
{
  public:
    WiFiManager(SSD1306& d);
    void enableOTA();
    void initWiFi();
    void disableWifi();
    bool isWifiEnable = false;

  private:
    SSD1306 display;
    // WiFi setup, define secrets on global enveiroment
    const char *ssid         = WIFI_SSID;
    const char *password     = WIFI_PASS;
    bool isEnableOTA = false;
    int reconnect = 0;
};

#endif
