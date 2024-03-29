// Battery programmer/reader program #1

#define VERSION "2.19"

/*
2.19	Fixed Auth chip delays that were too short
		Fix error in dsiplying LDMD and RDIS bits in the V2 GG reg
		General cleanup

2.18	Added blank auth chip checking

2.17	Added check to prevent hibernating new gas gauge batt

2.16	Added initial support for displaying Hawkeye battery data

2.15	Added support for dggf and sggf commands to PP+ V2 batts.

2.14	Fixed a typo in the MV2 command that set the initial data rev to 10 instead of 1.
		Added addtional format version checking to PPV2 validation.

2.13	Added code to "manufacture" PP+ V2 batts
		Added full unseal command

2.12	Inital released support for PP+ V2 batts.
		Reduced code size by simplifying validation error messages.
		Simplified display of GG registers.
		Reduced code size by using common flash based messages where possible.
		Added display data for MPA2 smart batts

2.11	Changed all auth chip batts to use 632 byte hex files.

2.10	Fixed display of full available capacity, it was showing nominal available capacity.

2.9		Cleaned up messages when checking PP+ factory static block.
		Fixed missing check of startup voltage when checking Pollux data.
		Added checking of PP SD660 data stored in PP+ areas
		Added text in verbose mode for EEPROM block allocation testing.

2.8		Added more verbose text when validating MPA2 and Hawkeye batts.
		Force JDM mode to always be non-verbose.
 
2.7		Added more verbose text messages when validating Pollux data.
		Fixed a bug that disabled auth chip serial number checks by default.
		Added message when battery fails validation, so you don't miss it.

2.6		Moved Elektra batts from Falcon to Sentry type.

2.5		Fixed Sentry checking of Pollus data.

2.4		Fixed handling of Sentry batts to account for split in data
		from EEPROM and Auth chip.
		Added support for showing size of both auth and EEPROM on
		batts that have both, reguardless of where data is stored.
		Added code to check if files being loaded are for auth or EEPROM
		to properly check for hex file size.

2.3		Fixed problem with hex file read from serial using
		WaitForKey() to get chars.

2.2		Added temp command to fix new gauge test batts "ID" block.
		Minor fixups found while converting code for Python lib.
		Added seperate VT_Valid flag.
		Added checks for rev 2 VT data format
		Fix for SD fat lib changing GetFileName to GetName
		Cleanup in VT data "Show"
		Added 'Update' function for VT batts

2.1		Value tier data format support finished
		Stored builtin auth chip SN string to remove need to read it every time.

2.0		Inital support for the new gas gauge
		Inital value tier support (Support for 632 byte auth data)

1.44	Added a check for the record type when validating the Pollux battery health data record.

1.43	Added display of 1725 temp limits to Pollux and PP+ data.

1.42	Fixed a bug where the sunrise key was being used on Frenzy batts

1.41	Cleaned up printing of cell ident data, masked
		non-printable characters.

1.40	Added extra battery names to select battery type list.
		Added support for Frozone EEPROM

1.39	Fixed a bug that was not updating the part number/rev in Pollux batts
		when doing an update EEPROM.

1.38	Added WA command to Falcon/Ironman batts.
		Removed unneeded battery types: sentry auth. beast auth & EEPROM,
		Lightning auth
		Added Galactus battery type. Only uses auth chip. no EEPROM support.

1.37	Added Ship Mode data display/checking

1.36	Fixed display of QC serial number data

1.35	Added seal/unseal gas gauge commands.

1.34	Added support for MC95 display batts.

1.33	Added ability to overide check of auth chip serial number.
		Skipped monotonic counters and last use key in auth config check.

1.32	Added display of current from gas gauge in DGG command.

1.31	Added Lightning Auth Chip battery type

1.30	Added display of JEITA data to DPP+ command

1.29	Re-factored auth code to save some flash/ram space.
		Added SAD command to dump all auth chip checking data

1.28	Fixed Pollux batts to be 512 bytes not 256

1.27	Added a warning when trying to overwrite a battery about un-making it.
		Added code to flush buffer before doing a VBD command so old data is not
		checked after a bad read.
		Replaced reset with unseal to exit sealed mode.
		Added commands to access Auth chip config area and keys/sigs used
		during the authentication process.

1.26	Removed the scrolling numbers when reading/writing the EEPROM/Auth chip
		Went to Verbose mode on as the default

1.25	Added DGGF - Dump gas gauge flash as hex file
		Added SGGF - Save gas gauge flash as hex file

1.24	Added OLV - Override low voltage check command
		Added Verbose mode
		Added VERB - Toggle verbose mode command

1.23	Changed minimum battery voltage to 3.3V to leave margin
		for I2C port detecting a high level. (Min in high is 3V@5V VCC)
		Changed the validate command to be VBD - Validate battery data
		Added check for battery connected on power up

1.22	Added status command
		Removed status from help menu
		Added ability to select "no battery" to disconnect
		the I2C pullups.

1.21	Added command to set the FULLSLEEP bit
		Added command to show version information
		Modified some command names to make them simpler to remember 

1.20	Added commands to display the gas gauge registers
		Added command to hibernate the gas gauge
		Added code to detect battery in/out
		Added command to reset the gas gauge

1.19	Added "for internal use only" message

1.18	Fixed an error where verifies did not get called if
		the buffer was non-blank
		Removed extra call to FlushBuffer() in read file function

1.17	Added display of seconds since first use
		Fixed a bug in the SD660 area checksum calculation
		Cleaned up the "Continue Operation" question look.
		Added code to show used/free blocks checked
		Changed message for unprogrammed manf blocks to say "Note"
		Fixed message for no ID in block 2, had said blk1
		Put more status messages during PP+ validation
		Fixed display of SD660 data
		Fixed checking of free data blocks
		Fixed display of ident data

1.16	Added UVLO, OVLO, and cutoff voltage to SD660 area printout
		Expanded allowed characters in file names
		Added ESC for cancel in GetFileName()
		Added initial cut at "Manufacture Battery"
		Changed GetFileName to GetText
		Fixed a bug that caused the unencrypted data from the
		SDM660 and Ident blocks to be used when the blocks
		were marked as free. This caused free block errors
		in blocks 14 and 15.
		Fixed a bug where the command buffer Buf[] in Loop() was not
		declared static, and not initialized properly

1.15	Fixed display PP+ data not failing on blank EEPROM

1.14	First "JDM" version

1.13	Modified help menu to show auth data read write
		commands for any battery with an auth chip
		Added code to return a failed status when user
		continues validation after any failure

1.12	Added "KeepGoing" function to allow validation
		to continue on an error if user wants
		Added code to check for a blank buffer
		before certain operations
		Added validation of Hawkeye data
		Fixed a bug in Pollux validation that skipped
		checking the checksum on the charger control data
		Redisplayed current command line buffer after
		getting an SD card In/Out message

1.11	Added display of Pollux/8956 data
		Added display of 660 data in PP+ area
		Changed battery validation to use data already
		in buffer instead of reading from chip

1.10	Fix for Beast showing too much hex data
		Fix for stuck TWI when batt is removed/replaced
		Added Pollux/8956 data validation

1.9 Added initial support for Beast

1.8	Added code to limit JDM device capability
	Added validation of PP+ data
	Added in GG reg read

1.7 Added framework for validate data support.
	Removed checking of slot 8 for default data.
	Removed check for blank OTP area.
	Fixed extra "Passed" when checking crypto chip config.

1.6	Fixed bug that prevented directory command from working
	Added EEPROM update support

1.5	Added bounds checking on hex reads

1.4	Added multiple character commands.

1.3	Added defs for all the battery type numbers in BattProg.h

1.2 Added flag for clock switch control needed
	Added code to check for successful I2C writes
	Added code to check for a "stuck" clock before transactions

1.1	Added auth data read write

1.0	Initial release
*/

// Includes
#include <SdFat.h>	// SD card stuff
#include <Wire.h>	// I2C lib

#include "pwmbatt.h"
#include "BattProg.h"
#include "aes.h"
#include "Auth.h"
#include "Manufacture.h"
#include "EEPROM.h"
#include "GasGauge.h"

// List of commands
const char *Cmds[] = { "?","VER","STAT","B"
						,"RE","RA","WE","WA"
						,"UE","UA","VE","VA"
						,"E","LS","DS"
						,"LF","SF","DIR"
						,"VBD","DPP","DPP+"
						,"A","DAC","SAC", "SAD"
						,"DGG","DGGH","HGG","FSGG","RGG"
						,"SGG","USGG","FUSGG"
						,"MB","OLV","OSN","VERB"
						,"DGGF","SGGF"
						,"DGGC","SGGC"
						,"MV2"
						};

