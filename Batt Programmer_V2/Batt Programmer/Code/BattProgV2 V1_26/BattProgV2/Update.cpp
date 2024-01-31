// Battery programming box
// EEPROM/Auth update code

#include <Arduino.h>
#include "Adafruit_SPIFlash.h"

#include <stdio.h>
#include <stdint.h>

#include "pwmbatt.h"
#include "BattProg.h"

// Function to update PP+ data
void DoGiftedUpdate(int StartAddr, uint8_t *Buf)
{
	int cnt;

	Serial.println("Doing PP+ data update...");

	// The data blocks 0-2 is copied over from the new data.
	for (cnt = 0; cnt < (3*16); ++cnt)
		Buf[cnt+StartAddr] = EEPROM_Data[cnt+StartAddr];

	// The data in blocks 3-7 is left untouched. (Dynamic blocks)

	// The data in blocks 8-14 is copied from the new data.
	for (cnt = (8*16); cnt < (15*16); ++cnt)
		Buf[cnt+StartAddr] = EEPROM_Data[cnt+StartAddr];

	// The data in blocks 15-16 is left untouched. (Cell identifying data)

	// The data in blocks 17-31 (25 for auth chip data) is copied from the new data.
	for (cnt = (17*16); cnt < (32*16); ++cnt)
		Buf[cnt+StartAddr] = EEPROM_Data[cnt+StartAddr];
}

// Function to update PP+ V2 data
uint8_t DoV2update(uint8_t *Buf)
{
	int cnt;

	Serial.println("Doing PP+ V2 data update...");

	// Bytes 0-147 will be updated.
	for (cnt = 0; cnt <= 147; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	// Bytes 148-199 will be left untouched. (Initial times, agg charge, health)

	// Bytes 200-631 will be updated.
	for (cnt = 200; cnt <= 631; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	return (0);
}

// Function to update VT PP data
uint8_t DoVTupdate(uint8_t *Buf)
{
	int cnt;
	uint8_t csum;

	Serial.println("Doing VT data update...");

	// The format revision byte will be updated, only values 1-2 are supported.
	if (EEPROM_Data[1] < 1 || EEPROM_Data[1] > 3)
	{
		Serial.println(BadRev);
		return (1);
	}

	// Bytes 1-21 will be updated.
	for (cnt = 1; cnt <= 21; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	// Bytes 22-30 will be left untouched. (serial number, date made)

	// Bytes 31-392 will be updated.
	for (cnt = 31; cnt <= 392; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	// Bytes 393-424 will be left untouched. (Cell ident data)

	// Bytes 425-431 will be updated.
	for (cnt = 425; cnt <= 431; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	// The checksum at byte 0 will be updated.
	csum = DoChecksum(Buf+1,Buf+431);
	Buf[0] = ~csum + (uint8_t)1;

	// Bytes 432-439 will be left untouched. (Health)
	// Bytes 440-447 will be left untouched. (aggregate charge)

	// Bytes 448-631 will be updated.
	for (cnt = 448; cnt <= 631; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	return (0);
}

// Function to update QC PP data
uint8_t DoPolluxUpdate(uint8_t *Buf)
{
	int cnt;
	uint8_t csum;

	Serial.println("Doing QC PP data update...");

	// The format revision byte will be updated, only values 0-3 are supported.
	if (EEPROM_Data[1] > 3)
	{
		Serial.println(BadRev);
		return (1);
	}
	
	// Bytes 1-5 will be updated.
	for (cnt = 1; cnt <= 5; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	// Bytes 6-12 will be left untouched. (serial number, date made)

	// Bytes 13-239 will be updated.
	for (cnt = 13; cnt <= 239; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	// The checksum at byte 0 will be updated.
	csum = DoChecksum(Buf+1,Buf+239);
	Buf[0] = ~csum + (uint8_t)1;

	// Bytes 240-252 will be left untouched. (aggregate charge)

	// Bytes 253-295 will be updated.
	for (cnt = 253; cnt <= 295; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	// Bytes 296-303 will be left untouched. (Health)

	// Bytes 304-511 will be updated.
	for (cnt = 304; cnt <= 511; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	return (0);
}

// Function to update Hawkeye PP data
uint8_t DoHawkeyeUpdate(uint8_t *Buf)
{
	int cnt;
	uint8_t csum;

	Serial.println("Doing Hawkeye data update...");

	// The format revision byte will be updated, only values 0-3 are supported.
	if (EEPROM_Data[1] > 3)
	{
		Serial.println(BadRev);
		return (1);
	}
	Buf[1] = EEPROM_Data[1];

	// Bytes 4-39 will be left untouched. (Part number, rev, serial number, date made)

	// Bytes 40-199 will be updated.
	for (cnt = 40; cnt <= 199; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	// The checksum at byte 0 will be updated.
	csum = DoChecksum(Buf+1,Buf+199);
	Buf[0] = ~csum + (uint8_t)1;

	// Bytes 201-215 will be left untouched. (aggregate charge)

	// Bytes 216-251 will be updated.
	for (cnt = 216; cnt <= 251; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	// Bytes 252-259 will be left untouched. (Health)

	// Bytes 260-415 will be updated.
	for (cnt = 260; cnt <= 415; ++cnt)
		Buf[cnt] = EEPROM_Data[cnt];

	return(0);
}
