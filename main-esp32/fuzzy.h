float fmass_light(uint16_t mass) {
  if (mass < fuzzy_light[0]) return 0;
  else if (mass >= fuzzy_light[0] && mass <= fuzzy_light[1]) return (mass - fuzzy_light[0])/(fuzzy_light[1] - fuzzy_light[0]);
  else if (mass >= fuzzy_light[1] && mass <= fuzzy_light[2]) return (fuzzy_light[2] - mass)/(fuzzy_light[2] - fuzzy_light[1]);
  else return 0;
}

float fmass_medium(uint16_t mass) {
  if (mass < fuzzy_medium[0]) return 0;
  else if (mass >= fuzzy_medium[0] && mass <= fuzzy_medium[1]) return (mass - fuzzy_medium[0])/(fuzzy_medium[1] - fuzzy_medium[0]);
  else if (mass >= fuzzy_medium[1] && mass <= fuzzy_medium[2]) return (fuzzy_medium[2] - mass)/(fuzzy_medium[2] - fuzzy_medium[1]);
  else return 0;
}

float fmass_heavy(uint16_t mass) {
  if (mass < fuzzy_heavy[0]) return 0;
  else if (mass >= fuzzy_heavy[0] && mass <= fuzzy_heavy[1]) return (mass - fuzzy_heavy[0])/(fuzzy_heavy[1] - fuzzy_heavy[0]);
  else if (mass >= fuzzy_heavy[1] && mass <= fuzzy_heavy[2]) return (fuzzy_heavy[2] - mass)/(fuzzy_heavy[2] - fuzzy_heavy[1]);
  else return 0;
}

float fmass_veryheavy(uint16_t mass) {
  if (mass < fuzzy_veryheavy[0]) return 0;
  else if (mass >= fuzzy_veryheavy[0] && mass <= fuzzy_veryheavy[1]) return (mass - fuzzy_veryheavy[0])/(fuzzy_veryheavy[1] - fuzzy_veryheavy[0]);
  else return 0;
}

float ftemp_slow(uint8_t setpoint) {
  if (setpoint < fuzzy_slow[0]) return 0;
  else if (setpoint == fuzzy_slow[0]) return 1; 
  else if (setpoint > fuzzy_slow[0] && setpoint <= fuzzy_slow[2]) return (fuzzy_slow[2] - setpoint) / (fuzzy_slow[2] - fuzzy_slow[1]);
  else return 0;
}

float ftemp_medium(uint8_t setpoint) {
  if (setpoint < fuzzy_normal[0]) return 0;
  else if (setpoint >= fuzzy_normal[0] && setpoint <= fuzzy_normal[1]) return (setpoint - fuzzy_normal[0])/(fuzzy_normal[1] - fuzzy_normal[0]);
  else if (setpoint >= fuzzy_normal[1] && setpoint <= fuzzy_normal[2]) return (fuzzy_normal[2] - setpoint)/(fuzzy_normal[2] - fuzzy_normal[1]);
  else return 0;
}

float ftemp_fast(uint8_t setpoint) {
  if (setpoint < fuzzy_fast[0]) return 0;
  else if (setpoint >= fuzzy_fast[0] && setpoint <= fuzzy_fast[1]) return (setpoint - fuzzy_fast[0])/(fuzzy_fast[1] - fuzzy_fast[0]);
  else return 0;
}

uint16_t durationFuzzy(uint8_t setpoint, uint16_t mass) {
  if (mass > 1500) mass = 1500;
  else if (mass < 600) mass = 600;

  float rule_1 = min(ftemp_slow(setpoint), fmass_light(mass));
  float rule_2 = min(ftemp_slow(setpoint), fmass_medium(mass));
  float rule_3 = min(ftemp_slow(setpoint), fmass_heavy(mass));
  float rule_4 = min(ftemp_slow(setpoint), fmass_veryheavy(mass));
  float rule_5 = min(ftemp_medium(setpoint), fmass_light(mass));
  float rule_6 = min(ftemp_medium(setpoint), fmass_medium(mass));
  float rule_7 = min(ftemp_medium(setpoint), fmass_heavy(mass));
  float rule_8 = min(ftemp_medium(setpoint), fmass_veryheavy(mass));
  float rule_9 = min(ftemp_fast(setpoint), fmass_light(mass));
  float rule_10 = min(ftemp_fast(setpoint), fmass_medium(mass));
  float rule_11 = min(ftemp_fast(setpoint), fmass_heavy(mass));
  float rule_12 = min(ftemp_fast(setpoint), fmass_veryheavy(mass));

  float duration_1 = 305; 
  float duration_2 = 325;
  float duration_3 = 340;
  float duration_4 = 360;
  float duration_5 = 380;
  float duration_6 = 400; 
  float duration_7 = 420; 
  float duration_8 = 425;
  float duration_9 = 445;
  float duration_10 = 470;

  float duration = (
    rule_1*duration_7 + rule_2*duration_8 + rule_3*duration_9 + \
    rule_4*duration_10 + rule_5*duration_4 + rule_6*duration_5 + \
    rule_7*duration_6 + rule_8*duration_8 + rule_9*duration_1 + \
    rule_10*duration_2 + rule_11*duration_3 + rule_12*duration_4
  )/(
    rule_1 + rule_2 + rule_3 + rule_4 + rule_5 + rule_6 + \
    rule_7 + rule_8 + rule_9 + rule_10 + rule_11 + rule_12
  );

  return duration*60;
}