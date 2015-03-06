/*******************************************************************************
 * code to run a sensor network used on the towers for the WeatherCopter
 * capstone project.
 *
 * v1 - GPS only using Adafruit tutorial
 * v2 - GPS and DS2 now working. Edited Adafruit_GPS, SoftwareSerial, and SDI12
 * 		libraries to prevent ISR vector collision.
 * v3 - (20150305) GPS, DS2, and SD card.
 * v4 - (20150206) moved from single string storage to multi string storage plus
 * 		print to file and Serial internal to functions. release version.
 *
 * author: 	nathan tyler rose
 * date: 	20150305
 * board: 	Seeeduino Stalker v2.3
 * circuit:
 * sensor		pin
 * ----------------
 * GPS GND		GND
 * GPS 3v3		3v3	
 * GPS RX		2
 * GPS TX		3
 * ----------------
 * DS2 5v		5v
 * DS2 GND		GND
 * DS2 DATA		8
 * ----------------
 * SHT 3v3(2)	3v3
 * SHT GND(3)	GND
 * SHT CLK(1)	6
 * SHT DATA(4)	7
*******************************************************************************/
#define FILENAME "test08.csv"

#include <Adafruit_GPS_NTR.h>													//edited Adafruit_GPS library
#include <SoftwareSerial_NTR.h>													//edited SoftwareSerial library
#include <SDI12_NTR.h>															//edited SDI12 library
#include <SdFat.h>
#include <Sensirion.h>

//gps
#define GPSECHO  true
#define GPS_TX 3
#define GPS_RX 2
SoftwareSerial_NTR gpsSerial(GPS_TX, GPS_RX);
Adafruit_GPS_NTR gps(&gpsSerial);

//anemometer
#define DS2_PIN 8																//red wire
SDI12_NTR ds2(DS2_PIN);
String cmd = "0R0!"; 															//continuous measurement on address 0

//sd objects
SdFat sd;
SdFile file;

//sht75
const uint8_t SHT_DATA_PIN =  7;												//SHT serial data
const uint8_t SHT_CLK_PIN =  6;													//SHT serial clock
const uint32_t TRHSTEP   = 5000UL;												//sensor query period
Sensirion sht = Sensirion(SHT_DATA_PIN, SHT_CLK_PIN);

void setup()
{
	Serial.begin(57600);
	ds2.begin();
	gps.begin(9600);															//initialize GPS serial at 9600 baud.

	gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
	gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
	gps.sendCommand(PMTK_API_SET_FIX_CTL_1HZ);
	gps.sendCommand(PGCMD_ANTENNA);

	delay(1000);

	gpsSerial.println(PMTK_Q_RELEASE);

	if(!sd.begin(10, SPI_HALF_SPEED)) sd.initErrorHalt();
}

void loop()
{
//test plan:
	/* need to verify that the time is set and kept
		// cold start (inside)
			// upload code. unplug USB
			// wait 10 seconds plug USB back into PC and open serial
			// results: prints the same GPS time twice with different DS-2 values.
		// warm start (from inside to outside)
			// keep code running per cold start (inside) test plan.
			// results: prints one GPS reading and one DS-2 reading per second.
		// cold start (outside)
			// upload code. unplug USB
			// wait 10 seconds plug USB back into PC and open serial
			// results0: sets GPS time with one sat. prints one GPS reading and one DS-2 reading per second.
			// concerns: printing is less precise when the GPS does not have a fix.
			// results1: same as results0. GPS needs time to aquire fix.
		// warm start (from outside to inside)
			// with setup remaining from cold start (outside) walk inside.
			// verify fix is lost.
			// results: once fix is lost prints the same GPS time twice with different DS-2 values.
		// added return statement below to check again for lastNMEA
			// results: using cold start (inside) test method prints one GPS with one DS-2 reading.
			// 			hangs just as fix transitions to true. otherwise seems good.
			//			continues to work using warm start (from outside to inside) method.
		*/
	gps.read();
	if (gps.newNMEAreceived())
	{
		if(!gps.parse(gps.lastNMEA()))
			return;
		if(!file.open(FILENAME, O_RDWR | O_CREAT | O_AT_END)) sd.errorHalt();
		readGPS();
		readDS2();
		readSHT();
		file.close();
	}
}

void readSHT()
{
	uint16_t rawData;

	sht.meas(TEMP, &rawData, BLOCK);											//start temp measurement
	float temperature = sht.calcTemp(rawData); 									//convert raw sensor data

	sht.meas(HUMI, &rawData, BLOCK);      										//start humidity measurement
	float humidity = sht.calcHumi(rawData, temperature);						//convert raw sensor data

	float dewpoint = sht.calcDewpoint(humidity, temperature);

	String retStr;
	retStr += Float2String(temperature) + "C,";
	retStr += Float2String(humidity) + "%,";
	retStr += Float2String(dewpoint) + "C";

	file.print(retStr);
	Serial.print(retStr);
	file.print('\n');
	Serial.print('\n');
}

String Float2String(const float& ff)
{
	String retStr;
  	char tempStr[32];
  	dtostrf(ff, 4, 2, tempStr);
  	retStr = String(tempStr);
  	
  	return retStr;
}

void readDS2()
{
	ds2.sendCommand(cmd);
	delay(250);																	//clean this by using a query command?

	String retStr;
	
	while(ds2.available())
	{
		char i = ds2.read();
		if (i != '\r')
		{
			if(i != '\n')
			{
				retStr += i;
			}
		}
	}
	ds2.flush();

	file.print(retStr);
	Serial.print(retStr);
	file.print(',');
	Serial.print(',');
}

void readGPS()
{
	String retStr;
	char gpsTime[15];
	sprintf(gpsTime, "%02u:%02u:%02u.%03u", gps.hour, gps.minute, gps.seconds, gps.milliseconds);
																				//can remove milliseconds once code is tested.
	retStr = String(gpsTime);
	file.print(retStr);
	Serial.print(retStr);
	file.print(',');
	Serial.print(',');
}


