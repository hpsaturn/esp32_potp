/**
 * @hpsaturn 2017
 *
 * Testing for WeMos board:
 * https://www.aliexpress.com/item/ESP8266-OLED-ESP32-wemos-for-arduino-ESP32-OLED-WiFi-Modules-Bluetooth-Dual-ESP-32-ESP-32S/32811052595.html
 *
 * Requeriments:
 *
 * platformio lib install 562
 *
 * Intall firmware with:
 *
 * platformio run -e lolin32 --target upload
 *
 */

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "esp_deep_sleep.h"
#include "esp_wifi.h"

#define MAX_RECONNECT 1
#define THRESHOLD 80

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 5, 4);
// WiFi setup, define secrets on global enveiroment
const char *ssid         = WIFI_SSID;
const char *password     = WIFI_PASS;
bool isWifiEnable = false;
bool isEnableOTA = false;
int reconnect = 0;
// Touche keys setup
bool touch2detected = false;
bool touch3detected = false;
int touch2count = 0;
int touch3count = 0;
unsigned long poweroff = 0;
bool isPowerOff = false;

void gotTouch2(){
 touch2detected = true;
}

void gotTouch3(){
 touch3detected = true;
}

void enableOTA(){
  ArduinoOTA.begin();
  ArduinoOTA.setHostname("esp32potp");
  WiFi.setHostname("esp32potp");
  ArduinoOTA.onStart([]() {
      Serial.println("ArduinoOTA starting..");
      display.clear();
      display.setFont(ArialMT_Plain_10);
      display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
      display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10, "OTA Update");
      display.display();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      display.drawProgressBar(4, 32, 120, 6, progress / (total / 100) );
      display.display();
  });

  ArduinoOTA.onError([](ota_error_t err) {
      Serial.print("OTA error: ");
      Serial.println(err);
      ESP.restart();
  });

  ArduinoOTA.onEnd([]() {
      Serial.println("OTA complete.");
      display.clear();
      display.setFont(ArialMT_Plain_10);
      display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
      display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, "Restart");
      display.display();
  });

  isEnableOTA=true;
 
}

void initWiFi(){
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  String msg = "WiFi Setup\nConnecting to ";
  display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10,msg+ssid);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED && reconnect<MAX_RECONNECT) { 
    Serial.print("Connection Failed! try to reconnect:");
    Serial.println(reconnect);
    reconnect++;
  }
  reconnect=0;

  if(WiFi.isConnected()){
    Serial.println("WiFi ready");
    display.clear();

    if (!MDNS.begin("esp32potp")) {
      Serial.println("Error setting up MDNS responder!");
      display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10, "MDNS Setup Failed!\nPress B to reboot");
      display.display();
      delay(3000);
    }
    if(!isEnableOTA)enableOTA();

    // Align text vertical/horizontal center
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.setFont(ArialMT_Plain_10);
    display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, "Ready for OTA:\n" + WiFi.localIP().toString());
    display.display();
    Serial.println("OTA ready");

    isWifiEnable=true;
  }
  else {
    Serial.println("WiFi setup failed!");
    isWifiEnable = false;
    display.clear();
    display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10, "Wifi Setup Failed!\nPress B to reboot");
    display.display();
  }
}

void showWelcome(){
  Serial.println("onWelcomeScreen");
  isPowerOff=false;
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_16);
  display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, "ESP32 POTP");
  display.display();
  delay(1000);
}

void disableWifi(){
  if(isWifiEnable){ 
    esp_wifi_set_ps(WIFI_PS_MODEM);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    isWifiEnable=false;
    showWelcome();
  }
}

void suspend(){
  Serial.println("Process Suspend..");
  isPowerOff=true; 
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, "Suspending..");
  display.display();
  delay(3000);
  display.clear();
  display.display();
  //esp_deep_sleep_enable_timer_wakeup(10000000);
  esp_deep_sleep_enable_touchpad_wakeup();
  esp_deep_sleep_start();
}

void reboot(){
  Serial.println("Process Reboot..");
  ESP.restart();
}

void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println("== ESP32 Booting ==");

  display.init();
  display.flipScreenVertically();
  display.setContrast(255);
  display.clear();
  Serial.println("OLED ready");

  // Init touch callbacks
  touchAttachInterrupt(T2, gotTouch2, THRESHOLD);
  touchAttachInterrupt(T3, gotTouch3, THRESHOLD);
  Serial.println("Buttons ready");

  showWelcome();

  Serial.println("== Setup ready ==");
}

void loop() {

  if(isWifiEnable)ArduinoOTA.handle();

  if(touch2detected){
    touch2detected = false;
    if(touch2count++>10){
      Serial.println("Touch 2 (GPIO2) reached");
      touch2count=0;
      if(isPowerOff)showWelcome();
      else suspend();
    }
  }

  if(touch3detected){
    touch3detected = false;
    if(touch3count++>5){
      Serial.println("Touch 3 (GPIO15) reached");
      touch3count=0;
      if(isWifiEnable)disableWifi();
      else initWiFi();
    }
  }

  /*
  if(poweroff++>9000000){
    Serial.println("Auto Power Off");
    poweroff=0;
    suspend();
  }
  */

}

