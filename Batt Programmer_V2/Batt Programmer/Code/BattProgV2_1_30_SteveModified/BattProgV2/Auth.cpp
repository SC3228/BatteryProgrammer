// Battery programmer auth chip functions

#include <Arduino.h>
#include "Adafruit_SPIFlash.h"
#include <SoftWire.h>	// I2C lib
extern SoftWire Wire;

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "pwmbatt.h"
#include "Batts.h"
#include "BattProg.h"
#include "Auth.h"
#include "Manufacture.h"
#include "GasGauge.h"

// Function prototypes
static uint16_t GetCRC(int len, uint8_t *buffer);
static void WakeCrypto(void);
static int CheckCryptoConfig(void);
//static int CheckOTP(void);
static int ReadSlot(uint8_t ChipAddr, uint8_t slot, uint8_t *buf);
static int WriteSlot(uint8_t ChipAddr, uint8_t slot, uint8_t *buf);
static int GenPubKey(uint8_t ChipAddr, uint8_t *buf);
static int Sign(uint8_t ChipAddr, unsigned char *in_buf, unsigned char *out_buf);
static int ReadAuthBytes(uint8_t ChipAddr, uint8_t Buffer[], uint8_t Bytes);
static int WriteAuthBytes(uint8_t ChipAddr, uint8_t Buffer[], uint8_t Bytes);
static int LoadTempReg(uint8_t ChipAddr, unsigned char *in_buf);


// ***************************************
// Verify chip stuff
// ***************************************

const uint8_t g_PubKey[KEY_LEN] = {
	0xd6, 0xb7, 0x42, 0xb0, 0x29, 0xae, 0x01, 0x77, 0x5a, 0x20, 0x23, 0x5a, 0xd4, 0x63, 0xf2, 0x02,
	0xa6, 0xc9, 0xb1, 0xef, 0x03, 0xf4, 0xfd, 0xfb, 0x6f, 0x17, 0xfd, 0xa9, 0x30, 0x5a, 0xb7, 0x77,
	0xea, 0x83, 0x1c, 0x27, 0x9f, 0x3d, 0x03, 0x71, 0x91, 0x07, 0x8a, 0x1b, 0x98, 0x33, 0x04, 0xb2,
	0x1f, 0x10, 0x61, 0xbf, 0x1e, 0xb3, 0x37, 0xdf, 0x3f, 0xea, 0x04, 0x21, 0xaa, 0x8a, 0x70, 0x55
};

const uint8_t g_config[128] = {
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
#define DEVCIE_PRI		0
#define DEVICE_PUB		9
#define DEVICE_SIG		10
#define SIGNER_PUB		11
#define SIGNER_SIG		12

// Slot end addresses
#define SLOT8_END    415 // Slot 8 end
#define SLOT13_END   487 // Slot 13 end
#define SLOT14_END   559 // Slot 14 end
#define SLOT15_END   631 // Slot 15 end

uint8_t AUTH_Read(uint8_t *buffer, bool doStatus)
{
	uint8_t *data;
	int ret;
	uint8_t inch;

	if (!BatteryType)  // Check for valid battery type
		return(1);

	FlushSerial();
	if (NULL == buffer)  // Check for which buffer
	{
		data = EEPROM_Data;
		Serial.print("Read ");
		Serial.print(Batts[BatteryType].Name);
		Serial.println(" Auth chip into buffer (y/n)?");
		while(!Serial.available())
		;
		inch = Serial.read();
		if (inch != 'y' && inch != 'Y')
			return(1);
	}
	else
		data = buffer;

	STATUS_LED_BUSY;
	// Wake up crypto chips
	WakeCrypto();

	// Read data from slot 8
	Serial.print("Reading slot8 data: ");
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,8,data)))
	{
		Serial.print("Bad slot8 read: ");
		Serial.println(ret);
		STATUS_LED_READY;
		return (1);
	}
	Serial.println(" ok");

	// Read data from slot 13
	Serial.print("Reading slot13 data: ");
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,13,data+SLOT8_END+1)))
	{
		Serial.print("Bad slot13 read: ");
		Serial.println(ret);
		STATUS_LED_READY;
		return (1);
	}
	Serial.println(" ok");

	// Read data from slot 14
	Serial.print("Reading slot14 data: ");
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,14,data+SLOT13_END+1)))
	{
		Serial.print("Bad slot14 read: ");
		Serial.println(ret);
		STATUS_LED_READY;
		return (1);
	}
	Serial.println(" ok");

	// Read data from slot 15
	Serial.print("Reading slot15 data: ");
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,15,data+SLOT14_END+1)))
	{
		Serial.print("Bad slot15 read: ");
		Serial.println(ret);
		STATUS_LED_READY;
		return (1);
	}
	Serial.println(" ok");

	Serial.println("Read ok!");
	BufferBlank = 0;

	if (doStatus)
		STATUS_LED_READY;

	return (0);
}

