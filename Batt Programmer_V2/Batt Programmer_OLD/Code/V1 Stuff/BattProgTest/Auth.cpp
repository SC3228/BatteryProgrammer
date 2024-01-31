// Function to read slot 8 of the auth chip into the buffer
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <Arduino.h>
#include <Wire.h>	// I2C lib

#include "auth.h"
#include "BattProg.h"

uint8_t GotSerial = 0;
static uint16_t GetCRC(int len, uint8_t *buffer);
static void WakeCrypto(void);
static int ReadSlot(int ChipAddr, uint8_t slot, uint8_t *buf);
static int ReadAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes);
static int WriteAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes);

#define CRYPTO_ADDR 0xC0  // I2C address of crypto chip

PROGMEM const uint8_t g_config[128] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
0xC0, 0x00, 0x55, 0x04, 0x0F, 0x0F, 0x0F, 0x0F,  0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,  0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF,  0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x55, 0x00,  0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00,  0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00,
0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00,  0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00
};

void printfSerial(char *fmt, ... ){
        char buf[80]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 80, fmt, args);
        va_end (args);
        Serial.print(buf);
}

static void WakeCrypto(void)
{
	unsigned char buf[2] = {0};

	// Write idle command
	Wire.beginTransmission(CRYPTO_ADDR>>1); // Set address to crypto chip
	buf[0] = 2;
	Wire.write(buf,1);
	Wire.endTransmission(); // Do idle write, Don't check for error, might fail
	delay(2);

	// Write a "wake up" message
	Wire.beginTransmission(0); // Set address to 0
	buf[0] = 0;
	Wire.write(buf,1);
	Wire.endTransmission();  // Don't check for error, will always fail

	delayMicroseconds(900);
}

void AUTH_EnterSerial()
{
	uint16_t crc;
	int cnt, cnt2;
	uint8_t CommBuf[40];

	// Build command to write slot 0
	CommBuf[0] = 39;
	CommBuf[1] = ATCA_WRITE;
	CommBuf[2] = ATCA_ZONE_READWRITE_32 | ATCA_ZONE_DATA;
	CommBuf[3] = 0;
	CommBuf[4] = 0;

	// Get string
	Serial.println();
	Serial.print(F("Enter 'SN:###   HW:#.#' string: "));
	if (!GetString(CommBuf+5,31))
	{
		Serial.println(F("Write serial number canceled!"));
		return;
	}

	Serial.println(F("Start write serial number..."));
	WakeCrypto();
Serial.println(F("1..."));

	crc = GetCRC(39,CommBuf);
	CommBuf[37] = (unsigned char)(crc & 0x0ff);
	CommBuf[38] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(CRYPTO_ADDR,CommBuf,39))
	{
		Serial.println(F("Failed to send write slot command"));
		return;
	}

Serial.println(F("2..."));
	delay(10);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,4))
	{
		Serial.println(F("Failed to get write slot responce"));
		return;
	}
Serial.println(F("3..."));

	// Check CRC
	crc = GetCRC(4,CommBuf);
	if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error"));
		return;
	}
Serial.println(F("4..."));

	if (CommBuf[1])
	{
		Serial.println(F("Error in write slot"));
		return;
	}

	Serial.println(F("Write SN string done"));

	GotSerial = 1;

	return;
}

static uint16_t GetCRC(int len, unsigned char *buffer)
{
	uint8_t counter;
	uint16_t crc_register = 0;
	uint16_t polynom = 0x8005;
	uint8_t shift_register;
	uint8_t data_bit, crc_bit;
	uint8_t length = len - 2;

	for (counter = 0; counter < length; counter++)
	{
		for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1)
		{
			data_bit = (buffer[counter] & shift_register) ? 1 : 0;
			crc_bit = crc_register >> 15;
			crc_register <<= 1;
			if (data_bit != crc_bit)
				crc_register ^= polynom;
		}
	}

	return (crc_register);
}

