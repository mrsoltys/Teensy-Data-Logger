
#include <SPI.h>
#include <SD.h>
const int chipSelect = BUILTIN_SDCARD;
File dataFile;

#include <Wire.h> // Must include Wire library for I2C
#include <SparkFun_MMA8452Q.h> // Includes the SFE_MMA8452Q library
MMA8452Q accel; // Default MMA8452Q object create. (Address = 0x1D)

void setup() {

  //Serial.begin(9600);
  accel.init(SCALE_8G, ODR_400); // Init and customize the FSR and ODR
  //Scale can be either SCALE_2G, SCALE_4G, or SCALE_8G. The "odr" variable can be either ODR_800, ODR_400, ODR_200, ODR_100, ODR_50, ODR_12, ODR_6, or ODR_1, respectively setting the data rate to 800, 400, 200, 100, 50, 12.5, 6.25, or 1.56 Hz.

  pinMode(22, OUTPUT); digitalWrite(22, LOW); //GND
  pinMode(21, OUTPUT); digitalWrite(21, LOW); //I1
  pinMode(20, OUTPUT); digitalWrite(20, LOW); //I2
  pinMode(17, OUTPUT); digitalWrite(17, HIGH); //3.3V

  pinMode(LED_BUILTIN, OUTPUT);

  //Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {}
  //Serial.println("Card failed, or not present");
  else  {
    //Serial.println("card initialized.");
    //Loop thru file names so we don't over-write older data
    int n = 0;
    char fileName[14];
    sprintf(fileName, "dataLog%02u.txt", n);
    while (SD.exists(fileName)) {
      n++;
      sprintf(fileName, "dataLog%02u.txt", n);
    }
    dataFile = SD.open(fileName, FILE_WRITE);
    String dataString = "Time,accelz";
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.flush();
      // print to the serial port too:
      //Serial.println(dataString);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      while (1) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
      }
      // if the file isn't open, pop up an error:
      //Serial.print("error opening ");Serial.println(fileName);
    }
  }

}

String dataString;

void loop() {
  // put your main code here, to run repeatedly:
  accel.read(); // Update acceleromter data
  dataString = String(micros()) + ", " + String(accel.z);
  //String dataString = String(micros()) + ", " + String(sqrt(accel.x^2+accel.y^2+accel.z^2)); //had holes in data
  //question: Would character arrays be faster than strings?
  //question: Would global variable be faster than local?

  dataFile.println(dataString);
  dataFile.flush(); //Note: Using .flush instead of .close to save time closing and reopining file every loop.
  //Serial.println(dataString);
}
