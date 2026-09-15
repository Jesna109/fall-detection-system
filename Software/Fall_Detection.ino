#define BLYNK_TEMPLATE_ID "xxxxx"
#define BLYNK_TEMPLATE_NAME "xxxxx"
#define BLYNK_AUTH_TOKEN "xxxxx"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

Adafruit_MPU6050 mpu;

char ssid[] = "XXXXXX";
char pass[] = "xxxxxx";

const int BUTTON_PIN = 18;
const int BUZZER_PIN = 19;

const float ACCEL_FALL_THRESHOLD = 20.0;
const float GYRO_FALL_THRESHOLD = 3.5;

bool fallDetected = false;
unsigned long fallTime = 0;
unsigned long lastBuzzTime = 0;
unsigned long lastPrintTime = 0;
bool buzzerState = false;
const unsigned long CONFIRM_WINDOW = 10000;
const unsigned long BUTTON_IGNORE_WINDOW = 200;

float peakAccel = 0;
float peakGyro = 0;

BLYNK_CONNECTED() {
  Blynk.virtualWrite(V8, 0);
  Blynk.virtualWrite(V9, "✅ System Online & Monitoring");
  Serial.println("Blynk Connected");
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  Serial.println("Fall Detection System Started");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);

  Serial.println("Connecting to WiFi and Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  delay(2000);
  Blynk.virtualWrite(V8, 0);
  Blynk.virtualWrite(V9, "✅ System Ready");
  Serial.println("✅ System Ready");
}

void loop() {
  Blynk.run();

  unsigned long currentTime = millis();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelMag = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  float gyroMag = sqrt(
    g.gyro.x * g.gyro.x +
    g.gyro.y * g.gyro.y +
    g.gyro.z * g.gyro.z
  );

  if (accelMag > peakAccel) peakAccel = accelMag;
  if (gyroMag > peakGyro) peakGyro = gyroMag;

  if (currentTime - lastPrintTime >= 1000) {
    Serial.print("Peak Accel: "); Serial.print(peakAccel);
    Serial.print(" m/s² | Peak Gyro: "); Serial.print(peakGyro);
    Serial.println(" rad/s");

    Blynk.virtualWrite(V0, a.acceleration.x);
    Blynk.virtualWrite(V1, a.acceleration.y);
    Blynk.virtualWrite(V2, a.acceleration.z);
    Blynk.virtualWrite(V6, peakAccel);
    Blynk.virtualWrite(V7, peakGyro);

    peakAccel = 0;
    peakGyro = 0;

    lastPrintTime = currentTime;
  }

  if (!fallDetected) {
    if (accelMag > ACCEL_FALL_THRESHOLD && gyroMag > GYRO_FALL_THRESHOLD) {
      fallDetected = true;
      fallTime = currentTime;
      lastBuzzTime = currentTime;
      buzzerState = false;

      Serial.println("==================================");
      Serial.println(">> POSSIBLE FALL DETECTED!");
      Serial.println(">> Press button to cancel if false alarm.");
      Serial.println("==================================");

      Blynk.virtualWrite(V8, 255);
      Blynk.virtualWrite(V9, "⚠️ Possible Fall - Verifying...");
    }
  }

  if (fallDetected) {

    if (currentTime - fallTime > BUTTON_IGNORE_WINDOW) {
      if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("==================================");
        Serial.println(">> FALSE ALARM - Alert cancelled.");
        Serial.println("==================================");

        fallDetected = false;
        noTone(BUZZER_PIN);
        buzzerState = false;

        Blynk.virtualWrite(V8, 0);
        Blynk.virtualWrite(V9, "✅ Normal - False Alarm Cleared");
        Blynk.logEvent("false_alert", "User confirmed: False alarm");

        unsigned long falseAlarmShowTime = millis();
        while (millis() - falseAlarmShowTime < 3000) {
          Blynk.run();
        }
        Blynk.virtualWrite(V9, "✅ System Online & Monitoring");

        return;
      }
    }

    static unsigned long lastCountTime = 0;
    if (currentTime - lastCountTime >= 1000) {
      unsigned long elapsed = currentTime - fallTime;
      if (elapsed < CONFIRM_WINDOW) {
        int remaining = (CONFIRM_WINDOW - elapsed) / 1000;
        Serial.print(">> Time remaining: ");
        Serial.print(remaining);
        Serial.println(" seconds");

        Blynk.virtualWrite(V9, String("⚠️ Verifying... ") + remaining + "s");
      }
      lastCountTime = currentTime;
    }

    if (currentTime - fallTime >= CONFIRM_WINDOW) {
      Serial.println("==================================");
      Serial.println("    🚨 FALL CONFIRMED! 🚨        ");
      Serial.println("  SENDING ALERT TO CARETAKER...  ");
      Serial.println("==================================");

      fallDetected = false;
      noTone(BUZZER_PIN);
      buzzerState = false;

      Blynk.logEvent("fall_alert","FALL DETECTED! Please check on the person immediately.");
      Blynk.virtualWrite(V8, 255);
      Blynk.virtualWrite(V9, "🚨 FALL CONFIRMED - Alert Sent!");

      unsigned long fallConfirmShowTime = millis();
      while (millis() - fallConfirmShowTime < 3000) {
        Blynk.run();
      }
      Blynk.virtualWrite(V8, 0);
      Blynk.virtualWrite(V9, "✅ System Online & Monitoring");

      return;
    }

    if (currentTime - lastBuzzTime >= 500) {
      buzzerState = !buzzerState;
      if (buzzerState) {
        tone(BUZZER_PIN, 1000);
      } else {
        noTone(BUZZER_PIN);
      }
      lastBuzzTime = currentTime;
    }
  }
}