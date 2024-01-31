// Battery programmer auth chip functions

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <Arduino.h>
#include <Wire.h>	// I2C lib

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <Wire.h>	// I2C lib

#include "pwmbatt.h"
#include "BattProg.h"
#include "Auth.h"
#include "Manufacture.h"

// Function prototypes
static uint16_t GetCRC(int len, uint8_t *buffer);
static void WakeCrypto(void);
static int CheckOTP(void);
static int CheckCryptoConfig(void);
static int ReadSlot(int ChipAddr, uint8_t slot, uint8_t *buf);
static int WriteSlot(int ChipAddr, uint8_t slot, uint8_t *buf);
static int Sign(int ChipAddr, unsigned char *in_buf, unsigned char *out_buf);
static int ReadAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes);
static int WriteAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes);
static LoadTempReg(int ChipAddr, unsigned char *in_buf);


// ***************************************
// Verify chip stuff
// ***************************************

const uint8_t g_PubKey[KEY_LEN] = {
	0xd6, 0xb7, 0x42, 0xb0, 0x29, 0xae, 0x01, 0x77, 0x5a, 0x20, 0x23, 0x5a, 0xd4, 0x63, 0xf2, 0x02,
	0xa6, 0xc9, 0xb1, 0xef, 0x03, 0xf4, 0xfd, 0xfb, 0x6f, 0x17, 0xfd, 0xa9, 0x30, 0x5a, 0xb7, 0x77,
	0xea, 0x83, 0x1c, 0x27, 0x9f, 0x3d, 0x03, 0x71, 0x91, 0x07, 0x8a, 0x1b, 0x98, 0x33, 0x04, 0xb2,
	0x1f, 0x10, 0x61, 0xbf, 0x1e, 0xb3, 0x37, 0xdf, 0x3f, 0xea, 0x04, 0x21, 0xaa, 0x8a, 0x70, 0x55
};

PROGMEM const uint8_t g_config[128] = {
0x01, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0xDE, 0x00, 0x01, 0x00,
0xD0, 0x00, 0x55, 0x06, 0x83, 0x0F, 0xC4, 0x44,  0x87, 0x20, 0xC4, 0x44, 0x8F, 0x0F, 0x8F, 0x8F,
0x9F, 0x8F, 0x82, 0x20, 0x0F, 0x0F, 0x0F, 0x8F,  0x0F, 0x8F, 0x0F, 0x8F, 0x0F, 0x8F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF,  0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,  0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x13, 0x00, 0x5C, 0x00, 0x13, 0x00, 0x5C, 0x00,  0x3C, 0x00, 0x1C, 0x00, 0x1C, 0x00, 0x33, 0x00,
0x3C, 0x00, 0x10, 0x00, 0x1C, 0x00, 0x10, 0x00,  0x1C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00
};