enum Command { HELP_MENU, SHOW_VERSION, SHOW_STATUS, SELECT_BATTERY
				, READ_EEPROM, READ_AUTH, WRITE_EEPROM, WRITE_AUTH
				, UPDATE_EEPROM, UPDATE_AUTH, VERIFY_EEPROM, VERIFY_AUTH
				, ERASE_BUFFER, LOAD_HEX, DUMP_HEX
				, LOAD_FILE, SAVE_FILE, DIRECTORY
				, VALIDATE_BATTERY, SHOW_PP, SHOW_PPP
				, AUTHENTICATE, DUMP_AUTH_CONFIG, SAVE_AUTH_CONFIG, SHOW_AUTH_DATA
				, SHOW_GG, SHOW_GG_HEX, ENTER_HIBERNATE, SET_FULLSLEEP, RESET_GG
				, SEAL_GG, UNSEAL_GG, FULL_UNSEAL_GG
				, MANUFACTURE, OVERRIDE_LOW_VOLTAGE, OVERRIDE_SERIAL_NUM, TOGGLE_VERBOSE
				, DUMP_GAS_GAUGE_FLASH, SAVE_GAS_GAUGE_FLASH
				, DUMP_GAS_GAUGE_CONFIG, SAVE_GAS_GAUGE_CONFIG
				, MANUF_V2
				, NO_CMD };

// Auth serial number string
char AuthSN[32] = "";

// JDM/Zebra stuff
uint8_t JDM_Mode = 1;

// Battery data table stuff
// NOTE: All battery type# defined in BattProg.h. Keep in sync!!!
const char NoBatt[] PROGMEM = "No Battery";
const char MPA2PP[] PROGMEM = "MPA2 PP";
const char MC95[] PROGMEM = "MC95 PP";
const char MC18[] PROGMEM = "MC18 PP+";
const char Rouge[] PROGMEM = "Rouge/TC8000";
const char Frenzy[] PROGMEM = "Frenzy/WT6000";
const char IronMan[] PROGMEM = "IronMan/Lightning";
const char Pollux[] PROGMEM = "Pollux";
const char Falcon[] PROGMEM = "Falcon/Thunder";
const char Hawkeye[] PROGMEM = "Hawkeye/TC20";
const char Sentry[] PROGMEM = "Sentry/Elektra";
const char Galactus[] PROGMEM = "Galactus/Badger/Firebird/Frozone Auth";
const char Frozone[] PROGMEM = "Frozone EEPROM";
const char Meteor[] PROGMEM = "Value Tier Battery";
const char NewGauge[] PROGMEM = "PP+ V2 Battery";

// Supported battery table
BattData Batts[MAX_BATTERY_TYPES] = {
	// No battery
	{0,0,0,0,0,
		0,0,
		PULLUP_V0,PULLUP_NONE,0,
		NO_KEYS,NoBatt,MIN_VOLTAGE},
	// MPA2 PP
	{MPA2_EEPROM_ADDR,MPA2_TMP_ADDR,0,0,0,
		MPA2_EEPROM_SIZE,MPA2_PAGE_SIZE,
		PULLUP_V3p3,PULLUP_2p9K,1,
		NO_KEYS,MPA2PP,MIN_VOLTAGE},
	// MC95 PP
	{MPA2_EEPROM_ADDR,MPA2_TMP_ADDR,0,0,0,
		MPA2_EEPROM_SIZE,MPA2_PAGE_SIZE,
		PULLUP_V3p3,PULLUP_2p9K,0,
		NO_KEYS,MC95,3.1},
	// MC18 PP+
	{MPA3_EEPROM_ADDR,MPA3_TMP_ADDR,MPA3_GG_ADDR,0,0,
		MPA3_EEPROM_SIZE,MPA3_PAGE_SIZE,
		PULLUP_V3p3,PULLUP_2p9K,1,
		MPA3_KEYS,MC18,MIN_VOLTAGE},
	// Rouge
	{MPA3_EEPROM_ADDR,MPA3_TMP_ADDR,MPA3_GG_ADDR,0,0,
		MPA3_EEPROM_SIZE,MPA3_PAGE_SIZE,
		PULLUP_V3p3,PULLUP_3p4K,1,
		MPA3_KEYS,Rouge,MIN_VOLTAGE},
	// Frenzy
	{MPA3_EEPROM_ADDR,MPA3_TMP_ADDR,MPA3_GG_ADDR,0,0,
		MPA3_EEPROM_SIZE,MPA3_PAGE_SIZE,
		PULLUP_V3p3,PULLUP_3p4K,1,
		MPA3_KEYS,Frenzy,MIN_VOLTAGE},
	// IronMan
	{MPA2_EEPROM_ADDR,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,0,
		QC_EEPROM_SIZE,QC_PAGE_SIZE,
		PULLUP_VBATT,PULLUP_2p3K,0,
		IRONMAN_KEYS,IronMan,MIN_VOLTAGE},
	// Pollux
	{MPA2_EEPROM_ADDR,0,0,0,0,
		POLLUX_EEPROM_SIZE,POLLUX_PAGE_SIZE,
		PULLUP_VBATT,PULLUP_5p1K,0,
		NO_KEYS,Pollux,MIN_VOLTAGE},
	// Falcon
	{MPA2_EEPROM_ADDR,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,0,
		QC_EEPROM_SIZE,QC_PAGE_SIZE,
		PULLUP_VBATT,PULLUP_5p1K,0,
		FALCON_KEYS,Falcon,MIN_VOLTAGE},
	// Hawkeye
	{0,0,0,AUTH_CHIP_ADDR,1,
		0,0,
		PULLUP_VBATT,PULLUP_NONE,0,
		NO_KEYS,Hawkeye,MIN_VOLTAGE},
	// Sentry
	{MPA2_EEPROM_ADDR,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,1,
		QC_EEPROM_SIZE,QC_PAGE_SIZE,
		PULLUP_VBATT,PULLUP_5p1K,0,
		FALCON_KEYS,Sentry,MIN_VOLTAGE},
	// Galactus Auth data
	{0,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,1,
		0,0,
		PULLUP_VBATT,PULLUP_2p3K,0,
		FALCON_KEYS,Galactus,MIN_VOLTAGE},
	// Frozone EEPROM
	{MPA2_EEPROM_ADDR,0,0,0,0,
		POLLUX_EEPROM_SIZE,POLLUX_PAGE_SIZE,
		PULLUP_VBATT,PULLUP_5p1K,0,
		NO_KEYS,Frozone,MIN_VOLTAGE},
	// Value Tier Auth data
	{0,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,1,
		0,0,
		PULLUP_VBATT,PULLUP_2p3K,0,
		NO_KEYS,Meteor,MIN_VOLTAGE},
	// New gauge test battery
	{0,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,1,
		0,0,
		PULLUP_V3p3,PULLUP_2p3K,0,
		NO_KEYS,NewGauge,MIN_VOLTAGE},
};

SdFat SD;	// SD card file system object
uint8_t HaveSD = 0;  // Have an SD card flag
uint8_t SD_WP = 0;  // SD is write protected flag

// Hex file download global variables
uint8_t HadError = 0;  // Error flag
uint8_t CSum; // Checksum

// Selected battery type
uint8_t BatteryType = 0;
uint8_t HaveBattery = 0;  // Have a battery flag

// Clock control
uint8_t SwitchClk = 0;

// EEPROM data buffer
uint8_t EEPROM_Data[MAX_EEPROM_SIZE];

// Buffer blank flag
uint8_t BufferBlank = 1;

// Skip voltage check flag
uint8_t NoCheckVoltage = 0;

// Skip auth serial number check flag
uint8_t NoCheckSerialNum = 0;

uint8_t Verbose = 1;  // Use verbose output flag

