void updateData() {
  strcpy(uploadData.timestamp, getDatetimeNow().c_str());
  if (simulation) {
    uploadData.voltage = random(2200, 2400) / 10.0;
    uploadData.current = random(0, 70) / 10.0;
    uploadData.power_factor = random(90, 100) / 100.0;
    uploadData.power = uploadData.voltage*uploadData.current*uploadData.power_factor;
    uploadData.frequency = random(49, 51);
    uploadData.humidity = random(100, 200) / 10.0;
    uploadData.temp = dataPID.temp;
  } else {
    structData bufferData;
    bufferData.voltage = pzem.voltage();
    bufferData.current = pzem.current();
    bufferData.power_factor = pzem.pf();
    bufferData.power = pzem.power();
    bufferData.frequency = pzem.frequency();
    bufferData.humidity = htu.readHumidity();
    
    uploadData.voltage = isNaNorZero(bufferData.voltage) ? uploadData.voltage : bufferData.voltage;
    uploadData.current = isNaNorZero(bufferData.current) ? uploadData.current : bufferData.current;
    uploadData.power = isNaNorZero(bufferData.power) ? uploadData.power : bufferData.power;
    uploadData.power_factor = isNaNorZero(bufferData.power_factor) ? uploadData.power_factor : bufferData.power_factor;
    uploadData.frequency = isNaNorZero(bufferData.frequency) ? uploadData.frequency : bufferData.frequency;
    uploadData.humidity = isNaNorZero(bufferData.humidity) ? uploadData.humidity : bufferData.humidity;
    uploadData.temp = dataPID.temp;
  }
}

void sendDataNano(uint8_t code = 0) {
  dataSendNano.code = code;

  xTaskNotifyGive(TaskHandleNano);
}

void TaskNano(void *pvParameters) {
  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (simulation) {
      if (sessionStation.initial_mass == 0) dataReceiveNano.loadcell = random(3000, 5000);
      else dataReceiveNano.loadcell = random(700, sessionStation.initial_mass/2);
      xTaskNotifyGive(TaskHandleMain);
    } else {
      SerialNano.flush();
      SerialNano.listen();

      SerialNano.write("#");
      SerialNano.write("@");
      SerialNano.write("~");
      SerialNano.write((const byte*)&dataSendNano, sizeof(dataSendNano));

      SerialNano.flush();
        // Serial.println("Perintah sudah dikirim");
      
      if ((dataSendNano.code == 4) || (dataSendNano.code == 5)) {
        for (uint8_t i = 0; i < 5; i++) {
          vTaskDelay(pdMS_TO_TICKS(3000));

          if (SerialNano.available()) {
            SerialNano.readBytes((byte*)&dataReceiveNano, sizeof(dataReceiveNano));
            // Serial.print(dataReceiveNano.A);
            // Serial.print(dataReceiveNano.B);
            // Serial.println(dataReceiveNano.C);

            if (dataReceiveNano.A == '$' && dataReceiveNano.B == '*' && dataReceiveNano.C == '!'){
              if (dataSendNano.code == 4) dataReceiveNano.loadcell = 0;
              else xTaskNotifyGive(TaskHandleMain);
              
              // Serial.println(dataReceiveNano.loadcell);
              SerialNano.flush();
              break;
            }
          }
        }       
      }
    }

    // Serial.println("Nano:" + String(uxTaskGetStackHighWaterMark(NULL)));
  }
}