void AUTH_Verify(void)
{
	int ret;
	int cnt;
	uint8_t SlotBuf[632];

	if (!BatteryType)  // Check for valid battery type
		return;

	FlushSerial();

	STATUS_LED_BUSY;

	// Wake up crypto chips
	WakeCrypto();

	// Read data from slot 8
	Serial.print("Reading slot8 data: ");
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,8,SlotBuf)))
	{
		Serial.print("Bad slot8 read: ");
		Serial.println(ret);
		STATUS_LED_READY;
		return;
	}

	Serial.println(ok);

	// Read data from slot 13
	Serial.print("Reading slot13 data: ");
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,13,SlotBuf+416)))
	{
		Serial.print("Bad slot13 read: ");
		Serial.println(ret);
		STATUS_LED_READY;
		return;
	}

	Serial.println(ok);

	// Read data from slot 14
	Serial.print("Reading slot14 data: ");
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,14,SlotBuf+416+72)))
	{
		Serial.print("Bad slot14 read: ");
		Serial.println(ret);
		STATUS_LED_READY;
		return;
	}

	Serial.println(ok);

	// Read data from slot 15
	Serial.print("Reading slot15 data: ");
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,15,SlotBuf+416+144)))
	{
		Serial.print("Bad slot15 read: ");
		Serial.println(ret);
		STATUS_LED_READY;
		return;
	}

	Serial.println(ok);

	// Do verify
	Serial.print("Checking: ");
	for (cnt = 0; cnt < MAX_AUTH_SIZE; ++cnt)
		if (SlotBuf[cnt] != EEPROM_Data[cnt])
		{
			Serial.print("Error at: ");
			Serial.println(cnt);
			STATUS_LED_READY;
			return;
		}

	STATUS_LED_READY;
	Serial.print(ok);
}

void AUTH_Write(uint8_t *buffer, bool ask)
{
	uint8_t inch;
	uint8_t *data;

	if (NULL == buffer)  // Check for which buffer
		data = EEPROM_Data;
	else
		data = buffer;

	if (!BatteryType)  // Check for valid battery type
		return;

	if (ask)
	{
		FlushSerial();
		Serial.print("Overwrite ");
		Serial.print(Batts[BatteryType].Name);
		Serial.println(" (y/n)?");
		while(!Serial.available())
		;
		inch = Serial.read();
		if (inch != 'y' && inch != 'Y')
			return;
	}

	STATUS_LED_BUSY;
	// Wake up crypto chips
	WakeCrypto();

	// Write data to slot 8
	Serial.print("Writing slot8 data: ");
	if (WriteSlot(Batts[BatteryType].CryptoAddress,8,data))
	{
		STATUS_LED_READY;
		return;
	}
	Serial.println(ok);

	Serial.print("Writing slot13 data: ");
	if (WriteSlot(Batts[BatteryType].CryptoAddress,13,data+SLOT8_END+1))
	{
		STATUS_LED_READY;
		return;
	}
	Serial.println(ok);

	Serial.print("Writing slot14 data: ");
	if (WriteSlot(Batts[BatteryType].CryptoAddress,14,data+SLOT13_END+1))
	{
		STATUS_LED_READY;
		return;
	}
	Serial.println(ok);

	Serial.print("Writing slot15 data: ");
	if (WriteSlot(Batts[BatteryType].CryptoAddress,15,data+SLOT14_END+1))
	{
		STATUS_LED_READY;
		return;
	}

	STATUS_LED_READY;
	Serial.println(ok);
	Serial.println("Auth write ok!");
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

uint8_t AUTH_ShowSN(void)
{
	if (!(AuthSN = AUTH_ReadSerial(0)))
		return(1);
	
	if (!(AuthHW = AUTH_ReadSerial(1)))
		return(1);
	
	if (AuthSN < 100 || AuthSN > 1999)
	{
		Serial.printf("Invalid SN: %d\r\n",AuthSN);
		JDM_Mode = 1;
		return (1);
	}

	if (AuthSN >= 1000)
		JDM_Mode = 1;
	else
		JDM_Mode = 0;

	return (0);
}

void EEPROM_CopyToAuth(void)
{
	Serial.println("Comming soon!");
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
		Serial.println("Failed to send Random command");
		return(1);
	}

	delay(30);  // Wait for exec

	// Get responce
	if (35 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,35))
	{
		Serial.println("Failed to get random responce");
		return(1);
	}

	// Copy over result
	for (cnt = 0; cnt < 32; ++cnt)
		rnd[cnt] = CommBuf[cnt+1];
	return (0);
}

