#include <rgb_lcd.h>

const int MOISTURE_SENSOR_PIN = A0;

const int MOTORPINS[] = {4, 5, 6, 7};

const int MAX_MOISTURE = 690;
const int MIN_MOISTURE = 440;
const int HYSTERESIS = 40;
const int EPSILON_MOISTURE = 8;
const int red[] = {255, 0, 0};
const int green[] = {0, 255, 0};
const int yellow[] = {255, 120, 0};
const int blue[] = {0, 0, 255};

const unsigned long UPDATE = 1000;

const int REVERSESTEPS = 256;

int stepscurrent;
int steppart;

int state;// 0: blue; 1: red; 2: yellow; 3: green;

rgb_lcd lcd;

bool activate_water = false;

unsigned long lastupdated;

void control_display(char message[], int color[], int moisture) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  lcd.setRGB(color[0], color[1], color[2]);
  lcd.setCursor(0, 1);
  lcd.print(moisture);
}

void control_water(int moisture) {
  unsigned long now = millis();
  if (now - lastupdated > UPDATE) {
    lastupdated = now;
    if (moisture < EPSILON_MOISTURE + (state <= 0 ? EPSILON_MOISTURE : 0)) {// less hysteresis, since it doesn't make sense to have more difference than low threshold
      activate_water = false;
      control_display("Not Connected!", blue, moisture);
    } else if (moisture > MAX_MOISTURE + (state <= 2 ? HYSTERESIS : 0)) {
      activate_water = false;
      control_display("Great!", green, moisture);
    } else if (moisture < MIN_MOISTURE + (state <= 1 ? HYSTERESIS : 0)) {
      activate_water = true;
      control_display("Not Good!", red, moisture);
    } else {
      activate_water = true;
      control_display("Okay!", yellow, moisture);
    }
  }
}

void movemotor() {
  if (steppart) {
    digitalWrite(MOTORPINS[steppart-1], LOW);
    digitalWrite(MOTORPINS[steppart], HIGH);
    steppart++;
    if (steppart >= 4) {
      steppart = 0;
      stepscurrent++;
      if (stepscurrent == REVERSESTEPS * 2) {
        stepscurrent = 0;
      }
    }
  } else {
    if (activate_water) {
      if (stepscurrent != REVERSESTEPS) {
        steppart = 1;
        digitalWrite(MOTORPINS[3], LOW);
        digitalWrite(MOTORPINS[0], HIGH);
      }
    } else {
      if (stepscurrent) {
        steppart = 1;
        digitalWrite(MOTORPINS[3], LOW);
        digitalWrite(MOTORPINS[0], HIGH);
      }
    }
  }
  delay(2);
}

void setup() {
  pinMode(MOISTURE_SENSOR_PIN, INPUT);
  lcd.begin(16, 2);
  lastupdated = millis()-UPDATE;
  stepscurrent = 0;
  steppart = 0;
  for (int i = 0; i < 4; i++) {
    pinMode(MOTORPINS[i], OUTPUT);
  }
  state = 0;
}

void loop() {
  int moisture = analogRead(MOISTURE_SENSOR_PIN);
  control_water(moisture);
  movemotor();
}