/*
PROGMEM const uint8_t g_Slot8[416] = {
	0x88, 0x00, 0x88, 0x01, 0x88, 0x02, 0x88, 0x03, 0x88, 0x04, 0x88, 0x05, 0x88, 0x06, 0x88, 0x07,
	0x88, 0x08, 0x88, 0x09, 0x88, 0x0A, 0x88, 0x0B, 0x88, 0x0C, 0x88, 0x0D, 0x88, 0x0E, 0x88, 0x0F,
	0x88, 0x10, 0x88, 0x11, 0x88, 0x12, 0x88, 0x13, 0x88, 0x14, 0x88, 0x15, 0x88, 0x16, 0x88, 0x17,
	0x88, 0x18, 0x88, 0x19, 0x88, 0x1A, 0x88, 0x1B, 0x88, 0x1C, 0x88, 0x1D, 0x88, 0x1E, 0x88, 0x1F,
	0x88, 0x20, 0x88, 0x21, 0x88, 0x22, 0x88, 0x23, 0x88, 0x24, 0x88, 0x25, 0x88, 0x26, 0x88, 0x27,
	0x88, 0x28, 0x88, 0x29, 0x88, 0x2A, 0x88, 0x2B, 0x88, 0x2C, 0x88, 0x2D, 0x88, 0x2E, 0x88, 0x2F,
	0x88, 0x30, 0x88, 0x31, 0x88, 0x32, 0x88, 0x33, 0x88, 0x34, 0x88, 0x35, 0x88, 0x36, 0x88, 0x37,
	0x88, 0x38, 0x88, 0x39, 0x88, 0x3A, 0x88, 0x3B, 0x88, 0x3C, 0x88, 0x3D, 0x88, 0x3E, 0x88, 0x3F,
	0x88, 0x40, 0x88, 0x41, 0x88, 0x42, 0x88, 0x43, 0x88, 0x44, 0x88, 0x45, 0x88, 0x46, 0x88, 0x47,
	0x88, 0x48, 0x88, 0x49, 0x88, 0x4A, 0x88, 0x4B, 0x88, 0x4C, 0x88, 0x4D, 0x88, 0x4E, 0x88, 0x4F,
	0x88, 0x50, 0x88, 0x51, 0x88, 0x52, 0x88, 0x53, 0x88, 0x54, 0x88, 0x55, 0x88, 0x56, 0x88, 0x57,
	0x88, 0x58, 0x88, 0x59, 0x88, 0x5A, 0x88, 0x5B, 0x88, 0x5C, 0x88, 0x5D, 0x88, 0x5E, 0x88, 0x5F,
	0x88, 0x60, 0x88, 0x61, 0x88, 0x62, 0x88, 0x63, 0x88, 0x64, 0x88, 0x65, 0x88, 0x66, 0x88, 0x67,
	0x88, 0x68, 0x88, 0x69, 0x88, 0x6A, 0x88, 0x6B, 0x88, 0x6C, 0x88, 0x6D, 0x88, 0x6E, 0x88, 0x6F,
	0x88, 0x70, 0x88, 0x71, 0x88, 0x72, 0x88, 0x73, 0x88, 0x74, 0x88, 0x75, 0x88, 0x76, 0x88, 0x77,
	0x88, 0x78, 0x88, 0x79, 0x88, 0x7A, 0x88, 0x7B, 0x88, 0x7C, 0x88, 0x7D, 0x88, 0x7E, 0x88, 0x7F,
	0x88, 0x80, 0x88, 0x81, 0x88, 0x82, 0x88, 0x83, 0x88, 0x84, 0x88, 0x85, 0x88, 0x86, 0x88, 0x87,
	0x88, 0x88, 0x88, 0x89, 0x88, 0x8A, 0x88, 0x8B, 0x88, 0x8C, 0x88, 0x8D, 0x88, 0x8E, 0x88, 0x8F,
	0x88, 0x90, 0x88, 0x91, 0x88, 0x92, 0x88, 0x93, 0x88, 0x94, 0x88, 0x95, 0x88, 0x96, 0x88, 0x97,
	0x88, 0x98, 0x88, 0x99, 0x88, 0x9A, 0x88, 0x9B, 0x88, 0x9C, 0x88, 0x9D, 0x88, 0x9E, 0x88, 0x9F,
	0x88, 0xA0, 0x88, 0xA1, 0x88, 0xA2, 0x88, 0xA3, 0x88, 0xA4, 0x88, 0xA5, 0x88, 0xA6, 0x88, 0xA7,
	0x88, 0xA8, 0x88, 0xA9, 0x88, 0xAA, 0x88, 0xAB, 0x88, 0xAC, 0x88, 0xAD, 0x88, 0xAE, 0x88, 0xAF,
	0x88, 0xB0, 0x88, 0xB1, 0x88, 0xB2, 0x88, 0xB3, 0x88, 0xB4, 0x88, 0xB5, 0x88, 0xB6, 0x88, 0xB7,
	0x88, 0xB8, 0x88, 0xB9, 0x88, 0xBA, 0x88, 0xBB, 0x88, 0xBC, 0x88, 0xBD, 0x88, 0xBE, 0x88, 0xBF,
	0x88, 0xC0, 0x88, 0xC1, 0x88, 0xC2, 0x88, 0xC3, 0x88, 0xC4, 0x88, 0xC5, 0x88, 0xC6, 0x88, 0xC7,
	0x88, 0xC8, 0x88, 0xC9, 0x88, 0xCA, 0x88, 0xCB, 0x88, 0xCC, 0x88, 0xCD, 0x88, 0xCE, 0x88, 0xCF
};
*/