static int VerifySig(uint8_t *Sig, const uint8_t *PubKey)
{
	uint16_t crc;
	int cnt;
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
		Serial.println("Failed to send Verify command");
		return(1);
	}

	delay(60);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,4))
	{
		Serial.println("Failed to get verify responce");
		return(1);
	}

	if (CommBuf[1])
	{
		Serial.println("Bad sig!");
		return(1);
	}
	else
		return (0);
}

static int SHA256Hash(uint8_t *Key, uint8_t Length)
{
	uint16_t crc;
	int cnt;
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
		Serial.println("Failed to send SHA start command");
		return(1);
	}

	delay(60);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,4))
	{
		Serial.println("Failed to get start responce");
		return(1);
	}

	if (CommBuf[1])
	{
		Serial.println("Error in start command");
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
		Serial.println("Failed to send SHA update command");
		return(1);
	}

	delay(60);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,4))
	{
		Serial.println("Failed to get update responce");
		return(1);
	}

	if (CommBuf[1])
	{
		Serial.println("Error in update command");
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

	delay(60);  // Wait for exec

	// Get responce
	if (35 != ReadAuthBytes(LOCAL_AUTH_ADDR,CommBuf,35))
	{
		Serial.println("Failed to get end responce");
		return(1);
	}

	return (0);
}

uint8_t CheckAuthChip(bool doStatus)
{
	int ret;
	int cnt;
	uint8_t SignerPubKey[KEY_LEN];
	uint8_t DevicePubKey[KEY_LEN];
	uint8_t SlotBuf[632];
	uint8_t hash[32];

	STATUS_LED_BUSY;

	// Wake up crypto chips
	WakeCrypto();

	// Check for correct serial number/config
	if (Verbose)
	{
		Serial.println();
		Serial.println("Validating Auth Chip");
		Serial.print("Config area: ");
	}
	if ((CheckCryptoConfig()))
	{
		STATUS_LED_READY;
		return (1);
	}
	if (Verbose)
		Serial.println(Passed);

	// Get signer public key
	if (Verbose)
		Serial.print("Signer public key: ");
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,SIGNER_PUB,SlotBuf)))
	{
		STATUS_LED_READY;
		return (1);
	}

	for (cnt = 0; cnt < 32; ++cnt)
		SignerPubKey[cnt] = SlotBuf[cnt+4];
	for (cnt = 32; cnt < KEY_LEN; ++cnt)
		SignerPubKey[cnt] = SlotBuf[cnt+8];

	// Get signer signature
	if (ReadSlot(Batts[BatteryType].CryptoAddress,SIGNER_SIG,SlotBuf))
	{
		STATUS_LED_READY;
		return (1);
	}

	// Do a SHA256 hash of signer public key
	if (SHA256Hash(SignerPubKey,KEY_LEN))
	{
		STATUS_LED_READY;
		return (1);
	}

	// Check that signer public key was signed by Zebra root private key
	if (VerifySig(SlotBuf,g_PubKey))
	{
		STATUS_LED_READY;
		return (1);
	}

	if (Verbose)
		Serial.println(Passed);

	// Check for valid device signature
	// Get device public key
	if (Verbose)
		Serial.print("Device public key: ");
	if ((ret = ReadSlot(Batts[BatteryType].CryptoAddress,DEVICE_PUB,SlotBuf)))
	{
		STATUS_LED_READY;
		return (1);
	}

	for (cnt = 0; cnt < 32; ++cnt)
		DevicePubKey[cnt] = SlotBuf[cnt+4];
	for (cnt = 32; cnt < KEY_LEN; ++cnt)
		DevicePubKey[cnt] = SlotBuf[cnt+8];

	// Get device signature
	if (ReadSlot(Batts[BatteryType].CryptoAddress,DEVICE_SIG,SlotBuf))
	{
		STATUS_LED_READY;
		return (1);
	}

	// Do a SHA256 hash of device public key
	if (SHA256Hash(DevicePubKey,KEY_LEN))
	{
		STATUS_LED_READY;
		return (1);
	}

	// Check that device public key was signed by signer private key
	if (VerifySig(SlotBuf,SignerPubKey))
	{
		STATUS_LED_READY;
		return (1);
	}

	if (Verbose)
		Serial.println(Passed);

	if (Verbose)
		Serial.print("Device private key: ");

	// Verify device private key matches device public key
	// Generate random 32 byte hash
	Random(hash);

	// Have chip sign random number with private key
	if (Sign(Batts[BatteryType].CryptoAddress,hash,SlotBuf))
	{
		Serial.println("Bad signing");
		STATUS_LED_READY;
		return (1);
	}

	// Check that random number key was signed by device private key
	if (LoadTempReg(LOCAL_AUTH_ADDR, hash))
	{
		STATUS_LED_READY;
		return (1);
	}
	if (VerifySig(SlotBuf,DevicePubKey))
	{
		STATUS_LED_READY;
		return (1);
	}

	if (doStatus)
		STATUS_LED_READY;

	if (Verbose)
		Serial.println(Passed);

	return (0);
}

