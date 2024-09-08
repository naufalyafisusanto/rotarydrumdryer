#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "HX711.h"

#define pinSerialESPRX 3
#define pinSerialESPTX 4

#define pinPUL A1
#define pinDIR A2
#define pinENA A3

#define pinDOUT1  12
#define pinSCK1   11
#define pinDOUT2  8
#define pinSCK2   7
#define pinDOUT3  6
#define pinSCK3   13
#define pinDOUT4  10
#define pinSCK4   9

typedef struct {
  char A;
  char B;
  char C;
  byte code = 0;
  /*
  0 = Stop
  1 = Start
  2 = Insert
  3 = Eject
  4 = Tare
  5 = Mass
  */
  } structReceive;
structReceive dataReceive;

typedef struct {
  uint16_t loadcell = 0;
  } structSend;
structSend dataSend;

uint8_t motorCode = 0;
// long moveStep = -153540;
long moveInsert = 204720;
long moveEject = -102360;
bool stateStepper = false;
bool runStepper = false;
bool constantStepper = false;
bool pauseStepper = false;
// unsigned long serialMillis;
unsigned long stepperMillis;

HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;
AccelStepper stepper(1, pinPUL, pinDIR);
SoftwareSerial SerialESP(pinSerialESPRX, pinSerialESPTX);