// Slot defines
#define DEVICE_PUB		9
#define DEVICE_SIG		10
#define SIGNER_PUB		11
#define SIGNER_SIG		12

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

	Serial.println();
	Serial.println(F("Read OK!"));
	BufferBlank = 0;

	return (0);
}

void AUTH_Verify(void)
{
	int ret;
	int cnt;
	uint8_t inch;
	uint8_t SlotBuf[416];

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

	// Do verify
	Serial.print(F("Checking: "));
	for (cnt = 0; cnt < 416; ++cnt)
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
	int ret;
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
}

uint8_t AUTH_ShowSN(void)
{
	uint16_t crc;
	uint8_t CommBuf[40];
	int SN;

	WakeCrypto();

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
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7)) {
		Serial.println(F("Failed to send read slot command"));
		return(1);
	}

	delayMicroseconds(1100);  // Wait for exec

	// Get responce
	if (35 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,35))
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

	// Get and check serial number
	SN = 0;
	sscanf(CommBuf+1,"SN: %d",&SN);

	if (SN < 100 || SN > 999)
	{
		Serial.println(F("Invalid SN!"));
		return (1);
	}

	if (SN >= 500)
		JDM_Mode = 1;
	else
		JDM_Mode = 0;

	return (0);
}

void EEPROM_CopyToAuth(void)
{
	Serial.println(F("Comming soon!"));
}

static int Random(uint8_t *rnd)
{
	uint16_t crc;
	int cnt;
	uint8_t CommBuf[40];

	// Build command to get a random nuimber
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_RANDOM;
	CommBuf[2] = 0;
	CommBuf[3] = 0;
	CommBuf[4] = 0;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7)) {
		Serial.println(F("Failed to send Random command"));
		return(1);
	}

	delay(30);  // Wait for exec

	// Get responce
	if (35 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,35))
	{
		Serial.println(F("Failed to get random responce"));
		return(1);
	}

	crc = GetCRC(35,CommBuf);
	if (CommBuf[33] != (unsigned char)(crc & 0x0ff) || CommBuf[34] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error in random responce"));
		return(1);
	}

	// Copy over result
	for (cnt = 0; cnt < 32; ++cnt)
		rnd[cnt] = CommBuf[cnt+1];
	return (0);
}

static int VerifySig(uint8_t *Sig, uint8_t *PubKey)
{
	uint16_t crc;
	int cnt, cnt2;
	uint8_t CommBuf[140];

	// Build command to verify sig
	CommBuf[0] = 135;
	CommBuf[1] = ATCA_VERIFY;
	CommBuf[2] = VERIFY_MODE_EXTERNAL;
	CommBuf[3] = VERIFY_KEY_P256;
	CommBuf[4] = 0;

	for (cnt = 0; cnt < KEY_LEN; ++cnt)
		CommBuf[cnt+5] = Sig[cnt];
	for (cnt = 0; cnt < KEY_LEN; ++cnt)
		CommBuf[cnt+5+KEY_LEN] = PubKey[cnt];

	crc = GetCRC(135,CommBuf);
	CommBuf[133] = (unsigned char)(crc & 0x0ff);
	CommBuf[134] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,135)) {
		Serial.println(F("Failed to send Verify command"));
		return(1);
	}

	delay(60);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,4))
	{
		Serial.println(F("Failed to get verify responce"));
		return(1);
	}

	crc = GetCRC(4,CommBuf);
	if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error in verify responce"));
		return(1);
	}

	if (CommBuf[1])
	{
		Serial.println(F("Bad sig!"));
		return(1);
	}
	else
		return (0);
}

