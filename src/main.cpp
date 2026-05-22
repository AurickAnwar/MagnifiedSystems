#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

Adafruit_MPU6050 mpu;

// -------------------- SETTINGS --------------------
const float START_ACCEL_DELTA = 16.0;     // stronger accel needed to start
const float START_GYRO_MAG    = 4.0;      // stronger rotation needed to start

const float QUIET_ACCEL_DELTA = 1.5;      // considered calm if below this
const float QUIET_GYRO_MAG    = 0.30;     // considered calm if below this

const unsigned long QUIET_TIME_MS = 450;  // must stay calm this long to finish
const unsigned long MIN_EVENT_MS  = 100;  // ignore tiny fake bumps

// -------------------- EVENT STATE --------------------
bool impactActive = false;
unsigned long eventStartTime = 0;
unsigned long lastMotionTime = 0;

float peakAccelDelta = 0.0;
float peakGyroMag = 0.0;

// -------------------- HELPERS --------------------
float getMagnitude(float x, float y, float z) {
  return sqrt(x * x + y * y + z * z);
}

float clampFloat(float x, float low, float high) {
  if (x < low) return low;
  if (x > high) return high;
  return x;
}

String getLabel(float score) {
  if (score >= 80.0) return "EMERGENCY";
  if (score >= 60.0) return "GET IT CHECKED OUT";
  if (score >= 30.0) return "MODERATE";
  return "NOT TOO SEVERE";
}

// Continuous severity score from 0 to 100
float calculateSeverity(float peakA, float peakG, unsigned long durationMs) {
  // Accel contribution
  // Below ~16 should barely count. 16 to 45 ramps up.
  float accelNorm = (peakA - 16.0) / (45.0 - 16.0);
  accelNorm = clampFloat(accelNorm, 0.0, 1.0);

  // Gyro contribution
  // Below ~4 should barely count. 4 to 10 ramps up.
  float gyroNorm = (peakG - 4.0) / (10.0 - 4.0);
  gyroNorm = clampFloat(gyroNorm, 0.0, 1.0);

  // Duration contribution
  // Very short events should not score high.
  float durationNorm = (durationMs - 120.0) / (1200.0 - 120.0);
  durationNorm = clampFloat(durationNorm, 0.0, 1.0);

  // Nonlinear shaping:
  // squaring the values keeps small shakes low
  accelNorm = accelNorm * accelNorm;
  gyroNorm = gyroNorm * gyroNorm;

  // Weighted final score
  float score =
      (accelNorm * 65.0) +
      (gyroNorm * 25.0) +
      (durationNorm * 10.0);

  return clampFloat(score, 0.0, 100.0);
}

void resetImpact() {
  impactActive = false;
  eventStartTime = 0;
  lastMotionTime = 0;
  peakAccelDelta = 0.0;
  peakGyroMag = 0.0;
}

void finishImpact() {
  unsigned long duration = millis() - eventStartTime;

  if (duration < MIN_EVENT_MS) {
    Serial.println("Tiny motion ignored.");
    resetImpact();
    return;
  }

  float severity = calculateSeverity(peakAccelDelta, peakGyroMag, duration);

  Serial.println();
  Serial.println("===== IMPACT COMPLETE =====");
  Serial.print("Duration (ms): ");
  Serial.println(duration);
  Serial.print("Peak accel delta: ");
  Serial.println(peakAccelDelta, 2);
  Serial.print("Peak gyro: ");
  Serial.println(peakGyroMag, 2);
  Serial.print("Severity score: ");
  Serial.println(severity, 1);   // one decimal place
  Serial.print("Status: ");
  Serial.println(getLabel(severity));
  Serial.println("===========================");
  Serial.println();

  resetImpact();
}

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("START");

  Wire.begin(21, 22);
  Wire.setClock(100000);

  if (!mpu.begin()) {
    Serial.println("MPU NOT FOUND");
  } else {
    Serial.println("MPU READY");
  }
}

// -------------------- LOOP --------------------
void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelMag = getMagnitude(a.acceleration.x, a.acceleration.y, a.acceleration.z);
  float accelDelta = fabs(accelMag - 9.81);

  float gyroMag = getMagnitude(g.gyro.x, g.gyro.y, g.gyro.z);

  bool startImpact = (accelDelta > START_ACCEL_DELTA) || (gyroMag > START_GYRO_MAG);
  bool quietNow = (accelDelta < QUIET_ACCEL_DELTA) && (gyroMag < QUIET_GYRO_MAG);

  if (!impactActive && startImpact) {
    impactActive = true;
    eventStartTime = millis();
    lastMotionTime = millis();
    peakAccelDelta = accelDelta;
    peakGyroMag = gyroMag;

    Serial.println("Impact started...");
  }

  if (impactActive) {
    if (accelDelta > peakAccelDelta) {
      peakAccelDelta = accelDelta;
    }

    if (gyroMag > peakGyroMag) {
      peakGyroMag = gyroMag;
    }

    if (!quietNow) {
      lastMotionTime = millis();
    }

    if (millis() - lastMotionTime >= QUIET_TIME_MS) {
      finishImpact();
    }
  }

  delay(20);
}