void setup()
{
	uint8_t cnt = 0;

	// Setup GPIOs
	InitPullupPins();
	digitalWrite(WP,LOW);  // Set WP to input, no pull up
	pinMode(WP,INPUT);
	digitalWrite(CD,LOW);  // Set CD to input, no pull up
	pinMode(CD,INPUT);

	// Turn off the digital input buffer on A0
	DIDR0 = _BV(ADC0D);

	// Setup serial port
//	Serial.begin(115200);
	Serial.begin(9600);

	ShowVersion();

	// Setup I2C
	SetVoltage(PULLUP_V3p3);  // Set to 3.3V pullup voltage
	SetResistor(PULLUP_10K); // Set to 10K pullup resistors
	PassiveClk();  // Default to passive clock drive

	while (BattVoltage() > 0.3)
	{
		if (!cnt)
		{
			Serial.println(F("Battery voltage detected, please remove battery to proceed."));
			Serial.println();  
			cnt = 20;
		}
		delay(100);
		--cnt;
	}

	StartWire(); // restart Wire stuff

	// Show serial number/version and set mode
	while (AUTH_ShowSN())
	{
		FlushSerial();
		Serial.println(F("Invalid auth chip, hit any key..."));
		while(!Serial.available())
		;
		Serial.read();
	}

	SetVoltage(PULLUP_V0);  // Default to 0V pullup voltage
	SetResistor(PULLUP_NONE); // Default to no pullup resistors

	// Check for correct "mode"
#ifdef ZEBRA_MODE
	Serial.println(F("For Zebra Inc. internal use ONLY!!!"));
	if (JDM_Mode)
	{
		Serial.println(F("Invalid software for this device!!!"));
		while(1)
			delay(1000);
	}
#else
	Verbose = 0;
#endif

	ClearValid();  // Set data to not valid

	FlushBuffer(0); // Blank data buffer

	CheckSD();  // Check for change in SD card status
	CheckBattery();  // Check for battery in/out

	Status();

	Serial.print(F("> ")); // Print prompt
}

void loop()
{
	static uint8_t inch, done = 0, pos = 0, ret = 0;
	static char buf[10] = {0};
	enum Command which;

	// Check for change in SD card status
	if (CheckSD() || CheckBattery())
	{
		Serial.println(); // Print prompt
		Serial.print(F("> "));
		Serial.print(buf);  // Print current buffer
	}

	if (Serial.available())
	{
		inch = Serial.read();

		// CR?
		if (CR == inch)
		{
			Serial.println();  // New line
			done = 1;
		}
		else if (BKSP == inch) // Backspace
		{
			if (pos > 0) // Can we move back?
			{
				--pos;
				buf[pos] = 0;  // Terminate string
				Serial.print(F("\010 \010")); // Clear character on screen
			}
		}
		else if ((inch >= 'a' && inch <= 'z') // Is it a valid character?
				|| (inch >= 'A' && inch <= 'Z')
				|| (inch >= '0' && inch <= '9')
				|| '_' == inch || '+' == inch
				|| '?' == inch)
		{  // Yes
			if (pos < 9)  // Room to add?
			{
				buf[pos] = inch; // Add to buffer
				++pos;  // Inc pointer
				buf[pos] = 0;  // Terminate string
				Serial.write(inch); // Echo to screen
			}
		}

		if (done)
		{
			done = 0;

			if (pos > 0)
			{
				which = CheckCmd(buf);
				pos = 0;
				buf[0] = 0;
			}
			else
				goto NoCmd;

			// Do command
			switch (which)
			{
#ifdef ZEBRA_MODE
			case TOGGLE_VERBOSE:  // Toggle verbose output
				if (Verbose)
				{
					Verbose = 0;
					Serial.println(F("Verbose: Off"));
				}
				else
				{
					Verbose = 1;
					Serial.println(F("Verbose: On"));
				}
				break;
			case OVERRIDE_LOW_VOLTAGE:  // Bypass low voltage check
				if (NoCheckVoltage)
				{
					NoCheckVoltage = 0; // set flag to not do voltage check
					Serial.println(F("Now doing low voltage check"));
				}
				else
				{
					NoCheckVoltage = 1; // set flag to not do voltage check
					Serial.println(F("Warning: Now skipping low voltage check!"));
				}
				break;
			case OVERRIDE_SERIAL_NUM:  // Bypass auth chip serial number check
				if (NoCheckSerialNum)
				{
					NoCheckSerialNum = 0; // set flag to do serial number check
					Serial.println(F("Now doing serial number check"));
				}
				else
				{
					NoCheckSerialNum = 1; // set flag to not do serial number check
					Serial.println(F("Warning: Now skipping serial number check!"));
				}
				break;
#endif
			case HELP_MENU:  // Help menu
				DoMenu();
				break;
			case SHOW_VERSION:  // Show version information
				ShowVersion();
				Serial.println(AuthSN);
				break;
			case SHOW_STATUS:  // Show status information
				Serial.println();
				Status();
				break;
			case SELECT_BATTERY:  // Set battery type
				ClearValid(); // Clear valid data flags
				SetType();
				break;
			case VALIDATE_BATTERY:
				// Clear valid data flags
				ClearValid(); // Clear valid data flags
				if (!BatteryType)
					NoType();
				else if (CheckVoltage())
				{
					break;
				}
				else // Good to go
				{
					StartWire();

					switch (BatteryType)
					{
					case MPA2_BATT:
					case MC95_BATT:
						// Read EEPROM data into the buffer
						FlushBuffer(0);
						EEPROM_Read(EEPROM_Data);
						if (!Validate_EEPROM(SMART_BATT_DATA))
						{
							Serial.println();
							Serial.println(BATT_OK);
						}
						else
						{
							Serial.println();
							Serial.println(FAILED_VAL);
						}
						break;
					case MC18_BATT:
					case ROGUE_BATT:
					case FRENZY_BATT:
						// Read EEPROM data into the buffer
						FlushBuffer(0);
						EEPROM_Read(EEPROM_Data);
						if (!Validate_EEPROM(GIFTED_BATT_DATA))
						{
							Serial.println();
							Serial.println(BATT_OK);
						}
						else
						{
							Serial.println();
							Serial.println(FAILED_VAL);
						}
						break;
					case IRONMAN_BATT:
					case FALCON_BATT:
						// Read EEPROM data into the buffer
						FlushBuffer(0);
						EEPROM_Read(EEPROM_Data);
						if (ret = Validate_EEPROM(COMBINED_BATT_DATA))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BATT_OK);
						else
							Serial.println(FAILED_VAL);
						break;
					case GALACTUS_BATT:
						// Read auth chip data into the buffer
						FlushBuffer(0);
						AUTH_Read(EEPROM_Data);
						if (ret = Validate_AUTH(GIFTED_BATT_DATA))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BATT_OK);
						else
							Serial.println(FAILED_VAL);
						break;
					case HAWKEYE_BATT:
						// Read auth chip data into the buffer
						FlushBuffer(0);
						AUTH_Read(EEPROM_Data);
						if (ret = Validate_AUTH(HAWKEYE_BATT_DATA))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BATT_OK);
						else
							Serial.println(FAILED_VAL);
						break;
					case METEOR_BATT:
						// Read auth chip data into the buffer
						FlushBuffer(0);
						AUTH_Read(EEPROM_Data);
						if (ret = Validate_AUTH(METEOR_BATT_DATA))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BATT_OK);
						else
							Serial.println(FAILED_VAL);
						break;
					case POLLUX_BATT:
						// Read EEPROM data into the buffer
						FlushBuffer(0);
						EEPROM_Read(EEPROM_Data);
						if (!Validate_EEPROM(POLLUX_BATT_DATA))
						{
							Serial.println();
							Serial.println(BATT_OK);
						}
						else
						{
							Serial.println();
							Serial.println(FAILED_VAL);
						}
						break;
					case SENTRY_BATT:
						// Read EEPROM data into the buffer
						FlushBuffer(0);
						EEPROM_Read(EEPROM_Data);
						// Read Auth chip data into the buffer
						AUTH_Read(EEPROM_Data+512);
						if (ret = Validate_EEPROM(COMBINED_BATT_DATA))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BATT_OK);
						else
							Serial.println(FAILED_VAL);
						break;
					case PPP_V2:
						// Read auth chip data into the buffer
						FlushBuffer(0);
						AUTH_Read(BD.Buf);
						if (ret = Validate_AUTH(PPP_V2_DATA))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BATT_OK);
						else
							Serial.println(FAILED_VAL);
						break;
					case FROZONE_EP_BATT:
					default:
						not_sup();
					}
				}
				break;
			case MANUFACTURE:
				if (!BatteryType)
					NoType();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					if (BufferBlank)
					{
						NoData();
						break;
					}
					StartWire();
					switch (BatteryType)
					{
					case MPA2_BATT:
						if (!Manufacture_EEPROM(SMART_BATT_DATA))
						{
							Serial.println();
							Serial.println(BATT_DONE);
						}
						break;
					case MC18_BATT:
					case ROGUE_BATT:
					case FRENZY_BATT:
						if (!Manufacture_EEPROM(GIFTED_BATT_DATA))
						{
							Serial.println();
							Serial.println(BATT_DONE);
						}
						break;
					case IRONMAN_BATT:
					case FALCON_BATT:
						if (Manufacture_EEPROM(COMBINED_BATT_DATA))
							break;
						if (!CheckAuthChip())
						{
							Serial.println();
							Serial.println(BATT_DONE);
						}
						break;
					case GALACTUS_BATT:
						if (!Manufacture_AUTH(GIFTED_BATT_DATA))
						{
							Serial.println();
							Serial.println(BATT_DONE);
						}
						break;
					case HAWKEYE_BATT:
						if (!Manufacture_AUTH(HAWKEYE_BATT_DATA))
						{
							Serial.println();
							Serial.println(BATT_DONE);
						}
						break;
					case POLLUX_BATT:
						if (!Manufacture_EEPROM(POLLUX_BATT_DATA))
						{
							Serial.println();
							Serial.println(BATT_DONE);
						}
						break;
					case SENTRY_BATT:
						if (!Manufacture_EEPROM(POLLUX8956_BATT_DATA))
						{
							Serial.println();
							Serial.println(BATT_DONE);
						}
					case PPP_V2:
					default:
						not_sup();
					}
				}
				break;
			case AUTHENTICATE:  // Authenticate battery
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].CryptoAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else
				{
					StartWire();
					if (!CheckAuthChip())
					{
						Serial.println();
						Serial.println(F("Auth chip is OK!"));
					}
				}
				break;