static int SHA256Hash(uint8_t *Key, uint8_t Length)
{
	uint16_t crc;
	int cnt, cnt2;
	unsigned char CommBuf[72];

	// Build command to start SHA256
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_SHA;
	CommBuf[2] = SHA_MODE_SHA256_START;
	CommBuf[3] = 0;
	CommBuf[4] = 0;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7)) {
		Serial.println(F("Failed to send SHA start command"));
		return(1);
	}

	delayMicroseconds(10000);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,4))
	{
		Serial.println(F("Failed to get start responce"));
		return(1);
	}

	crc = GetCRC(4,CommBuf);

	if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error on start responce"));
		return(1);
	}

	if (CommBuf[1])
	{
		Serial.println(F("Error in start command"));
		return(1);
	}

	// Build command to update SHA256
	CommBuf[0] = Length+7;
	CommBuf[1] = ATCA_SHA;
	CommBuf[2] = SHA_MODE_SHA256_UPDATE;
	CommBuf[3] = KEY_LEN;
	CommBuf[4] = 0;

	for (cnt = 0; cnt < Length; ++cnt)
		CommBuf[cnt+5] = Key[cnt];

	crc = GetCRC(Length+7,CommBuf);
	CommBuf[Length+5] = (unsigned char)(crc & 0x0ff);
	CommBuf[Length+6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,71)) {
		Serial.println(F("Failed to send SHA update command"));
		return(1);
	}

	delayMicroseconds(10000);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,4))
	{
		Serial.println(F("Failed to get update responce"));
		return(1);
	}

	crc = GetCRC(4,CommBuf);
	if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error in update responce"));
		return(1);
	}

	if (CommBuf[1])
	{
		Serial.println(F("Error in update command"));
		return(1);
	}

	// Build command to end SHA256
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_SHA;
	CommBuf[2] = SHA_MODE_SHA256_END;
	CommBuf[3] = 0;
	CommBuf[4] = 0;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(LOCAL_AUTH_ADDR,CommBuf,7)) {
		return(1);
	}

	delayMicroseconds(10000);  // Wait for exec

	// Get responce
	if (35 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,35))
	{
		Serial.println(F("Failed to get end responce"));
		return(1);
	}

	crc = GetCRC(35,CommBuf);
	if (CommBuf[33] != (unsigned char)(crc & 0x0ff) || CommBuf[34] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error in end responce"));
		return(1);
	}
	else
		return (0);
}

uint8_t CheckAuthChip(void)
{
	int ret;
	int cnt;
	uint8_t SignerPubKey[KEY_LEN];
	uint8_t DevicePubKey[KEY_LEN];
	uint8_t SlotBuf[416];
	uint8_t hash[32];

	// Wake up crypto chips
	WakeCrypto();

	// Check for correct serial number/config
	if (Verbose)
		Serial.print(F("Check config area: "));
	if ((CheckCryptoConfig()))
		return (1);
	if (Verbose)
		Serial.println(F("Passed"));


/*
	// Check OTP area
	Serial.print(F("Check OTP area: "));
	if ((ret = CheckOTP()))
		return;
	if (Verbose)
		Serial.println(F("Passed"));
*/


	// Get signer public key
	if (Verbose)
		Serial.print(F("Check signer public key: "));
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,SIGNER_PUB,SlotBuf)))
		return (1);

	for (cnt = 0; cnt < 32; ++cnt)
		SignerPubKey[cnt] = SlotBuf[cnt+4];
	for (cnt = 32; cnt < KEY_LEN; ++cnt)
		SignerPubKey[cnt] = SlotBuf[cnt+8];

	// Get signer signature
	if (ReadSlot(Batts[BatteryType].CryptoAddress,SIGNER_SIG,SlotBuf))
		return (1);

	// Do a SHA256 hash of signer public key
	if (SHA256Hash(SignerPubKey,KEY_LEN))
		return (1);

	// Check that signer public key was signed by Zebra root private key
	if (VerifySig(SlotBuf,g_PubKey)) // ***FIX*** move to flash!!!
		return (1);

	if (Verbose)
		Serial.println(F("Passed"));

	// Check for valid device signature
	// Get device public key
	if (Verbose)
		Serial.print(F("Check device public key: "));
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,DEVICE_PUB,SlotBuf)))
		return (1);

	for (cnt = 0; cnt < 32; ++cnt)
		DevicePubKey[cnt] = SlotBuf[cnt+4];
	for (cnt = 32; cnt < KEY_LEN; ++cnt)
		DevicePubKey[cnt] = SlotBuf[cnt+8];

	// Get device signature
	if (ReadSlot(Batts[BatteryType].CryptoAddress,DEVICE_SIG,SlotBuf))
		return (1);

	// Do a SHA256 hash of device public key
	if (SHA256Hash(DevicePubKey,KEY_LEN))
		return (1);

	// Check that device public key was signed by signer private key
	if (VerifySig(SlotBuf,SignerPubKey))
		return (1);

	if (Verbose)
		Serial.println(F("Passed"));

	if (Verbose)
		Serial.print(F("Check device private key: "));

	// Verify device private key matches device public key
	// Generate random 32 byte hash
	Random(hash);

	// Have chip sign random number with private key
	if (Sign(Batts[BatteryType].CryptoAddress,hash,SlotBuf))
	{
		Serial.println(F("Bad signing"));
		return (1);
	}

	// Check that random number key was signed by device private key
	if (LoadTempReg(LOCAL_AUTH_ADDR, hash))
		return (1);
	if (VerifySig(SlotBuf,DevicePubKey))
		return (1);

	if (Verbose)
		Serial.println(F("Passed"));

	return (0);
}