static int ReadSlot(int ChipAddr, uint8_t slot, uint8_t *buf)
{
	uint16_t crc;
	int cnt, cnt2;
	unsigned char CommBuf[40];

	for (cnt = 0; cnt < (slot == 8 ? 13 : 3); ++cnt)
	{
		// Build command to read slot
		CommBuf[0] = 7;
		CommBuf[1] = ATCA_READ;
		CommBuf[2] = ATCA_ZONE_READWRITE_32 | ATCA_ZONE_DATA;
		CommBuf[3] = slot << 3;
		CommBuf[4] = cnt;

		crc = GetCRC(7,CommBuf);
		CommBuf[5] = (unsigned char)(crc & 0x0ff);
		CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

		// Send command
		if (WriteAuthBytes(ChipAddr,CommBuf,7)) {
			Serial.println(F("Failed to send read slot command"));
			return(1);
		}

		delayMicroseconds(1100);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(ChipAddr,CommBuf,35))
		{
			Serial.println(F("Failed to read slot"));
			return(1);
		}

		crc = GetCRC(35,CommBuf);

		if (CommBuf[33] != (unsigned char)(crc & 0x0ff) || CommBuf[34] != (unsigned char)((crc >> 8) & 0x0ff))
		{
			Serial.println(F("CRC error on slot read"));
			return(1);
		}

		for (cnt2 = 0; cnt2 < 32; ++cnt2)
			buf[cnt*32+cnt2] = CommBuf[cnt2+1];
	}

	return (0);
}

static int DumpCryptoConfig(void)
{
	uint16_t crc;
	int cnt, cnt2;
	uint8_t CommBuf[40];

	// Do bytes 16-19
	for (cnt = 0; cnt < 4; ++cnt)
	{
		// Build command to read 32 bytes of config area
		CommBuf[0] = 7;
		CommBuf[1] = ATCA_READ;
		CommBuf[2] = ATCA_ZONE_READWRITE_32;
		CommBuf[3] = cnt << 3;
		CommBuf[4] = 0;

		crc = GetCRC(7,CommBuf);
		CommBuf[5] = (unsigned char)(crc & 0x0ff);
		CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

		// Send command
		if (WriteAuthBytes(CRYPTO_ADDR,CommBuf,7)) {
			Serial.println(F("Failed to send read config command"));
			return(1);
		}

		delayMicroseconds(1100);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,35))
		{
			Serial.println(F("Failed to read config area"));
			return(1);
		}

		// Check CRC
		crc = GetCRC(35,CommBuf);
		if (CommBuf[33] != (unsigned char)(crc & 0x0ff) || CommBuf[34] != (unsigned char)((crc >> 8) & 0x0ff))
		{
			Serial.println(F("CRC error"));
			return(1);
		}

		for (cnt2 = 1; cnt2 <= 16; ++cnt2)
		{
			printfSerial("%02X ",CommBuf[cnt2]);
		}
		Serial.println();
		for (cnt2 = 17; cnt2 <= 32; ++cnt2)
		{
			printfSerial("%02X ",CommBuf[cnt2]);
		}
		Serial.println("");
	}

	// Build command to read slot0
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_READ;
	CommBuf[2] = ATCA_ZONE_READWRITE_32 | ATCA_ZONE_DATA;
	CommBuf[3] = 0;
	CommBuf[4] = 0;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(CRYPTO_ADDR,CommBuf,7)) {
		Serial.println(F("Failed to send read slot command"));
		return(1);
	}

	delayMicroseconds(1100);  // Wait for exec

	// Get responce
	if (35 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,35))
	{
		Serial.println(F("Failed to read slot"));
		return(1);
	}

	crc = GetCRC(35,CommBuf);

	if (CommBuf[33] != (unsigned char)(crc & 0x0ff) || CommBuf[34] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error on slot read"));
		return(1);
	}

	CommBuf[32] = 0; // Terminate just in case
	Serial.println((char *)(CommBuf+1));

	return (0);
}

// Write 4 bytes to the config area
static WriteConfig(uint8_t offset, uint8_t size)
{
	uint16_t crc;
	int cnt;
	uint8_t CommBuf[40];

	// Do bytes starting at offset
	CommBuf[0] = (size ? 39 : 11);
	CommBuf[1] = ATCA_WRITE;
	CommBuf[2] = ATCA_ZONE_CONFIG | (size ? ATCA_ZONE_READWRITE_32 : 0);
	CommBuf[3] = offset >> 2;
	CommBuf[4] = 0;

	for (cnt = 0; cnt < (size ? 32 : 4); ++cnt)
		CommBuf[cnt+5] = pgm_read_byte_far(g_config + offset + cnt);

	crc = GetCRC((size ? 39 : 11),CommBuf);
	CommBuf[(size ? 37 : 9)] = (unsigned char)(crc & 0x0ff);
	CommBuf[(size ? 38 : 10)] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(CRYPTO_ADDR,CommBuf,(size ? 39 : 11))) {
		Serial.println(F("Failed to send write config command"));
		return(1);
	}

	delay(30);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,4))
	{
		Serial.println(F("Failed to get write responce"));
		return(1);
	}

	// Check CRC
	crc = GetCRC(4,CommBuf);
	if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error"));
		return(1);
	}

	if (CommBuf[1])
	{
		Serial.print(F("Error in write offset: "));
		Serial.print(offset);
		Serial.print(F(" size: "));
		Serial.println((size ? 32 : 4));
		return(1);
	}
	
	return (0);
}

