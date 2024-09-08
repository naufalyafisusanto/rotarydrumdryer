typedef struct {
  IPAddress localIP = IPAddress(0, 0, 0, 0);
  char hostname[30] = "";
  uint8_t id = 0;
  char token[10] = "";
} structConf;
structConf stationConf;

typedef struct {
  unsigned long id = 0;
  uint16_t duration = 0;
  uint32_t eta = 0;
  uint8_t mode = 0;
  char token[10] = "";
  char start_at[20];
  uint16_t initial_mass = 0;
  char end_at[20];
  uint16_t final_mass = 0;
} structSession;
structSession sessionStation;

typedef struct {
  char timestamp[20];
  float voltage = 0;
  float current = 0;
  float power = 0;
  uint8_t frequency = 0;
  float power_factor = 0;
  float temp = 0;
  float humidity = 0;
  } structData;
structData uploadData;

typedef struct {
  uint16_t speed_fan = 150;
  uint16_t max_heater_power = 90;
  uint8_t set_temperature_fast = 75;
  uint8_t set_temperature_medium = 65;
  uint8_t set_temperature_low = 55;
  float pid_proportional = 5.6;
  float pid_integral = 0.02;
  float pid_derivative = 2185.87;
} structParams;
structParams stationParams;

typedef struct __attribute__((packed)) {
  float temp = 0;
  unsigned long previousTime = 0;
  float lastError = 0;
  float lastIntegral = 0;
  } structPID;
structPID dataPID;

typedef struct __attribute__((packed)) {
  byte code = 0;
  /*
  0 = Stop
  1 = Start
  2 = Insert
  3 = Eject
  4 = Tare
  */
  } structSendNano;
structSendNano dataSendNano;

typedef struct __attribute__((packed)) {
  char A;
  char B;
  char C;
  uint16_t loadcell = 0;
  } structReceiveNano;
structReceiveNano dataReceiveNano;