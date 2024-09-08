server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  String response = "";
  if (simulation) response += "[---SIMULATION MODE---]\n";
  response += "Name       : " + String(WiFi.getHostname()) + "\n";
  response += "Station ID : " + String(stationConf.id) + "\n";
  response += "ESP32 ID   : " + String((uint32_t)ESP.getEfuseMac(), HEX) + "\n";
  response += "Uptime     : " + getUptime() + "\n";
  String status = "";
  if (running) status = "Running";
  else {
    if (rotation == 0) status = "Stopped (Brake)";
    else if (rotation == 1) status = "Stopped (Insert)";
    else if (rotation == -1) status = "Stopped (Eject)";
  }
  response += "Status     : " + status;
  request->send(200, "text/plain", response);
});

server.on("/running", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", String(running ? "1" : "0"));
});

server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
  StaticJsonDocument<100> doc_status;
  doc_status["running"] = running;
  doc_status["rotation"] = rotation;

  String json_status;
  serializeJson(doc_status, json_status);
  request->send(200, "application/json", json_status);
});

server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request){ 
  if (request->hasParam("ip", true) && \
      request->hasParam("hostname", true) && \
      request->hasParam("token", true)) {
    String token = request->getParam("token", true)->value();
    if (token.equals(stationConf.token) || String("").equals(stationConf.token)) {
      structConf saveConf;
      String ip, hostname;
      ip = request->getParam("ip", true)->value();
      hostname = request->getParam("hostname", true)->value();
      
      uint8_t first = 0, second = 0, third = 0, fourth = 0;
      sscanf(ip.c_str(), "%d.%d.%d.%d", &first, &second, &third, &fourth);

      if ((first != 192) || (second != 168) || (third != 22) || (fourth == 0)) {
        request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"IP is Invalid!\"}");
      } else {
        saveConf.localIP = IPAddress(first, second, third, fourth);
        strcpy(saveConf.hostname, hostname.c_str());
        strcpy(saveConf.token, token.c_str());

        File f = SPIFFS.open("/stationConf", "w");
        if (f) {
          f.write((const unsigned char*)&saveConf, sizeof(saveConf));
          f.close();
        }

        request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Accepted\"}");

        drawInfo("The Configuration", "Successfully Saved", 3000, false);
        
        drawInfoLoading("Currently Rebooting", "The Station", 2000, false);
      
        ESP.restart();
      }
    } else {
      request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Invalid Token!\"}");
    }
  } else {
    request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Incomplete Parameters\"}");
  }      
});

server.on("/action", HTTP_POST, [](AsyncWebServerRequest *request){
  if (request->hasParam("action", true) && request->hasParam("token", true)) {
    String token = request->getParam("token", true)->value();
    if (token.equals(stationConf.token)) {
      String action = request->getParam("action", true)->value();
      if (action.equals("tare")) {
        if (rotation == 0 && !running) {
          sendDataNano(4);
          request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Tare\",\"running\":\"0\",\"rotation\":\"0\"}");
        } else {
          request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Failed to tare. Brake or stop the station first!\"}");
        }
      } else if (action.equals("start_fast")) {
        if (rotation == 0) {
          sessionStation.mode = 1;
          running = true;
          request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Start (Fast)\",\"running\":\"1\",\"rotation\":\"0\"}");
        } else {
          request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Failed to start. Brake the station first!\"}");
        }
      } else if (action.equals("start_medium")) {
        if (rotation == 0) {
          sessionStation.mode = 2;
          running = true;
          request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Start (Medium)\",\"running\":\"1\",\"rotation\":\"0\"}");
        } else {
          request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Failed to start. Brake the station first!\"}");
        }
      } else if (action.equals("start_slow")) {
        if (rotation == 0) {
          sessionStation.mode = 3;
          running = true;
          request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Start (Slow)\",\"running\":\"1\",\"rotation\":\"0\"}");
        } else {
          request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Failed to start. Brake the station first!\"}");
        }
      } else if (action.equals("stop")) {
        if (rotation == 0) {
          running = false;
          request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Stop\",\"running\":\"0\",\"rotation\":\"0\"}");
        } else {
          request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Failed to stop. Brake the station first!\"}");
        }
      } else if (action.equals("insert")) {
        if (!running) {
          rotation = 1;
          sendDataNano(2);
          request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Insert\",\"running\":\"0\",\"rotation\":\"1\"}");
        } else {
          request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Failed to insert. Stop the station first!\"}");
        }        
      } else if (action.equals("brake")) {
        if (!running) {
          rotation = 0;
          sendDataNano(0);
          request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Brake\",\"running\":\"0\",\"rotation\":\"0\"}");
        } else {
          request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Failed to brake. Stop the station first!\"}");
        }        
      } else if (action.equals("eject")) {
        if (!running) {
          rotation = -1;
          sendDataNano(3);
          request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Eject\",\"running\":\"0\",\"rotation\":\"-1\"}");
        } else {
          request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Failed to eject. Stop the station first!\"}");
        }        
      } else {
        request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Invalid Action\"}");
      }      
    } else {
      request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Invalid Token\"}");
    }
  } else {
    request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Incomplete Parameters\"}");
  }
});

