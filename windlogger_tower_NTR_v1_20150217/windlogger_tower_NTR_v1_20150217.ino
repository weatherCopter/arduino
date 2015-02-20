//this is a test from small pc
#define COPTER  //set to TOWER for a tower; anything else will be interpreted as the copter
#define IDENTIFIER 0

// Date and time functions using RX8025 RTC connected via I2C and Wire lib

#include <Wire.h>

#include <DS3231.h>  //real-time clock
#include <SDI12.h>   //communication protocol for anemometer
#include <Fat16.h>
#include <Fat16util.h>

#include <L3G4200D.h>   //gyro
#include <ADXL345.h>    //accelerometer
#include <HMC5883L.h>   //compass
#include <Adafruit_BMP085.h>   //barometric pressure/altitude
#include <Sensirion.h>         //SHT-15

L3G4200D gyro;
ADXL345 accel; 
HMC5883L compass;
Adafruit_BMP085 bmp;

//sht75
const uint8_t SHT_DATA_PIN =  9;              // SHT serial data
const uint8_t SHT_CLK_PIN =  8;              // SHT serial clock
const uint32_t TRHSTEP   = 5000UL;       // Sensor query period
Sensirion sht = Sensirion(SHT_DATA_PIN, SHT_CLK_PIN);

//Linear Thermistors
int THERMISTOR_PIN1 = A1;
int THERMISTOR_PIN2 = A0;
double slope = -0.1302;    //Place calibrated thermistor slope here in degC/V
double offset = 89.113;        //Place calibrated thermistor offset here in degC

//Greg calculates:
//double slope = -0.143824;
//double offset = 95.7935;

//anemometer
#define DS2_PIN 3 
SDI12 mySDI12(DS2_PIN);
String myCommand = "0R0!";

//SD card
SdCard card;
Fat16 file;

//store error strings in flash to save RAM
#define error(s) error_P(PSTR(s))

void error_P(const char* str) 
{
  PgmPrint("error: ");
  SerialPrintln_P(str);
  if (card.errorCode) {
    PgmPrint("SD error: ");
    Serial.println(card.errorCode, HEX);
  }

//  while(1);
}
//comment test
//comment 2
//comment web


//real-time clock:
//year, month, date, hour, min, sec and week-day(starts from 0 and goes to 6)
//writing any non-existent time-data may interfere with normal operation of the RTC.
//Take care of week-day also.
DS3231 RTC; //Create the R8025 object
char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

void setup () 
{
    Serial.begin(57600);
    Wire.begin();
    RTC.begin();
    mySDI12.begin();

    gyro.enableDefault();
    accel.begin();
    compass = HMC5883L();
    compass.SetScale(1.3); // Set the scale of the compass.
    compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous 
    bmp.begin();

  // initialize the SD card
  if (!card.init()) error("card.init");
  
  // initialize a FAT16 volume
  if (!Fat16::init(&card)) error("Fat16::init");
}

String outputString;

//allows us to read the sensors every second, regardless of how much time we've spent doing other things
DateTime lastSensorRead;
boolean OneSecondHasPassed(void)
{
  DateTime rightNow = RTC.now();
  if(rightNow.get() >= lastSensorRead.get() + 1) //egads, who is the incompetent who wrote the RTC library?
  {
    lastSensorRead = rightNow;
    return true;
  }
  else return false;
}

void loop () 
{
  //receives serial input to set real-time clock
  while (Serial.available()) 
  {
    // look for the next valid integer in the incoming serial stream:
    int year = Serial.parseInt(); 
    // do it again:
    int month = Serial.parseInt(); 
    // do it again:
    int day = Serial.parseInt(); 
    // do it again:
    int hour = Serial.parseInt(); 
    // do it again:
    int min = Serial.parseInt(); 
    // do it again:
    int sec = Serial.parseInt(); 
    // do it again:
    int weekday = Serial.parseInt(); 
    
    // look for the newline. That's the end of your
    // sentence:
    if (Serial.read() == '\n') 
    {
      Serial.print("Setting time to: ");
      Serial.print(year);
      Serial.print("/");
      Serial.print(month);
      Serial.print("/");
      Serial.print(day);
      Serial.print(" ");
      Serial.print(hour);
      Serial.print(":");
      Serial.print(min);
      Serial.print(":");
      Serial.print(sec);
      Serial.print(" ");
      Serial.println(weekday);

      DateTime newTime(year, month, day, hour, min, sec, weekday);
      RTC.adjust(newTime);
      lastSensorRead = RTC.now();      
    }
  }

  if(OneSecondHasPassed()) ReadSensors();
}

