
//For SD card read/write
#include <SD.h>
File dataFile;

//For reading accelerometer 
#include <Wire.h> // Must include Wire library for I2C
#include <SparkFun_MMA8452Q.h> // Includes the SFE_MMA8452Q library
#include <SPI.h>
MMA8452Q accel; // Default MMA8452Q object create. (Address = 0x1D)


// Buffer Variables
//=================
// We will add samples to a buffer and then write to the SD card in chunks to aid in faster sampling. 
const uint16_t BUFFER_SIZE = 512;             // Buffer size in samples
volatile uint16_t bufferIndex = 0;
volatile char dataBuffer[BUFFER_SIZE * 32];   // adjust 32 bytes/sample based on your data


//Sampling
//=========
//Set desired sample rate in hZ HERE, and adjust sampling code here. 
const float SAMPLE_RATE_HZ = 750;         // Set your desired sample rate
//Note: The fastest sample rate for the accelerometer in this example is 800 hz
IntervalTimer sampleTimer;
String dataString;
// This code void sample() will run at sample rate. it depends on the sample timer
// sampleTimer.begin(sample, 1000000 / SAMPLE_RATE_HZ); // period in microseconds in void setup()
void sample() {                             
  if (bufferIndex < BUFFER_SIZE) {
    accel.read(); // get latest accel reading
    // Format line: time,accelZ
    snprintf(&dataBuffer[bufferIndex * 32], 32, "%lu,%d\n", micros(), accel.z);
    bufferIndex++;
  }
}

void setup() {
  //Serial.begin(9600);
  //Sensor Setup
  accel.init(SCALE_8G, ODR_800); // Init and customize the FSR and ODR
  //Scale can be either SCALE_2G, SCALE_4G, or SCALE_8G. The "odr" variable can be either ODR_800, ODR_400, ODR_200, ODR_100, ODR_50, ODR_12, ODR_6, or ODR_1, respectively setting the data rate to 800, 400, 200, 100, 50, 12.5, 6.25, or 1.56 Hz.

  pinMode(LED_BUILTIN, OUTPUT);

  if (!SD.begin(BUILTIN_SDCARD)) {
    while (1) { digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); delay(100); }
  }

  int n = 0;
  char fileName[20];
  sprintf(fileName, "data%02u.csv", n);
  while (SD.exists(fileName)) {
    n++;
    sprintf(fileName, "data%02u.csv", n);
  }

  dataFile = SD.open(fileName, FILE_WRITE);
  if (!dataFile) {
    while (1) { digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); delay(100); }
  }

  dataFile.println("time_us,accelZ");
  dataFile.flush();

  // Start the sampling timer
  sampleTimer.begin(sample, 1000000 / SAMPLE_RATE_HZ); // period in microseconds
}

void loop() {
  if (bufferIndex >= BUFFER_SIZE) {
    sampleTimer.end();
    dataFile.write((const char*)dataBuffer, bufferIndex * 32);
    dataFile.flush();
    bufferIndex = 0;
    sampleTimer.begin(sample, 1000000 / SAMPLE_RATE_HZ);
  }
}