#ifdef ZEBRA_MODE
			case DUMP_AUTH_CONFIG:  // Dump auth chip config
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].CryptoAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else
				{
					StartWire();
					AUTH_ShowConfigHex();
				}
				break;
			case SAVE_AUTH_CONFIG:  // Dump auth chip config
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].CryptoAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else
				{
					StartWire();
					AUTH_SaveConfigHex();
				}
				break;
			case SHOW_AUTH_DATA:  // Dump auth chip config
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].CryptoAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else
				{
					StartWire();
					AUTH_ShowAllData();
				}
				break;
#endif
			case READ_EEPROM:  // Read EEPROM into buffer
				ClearValid(); // Clear valid data flags
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].EEPROM_Address)
					not_sup();
				else if (CheckVoltage())
					break;
				else // OK!
				{
					StartWire();
					EEPROM_Read();
				}
				break;
			case READ_AUTH:  // Read auth chip into buffer
				ClearValid(); // Clear valid data flags
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].CryptoAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else
				{
					StartWire();
					AUTH_Read();
				}
				break;
			case WRITE_EEPROM:  // Write EEPROM from buffer
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].EEPROM_Address)
					not_sup();
				else if (CheckVoltage())
					break;
				else
				{
					if (BufferBlank)
					{
						NoData();
						if (!KeepGoing())
							break;
					}
					StartWire();
					EEPROM_Write();
				}
				break;
			case WRITE_AUTH:  // Write auth chip from buffer
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].CryptoAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else
				{
					if (BufferBlank)
					{
						NoData();
						if (!KeepGoing())
							break;
					}
					StartWire();
					AUTH_Write();
				}
				break;
			case UPDATE_EEPROM:  // Update EEPROM from buffer
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].EEPROM_Address)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					if (BufferBlank)
					{
						NoData();
						if (!KeepGoing())
							break;
					}
					FlushSerial();
					Serial.print(F("Update "));
					Serial.print((__FlashStringHelper*)Batts[BatteryType].Name);
					Serial.println(F(" EEPROM data (y/n)?"));
					while(!Serial.available())
					;
					inch = Serial.read();
					if (inch != 'y' && inch != 'Y')
						break;

					StartWire();
					switch (BatteryType)
					{
					case MPA2_BATT:
						EEPROM_Update(SMART_BATT_DATA);
						break;
					case MC18_BATT:
					case ROGUE_BATT:
					case FRENZY_BATT:
						EEPROM_Update(GIFTED_BATT_DATA);
						break;
					case IRONMAN_BATT:
					case FALCON_BATT:
						EEPROM_Update(COMBINED_BATT_DATA);
						break;
					case POLLUX_BATT:
						EEPROM_Update(POLLUX_BATT_DATA);
						break;
					case SENTRY_BATT:
						EEPROM_Update(POLLUX8956_BATT_DATA);
						break;
					default:
						not_sup();
						break;
					}
				}
				break;
			case UPDATE_AUTH:  // Update auth chip from buffer
				if (!BatteryType)
					NoType();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					if (BufferBlank)
					{
						NoData();
						if (!KeepGoing())
							break;
					}
					FlushSerial();
					Serial.print(F("Update "));
					Serial.print((__FlashStringHelper*)Batts[BatteryType].Name);
					Serial.println(F(" auth chip data (y/n)?"));
					while(!Serial.available())
					;
					inch = Serial.read();
					if (inch != 'y' && inch != 'Y')
						break;

					StartWire();
					switch (BatteryType)
					{
					case METEOR_BATT:
					case HAWKEYE_BATT:
					case GALACTUS_BATT:
						AUTH_Update(BatteryType);
						break;
					default:
						not_sup();
						break;
					}
				}
				break;
			case VERIFY_EEPROM:  // Verify EEPROM from buffer
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].EEPROM_Address)
					not_sup();
				else if (CheckVoltage())
					break;
				else if (BufferBlank)
				{
					NoData();
					if (KeepGoing())
					{
						StartWire();
						EEPROM_Verify();
					}
				}
				else
				{
					StartWire();
					EEPROM_Verify();
				}
				break;
			case VERIFY_AUTH:  // Verify auth chip from buffer
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].CryptoAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else if (BufferBlank)
				{
					NoData();
					if (KeepGoing())
					{
						StartWire();
						AUTH_Verify();
					}
				}
				else
				{
					StartWire();
					AUTH_Verify();
				}
				break;
			case ERASE_BUFFER:  // Erase buffer
				ClearValid(); // Clear valid data flags
				FlushBuffer(1);
				break;
			case LOAD_HEX:  // Load buffer
				// Clear valid data flags
				ClearValid();
				if (!BatteryType)
					NoType();
				else // Good to go
				{	// Check for need to ask which kind of file this is
					if (Batts[BatteryType].CryptoAddress && Batts[BatteryType].EEPROM_Address)
					{
						if (GetAuthEEPROM())
							HEX_Read(Batts[BatteryType].Size);
						else
							HEX_Read(MAX_AUTH_SIZE);
					}
					else if (Batts[BatteryType].CryptoAddress)
						HEX_Read(MAX_AUTH_SIZE);
					else
						HEX_Read(Batts[BatteryType].Size);
				}
				break;
			case DUMP_HEX: // Dump buffer
				if (!BatteryType)
					NoType();
				else // Good to go
				{	// Check for need to ask which kind of file this is
					if (Batts[BatteryType].CryptoAddress && Batts[BatteryType].EEPROM_Address)
					{
						if (GetAuthEEPROM())
							HEX_Write(Batts[BatteryType].Size);
						else
							HEX_Write(MAX_AUTH_SIZE);
					}
					else if (Batts[BatteryType].CryptoAddress)
						HEX_Write(MAX_AUTH_SIZE);
					else
						HEX_Write(Batts[BatteryType].Size);
				}
				break;