static int LoadTempReg(uint8_t ChipAddr, unsigned char *in_buf)
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

	delay(60);  // Wait for exec

	// Get responce
	if (4 != ReadAuthBytes(ChipAddr,CommBuf,4))
	{
		Serial.println("Failed to get nonce responce");
		return(1);
	}

	if (CommBuf[1])
	{
		Serial.println("Error in nonce command");
		return(1);
	}

	return (0);
}

static int Sign(uint8_t ChipAddr, unsigned char *in_buf, unsigned char *out_buf)
{
	uint16_t crc;
	int cnt;
	unsigned char CommBuf[70];

	// Load stuff to sign
	if (LoadTempReg(ChipAddr, in_buf))
		return(1);

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
		Serial.println("Failed to get sign response");
		return(1);
	}

	for (cnt = 0; cnt < KEY_LEN; ++cnt)
		out_buf[cnt] = CommBuf[cnt+1];

	return (0);
}

static int ReadSlot(uint8_t ChipAddr, uint8_t slot, uint8_t *buf)
{
	uint16_t crc;
	int cnt, cnt2;
	unsigned char CommBuf[40];
	int size;

	// Set slot size
	if (slot <= 7)
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
			Serial.println("Failed to send read slot command");
			return(1);
		}

		delay(62);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(ChipAddr,CommBuf,35))
		{
			Serial.println("Failed to get read slot responce");
			return(1);
		}

		for (cnt2 = 0; cnt2 < (size>=32 ? 32 : size); ++cnt2)
			buf[cnt+cnt2] = CommBuf[cnt2+1];

		cnt += 32;
		size -= 32;
	}

	return (0);
}

static int GenPubKey(uint8_t ChipAddr, uint8_t *buf)
{
	uint16_t crc;
	int cnt;
	unsigned char CommBuf[80];

	// Build command to read slot
	CommBuf[0] = 10;
	CommBuf[1] = ATCA_GENKEY;
	CommBuf[2] = 0;
	CommBuf[3] = DEVICE_PRIV;
	CommBuf[4] = 0;
	CommBuf[5] = 0;
	CommBuf[6] = 0;
	CommBuf[7] = 0;

	crc = GetCRC(10,CommBuf);
	CommBuf[8] = (unsigned char)(crc & 0x0ff);
	CommBuf[9] = (unsigned char)((crc >> 8) & 0x0ff);

	// Send command
	if (WriteAuthBytes(ChipAddr,CommBuf,10)) {
		Serial.println("Failed to send genkey command");
		return(1);
	}

	delay(130);  // Wait for exec

	// Get responce
	if (!ReadAuthBytes(ChipAddr,CommBuf,67))
	{
		Serial.println("Failed to get genkey response");
		return(1);
	}

	if (4 == CommBuf[0])
	{
		Serial.print("Error: ");
		Serial.println(CommBuf[1]);
		return(1);
	}

	for (cnt = 0; cnt < 64; ++cnt)
		buf[cnt] = CommBuf[cnt+1];

	return (0);
}

