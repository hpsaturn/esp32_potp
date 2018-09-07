#ifndef WifiManager_h
#define WifiManager_h

#include "Arduino.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "OLEDDisplay.h"

#define MAX_RECONNECT 5

class WiFiManager
{

  public:

    WiFiManager(OLEDDisplay *d);

    OLEDDisplay *display;

    bool init();

    void disableWifi();

    void enableOTA();

    bool isWifiEnable = false;

  private:

    // WiFi setup, define secrets on global enveiroment
    const char *ssid         = WIFI_SSID;

    const char *password     = WIFI_PASS;

    bool isEnableOTA = false;

    int reconnect = 0;

};

#endif