static int SetCryptoConfig(void)
{
	uint8_t cnt;

	Serial.print(F("Writting config area: "));

	// Do bytes 16-32
	for (cnt = 16; cnt < 32; cnt += 4)
		if (WriteConfig(cnt,0))
			return (1);

	// Do bytes 32-63
	if (WriteConfig(32,1))
		return (1);

	// Do bytes 64-83
	for (cnt = 64; cnt < 84; cnt += 4)
		if (WriteConfig(cnt,0))
			return (1);
	
	// Do bytes 88-91
	if (WriteConfig(88,0))
		return (1);

	// Do bytes 92-95
	if (WriteConfig(92,0))
		return (1);

	// Do bytes 96-127
	if (WriteConfig(96,1))
		return (1);

	Serial.println(F("Passed"));
	return (0);
}

static int LockConfig(void)
{
	uint16_t crc;
	int cnt, cnt2;
	uint8_t CommBuf[40], crcbuf[128];

	Serial.print(F("Locking config area: "));
	for (cnt = 0; cnt < 4; ++cnt)
	{
		// Build command to read 32 bytes of config area
		CommBuf[0] = 7;
		CommBuf[1] = ATCA_READ;
		CommBuf[2] = ATCA_ZONE_READWRITE_32;
		CommBuf[3] = cnt << 3;
		CommBuf[4] = 0;

		crc = GetCRC(7,CommBuf);
		CommBuf[5] = (unsigned char)(crc & 0x0ff);
		CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

		// Send command
		if (WriteAuthBytes(CRYPTO_ADDR,CommBuf,7)) {
			Serial.println(F("Failed to send read config command"));
			return(1);
		}

		delayMicroseconds(1100);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,35))
		{
			Serial.println(F("Failed to read config area"));
			return(1);
		}

		// Check CRC
		crc = GetCRC(35,CommBuf);
		if (CommBuf[33] != (unsigned char)(crc & 0x0ff) || CommBuf[34] != (unsigned char)((crc >> 8) & 0x0ff))
		{
			Serial.println(F("Read config CRC error"));
			return(1);
		}

		// Copy over data
		for (cnt2 = 0; cnt2 < 32; ++cnt2)
			crcbuf[cnt * 32 + cnt2] = CommBuf[cnt2+1];
	}

	// Get CRC of config area
	crc = GetCRC(130,crcbuf);

	// Build command to lock config area
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_LOCK;
	CommBuf[2] = 0;
	CommBuf[3] = crc & 0xff;
	CommBuf[4] = crc >> 8;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(CRYPTO_ADDR,CommBuf,7)) {
		Serial.println(F("Failed to send lock command"));
		return(1);
	}

	delay(35);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,4))
	{
		Serial.println(F("Failed to lock config area"));
		return(1);
	}

	// Check CRC
	crc = GetCRC(4,CommBuf);
	if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC Error in lock config responce"));
		return(1);
	}

	// Check result
	if (CommBuf[1])
	{
		Serial.println(F("Error in lock config"));
		return(1);
	}

	// Build command to lock data area
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_LOCK;
	CommBuf[2] = 0x81;
	CommBuf[3] = 0;
	CommBuf[4] = 0;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(CRYPTO_ADDR,CommBuf,7)) {
		Serial.println(F("Failed to send lock data command"));
		return(1);
	}

	delay(35);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,4))
	{
		Serial.println(F("Failed to lock data area"));
		return(1);
	}

	// Check CRC
	crc = GetCRC(4,CommBuf);
	if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC Error in lock data responce"));
		return(1);
	}

	// Check result
	if (CommBuf[1])
	{
		Serial.println(F("Error in lock data"));
		return(1);
	}

	Serial.println(F("Sucsess!"));

	return (0);
}

