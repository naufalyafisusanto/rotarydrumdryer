void drawInfo(String text_top, String text_bottom, unsigned int delayMillis = 2000, bool clear_bg = true, uint8_t padding_x = 20, uint8_t padding_y = 15, float line_spacing = 1) {
  tft.setTextColor(0x4208);
  tft.setTextFont(2);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  uint16_t width = max(tft.textWidth(text_top), tft.textWidth(text_bottom)) + 2*padding_x;
  uint16_t font_height = tft.fontHeight();
  uint16_t height = 2*font_height + (line_spacing-1)*font_height + 2*padding_y;
  if (width >= 320) width = 310;

  if (clear_bg) tft.fillScreen(TFT_BLACK);
  else tft.fillRoundRect((tft.width() - width - 4)/2, (tft.height() - height - 4)/2, width + 4, height + 4, 5, 0x4208);
  tft.fillRoundRect((tft.width() - width)/2, (tft.height() - height)/2, width, height, 5, TFT_WHITE);

  tft.drawString(text_top, tft.width()/2, tft.height()/2 - line_spacing*font_height/2);
  tft.drawString(text_bottom, tft.width()/2, tft.height()/2 + line_spacing*font_height/2);

  if (!clear_bg) {
    overflow = true;
    redraw_bg = true;
  }
  if (delayMillis > 0) {
    unsigned long displayMillis = millis();
    while (true) {
      if (millis() - displayMillis > delayMillis) {
        overflow = false;
        break;
      }
    }
  }
}

uint16_t drawInfoLoading(String text_top, String text_bottom, unsigned int delayMillis = 2000, bool clear_bg = true, uint8_t padding_x = 20, uint8_t padding_y = 15, float line_spacing = 1) {
  tft.setTextColor(0x4208);
  tft.setTextFont(2);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  uint16_t width = max(tft.textWidth(text_top), tft.textWidth(text_bottom)) + 2*padding_x;
  uint16_t font_height = tft.fontHeight();
  uint16_t height = 3*font_height + (line_spacing-1)*2*font_height + 2*padding_y;
  if (width >= 320) width = 310;

  if (clear_bg) tft.fillScreen(TFT_BLACK);
  else tft.fillRoundRect((tft.width() - width - 4)/2, (tft.height() - height - 4)/2, width + 4, height + 4, 5, 0x4208);
  tft.fillRoundRect((tft.width() - width)/2, (tft.height() - height)/2, width, height, 5, TFT_WHITE);

  tft.drawString(text_top, tft.width()/2, tft.height()/2 - line_spacing*font_height);
  tft.drawString(text_bottom, tft.width()/2, tft.height()/2);

  if (!clear_bg) {
    overflow = true;
    redraw_bg = true;
  }
  if (delayMillis > 0) {
    unsigned long blinkMillis, displayMillis = millis();
    while (true) {
      if (millis() - displayMillis > delayMillis) {
        overflow = false;
        break;
      }
      if (millis() - blinkMillis > 200) {
        blinkMillis = millis();
        tft.fillCircle(tft.width()/2, tft.height()/2 + line_spacing*font_height, font_height/2.5, random(0xFFFF));
      }
    }
    return 0;
  } else {
    return font_height;
  }
}

