#include "Arduino.h"
#include "WiFiManager.h"
#include "esp_wifi.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

WiFiManager::WiFiManager(OLEDDisplay *d){
  this->display = d;
}

void WiFiManager::enableOTA(){
  ArduinoOTA.begin();
  ArduinoOTA.onStart([&]() {
      Serial.println("ArduinoOTA starting..");
      display->clear();
      display->setFont(ArialMT_Plain_10);
      display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
      display->drawString(display->getWidth()/2, display->getHeight()/2 - 10, "OTA Update");
      display->display();
  });

  ArduinoOTA.onProgress([&](unsigned int progress, unsigned int total) {
      display->drawProgressBar(4, 32, 120, 6, progress / (total / 100) );
      display->display();
  });

  ArduinoOTA.onError([&](ota_error_t err) {
      Serial.print("OTA error: ");
      Serial.println(err);
      ESP.restart();
  });

  ArduinoOTA.onEnd([&]() {
      Serial.println("OTA complete.");
      display->clear();
      display->setFont(ArialMT_Plain_10);
      display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
      display->drawString(display->getWidth()/2, display->getHeight()/2, "Restart");
      display->display();
  });

  isEnableOTA=true;

}

bool WiFiManager::init(){

  display->clear();
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  String msg = "WiFi Setup\nConnecting to ";
  display->drawString(display->getWidth()/2, display->getHeight()/2 - 10,msg+ssid);
  display->display();
  Serial.print("WiFi Setup => connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED && reconnect<MAX_RECONNECT) {
    Serial.print("Connection Failed! try to reconnect:");
    Serial.println(reconnect);
    reconnect++;
    delay(100);
  }

  reconnect=0;

  if(WiFi.isConnected()){
    Serial.println("WiFi ready");
    WiFi.setHostname("esp32potp");
    display->clear();

    if(!isEnableOTA)enableOTA();

    // Align text vertical/horizontal center
    display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display->setFont(ArialMT_Plain_10);
    display->drawString(display->getWidth()/2, display->getHeight()/2, "Ready for OTA:\n" + WiFi.localIP().toString());
    display->display();
    Serial.println("OTA ready");

    isWifiEnable=true;
  }
  else {
    Serial.println("WiFi setup failed!");
    isWifiEnable = false;
    display->clear();
    display->drawString(display->getWidth()/2, display->getHeight()/2 - 10, "Wifi Setup Failed!\nPress B to reboot");
    display->display();
    delay(1000);
  }

  return isWifiEnable;

}

void WiFiManager::disableWifi(){
  if(isWifiEnable){
    Serial.println("Disabling WiFi..");
    esp_wifi_set_ps(WIFI_PS_MODEM);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    WiFi.disconnect(true);
    ESP.restart();
  }
}
