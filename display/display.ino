#include <StrToHex.h>
#include <cdefines.h>
#include <secrets.h>

#include "Adafruit_LiquidCrystal.h"
#include "RAK811.h"
#include "SoftwareSerial.h"
#include "Wire.h"

String NwkSKey = NWKSKEYD;
String AppSKey = APPSKEYD;
String DevAddr = DEVADDRD; // Like this is really really bad!!

#define LORA_CLASS LORA_CLASS_D // LoRa class for the display

SoftwareSerial ATSerial(RXpin, TXpin); // Declare a virtual serial port between RAK and Arduino

// initialize the library with the numbers of the interface pins
Adafruit_LiquidCrystal lcd(13, 12, 5, 4, 3, 2);

bool InitLoRaWAN(void);
RAK811 RAKLoRa(ATSerial, DebugSerial);

void setup()
{
	// set up the LCD's number of rows and columns:
	lcd.begin(16, 2);

	DebugSerial.begin(115200);
	while (DebugSerial.available()) {
		DebugSerial.read();
	}
	delay(500); // Wait some time for everything to be initialized
	DebugSerial.println("DebugSerial started!");

	DebugSerial.println("Starting ATSerial...");
	ATSerial.begin(9600); //set ATSerial baudrate:This baud rate has to be consistent with  the baud rate of the WisNode device.
	while (ATSerial.available()) {
		ATSerial.read();
	}
	DebugSerial.println("ATSerial started!");

	RAKLoRa.rk_getVersion();					//get RAK811 firmware version
	DebugSerial.println(RAKLoRa.rk_recvData()); //print version number

	DebugSerial.println("Setting class...");
	if (!RAKLoRa.rk_setClass(LORA_CLASS)) //set WisNode LoRa class to C.
	{
		DebugSerial.println(F("set class failed, please reset module."));
	}
	DebugSerial.println("class set!");

	DebugSerial.println("Setting work_mode...");
	if (!RAKLoRa.rk_setWorkingMode(WORK_MODE)) //set WisNode work_mode to LoRaWAN.
	{
		DebugSerial.println(F("set work_mode failed, please reset module."));
	}
	DebugSerial.println("work_mode set!");

	DebugSerial.println(F("Start init RAK811 parameters..."));

	if (!InitLoRaWAN()) //init LoRaWAN
	{
		DebugSerial.println(F("Init error,please reset module."));
	}

	DebugSerial.println(F("Start to join LoRaWAN..."));
	while (!RAKLoRa.rk_joinLoRaNetwork(60)) //Joining LoRaNetwork timeout 60s
	{
		DebugSerial.println();
		DebugSerial.println(F("Rejoin again after 5s..."));
		delay(5000);
	}
	DebugSerial.println(F("Join LoRaWAN success"));

	if (!RAKLoRa.rk_isConfirm(PACKAGE_TYPE_CONFIRM)) //set LoRa data send package type:0->unconfirm, 1->confirm
	{
		DebugSerial.println(F("LoRa data send package set error,please reset module."));
	}
}

bool InitLoRaWAN(void)
{
	if (RAKLoRa.rk_setJoinMode(JOIN_MODE)) //set join_mode:ABP
	{
		if (RAKLoRa.rk_setRegion(US915)) //set region US915
		{
			if (RAKLoRa.rk_initABP(DevAddr, NwkSKey, AppSKey)) //set ABP mode parameters
			{
				DebugSerial.println(F("RAK811 init OK!"));
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
	unsigned i = str.indexOf(':') + 1; // If ':' isn't present, indexOf returns -1
	if (i == 0)						   // If ':' isn't present, then there is no data
		return "";

	ret = str.substring(i, str.length());
	return ret;
}


void loop()
{
	String ret = RAKLoRa.rk_recvData();
	do { // Continue looking for downlink
		DebugSerial.println(ret);
		String data = strip_data(ret);
		ret = RAKLoRa.rk_recvData();
		DebugSerial.println(ret);
		if (data.length() < 2) // If only a few characters are sent, discard
			continue;
		if (data != "") {
			DebugSerial.println(data);
			char str[BUF_SIZE];
			hex_to_str(data, str);
			DebugSerial.println(str);
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print(str);
		} else
			DebugSerial.println("No data!");
	} while (ret != "");
	delay(1000);
}
