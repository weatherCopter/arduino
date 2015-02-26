// Date and time functions using RX8025 RTC connected via I2C and Wire lib

#include <Wire.h>

#include <DS3231.h>  //real-time clock

#include <Fat16.h>
#include <Fat16util.h>

#include <Sensirion.h>         //SHT-15
#include <SdCard.h>


//sht75
const uint8_t SHT75_DATA_COVER =  7;              // SHT serial data
const uint8_t SHT75_CLK_COVER =  6;              // SHT serial clock
const uint8_t SHT75_DATA_noCOVER =  2;              // SHT serial data
const uint8_t SHT75_CLK_noCOVER =  6;              // SHT serial clock
const uint8_t SHT15_DATA_COVER =  1;              // SHT serial data
const uint8_t SHT15_CLK_COVER =  6;              // SHT serial clock
const uint8_t SHT15_DATA_noCOVER =  3;              // SHT serial data
const uint8_t SHT15_CLK_noCOVER =  6;              // SHT serial clock
const uint32_t TRHSTEP   = 5000UL;       // Sensor query period
Sensirion sht75_cover = Sensirion(SHT75_DATA_COVER, SHT75_CLK_COVER);
Sensirion sht75_noCover = Sensirion(SHT75_DATA_noCOVER, SHT75_CLK_noCOVER);
Sensirion sht15_cover = Sensirion(SHT15_DATA_COVER, SHT15_CLK_COVER);
Sensirion sht15_noCover = Sensirion(SHT15_DATA_noCOVER, SHT15_CLK_noCOVER);

//Linear Thermistors
//int THERMISTOR_PIN1 = A1;
//int THERMISTOR_PIN2 = A0;
//double slope = -0.1302;    //Place calibrated thermistor slope here in degC/V
//double offset = 89.113;        //Place calibrated thermistor offset here in degC

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

  // initialize the SD card
  if (!card.init()) error("card.init");
  
  // initialize a FAT16 volume
  if (!Fat16::init(&card)) error("Fat16::init");
}

String outputString1;
String outputString2;
String outputString3;
String outputString4;
String prettyTime;

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
  
  if(OneSecondHasPassed()) ReadSensors();
}

int ReadSensors(void)
{
  //Do we need a delay?

  outputString1 = "75C,";
  outputString2 = "75,";
  outputString3 = "15C,";
  outputString4 = "15,";
  prettyTime = makePrettyTime();

  outputString1 += prettyTime;
  outputString1 += (',');
  outputString2 += prettyTime;
  outputString2 += (',');
  outputString3 += prettyTime;
  outputString3 += (',');
  outputString4 += prettyTime;
  outputString4 += (',');

  
  outputString1 += printSHT75C() + ",";
  outputString1 += '\n';
  outputString2 += printSHT75() + ",";
  outputString2 += '\n';
  outputString3 += printSHT15C() + ",";
  outputString3 += '\n';
  outputString4 += printSHT15() + ",";
  outputString4 += '\n';


  //print to serial
  Serial.print(outputString1);
  Serial.print(outputString2);
  Serial.print(outputString3);
  Serial.print(outputString4);

  //and to a file
  char name[] = "datalog.csv";
  // clear write error
  file.writeError = false;
  
  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!file.open(name, O_CREAT | O_APPEND | O_WRITE))
    error("error opening file");  

  file.print(outputString1);
  file.print(outputString2);
  file.print(outputString3);
  file.print(outputString4);
    
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

//sht75C
String printSHT75C() 
{
  uint16_t rawData;
  
  sht75_cover.meas(TEMP, &rawData, BLOCK);        // Start temp measurement
  float temperature = sht75_cover.calcTemp(rawData);     // Convert raw sensor data
  
  sht75_cover.meas(HUMI, &rawData, BLOCK);      // Start humidity measurement
  float humidity = sht75_cover.calcHumi(rawData, temperature); // Convert raw sensor data
  
  float dewpoint = sht75_cover.calcDewpoint(humidity, temperature);
  
  String retStr = Float2String(temperature) + "C,";
  retStr += Float2String(humidity) + "%,";
  retStr += Float2String(dewpoint) + "C";
  
  return retStr;
}
String printSHT75() 
{
  uint16_t rawData;
  
  sht75_noCover.meas(TEMP, &rawData, BLOCK);        // Start temp measurement
  float temperature = sht75_noCover.calcTemp(rawData);     // Convert raw sensor data
  
  sht75_noCover.meas(HUMI, &rawData, BLOCK);      // Start humidity measurement
  float humidity = sht75_noCover.calcHumi(rawData, temperature); // Convert raw sensor data
  
  float dewpoint = sht75_noCover.calcDewpoint(humidity, temperature);
  
  String retStr = Float2String(temperature) + "C,";
  retStr += Float2String(humidity) + "%,";
  retStr += Float2String(dewpoint) + "C";
  
  return retStr;
}
String printSHT15C() 
{
  uint16_t rawData;
  
  sht15_cover.meas(TEMP, &rawData, BLOCK);        // Start temp measurement
  float temperature = sht15_cover.calcTemp(rawData);     // Convert raw sensor data
  
  sht15_cover.meas(HUMI, &rawData, BLOCK);      // Start humidity measurement
  float humidity = sht15_cover.calcHumi(rawData, temperature); // Convert raw sensor data
  
  float dewpoint = sht15_cover.calcDewpoint(humidity, temperature);
  
  String retStr = Float2String(temperature) + "C,";
  retStr += Float2String(humidity) + "%,";
  retStr += Float2String(dewpoint) + "C";
  
  return retStr;
}
String printSHT15() 
{
  uint16_t rawData;
  
  sht15_noCover.meas(TEMP, &rawData, BLOCK);        // Start temp measurement
  float temperature = sht15_noCover.calcTemp(rawData);     // Convert raw sensor data
  
  sht15_noCover.meas(HUMI, &rawData, BLOCK);      // Start humidity measurement
  float humidity = sht15_noCover.calcHumi(rawData, temperature); // Convert raw sensor data
  
  float dewpoint = sht15_noCover.calcDewpoint(humidity, temperature);
  
  String retStr = Float2String(temperature) + "C,";
  retStr += Float2String(humidity) + "%,";
  retStr += Float2String(dewpoint) + "C";
  
  return retStr;
}
