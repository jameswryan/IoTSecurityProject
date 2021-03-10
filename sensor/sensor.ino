#include "RAK811.h"
#include "SoftwareSerial.h"
#include "StrToHex.h"
#include "Ultrasonic.h"
#include "cdefines.h"
#include "secrets.h"

// These are defined in secrets.h and are changed depending on which sensor is being programmed
String NwkSKey = NWKSKEYS;
String AppSKey = APPSKEYS;
String DevAddr = DEVADDRS;

SoftwareSerial ATSerial(RXpin, TXpin); // Declare a virtual serial port between RAK and Arduino

#define ULTRASONIC_SENSOR_PIN 52
Ultrasonic dist_sensor(ULTRASONIC_SENSOR_PIN);
long dist_ref = 0;

bool InitLoRaWAN(void);
RAK811 RAKLoRa(ATSerial, DebugSerial);

// Determine if measured distance is within sufficient range to count as vehicle.
bool within_range(const int);

void setup()
{
	#if __DEBUG_MODE__ == 1
	DebugSerial.begin(DEBUG_SERIAL_BAUD);
	while (DebugSerial.available()) {
		DebugSerial.read();
	}
	#endif
	delay(500); // Wait some time for everything to initialize
	debug_println("DebugSerial started!");

	debug_println("Starting ATSerial...");
	ATSerial.begin(9600); //set ATSerial baudrate:This baud rate has to be consistent with  the baud rate of the WisNode device.
	while (ATSerial.available()) {
		ATSerial.read();
	}
	debug_println("ATSerial started!");

	debug_println("Setting work_mode...");
	if (!RAKLoRa.rk_setWorkingMode(WORK_MODE)) //set WisNode work_mode to LoRaWAN.
	{
		debug_println(F("set work_mode failed, please reset module."));
	}
	debug_println("work_mode set!");

	RAKLoRa.rk_getVersion();			  //get RAK811 firmware version
	debug_println(RAKLoRa.rk_recvData()); //print version number

	debug_println(F("Start init RAK811 parameters..."));

	if (!InitLoRaWAN()) //init LoRaWAN
	{
		debug_println(F("Init error,please reset module."));
	}

	debug_println(F("Start to join LoRaWAN..."));
	while (!RAKLoRa.rk_joinLoRaNetwork(60)) //Joining LoRaNetwork timeout 60s
	{
		debug_println();
		debug_println(F("Rejoin again after 5s..."));
		delay(5000);
	}
	debug_println(F("Join LoRaWAN success"));

	if (!RAKLoRa.rk_isConfirm(PACKAGE_TYPE_CONFIRM)) //set LoRa data send package type:0->unconfirm, 1->confirm
	{
		debug_println(F("LoRa data send package set error,please reset module."));
	}

	// Put RAK in sleep mode by default
	RAKLoRa.rk_sleep(RAK_SLEEP);

	debug_println("Getting distance reference...");
	//dist_ref = dist_sensor.MeasureInCentimeters();
	dist_ref = 400;
	debug_println("Distance ref:");
	debug_println(dist_ref);

}

bool InitLoRaWAN(void)
{
	if (RAKLoRa.rk_setJoinMode(JOIN_MODE)) //set join_mode:ABP
	{
		if (RAKLoRa.rk_setRegion(US915)) //set region US915
		{
			if (RAKLoRa.rk_initABP(DevAddr, NwkSKey, AppSKey)) //set ABP mode parameters
			{
				debug_println(F("RAK811 init OK!"));
				return true;
			}
		}
	}
	return false;
}

int sgn = 1;
void loop()
{
	debug_println("=====================");

	// Get distance data
	//long dist = dist_sensor.MeasureInCentimeters();

	int ran = rand();
	int i =  (sgn) * constrain(ran, 0, 50);
	sgn = sgn * -1;

	long dist = 200 + i;

	// Determine if vehicle has entered
	if (true) {

		// Wake up RAK from sleep
		RAKLoRa.rk_sleep(RAK_WAKEUP);

		// Format data
		String data = (String)dist;
		debug_println(data);

		// rk_sendData requires data to be a hex string made of ASCII chars. WTF
		char buff[BUF_SIZE];
		memset(buff, 0, BUF_SIZE);
		str_to_hex(data, buff);

		debug_println(F("Sending data: "));
		debug_println((String)buff);
		RAKLoRa.rk_sendData(1, buff);

		String ret = RAKLoRa.rk_recvData();
		debug_println(ret);

		ret = RAKLoRa.rk_recvData();
		debug_println(ret);

		// Put RAK to sleep
		RAKLoRa.rk_sleep(RAK_SLEEP);
		delay(1000);
	}
}

bool within_range(const long dist)
{
	// Until it can be better tested, I set the threshold between 0.5 and 2.5 meters
	if (dist > 50 && dist < 250)
		return true;
	return false;
}