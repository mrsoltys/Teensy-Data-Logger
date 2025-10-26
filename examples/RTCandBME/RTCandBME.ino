#include <Arduino.h>
#include <SD.h>
#include <Wire.h>
//#include <SparkFun_RV8803.h>
#include <SparkFun_RV1805.h>
#include <SparkFunBME280.h>

// ========== CONFIG ==========
const int chipSelect = BUILTIN_SDCARD;

// ========== SENSORS ==========
RV1805 rtc;
//RV8803 rtc;
BME280 bme;

// ========== BUFFER ==========
const int BUFFER_SIZE = 512;
char buffer[BUFFER_SIZE];
volatile int bufferIndex = 0;
volatile bool bufferReady = false;

// ========== STATE ==========
bool recording = false;
File dataFile;

// ========== BUTTON + LED ==========
const int LED_PIN = 13;
const int BUTTON_PIN = 14;
const int BUTTON_GND_PIN = 31;

// Debounce state variables
int lastRawButtonState = HIGH;     // last raw reading from digitalRead()
int debouncedButtonState = HIGH;   // "stable" button state after debouncing
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms

// ========== ISR ==========
const float SAMPLE_RATE_HZ = 0.1;  // 0.01 Hz = one sample every 100 sec
IntervalTimer sampleTimer;
unsigned long lastSampleMillis = 0;
const unsigned long SAMPLE_PERIOD_MS = (1000.0 / SAMPLE_RATE_HZ);
void sampleISR() {
  //Do nothing if not recording
  if (!recording) return;

  //======READ SENSORS========
  float temp = bme.readTempC();
  float hum = bme.readFloatHumidity();
  float press = bme.readFloatPressure();
  
  rtc.updateTime();
  int year = rtc.getYear();
  int month = rtc.getMonth();
  int day = rtc.getDate();
  int hour = rtc.getHours();
  int minute = rtc.getMinutes();
  int second = rtc.getSeconds();
  

  int written = snprintf(&buffer[bufferIndex], BUFFER_SIZE - bufferIndex,
                         "%04d-%02d-%02d %02d:%02d:%02d,%.2f,%.2f,%.2f,%lu\n",
                         2000 + year, month, day, hour, minute, second,
                         temp, hum, press, millis());
  bufferIndex += written;

  if (bufferIndex >= BUFFER_SIZE - 80) {
    bufferReady = true;
  }
}

// ========== SETUP ==========
void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_GND_PIN, OUTPUT);
  digitalWrite(BUTTON_GND_PIN, LOW);

  Serial.begin(115200);
  while (!Serial);
  Wire.begin();

  Serial.println("Initializing SD...");
  if (!SD.begin(chipSelect)) {
    Serial.println("SD init failed!");
    while (1);
  }
  Serial.println("SD OK.");

  if (bme.begin() == false) {
    Serial.println("BME280 not found!");
    while (1);
  }
  Serial.println("BME280 OK.");

  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    while (1);
  }
  Serial.println("RTC OK.");

  if (Serial) {
    Serial.println("Setting RTC to compiler time...");
    rtc.setToCompilerTime();
    float offsetHours = -6.5; //I find myself needing to adjust the clock -6.5 hrs.
    uint8_t hour = rtc.getHours();
    int correctedHour = (int)(hour + offsetHours + 24) % 24;
    rtc.setHours(correctedHour);
  }

  // Start the sampling timer only for fast sampling rates. Otherwise, do this manually in the loops for slow sampling rates
  if (SAMPLE_RATE_HZ >= 0.1) {
    sampleTimer.begin(sampleISR, (1.0e6 / SAMPLE_RATE_HZ));
  } 
  Serial.print("Sampling at ");
  Serial.print(SAMPLE_RATE_HZ);
  Serial.println(" Hz");
}

// ========== START / STOP ==========
void startRecording() {
  lastSampleMillis = millis() - SAMPLE_PERIOD_MS; // ensure immediate first sample
  char filename[32];
  int filenum = 0;
  do {
    snprintf(filename, sizeof(filename), "LOG%03d.CSV", filenum++);
  } while (SD.exists(filename));

  dataFile = SD.open(filename, FILE_WRITE);
  if (!dataFile) {
    Serial.println("Failed to create file!");
    return;
  }

  dataFile.println("datetime,temp_C,humidity_pct,pressure_Pa,millis");
  dataFile.flush();

  recording = true;
  bufferIndex = 0;
  bufferReady = false;

  Serial.print("Recording started: ");
  Serial.println(filename);
}

void stopRecording() {
  Serial.println("Stopping recording...");
  if (bufferIndex > 0) {
    Serial.print("Flushing remaining buffer (");
    Serial.print(bufferIndex);
    Serial.println(" bytes)");
    dataFile.write((uint8_t*)buffer, bufferIndex);
    bufferIndex = 0;
  }
  dataFile.close();
  recording = false;
  Serial.println("Recording stopped and file closed.");
}

// ========== LOOP ==========
void loop() {
   // --- Debounce button (stable state only updated after stable period) ---
  int rawReading = digitalRead(BUTTON_PIN);

  // If the raw reading changed, reset the debounce timer
  if (rawReading != lastRawButtonState) {
    lastDebounceTime = millis();
  }

  // Only if the input has been stable longer than debounceDelay do we accept it
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the debounced state is different from the raw reading, update it
    if (rawReading != debouncedButtonState) {
      debouncedButtonState = rawReading;

      // We're using INPUT_PULLUP: pressed -> LOW, released -> HIGH
      if (debouncedButtonState == LOW) {
        // Button was pressed (falling edge after debounce)
        if (!recording) startRecording();
        else stopRecording();
      }
    }
  }

  // Save the raw reading for next loop comparison
  lastRawButtonState = rawReading;

  // LED behavior
  if (recording) {
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 500) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      lastBlink = millis();
    }
  } else {
    digitalWrite(LED_PIN, HIGH);
  }

  // Write buffer if ready
  if (bufferReady && recording) {
    Serial.print("Buffer ready, writing ");
    Serial.print(bufferIndex);
    Serial.println(" bytes...");
    dataFile.write((uint8_t*)buffer, bufferIndex);
    bufferIndex = 0;
    bufferReady = false;
    Serial.println("Buffer written.");
  }

  // ---- Replace ISR with millis() check for very slow rates ----
  if (SAMPLE_RATE_HZ < 0.1) {
    while (millis() - lastSampleMillis >= SAMPLE_PERIOD_MS) {
      lastSampleMillis += SAMPLE_PERIOD_MS;
      sampleISR();
    }
  }
}