int ReadSensors(void)
{
  mySDI12.sendCommand(myCommand);
  delay(250);           // wait a while for a response

#ifdef TOWER
  outputString = ("T");
  outputString += IDENTIFIER;
#else
  outputString = ("C0");
#endif

  outputString += (',');
  outputString += makePrettyTime();
  outputString += (',');

  String sdi12string;
  while(mySDI12.available())  //this should be made more fault tolerant -- hangs if \n is lost
  {
    char i = mySDI12.read();
    if (i != '\n') {
      sdi12string += i;
    } 
  }
  
  if(sdi12string.length() < 10)
  {
    error("DS-2 CONNECTION ERROR!");
  }

  outputString += sdi12string;
  outputString += ",";
  outputString += printAccel() + ",";
  outputString += printHeading();

#ifndef TOWER
  outputString += ",";
  outputString += printLT(THERMISTOR_PIN1) + ",";
  outputString += printLT(THERMISTOR_PIN2) + ",";
  outputString += printSHT() + ",";
  //outputString += printGyro() + ",";
  outputString += printBMP();
#endif

  outputString += '\n';

  //print to serial
  Serial.print(outputString);

  //and to a file
  char name[] = "datalog.csv";
  // clear write error
  file.writeError = false;
  
  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!file.open(name, O_CREAT | O_APPEND | O_WRITE))
    error("error opening file");  

  file.print(outputString);
    
  if (!file.close()) 
    error("error closing file");
    
  return 0;
}

String makePrettyTime(void)
{
  DateTime now = RTC.now(); //get the current date-time
  char timestamp[32];
  sprintf(timestamp, "%04d-%02d-%02d-%02d:%02d:%02d", now.year(), now.month(), now.date(), now.hour(), now.minute(), now.second());
  return String(timestamp);
}  

String Float2String(const float& ff)
{
  //ugh...float to String is a pain in Arduino World...
  char tempStr[32];
  dtostrf(ff, 4, 2, tempStr);

  return String(tempStr);
}

//linear thermistor
String printLT(int pin) 
{
  int inputValue = analogRead(pin);
  float tempInCelsius = inputValue * slope + offset;
  
  String retStr = Float2String(tempInCelsius) + "C";
  return retStr;
}

//sht75
String printSHT() 
{
  uint16_t rawData;
  
  sht.meas(TEMP, &rawData, BLOCK);        // Start temp measurement
  float temperature = sht.calcTemp(rawData);     // Convert raw sensor data
  
  sht.meas(HUMI, &rawData, BLOCK);      // Start humidity measurement
  float humidity = sht.calcHumi(rawData, temperature); // Convert raw sensor data
  
  float dewpoint = sht.calcDewpoint(humidity, temperature);
  
  String retStr = Float2String(temperature) + "C,";
  retStr += Float2String(humidity) + "%,";
  retStr += Float2String(dewpoint) + "C";
  
  return retStr;
}

// gryo values
String printGyro(void) 
{
  gyro.read();

  String retStr;
  retStr += String ((int)gyro.g.x) + ",";
  retStr += String ((int)gyro.g.y) + ",";
  retStr += String ((int)gyro.g.z);

  return retStr;
}

// accel values
String printAccel() {
  double xG, yG, zG, acc_data[3];
  accel.read(&acc_data[0], &acc_data[1], &acc_data[2]);
  
  float length = 0.;
  String retStr;
  
  for(int i = 0; i < 3; i++)
  {
    length += acc_data[i] * acc_data[i];
    retStr += Float2String(acc_data[i]) + ",";
  }
  
  length = sqrt(length);
  retStr += Float2String(length);
  
  return retStr;
}

// bmp values
String printBMP()
{
  String retStr;
  
  retStr += Float2String(bmp.readTemperature()) + "C,";
  retStr += Float2String(bmp.readPressure()) + ",";
  retStr += Float2String(bmp.readAltitude());

  return retStr;
}

// compass heading
String printHeading()
{
  // Retrive the raw values from the compass (not scaled).
  MagnetometerRaw raw = compass.ReadRawAxis();
  // Retrived the scaled values from the compass (scaled to the configured scale).
  MagnetometerScaled scaled = compass.ReadScaledAxis();
   
  // Values are accessed like so:
  int MilliGauss_OnThe_XAxis = scaled.XAxis;// (or YAxis, or ZAxis)
 
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(scaled.YAxis, scaled.XAxis);  
   
  float declinationAngle = 0.0457;
  heading += declinationAngle;
   
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
     
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
    
  // Convert radians to degrees for readability.
  heading = heading * 180/M_PI;

  String retStr = String((int)heading);  
  
  return retStr;
}
