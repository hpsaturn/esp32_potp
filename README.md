# ESP32 POTP (Portable Google Authenticator)

**[status: in developing]**

Portable EAP Protected One-Time Password (EAP-POTP) or GoogleAuthenticator hardware for provides two-factor user authentication. This is a
PlatformIO DIY project for WeMOS like board with ESP32 and OLED SSD1306 display, see materials and instructions below. <a href="https://github.com/hpsaturn/esp32_wemos_oled/blob/master/assets/esp32_potp_intro.jpg"><img src="https://github.com/hpsaturn/esp32_wemos_oled/blob/master/assets/esp32_potp_intro.jpg" align="right" height="512" width="512" ></a>


## Development Status: 

- [X] WifiManager
- [X] Wifi OTA Handler
- [X] DeepSleep mode (testing)
- [X] GUI overlay tests
- [X] RFC 4793 implementation
- [ ] Sync RTC via Bluetooth
- [ ] GUI Page viewer for OTP codes
- [ ] Android GoogleAuth POTP sync
- [ ] Vulnerability security test 
- [ ] 3D print case?
- [ ] Resina case?
- [ ] Latex case?

## DIY Building

## Dependencies

Please install before, [PlatformIO](http://platformio.org/) open source ecosystem for IoT development.

## Compile and firmware upload

#### Setup WiFi

Setup your WiFi on `secrets.load` and run:

``` bash
git clone https://github.com/hpsaturn/esp32_wemos_oled.git
cd esp32_wemos_oled
cp secrets.load.sample secrets.load
chmod 755 secrets.load
```

#### Compile firmware and deploy

``` javascript
platformio run --target upload
``` 

#### OTA Updates (Optional)

For faster firmware updates via air (OTA)

``` javascript
platformio run --target upload --upload-port 192.168.1.107
``` 

##### Library installation Troubleshooting

if you have the next error:

```C
src/Main.cpp:27:18: fatal error: sha1.h: No such file or directory
compilation terminated.
```
it is because the TOTP Arduino library is broken, please clean and remove hidden directories with `rm -rf .pioenvs .piolibdeps` and run `git pull origin master` again.

## Basic Connection

![alt text][diagram_pinout]

[diagram_pinout]:https://github.com/hpsaturn/esp32_wemos_oled/blob/master/assets/diagram00.jpg  "General connection for WeMOS OLED"

## Board WeMOS like OLED:

![alt text][wemos_oled]

[wemos_oled]:https://github.com/hpsaturn/esp32_wemos_oled/blob/master/assets/wemos_oled_00.jpg  "WeMOS OLED like"


