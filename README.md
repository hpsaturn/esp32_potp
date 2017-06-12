# ESP32 WeMOS OLED

PlatformIO project for WeMOS like board with ESP32 and OLED SSD1306 display

## Framework and libraries dependencies

Before install [PlatformIO](http://platformio.org/) open source ecosystem for IoT development, then install libraries:

## Compile and firmware upload

#### Setup WiFi

Before, please setup your WiFi on `secrets.load.sample` and run:

``` bash
git clone https://github.com/hpsaturn/esp32_wemos_oled.git
cd esp32_wemos_oled
cp secrets.load.sample secrets.load
chmod 755 secrets.load
export PLATFORMIO_BUILD_FLAGS=`bash ./secrets.load`
```

#### Compile firmware and deploy

``` javascript
platformio run --target upload
``` 

#### OTA Updates (Optional)

For faster firmare updates via air (OTA)

``` javascript
platformio run --target upload --upload-port 192.168.1.107
``` 

## Basic Connection

![alt text][drv8825]

[drv8825]:https://github.com/hpsaturn/esp32_wemos_oled/blob/master/assets/diagram00.jpg  "General connection for WeMOS OLED"

## Board WeMOS like OLED:

![alt text][wemos_oled]

[wemos_oled]:https://github.com/hpsaturn/esp32_wemos_oled/blob/master/assets/wemos_oled_00.jpg  "WeMOS OLED like"





