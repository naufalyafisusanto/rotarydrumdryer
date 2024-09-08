void calibrateTouch() {
  uint16_t calData[5];
  bool calDataOK = false;

  if (SPIFFS.exists("/touchConf")) {
    File f = SPIFFS.open("/touchConf", "r");
    if (f) {
      if (f.readBytes((char *)calData, 14) == 14)
        calDataOK = true;
      f.close();
    }
  }

  if (calDataOK) {
    tft.setTouch(calData);
  } else {
    drawInfo("Touch Corners", "as Indicated!", 0);

    tft.calibrateTouch(calData, TFT_RED, TFT_BLACK, 15);

    drawInfoLoading("Currently Saving", "Calibration File", 1500);

    File f = SPIFFS.open("/touchConf", "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }

    drawInfo("Calibration File", "Successfully Saved", 2000);
  }
}
