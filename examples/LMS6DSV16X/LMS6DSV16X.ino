#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <IntervalTimer.h>
#include "SparkFun_LSM6DSV16X.h"

// ================= USER CONFIG =================
const float SAMPLE_RATE_HZ = 480.0;
const int BUFFER_SIZE = 4096;
const uint8_t ACCEL_RANGE = 8;

// Pins
const int LED_PIN = 13;
const int BUTTON_PIN = 28;
const int BUTTON_GND_PIN = 31;

// ================= HARDWARE =================
SparkFun_LSM6DSV16X imu;
File dataFile;
IntervalTimer sampleTimer;

// ================= PACKED DATA STRUCT =================
struct __attribute__((packed)) Sample {
  uint32_t t;   // timestamp (micros)
  int16_t x, y, z;
};

const int SAMPLES_PER_BUFFER = BUFFER_SIZE / sizeof(Sample);

// Double buffers
volatile Sample bufferA[SAMPLES_PER_BUFFER];
volatile Sample bufferB[SAMPLES_PER_BUFFER];

volatile Sample* activeBuffer = bufferA;
volatile Sample* writeBuffer = bufferB;

volatile int bufferIndex = 0;
volatile bool bufferReady = false;

// ================= STATE =================
bool recording = false;

// ================= BUTTON =================
int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// ================= FILE =================
int fileIndex = 0;

void openNewFile() {
  char filename[20];
  sprintf(filename, "log%03d.bin", fileIndex);
  while (SD.exists(filename)) {
    fileIndex++;
    sprintf(filename, "log%03d.bin", fileIndex);
  }
  dataFile = SD.open(filename, FILE_WRITE);
}

// ================= ISR =================
void sampleISR() {
  if (!recording) return;

  if (bufferReady) return; // prevent overwrite

  sfe_lsm_raw_data_t accelData;
  if (!imu.getRawAccel(&accelData)) return;

  activeBuffer[bufferIndex].t = micros();
  activeBuffer[bufferIndex].x = accelData.xData;
  activeBuffer[bufferIndex].y = accelData.yData;
  activeBuffer[bufferIndex].z = accelData.zData;

  bufferIndex++;

  if (bufferIndex >= SAMPLES_PER_BUFFER) {
    volatile Sample* temp = activeBuffer;
    activeBuffer = writeBuffer;
    writeBuffer = temp;

    bufferIndex = 0;
    bufferReady = true;
  }
}

// ================= SETUP =================
void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_GND_PIN, OUTPUT);
  digitalWrite(BUTTON_GND_PIN, LOW);

  Wire.begin();
  Wire.setClock(1000000);

  if (!imu.begin()) {
    while (1) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }

  imu.setAccelDataRate(LSM6DSV16X_ODR_AT_480Hz);

  if (ACCEL_RANGE == 2) imu.setAccelFullScale(LSM6DSV16X_2g);
  else if (ACCEL_RANGE == 4) imu.setAccelFullScale(LSM6DSV16X_4g);
  else if (ACCEL_RANGE == 8) imu.setAccelFullScale(LSM6DSV16X_8g);
  else if (ACCEL_RANGE == 16) imu.setAccelFullScale(LSM6DSV16X_16g);

  if (!SD.begin(BUILTIN_SDCARD)) {
    while (1) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }

  openNewFile();

  sampleTimer.begin(sampleISR, 1000000.0 / SAMPLE_RATE_HZ);
}

// ================= LOOP =================
void loop() {

  // -------- BUTTON --------
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        recording = !recording;

        if (!recording) {
          noInterrupts();
          int remaining = bufferIndex;
          interrupts();

          if (remaining > 0) {
            dataFile.write((uint8_t*)activeBuffer, remaining * sizeof(Sample));
          }

          dataFile.flush();
          dataFile.close();

          fileIndex++;
          openNewFile();
        } else {
          bufferIndex = 0;
        }
      }
    }
  }

  lastButtonState = reading;

  // -------- LED --------
  if (recording) {
    digitalWrite(LED_PIN, millis() % 200 < 100);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // -------- WRITE BUFFER --------
  if (bufferReady) {
    noInterrupts();
    Sample* buf = (Sample*)writeBuffer;
    bufferReady = false;
    interrupts();

    dataFile.write((uint8_t*)buf, SAMPLES_PER_BUFFER * sizeof(Sample));
  }
}