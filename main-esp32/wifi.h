bool setupWiFi() {
  bool scan = true, foundNetwork = false;
  while (scan) {
    drawInfoLoading("Scanning", "Wi-Fi Networks", 1500);
    int n = WiFi.scanNetworks();

    for (uint8_t i = 0; i < n; ++i) {
      if (WiFi.SSID(i).equals(WIFI_SSID)) {
        foundNetwork = true;
        drawInfo("Network Found!", String("SSID : ") + WIFI_SSID, 2000);
        break;
      }
    }

    if (!foundNetwork) {
      tft.fillScreen(TFT_BLACK);
      tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 - 80/2 - 25, 260, 80, 5, TFT_WHITE);
      tft.drawString("No Wi-Fi", tft.width()/2, tft.height()/2 - 15 - 25);
      tft.drawString("Network Found!", tft.width()/2, tft.height()/2 + 15 - 25);
      tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, TFT_RED);
      tft.fillRoundRect(tft.width()/2 + 260/2 - 125, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, 0x05E0);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(1);
      uint8_t pos_xn = tft.width()/2 - 260/2 + 62;
      uint8_t pos_yn = tft.height()/2 + 80/2 + 10 - 25 + 20;
      tft.drawString("DISCARD", pos_xn, pos_yn);
      uint8_t pos_xp = tft.width()/2 + 260/2 - 125 + 62;
      uint8_t pos_yp = tft.height()/2 + 80/2 + 10 - 25 + 20;
      tft.drawString("RESCAN", pos_xp, pos_yp);

      while (true) {
        uint16_t x, y;

        if (tft.getTouch(&x, &y)) {
          if ((x >= (pos_xn - 62)) && (x <= (pos_xn + 62)) && (y >= (pos_yn - 20)) && (y <= (pos_yn + 20))) {
            scan = false;
            break;
          }
          if ((x >= (pos_xp - 62)) && (x <= (pos_xp + 62)) && (y >= (pos_yp - 20)) && (y <= (pos_yp + 20))) {
            break;
          }
        }
      }

      if (!scan) {
        tft.fillScreen(TFT_BLACK);
        tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 - 80/2 - 25, 260, 80, 5, TFT_WHITE);
        tft.setTextColor(0x4208);
        tft.setTextSize(2);
        tft.drawString("The Device Can't", tft.width()/2, tft.height()/2 - 15 - 25);
        tft.drawString("Work Properly!", tft.width()/2, tft.height()/2 + 15 - 25);
        tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, 0x8410);
        tft.fillRoundRect(tft.width()/2 + 260/2 - 125, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, 0x05E0);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        tft.drawString("WAIT 5S", pos_xn, pos_yn);
        tft.drawString("RESCAN", pos_xp, pos_yp);
        unsigned long lastMillis = millis();
        uint8_t count = 5;

        while (true) {
          uint16_t x, y;
          if ((millis() - lastMillis > 1000) && (count > 0)) {
            lastMillis = millis();
            count--;
            if (count > 0) {
              tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, 0x8410);
              tft.drawString("WAIT " + String(count) + "S" , pos_xn, pos_yn);
            } else {
              tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, TFT_RED);
              tft.drawString("CONTINUE", pos_xn, pos_yn);
            }
          }

          if (tft.getTouch(&x, &y)) {
            if ((x >= (pos_xn - 62)) && (x <= (pos_xn + 62)) && (y >= (pos_yn - 20)) && (y <= (pos_yn + 20)) && (count == 0)) {
              connection = false;
              break;
            }
            if ((x >= (pos_xp - 62)) && (x <= (pos_xp + 62)) && (y >= (pos_yp - 20)) && (y <= (pos_yp + 20))) {
              scan = true;
              break;
            }
          }
        }
      }
    } else {
      break;
    }
  }
  return foundNetwork;
}

void connectWiFi() {
  IPAddress subnet(255, 255, 255, 0);
  IPAddress gateway(192, 168, 22, 1);
  IPAddress primaryDNS(192, 168, 22, 1);
  IPAddress secondaryDNS(8, 8, 8, 8);

  if (SPIFFS.exists("/stationConf")) {
    File fr = SPIFFS.open("/stationConf", "r");
    if (fr) {
      fr.readBytes((char*)&stationConf, sizeof(stationConf));
      fr.close();
      if (WiFi.config(stationConf.localIP, gateway, subnet, primaryDNS, secondaryDNS)) {
        WiFi.setHostname(stationConf.hostname);
      }
    } 
  }
  
  uint16_t font_height = drawInfoLoading("Connecting to", WIFI_SSID + String(" Network"), 0);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long blinkMillis, statusMillis, reconnectMillis = millis();
  while (true) {
    if (WiFi.status() != WL_CONNECTED) {
      statusMillis = millis();
      if (millis() - reconnectMillis > 20000) {
        reconnectMillis = millis();
        WiFi.disconnect();
        WiFi.reconnect();
      }
    }
    if (millis() - statusMillis > 1500) break;
    if (millis() - blinkMillis > 200) {
      blinkMillis = millis();
      tft.fillCircle(tft.width()/2, tft.height()/2 + font_height, font_height/2.5, random(0xFFFF));
    }
  }

  drawInfo("Successfully Connected", String("to ") + WIFI_SSID, 2000);
}

