/**
* [status: in developing]
* Portable EAP Protected One-Time Password (EAP-POTP) or GoogleAuthenticator
* hardware for provides two-factor user authentication.

* This is a PlatformIO DIY project for WeMOS like board with ESP32 and
* OLED SSD1306 display, see materials and instructions on:
* https://github.com/hpsaturn/esp32_potp
*
* Before, please setup your WiFi on secrets.load.sample and run:
* cp secrets.load.sample secrets.load
* chmod 755 secrets.load
* export PLATFORMIO_BUILD_FLAGS=`bash ./secrets.load`
*
* Build and run:
* platformio run --target upload
* platformio run --target upload --upload-port 192.168.x.x
*/

#include <Arduino.h>
#include <SSD1306.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include "WiFiManager.h"
#include "esp_sleep.h"
#include "sha1.h"
#include "TOTP.h"
#include "lwip/err.h"
#include "apps/sntp/sntp.h"

// firmware version from git rev-list command
String VERSION_CODE = "rev";
#ifdef SRC_REV
int VCODE = SRC_REV;
#else
int VCODE = 0;
#endif
// OTP setup ==> test code (from GoogleAuth)
uint8_t hmacKey[] = {0xea,0x41,0x68,0x5c,0x9b,0x10,0x13,0x5d,0x8c,0xa0,0x35,0x05,0x38,0xcb,0xa9,0x96,0x75,0xa0,0x5a,0xaf};
TOTP totp = TOTP(hmacKey, 20);
char code[7];
// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, 5, 4);
WiFiManager wifi(&display);
// Power vars
unsigned long poweroff = 0;
bool isPowerOff = false;
// Touche keys setup
#define THRESHOLD 80
bool touch2detected = false;
bool touch3detected = false;
int touch2count = 0;
int touch3count = 0;
void gotTouch2(){ touch2detected = true; }
void gotTouch3(){ touch3detected = true; }

/**************************   POWER METHODS   **********************************/

void reboot(){
  Serial.println("-->Process Reboot..");
  ESP.restart();
}

void suspend(){
  Serial.println("-->Process Suspend..");
  isPowerOff=true;
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth()/2, display.getHeight()/2, "Suspending..");
  display.display();
  delay(500);
  display.clear();
  display.display();
  wifi.disableWifi();
  esp_sleep_enable_touchpad_wakeup();
  esp_deep_sleep_start();
}

/**************************   NTP HANDLING   **********************************/

static void initSNTP(void) {
  Serial.println("-->Initializing SNTP");
  configTime(0,0,"pool.ntp.org");
}

static void getTimeFromSNTP(void) {
  initSNTP();
  // wait for time to be set
  time_t now = 0;
  struct tm timeinfo = {0};
  int retry = 0;
  const int retry_count = 20;
  Serial.print("-->Waiting for system time to be set.");
  while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
    Serial.print(".");
    delay(10);
    time(&now);
    localtime_r(&now, &timeinfo);
  }
  Serial.println("ready.");
}

/************************** DISPLAY HANDLING **********************************/

void showWelcome(){
  isPowerOff=false;
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth()/2, display.getHeight()/2, "ESP32 POTP");
  display.display();
  Serial.println("-->Welcome screen ready");
  delay(500); // TODO: remove if bluetooth will be scan
}

void showTime(char timebuf[]) {
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth() / 2, ((display.getHeight()/ 2)-24),timebuf);
}

void showRevisionCode(){
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(display.getWidth()-5,display.getHeight()-10, VERSION_CODE+VCODE);
}

void showOTPCode(String OTPcode){
  display.setFont(ArialMT_Plain_24);
  display.drawString(display.getWidth() / 2, (display.getHeight()/ 2)+5, OTPcode);
}

void calcOTPCodeAndPrintScreen() {
  time_t now;
  struct tm timeinfo;
  char timebuf[64];
  char *newCode = totp.getCode(time(&now));
  localtime_r(&now, &timeinfo);
  strftime(timebuf, sizeof(timebuf),"%Y %a, %d %b %H:%M:%S", &timeinfo);
  if (strcmp(code, newCode) != 0) {
    strcpy(code, newCode);
    if (timeinfo.tm_year < (2016 - 1900)) {
      Serial.println("Time is not set yet! PRESS RIGHT BUTTON! (connect to NTP)");
    } else {
      Serial.print(timebuf);
      Serial.print(" ==> OTP code: [");
      Serial.print(code);
      Serial.println("]");
    }
  }
  display.clear();
  // show current time (clock)
  showTime(timebuf);
  // show OTP code
  showOTPCode(code);
  // show revision code
  showRevisionCode();
  // print display
  display.display();
}

/******************************************************************************
*****************************   S E T U P   ***********************************
******************************************************************************/

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("== ESP32 Booting ==");
  // display setup
  display.init();
  display.flipScreenVertically();
  display.setContrast(128);
  display.clear();
  Serial.println("-->OLED ready");
  wifi.disableWifi();
  // Init touch callbacks
  touchAttachInterrupt(T2, gotTouch2, THRESHOLD);
  touchAttachInterrupt(T3, gotTouch3, THRESHOLD);
  Serial.println("-->Buttons ready");
  // Set timezone for America/Bogota
  setenv("TZ", "<-05>5", 1);
  tzset();
  // Splash Window
  showWelcome();
  Serial.println("== Setup ready ==");
}

/******************************************************************************
*****************************    L O O P    ***********************************
******************************************************************************/

void loop() {
  if (wifi.isWifiEnable) // OTA mode
    ArduinoOTA.handle();
  else                   // OTPcode mode
    calcOTPCodeAndPrintScreen();

  // touch LEFT capture
  if (touch2detected) {
    touch2detected = false;
    if (touch2count++ > 10) {
      Serial.println("Touch 2 (GPIO2) reached");
      touch2count = 0;
      if (isPowerOff)
        showWelcome();
      else
        suspend();
    }
  }
  // touch RIGTH capture
  if (touch3detected) {
    touch3detected = false;
    if (touch3count++ > 5) {
      Serial.println("Touch 3 (GPIO15) reached");
      touch3count = 0;
      if (wifi.isWifiEnable) wifi.disableWifi();
      else if (wifi.init()) getTimeFromSNTP();
      delay(200);
    }
  }
}
