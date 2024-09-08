bool sendData() {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, "http://192.168.22.1/api/esp/data");
  http.addHeader("Content-Type", "application/json");
  
  StaticJsonDocument<250> uploadJSON;

  uploadJSON["id_session"] = sessionStation.id;
  uploadJSON["token"] = sessionStation.token;

  uploadJSON["timestamp"] = uploadData.timestamp;
  uploadJSON["voltage"] = uploadData.voltage;
  uploadJSON["current"] = uploadData.current;
  uploadJSON["power"] = uploadData.power;
  uploadJSON["frequency"] = uploadData.frequency;
  uploadJSON["power_factor"] = uploadData.power_factor;
  uploadJSON["temp"] = uploadData.temp;
  uploadJSON["humidity"] = uploadData.humidity;

  String uploadString;
  serializeJson(uploadJSON, uploadString);
  // unsigned long requestMillis = millis();
  uint8_t status = http.POST(uploadString);
  // unsigned long responseMillis = millis();
  String response = http.getString();
  http.end();

  // Serial.println("http://192.168.22.1/api/esp/data");
  // Serial.print("Request sent: ");
  // Serial.println(requestMillis);
  // Serial.print("Response received: ");
  // Serial.println(responseMillis);
  // Serial.print("Waiting time: ");
  // Serial.print(responseMillis - requestMillis);
  // Serial.println("ms");
  // Serial.print("Status Code: ");
  // Serial.println(status);
  // Serial.println("\n\n\n\n\n\n\n\n\n\n\n");

  if (status == 200)
    if (response.equals("OK")) return true;
  return false;
}

int getStation(String ip, String token) {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, "http://192.168.22.1/api/esp/station");
  http.addHeader("Content-Type", "application/json");

  // unsigned long requestMillis = millis();
  uint8_t status = http.POST("{\"ip\":\"" + ip + "\",\"token\":\"" + token + "\"}");
  // unsigned long responseMillis = millis();
  String response = http.getString();
  http.end();
  
  // Serial.println("\n\n\n\n\n\n\n");
  // Serial.println("http://192.168.22.1/api/esp/station");
  // Serial.print("Request sent: ");
  // Serial.println(requestMillis);
  // Serial.print("Response received: ");
  // Serial.println(responseMillis);
  // Serial.print("Waiting time: ");
  // Serial.print(responseMillis - requestMillis);
  // Serial.println("ms");
  // Serial.print("Status Code: ");
  // Serial.println(status);
  // Serial.println("\n\n\n\n\n\n\n\n\n\n\n");
  
  if (status == 200) return response.toInt();
  else return 0;
}

bool startSession() {
  dataPID.previousTime = 0;
  dataPID.lastError = 0;
  dataPID.lastIntegral = 0;

  if (!connection) {
    File f = SPIFFS.open("/sessionStation", "w");
    if (f) {
      f.write((const unsigned char*)&sessionStation, sizeof(sessionStation));
      f.close();
    }
    return true;
  }

  WiFiClient client;
  HTTPClient http;

  http.begin(client, "http://192.168.22.1/api/esp/start");
  http.addHeader("Content-Type", "application/json");
  StaticJsonDocument<200> uploadJSON;
  uploadJSON["id_station"] = stationConf.id;
  uploadJSON["token"] = stationConf.token;
  uploadJSON["start_at"] = sessionStation.start_at;
  uploadJSON["initial_mass"] = sessionStation.initial_mass;

  String uploadString;
  serializeJson(uploadJSON, uploadString);
  // unsigned long requestMillis = millis();
  uint8_t status = http.POST(uploadString);
  // unsigned long responseMillis = millis();
  String response = http.getString();
  http.end();

  // Serial.println("http://192.168.22.1/api/esp/start");
  // Serial.print("Request sent: ");
  // Serial.println(requestMillis);
  // Serial.print("Response received: ");
  // Serial.println(responseMillis);
  // Serial.print("Waiting time: ");
  // Serial.print(responseMillis - requestMillis);
  // Serial.println("ms");
  // Serial.print("Status Code: ");
  // Serial.println(status);
  // Serial.println("\n\n\n\n\n\n\n\n\n\n\n");

  if (status == 200) {
    int parsed = sscanf(response.c_str(), "%d,%9s", &sessionStation.id, sessionStation.token);
    if (sessionStation.id != 0) {
      File f = SPIFFS.open("/sessionStation", "w");
      if (f) {
        f.write((const unsigned char*)&sessionStation, sizeof(sessionStation));
        f.close();
      }
      return true;
    }
  }
  return false;
}