void configWiFi() {
  uint8_t fourth;
  sscanf(WiFi.localIP().toString().c_str(), "192.168.22.%d", &fourth);
  if (fourth < 200) {
    bool confignow = true;
    tft.fillScreen(TFT_BLACK);
    tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 - 80/2 - 25, 260, 80, 5, TFT_WHITE);
    tft.drawString("The Device Is Not", tft.width()/2, tft.height()/2 - 15 - 25);
    tft.drawString("Configured Yet", tft.width()/2, tft.height()/2 + 15 - 25);
    tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, TFT_RED);
    tft.fillRoundRect(tft.width()/2 + 260/2 - 125, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, 0x05E0);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    uint8_t pos_xn = tft.width()/2 - 260/2 + 62;
    uint8_t pos_yn = tft.height()/2 + 80/2 + 10 - 25 + 20;
    tft.drawString("LATER", pos_xn, pos_yn);
    uint8_t pos_xp = tft.width()/2 + 260/2 - 125 + 62;
    uint8_t pos_yp = tft.height()/2 + 80/2 + 10 - 25 + 20;
    tft.drawString("CONFIG NOW", pos_xp, pos_yp);

    while (true) {
      uint16_t x, y;

      if (tft.getTouch(&x, &y)) {
        if ((x >= (pos_xn - 62)) && (x <= (pos_xn + 62)) && (y >= (pos_yn - 20)) && (y <= (pos_yn + 20))) {
          confignow = false;
          break;
        }
        if ((x >= (pos_xp - 62)) && (x <= (pos_xp + 62)) && (y >= (pos_yp - 20)) && (y <= (pos_yp + 20))) {
          break;
        }
      }
    }

    if (!confignow) {
      tft.fillScreen(TFT_BLACK);
      tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 - 80/2 - 25, 260, 80, 5, TFT_WHITE);
      tft.setTextColor(0x4208);
      tft.setTextSize(2);
      tft.drawString("The Device Can't", tft.width()/2, tft.height()/2 - 15 - 25);
      tft.drawString("Work Properly!", tft.width()/2, tft.height()/2 + 15 - 25);
      tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, 0x8410);
      tft.fillRoundRect(tft.width()/2 + 260/2 - 125, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, 0x05E0);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(1);
      tft.drawString("WAIT 5S", pos_xn, pos_yn);
      tft.drawString("CONFIG NOW", pos_xp, pos_yp);
      unsigned long lastMillis = millis();
      uint8_t count = 5;

      while (true) {
        uint16_t x, y;
        if ((millis() - lastMillis > 1000) && (count > 0)) {
          lastMillis = millis();
          count--;
          if (count > 0) {
            tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, 0x8410);
            tft.drawString("WAIT " + String(count) + "S" , pos_xn, pos_yn);
          } else {
            tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, TFT_RED);
            tft.drawString("CONTINUE", pos_xn, pos_yn);
          }
        }

        if (tft.getTouch(&x, &y)) {
          if ((x >= (pos_xn - 62)) && (x <= (pos_xn + 62)) && (y >= (pos_yn - 20)) && (y <= (pos_yn + 20)) && (count == 0)) {
            connection = false;
            break;
          }
          if ((x >= (pos_xp - 62)) && (x <= (pos_xp + 62)) && (y >= (pos_yp - 20)) && (y <= (pos_yp + 20))) {
            confignow = true;
            break;
          }
        }
      }
    }

    if (confignow) {
      tft.fillScreen(TFT_WHITE);
      String ipmac = WiFi.localIP().toString().substring(8) + "," + WiFi.macAddress();
      qrcode.create(encryptAES(ipmac), tft.width()/2, tft.height()/2 - 17, 4);
      tft.setTextColor(0x4208);
      tft.setTextFont(4);
      tft.setTextSize(1);
      tft.setTextDatum(BC_DATUM);
      tft.drawString("Scan The QR Code", tft.width()/2, tft.height() - 7);

      while (true) {
      }
    } else {
      WiFi.disconnect();
      server.reset();
    }
  } else {
    if (stationConf.id == 0) {
      drawInfoLoading("Currently Obtaining", "The Configuration", 2000);

      stationConf.id = getStation(stationConf.localIP.toString(), String(stationConf.token));

      drawInfoLoading("Currently Saving", "The Configuration", 2000);

      File f = SPIFFS.open("/stationConf", "w");
      if (f) {
        f.write((const unsigned char*)&stationConf, sizeof(stationConf));
        f.close();
      }

      drawInfo("The Configuration", "Successfully Saved", 2000);
    } else {
      int response_id = getStation(stationConf.localIP.toString(), String(stationConf.token));

      if (response_id < 0) {
        drawInfo("This Station", "Has Been Deleted!", 2000);

        SPIFFS.remove("/stationConf");

        drawInfoLoading("Currently Rebooting", "The Station", 2000, false);
        ESP.restart();
      }
    }
  }
}
