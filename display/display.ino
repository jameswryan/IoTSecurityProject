#include <StrToHex.h>
#include <cdefines.h>
#include <secrets.h>

#include "Adafruit_LiquidCrystal.h"
#include "RAK811.h"
#include "SoftwareSerial.h"
#include "Wire.h"

/*
	See comment in sensor/sensor.ino about ABP vs OTAA.
*/

String NwkSKey = NWKSKEYD;
String AppSKey = APPSKEYD;
String DevAddr = DEVADDRD;

#define LORA_CLASS LORA_CLASS_C // LoRa class for the display

SoftwareSerial ATSerial(RXpin, TXpin); // Declare a virtual serial port between RAK and Arduino

// initialize the library with the numbers of the interface pins
Adafruit_LiquidCrystal lcd(13, 12, 5, 4, 3, 2);

bool InitLoRaWAN(void);
RAK811 RAKLoRa(ATSerial, DebugSerial);

void setup()
{
	// set up the LCD's number of rows and columns:
	lcd.begin(16, 2);
	lcd.print("Spots: ?/?");
#if __DEBUG_MODE__ == 1
	DebugSerial.begin(115200);
	while (DebugSerial.available()) {
		DebugSerial.read();
	}
	debug_println("DebugSerial started!");
#endif

	debug_println("Starting ATSerial...");
	ATSerial.begin(9600); //set ATSerial baudrate:This baud rate has to be consistent with  the baud rate of the WisNode device.
	while (ATSerial.available()) {
		ATSerial.read();
	}
	delay(500); // Wait some time for everything to be initialized
	debug_println("ATSerial started!");

	RAKLoRa.rk_getVersion();			  //get RAK811 firmware version
	debug_println(RAKLoRa.rk_recvData()); //print version number

	debug_println("Setting class...");
	if (!RAKLoRa.rk_setClass(LORA_CLASS)) //set WisNode LoRa class to C.
	{
		debug_println(F("set class failed, please reset module."));
	}
	debug_println("class set!");

	debug_println("Setting work_mode...");
	if (!RAKLoRa.rk_setWorkingMode(WORK_MODE)) //set WisNode work_mode to LoRaWAN.
	{
		debug_println(F("set work_mode failed, please reset module."));
	}
	debug_println("work_mode set!");

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

	// Send package to open windows
	RAKLoRa.rk_sendData(1, "1234");
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

// Get payload from string
String strip_data(const String& str)
{
	String ret;
	unsigned i = str.indexOf(':') + 1; // If ':' isn't present, indexOf returns -1. If it is
	if (i == 0)						   // If ':' isn't present, then there is no data
		return "";

	ret = str.substring(i, str.length());
	return ret;
}

void loop()
{
	/*
		There is no attempt made to check for errors, and whatever was sent is what is displayed.
		A better method would be to only accept integer data, and to store the text on the
		display. Additionally, there should be some checks that no errors were encountered.
		A simple check would be to look for the text 'ERROR', and wait 1 second before trying again
		if an error is encountered.
	*/

	String ret = RAKLoRa.rk_recvData();
	debug_println(ret);
	String data = strip_data(ret);
	//debug_println(data);
	if (data.length() > 2) { // If only a few characters are sent, discard
		debug_println(data);
		char str[BUF_SIZE];
		hex_to_str(data, str);
		debug_println(str);
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print(str);
	}

	delay(1000);
}
