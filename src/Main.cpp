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
#include "SSD1306Wire.h" // alias for `#include "SSD1306Wire.h"`
#include "WiFiManager.h"
#include "esp_deep_sleep.h"
#include "sha1.h"
#include "TOTP.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "lwip/err.h"
#include "apps/sntp/sntp.h"

// The shared secret is MyLegoDoor
uint8_t hmacKey[] = {0x4d, 0x79, 0x4c, 0x65, 0x67, 0x6f, 0x44, 0x6f, 0x6f, 0x72};

TOTP totp = TOTP(hmacKey, 10);
char code[7];

#define THRESHOLD 80

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, 5, 4);
WiFiManager wifi(&display);
// Power vars
unsigned long poweroff = 0;
bool isPowerOff = false;
// Touche keys setup
bool touch2detected = false;
bool touch3detected = false;
int touch2count = 0;
int touch3count = 0;

void gotTouch2(){
 touch2detected = true;
}

void gotTouch3(){
 touch3detected = true;
}

void reboot(){
  Serial.println("Process Reboot..");
  ESP.restart();
}

void showWelcome(){
  Serial.println("Welcome ready");
  isPowerOff=false;
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_16);
  display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, "ESP32 POTP");
  display.display();
  delay(1000);
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

static void initialize_sntp(void)
{
    Serial.println("Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
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

  time_t now;
  struct tm timeinfo;
  time(&now);

  showWelcome();

  Serial.println("== Setup ready ==");
}

void loop() {

  if(wifi.isWifiEnable)ArduinoOTA.handle();

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
      if(wifi.isWifiEnable)wifi.disableWifi();
      else if(wifi.init()){
        initialize_sntp();
      }
    }
  }

  /*
  if(poweroff++>1000000){
    Serial.println("Auto Power Off");
    poweroff=0;
    suspend();
  }
  */

  time_t now;
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  long GMT = time(&now);

  char* newCode = totp.getCode(GMT);
  if(strcmp(code, newCode) != 0) {
    strcpy(code, newCode);
    Serial.print(timeinfo.tm_year);
    Serial.print(" [");
    Serial.print(code);
    Serial.println("]");
  }

  delay(10); // fix for OTA upload freeze

}