#ifdef ZEBRA_MODE
			case SHOW_PP: // Show PP data
				if (BufferBlank)
					NoData();
				switch (BatteryType)
				{
				case MPA2_BATT:
					ShowMPA2();
					break;
				case METEOR_BATT:
					ShowMeteor();
					break;
				case IRONMAN_BATT:
				case POLLUX_BATT:
				case FALCON_BATT:
				case SENTRY_BATT:
				case FROZONE_EP_BATT:
					ShowPollux();
					break;
				case HAWKEYE_BATT:
					ShowHawkeye();
					break;
				default:
					not_sup();
					break;
				}
				break;
			case SHOW_PPP: // Show PP+ data
				if (BufferBlank)
					NoData();
				else
					switch (BatteryType)
					{
					case IRONMAN_BATT:
					case POLLUX_BATT:
					case FALCON_BATT:
					case SENTRY_BATT:
					case MC18_BATT:
					case ROGUE_BATT:
					case FRENZY_BATT:
					case GALACTUS_BATT:
					case FROZONE_EP_BATT:
						ShowPPP();
						break;
					case PPP_V2:
						ShowPPPV2();
						break;
					default:
						not_sup();
						break;
					}
				break;
			case DUMP_GAS_GAUGE_CONFIG:
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					if (PPP_V2 == BatteryType)
						DumpNewGGConfig();
					else
						DumpGGConfig();
				}
				break;
			case SAVE_GAS_GAUGE_CONFIG:
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					if (PPP_V2 == BatteryType)
						SaveNewGGConfig();
					else
						SaveGGConfig();
				}
				break;
			case DUMP_GAS_GAUGE_FLASH:
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					if (PPP_V2 == BatteryType)
						DumpNewGGflash();
					else
						DumpGGflash();
				}
				break;
			case SAVE_GAS_GAUGE_FLASH:
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					if (PPP_V2 == BatteryType)
						SaveNewGGflash();
					else
						SaveGGflash();
				}
				break;
			case SHOW_GG: // Show gas gauge regs
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					ShowGGregs();
				}
				break;
			case SHOW_GG_HEX: // Show gas gauge regs in hex
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					ShowGGregsHex();
				}
				break;
			case ENTER_HIBERNATE: // Enable hibernate mode
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress || NEW_GAUGE)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					GG_EnterHibernate();
				}
				break;
			case SET_FULLSLEEP: // Set the FULLSLEEP bit
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					GG_SetFullSleep();
				}
				break;
			case RESET_GG: // Reset the gas gauge
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					GG_Reset();
				}
				break;
			case SEAL_GG: // Seal the gas gauge
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					GG_do_Seal();
				}
				break;
			case UNSEAL_GG: // Unseal the gas gauge
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					GG_do_Unseal();
				}
				break;
			case FULL_UNSEAL_GG: // Unseal the gas gauge
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].GasGaugeAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					GG_do_FullUnseal();
				}
				break;
			case MANUF_V2:  // Set serial number in V2 PP+ batts
				if (!BatteryType)
					NoType();
				else if (BatteryType != PPP_V2)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
					StartWire();
					ManufV2();
				}
				break;
#endif
			case LOAD_FILE:  // Load buffer from file
				ClearValid(); // Clear valid data flags
				if (!BatteryType)
					NoType();
				else if (!HaveSD)
					NoCard();
				else
				{	// Check for need to ask which kind of file this is
					if (Batts[BatteryType].CryptoAddress && Batts[BatteryType].EEPROM_Address)
					{
						if (GetAuthEEPROM())
							HEX_ReadFile(Batts[BatteryType].Size);
						else
							HEX_ReadFile(MAX_AUTH_SIZE);
					}
					else if (Batts[BatteryType].CryptoAddress)
						HEX_ReadFile(MAX_AUTH_SIZE);
					else
						HEX_ReadFile(Batts[BatteryType].Size);
				}
				break;
			case SAVE_FILE:  // Save to SD card
				if (!BatteryType)
					NoType();
				else if (!HaveSD)
					NoCard();
				else if (SD_WP)
					WPCard();
				else // Good to go
				{	// Check for need to ask which kind of file this is
					if (Batts[BatteryType].CryptoAddress && Batts[BatteryType].EEPROM_Address)
					{
						if (GetAuthEEPROM())
							SaveFile(Batts[BatteryType].Size);
						else
							SaveFile(MAX_AUTH_SIZE);
					}
					else if (Batts[BatteryType].CryptoAddress)
						SaveFile(MAX_AUTH_SIZE);
					else
						SaveFile(Batts[BatteryType].Size);
				}
				break;
			case DIRECTORY: // SD card directory
				if (!HaveSD)
					NoCard();
				else
					ListDir();
				break;
			default:
				Serial.println(F("Unknown command"));
				break;
			}

NoCmd:
			FlushSerial();
			Serial.println(); // Print prompt
			Serial.print(F("> "));
		}
	}
}

uint8_t CheckVoltage(void)
{
	if (NoCheckVoltage)
		return (0);

	if ((!Batts[BatteryType].ActiveClk // If passive clk drive
				|| Batts[BatteryType].GasGaugeAddress // or using a gas gauge
				|| Batts[BatteryType].CryptoAddress) // or using an auth chip
				&& BattVoltage() <= Batts[BatteryType].MinVoltage) // And battery is <= 3V
	{
		LowBatt();
		return (1);
	}

	return (0);
}

void ShowVersion(void)
{
	Serial.println();
	Serial.print(F("EEPROM Programmer Version "));
	Serial.println(F(VERSION));
}

// Clear valid data flags
void ClearValid()
{
	PP_Valid = 0;
	PPP_Valid = 0;
	VT_Valid = 0;
	PPPV2_Valid = 0;
}

void StartWire(void)
{
	TWCR=0;  // Reset TWI in case it's stuck

	Wire.begin();  // Start/restart TWI

	// Remove SCL/SDA internal pullups
	digitalWrite(SCL,LOW);
	digitalWrite(SDA,LOW);

	// Change I2C clock to ~32KHz
	Wire.setClock(32250);
}

enum Command CheckCmd(char *in)
{
	uint8_t cnt, next, cmd;

	// Anything in buffer?
	if (0 == in[0])
		return (NO_CMD);

	// Convert to upper case
	strupr(in);

	for (cmd = HELP_MENU; cmd < NO_CMD; ++cmd)
	{
		next=0;
		for (cnt = 0; Cmds[cmd][cnt] != 0; ++cnt)
		{
			if (Cmds[cmd][cnt] != in[cnt])
			{
				next = 1;
				break;
			}
		}
		if (!next)
		{
			if (!in[cnt])
				return((Command)cmd);
		}
	}
	return(NO_CMD);
}

void DoMenu(void)
{
	Serial.println(F("Currently Available Commands:"));
	Serial.println(F("B - Select battery type"));
	if (BatteryType) // If we have selected a battery
	{
#ifdef ZEBRA_MODE
		if (!BufferBlank)
			Serial.println(F("MB - Manufacture Battery"));

		if (PP_Valid)
			Serial.println(F("DPP - Display PP battery data"));

		if (PPP_Valid)
			Serial.println(F("DPP+ - Display PP+ battery data"));

		if (Batts[BatteryType].GasGaugeAddress)
		{
			Serial.println(F("DGG - Display Gas Gauge registers"));
			Serial.println(F("DGGH - Display Gas Gauge registers in hex"));
			if (PPP_V2 != BatteryType)
			{
				Serial.println(F("HGG - Put Gas Gauge into hibernate mode"));
				Serial.println(F("FSGG - Set FULLSLEEP bit"));
			}
			Serial.println(F("RGG - Reset Gas Gauge"));
			Serial.println(F("SGG - Seal Gas Gauge"));
			Serial.println(F("USGG - Unseal Gas Gauge"));
			Serial.println(F("DGGF - Dump Gas Gauge Flash data"));
			Serial.println(F("SGGF - Save Gas Gauge Flash data in hex file"));
		}

		if (PPP_V2 == BatteryType)
			Serial.println(F("MV2 - Manufacture V2 battery"));
#endif

		Serial.println(F("VBD - Validate battery data"));

		if (Batts[BatteryType].CryptoAddress)
		{ // Using an auth chip
			Serial.println(F("A - Check battery authentication chip"));
			Serial.println(F("RA - Read battery Auth chip data into buffer"));
			Serial.println(F("WA - Write battery Auth chip data from buffer"));
			Serial.println(F("UA - Update battery Auth chip data from buffer"));
			Serial.println(F("VA - Verify battery Auth chip data against buffer"));
		}

		if (Batts[BatteryType].EEPROM_Address)
		{ // Using EEPROM for data
			Serial.println(F("RE - Read battery EEPROM into buffer"));
			Serial.println(F("WE - Write battery EEPROM from buffer"));
			Serial.println(F("UE - Update battery EEPROM from buffer"));
			Serial.println(F("VE - Verify battery EEPROM against buffer"));
		}
	}
	Serial.println(F("E  - Erase buffer"));
	Serial.println(F("LS  - Load hex file over the serial connection into buffer"));
	Serial.println(F("DS  - Dump buffer over the serial connection as hex file"));
	if (HaveSD)
	{
		Serial.println(F("LF  - Load a hex file from the SD card"));
		if (!SD_WP)
			Serial.println(F("SF  - Save buffer to file on SD card"));
		Serial.println(F("DIR  - Directory of SD card"));
	}
	Serial.println(F("VER  - Show version information"));
#ifdef ZEBRA_MODE
	Serial.println(F("VERB - Toggle verbose mode"));
#endif
	Serial.println(F("STAT  - Show current status"));
	Serial.println(F("?  - help menu"));
}