static LoadTempReg(int ChipAddr, unsigned char *in_buf)
{
	uint16_t crc;
	int cnt;
	unsigned char CommBuf[70];

	// Send random number
	CommBuf[0] = 39;
	CommBuf[1] = ATCA_NONCE;
	CommBuf[2] = NONCE_MODE_PASSTHROUGH;
	CommBuf[3] = 0;
	CommBuf[4] = 0;

	// Copy number to buffer
	for (cnt = 0; cnt < 32; ++cnt)
		CommBuf[5+cnt] = in_buf[cnt];
	
	crc = GetCRC(39,CommBuf);
	CommBuf[37] = (unsigned char)(crc & 0x0ff);
	CommBuf[38] = (unsigned char)((crc >> 8) & 0x0ff);

	if (WriteAuthBytes(ChipAddr,CommBuf,39)) {
		return(1);
	}

	delayMicroseconds(10000);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(ChipAddr,CommBuf,4))
	{
		Serial.println(F("Failed to get nonce responce"));
		return(1);
	}

	crc = GetCRC(4,CommBuf);
	if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error in nonce responce"));
		return(1);
	}

	if (CommBuf[1])
	{
		Serial.println(F("Error in nonce command"));
		return(1);
	}

	return (0);
}

static int Sign(int ChipAddr, unsigned char *in_buf, unsigned char *out_buf)
{
	uint16_t crc;
	int cnt;
	unsigned char CommBuf[70];

	// Load stuff to sign
	if (LoadTempReg(ChipAddr, in_buf))
		return;

	// Sign it
	CommBuf[0] = 7;
	CommBuf[1] = ATCA_SIGN;
	CommBuf[2] = SIGN_MODE_EXTERNAL;
	CommBuf[3] = DEVICE_PRIV;
	CommBuf[4] = 0;

	crc = GetCRC(7,CommBuf);
	CommBuf[5] = (unsigned char)(crc & 0x0ff);
	CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

	if (WriteAuthBytes(ChipAddr,CommBuf,7)) {
		return(1);
	}

	delay(60);  // Wait for exec

	// Get responce
	if (67 != ReadAuthBytes(ChipAddr,CommBuf,67))
	{
		Serial.println(F("Failed to get sign responce"));
		return(1);
	}

	crc = GetCRC(67,CommBuf);
	if (CommBuf[65] != (unsigned char)(crc & 0x0ff) || CommBuf[66] != (unsigned char)((crc >> 8) & 0x0ff))
	{
		Serial.println(F("CRC error in sign responce"));
		return(1);
	}

	for (cnt = 0; cnt < KEY_LEN; ++cnt)
		out_buf[cnt] = CommBuf[cnt+1];

	return (0);
}

static int ReadSlot(int ChipAddr, uint8_t slot, uint8_t *buf)
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

		crc = GetCRC(35,CommBuf);

		if (CommBuf[33] != (unsigned char)(crc & 0x0ff) || CommBuf[34] != (unsigned char)((crc >> 8) & 0x0ff))
		{
			Serial.println(F("CRC error on slot read"));
			return(1);
		}

		for (cnt2 = 0; cnt2 < (size>=32 ? 32 : size); ++cnt2)
			buf[cnt+cnt2] = CommBuf[cnt2+1];

		cnt += 32;
		size -= 32;
	}

	return (0);
}

