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

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // alias for `#include "SSD1306Wire.h"`
#include "WiFiManager.h"
#include "esp_deep_sleep.h"
#include "sha1.h"
#include "TOTP.h"
#include "lwip/err.h"
#include "apps/sntp/sntp.h"
#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("0d563a58-196a-48ce-ace2-dfec78acc814");

static BLEAddress *pServerAddress;
static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
}

// ymqw 2tcw 7ta3 wfeo fykk 7mys vhu2 drqj
//uint8_t hmacKey[] = {0x4d, 0x79, 0x4c, 0x65, 0x67, 0x6f, 0x44, 0x6f, 0x6f, 0x72};
uint8_t hmacKey[] = {0xea,0x41,0x68,0x5c,0x9b,0x10,0x13,0x5d,0x8c,0xa0,0x35,0x05,0x38,0xcb,0xa9,0x96,0x75,0xa0,0x5a,0xaf};

TOTP totp = TOTP(hmacKey, 20);
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
  display.drawString(display.getWidth()/2, display.getHeight()/2, "ESP32 POTP");
  display.display();
  delay(1000);
}

void suspend(){
  Serial.println("Process Suspend..");
  isPowerOff=true;
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth()/2, display.getHeight()/2, "Suspending..");
  display.display();
  delay(3000);
  display.clear();
  display.display();
  esp_deep_sleep_enable_touchpad_wakeup();
  esp_deep_sleep_start();
}

static void initialize_sntp(void) {
  Serial.println("Initializing SNTP");
  configTime(0,0,"pool.ntp.org");
}

static void obtain_time(void) {
  initialize_sntp();
  // wait for time to be set
  time_t now = 0;
  struct tm timeinfo = {0};
  int retry = 0;
  const int retry_count = 10;
  Serial.print("Waiting for system time to be set.");
  while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
    Serial.print(".");
    delay(100);
    time(&now);
    localtime_r(&now, &timeinfo);
  }
  Serial.println("ready.");
}

static void printTime() {
  time_t now = 0;
  struct tm timeinfo;
  time(&now);
  char strftime_buf[64];
  // Set timezone for America/Bogota
  setenv("TZ", "<-05>5", 1);
  tzset();
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  //Serial.println(strftime_buf);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth() / 2, ((display.getHeight()/ 2)-24), strftime_buf);
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

void showTOTPCode() {
  time_t now;
  struct tm timeinfo;
  char *newCode = totp.getCode(time(&now));
  localtime_r(&now, &timeinfo);
  if (strcmp(code, newCode) != 0) {
    strcpy(code, newCode);
    if (timeinfo.tm_year < (2016 - 1900)) {
      Serial.println(
          "Time is not set yet. Connecting to WiFi and getting time over NTP.");
    } else {
      Serial.print(timeinfo.tm_year);
      Serial.print(" [");
      Serial.print(code);
      Serial.println("]");
    }
  }
  display.clear();
  printTime();
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth() / 2, display.getHeight()/ 2, code);
  display.display();
}

void loop() {

  if (wifi.isWifiEnable)
    ArduinoOTA.handle();
  else
    showTOTPCode();

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

  if (touch3detected) {
    touch3detected = false;
    if (touch3count++ > 5) {
      Serial.println("Touch 3 (GPIO15) reached");
      touch3count = 0;
      if (wifi.isWifiEnable) wifi.disableWifi();
      else if (wifi.init()) obtain_time();
    }
  }

  //if(!wifi.isWifiEnable)showTOTPCode();

  //delay(10); // fix for OTA upload freeze

  /*
  if(poweroff++>1000000){
    Serial.println("Auto Power Off");
    poweroff=0;
    suspend();
  }
  */
}
