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
	Wire.beginTransmission(LOCAL_AUTH_ADDR>>1); // Set address to crypto chip
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

// Function to read device serial number or hardware rev
// "which" is 0 for serial number or 1 for hardware rev
// Returns 0 on error
uint16_t AUTH_ReadSerial(int which)
{
	uint16_t crc;
	unsigned char CommBuf[40];

	if (0 != which && 1 != which)
	{
		Serial.println("Bad selection for SN read");
		return(0);
	}

	// Wake up crypto chip
	WakeCrypto();

	// Build command to read slot 0
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_READ;
	CommBuf[2] = ATCA_ZONE_READWRITE_32 | ATCA_ZONE_DATA;
	CommBuf[3] = which << 3;
	CommBuf[4] = 0;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7))
	{
		Serial.println("Failed to send read slot command");
		return(0);
	}

	delay(62);  // Wait for exec

	// Get responce
	if (35 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,35))
	{
		Serial.println("Failed to get read slot responce");
		return(0);
	}

	// Get serial number from slot data, low byte first
	crc = CommBuf[1];
	crc |= CommBuf[2] << 8;

	return (crc);
}

// Function to write the serial number or hardware rev
// "which" is 0 for serial number or 1 for hardware rev
bool AUTH_WriteSerial(int which, uint16_t SN)
{
	uint16_t crc;
	uint8_t CommBuf[40] = {0};

	if (0 != which && 1 != which)
	{
		Serial.println("Bad selection for SN write");
		return(0);
	}

	// Wake up crypto chip
	WakeCrypto();

	// Build command to write slot 0
	CommBuf[0] = 39;
	CommBuf[1] = ATCA_WRITE;
	CommBuf[2] = ATCA_ZONE_READWRITE_32 | ATCA_ZONE_DATA;
	CommBuf[3] = which << 3;
	CommBuf[4] = 0;

	Serial.print("\r\nWrite serial number: ");

	// Store serial number low byte first
	CommBuf[5] = SN & 0xff;
	CommBuf[6] = SN >> 8;

	crc = GetCRC(39,CommBuf);
	CommBuf[37] = (unsigned char)(crc & 0x0ff);
	CommBuf[38] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,39))
	{
		Serial.println("Failed to send write slot command");
		return (false);
	}

	delay(10);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,4))
	{
		Serial.println("Failed to get write slot responce");
		return (false);
	}

	// Check CRC
	crc = GetCRC(4,CommBuf);
	if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println("CRC error on responce");
		return (false);
	}

	if (CommBuf[1])
	{
		Serial.print("Error in write slot: ");
		Serial.println((uint)CommBuf[1]);
		return (false);
	}

	Serial.println("Done");

	return (true);
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
		if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7)) {
			Serial.println(F("Failed to send read config command"));
			return(1);
		}

		delay(52);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,35))
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

// Locks a single data slot
// Returns false on error
bool AUTH_LockSlot(uint8_t SlotNum)
{
	uint16_t crc;
	uint8_t CommBuf[40];

	// Wake up crypto chip
	WakeCrypto();

	Serial.printf("Locking slot: %d  ... ",SlotNum);

	// Build command to lock slot
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_LOCK;
	CommBuf[2] = (SlotNum << 2) | 2;
	CommBuf[3] = 0;
	CommBuf[4] = 0;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7)) {
		Serial.println("Failed to send config lock command");
		return(false);
	}

	delay(35);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,4))
	{
		Serial.printf("Failed to lock slot %d\r\n",SlotNum);
		return(false);
	}

	// Check CRC
	crc = GetCRC(4,CommBuf);
	if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println("CRC Error in lock slot responce");
		return(false);
	}

	// Check result
	if (CommBuf[1])
	{
		Serial.println("Error in lock slot");
		return(false);
	}

	Serial.println("Locked");
	return (true);
}

void AUTH_Dump(void)
{
	// Wake up crypto chip
	WakeCrypto();

	// Dump config
	DumpCryptoConfig();
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
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7)) {
		Serial.println("Failed to send read config command");
		return(AUTH_NO_TYPE);
	}

	delay(52);  // Wait for exec

	// Get responce
	if (7 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7))
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
bool AUTH_GetLockBytes(uint8_t *Value, uint8_t *Config)
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
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7)) {
		Serial.println("Failed to send read config command");
		return(false);
	}

	delay(52);  // Wait for exec

	// Get responce
	if (7 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7))
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