// ***FIX*** only works for slot 8!!!
static int WriteSlot(int ChipAddr, uint8_t slot, uint8_t *buf)
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

		crc = GetCRC(4,CommBuf);

		if (CommBuf[2] != (unsigned char)(crc & 0x0ff) || CommBuf[3] != (unsigned char)((crc >> 8) & 0x0ff))
		{
			Serial.println(F("CRC error on slot write"));
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

static int GetCryptoConfig(uint8_t *Buf)
{
	uint16_t crc;
	int cnt, cnt2;
	uint8_t CommBuf[40];

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
		if (WriteAuthBytes(Batts[BatteryType].CryptoAddress,CommBuf,7)) {
			Serial.println(F("Failed to send read config command"));
			return(1);
		}

		delayMicroseconds(1100);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(Batts[BatteryType].CryptoAddress,CommBuf,35))
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


		// Copy over the data
		for (cnt2 = 0; cnt2 < 32; ++cnt2)
			Buf[cnt*32+cnt2] = CommBuf[cnt2+1];
	}

	return (0);
}

static int CheckCryptoConfig(void)
{
	int cnt2;
	uint8_t CommBuf[128];

	if (GetCryptoConfig(CommBuf))
		return (1);

	// Check serial number and first 16 bytes
	if (AUTH_SN0 != CommBuf[0] || AUTH_SN1 != CommBuf[1] || AUTH_SN8 != CommBuf[12])
	{
		Serial.println(F("Bad SN"));
		return(1);
	}

	for (cnt2 = 16; cnt2 < 86; ++cnt2)
		if (CommBuf[cnt2] != pgm_read_byte_far(g_config + cnt2))
		{
			Serial.print(F("Bad Config at "));
			Serial.println(cnt2);
			return(1);
		}

	if (0x55 == CommBuf[86])
	{
		Serial.print(F("Data area unlocked!!!"));
		return(1);
	}

	if (0x55 == CommBuf[87])
	{
		Serial.print(F("Config area unlocked!!!"));
		return(1);
	}

	for (cnt2 = 88; cnt2 < 128; ++cnt2)
		if (CommBuf[cnt2] != pgm_read_byte_far(g_config + cnt2))
		{
			Serial.print(F("Bad Config at "));
			Serial.println(cnt2);
			return(1);
		}

	return (0);
}

