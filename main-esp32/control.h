void loadSimulation() {
  if (SPIFFS.exists("/stationSimulation")) {
    File fr = SPIFFS.open("/stationSimulation", "r");
    if (fr) {
      fr.readBytes((char*)&simulation, sizeof(simulation));
      fr.close();
    }
  }

  if (simulation) drawInfo("STATION IN", "SIMULATION MODE", 4000);
}

void loadParams() {
  bool loadSuccess = false;
  
  if (SPIFFS.exists("/stationParams")) {
    File fr = SPIFFS.open("/stationParams", "r");
    if (fr) {
      fr.readBytes((char*)&stationParams, sizeof(stationParams));
      fr.close();
      loadSuccess = true;
    }
  }

  if (loadSuccess) drawInfo("Successfully Loaded", "Params from Filesystem", 4000);
  else drawInfo("Successfully Loaded", "Params from Sketch", 4000);
}

void loadPreviousSession() {
  if (SPIFFS.exists("/sessionStation")) {
    File fr = SPIFFS.open("/sessionStation", "r");
    if (fr) {
      fr.readBytes((char*)&sessionStation, sizeof(sessionStation));
      fr.close();
    
      tft.fillScreen(TFT_BLACK);
      tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 - 80/2 - 25, 260, 80, 5, TFT_WHITE);
      tft.setTextColor(0x4208);
      tft.setTextSize(2);
      tft.drawString("Continue The Last", tft.width()/2, tft.height()/2 - 15 - 25);
      tft.drawString("Saved Session?", tft.width()/2, tft.height()/2 + 15 - 25);
      tft.fillRoundRect(tft.width()/2 - 260/2, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, TFT_RED);
      tft.fillRoundRect(tft.width()/2 + 260/2 - 125, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, 0x05E0);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(1);
      uint8_t pos_xn = tft.width()/2 - 260/2 + 62;
      uint8_t pos_yn = tft.height()/2 + 80/2 + 10 - 25 + 20;
      tft.drawString("NO", pos_xn, pos_yn);
      uint8_t pos_xp = tft.width()/2 + 260/2 - 125 + 62;
      uint8_t pos_yp = tft.height()/2 + 80/2 + 10 - 25 + 20;    
      tft.drawString("YES (7S)", pos_xp, pos_yp);

      unsigned long lastMillis = millis();
      uint8_t count = 7;

      while (true) {
        uint16_t x, y;
        if ((millis() - lastMillis > 1000) && (count > 0)) {
          lastMillis = millis();
          count--;
          if (count > 0) {
            tft.fillRoundRect(tft.width()/2 + 260/2 - 125, tft.height()/2 + 80/2 + 10 - 25, 125, 40, 5, 0x05E0);
            tft.drawString("YES (" + String(count) + "S)" , pos_xp, pos_yp);
          } else {
            continue_session = true;
            break;
          }
        }

        if (tft.getTouch(&x, &y)) {
          if ((x >= (pos_xn - 62)) && (x <= (pos_xn + 62)) && (y >= (pos_yn - 20)) && (y <= (pos_yn + 20))) {
            continue_session = false;
            break;
          }
          if ((x >= (pos_xp - 62)) && (x <= (pos_xp + 62)) && (y >= (pos_yp - 20)) && (y <= (pos_yp + 20))) {
            continue_session = true;
            break;
          }
        }
      }

      if (continue_session) {
        running = true;
      } else {
        running_stop = true;
        running_stop_setup = true;
      }
    }
  }
}

void TaskPID(void *pvParameters) {
  while (true) {
    float temp;
    if (simulation) {
      temp = random(500, 700) / 10.0;
      dataPID.temp = temp;
    } else {
      sensors.requestTemperatures();
      temp = sensors.getTempCByIndex(0);
      if (temp > 0) dataPID.temp = temp;
    }

    if (running && !running_start) {
      uint8_t setpoint = 0;
      if (sessionStation.mode == 1) setpoint = stationParams.set_temperature_fast;
      else if (sessionStation.mode == 2) setpoint = stationParams.set_temperature_medium;
      else if (sessionStation.mode == 3) setpoint = stationParams.set_temperature_low;

      if ((temp >= setpoint) && !fuzzy_state) {
        Serial.println("FUZZY");
        Serial.println(sessionStation.eta);
        Serial.println(rtc.getEpoch());
        Serial.println(sessionStation.duration);
        sessionStation.eta = rtc.getEpoch() + sessionStation.duration;
        Serial.println(sessionStation.eta);
        fuzzy_state = true;
        sendETA();
      }

      float proporsional, integral, derivative;

      unsigned long currentTime = millis();
      float deltaTime = (currentTime - dataPID.previousTime)/1000.0;
      float error = setpoint - dataPID.temp;
      float deltaError = error - dataPID.lastError;

      proporsional = stationParams.pid_proportional*error;
      integral = constrain((dataPID.lastIntegral + stationParams.pid_integral*error*deltaTime), 0, 80);
      derivative = stationParams.pid_derivative*(deltaError/deltaTime);
      
      dataPID.lastError = error;
      dataPID.lastIntegral = integral;
      dataPID.previousTime = currentTime;

      uint8_t totalPID = constrain(proporsional + integral + derivative, 25, stationParams.max_heater_power);
      dimmer.setPower(totalPID);

      analogWrite(pinFan, constrain(stationParams.speed_fan, 50, 255));
      
      // Serial.print("Temp: ");
      // Serial.print(dataPID.temp);
      // Serial.print("°C | Setpoint: ");
      // Serial.print(setpoint);
      // Serial.print("°C | Dimmer: ");
      // Serial.print(totalPID);
      // Serial.println("%");

      vTaskDelay(pdMS_TO_TICKS(1000));

      if (fuzzy_state && (rtc.getEpoch() >= sessionStation.eta)) {
        Serial.println("FUZZY END");
        Serial.println(sessionStation.eta);
        Serial.println(rtc.getEpoch());
        running = false;
        fuzzy_state = false;
      }
    } else vTaskDelay(pdMS_TO_TICKS(2000)); 

    // Serial.println("PID:" + String(uxTaskGetStackHighWaterMark(NULL)));
  }  
}

