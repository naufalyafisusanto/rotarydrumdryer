bool isNaNorZero(float value) {
  return (isnan(value) || value == 0.00);
}

bool isNaNorZero(int value) {
  return (value == 0);
}

bool convertStringtoBool(String str) {
  if (str.equalsIgnoreCase("true") || str.equals("1")) {
    return true;
  } else if (str.equalsIgnoreCase("false") || str.equals("0")) {
    return false;
  } else {
    return false;
  }
}

String encryptAES(String inputText) { 
  int bytesInputLength = inputText.length() + 1;
  byte bytesInput[bytesInputLength];
  inputText.getBytes(bytesInput, bytesInputLength);

  int outputLength = aesLib.get_cipher_length(bytesInputLength);
  byte bytesEncrypted[outputLength];

  // Cipher Mode and Key Size are preset in AESLib
  // Cipher Mode = CBC
  // Key & IV Size = 128 bits / 16 bytes
  // Padding Mode = CMS

  String aesKeyStr = "rliQbrzKBOE53mK3";
  String aesIvStr = "Vu648RUf7fr2PYxB";
  byte aesKeyBuf[17], aesIvBuf[17], aesKey[16], aesIv[16];
  aesKeyStr.getBytes(aesKeyBuf, 17);
  aesIvStr.getBytes(aesIvBuf, 17);
  for (uint8_t i = 0; i < 16; i++) {
    aesKey[i] = aesKeyBuf[i];
    aesIv[i] = aesIvBuf[i];
  }

  aesLib.set_paddingmode(paddingMode::CMS);
  aesLib.encrypt(bytesInput, bytesInputLength, bytesEncrypted, aesKey, 16, aesIv);
  char base64EncodedOutput[base64::encodeLength(outputLength)];
  base64::encode(bytesEncrypted, outputLength, base64EncodedOutput);

  return String(base64EncodedOutput);
}

String getUptime() {
  unsigned long uptimeInSeconds = millis() / 1000;
  uint8_t hours = uptimeInSeconds / 3600;
  uint8_t minutes = (uptimeInSeconds % 3600) / 60;
  uint8_t seconds = (uptimeInSeconds % 3600) % 60;

  char buffer[9];
  sprintf(buffer, "%02u:%02u:%02u", hours, minutes, seconds);

  return String(buffer);
}

String getDatetimeNow() {
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    unsigned long rtc_epoch = rtc.getEpoch();
    unsigned long ntp_epoch = timeClient.getEpochTime();

    uint8_t timeDiff;
    if (rtc_epoch > ntp_epoch) timeDiff = rtc_epoch - ntp_epoch;
    else timeDiff = ntp_epoch - rtc_epoch;

    if (timeDiff > 5) rtc.setTime(ntp_epoch);
  }

  // if (x == 1) return String(rtc.getEpoch());
  return rtc.getTime("%Y-%m-%d %H:%M:%S");
}