void Status(void)
{
	if (BatteryType)
	{
		Serial.print(F("Battery: "));
		Serial.println((__FlashStringHelper*)Batts[BatteryType].Name);
		if (Batts[BatteryType].EEPROM_Address)
		{
			Serial.print(Batts[BatteryType].Size);
			Serial.println(F(" Bytes of EEPROM storage"));
		}
		if (Batts[BatteryType].CryptoAddress)
			Serial.println(F("632 Bytes of Auth chip storage"));
	}
	else
		Serial.println(F("No Battery Selected!"));

#ifdef ZEBRA_MODE
	if (Verbose)
		Serial.println(F("Verbose: On"));
	else
		Serial.println(F("Verbose: Off"));

	if (NoCheckVoltage)
		Serial.println(F("Warning: Now skipping low voltage check!"));
#endif

	PrintVoltage();
	PrintResistor();
	PrintClock();
	Serial.print(F("Battery Voltage: "));
	Serial.print(BattVoltage());
	Serial.println(F("V"));
}

// Function to read the battery voltage
// Returns voltage in mv
float BattVoltage(void)
{
	float voltage, Vcc;
	int AtD;

	Vcc = calibrate();
	AtD = analogRead(BATT_PLUS);
	voltage = AtD * (Vcc / 1023.0);
	return (voltage);
}

// Function to read the supply voltage
// Used to calibrate the ADC readings
float calibrate()
{
	int reading;
	float Vcc;

	// Read the 1.1V ref using 5V as ADC ref level
	ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

	delay(10); // Wait for Vref to settle

	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA,ADSC)) // measuring
	;

	// Toss the first one
	delay(10);
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA,ADSC)) // measuring
	;

	reading = ADC; // Get result

	// Calculate 5V supply level
	Vcc = 5.0 / 1023.0 * (float)reading;
	Vcc = 1.1 / Vcc * 5.0;

	return(Vcc);
}

// Function to update battery status
// Returns true if status changed, false otherwise
uint8_t CheckBattery(void)
{
	if (HaveBattery) // Do we have a battery?
	{  // Check for removal
		if (BattVoltage() < 0.5)
		{
			Serial.println();  
			Serial.println(F("Battery removed!"));  
			HaveBattery = 0;  // No battery now
			return (1);
		}
	}
	else
	{  // Check for battery insertion
		if (BattVoltage() > 0.3)
		{
			Serial.println();  
			Serial.print(F("Battery inserted, voltage: "));
			delay(200);
			Serial.print(BattVoltage());
			Serial.println(F("V"));
			HaveBattery = 1;  // Have battery now			
			return (1);
		}
	}

	return (0);
}

// Function to update SD card status
// Returns true if status changed, false otherwise
uint8_t CheckSD(void)
{
	if (HaveSD) // Do we have a card?
	{  // Check for removal
		if (digitalRead(CD))
		{
			Serial.println();  
			Serial.println(F("SD removed!"));  
			HaveSD = 0;  // No card now
			return (1);
		}
	}
	else
	{  // Check for card insertion
		if (!digitalRead(CD))
		{
			if (!SD.begin(SD_CS, SPI_HALF_SPEED))  // see if the card is present and can be initialized
			{
				Serial.println();  
				Serial.println(F("Bad SD card!"));
				return (1);
			}
			else
			{
				if (!digitalRead(WP))
				{
					Serial.println();  
					Serial.println(F("Found SD card!"));
					SD_WP = 0;
				}
				else
				{
					Serial.println();  
					Serial.println(F("Found write protected SD card!"));
					SD_WP = 1;
				}
				HaveSD = 1;  // Have card now			
				return (1);
			}
		}
	}

	return (0);
}

// Function to print message for no SD card
void NoCard(void)
{
	Serial.println(F("Please insert an SD card first!"));
}

// Function to print message for write protected SD card
void WPCard(void)
{
	Serial.println(F("SD is write protected!"));
}

// Function to print message for no selected battery type
void NoType(void)
{
	Serial.println(F("Please select a battery type first!"));
}

// Function to print message for low battery voltage
void LowBatt(void)
{
	Serial.print(F("Battery voltage must be > "));
	Serial.println(Batts[BatteryType].MinVoltage);
}

// Function to print message for not supported operation
void not_sup(void)
{
	Serial.println(F("Function not supported on this battery type"));
}

// Function to print message for no data in buffer
void NoData(void)
{
	Serial.println(F("The data buffer is blank!"));
}

// Function to calculate a checksum
uint8_t DoChecksum(uint8_t *Start, uint8_t * End)
{
	uint8_t *pnt;
	uint8_t csum = 0;

	for (pnt = Start; pnt <= End; ++pnt)
		csum += *pnt;

	return (csum);
}

// Serial port printf function
void printfSerial(char *fmt, ... )
{
        char buf[100]; // resulting string limited to 100 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 100, fmt, args);
        va_end (args);
        Serial.print(buf);
}

// Funtion to manually set the battery type
void SetType(void)
{
	int cnt, cnt1;
	char msg[10];

	Serial.println();
	FlushSerial();

	for (cnt = 0; cnt < MAX_BATTERY_TYPES; ++cnt)  // Loop thru battery types
	{
		printfSerial("%d - ",cnt);
		Serial.println((__FlashStringHelper*)(Batts[cnt].Name));
	}

	// Get value
	Serial.println();
	Serial.print(F("Select battery type: "));
	cnt = GetInt();

	if (cnt >= 0) // Got something?
	{ // yes
		if (cnt >= MAX_BATTERY_TYPES)
			Serial.println(F("Not a valid choice, battery type not changed!"));
		else
		{
			BatteryType = cnt;
			if (Batts[BatteryType].ActiveClk)
				ActiveClk();
			else
				PassiveClk();
			SetVoltage(Batts[BatteryType].Pullup_volt);
			SetResistor(Batts[BatteryType].Pullup_resistor);
			Status();

			// Do we need to switch the clock?
			if (Batts[BatteryType].ActiveClk && (Batts[BatteryType].GasGaugeAddress || Batts[BatteryType].CryptoAddress))
				SwitchClk = 1;
			else
				SwitchClk = 0;
		}

		// Handle any special cases
		switch (BatteryType)
		{
		case MC95_BATT:
			Serial.println(F("**** For MC95 Batts supply 3.3V ****"));
			Serial.println(F("**** to batt external power pin ****"));
			Serial.println(F("**** and Batt+ on this box!     ****"));
			Serial.println();
			break;
		default:
			break;
		}
	}
	else
	{ // Nope
		Serial.println(F("Battery type not changed!"));
	}
}

// Function to read in hex file
// Returns 0 if OK, 1 on error
int HEX_Read(uint16_t size)
{
	unsigned char x;
	bool done;
	unsigned int Address;		// Record offset/address
	unsigned char RecLen;		// Record length
	unsigned char RecType;		// Record type
	unsigned char DataBuf[MAX_REC_LEN];	// Buffer for hex data, we only support 16 byte records max.
	unsigned int CurPage;		// Current flash page being loaded

	if (!BatteryType)  // Check for valid battery type
		return(1);

	// Setup segment, address, etc.
	CurPage = 0;
	Address = 0;

	FlushBuffer(1);	// Clear buffer if needed/wanted

	FlushSerial();

	done = false;	// Clear finished flag
	HadError = 0;  // Clear error flag

	Serial.println(F("Send file now, ESC to cancel..."));

	do {	// Loop to do hex records
		// Get next hex record
		do {			// Wait for a colon
			while (!Serial.available())
			;
			RecLen = Serial.read();
			if (RecLen == ESC) { // Check for user abort
				Serial.println(F("Stopped by user!"));
				FlushSerial();
				return(1);
			}
			if (HadError) // Check for error
			{
				FlushSerial();
				return(1);
			}
		} while (RecLen != ':');

		CSum = 0;		// Clear checksum

		RecLen = GetByte();	// Get Record Length

		Address = GetByte();	// Get Offset (Address)
		Address = Address << 8;
		Address |= GetByte();

		RecType = GetByte();	// Get Record Type

		if (RecLen > MAX_REC_LEN) {	// Check for record to big
			Serial.print(F("Record > "));
			Serial.print(MAX_REC_LEN);
			Serial.println(F(" Bytes"));
			FlushSerial();
			return(1);
		}

		for (x = 0; x < RecLen; ++x)	// Get data
			DataBuf[x] = GetByte();

		GetByte();			// Get Checksum

		if (CSum) {
			Serial.println(BAD_CHECKSUM);
			FlushSerial();
			return(1);
		}

		// Handle record type
		switch (RecType) {
		case DATA_REC:	// Data record
			// Load data to buffer
			for (x = 0; x < RecLen; ++x, ++Address)	// Move data
				if (Address >= size)
				{
					Serial.println(F("ERROR: Address past end of part!"));
					return (1);
				}
				else
					EEPROM_Data[Address] = DataBuf[x];	// Put into buffer
			break;
		case EOF_REC:	// End of file record
			done = true;		// Set done flag
			break;
		case START_REC:	// Start segment address record, just ignore
			break;
		default:
			Serial.println(F("Bad record type"));
			FlushSerial();
			return(1);
		}
	} while (!done);

	FlushSerial();
	Serial.println(F("Done with file load."));
	BufferBlank = 0;  // Stuff in buffer

	return(0);
}