void drawNavbar() {
  timeSprite.createSprite(320, 20);

  uint16_t top = TFT_WHITE, bottom = TFT_WHITE;

  if (WiFi.status() == WL_CONNECTED) {
    if ((running && !running_start) || running_stop) timeSprite.fillSprite(TFT_GREEN);
    else {
      if (rotation == 0) timeSprite.fillSprite(TFT_RED);
      else if (rotation == 1) timeSprite.fillRectVGradient(0, 0, 320, 20, TFT_RED, TFT_MAGENTA);
      else if (rotation == -1) timeSprite.fillRectVGradient(0, 0, 320, 20, TFT_RED, TFT_ORANGE);
    }
  } else timeSprite.fillSprite(TFT_WHITE);
  timeSprite.setTextColor(0x4208);
  timeSprite.loadFont(Digits);
  timeSprite.setTextDatum(TL_DATUM);
  timeSprite.drawString(getDatetimeNow(), 2, 2);
  timeSprite.setTextDatum(TR_DATUM);
  if (WiFi.status() == WL_CONNECTED) timeSprite.drawString(String(WiFi.RSSI()) + " dBm", tft.width() - 2, 2);
  else timeSprite.drawString("None", tft.width() - 2, 2);
  timeSprite.unloadFont();
  timeSprite.pushSprite(0, 0);

  timeSprite.deleteSprite();
}

void drawTHVC(bool drawbg = false, uint8_t padding = 12, uint8_t num_x = 2, uint8_t num_y = 2, uint8_t off_x = 0, uint8_t off_y = 20) {  
  uint16_t width = (320 - off_x - (num_x + 1)*padding)/num_x;
  uint16_t height = (240 - off_y - (num_y + 1)*padding)/num_y;

  dataSprite.createSprite(width, height);
  if (redraw_bg) {
    tft.fillRect(0, 20, 320, 220, TFT_GREY);
    redraw_bg = false;
  }
  if (drawbg) tft.fillRect(0, 20, 320, 220, TFT_GREY);

  char charTemp[6], charVoltage[6], charCurrent[6], charHumidity[6];
  dtostrf(uploadData.temp, 3, 1, charTemp);
  dtostrf(uploadData.humidity, 3, 1, charHumidity);
  dtostrf(uploadData.voltage, 3, 0, charVoltage);
  dtostrf(uploadData.current, 3, 2, charCurrent);
  
  dataSprite.fillScreen(TFT_GREY);
  dataSprite.fillRoundRect(0, 0, width, height, 7, TFT_WHITE);
  dataSprite.setTextDatum(TC_DATUM);
  dataSprite.setTextColor(TFT_BLACK);
  dataSprite.loadFont(Title);
  dataSprite.drawString("Temp (°C)", width/2, 10);
  dataSprite.unloadFont();
  dataSprite.setTextDatum(BC_DATUM);
  dataSprite.setTextFont(7);
  dataSprite.setTextSize(1);
  dataSprite.drawString(String(charTemp), width/2, height - 7);
  dataSprite.pushSprite(off_x + padding, off_y + padding);

  dataSprite.fillScreen(TFT_GREY);
  dataSprite.fillRoundRect(0, 0, width, height, 7, TFT_WHITE);
  dataSprite.setTextDatum(TC_DATUM);
  dataSprite.setTextColor(TFT_BLACK);
  dataSprite.loadFont(Title);
  dataSprite.drawString("Humidity (%)", width/2, 10);
  dataSprite.unloadFont();
  dataSprite.setTextDatum(BC_DATUM);
  dataSprite.setTextFont(7);
  dataSprite.setTextSize(1);
  dataSprite.drawString(String(charHumidity), width/2, height - 7);
  dataSprite.pushSprite(off_x + padding*num_x + width, off_y + padding);

  dataSprite.fillScreen(TFT_GREY);
  dataSprite.fillRoundRect(0, 0, width, height, 7, TFT_WHITE);
  dataSprite.setTextDatum(TC_DATUM);
  dataSprite.setTextColor(TFT_BLACK);
  dataSprite.loadFont(Title);
  dataSprite.drawString("Voltage (V)", width/2, 10);
  dataSprite.unloadFont();
  dataSprite.setTextDatum(BC_DATUM);
  dataSprite.setTextFont(7);
  dataSprite.setTextSize(1);
  dataSprite.drawString(String(charVoltage), width/2, height - 7);
  dataSprite.pushSprite(off_x + padding, off_y + padding*num_y + height);

  dataSprite.fillScreen(TFT_GREY);
  dataSprite.fillRoundRect(0, 0, width, height, 7, TFT_WHITE);
  dataSprite.setTextDatum(TC_DATUM);
  dataSprite.setTextColor(TFT_BLACK);
  dataSprite.loadFont(Title);
  dataSprite.drawString("Current (A)", width/2, 10);
  dataSprite.unloadFont();
  dataSprite.setTextDatum(BC_DATUM);
  dataSprite.setTextFont(7);
  dataSprite.setTextSize(1);
  dataSprite.drawString(String(charCurrent), width/2, height - 7);
  dataSprite.pushSprite(off_x + padding*num_x + width, off_y + padding*num_y + height);

  dataSprite.deleteSprite();
}

uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

void drawBMP(const char *filename, int16_t x, int16_t y) {
  if ((x >= tft.width()) || (y >= tft.height())) return;
  fs::File bmpFS;
  bmpFS = SPIFFS.open(filename, "r");

  if (!bmpFS) {
    // Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42) {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0)) {
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++) {
        
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++) {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
      // Serial.print("Loaded in ");
      // Serial.print(millis() - startTime);
      // Serial.println(" ms");
    }
    // else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

void drawLogo() {
  tft.fillScreen(TFT_WHITE);
  drawBMP("/logo.bmp", (tft.width()  - 200)/2, (tft.height() - 200)/2 - 35);
  tft.setTextColor(0x4208);
  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Piper Nigrum", tft.width()/2, tft.height()/2 + 65);
  tft.drawString("Universitas Diponegoro", tft.width()/2, tft.height()/2 + 95);
  delay(2000);

  drawBMP("/innovillage.bmp", 0, 0);
  delay(2000);
}

void TaskNavbar(void *pvParameters) {
  while(true) {
    // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (xSemaphoreTake(xTFTSemaphore, portMAX_DELAY)) {
      drawNavbar();
      // Serial.println("Navbar:" + String(uxTaskGetStackHighWaterMark(NULL)));

      xSemaphoreGive(xTFTSemaphore);
      vTaskDelay(pdMS_TO_TICKS(2000));
    }
  }
}

void TaskDisplay(void *pvParameters) {
  while (true) {
    if (xSemaphoreTake(xTFTSemaphore, portMAX_DELAY)) {
      if (state_display == 0) {
        updateData();
        if (!overflow) drawTHVC();
        if (connection && running && !running_start && (millis() - startMillis > 3000)) sendData();
      } else {
        tft.fillRect(0, 20, 320, 220, TFT_GREY);
      }
      
      Serial.println("Display:" + String(uxTaskGetStackHighWaterMark(NULL)));

      xSemaphoreGive(xTFTSemaphore);
      // vTaskDelay(pdMS_TO_TICKS(5000));
      ulTaskNotifyTake(pdTRUE, 5000);
    }
  }
}

void TaskTouchScreen(void *pvParameters) {
  while (true) {
    if (xSemaphoreTake(xTFTSemaphore, portMAX_DELAY)) {
      uint16_t x, y;
      if (tft.getTouch(&x, &y)) {
        if ((x >= 0) && (x <= 77) && (y >= 140) && (y <= 240)) {
          Serial.println("Kiri");
          if (state_display > -1) state_display -= 1;
          Serial.println(state_display);
          xTaskNotifyGive(TaskHandleDisplay);
        }
        if ((x >= 243) && (x <= 320) && (y >= 140) && (y <= 240)) {
          Serial.println("Kanan");
          if (state_display < 2) state_display += 1;
          Serial.println(state_display);
          xTaskNotifyGive(TaskHandleDisplay);
        }
      }

      Serial.println("Touch:" + String(uxTaskGetStackHighWaterMark(NULL)));
      
      xSemaphoreGive(xTFTSemaphore);
      vTaskDelay(pdMS_TO_TICKS(250));
    }
  }
}