static int CheckOTP(void)
{
	uint16_t crc;
	int cnt, cnt2;
	unsigned char CommBuf[40];

	for (cnt = 0; cnt < 2; ++cnt)
	{
		// Build command to read 32 bytes of OTP area
		CommBuf[0] = 7;
		CommBuf[1] = ATCA_READ;
		CommBuf[2] = ATCA_ZONE_READWRITE_32 | ATCA_ZONE_OTP;
		CommBuf[3] = cnt << 3;
		CommBuf[4] = 0;

		crc = GetCRC(7,CommBuf);
		CommBuf[5] = (unsigned char)(crc & 0x0ff);
		CommBuf[6] = (unsigned char)((crc >> 8) & 0x0ff);

		if (WriteAuthBytes(Batts[BatteryType].CryptoAddress,CommBuf,7)) {
			Serial.println(F("Failed to send read config command"));
			return(1);
		}

		delayMicroseconds(1100);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(Batts[BatteryType].CryptoAddress,CommBuf,35))
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

		// Check data
		for (cnt2 = 0; cnt2 < 32; ++cnt2)
			if (CommBuf[cnt2+1] != 0xff)
			{
				Serial.println(F("OTP not blank"));
				return(1);
			}
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
static int ReadAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes)
{
	uint8_t cnt, cnt2 = 0, BytesLeft = Bytes;
	int get, got;

	if (CheckClk()) // Check for clock not "stuck"
		return;

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
		DoGiftedUpdate(0,LocalBuf);
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

// Function to validate data in the auth chip
uint8_t Validate_AUTH(uint8_t type)
{
	uint8_t ret;

	if (!BatteryType)  // Check for valid battery type
		return (1);

	switch (type)
	{
	case GIFTED_BATT_DATA:
		ret = ValidatePPP(EEPROM_Data);
		break;
	case HAWKEYE_BATT_DATA:
		ret = ValidateHawkeye(EEPROM_Data);
		break;
	default:
		Serial.println(F("Unsupported data type!"));
		break;
	}

	return (ret);
}

// Function to Manufacture data in the auth chip
uint8_t Manufacture_AUTH(uint8_t type)
{
	uint8_t ret;

	if (!BatteryType)  // Check for valid battery type
		return (1);

	switch (type)
	{
	case GIFTED_BATT_DATA:
		ret = ManufacturePPP(EEPROM_Data);
		break;
	case HAWKEYE_BATT_DATA:
		ret = ManufactureHawkeye(EEPROM_Data);
		break;
	default:
		Serial.println(F("Unsupported data type!"));
		break;
	}

	return (ret);
}

void AUTH_ShowConfigHex(void)
{
	uint8_t CommBuf[128];

	// Wake up crypto chips
	WakeCrypto();

	if (GetCryptoConfig(CommBuf))
		return (1);

	HEX_Write(CommBuf,128);
}

void AUTH_SaveConfigHex(void)
{
	uint8_t CommBuf[128];

	// Wake up crypto chips
	WakeCrypto();

	if (GetCryptoConfig(CommBuf))
		return (1);

	SaveFile(CommBuf,128);
}

void AUTH_ShowAllData(void)
{
	uint8_t SlotBuf[416];
	uint8_t CommBuf[128];
	uint8_t cnt;
	uint8_t hash[32];

	// Wake up crypto chips
	WakeCrypto();

	if (GetCryptoConfig(CommBuf))
		return;

	// Print serial number
	Serial.println();
	Serial.print(F("Serial:"));
	for (cnt = 0; cnt <= 3; ++cnt)
		printfSerial(" %02X",CommBuf[cnt]);
	for (cnt = 8; cnt <= 12; ++cnt)
		printfSerial(" %02X",CommBuf[cnt]);
	Serial.println();
	// Print rev
	Serial.print(F("Rev:"));
	for (cnt = 4; cnt <= 7; ++cnt)
		printfSerial(" %02X",CommBuf[cnt]);
	Serial.println();


	// Wake up crypto chips
	WakeCrypto();

	// Get signer public key
	Serial.print(F("Signer public key"));
	if (ReadSlot(Batts[BatteryType].CryptoAddress,SIGNER_PUB,SlotBuf))
		return;

	for (cnt = 0; cnt < 72; ++cnt)
	{
		if (!(cnt % 16))
			Serial.println();
		printfSerial("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();

	// Get signer signature
	if (ReadSlot(Batts[BatteryType].CryptoAddress,SIGNER_SIG,SlotBuf))
		return;

	Serial.print(F("Signer sig"));
	for (cnt = 0; cnt < 72; ++cnt)
	{
		if (!(cnt % 16))
			Serial.println();
		printfSerial("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();

	// Get device public key
	if (ReadSlot(Batts[BatteryType].CryptoAddress,DEVICE_PUB,SlotBuf))
		return;

	Serial.print(F("Device Pub Key"));
	for (cnt = 0; cnt < 72; ++cnt)
	{
		if (!(cnt % 16))
			Serial.println();
		printfSerial("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();

	// Get device signature
	if (ReadSlot(Batts[BatteryType].CryptoAddress,DEVICE_SIG,SlotBuf))
		return;

	Serial.print(F("Device sig:"));
	for (cnt = 0; cnt < 72; ++cnt)
	{
		if (!(cnt % 16))
			Serial.println();
		printfSerial("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();

	// Verify device private key matches device public key
	// Generate random 32 byte hash
	Random(hash);

	Serial.print(F("Hash:"));
	for (cnt = 0; cnt < 32; ++cnt)
	{
		if(!(cnt%16))
			Serial.println();
		printfSerial("0x%02X,",hash[cnt]);
	}
	Serial.println();

	// Have chip sign random number with private key
	if (Sign(Batts[BatteryType].CryptoAddress,hash,SlotBuf))
	{
		Serial.println(F("Bad signing"));
		return (1);
	}

	Serial.print(F("Sig:"));
	for (cnt = 0; cnt < 64; ++cnt)
	{
		if(!(cnt%16))
			Serial.println();
		printfSerial("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();
}
