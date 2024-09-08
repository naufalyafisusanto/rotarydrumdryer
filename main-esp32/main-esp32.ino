#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <IPAddress.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include <TFT_eSPI_QRCode.h>
#include <ESP32Time.h>
#include <SoftwareSerial.h>
#include <RBDdimmer.h>
#include <PZEM004Tv30.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>

#include "Adafruit_HTU21DF.h"
#include "AESLib.h"
#include "arduino_base64.hpp"
#include "fonts.h"

#define pinSerialNanoRX         25
#define pinSerialNanoTX         13

#define pinPZEMRX               16 
#define pinPXEMTX               17

#define pinOutputDimmer         32
#define pinZeroCrossDimmer      33

#define pinRelay1               26
#define pinRelay2               27

#define pinFan                  12

#define pinDS                   14

#define Digits      Leelawadee_20
#define Title       Poppins_Light_20
#define TFT_GREY    0x5AEB

#define FS_NO_GLOBALS

#define WIFI_SSID       "Piper Nigrum"
#define WIFI_PASSWORD   "bismillah"

#include "variable.h"
#include "struct.h"
#include "object.h"
#include "global.h"
#include "http.h"
#include "data.h"
#include "view.h"
#include "touch.h"
#include "wifi.h"
#include "fuzzy.h"
#include "control.h"
#include <AsyncElegantOTA.h>

void startServer() {
  server.serveStatic("/spiffs/", SPIFFS, "/");
  AsyncElegantOTA.begin(&server, stationConf.token);
  server.begin();
}

void setup() {
  Serial.begin(9600);
  SerialNano.begin(9600);

  pinMode(pinRelay1, OUTPUT);
  pinMode(pinRelay2, OUTPUT);
  digitalWrite(pinRelay1, HIGH);
  digitalWrite(pinRelay2, HIGH);

  dimmer.begin(NORMAL_MODE, ON);
  dimmer.setPower(5);

  pinMode(pinFan, OUTPUT);
  analogWrite(pinFan, 30);

  SPIFFS.begin();
  sensors.begin();
  htu.begin();
  
  #include "server.h"

  tft.init();
  tft.setRotation(1);

  drawLogo();
  calibrateTouch();
  if (setupWiFi()) {
    connectWiFi();

    startServer();

    timeClient.begin();
    timeClient.setTimeOffset(0);
    getDatetimeNow();

    configWiFi();
  }

  // connectWiFi();
  // startServer();
  // timeClient.begin();
  // timeClient.setTimeOffset(0);
  // getDatetimeNow();
  // Skip

  loadSimulation();
  loadParams();
  loadPreviousSession();

  tft.fillScreen(TFT_GREY);
  drawNavbar();
  drawTHVC();
  
  xTFTSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xTFTSemaphore);
  #include "rtos.h"
}

void loop() {
  // Kalo diisi berarti idiot!
}