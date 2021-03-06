#include "RAK811.h"
#include "SoftwareSerial.h"
#include "StrToHex.h"
#include "secrets.h"
#include

#define WORK_MODE LoRaWAN	   // LoRaWAN or LoRaP2P
#define JOIN_MODE ABP		   // OTAA or ABP
#define US915 8				   // See table in RAK811.h
#define PACKAGE_TYPE_CONFIRM 1 // See table in RAK811.h

String NwkSKey = NWKSKEYR;
String AppSKey = APPSKEYR;
String DevAddr = DEVADDRR;

#define TXpin 11 // Set the virtual serial port pins
#define RXpin 10

#define DebugSerial Serial
SoftwareSerial ATSerial(RXpin, TXpin); // Declare a virtual serial port between RAK and Arduino

bool InitLoRaWAN(void);
RAK811 RAKLoRa(ATSerial, DebugSerial);

void setup()
{

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

	DebugSerial.println("Setting work_mode...");
	if (!RAKLoRa.rk_setWorkingMode(WORK_MODE)) //set WisNode work_mode to LoRaWAN.
	{
		DebugSerial.println(F("set work_mode failed, please reset module."));
	}
	DebugSerial.println("work_mode set!");

	RAKLoRa.rk_getVersion();					//get RAK811 firmware version
	DebugSerial.println(RAKLoRa.rk_recvData()); //print version number

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

void loop()
{
	DebugSerial.println("=====================");

	String data = "Left: 2m";
	// rk_sendData requires data to be a hex string made of ASCII chars. WTF
	char buff[BUF_SIZE] = "";
	memset(buff, 0, BUF_SIZE);
	str_to_hex(data, buff);

	DebugSerial.println(F("Sending data: "));
	DebugSerial.println((String)buff);
	RAKLoRa.rk_sendData(1, buff);
	String ret = RAKLoRa.rk_recvData();
	DebugSerial.println(ret);
	ret = RAKLoRa.rk_recvData();
	DebugSerial.println(ret);
	delay(500);
}
