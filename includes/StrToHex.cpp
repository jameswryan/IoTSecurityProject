#ifndef STR_TO_HEX_CPP
#define STR_TO_HEX_CPP

#include "Arduino.h"
#include "StrToHex.h"
#include "WString.h"

const char upper_bits(const char ch)
{
	int bits = (ch & 0xF0) >> 4;
	return ASCII_HEX_LOOKUP[bits];
}

const char lower_bits(const char ch)
{
	int bits = ch & 0x0F;
	return ASCII_HEX_LOOKUP[bits];
}

void str_to_hex(const String str, char* buff)
{

	if (str.length() > (BUF_SIZE - 1) / 2) // if there are (BUF_SIZE-1)/2 characters in str, then we don't have enough space to null terminate buff
		return;

	// Set buff to zero
	memset(buff, 0, BUF_SIZE);

	// Convert to hex
	for (unsigned i = 0; i < str.length(); ++i) {
		buff[2 * i] = upper_bits(str[i]);
		buff[2 * i + 1] = lower_bits(str[i]);
	}
}
#endif