// Routine to flush incoming serial data
void FlushSerial(void)
{
	unsigned long StartTime = millis();  // Get start time

	do
	{
		if (Serial.available())  // Data still coming in?
		{  // Yes
			Serial.read();  // Toss it
			StartTime = millis();  // Reset timer
		}
	} while((millis() - StartTime) < 200); // Make sure no data for a while
}

// Routine to get a hex byte from the com port
unsigned char GetByte(void)
{
	char val;

	while (!Serial.available())
	;
	val = ConvHex(Serial.read());	// Convert from hex
	val = val << 4;		// Shift it over

	while (!Serial.available())
	;
	val |= ConvHex(Serial.read());	// Convert from hex and add in

	CSum += val;		// Add to checksum

	return (val);
}

// Routine to convert from hex to bin
unsigned char ConvHex(unsigned char c)
{
	if (c >= '0' && c <= '9')	// Do number
		return ((c - '0'));

	if (c >= 'A' && c <= 'F')	// Do letter
		return (c + 10 - 'A');

	Serial.println(F("Bad Character"));	// Bad character
	HadError = 1;  // Set error flag
	return (0);
}

// Function to reset the EEPROM buffer
void FlushBuffer(int ask)
{
	int cnt;
	uint8_t inch;

	FlushSerial();
	if (ask)
	{
		Serial.println(F("Erase data buffer (y/n)?"));
		while(!Serial.available())
		;
		inch = Serial.read();
		if (inch != 'y' && inch != 'Y')
			return;
	}

	// Flush EEPROM data
	for (cnt = 0; cnt < MAX_EEPROM_SIZE; ++cnt)
//{EEPROM_Data[cnt] = cnt >> 8;EEPROM_Data[cnt+1] = cnt & 0xff;++cnt;}
		EEPROM_Data[cnt] = 0xff;

	Serial.println(F("Buffer erased"));

	BufferBlank = 1;
	return;
}

// Function to reset the EEPROM buffer
void TestPattern(void)
{
	int cnt;
	uint8_t inch;

	Serial.println(F("Overwrite data buffer (y/n)?"));
	while(!Serial.available())
	;
	inch = Serial.read();
	if (inch != 'y' && inch != 'Y')
		return;

	// Generate EEPROM test data
	for (cnt = 0; cnt < MAX_EEPROM_SIZE; ++cnt)
		EEPROM_Data[cnt] = (cnt >> 4) ;

	Serial.println(F("Test pattern in buffer!"));
	BufferBlank = 0;
}

// Function to write hex file from buffer
void HEX_Write(uint16_t size, uint8_t *buf)
{
	uint8_t inch, *buffer = NULL;
	uint16_t cnt, cnt2, reclen;
	char msg[80];  // Output text buffer
	uint8_t csum;  // Record checksum

	if (!BatteryType)  // Check for valid battery type
		return;
Serial.println(size);
	FlushSerial();
	Serial.println(F("Hit any key to start hex file output, then hit another key when done"));
	WaitForKey();

	// Check for what to send
	if (NULL != buf)  // Use the provided buffer
		buffer = buf;
	else // Use the default buffer
		buffer = EEPROM_Data;

	// loop thru 16 bytes at a time
	for (cnt = 0; cnt < size; cnt += 16)
	{
		// Get record length
		if ((size-cnt) < 16)  // Partial row?
			reclen = size - cnt;
		else
			reclen = 16;

		// Output hex record
		sprintf(msg,":%02X%04X00",reclen,cnt);
		Serial.print(msg);

		csum = reclen; // Startup checksum
		csum += cnt & 0xff;
		csum += cnt  >> 8;

		for (cnt2 = 0; cnt2 < reclen; ++cnt2)  // Loop thru 16 byte record
		{
			inch = buffer[cnt+cnt2]; // Get next byte
			csum += inch;
			sprintf(msg,"%02X",inch);
			Serial.print(msg);
		}
		sprintf(msg,"%02X",(~csum+1)&0xff);  // Ouput checksum
		Serial.println(msg);

		// Check for user hitting ESC
		if (Serial.available() && ESC == Serial.read())
		{
			Serial.println(F("Interrupted by user"));
			goto ReadError;
		}
	}
	Serial.println(F(":00000001FF"));
	WaitForKey();
	Serial.println(F("Hex file output done."));
ReadError:
	return;
}

// Routine to wait for a keypress
int WaitForKey(void)
{
	FlushSerial();
	while (!Serial.available())
	;
	return(Serial.read());
}

// Routine to get int from serial port
int GetInt()
{
	char inch;
	char buf[10];
	int retval, done = 0, pos = 0;

	buf[pos] = 0;  // Terminate string

	do {
		// Get a character
		while(!Serial.available())
		;
		inch = Serial.read();

		// CR?
		if (CR == inch)
			done = 1;
		else if (BKSP == inch) // Backspace
		{
			if (pos > 0) // Can we move back?
			{
				--pos;
				buf[pos] = 0;  // Terminate string
				Serial.print(F("\010 \010")); // Clear character on screen
			}
		}
		else if (inch >= '0' && inch <= '9') // Is it a digit?
		{  // Yes
			if (pos < 9)  // Room to add?
			{
				buf[pos] = inch; // Add to buffer
				++pos;  // Inc pointer
				buf[pos] = 0;  // Terminate string
				Serial.write(inch); // Echo to screen
			}
		}
	} while(!done);

	Serial.println();  // New line

	if (pos > 0)
		sscanf(buf,"%d",&retval);
	else
		retval = -1;

	return(retval);
}

// Routine to display file directory
void ListDir(void)
{
	File file, dir;
	char msg[15], name[15];

	Serial.println(F("Name        Date                      Size"));
	dir.open("/");  // Open root dir

	while (file = dir.openNextFile())
	{
		if (!file.isDir()) {
			file.getName(name,15);
			sprintf(msg,"%13s ",name);
			Serial.print(msg);
			file.printModifyDateTime(&Serial);
			Serial.write(' ');    
			file.printFileSize(&Serial);
			Serial.println();
		}
		file.close();
	}

	dir.close();
	Serial.println();
}

// Routine to save buffer to file
void SaveFile(uint16_t size, uint8_t *buf)
{
	SdFile file;
	char name[15], msg[80];
	uint8_t inch, *buffer;
	uint16_t cnt, cnt2, reclen;
	uint8_t csum;  // Record checksum
	uint16_t Year, Month, Day, Hour, Minute, Second;

	if (!BatteryType)  // Check for valid battery type
		return;

	Year = 2017;
	Month = 12;
	Day = 13;
	Hour = 11;
	Minute = 59;
	Second = 0;

	// Get file name
	FlushSerial();
	Serial.println(F("Enter file name for save"));
	Serial.print(F("8 chars max, no extension: "));
	name[0] = 0;  // Terminate buffer
	if (!GetText(name))
	{
		Serial.println();
		Serial.println("Canceled");
		Serial.println();
		return;
	}
	sprintf(msg,"%s.hex",name);

	// Check for file already there
	if (file.open(msg, O_READ))
	{
		file.close(); // Close file
		Serial.println(F("File exists, overwrite (y/n)?"));
		while(!Serial.available())
		;
		inch = Serial.read();
		if (inch != 'y' && inch != 'Y')
			return;
	}

	// Open/Create file
	if (!file.open(msg, O_CREAT | O_WRITE | O_TRUNC))
	{
		Serial.println(F("Error opening file!"));
		return;
	}
  
	// set creation date time
	if (!file.timestamp(T_CREATE, Year, Month, Day, Hour, Minute, Second))
		Serial.println(F("Set create time failed."));
	// set write/modification date time
	if (!file.timestamp(T_WRITE, Year, Month, Day, Hour, Minute, Second))
		Serial.println(F("Set write time failed."));
	// set access date
	if (!file.timestamp(T_ACCESS, Year, Month, Day, Hour, Minute, Second))
		Serial.println(F("Set access time failed."));

	// Check for what to write
	if (NULL != buf)  // Use the provided buffer
		buffer = buf;
	else  // Use the default buffer
		buffer = EEPROM_Data;

	// Write data
	// loop thru 16 bytes at a time
	for (cnt = 0; cnt < size; cnt += 16)
	{
		if ((size-cnt) >= 16)
			reclen = 16;
		else
			reclen = size - cnt;

		// Output hex record
		sprintf(msg,":%02X%04X00",reclen,cnt);
		file.print(msg);

		csum = reclen; // Startup checksum
		csum += cnt & 0xff;
		csum += cnt  >> 8;

		for (cnt2 = 0; cnt2 < reclen; ++cnt2)  // Loop thru record
		{
			inch = buffer[cnt+cnt2]; // Get next byte
			csum += inch;
			sprintf(msg,"%02X",inch);
			file.print(msg);
		}
		sprintf(msg,"%02X",(~csum+1)&0xff);  // Ouput checksum
		file.println(msg);

	}
	file.println(F(":00000001FF"));
	file.close(); // Close file
	Serial.println(F("File save done."));
}

