#include <Adafruit_GPS_NTR.h>		// edited Adafruit_GPS library
#include <SoftwareSerial_NTR.h>		// edited SoftwareSerial library
#include <SDI12_NTR.h>				// edited SDI12 library

SoftwareSerial_NTR gpsSerial(3, 2);
Adafruit_GPS_NTR GPS(&gpsSerial);

#define GPSECHO  true

//anemometer
#define DS2_PIN 8
SDI12_NTR ds2(DS2_PIN);
String myCommand = "0R0!"; //"0R1!"

String output;

void setup()  
{
  Serial.begin(57600);
  ds2.begin();
  GPS.begin(9600);
  
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);  
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  
  gpsSerial.println(PMTK_Q_RELEASE);
  

}

String readDS2(void)
// this could be cleaned up to query the device instead of using delay.
{
  ds2.sendCommand(myCommand);
  delay(250);          

  String sdi12string;
  while(ds2.available())
  {
    char i = ds2.read();
    if (i != '\n') {
      sdi12string += i;
    } 
  }
  ds2.flush();

  return sdi12string;
}

String makePrettyGps(void)
{
  char gpsTime[15];
  sprintf(gpsTime, "%02u:%02u:%02u.%03u", GPS.hour, GPS.minute, GPS.seconds, GPS.milliseconds);
  return String(gpsTime);
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
	GPS.read();
	if (GPS.newNMEAreceived()) {
		if(!GPS.parse(GPS.lastNMEA()))
			return;
		Serial.println(GPS.fix);
		//Serial.println(GPS.satellites);
		output = "GPS TIME: ";
		output += makePrettyGps();
		output += ',';
		output += "DS-2 READING: ";
		output += readDS2();
		Serial.println(output);
	}
}