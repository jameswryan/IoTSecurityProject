/*
	Common defines that multiple files use that don't really belong in secrets.h
*/
#ifndef COMMON_DEFINES_IOT_SECURITY_PARKING
#define COMMON_DEFINES_IOT_SECURITY_PARKING

// LoRaWAN defines
#define WORK_MODE LoRaWAN	   // LoRaWAN or LoRaP2P
#define JOIN_MODE ABP		   // OTAA or ABP
#define US915 8				   // See table in RAK811.h
#define PACKAGE_TYPE_CONFIRM 1 // See table in RAK811.h
#define RAK_WAKEUP 0
#define RAK_SLEEP 1
#define TXpin 11			   // Set the virtual serial port pins
#define RXpin 10
#define AT_SERIAL_BAUD 9600


// Serial defines
#define DebugSerial Serial
#define DEBUG_SERIAL_BAUD 115200

// This allows us to only try and access the serial monitor in debug mode
#define __DEBUG_MODE__ 0
// This is probably not the best way to do this, but it works
#if __DEBUG_MODE__ == 1
#define debug_println(st) DebugSerial.println(st)
#else __DEBUG_MODE__ == 0
	#define debug_println(st)
#endif

#endif