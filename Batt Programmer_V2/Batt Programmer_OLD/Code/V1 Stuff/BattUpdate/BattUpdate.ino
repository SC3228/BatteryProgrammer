// Battery "update" code for generic Arduino Mega

#define VERSION "1.0"

/*
1.0	Initial release created from BattProg1 code
*/

// Includes
#include <Wire.h>	// I2C lib

#include "pwmbatt.h"
#include "BattProg.h"
#include "Auth.h"
#include "EEPROM.h"

// List of commands
const char *Cmds[] = { "?","VER","STAT","B"
						,"RE","RA","WE","WA"
						,"UE","UA","VE","VA"
						,"E","LS","DS"
					};

enum Command { HELP_MENU, SHOW_VERSION, SHOW_STATUS, SELECT_BATTERY
				, READ_EEPROM, READ_AUTH, WRITE_EEPROM, WRITE_AUTH
				, UPDATE_EEPROM, UPDATE_AUTH, VERIFY_EEPROM, VERIFY_AUTH
				, ERASE_BUFFER, LOAD_HEX, DUMP_HEX
				, NO_CMD };

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
const char Falcon[] PROGMEM = "Falcon/Thunder/Elektra";
const char Hawkeye[] PROGMEM = "Hawkeye/TC20";
const char Sentry[] PROGMEM = "Sentry";
const char Galactus[] PROGMEM = "Galactus/Badger/Firebird/Frozone Auth";
const char Frozone[] PROGMEM = "Frozone EEPROM";
const char Meteor[] PROGMEM = "Meteor/Gravity/Simba";

// Supported battery table
BattData Batts[MAX_BATTERY_TYPES] = {
	// No battery
	{0,0,0,0,0,
		0,0,
		PULLUP_V0,PULLUP_NONE,0,
		NO_KEYS,NoBatt,MIN_VOLTAGE,0},
	// MPA2 PP
	{MPA2_EEPROM_ADDR,MPA2_TMP_ADDR,0,0,0,
		MPA2_EEPROM_SIZE,MPA2_PAGE_SIZE,
		PULLUP_V3p3,PULLUP_2p9K,1,
		NO_KEYS,MPA2PP,MIN_VOLTAGE,0},
	// MC95 PP
	{MPA2_EEPROM_ADDR,MPA2_TMP_ADDR,0,0,0,
		MPA2_EEPROM_SIZE,MPA2_PAGE_SIZE,
		PULLUP_V3p3,PULLUP_2p9K,0,
		NO_KEYS,MC95,3.1},
	// MC18 PP+
	{MPA3_EEPROM_ADDR,MPA3_TMP_ADDR,MPA3_GG_ADDR,0,0,
		MPA3_EEPROM_SIZE,MPA3_PAGE_SIZE,
		PULLUP_V3p3,PULLUP_2p9K,1,
		MPA3_KEYS,MC18,MIN_VOLTAGE,0},
	// Rouge
	{MPA3_EEPROM_ADDR,MPA3_TMP_ADDR,MPA3_GG_ADDR,0,0,
		MPA3_EEPROM_SIZE,MPA3_PAGE_SIZE,
		PULLUP_V3p3,PULLUP_3p4K,1,
		MPA3_KEYS,Rouge,MIN_VOLTAGE,0},
	// Frenzy
	{MPA3_EEPROM_ADDR,MPA3_TMP_ADDR,MPA3_GG_ADDR,0,0,
		MPA3_EEPROM_SIZE,MPA3_PAGE_SIZE,
		PULLUP_V3p3,PULLUP_3p4K,1,
		MPA3_KEYS,Frenzy,MIN_VOLTAGE,0},
	// IronMan
	{MPA2_EEPROM_ADDR,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,0,
		QC_EEPROM_SIZE,QC_PAGE_SIZE,
		PULLUP_VBATT,PULLUP_2p3K,0,
		IRONMAN_KEYS,IronMan,MIN_VOLTAGE,0},
	// Pollux
	{MPA2_EEPROM_ADDR,0,0,0,0,
		POLLUX_EEPROM_SIZE,POLLUX_PAGE_SIZE,
		PULLUP_VBATT,PULLUP_5p1K,0,
		NO_KEYS,Pollux,MIN_VOLTAGE,0},
	// Falcon
	{MPA2_EEPROM_ADDR,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,0,
		QC_EEPROM_SIZE,QC_PAGE_SIZE,
		PULLUP_VBATT,PULLUP_5p1K,0,
		FALCON_KEYS,Falcon,MIN_VOLTAGE,0},
	// Hawkeye
	{0,0,0,AUTH_CHIP_ADDR,1,
		AUTH_EEPROM_SIZE,0,
		PULLUP_VBATT,PULLUP_NONE,0,
		NO_KEYS,Hawkeye,MIN_VOLTAGE,0},
	// Sentry EEPROM
	{MPA2_EEPROM_ADDR,MPA3_TMP_ADDR,MPA3_GG_ADDR,AUTH_CHIP_ADDR,0,
		QC_EEPROM_SIZE,MPA2_PAGE_SIZE,
		PULLUP_VBATT,PULLUP_5p1K,0,
		FALCON_KEYS,Sentry,MIN_VOLTAGE,0},
	// Galactus Auth data
	{0,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,1,
		AUTH_EEPROM_SIZE,0,
		PULLUP_VBATT,PULLUP_2p3K,0,
		FALCON_KEYS,Galactus,MIN_VOLTAGE,0},
	// Frozone EEPROM
	{MPA2_EEPROM_ADDR,0,0,0,0,
		POLLUX_EEPROM_SIZE,POLLUX_PAGE_SIZE,
		PULLUP_VBATT,PULLUP_5p1K,0,
		NO_KEYS,Frozone,MIN_VOLTAGE,0},
	// Value Tier Auth data
	{0,0,MPA3_GG_ADDR,AUTH_CHIP_ADDR,1,
		MAX_AUTH_SIZE,0,
		PULLUP_VBATT,PULLUP_2p3K,0,
		NO_KEYS,Meteor,MIN_VOLTAGE,0},
};

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

	// Setup serial port
	Serial.begin(9600);

	ShowVersion();

	// Setup I2C

	StartWire(); // restart Wire stuff
	FlushBuffer(0); // Blank data buffer
	Status();

	Serial.print(F("> ")); // Print prompt
}