bool sendETA() {
  if (!connection) return true;

  WiFiClient client;
  HTTPClient http;

  http.begin(client, "http://192.168.22.1/api/esp/eta");
  http.addHeader("Content-Type", "application/json");
  StaticJsonDocument<200> uploadJSON;
  uploadJSON["id"] = sessionStation.id;
  uploadJSON["token"] = sessionStation.token;
  uploadJSON["eta"] = sessionStation.eta;

  String uploadString;
  serializeJson(uploadJSON, uploadString);
  // unsigned long requestMillis = millis();
  uint8_t status = http.POST(uploadString);
  // unsigned long responseMillis = millis();
  String response = http.getString();
  http.end();

  // Serial.println("http://192.168.22.1/api/esp/eta");
  // Serial.print("Request sent: ");
  // Serial.println(requestMillis);
  // Serial.print("Response received: ");
  // Serial.println(responseMillis);
  // Serial.print("Waiting time: ");
  // Serial.print(responseMillis - requestMillis);
  // Serial.println("ms");
  // Serial.print("Status Code: ");
  // Serial.println(status);
  // Serial.println("\n\n\n\n\n\n\n\n\n\n\n");

  if (status == 200) {
    if (response.equals("OK")) return true;
  }
  return false;
}

bool endSession() {
  SPIFFS.remove("/sessionStation");

  if (!connection) {
    sessionStation.id = 0;
    sessionStation.duration = 0;
    sessionStation.eta = 0;
    sessionStation.mode = 0;
    strcpy(sessionStation.token, "");
    strcpy(sessionStation.start_at, "");
    sessionStation.initial_mass = 0;
    strcpy(sessionStation.end_at, "");
    sessionStation.final_mass = 0;
    return true;
  }
  
  WiFiClient client;
  HTTPClient http;

  http.begin(client, "http://192.168.22.1/api/esp/end");
  http.addHeader("Content-Type", "application/json");
  StaticJsonDocument<200> uploadJSON;
  uploadJSON["id"] = sessionStation.id;
  uploadJSON["token"] = sessionStation.token;
  uploadJSON["end_at"] = sessionStation.end_at;
  uploadJSON["final_mass"] = sessionStation.final_mass;

  String uploadString;
  serializeJson(uploadJSON, uploadString);
  // unsigned long requestMillis = millis();
  uint8_t status = http.POST(uploadString);
  // unsigned long responseMillis = millis();
  String response = http.getString();
  http.end();

  // Serial.println("http://192.168.22.1/api/esp/end");
  // Serial.print("Request sent: ");
  // Serial.println(requestMillis);
  // Serial.print("Response received: ");
  // Serial.println(responseMillis);
  // Serial.print("Waiting time: ");
  // Serial.print(responseMillis - requestMillis);
  // Serial.println("ms");
  // Serial.print("Status Code: ");
  // Serial.println(status);
  // Serial.println("\n\n\n\n\n\n\n\n\n\n\n");

  if ((status == 200) && response.equals("OK")) {
    sessionStation.id = 0;
    sessionStation.duration = 0;
    sessionStation.eta = 0;
    sessionStation.mode = 0;
    strcpy(sessionStation.token, "");
    strcpy(sessionStation.start_at, "");
    sessionStation.initial_mass = 0;
    strcpy(sessionStation.end_at, "");
    sessionStation.final_mass = 0;
    return true;
  }
  return false;
}