void TaskSerial(void *pvParameters) {
  scale1.tare(11);
  scale2.tare(11);
  scale3.tare(11);
  scale4.tare(11);
  vTaskDelay(pdMS_TO_TICKS(500));
  long tare1, tare2, tare3, tare4;
  EEPROM.get(0, tare1);
  EEPROM.get(sizeof(long), tare2);
  EEPROM.get(sizeof(long)*2, tare3);
  EEPROM.get(sizeof(long)*3, tare4);
  scale1.set_offset(tare1);
  scale2.set_offset(tare2);
  scale3.set_offset(tare3);
  scale4.set_offset(tare4);

  while (true) {
    if (SerialESP.available()) {
      SerialESP.listen();
      // Serial.println(SerialESP.available());
      SerialESP.readBytes((byte*)&dataReceive, sizeof(dataReceive));
      
      // Serial.print(dataReceive.A);
      // Serial.print(dataReceive.B);
      // Serial.println(dataReceive.C);

      if (dataReceive.A == '#' && dataReceive.B == '@' && dataReceive.C == '~'){
        dataSend.loadcell = 0;
        char printbuffer[40];
        sprintf(printbuffer, "Code : %d", dataReceive.code);
        // Serial.println(printbuffer);
        if (dataReceive.code == 1) {
          digitalWrite(pinENA, LOW);
          stepper.setAcceleration(1600);
          stepper.setMaxSpeed(6400);
          stepper.setCurrentPosition(stepper.currentPosition());
          stateStepper = false;
          stepper.move(moveEject);
          motorCode = dataReceive.code;
          runStepper = true;
        } else if (dataReceive.code == 2) {
          digitalWrite(pinENA, LOW);
          stepper.stop();
          vTaskDelay(pdMS_TO_TICKS(1000));
          stepper.setSpeed(9000);
          stepper.setMaxSpeed(9000);
          vTaskDelay(pdMS_TO_TICKS(500));
          constantStepper = true;
          motorCode = dataReceive.code;
        } else if (dataReceive.code == 3) {
          digitalWrite(pinENA, LOW);
          stepper.stop();
          vTaskDelay(pdMS_TO_TICKS(1000));
          stepper.setSpeed(-9000);
          stepper.setMaxSpeed(9000);
          vTaskDelay(pdMS_TO_TICKS(500));
          constantStepper = true;
          motorCode = dataReceive.code;
        } else if (dataReceive.code == 4) {
          pauseStepper = true;
          if (runStepper || constantStepper) vTaskDelay(pdMS_TO_TICKS(3500));          
          scale1.tare(11);
          scale2.tare(11);
          scale3.tare(11);
          scale4.tare(11);
          vTaskDelay(pdMS_TO_TICKS(500)); 
          EEPROM.put(0, scale1.get_offset());
          EEPROM.put(sizeof(long), scale2.get_offset());
          EEPROM.put(sizeof(long)*2, scale3.get_offset());
          EEPROM.put(sizeof(long)*3, scale4.get_offset());
          if (runStepper || constantStepper) vTaskDelay(pdMS_TO_TICKS(1500));
          pauseStepper = false;
        } else if (dataReceive.code == 5) {
          pauseStepper = true;
          if (runStepper || constantStepper) vTaskDelay(pdMS_TO_TICKS(3500));         
          uint16_t weight1 = scale1.get_units(11);
          uint16_t weight2 = scale2.get_units(11);
          uint16_t weight3 = scale3.get_units(11);
          uint16_t weight4 = scale4.get_units(11);
          if (runStepper || constantStepper) vTaskDelay(pdMS_TO_TICKS(1500));
          pauseStepper = false;
          
          dataSend.loadcell = weight1 + weight2 + weight3 + weight4;
          if (dataSend.loadcell > 20000) dataSend.loadcell = 0;
          
          // Serial.print(weight1);
          // Serial.print(",");
          // Serial.print(weight2);
          // Serial.print(",");
          // Serial.print(weight3);
          // Serial.print(",");
          // Serial.println(weight4);
          // Serial.println(dataSend.loadcell);

        } else {
          motorCode = 0;
          dataReceive.code = 0;
          digitalWrite(pinENA, HIGH);
        }

        SerialESP.write("$");
        SerialESP.write("*");
        SerialESP.write("!");
        SerialESP.write((const byte*)&dataSend, sizeof(dataSend));
        SerialESP.flush();
      }
    }

    SerialESP.flush();
    
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void TaskStepper(void *pvParameters) {
  stepper.setAcceleration(1600);
  stepper.setMaxSpeed(9000);
  stepper.setSpeed(9000);

  while (true) {
    if (!pauseStepper) {
      if (motorCode == 0) {
        stepper.moveTo(stepper.currentPosition());
        if (millis() - stepperMillis > 100) {
          stepperMillis = millis();
          runStepper = false;
          constantStepper = false;
        }
      } else if (motorCode == 1) {
        if (runStepper && (stepper.distanceToGo() == 0)) {
          stateStepper = !stateStepper;
          if (stateStepper) stepper.move(moveInsert);
          else stepper.move(moveEject);
        }
        stepper.run();
      } else if (motorCode == 2 || motorCode == 3) {
        if (constantStepper) stepper.runSpeed();
      } 
    } else vTaskDelay(pdMS_TO_TICKS(1000));

    // if (millis() - serialMillis > 500) {
    //   serialMillis = millis();
    //   Serial.println(stepper.distanceToGo() + String(",") + stepper.targetPosition() + "," + stepper.currentPosition() + "," + String(constantStepper));
    // }
  }
}

void setup() {
  // Serial.begin(9600);
  SerialESP.begin(9600);

  scale1.begin(pinDOUT1, pinSCK1);
  scale2.begin(pinDOUT2, pinSCK2);
  scale3.begin(pinDOUT3, pinSCK3);
  scale4.begin(pinDOUT4, pinSCK4);

  scale1.set_scale(217.332);
  scale2.set_scale(212.237);
  scale3.set_scale(217.915);
  scale4.set_scale(202.914);

  scale1.tare(11);
  scale2.tare(11);
  scale3.tare(11);
  scale4.tare(11);

  pinMode(pinENA, OUTPUT);
  digitalWrite(pinENA, HIGH);

  xTaskCreate(
    TaskSerial,    // Task function
    "Serial",      // Task name
    512,           // Stack size
    NULL,          // Task parameter
    2,             // Task priority
    NULL           // Task handle
  );

  xTaskCreate(
    TaskStepper,   // Task function
    "Stepper",     // Task name
    128,           // Stack size
    NULL,          // Task parameter
    1,             // Task priority
    NULL           // Task handle
  );
}

void loop() {
  // Kalo diisi berarti idiot!
}