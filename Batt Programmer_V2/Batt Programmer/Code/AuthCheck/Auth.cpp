// Battery programmer auth chip functions

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <Arduino.h>
#include <Wire.h>	// I2C lib

#include <Wire.h>	// I2C lib

#include "Auth.h"

// Function prototypes
static uint16_t GetCRC(int len, uint8_t *buffer);
static void WakeCrypto(void);
static int CheckCryptoConfig(void);
static int ReadSlot(int ChipAddr, uint8_t slot, uint8_t *buf);
static int Sign(int ChipAddr, unsigned char *in_buf, unsigned char *out_buf);
static LoadTempReg(int ChipAddr, unsigned char *in_buf);
static int ReadAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes);
static int WriteAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes);
static int VerifySig(uint8_t *Sig, uint8_t *PubKey);

// Slot defines
#define DEVCIE_PRI		0
#define DEVICE_PUB		9
#define DEVICE_SIG		10
#define SIGNER_PUB		11
#define SIGNER_SIG		12

PROGMEM const uint8_t g_PubKey[KEY_LEN] = {
	0xd6, 0xb7, 0x42, 0xb0, 0x29, 0xae, 0x01, 0x77, 0x5a, 0x20, 0x23, 0x5a, 0xd4, 0x63, 0xf2, 0x02,
	0xa6, 0xc9, 0xb1, 0xef, 0x03, 0xf4, 0xfd, 0xfb, 0x6f, 0x17, 0xfd, 0xa9, 0x30, 0x5a, 0xb7, 0x77,
	0xea, 0x83, 0x1c, 0x27, 0x9f, 0x3d, 0x03, 0x71, 0x91, 0x07, 0x8a, 0x1b, 0x98, 0x33, 0x04, 0xb2,
	0x1f, 0x10, 0x61, 0xbf, 0x1e, 0xb3, 0x37, 0xdf, 0x3f, 0xea, 0x04, 0x21, 0xaa, 0x8a, 0x70, 0x55
};

// Use local chip to generate a random number
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

	// Copy over result
	for (cnt = 0; cnt < 32; ++cnt)
		rnd[cnt] = CommBuf[cnt+1];
	return (0);
}

// Verify ECDSA sig using local chip
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

	if (CommBuf[1])
	{
		Serial.println(F("Bad sig!"));
		return(1);
	}
	else
		return (0);
}

// Do a SHA256 hash using local chip
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

	return (0);
}

// Function to check if a battery auth chip is valid
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

	// Get signer public key
	Serial.print(F("Check signer public key: "));
	if ((ret = ReadSlot(BATT_AUTH_ADDR,SIGNER_PUB,SlotBuf)))
		return (1);

	for (cnt = 0; cnt < 32; ++cnt)
		SignerPubKey[cnt] = SlotBuf[cnt+4];
	for (cnt = 32; cnt < KEY_LEN; ++cnt)
		SignerPubKey[cnt] = SlotBuf[cnt+8];

	// Get signer signature
	if (ReadSlot(BATT_AUTH_ADDR,SIGNER_SIG,SlotBuf))
		return (1);

	// Do a SHA256 hash of signer public key
	if (SHA256Hash(SignerPubKey,KEY_LEN))
		return (1);

	// Copy Zebra public key from flash
	for (cnt = 0; cnt < KEY_LEN; ++cnt)
		DevicePubKey[cnt] = pgm_read_byte(g_PubKey+cnt);

	// Check that signer public key was signed by Zebra root private key
	if (VerifySig(SlotBuf,DevicePubKey))
		return (1);

	Serial.println(F("Passed"));

	// Check for valid device signature
	// Get device public key
	Serial.print(F("Check device public key: "));
	if ((ret = ReadSlot(BATT_AUTH_ADDR,DEVICE_PUB,SlotBuf)))
		return (1);

	for (cnt = 0; cnt < 32; ++cnt)
		DevicePubKey[cnt] = SlotBuf[cnt+4];
	for (cnt = 32; cnt < KEY_LEN; ++cnt)
		DevicePubKey[cnt] = SlotBuf[cnt+8];

	// Get device signature
	if (ReadSlot(BATT_AUTH_ADDR,DEVICE_SIG,SlotBuf))
		return (1);

	// Do a SHA256 hash of device public key
	if (SHA256Hash(DevicePubKey,KEY_LEN))
		return (1);

	// Check that device public key was signed by signer private key
	if (VerifySig(SlotBuf,SignerPubKey))
		return (1);

	Serial.println(F("Passed"));

	Serial.print(F("Check device private key: "));

	// Verify device private key matches device public key
	// Generate random 32 byte hash
	Random(hash);

	// Have chip sign random number with private key
	if (Sign(BATT_AUTH_ADDR,hash,SlotBuf))
	{
		Serial.println(F("Bad signing"));
		return (1);
	}

	// Check that random number key was signed by device private key
	if (LoadTempReg(LOCAL_AUTH_ADDR, hash))
		return (1);
	if (VerifySig(SlotBuf,DevicePubKey))
		return (1);

	Serial.println(F("Passed"));

	return (0);
}

// Load the auth chip temp reg
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

	if (CommBuf[1])
	{
		Serial.println(F("Error in nonce command"));
		return(1);
	}

	return (0);
}

// Perform ECDSA sign
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
		Serial.println(F("Failed to get sign response"));
		return(1);
	}

	for (cnt = 0; cnt < KEY_LEN; ++cnt)
		out_buf[cnt] = CommBuf[cnt+1];

	return (0);
}

// Read data from auth chip slot
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

		for (cnt2 = 0; cnt2 < (size>=32 ? 32 : size); ++cnt2)
			buf[cnt+cnt2] = CommBuf[cnt2+1];

		cnt += 32;
		size -= 32;
	}

	return (0);
}

// Wake up both auth chips
static void WakeCrypto(void)
{
	unsigned char buf[2] = {0};

	// Write idle command to local chip
	Wire.beginTransmission(LOCAL_AUTH_ADDR>>1); // Set address to crypto chip
	buf[0] = 2;
	Wire.write(buf,1);
	Wire.endTransmission(); // Do idle write, Don't check for error, might fail
	delay(2);

	// Write idle command to battery chip
	Wire.beginTransmission(BATT_AUTH_ADDR>>1); // Set address to crypto chip
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

// Calculate the CRC for auth chip packages
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
	uint8_t cnt, cnt2, BytesLeft, size;
	int get, got;
	uint16_t crc;

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

// Routine to write multiple bytes to an auth chip
// Returns pass/fail
static int WriteAuthBytes(int ChipAddr, uint8_t Buffer[], uint8_t Bytes)
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
