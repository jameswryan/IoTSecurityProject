/*
	rk_sendData wants to send a string of hex data encoded as ASCII chars.
	These functions perform those conversions.
*/

#ifndef STR_TO_HEX
#define STR_TO_HEX
#include "Arduino.h"

// Size of buffer assumed by str_to_hex
#define BUF_SIZE 64

const static char ASCII_HEX_LOOKUP[17] = "0123456789ABCDEF";

// Hex representation of upper 4 bits of ascii representation of char
const char upper_bits(const char ch);

// Hex reperesentatino of lower 4 bits of ascii representation of char
const char lower_bits(const char ch);

// Convert data in str to hex, store in BUF_SIZE character buffer.
// If buffer is large enough, sets all elements to zero
// Undefined behaivior if buff is smaller than BUF_SIZE
void str_to_hex(const String str, char* buff);

#endif