static int WriteSlot(uint8_t ChipAddr, uint8_t slot, uint8_t *buf)
{
	uint16_t crc;
	int cnt, cnt2;
	unsigned char CommBuf[40];
	int size;

	// Set slot size
	if (slot <= 7)
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
			Serial.println("Failed to send write slot command");
			return(1);
		}

		delay(30);  // Wait for exec

		// Get responce
		if (4 != ReadAuthBytes(ChipAddr,CommBuf,4))
		{
			Serial.println("Failed to get write slot responce");
			return(1);
		}

		if (CommBuf[1]) {
			Serial.println("Error in write slot");
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
			Serial.println("Failed to send read config command");
			return(1);
		}

		delay(62);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(Batts[BatteryType].CryptoAddress,CommBuf,35))
		{
			Serial.println("Failed to read config area");
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

// ***FIX*** Check that below is correct!!!
	// Check serial number
	if (!NoCheckSerialNum)
	{
		if (AUTH_SN0 != CommBuf[0] || AUTH_SN1 != CommBuf[1] || AUTH_SN8 != CommBuf[12])
		{
			Serial.println("Bad SN");
			return(1);
		}
	}
	else if (Verbose)
			Serial.print("Skiping SN check: ");

	// Check first chunk, up to monotonic counters
	for (cnt2 = 16; cnt2 < 52; ++cnt2)
		if (CommBuf[cnt2] != g_config[cnt2])
		{
			Serial.print("Bad Config at ");
			Serial.println(cnt2);
			return(1);
		}

	if (0x55 == CommBuf[86])
	{
		Serial.print("Data area unlocked!!!");
		return(1);
	}

	if (0x55 == CommBuf[87])
	{
		Serial.print("Config area unlocked!!!");
		return(1);
	}

	for (cnt2 = 88; cnt2 < 128; ++cnt2)
		if (CommBuf[cnt2] != g_config[cnt2])
		{
			Serial.print("Bad Config at ");
			Serial.println(cnt2);
			return(1);
		}

	return (0);
}

