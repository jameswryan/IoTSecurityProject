#include <StrToHex.h>
#include <cdefines.h>
#include <secrets.h>

#include "RAK811.h"
#include "SoftwareSerial.h"
#include "Ultrasonic.h"

/*
	We're joining with Activation By Personalization, which is not the preferred method of joining.
	Over The Air Activation would be better, since it reduces the number of necessary shared secrets
	and generates new NwkSKeys and AppSKeys each time the device joins.
	Using ABP, a malicious device could impersonate this device by capturing the join request sent to
	the gateway, and stealing the NwkSKey, AppSKey, and DevAddr.
	Were we using OTAA, the NwkSKey and AppSKey would be derived before joining by this device,
	which would prevent the attack mentioned above.
*/

// These are defined in secrets.h and are changed depending on which sensor is being programmed
String NwkSKey = NWKSKEYS;
String AppSKey = APPSKEYS;
String DevAddr = DEVADDRS;

SoftwareSerial ATSerial(RXpin, TXpin); // Declare a virtual serial port between RAK and Arduino

#define ULTRASONIC_SENSOR_PIN 52
Ultrasonic dist_sensor(ULTRASONIC_SENSOR_PIN);

bool InitLoRaWAN(void);
RAK811 RAKLoRa(ATSerial, DebugSerial);

// Determine if measured distance is within sufficient range to count as vehicle.
bool within_range(const long);

void setup()
{
#if __DEBUG_MODE__ == 1
	DebugSerial.begin(DEBUG_SERIAL_BAUD);
	while (DebugSerial.available()) {
		DebugSerial.read();
	}
	debug_println("DebugSerial started!");
#endif

	debug_println("Starting ATSerial...");
	ATSerial.begin(9600); //set ATSerial baudrate:This baud rate has to be consistent with the baud rate of the WisNode device.
	while (ATSerial.available()) {
		ATSerial.read();
	}
	delay(500); // Wait some time for everything to initialize
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
	//RAKLoRa.rk_sleep(RAK_SLEEP);

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

void loop()
{
	/*
		Throughout this project, we are keeping track of the frame count.
		This leaves our devices open to replay attacks, where a malicious device captures the data we
		send to the gateway, and just sends it again.
		We should really keep track of the up- and down- frame counts to prevent this.
		We also don't do much error handling, preferring to just ignore it and try again.
		This is more of an engineering problem than a security one, but it is important to mention.
	*/
	debug_println("=====================");

	// Get distance data
	long dist = dist_sensor.MeasureInCentimeters();
	debug_println("Distance: ");
	debug_println(dist);

	// Determine if vehicle has entered
	if (within_range(dist)) {

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

		/*
			We should really check for an error here. Since we're not, we don't know whether the
			data was recieved properly.
		*/

		// Put RAK to sleep
		RAKLoRa.rk_sleep(RAK_SLEEP);
		delay(1000);
	}
}

bool within_range(const long dist)
{
	// Until it can be better tested, I set the threshold between 1 and 3.8 meters.
	//The sensor is not reliable above 4 meters, and so I leave a bit of space
	if (dist > 150 && dist < 380)
		return true;
	return false;
}