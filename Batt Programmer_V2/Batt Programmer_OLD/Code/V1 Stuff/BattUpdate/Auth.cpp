// Battery programmer auth chip functions

#include <Arduino.h>
#include <Wire.h>	// I2C lib

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "pwmbatt.h"
#include "BattProg.h"
#include "Auth.h"

// Function prototypes
static uint16_t GetCRC(int len, uint8_t *buffer);
static void WakeCrypto(void);
static int ReadSlot(uint8_t ChipAddr, uint8_t slot, uint8_t *buf);
static int WriteSlot(uint8_t ChipAddr, uint8_t slot, uint8_t *buf);
static int ReadAuthBytes(uint8_t ChipAddr, uint8_t Buffer[], uint8_t Bytes);
static int WriteAuthBytes(uint8_t ChipAddr, uint8_t Buffer[], uint8_t Bytes);

// Slot end addresses
#define SLOT8_END    415 // Slot 8 end
#define SLOT13_END   487 // Slot 13 end
#define SLOT14_END   559 // Slot 14 end
#define SLOT15_END   631 // Slot 15 end

uint8_t AUTH_Read(uint8_t *buffer = NULL)
{
	uint8_t *data;
	int cnt, ret;
	uint8_t inch;

	if (!BatteryType)  // Check for valid battery type
		return(1);

	FlushSerial();
	if (NULL == buffer)  // Check for which buffer
	{
		data = EEPROM_Data;
		Serial.print(F("Read "));
		Serial.print((__FlashStringHelper*)Batts[BatteryType].Name);
		Serial.println(F(" Auth chip into buffer (y/n)?"));
		while(!Serial.available())
		;
		inch = Serial.read();
		if (inch != 'y' && inch != 'Y')
			return(1);
	}
	else
		data = buffer;

	// Wake up crypto chips
	WakeCrypto();

	// Read data from slot 8
	Serial.print(F("Reading slot8 data: "));
	if (ret = ReadSlot(Batts[BatteryType].CryptoAddress,8,data))
	{
		Serial.print(F("Bad slot8 read: "));
		Serial.println(ret);
		return (1);
	}
	Serial.println(" OK");

	if (Batts[BatteryType].Size > 416)
	{
		// Read data from slot 13
		Serial.print(F("Reading slot13 data: "));
		if (ret = ReadSlot(Batts[BatteryType].CryptoAddress,13,data+SLOT8_END+1))
		{
			Serial.print(F("Bad slot13 read: "));
			Serial.println(ret);
			return (1);
		}
		Serial.println(" OK");

		// Read data from slot 14
		Serial.print(F("Reading slot14 data: "));
		if (ret = ReadSlot(Batts[BatteryType].CryptoAddress,14,data+SLOT13_END+1))
		{
			Serial.print(F("Bad slot14 read: "));
			Serial.println(ret);
			return (1);
		}
		Serial.println(" OK");

		// Read data from slot 15
		Serial.print(F("Reading slot15 data: "));
		if (ret = ReadSlot(Batts[BatteryType].CryptoAddress,15,data+SLOT14_END+1))
		{
			Serial.print(F("Bad slot15 read: "));
			Serial.println(ret);
			return (1);
		}
		Serial.println(" OK");
	}
	
	Serial.println(F("Read OK!"));
	BufferBlank = 0;

	return (0);
}

void AUTH_Verify(void)
{
	int ret;
	int cnt;
	int size;
	uint8_t inch;
	uint8_t SlotBuf[632];

	if (!BatteryType)  // Check for valid battery type
		return;

	FlushSerial();

	// Wake up crypto chips
	WakeCrypto();

	// Read data from slot 8
	Serial.print(F("Reading slot8 data: "));
	if (ReadSlot(Batts[BatteryType].CryptoAddress,8,SlotBuf))
	{
		Serial.print(F("Bad slot8 read: "));
		Serial.println(ret);
		return;
	}

	Serial.println(F("OK"));

	size = Batts[BatteryType].Size;

	if (size > 416)
	{
		// Read data from slot 13
		Serial.print(F("Reading slot13 data: "));
		if (ReadSlot(Batts[BatteryType].CryptoAddress,13,SlotBuf+416))
		{
			Serial.print(F("Bad slot13 read: "));
			Serial.println(ret);
			return;
		}

		Serial.println(F("OK"));

		// Read data from slot 14
		Serial.print(F("Reading slot14 data: "));
		if (ReadSlot(Batts[BatteryType].CryptoAddress,14,SlotBuf+416+72))
		{
			Serial.print(F("Bad slot14 read: "));
			Serial.println(ret);
			return;
		}

		Serial.println(F("OK"));

		// Read data from slot 15
		Serial.print(F("Reading slot15 data: "));
		if (ReadSlot(Batts[BatteryType].CryptoAddress,15,SlotBuf+416+144))
		{
			Serial.print(F("Bad slot15 read: "));
			Serial.println(ret);
			return;
		}

		Serial.println(F("OK"));
	}

	// Do verify
	Serial.print(F("Checking: "));
	for (cnt = 0; cnt < size; ++cnt)
		if (SlotBuf[cnt] != EEPROM_Data[cnt])
		{
			Serial.print(F("Error at: "));
			Serial.println(cnt);
			return;
		}
	Serial.print(F("OK"));
}