static int CheckCryptoConfig(void)
{
	uint16_t crc;
	int cnt, cnt2;
	uint8_t CommBuf[40];

	Serial.print(F("Checking config area: "));
	for (cnt = 0; cnt < 4; ++cnt)
	{
		// Build command to read 32 bytes of config area
		CommBuf[0] = 7;
		CommBuf[1] = ATCA_READ;
		CommBuf[2] = ATCA_ZONE_READWRITE_32;
		CommBuf[3] = cnt << 3;
		CommBuf[4] = 0;

		crc = GetCRC(7,CommBuf);
		CommBuf[5] = (unsigned char)(crc & 0x0ff);
		CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

		// Send command
		if (WriteAuthBytes(CRYPTO_ADDR,CommBuf,7)) {
			Serial.println(F("Failed to send read config command"));
			return(1);
		}

		delayMicroseconds(1100);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,35))
		{
			Serial.println(F("Failed to read config area"));
			return(1);
		}

		// Check CRC
		crc = GetCRC(35,CommBuf);
		if (CommBuf[33] != (unsigned char)(crc & 0x0ff) || CommBuf[34] != (unsigned char)((crc >> 8) & 0x0ff))
		{
			Serial.println(F("CRC error"));
			return(1);
		}


		switch (cnt)
		{
		case 0:
			// Check serial number and first 16 bytes
			if (AUTH_SN0 != CommBuf[1] || AUTH_SN1 != CommBuf[2] || AUTH_SN8 != CommBuf[13])
			{
				Serial.println(F("Bad SN"));
				return(1);
			}

			for (cnt2 = 16; cnt2 < 32; ++cnt2)
				if (CommBuf[cnt2+1] != pgm_read_byte_far(g_config + cnt2))
				{
					Serial.print(F("Bad Config at "));
					Serial.println(cnt2);
					return(1);
				}
			break;
		case 1:
			// Check next 32 bytes
			for (cnt2 = 32; cnt2 < 64; ++cnt2)
				if (CommBuf[cnt2-31] != pgm_read_byte_far(g_config + cnt2))
				{
					Serial.print(F("Bad Config at "));
					Serial.println(cnt2);
					return(1);
				}
			break;
		case 2:
			// Check next 32 bytes, skipping a few
			for (cnt2 = 64; cnt2 < 84; ++cnt2)
				if (CommBuf[cnt2-63] != pgm_read_byte_far(g_config + cnt2))
				{
					Serial.print(F("Bad Config at "));
					Serial.println(cnt2);
					return(1);
				}
			for (cnt2 = 92; cnt2 < 96; ++cnt2)
				if (CommBuf[cnt2-63] != pgm_read_byte_far(g_config + cnt2))
				{
					Serial.print(F("Bad Config at "));
					Serial.println(cnt2);
					return(1);
				}
			break;
		case 3:
			// Check last 32 bytes
			for (cnt2 = 96; cnt2 < 128; ++cnt2)
				if (CommBuf[cnt2-95] != pgm_read_byte_far(g_config + cnt2))
				{
					Serial.print(F("Bad Config at "));
					Serial.println(cnt2);
					return(1);
				}
			break;
		}
	}

	Serial.println(F("Passed"));
	return (0);
}

void AUTH_Dump(void)
{
	// Wake up crypto chip
	WakeCrypto();

	// Dump config
	DumpCryptoConfig();
}

void AUTH_Read(void)
{
	// Wake up crypto chip
	WakeCrypto();

	// Check for correct serial number/config
	CheckCryptoConfig();
}

void AUTH_Config(void)
{
	if (!GotSerial)
	{
		Serial.print(F("Must enter serial number first!!!"));
		return;
	}

	// Wake up crypto chip
	WakeCrypto();

	// Setup config
	if (SetCryptoConfig())
		return;

	// Check for correct serial number/config and lock if OK
	if (!CheckCryptoConfig())
		LockConfig();
}

// Routine to read multiple bytes from an auth chip
// Returns count of bytes left
static int ReadAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes)
{
	uint8_t cnt, cnt2 = 0, BytesLeft = Bytes;
	int get, got;

	while (BytesLeft)
	{
		if (BytesLeft >= 32)
			get = 32;
		else
			get = BytesLeft;

		// Do read
		got = Wire.requestFrom(ChipAddr >> 1,get);

		// Move over data
		for (cnt = 0; cnt < got; ++cnt)
			if (Wire.available())
			{
				Buffer[cnt+cnt2] = Wire.read();
				--BytesLeft;
				--get;
			}
			else
				return(Bytes - BytesLeft);

		if (get) // Did we get it all?
			return(Bytes - BytesLeft);  // Nope

		cnt2 += 32;
	}

	return(Bytes - BytesLeft);  // Return count of bytes read
}

// Routine to write multiple bytes to the auth chip
// Returns pass/fail
int WriteAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes)
{
	uint8_t BytesLeft = Bytes;
	int send, sent = 0;

	while (BytesLeft)
	{
		// Start transmission
		Wire.beginTransmission(ChipAddr >> 1); // Set address to crypto chip

		if (BytesLeft >= 31)
			send = 31;
		else
			send = BytesLeft;

		Wire.write(3);
		Wire.write(Buffer+sent,send);
		sent += send;
		BytesLeft -= send;
		if (Wire.endTransmission())
			return(1);
	}

	return (0);
}

