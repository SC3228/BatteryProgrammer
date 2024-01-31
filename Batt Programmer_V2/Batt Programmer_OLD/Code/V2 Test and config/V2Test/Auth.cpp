// Function to read slot 8 of the auth chip into the buffer
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <SoftWire.h>	// I2C lib
extern SoftWire Wire;

#include "auth.h"
#include "BattProg.h"

uint8_t GotSerial = 0;
static uint16_t GetCRC(int len, uint8_t *buffer);
static void WakeCrypto(void);
static int ReadAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes);
static int WriteAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes);

#define CRYPTO_ADDR 0xC0  // I2C address of crypto chip

const char BAD_CONFIG[] = "Bad Config at ";

const uint8_t BP_Config508A[128] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
0xC0, 0x00, 0x55, 0x04, 0x0F, 0x0F, 0x0F, 0x0F,  0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,  0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF,  0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,  0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00,  0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00,
0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00,  0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00
};

const uint8_t BP_Config608A[128] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
0xC0, 0x00, 0x00, 0x04, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00,
0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00
};

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

	delay(51);
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

static int DumpCryptoConfig(void)
{
	uint16_t crc;
	int cnt, cnt2;
	uint8_t CommBuf[40];

	// Loop thru config blocks
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

		delay(52);  // Wait for exec

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
			Serial.printf("%02X ",CommBuf[cnt2]);

		Serial.println();
		for (cnt2 = 17; cnt2 <= 32; ++cnt2)
			Serial.printf("%02X ",CommBuf[cnt2]);

		Serial.println("");
	}

	return (0);
}

// Write 4/32 bytes to the config area
static int WriteConfig(uint8_t offset, uint8_t size, const uint8_t *src)
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
		CommBuf[cnt+5] = src[offset+cnt];

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

static int SetCryptoConfig(AuthChipType Chip)
{
	uint8_t cnt;
	const uint8_t *Config;

	// Set to correct config based on chip type
	switch (Chip) {
	case AUTH_508A:
		Config = BP_Config508A;
		break;
	case AUTH_608A:
		Config = BP_Config608A;
		break;
	default:
		Serial.println("Unknown chip type!!!");
		return (1);
		break;
	}

	Serial.print("Writting config area: ");

	// Do bytes 16-32
	for (cnt = 16; cnt < 32; cnt += 4)
		if (WriteConfig(cnt,0,Config))
			return (1);

	// Do bytes 32-63
	if (WriteConfig(32,1,Config))
		return (1);

	// Do bytes 64-83
	for (cnt = 64; cnt < 84; cnt += 4)
		if (WriteConfig(cnt,0,Config))
			return (1);
	
	// Do bytes 88-91
	if (WriteConfig(88,0,Config))
		return (1);

	// Do bytes 92-95
	if (WriteConfig(92,0,Config))
		return (1);

	// Do bytes 96-127
	if (WriteConfig(96,1,Config))
		return (1);

	Serial.print(Passed);
	return (0);
}

// Locks both the config and data locks
static int LockConfig(void)
{
	uint16_t crc;
	int cnt, cnt2;
	uint8_t CommBuf[40], crcbuf[128];

	Serial.print("Locking config & data areas: ");
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
			Serial.println("Failed to send read config command");
			return(1);
		}

		delay(52);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,35))
		{
			Serial.println("Failed to read config area");
			return(1);
		}

		// Check CRC
		crc = GetCRC(35,CommBuf);
		if (CommBuf[33] != (unsigned char)(crc & 0x0ff) || CommBuf[34] != (unsigned char)((crc >> 8) & 0x0ff))
		{
			Serial.println("Read config CRC error");
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
		Serial.println(F("Failed to send config lock command"));
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

	Serial.print(Passed);

	return (0);
}

