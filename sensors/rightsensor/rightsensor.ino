#include "RAK811.h"
#include "SoftwareSerial.h"
#define WORK_MODE LoRaWAN // LoRaWAN or LoRaP2P
#define JOIN_MODE ABP	  // OTAA or ABP
#define US915 8			  // See table in RAK811.h
#define PACKAGE_TYPE 1	  // See table in RAK811.h

String NwkSKey = NWKSKEYR
String AppSKey = APPSKEYR
String DevArrd = DEVADDRR


#define TXpin 11 // Set the virtual serial port pins
#define RXpin 10
#define DebugSerial Serial
SoftwareSerial ATSerial(RXpin, TXpin); // Declare a virtual serial port between RAK and Arduino

char buff[64] = "72616B776972656C657373";

// I can't believe this is necessary
char hex_char_lookup[17] = "0123456789ABCDEF";

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

	if (!RAKLoRa.rk_isConfirm(PACKAGE_TYPE)) //set LoRa data send package type:0->unconfirm, 1->confirm
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

// Hex representation of upper 4 bits of ascii representation of char
char upper_bits(const char ch)
{
	int bits = (ch & 0xF0) >> 4;
	return hex_char_lookup[bits];
}

// Hex reperesentatino of lower 4 bits of ascii representatino of char
char lower_bits(const char ch)
{
	int bits = ch & 0x0F;
	return hex_char_lookup[bits];
}

// Convert data in str to hex and store in buffer
void str_to_buffer(String str)
{
	// Clear buffer
	memset(buff, 0, 64);
	if (str.length() > 63 / 2)
		*buff = 0;
	for (unsigned i = 0; i < str.length(); ++i) {
		buff[2 * i] = upper_bits(str[i]);
		buff[2 * i + 1] = lower_bits(str[i]);
	}
}

void loop()
{
	DebugSerial.println("=====================");

	String data = "Left: 2m";
	str_to_buffer(data);

	DebugSerial.println(F("Sending data: "));
	DebugSerial.println((String)buff);
	RAKLoRa.rk_sendData(1, buff);
	String ret = RAKLoRa.rk_recvData();
	DebugSerial.println(ret);
	ret = RAKLoRa.rk_recvData();
	DebugSerial.println(ret);
	delay(500);


}