void AUTH_Write(uint8_t *buffer = NULL)
{
	int ret, size;
	int cnt;
	uint8_t inch;
	uint8_t *data;

	if (NULL == buffer)  // Check for which buffer
		data = EEPROM_Data;
	else
		data = buffer;

	if (!BatteryType)  // Check for valid battery type
		return;

	FlushSerial();
	Serial.print(F("Overwrite "));
	Serial.print((__FlashStringHelper*)Batts[BatteryType].Name);
	Serial.println(F(" (y/n)?"));
	while(!Serial.available())
	;
	inch = Serial.read();
	if (inch != 'y' && inch != 'Y')
		return;

	// Wake up crypto chips
	WakeCrypto();

	// Write data to slot 8
	Serial.print(F("Writing slot8 data: "));
	if (WriteSlot(Batts[BatteryType].CryptoAddress,8,data))
		return;
	Serial.println(F("OK"));

	size = Batts[BatteryType].Size;

	if (size > 416)
	{
		Serial.print(F("Writing slot13 data: "));
		if (WriteSlot(Batts[BatteryType].CryptoAddress,13,data+SLOT8_END+1))
			return;
		Serial.println(F("OK"));

		Serial.print(F("Writing slot14 data: "));
		if (WriteSlot(Batts[BatteryType].CryptoAddress,14,data+SLOT13_END+1))
			return;
		Serial.println(F("OK"));

		Serial.print(F("Writing slot15 data: "));
		if (WriteSlot(Batts[BatteryType].CryptoAddress,15,data+SLOT14_END+1))
			return;
		Serial.println(F("OK"));

	}

	Serial.println(F("Auth write OK!"));
}

static int ReadSlot(uint8_t ChipAddr, uint8_t slot, uint8_t *buf)
{
	uint16_t crc;
	int cnt, cnt2;
	unsigned char CommBuf[40];
	int size;

	// Set slot size
	if (slot >= 0 && slot <= 7)
		size = 36;
	else if (8 == slot)
		size = 416;
	else
		size = 72;

	// loop thru data transfers
	cnt = 0;
	while (size > 0)
	{
		// Build command to read slot
		CommBuf[0] = 7;
		CommBuf[1] = ATCA_READ;
		CommBuf[2] = ATCA_ZONE_READWRITE_32 | ATCA_ZONE_DATA;
		CommBuf[3] = slot << 3;
		CommBuf[4] = cnt >> 5;

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
			Serial.println(F("Failed to get read slot responce"));
			return(1);
		}

		for (cnt2 = 0; cnt2 < (size>=32 ? 32 : size); ++cnt2)
			buf[cnt+cnt2] = CommBuf[cnt2+1];

		cnt += 32;
		size -= 32;
	}

	return (0);
}

static int WriteSlot(uint8_t ChipAddr, uint8_t slot, uint8_t *buf)
{
	uint16_t crc;
	int cnt, cnt2;
	unsigned char CommBuf[40];
	int size;

	// Set slot size
	if (slot >= 0 && slot <= 7)
		size = 36;
	else if (8 == slot)
		size = 416;
	else
		size = 72;

	// loop thru data transfers
	cnt = 0;
	while (size > 0)
	{
		// Build command to write slot
		CommBuf[0] = 39;
		CommBuf[1] = ATCA_WRITE;
		CommBuf[2] = ATCA_ZONE_READWRITE_32 | ATCA_ZONE_DATA;
		CommBuf[3] = slot << 3;
		CommBuf[4] = cnt >> 5;

		// Copy over data
		for (cnt2 = 0; cnt2 < 32; ++cnt2)
			CommBuf[5+cnt2] = buf[cnt2+cnt];

		crc = GetCRC(39,CommBuf);
		CommBuf[37] = (unsigned char)(crc & 0x0ff);
		CommBuf[38] = (unsigned char)((crc >> 8) & 0x0ff);

		// Send command
		if (WriteAuthBytes(ChipAddr,CommBuf,39)) {
			Serial.println(F("Failed to send write slot command"));
			return(1);
		}

		delay(30);  // Wait for exec

		// Get responce
		if (4 != ReadAuthBytes(ChipAddr,CommBuf,4))
		{
			Serial.println(F("Failed to get write slot responce"));
			return(1);
		}

		if (CommBuf[1]) {
			Serial.println(F("Error in write slot"));
			return (1);
		}

		cnt += 32;
		size -= 32;
	}

	return (0);
}