server.on("/params", HTTP_GET, [](AsyncWebServerRequest *request){
  StaticJsonDocument<200> doc_params;
  doc_params["speed-fan"] = stationParams.speed_fan;
  doc_params["max-heater-power"] = stationParams.max_heater_power;
  doc_params["set-temperature-fast"] = stationParams.set_temperature_fast;
  doc_params["set-temperature-medium"] = stationParams.set_temperature_medium;
  doc_params["set-temperature-low"] = stationParams.set_temperature_low;
  doc_params["pid-proportional"] = stationParams.pid_proportional;
  doc_params["pid-integral"] = stationParams.pid_integral;
  doc_params["pid-derivative"] = stationParams.pid_derivative;

  String json_params;
  serializeJson(doc_params, json_params);
  request->send(200, "application/json", json_params);
});

server.on("/params", HTTP_POST, [](AsyncWebServerRequest *request){
  if (request->hasParam("speed-fan", true) && \
      request->hasParam("max-heater-power", true) && \
      request->hasParam("set-temperature-fast", true) && \
      request->hasParam("set-temperature-medium", true) && \
      request->hasParam("set-temperature-low", true) && \
      request->hasParam("pid-proportional", true) && \
      request->hasParam("pid-integral", true) && \
      request->hasParam("pid-derivative", true) && \
      request->hasParam("token", true)) {
    String token = request->getParam("token", true)->value();
    if (token.equals(stationConf.token)) {
      stationParams.speed_fan = (uint16_t)request->getParam("speed-fan", true)->value().toInt();
      stationParams.max_heater_power = (uint16_t)request->getParam("max-heater-power", true)->value().toInt();
      stationParams.set_temperature_fast = request->getParam("set-temperature-fast", true)->value().toInt();
      stationParams.set_temperature_medium = request->getParam("set-temperature-medium", true)->value().toInt();
      stationParams.set_temperature_low = request->getParam("set-temperature-low", true)->value().toInt();
      stationParams.pid_proportional = request->getParam("pid-proportional", true)->value().toFloat();
      stationParams.pid_integral = request->getParam("pid-integral", true)->value().toFloat();
      stationParams.pid_derivative = request->getParam("pid-derivative", true)->value().toFloat();

      File f = SPIFFS.open("/stationParams", "w");
      if (f) {
        f.write((const unsigned char*)&stationParams, sizeof(stationParams));
        f.close();
      }

      drawInfo("The Parameters", "Successfully Saved", 3000, false);

      request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Accepted\"}");
    } else {
      request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Invalid Token!\"}");
    }
  } else {
    request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Incomplete Parameters!\"}");
  }
});

server.on("/delete", HTTP_POST, [](AsyncWebServerRequest *request){
  if (request->hasParam("token", true)) {
    String token = request->getParam("token", true)->value();
    if (token.equals(stationConf.token)) {
      SPIFFS.remove("/stationConf");

      request->send(200, "application/json", "{\"msg\":\"OK\",\"data\":\"Accepted\"}");
      
      drawInfo("The Configuration", "Successfully Deleted", 3000, false);
      
      drawInfoLoading("Currently Rebooting", "The Station", 2000, false);
    
      ESP.restart();     
    } else {
      request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Invalid Token\"}");
    }
  } else {
    request->send(200, "application/json", "{\"msg\":\"ERROR\",\"data\":\"Incomplete Parameters\"}");
  }
});

server.on("/simulation", HTTP_GET, [](AsyncWebServerRequest *request) {
  if (request->hasParam("set")) {
    String setValue = request->getParam("set")->value();
    if (setValue == "1") {
      simulation = true;
      File f = SPIFFS.open("/stationSimulation", "w");
      if (f) {
        f.write((const unsigned char*)&simulation, sizeof(simulation));
        f.close();
      }
      request->send(200, "text/plain", "OK " + String(simulation ? "1" : "0"));
    } else {
      simulation = false;
      File f = SPIFFS.open("/stationSimulation", "w");
      if (f) {
        f.write((const unsigned char*)&simulation, sizeof(simulation));
        f.close();
      }
      request->send(200, "text/plain", "OK " + String(simulation ? "1" : "0"));
    }
  } else request->send(200, "text/plain", String(simulation ? "1" : "0"));
});