// Routine to get file name from serial port
// Input is buffer to hold up to 8 character name and terminating zero
uint8_t GetText(char *buf,uint8_t MaxLen,uint8_t NoFilter)
{
	char inch;
	int retval, done = 0, pos = 0;

	// Check for "preloaded" string
	pos = strlen(buf);
	if (pos)
		Serial.print(buf);

	do {
		// Get a character
		while(!Serial.available())
		;
		inch = Serial.read();

		if (ESC == inch)  // ESC?
			return (0);
		else if (CR == inch)  // CR?
			done = 1;
		else if (BKSP == inch) // Backspace
		{
			if (pos > 0) // Can we move back?
			{
				--pos;
				buf[pos] = 0;  // Terminate string
				Serial.print(F("\010 \010")); // Clear character on screen
			}
		}
		else if (NoFilter || (' ' == inch || '!' == inch // Is it a valid character?
				|| (inch >= '#' && inch <= '.')
				|| (inch >= '0' && inch <= '9')
				|| ';' == inch || '=' == inch
				|| (inch >= '?' && inch <= '~')))
		{  // Yes
			if (pos < MaxLen)  // Room to add?
			{
				buf[pos] = inch; // Add to buffer
				++pos;  // Inc pointer
				buf[pos] = 0;  // Terminate string
				Serial.write(inch); // Echo to screen
			}
		}
	} while(!done);

	Serial.println();  // New line

	return(pos);
}

// Function to read in hex file from SD card
// Returns 0 if OK, 1 on error
uint8_t HEX_ReadFile(uint16_t size)
{
	unsigned char x;
	bool done;
	unsigned int Address;		// Record offset/address
	unsigned char RecLen;		// Record length
	unsigned char RecType;		// Record type
	unsigned char DataBuf[MAX_REC_LEN];	// Buffer for hex data, we only support 16 byte records max.
	unsigned int CurPage;		// Current flash page being loaded
	SdFile file;
	char name[20], msg[20];
	uint8_t inch;

	if (!BatteryType)  // Check for valid battery type
		return (1);

	// Get file name
	FlushSerial();
	Serial.println(F("Enter file name for read"));
	Serial.print(F("8 chars max, no extension: "));
	name[0] = 0;  // Terminate buffer
	if (!GetText(name))
	{
		Serial.println();
		Serial.println("Canceled");
		Serial.println();
		return (0);
	}
	sprintf(msg,"%s.hex",name);

	FlushBuffer(1);  // Clear buffer if needed/wanted

	FlushSerial();

	// Open file
	if (!file.open(msg, O_READ))
	{
		Serial.println(F("Error opening file!"));
		return (1);
	}

	// Setup segment, address, etc.
	CurPage = 0;
	Address = 0;

	done = false;	// Clear finished flag
	HadError = 0;  // Clear error flag

	do {	// Loop to do hex records
		// Get next hex record
		do {			// Wait for a colon
			if (1 != file.read(&RecLen,1) || HadError)  // Get next byte from file and check for read error
			{
				Serial.println(F("Error reading file!"));
				file.close();
				return(1);
			}
		} while (RecLen != ':');

		CSum = 0;		// Clear checksum

		RecLen = GetFileByte(&file);	// Get Reord Length

		Address = GetFileByte(&file);	// Get Offset (Address)
		Address = Address << 8;
		Address |= GetFileByte(&file);

		RecType = GetFileByte(&file);	// Get Record Type

		if (RecLen > MAX_REC_LEN) {	// Check for record to big
			Serial.println(F("Record > 32 bytes"));
			file.close();
			return(1);
		}

		for (x = 0; x < RecLen; ++x)	// Get data
			DataBuf[x] = GetFileByte(&file);

		GetFileByte(&file);			// Get Checksum

		if (CSum) {
			Serial.println(BAD_CHECKSUM);
			file.close();
			return(1);
		}

		// Handle record type
		switch (RecType) {
		case DATA_REC:	// Data record
			// Load data to buffer
			for (x = 0; x < RecLen; ++x, ++Address)	// Move data
				if (Address >= size)
				{
					Serial.println(F("ERROR: Address past end of part!"));
					file.close();
					return (1);
				}
				else
					EEPROM_Data[Address] = DataBuf[x];	// Put into buffer
			break;
		case EOF_REC:	// End of file record
			done = true;		// Set done flag
			break;
		case START_REC:	// Start segment address record, just ignore
			break;
		default:
			Serial.println(F("Bad record type"));
			file.close();
			return(1);
		}
	} while (!done);

	file.close();

	Serial.println(F("Done with load from SD."));
	BufferBlank = 0;  // Stuff in buffer

	return(0);
}

// Routine to get a hex byte from a file
unsigned char GetFileByte(SdFile *inFile)
{
	uint8_t val, inch;

	if (1 != inFile->read(&inch,1))  // Get next byte from file
	{
		Serial.println(READ_ERR);
		HadError = 1;
	}
	val = ConvHex(inch);	// Convert from hex
	val = val << 4;		// Shift it over

	if (1 != inFile->read(&inch,1))  // Get next byte from file
	{
		Serial.println(READ_ERR);
		HadError = 1;
	}
	val |= ConvHex(inch);	// Convert from hex and add in

	CSum += val;		// Add to checksum

	return (val);
}

uint8_t CheckClk(void)
{
	if (!digitalRead(I2C_CLK))
	{
		Serial.println(F("Error: I2C clock is low!"));
		return (1);
	}

	return (0);
}

// Function to update PP+ data
void DoGiftedUpdate(int StartAddr, uint8_t *Buf)
{
	int cnt;

	Serial.println(F("Doing PP+ data update..."));

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

// Function to update VT PP data
uint8_t DoVTupdate(uint8_t *Buf)
{
	int cnt;
	uint8_t csum;

	Serial.println(F("Doing VT data update..."));

	// The format revision byte will be updated, only values 1-2 are supported.
	if (EEPROM_Data[1] < 1 || EEPROM_Data[1] > 3)
	{
		Serial.println(BAD_REV);
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

	Serial.println(F("Doing QC PP data update..."));

	// The format revision byte will be updated, only values 0-3 are supported.
	if (EEPROM_Data[1] > 3)
	{
		Serial.println(BAD_REV);
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

	Serial.println(F("Doing Hawkeye data update..."));

	// The format revision byte will be updated, only values 0-3 are supported.
	if (EEPROM_Data[1] > 3)
	{
		Serial.println(BAD_REV);
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

void SaveGGflash(void)
{
	uint8_t buf[96];

	if (GG_GetManfBlocks(buf))
		return;

	SaveFile(96,buf);
}

void DumpGGflash(void)
{
	uint8_t buf[96];

	if (GG_GetManfBlocks(buf))
		return;

	HEX_Write(96,buf);
}

void SaveNewGGflash(void)
{
	uint8_t buf[96];

	if (GG_GetNewManfBlocks(buf))
		return;

	SaveFile(96,buf);
}

void DumpNewGGflash(void)
{
	uint8_t buf[96];

	if (GG_GetNewManfBlocks(buf))
		return;

	HEX_Write(96,buf);
}

// Function to ask if the operation is for an Auth chip or an EEPROM
// Returns true for an EEPROM
uint8_t GetAuthEEPROM()
{
	uint8_t ret;

	do
	{
		FlushSerial();
		Serial.println();
		Serial.print(F("'E'EPROM or 'A'uth chip data? "));
		while (!Serial.available())
		;
		ret = Serial.read() | 0x20;
	} while (ret != 'e' && ret != 'a');

	Serial.println();

	if ('e' == ret)
		return (1);
	else
		return (0);
}