// Check config memeory against config data
// Returns false on error
bool CheckCryptoConfig(int Locked,AuthChipType Chip)
{
	uint16_t crc;
	int cnt, cnt2;
	uint8_t CommBuf[40];
	const uint8_t *BP_Config;

	// Wake up crypto chip
	WakeCrypto();

	// Set to correct config based on chip type
	switch (Chip) {
	case AUTH_508A:
		BP_Config = BP_Config508A;
		break;
	case AUTH_608A:
		BP_Config = BP_Config608A;
		break;
	default:
		Serial.println("Unknown chip type!!!");
		return (false);
		break;
	}

	Serial.print("Checking config area: ");
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
			Serial.println("Failed to send read config command");
			return(false);
		}

		delay(52);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,35))
		{
			Serial.println("Failed to read config area");
			return(false);
		}

		// Check CRC
		crc = GetCRC(35,CommBuf);
		if (CommBuf[33] != (unsigned char)(crc & 0x0ff) || CommBuf[34] != (unsigned char)((crc >> 8) & 0x0ff))
		{
			Serial.println("CRC error");
			return(false);
		}

		switch (cnt)
		{
		case 0:
			// Check serial number and first 16 bytes
			if (AUTH_SN0 != CommBuf[1] || AUTH_SN1 != CommBuf[2] || AUTH_SN8 != CommBuf[13])
			{
				Serial.println("Bad SN");
				return(false);
			}

			for (cnt2 = 16; cnt2 < 32; ++cnt2)
				if (CommBuf[cnt2+1] != BP_Config[cnt2])
				{
					Serial.print(BAD_CONFIG);
					Serial.println(cnt2);
					return(false);
				}
			break;
		case 1:
			// Check 32 bytes
			for (cnt2 = 32; cnt2 < 64; ++cnt2)
				if (CommBuf[cnt2-31] != BP_Config[cnt2])
				{
					Serial.print(BAD_CONFIG);
					Serial.println(cnt2);
					return(false);
				}
			break;
		case 2:
			// Check 32 bytes
			for (cnt2 = 64; cnt2 < 96; ++cnt2)
				if (!Locked && (86 == cnt2 || 87 == cnt2))  // Do lock bytes manually for unlocked
				{
					if (0x55 != CommBuf[cnt2-63])
					{
						Serial.print(BAD_CONFIG);
						Serial.println(cnt2);
						return(false);
					}
				}
				else if (CommBuf[cnt2-63] != BP_Config[cnt2])
				{
					Serial.print(BAD_CONFIG);
					Serial.println(cnt2);
					return(false);
				}
			break;
		case 3:
			// Check last 32 bytes
			for (cnt2 = 96; cnt2 < 128; ++cnt2)
				if (CommBuf[cnt2-95] != BP_Config[cnt2])
				{
					Serial.print(BAD_CONFIG);
					Serial.println(cnt2);
					return(false);
				}
			break;
		}
	}

	Serial.print(Passed);
	return (true);
}

void AUTH_Dump(void)
{
	// Wake up crypto chip
	WakeCrypto();

	// Dump config
	DumpCryptoConfig();
}

// Configs auth chip
// Returns false on error
bool AUTH_Config(AuthChipType Chip)
{
	// Wake up crypto chip
	WakeCrypto();

	// Setup config
	Serial.print("Loading config: ");
	if (SetCryptoConfig(Chip))
		return (false);

	// Check for correct serial number/config and lock if OK
	if (!CheckCryptoConfig(0,Chip))
		return (false);

	Serial.println("Do lock!");
	if (LockConfig())
		return (false);

	if (!CheckCryptoConfig(1,Chip))
		return (false);

	return (true);
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

		if (BytesLeft >= 63)
			send = 63;
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

// Function to get the type of chip
AuthChipType GetChipType(void)
{
	uint16_t crc;
	uint8_t CommBuf[10];

	// Wake up crypto chip
	WakeCrypto();

	// Build command to read 4 bytes of config area at address 4
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_READ;
	CommBuf[2] = 0;
	CommBuf[3] = 1;
	CommBuf[4] = 0;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(CRYPTO_ADDR,CommBuf,7)) {
		Serial.println("Failed to send read config command");
		return(AUTH_NO_TYPE);
	}

	delay(52);  // Wait for exec

	// Get responce
	if (7 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,7))
	{
		Serial.println("Failed to read config area");
		return(AUTH_NO_TYPE);
	}

	// Check CRC
	crc = GetCRC(7,CommBuf);
	if (CommBuf[5] != (unsigned char)(crc & 0x0ff) || CommBuf[6] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println("CRC error");
		return(AUTH_NO_TYPE);
	}

	switch (CommBuf[3]) {
	case 0x50:
		return (AUTH_508A);
		break;
	case 0x60:
		return (AUTH_608A);
		break;
	}

	return (AUTH_NO_TYPE);	
}

// Function to get the lock byte values
// Returns false on an error
bool GetLockBytes(uint8_t *Value, uint8_t *Config)
{
	uint16_t crc;
	uint8_t CommBuf[10];

	// Wake up crypto chip
	WakeCrypto();

	// Build command to read 4 bytes of config area at address 84
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_READ;
	CommBuf[2] = 0;
	CommBuf[3] = (2 << 3) | 5;
	CommBuf[4] = 0;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(CRYPTO_ADDR,CommBuf,7)) {
		Serial.println("Failed to send read config command");
		return(false);
	}

	delay(52);  // Wait for exec

	// Get responce
	if (7 != ReadAuthBytes(CRYPTO_ADDR,CommBuf,7))
	{
		Serial.println("Failed to read config area");
		return(false);
	}

	// Check CRC
	crc = GetCRC(7,CommBuf);
	if (CommBuf[5] != (unsigned char)(crc & 0x0ff) || CommBuf[6] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println("CRC error");
		return(false);
	}

	*Value = CommBuf[3];
	*Config = CommBuf[4];

	return (true);	
}