void TaskMain(void *pvParameters) {
  unsigned long sendMillis;
  unsigned long fanMillis;
  unsigned long timeMillis;
  sendDataNano(0);

  while (true) {
    if (running) {
      if (running_start) {
        bool session = false;

        if (continue_session) {
          running_start_setup = false;
          running_stop_setup = true;
        }

        if (running_start_setup) {
          sendDataNano(5);
          ulTaskNotifyTake(pdTRUE, 15000);
          
          vTaskSuspend(TaskHandleTouchScreen);
          vTaskSuspend(TaskHandleNavbar);
          vTaskSuspend(TaskHandleDisplay);

          sessionStation.initial_mass = dataReceiveNano.loadcell;
          strcpy(sessionStation.start_at, getDatetimeNow().c_str());
          uint8_t setpoint = 0;
          if (sessionStation.mode == 1) setpoint = stationParams.set_temperature_fast;
          else if (sessionStation.mode == 2) setpoint = stationParams.set_temperature_medium;
          else if (sessionStation.mode == 3) setpoint = stationParams.set_temperature_low;
          sessionStation.duration = durationFuzzy(setpoint, sessionStation.initial_mass);

          vTaskResume(TaskHandleTouchScreen);
          vTaskResume(TaskHandleNavbar);
          vTaskResume(TaskHandleDisplay);

          running_start_setup = false;
          running_stop_setup = true;

          //Start Setup
        }

        vTaskSuspend(TaskHandleTouchScreen);
        vTaskSuspend(TaskHandleNavbar);
        vTaskSuspend(TaskHandleDisplay);

        if (continue_session) {
          session = true;
          if (sessionStation.eta != 0) fuzzy_state = true;
        } else {
          if (startSession()) {
            session = true;
            fuzzy_state = false;
          }
        }

        if (session) {
          digitalWrite(pinRelay1, LOW);
          digitalWrite(pinRelay2, LOW);
          dataPID.previousTime = millis();
          sendDataNano(1);

          running_start = false;
          running_stop = true;
        }

        vTaskResume(TaskHandleTouchScreen);
        vTaskResume(TaskHandleNavbar);
        vTaskResume(TaskHandleDisplay);

      } else {

        //Main Loop
      }
    } else {
      if (running_stop) {
        if (running_stop_setup) {
          vTaskSuspend(TaskHandleTouchScreen);
          vTaskSuspend(TaskHandleNavbar);
          vTaskSuspend(TaskHandleDisplay);

          fanMillis = millis();

          digitalWrite(pinRelay1, HIGH);
          digitalWrite(pinRelay2, HIGH);
          strcpy(sessionStation.end_at, getDatetimeNow().c_str());
          sendDataNano(0);
          analogWrite(pinFan, 200);
          // analogWrite(pinFan, 30);

          vTaskResume(TaskHandleTouchScreen);
          vTaskResume(TaskHandleNavbar);
          vTaskResume(TaskHandleDisplay);
          
          vTaskDelay(pdMS_TO_TICKS(5000));

          sendDataNano(5);
          ulTaskNotifyTake(pdTRUE, 15000);

          sessionStation.final_mass = dataReceiveNano.loadcell;
          
          running_stop_setup = false;
          running_start_setup = true;
          
          //Stop Setup
        }

        vTaskSuspend(TaskHandleTouchScreen);
        vTaskSuspend(TaskHandleNavbar);
        vTaskSuspend(TaskHandleDisplay);

        if (endSession()) {
          startMillis = millis();
          running_stop = false;
          running_start = true;

          dataPID.previousTime = 0;
          dataPID.lastError = 0;
          dataPID.lastIntegral = 0;
        }

        vTaskResume(TaskHandleTouchScreen);
        vTaskResume(TaskHandleNavbar);
        vTaskResume(TaskHandleDisplay);
      }

      //Stop Loop
    }

    if ((millis() - fanMillis > 180000) && !running) {
      analogWrite(pinFan, 30);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    // Serial.println("Main:" + String(uxTaskGetStackHighWaterMark(NULL)));
  }
}