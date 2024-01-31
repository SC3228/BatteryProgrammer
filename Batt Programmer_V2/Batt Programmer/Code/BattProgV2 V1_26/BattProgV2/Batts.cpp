// Battery data table

#ifdef ON_PI  // Building for Pi?
	#include <stdint.h>
	#include <python3.7/Python.h>	// Needed for Python interface
	#include <stdio.h>
#else
	#include <Arduino.h>
	#include "Adafruit_SPIFlash.h"
#endif

#include "pwmbatt.h"
#include "Batts.h"
#include "BattProg.h"

// NOTE: All battery type# defined in BattProg.h. Keep in sync!!!
const char NoBatt[] = "No Battery";
const char MPA2PP[] = "MPA2 PP";
const char MC95[] = "MC95 PP";
const char MC18[] = "MC18 PP+";
const char Rouge[] = "Rouge/TC8000";
const char Frenzy[] = "Frenzy/WT6000";
const char IronMan[] = "IronMan/Lightning";
const char Pollux[] = "Pollux";
const char Falcon[] = "Falcon/Thunder";
const char Hawkeye[] = "Hawkeye/TC20";
const char Sentry[] = "Sentry/Elektra";
const char Galactus[] = "Galactus/Badger/Firebird/Frozone";
// const char Frozone[] = "Frozone EEPROM";
const char Meteor[] = "Value Tier Battery";
const char VTeeprom[] = "Value Tier Battery w/EEPROM";
const char NewGauge[] = "PP+ V2 Battery";

// Supported battery table
const BattData Batts[MAX_BATTERY_TYPES] = {
	// No battery
	{0,0,0,0,0,
		0,0,0,
		PULLUP_VBATT,PULLUP_NONE,0,
		NO_KEYS,NoBatt,MIN_VOLTAGE,
		0},
	// MPA2 PP
	{MPA2_EEPROM_ADDR,MPA2_TMP_ADDR,0,0,0,
		MPA2_EEPROM_SIZE,MPA2_PAGE_SIZE,0,
		PULLUP_V3p3,PULLUP_2p9K,1,
		NO_KEYS,MPA2PP,MIN_VOLTAGE,
		0},
	// MC95 PP
	{MPA2_EEPROM_ADDR,MPA2_TMP_ADDR,0,0,0,
		MPA2_EEPROM_SIZE,MPA2_PAGE_SIZE,0,
		PULLUP_V3p3,PULLUP_2p9K,0,
		NO_KEYS,MC95,3100,
		0},
	// MC18 PP+
	{MPA3_EEPROM_ADDR,MPA3_TMP_ADDR,MPA3_GG_ADDR,0,0,
		MPA3_EEPROM_SIZE,MPA3_PAGE_SIZE,0,
		PULLUP_V3p3,PULLUP_2p9K,1,
		SUNRISE_KEYS,MC18,MIN_VOLTAGE,
		0},
	// Rouge
	{MPA3_EEPROM_ADDR,MPA3_TMP_ADDR,MPA3_GG_ADDR,0,0,
		MPA3_EEPROM_SIZE,MPA3_PAGE_SIZE,0,
		PULLUP_V3p3,PULLUP_3p4K,1,
		SUNRISE_KEYS,Rouge,MIN_VOLTAGE,
		0},
	// Frenzy
	{MPA3_EEPROM_ADDR,MPA3_TMP_ADDR,MPA3_GG_ADDR,0,0,
		MPA3_EEPROM_SIZE,MPA3_PAGE_SIZE,0,
		PULLUP_V3p3,PULLUP_3p4K,1,
		FREEPORT_KEYS,Frenzy,MIN_VOLTAGE,
		0},
	// IronMan
	{MPA2_EEPROM_ADDR,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,0,
		QC_EEPROM_SIZE,QC_PAGE_SIZE,AUTH_EEPROM_SIZE,
		PULLUP_VBATT,PULLUP_2p3K,0,
		IRONMAN_KEYS,IronMan,MIN_VOLTAGE,
		1},
	// Pollux
	{MPA2_EEPROM_ADDR,0,0,0,0,
		POLLUX_EEPROM_SIZE,POLLUX_PAGE_SIZE,0,
		PULLUP_VBATT,PULLUP_5p1K,0,
		NO_KEYS,Pollux,MIN_VOLTAGE,
		1},
	// Falcon
	{MPA2_EEPROM_ADDR,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,0,
		QC_EEPROM_SIZE,QC_PAGE_SIZE,AUTH_EEPROM_SIZE,
		PULLUP_VBATT,PULLUP_5p1K,0,
		FALCON_KEYS,Falcon,MIN_VOLTAGE,
		1},
	// Hawkeye
	{0,0,0,AUTH_CHIP_ADDR,1,
		0,0,0,
		PULLUP_VBATT,PULLUP_NONE,0,
		NO_KEYS,Hawkeye,MIN_VOLTAGE,
		1},
	// Sentry
	{MPA2_EEPROM_ADDR,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,1,
		QC_EEPROM_SIZE,QC_PAGE_SIZE,MPA3_EEPROM_SIZE,
		PULLUP_VBATT,PULLUP_5p1K,0,
		FALCON_KEYS,Sentry,MIN_VOLTAGE,
		1},
	// Galactus Auth data
	{0,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,1,
		0,0,AUTH_EEPROM_SIZE,
		PULLUP_VBATT,PULLUP_2p3K,0,
		FALCON_KEYS,Galactus,MIN_VOLTAGE,
		1},
/*
	// Frozone EEPROM
	{MPA2_EEPROM_ADDR,0,0,0,0,
		POLLUX_EEPROM_SIZE,POLLUX_PAGE_SIZE,0,
		PULLUP_VBATT,PULLUP_5p1K,0,
		NO_KEYS,Frozone,MIN_VOLTAGE,
		1},
*/
	// Value Tier Auth data
	{0,0,0,AUTH_CHIP_ADDR,1,
		0,0,MAX_AUTH_SIZE,
		PULLUP_VBATT,PULLUP_2p3K,0,
		NO_KEYS,Meteor,MIN_VOLTAGE,
		1},
	// VT w/EEPROM
	{MPA2_EEPROM_ADDR,0,0,AUTH_CHIP_ADDR,0,
		VT_EEPROM_SIZE,VT_PAGE_SIZE,MAX_AUTH_SIZE,
		PULLUP_VBATT,PULLUP_2p3K,1,
		NO_KEYS,VTeeprom,MIN_VOLTAGE,
		0},
	// New gauge V2 PP+ battery
	{0,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,1,
		0,0,MAX_AUTH_SIZE,
		PULLUP_VBATT,PULLUP_2p3K,0,
		NO_KEYS,NewGauge,MIN_VOLTAGE,
		1},
};