void loop()
{
	static uint8_t inch, done = 0, pos = 0;
	static char buf[10] = {0};
	enum Command which;

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
			case HELP_MENU:  // Help menu
				DoMenu();
				break;
			case SHOW_VERSION:  // Show version information
				ShowVersion();
				break;
			case SHOW_STATUS:  // Show status information
				Serial.println();
				Status();
				break;
			case SELECT_BATTERY:  // Set battery type
				SetType();
				break;
			case READ_EEPROM:  // Read EEPROM into buffer
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].EEPROM_Address || Batts[BatteryType].ActiveClk)
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
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].CryptoAddress || Batts[BatteryType].ActiveClk)
					not_sup();
				else
				{
					StartWire();
					AUTH_Read();
				}
				break;
			case WRITE_EEPROM:  // Write EEPROM from buffer
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].EEPROM_Address || Batts[BatteryType].ActiveClk)
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
				else if (!Batts[BatteryType].CryptoAddress || Batts[BatteryType].ActiveClk)
					not_sup();
				else
				{
					if (BufferBlank)
					{
						NoData();
						if (!KeepGoing())
							break;
					}
					StartWire();
					switch (BatteryType)
					{
					case HAWKEYE_BATT:
					case GALACTUS_BATT:
					case IRONMAN_BATT:
					case FALCON_BATT:
					case METEOR_BATT:
						AUTH_Write();
						break;
					default:
						not_sup();
					}
				}
				break;
			case UPDATE_EEPROM:  // Update EEPROM from buffer
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].EEPROM_Address || Batts[BatteryType].ActiveClk)
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
					}
				}
				break;
			case UPDATE_AUTH:  // Update auth chip from buffer
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].CryptoAddress || Batts[BatteryType].ActiveClk)
					not_sup();
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
				else if (!Batts[BatteryType].EEPROM_Address || Batts[BatteryType].ActiveClk)
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
				else if (!Batts[BatteryType].CryptoAddress || Batts[BatteryType].ActiveClk)
					not_sup();
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
				FlushBuffer(1);
				break;
			case LOAD_HEX:  // Load buffer
				if (!BatteryType)
					NoType();
				else // Good to go
					HEX_Read();
				break;
			case DUMP_HEX: // Dump buffer
				if (!BatteryType)
					NoType();
				else // Good to go
					HEX_Write();
				break;
			default:
Unknown:
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
	return (0);
}

void ShowVersion(void)
{
	Serial.println();
	Serial.print(F("Battery Updater Version "));
	Serial.println(F(VERSION));
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
				return(cmd);
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
		if (Batts[BatteryType].CryptoAddress)
		{ // Using an auth chip
			Serial.println(F("RA - Read battery Auth chip data into buffer"));
			Serial.println(F("WA - Write battery Auth chip data from buffer"));
			Serial.println(F("UA - Update battery Auth chip data from buffer"));
		}

		if (Batts[BatteryType].EEPROM_Address)
		{ // Using EEPROM for data
			Serial.println(F("RE - Read battery EEPROM into buffer"));
			Serial.println(F("WE - Write battery EEPROM from buffer"));
			Serial.println(F("UE - Update battery EEPROM from buffer"));
		}
	}
	Serial.println(F("E  - Erase buffer"));
	Serial.println(F("LS  - Load hex file over the serial connection into buffer"));
	Serial.println(F("DS  - Dump buffer over the serial connection as hex file"));
	Serial.println(F("VER  - Show version information"));
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
			Serial.print(F(" Bytes of"));
		}
		if (!Batts[BatteryType].EEPROM_Address)
			Serial.println(F(" Auth chip storage"));
		else
			Serial.println(F(" EEPROM storage"));
	}
	else
		Serial.println(F("No Battery Selected!"));
}

// Function to print message for no selected battery type
void NoType(void)
{
	Serial.println(F("Please select a battery type first!"));
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
			Status();
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
int HEX_Read(void)
{
	unsigned char x;
	bool done;
	unsigned int Address;		// Record offset/address
	unsigned char RecLen;		// Record length
	unsigned char RecType;		// Record type
	unsigned char DataBuf[MAX_REC_LEN];	// Buffer for hex data, we only support 16 byte records max.
	unsigned int CurPage;		// Current flash page being loaded

	if (!BatteryType)  // Check for valid battery type
		return;

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
			Serial.println(F("Bad checksum"));
			FlushSerial();
			return(1);
		}

		// Handle record type
		switch (RecType) {
		case DATA_REC:	// Data record
			// Load data to buffer
			for (x = 0; x < RecLen; ++x, ++Address)	// Move data
				if (Address >= Batts[BatteryType].Size)
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

// Function to write hex file from buffer
void HEX_Write(uint8_t *buf = NULL, uint16_t len = 0)
{
	uint8_t inch, *buffer = NULL;
	uint16_t cnt, cnt2, size = 0;
	char msg[80];  // Output text buffer
	uint8_t csum;  // Record checksum

	if (!BatteryType)  // Check for valid battery type
		return;

	FlushSerial();
	Serial.println(F("Hit any key to start hex file output, then hit another key when done"));
	WaitForKey();

	// Check for what to send
	if (NULL != buf && len)
	{ // Use the provided buffer
		buffer = buf;
		size = len;
	}
	else
	{ // Use the default buffer
		buffer = EEPROM_Data;
		size = Batts[BatteryType].Size;
	}

	// loop thru 16 bytes at a time
	for (cnt = 0; cnt < size; cnt += 16)
	{
		// Output hex record
		sprintf(msg,":10%04X00",cnt);
		Serial.print(msg);

		csum = 0x10; // Startup checksum
		csum += cnt & 0xff;
		csum += cnt  >> 8;

		for (cnt2 = 0; cnt2 < 16; ++cnt2)  // Loop thru 16 byte record
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
		Serial.println(F("Bad revision byte!"));
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
		Serial.println(F("Bad revision byte!"));
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
		Serial.println(F("Bad revision byte!"));
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

// Function to get a y/n on continuing the current process
uint8_t KeepGoing()
{
	uint8_t inch;

	FlushSerial();
	Serial.print(F("   Continue operation (y/n)?"));
	while(!Serial.available())
	;
	inch = Serial.read();
	Serial.println();
	if (inch != 'y' && inch != 'Y')
		return(0);

	return(1);
}

void ActiveClk()
{
}

void PassiveClk()
{
}