/*
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
			Serial.println("Failed to send read config command");
			return(1);
		}

		delay(62);  // Wait for exec

		// Get responce
		if (35 != ReadAuthBytes(Batts[BatteryType].CryptoAddress,CommBuf,35))
		{
			Serial.println("Failed to read config area");
			return(1);
		}

		// Check data
		for (cnt2 = 0; cnt2 < 32; ++cnt2)
			if (CommBuf[cnt2+1] != 0xff)
			{
				Serial.println("OTP not blank");
				return(1);
			}
	}

	return (0);
}
*/

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

	delay(51);
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
// Returns count of bytes read
static int ReadAuthBytes(uint8_t ChipAddr, uint8_t Buffer[], uint8_t Bytes)
{
	uint8_t cnt, cnt2, BytesLeft, size;
	uint8_t get, got;
	uint16_t crc;

	if (CheckClk()) // Check for clock not "stuck"
		return(0);

	// get count
	got = Wire.requestFrom((uint8_t)(ChipAddr >> 1),(uint8_t)1);
	if (got < 1 || !Wire.available())
	{
		Serial.println("Error getting packet size on read");
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
		got = Wire.requestFrom((uint8_t)(ChipAddr >> 1),get);

		// Move over data
		for (cnt = 0; cnt < got; ++cnt)
			if (Wire.available())
			{
				if ((cnt+cnt2)>=Bytes)
				{
					Serial.println("Buffer to small for read");
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
		Serial.println("CRC error on read");
		return(0);
	}

	return(size);  // Return total count of bytes read
}

// Routine to write multiple bytes to the auth chip
// Returns pass/fail
int WriteAuthBytes(uint8_t ChipAddr, uint8_t Buffer[], uint8_t Bytes)
{
	uint8_t BytesLeft = Bytes, send, sent = 0;

	if (CheckClk()) // Check for clock not "stuck"
		return(1);

	while (BytesLeft)
	{
		// Start transmission
		Wire.beginTransmission(ChipAddr >> 1); // Set address to crypto chip

		if (BytesLeft >= 63)
			send = 63;
		else
			send = BytesLeft;

		Wire.write(3);
		if (Wire.write(Buffer+sent,send) != send)
		{
			Serial.println("Error writing data");
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

void AUTH_Update(bool ask)
{
	uint8_t LocalBuf[MAX_EEPROM_SIZE];

	if (!BatteryType)  // Check for valid battery type
		return;

	// Get data from the battery
	if (AUTH_Read(LocalBuf))
		return;

	switch (BatteryType)
	{
	case SENTRY_BATT:
	case GALACTUS_BATT:
		DoGiftedUpdate(0,LocalBuf);
		break;
	case METEOR_BATT:
	case COMET_BATT:
		DoVTupdate(LocalBuf);
		break;
	case PPP_V2:
		DoV2update(LocalBuf);
		break;
	default:
		not_sup();
		return;
		break;
	}

	// Write data back to battery
	AUTH_Write(LocalBuf,ask);
}

// Function to validate data in the auth chip
uint8_t Validate_AUTH(uint8_t type)
{
	uint8_t ret = 1;

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
	case METEOR_BATT_DATA:
		ret = ValidateVT((VALUE_TIER_BATT_DATA_t *)EEPROM_Data);
		break;
	case COMET_BATT_DATA:
		ret = ValidateVT((VALUE_TIER_BATT_DATA_t *)EEPROM_Data,COMET_BATT_DATA);
		break;
	case PPP_V2_DATA:
		if ((ret = newGG_GetStuff(&BD.NG)))
			break;
		ret = ValidatePPP_V2();
		break;
	default:
		Serial.println(Unsupported);
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
		Serial.println(Unsupported);
		ret = 1;
		break;
	}

	return (ret);
}

void AUTH_ShowConfigHex(void)
{
	uint8_t CommBuf[128];

	STATUS_LED_BUSY

	// Wake up crypto chip
	WakeCrypto();

	if (GetCryptoConfig(CommBuf))
	{
		STATUS_LED_READY
		return;
	}

	STATUS_LED_READY
	HEX_Write(128,CommBuf);
}

void AUTH_SaveConfigHex(void)
{
	uint8_t CommBuf[128];

	STATUS_LED_BUSY

	// Wake up crypto chip
	WakeCrypto();

	if (GetCryptoConfig(CommBuf))
	{
		STATUS_LED_READY
		return;
	}

	STATUS_LED_READY
	SaveFile(128,CommBuf);
}

void AUTH_ShowAllData(void)
{
	uint8_t SlotBuf[416];
	uint8_t CommBuf[128];
	uint8_t cnt;
	uint8_t hash[32];

	STATUS_LED_BUSY

	// Wake up crypto chips
	WakeCrypto();

	if (GetCryptoConfig(CommBuf))
	{
		STATUS_LED_READY
		return;
	}

	// Print serial number
	Serial.println();
	Serial.print("Serial:");
	for (cnt = 0; cnt <= 3; ++cnt)
		Serial.printf(" %02X",CommBuf[cnt]);
	for (cnt = 8; cnt <= 12; ++cnt)
		Serial.printf(" %02X",CommBuf[cnt]);
	Serial.println();
	// Print rev
	Serial.print("Rev:");
	for (cnt = 4; cnt <= 7; ++cnt)
		Serial.printf(" %02X",CommBuf[cnt]);
	Serial.println();


	// Wake up crypto chips
	WakeCrypto();

	// Get signer public key
	Serial.print("Signer public key");
	if (ReadSlot(Batts[BatteryType].CryptoAddress,SIGNER_PUB,SlotBuf))
	{
		STATUS_LED_READY
		return;
	}

	for (cnt = 0; cnt < 72; ++cnt)
	{
		if (!(cnt % 16))
			Serial.println();
		Serial.printf("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();

	// Get signer signature
	if (ReadSlot(Batts[BatteryType].CryptoAddress,SIGNER_SIG,SlotBuf))
	{
		STATUS_LED_READY
		return;
	}

	Serial.print("Signer sig");
	for (cnt = 0; cnt < 72; ++cnt)
	{
		if (!(cnt % 16))
			Serial.println();
		Serial.printf("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();

	// Get device public key
	if (ReadSlot(Batts[BatteryType].CryptoAddress,DEVICE_PUB,SlotBuf))
	{
		STATUS_LED_READY
		return;
	}

	Serial.print("Device Pub Key");
	for (cnt = 0; cnt < 72; ++cnt)
	{
		if (!(cnt % 16))
			Serial.println();
		Serial.printf("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();

	// Get device public key from private key
	if (GenPubKey(Batts[BatteryType].CryptoAddress,SlotBuf))
	{
		STATUS_LED_READY
		return;
	}

	Serial.print("Device Pub Key, from Prv Key");
	for (cnt = 0; cnt < 64; ++cnt)
	{
		if (!(cnt % 16))
			Serial.println();
		Serial.printf("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();

	// Get device signature
	if (ReadSlot(Batts[BatteryType].CryptoAddress,DEVICE_SIG,SlotBuf))
	{
		STATUS_LED_READY
		return;
	}

	Serial.print("Device sig:");
	for (cnt = 0; cnt < 72; ++cnt)
	{
		if (!(cnt % 16))
			Serial.println();
		Serial.printf("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();

	// Verify device private key matches device public key
	// Generate random 32 byte hash
	Random(hash);

	Serial.print("Hash:");
	for (cnt = 0; cnt < 32; ++cnt)
	{
		if(!(cnt%16))
			Serial.println();
		Serial.printf("0x%02X,",hash[cnt]);
	}
	Serial.println();

	// Have chip sign random number with private key
	if (Sign(Batts[BatteryType].CryptoAddress,hash,SlotBuf))
	{
		Serial.println("Bad signing");
		STATUS_LED_READY
		return;
	}

	Serial.print("Sig:");
	for (cnt = 0; cnt < 64; ++cnt)
	{
		if(!(cnt%16))
			Serial.println();
		Serial.printf("0x%02X,",SlotBuf[cnt]);
	}
	Serial.println();

	STATUS_LED_READY
}

// Function to update auth chip data from hex file
void Auth_Hex_Data_Update()
{
	File file;
	char hex[50];
	int cnt;
	uint8_t VerboseSaved;

	if (!BatteryType)  // Check for valid battery type
		return;

	ResetFS(); // Reset file system first

	// Set to terse, save current state
	VerboseSaved = Verbose;
	Verbose = 0;

	STATUS_LED_BUSY;

	// Check if battery is OK
	Serial.println("Checking battery");

	// Read GG data
	if (newGG_GetStuff(&BD.NG))
	{
		Serial.println("Invalid battery, can't update GG");
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	// Read auth chip data into the buffer
	AUTH_Read(BD.Buf);
	if (ValidatePPP_V2())
	{
		Serial.println();
		Serial.println("Invalid battery data, can't auto update hex data");
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	Serial.println("Checking auth chip");
	if (CheckAuthChip(false))
	{
		Serial.println();
		Serial.println("Non-Zebra auth chip, can't update GG");
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	Serial.println();
	ClearValid(); // Clear valid data flags

	// Drop rev from part number string
	for (cnt = 0; BD.NG.PartNumber[cnt]; ++cnt)
		if ('%' == BD.NG.PartNumber[cnt])
			BD.NG.PartNumber[cnt] = 0;

	// Get part number, create file names
	sprintf(hex,"GG/%s.hex",(char *)BD.NG.PartNumber);

	// Check hex file
	Serial.println("Checking HEX file");
	if (GetHexFile(hex,MAX_AUTH_SIZE,false))
	{
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	Serial.println();
	Serial.println("DO NOT REMOVE BATTERY TILL UPDATE DONE!!!");

	Serial.println();
	Serial.println("Updating Auth data");

	// Get file into buffer
	if (GetHexFile(hex,MAX_AUTH_SIZE,false))
	{
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}
	AUTH_Update(false);  // Do update

	// Recheck battery
	Serial.println();
	Serial.println("Rechecking battery");

	// Read auth chip data into the buffer
	ClearValid(); // Clear valid data flags
	AUTH_Read(BD.Buf);
	if (Validate_AUTH(PPP_V2_DATA))
	{
		Serial.println();
		Serial.println("Invalid battery, failed update");
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	Serial.println();
	Serial.println("Battery hex data update success!");
	Serial.println();

	Verbose = VerboseSaved; // Restore verbose setting
	STATUS_LED_READY;
}
