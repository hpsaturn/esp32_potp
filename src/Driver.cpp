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
#include "OLEDDisplayUi.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Define on platformio.ini or env
const char *ssid         = WIFI_SSID;
const char *password     = WIFI_PASS;
// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 5, 4);

int threshold = 40;
bool touch1detected = false;
bool touch5detected = false;

void gotTouch1(){
 touch1detected = true;
}

void gotTouch5(){
 touch5detected = true;
}

void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting");

  display.init();
  display.flipScreenVertically();
  display.setContrast(255);
  display.clear();

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  String msg = "WiFi Setup\nConnecting to ";
  display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10,msg+ssid);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    display.clear();
    display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10, "Wifi Setup Failed!\nRebooting..");
    display.display();
    delay(3000);
    ESP.restart();
  }
    
  display.clear();

  if (!MDNS.begin("esp32potp")) {
    Serial.println("Error setting up MDNS responder!");
    display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10, "MDNS Setup Failed!\nRebooting..");
    display.display();
    delay(3000);
    ESP.restart();
  }

  ArduinoOTA.begin();
  ArduinoOTA.onStart([]() {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10, "OTA Update");
    display.display();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    display.drawProgressBar(4, 32, 120, 8, progress / (total / 100) );
    display.display();
  });

  ArduinoOTA.onEnd([]() {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, "Restart");
    display.display();
  });

  // Align text vertical/horizontal center
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_10);
  display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, "Ready for OTA:\n" + WiFi.localIP().toString());
  display.display();

  // Init touch callbacks
  Serial.println("ESP32 Touch Interrupt Test");
  touchAttachInterrupt(T2, gotTouch1, threshold);
  touchAttachInterrupt(T5, gotTouch5, threshold);
}

void loop() {
  ArduinoOTA.handle();

  if(touch1detected){
    touch1detected = false;
    Serial.println("Touch 1 detected");
  }
  if(touch5detected){
    touch5detected = false;
    Serial.println("Touch 5 detected");
  }

}

