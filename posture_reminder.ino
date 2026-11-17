#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

const int MOTOR_PIN = 4;
const float SLOUCH_THRESHOLD = 15.0;
const int BAD_POSTURE_DELAY = 2000;

float baselineX = 0;
float baselineY = 0;
bool isCalibrated = false;
unsigned long badPostureStart = 0;
bool motorOn = false;

float angleDiff(float a, float b) {
  float diff = a - b;
  while (diff > 180) diff -= 360;
  while (diff < -180) diff += 360;
  return abs(diff);
}

void buzz(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(MOTOR_PIN, HIGH);
    delay(200);
    digitalWrite(MOTOR_PIN, LOW);
    delay(200);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  if (!bno.begin()) {
    Serial.println("Sensor failed!");
    while(1);
  }

  Serial.println("Put device on your back and stand up straight...");
  buzz(3);

  for (int i = 5; i > 0; i--) {
    Serial.print("Calibrating in ");
    Serial.print(i);
    Serial.println(" seconds...");
    delay(1000);
  }

  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  baselineX = euler.x();
  baselineY = euler.y();
  isCalibrated = true;

  digitalWrite(MOTOR_PIN, HIGH);
  delay(600);
  digitalWrite(MOTOR_PIN, LOW);

  Serial.println("Calibrated! Monitoring posture now.");
}

void loop() {
  if (!isCalibrated) return;

  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  float currentX = euler.x();
  float currentY = euler.y();

  float deviationX = angleDiff(currentX, baselineX);
  float deviationY = angleDiff(currentY, baselineY);
  float deviation = max(deviationX, deviationY);

  Serial.print("Deviation: ");
  Serial.print(deviation);
  Serial.print(" deg  ->  ");

  if (deviation > SLOUCH_THRESHOLD) {
    if (badPostureStart == 0) badPostureStart = millis();
    if (millis() - badPostureStart > BAD_POSTURE_DELAY) {
      if (!motorOn) {
        digitalWrite(MOTOR_PIN, HIGH);
        motorOn = true;
        Serial.println("BAD POSTURE - buzzing!");
      }
    } else {
      Serial.println("bad posture (waiting...)");
    }
  } else {
    badPostureStart = 0;
    if (motorOn) {
      digitalWrite(MOTOR_PIN, LOW);
      motorOn = false;
      Serial.println("Good posture - motor off");
    } else {
      Serial.println("Good posture");
    }
  }

  delay(100);
}