static void WakeCrypto(void)
{
	unsigned char buf[2] = {0};

	if (CheckClk()) // Check for clock not "stuck"
		return;

	// Write idle command to local chip
	Wire.beginTransmission(LOCAL_AUTH_ADDR>>1); // Set address to crypto chip
	buf[0] = 2;
	Wire.write(buf,1);
	Wire.endTransmission(); // Do idle write, Don't check for error, might fail
	delay(2);

	// Write idle command to battery chip
	Wire.beginTransmission(Batts[BatteryType].CryptoAddress>>1); // Set address to crypto chip
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

static uint16_t GetCRC(int len, uint8_t *buffer)
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

// Routine to read multiple bytes from an auth chip
// Returns count of bytes left
static int ReadAuthBytes(uint8_t ChipAddr, uint8_t Buffer[], uint8_t Bytes)
{
	uint8_t cnt, cnt2, BytesLeft, size;
	int get, got;
	uint16_t crc;

	if (CheckClk()) // Check for clock not "stuck"
		return;

	// get count
	got = Wire.requestFrom(ChipAddr >> 1,1);
	if (got < 1 || !Wire.available())
	{
		Serial.println(F("Error getting packet size on read"));
		return (0);
	}
	size = Wire.read();
	Buffer[0] = size;
	cnt2 = 1;
	BytesLeft = size;
	--BytesLeft;

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
				if ((cnt+cnt2)>=Bytes)
				{
					Serial.println(F("Buffer to small for read"));
					return (0);
				}
				Buffer[cnt+cnt2] = Wire.read();
				--BytesLeft;
				--get;
			}
			else
				return(size - BytesLeft);

		if (get) // Did we get it all?
			return(Bytes - BytesLeft);  // Nope

		cnt2 += 32;
	}

	// Check CRC
	crc = GetCRC(size,Buffer);
	if (Buffer[size-2] != (unsigned char)(crc & 0x0ff) || Buffer[size-1] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error on read"));
		return(0);
	}

	return(size);  // Return total count of bytes read
}

// Routine to write multiple bytes to the auth chip
// Returns pass/fail
int WriteAuthBytes(uint8_t ChipAddr, uint8_t Buffer[], uint8_t Bytes)
{
	uint8_t BytesLeft = Bytes;
	int send, sent = 0;

	if (CheckClk()) // Check for clock not "stuck"
		return;

	while (BytesLeft)
	{
		// Start transmission
		Wire.beginTransmission(ChipAddr >> 1); // Set address to crypto chip

		if (BytesLeft >= 31)
			send = 31;
		else
			send = BytesLeft;

		Wire.write(3);
		if (Wire.write(Buffer+sent,send) != send)
		{
			Serial.println(F("Error writing data"));
			Wire.endTransmission();
			return (1);
		}
		sent += send;
		BytesLeft -= send;
		if (Wire.endTransmission())
			return(1);
	}

	return (0);
}

void AUTH_Update(uint8_t type)
{
	uint8_t LocalBuf[MAX_EEPROM_SIZE];

	if (!BatteryType)  // Check for valid battery type
		return;

	// Get data from the battery
	AUTH_Read(LocalBuf);

	switch (type)
	{
	case GIFTED_BATT_DATA:
	case GALACTUS_BATT_DATA:
		DoGiftedUpdate(0,LocalBuf);
		break;
	case METEOR_BATT:
		DoVTupdate(LocalBuf);
		break;
	case POLLUX_BATT_DATA:
		DoPolluxUpdate(LocalBuf);
		break;
	default:
		Serial.println(F("Unsupported data type!"));
		return;
		break;
	}

	// Write data back to battery
	AUTH_Write(LocalBuf);
}
