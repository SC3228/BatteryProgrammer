// Battery programmer/reader program for V2 devices
/*
**************************************************************************
**** NOTE: You must select Adafruit Feather M4 CAN as the board type! ****
**************************************************************************
*/

#define VERSION "1.30"

/*
1.30	Removed SerialConnect, not used anywhere.

1.29	Added time since first use to V2 PP+ batts in dgg output

1.28	Added storage of CPU type, 0 - SAMD51, 1 - SAME51.
		Fixed bug in auth chip update that cased "not supported" error.
		Fixed bug where auth update didn't error out on chip read errors.
		Moved USB serial start to begining of setup()
		
1.27	Moved MV2 command to under the normal MB command
		"No supported" the MB commnand for all but V2 PP+
		because no other battery has support for that yet.

1.26	Added SAMD vs SAME detection to allow moving NEO pixel pin
		This adds support for CAN Feather boards.

1.25	Eliminated old code that used PULL_ENB pin

1.24	Improved the speed of large EEPROM read/writes
		for COMET batts.

1.23	Removed Frozone EEPROM battery type, not needed.
		Fixed VBD saying Comet batt was OK if EEPROM failed
		Fixed a bug in the printing of the new VT cell ident data

1.22	Added support for VT 16KB EEPROM data

1.21	Fixed an issue where the battery temp values were unsigned

1.20	Cleaned up the V2 PP+ show data stuff
		Fixed GG current being an unsigned instead of signed
		Added test of RRM (Recover ROM mode) command

1.19	Added a function to set the timestamp to 31 days past the QMAX_DAY
		value to force a calibration.
		Added display of QMAXDOD to DGG command
		Fixed address of GG in ROM mode
		Fixed ROM mode gauge update

1.18	Fixed a bug in the read of QMAX_DAY which would overwrite the next
		30 bytes of data.
		Added a check for the gas gauge in PP+ batts being unsealed when you
		do a VBD command.
		Added descriptions to known addresses for scan bus command.

1.17	Added support for showing the time since first use and Qmax day
		for new gas gauge batts.
		Added support for reading the temp from the MPA2 temp chip.
		Added UHD "Update hex data" command to auto update battery data

1.16	Small changes to gas gauge update function after code review.
		Fixed an issue in displaying the time in temp/cap ranges

1.15	Changed to combined key arrays to be able to use a common
		key file with Android, the battery programming box and ZBAT.
		Seperated out the battery type data to a new .cpp and include file
		to make maintaing the list and using the code on BattProgLib simpler.
		Added checking/printing of V2 PP+ cell identification data
		Modified the update gas gauge function as per requests from Chris P.

1.14	Finished GG drive strength function.
		Changed V2 pullups from 3.3V to Vbatt

1.13	Fixed green LED during gas gauge update.
		Started adding drive strength function.

1.12	Added display of health and acc chgr records to V2 batts.
		Added busy LED indication to all file acceses.

1.11	Fixed green LED during file save
		Removed sync command, not needed.
		Removed extra check of data block 3 from V2 validation.
		Added format disk command

1.10	Added I2C drive strength setting to GG update function

1.9		Added code to call firmware update when Zebra internal
		code is loaded into a JDM box

1.8		Claned up printing of resetting message on UF command
		Added "SB" command to scan the I2C bus
		Added "UGG" command to program a .fs file.
		Added update auth for PP+ V2 batts
		Cleaned up JDM version selection macros

1.7		Added SharePoint link to startup and help messages

1.6		Added support for update battery command to PP+ V2 batts.
		Changed help text for update firmware command to show UF iinstead of LF.

1.5		Added automatcally ask for battery type on battery insertion.
		Added SN and HW version to inital startup message
		Added flag to tell if batttery should have a thermistor.
		Split out the battery data table to a seperate .h file
		Added a fix for 0 byte read errors. Restarted the FAT filesystem
		before every read/write/dir listing to resync it flash image.
		Added code to force JDM mode on invalid serial number.

1.4		Did a work around for bytes of 0 returned on file reads

1.3		Changed name to BattProgV2
		removed all use of F() for storing strings in flash. Not needed for ARM architecture
		General clean up.
		Added a prompt before restarting to load new firmware

1.2		Changed SN and HW rev storage. SN is now first two bytes of slot 0. HW rev is first 2 bytes of slot 1

1.1		Changed thermistor pull up from 33K to 5.1K to support BLE battery micro
		Changed relays to only be on when we have a battery inserted, and a valid battery type
		Improved help menu readability

1.0		Conversion from Arduino Mega version 2.19 to Adafruit M4 Express

*/

// Includes
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include <Adafruit_NeoPixel.h>
#include <SoftWire.h>	// I2C lib

#include "pwmbatt.h"
#include "Batts.h"
#include "BattProg.h"
#include "aes.h"
#include "Auth.h"
#include "Manufacture.h"
#include "EEPROM.h"
#include "GasGauge.h"

// Local prototypes
bool CheckVoltage(bool ForceCheck = false);

// Status LED stuff
#define NEO_PIN_E	8
#define NEO_PIN_D	7
#define NUMPIXELS 1
Adafruit_NeoPixel statusLED;

// I2C stuff
#define I2C_BUFSIZE 64
#define SDA_PIN 21
#define SCL_PIN 22
static uint8_t txBuf[I2C_BUFSIZE], rxBuf[I2C_BUFSIZE];
SoftWire Wire(SDA_PIN,SCL_PIN);  // SoftWire Object

// A to D stuff
#define REF_VOLT	3300  // ADC ref voltage in mv
#define FULL_SCALE	1023  // ADC full scale reading
#define R1	3650  // Battery voltage resistor divider values in ohms
#define R2	6810
#define RT	5100  // Thermistor divider resistor
#define MAX_THERM	500000  // Max alowable thermistor resistance value

// Pin defines
#define BATT_PLUS	A0
#define THERM_PIN	A1
#define LED_PIN 13

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
						,"MV2", "FC", "RRM"
						,"UF","FD"
						,"SB","UGG"
						,"DDS"
						,"RTC","UHD"
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
				, MANUF_V2, FORCE_CAL, RECOVER_ROM_MODE
				, UPDATE_FIRMWARE, FORMAT_DISK
				, SCAN_BUS, UPDATE_GG
				, SHOW_DRIVE_STRENGTH
				, READ_TEMP_CHIP, UPDATE_HEX_DATA
				, NO_CMD };

// Auth serial number
uint16_t AuthSN = 0, AuthHW = 0;

// JDM/Zebra stuff
uint8_t JDM_Mode = 1;

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

// Get a battery type flag
uint8_t GetBatteryType = 0;;

// Use verbose output flag
uint8_t Verbose = 1;

// CPU type
uint8_t CPU_Type;

void setup()
{
	// Setup serial port
	Serial.begin(9600);

	// Start up the status NeoPixel
	statusLED.updateLength(NUMPIXELS);
	statusLED.updateType(NEO_GRB + NEO_KHZ800);
	switch (DSU->DID.bit.FAMILY) // Set pin based on CPU type
	{
	case 0: // SAMD51
		CPU_Type = 0;
		statusLED.setPin(NEO_PIN_D);
		break;
	case 3: // SAME51
		CPU_Type = 1;
		statusLED.setPin(NEO_PIN_E);
		break;
	default:  // ???
		while (1)
		{
			Serial.println("Unknown CPU type!!!");
			delay(3000);
		}
		break;
	}
	statusLED.begin();

	delay(50);
	STATUS_LED_WAITING

	// Setup relay control pin
	digitalWrite(RELAY_CTRL,LOW);
	pinMode(RELAY_CTRL,OUTPUT);

	// Setup pullup control GPIOs
	InitPullupPins();

	// Setup Flash drive
	USBdiskInit();

	// Good to go!
	STATUS_LED_ONLINE

	// Setup I2C
	RelayOff(); // Open relays
	SetVoltage(PULLUP_V3p3);  // Set to 3.3V pullup voltage
	SetResistor(PULLUP_10K); // Set to 10K pullup resistors
	PassiveClk();  // Default to passive clock drive

	// Startup I2C
	StartWire();

	while (!Serial.dtr())
		delay(500);   // wait for usb
	delay(500);

	// Show serial number/version and set mode
	if (AUTH_ShowSN())
	{
		Serial.println("Invalid auth chip!");
		while(1)
		{
			STATUS_LED_BUSY
			delay(500);
			STATUS_LED_OFF
			delay(500);
		}
	}

	ShowVersion();

	// Check for correct "mode"
#ifdef ZEBRA_MODE
	Serial.println("For Zebra Inc. internal use ONLY!!!");
	Serial.println();
	if (JDM_Mode)
	{
		Serial.println("Invalid software for this device!!!");
		while(1)
			FWupdate();
	}
#else
	Verbose = 0;
#endif

	ClearValid();  // Set data to not valid

	FlushBuffer(0); // Blank data buffer

	Status();

	CheckBattery();  // Check for battery in/out

	Serial.print("> "); // Print prompt
}

void loop()
{
	static uint8_t inch, done = 0, pos = 0, ret = 0;
	static char buf[10] = {0};
	enum Command which;

	// Check for connected serial port
	if (!Serial.dtr())
	{
		// Discconect battery if needed
		if (HaveBattery)
		{
			HaveBattery = 0;
			RelayOff();
		}
		STATUS_LED_WAITING  // Show no comm status
		pos = 0; // Flush command buffer
		while (!Serial.dtr())
			delay(200);   // wait for usb serial to come back

		delay(500);
		// Connection is back
		if (BatteryType)
			STATUS_LED_READY
		else
			STATUS_LED_ONLINE

		if (!CheckBattery())
			Serial.printf("\r\nNo battery detected\r\n");
		Serial.println();
		Status();
		Serial.print("> "); // Print prompt
	}

	// Check for change in battery status
	if (CheckBattery() || GetBatteryType)
	{
		if (GetBatteryType)
		{
			ClearValid(); // Clear valid data flags
			SetType();  // Get battery type
			GetBatteryType = 0;
		}

		FlushSerial();
		Serial.println(); // Print prompt
		Serial.print("> ");
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
				Serial.print("\010 \010"); // Clear character on screen
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
			case UPDATE_GG:
				switch (BatteryType)
				{
				case 0:
					NoType();
					break;
				case PPP_V2:
					if (CheckVoltage(true))  // Good voltage?
						break;
					// Update the gas gauge
					StartWire();
					GG_Update();
					break;
				default:
					not_sup();
					break;
				}
				break;
			case FORMAT_DISK:
				FormatDisk();
				break;
			case UPDATE_FIRMWARE:
				FWupdate();
				break;
			case SCAN_BUS:
				StartWire();
				ScanBus();
				break;
#ifdef ZEBRA_MODE
			case TOGGLE_VERBOSE:  // Toggle verbose output
				if (Verbose)
				{
					Verbose = 0;
					Serial.println("Verbose: Off");
				}
				else
				{
					Verbose = 1;
					Serial.println("Verbose: On");
				}
				break;
			case OVERRIDE_LOW_VOLTAGE:  // Bypass low voltage check
				if (NoCheckVoltage)
				{
					NoCheckVoltage = 0; // set flag to not do voltage check
					Serial.println("Now doing low voltage check");
				}
				else
				{
					NoCheckVoltage = 1; // set flag to not do voltage check
					Serial.println("Warning: Now skipping low voltage check!");
				}
				break;
			case OVERRIDE_SERIAL_NUM:  // Bypass auth chip serial number check
				if (NoCheckSerialNum)
				{
					NoCheckSerialNum = 0; // set flag to do serial number check
					Serial.println("Now doing serial number check");
				}
				else
				{
					NoCheckSerialNum = 1; // set flag to not do serial number check
					Serial.println("Warning: Now skipping serial number check!");
				}
				break;
#endif
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
							Serial.println(BattOK);
						}
						else
						{
							Serial.println();
							Serial.println(FailedVal);
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
							Serial.println(BattOK);
						}
						else
						{
							Serial.println();
							Serial.println(FailedVal);
						}
						break;
					case IRONMAN_BATT:
					case FALCON_BATT:
						// Read EEPROM data into the buffer
						FlushBuffer(0);
						EEPROM_Read(EEPROM_Data);
						if ((ret = Validate_EEPROM(COMBINED_BATT_DATA)))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BattOK);
						else
							Serial.println(FailedVal);
						break;
					case GALACTUS_BATT:
						// Read auth chip data into the buffer
						FlushBuffer(0);
						AUTH_Read(EEPROM_Data);
						if ((ret = Validate_AUTH(GIFTED_BATT_DATA)))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BattOK);
						else
							Serial.println(FailedVal);
						break;
					case HAWKEYE_BATT:
						// Read auth chip data into the buffer
						FlushBuffer(0);
						AUTH_Read(EEPROM_Data);
						if ((ret = Validate_AUTH(HAWKEYE_BATT_DATA)))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BattOK);
						else
							Serial.println(FailedVal);
						break;
					case METEOR_BATT:
						// Read auth chip data into the buffer
						FlushBuffer(0);
						AUTH_Read(EEPROM_Data);
						if ((ret = Validate_AUTH(METEOR_BATT_DATA)))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BattOK);
						else
							Serial.println(FailedVal);
						break;
					case COMET_BATT:
						// Read EEPROM data into the buffer
						FlushBuffer(0);
						EEPROM_Read(EEPROM_Data);
						if (!(ret = Validate_EEPROM(COMET_BATT_DATA)))
						{
							Serial.println();
							Serial.println(EEPROMok);
						}
						else
						{
							Serial.println();
							Serial.println(FailedEEPROM);
						}
						// Read auth chip data into the buffer
						FlushBuffer(0);
						AUTH_Read(EEPROM_Data);
						if (ret)
						{
							Validate_AUTH(COMET_BATT_DATA);
							CheckAuthChip();
						}
						else if ((ret = Validate_AUTH(COMET_BATT_DATA)))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BattOK);
						else
							Serial.println(FailedVal);
						break;
					case POLLUX_BATT:
						// Read EEPROM data into the buffer
						FlushBuffer(0);
						EEPROM_Read(EEPROM_Data);
						if (!Validate_EEPROM(POLLUX_BATT_DATA))
						{
							Serial.println();
							Serial.println(BattOK);
						}
						else
						{
							Serial.println();
							Serial.println(FailedVal);
						}
						break;
					case SENTRY_BATT:
						// Read EEPROM data into the buffer
						FlushBuffer(0);
						EEPROM_Read(EEPROM_Data);
						// Read Auth chip data into the buffer
						AUTH_Read(EEPROM_Data+512);
						if ((ret = Validate_EEPROM(COMBINED_BATT_DATA)))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BattOK);
						else
							Serial.println(FailedVal);
						break;
					case PPP_V2:
						// Read auth chip data into the buffer
						AUTH_Read(BD.Buf);
						if ((ret = Validate_AUTH(PPP_V2_DATA)))
							CheckAuthChip();
						else
							ret = CheckAuthChip();
						Serial.println();
						if (!ret)
							Serial.println(BattOK);
						else
							Serial.println(FailedVal);
						break;
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
						Serial.println("Auth chip is ok!");
					}
				}
				break;
#ifdef ZEBRA_MODE
			case MANUFACTURE:
				if (!BatteryType)
					NoType();
				else if (CheckVoltage())
					break;
				else // Good to go
				{
/*
					if (BufferBlank)
					{
						NoData();
						break;
					}
*/
					StartWire();
					switch (BatteryType)
					{
					case MPA2_BATT:
/*
						if (!Manufacture_EEPROM(SMART_BATT_DATA))
						{
							Serial.println();
							Serial.println(BattDone);
						}
*/
						not_sup();
						break;
					case MC18_BATT:
					case ROGUE_BATT:
					case FRENZY_BATT:
/*
						if (!Manufacture_EEPROM(GIFTED_BATT_DATA))
						{
							Serial.println();
							Serial.println(BattDone);
						}
*/
						not_sup();
						break;
					case IRONMAN_BATT:
					case FALCON_BATT:
/*
						if (Manufacture_EEPROM(COMBINED_BATT_DATA))
							break;
						if (!CheckAuthChip())
						{
							Serial.println();
							Serial.println(BattDone);
						}
*/
						not_sup();
						break;
					case GALACTUS_BATT:
/*
						if (!Manufacture_AUTH(GIFTED_BATT_DATA))
						{
							Serial.println();
							Serial.println(BattDone);
						}
*/
						not_sup();
						break;
					case HAWKEYE_BATT:
/*
						if (!Manufacture_AUTH(HAWKEYE_BATT_DATA))
						{
							Serial.println();
							Serial.println(BattDone);
						}
*/
						not_sup();
						break;
					case POLLUX_BATT:
/*
						if (!Manufacture_EEPROM(POLLUX_BATT_DATA))
						{
							Serial.println();
							Serial.println(BattDone);
						}
*/
						not_sup();
						break;
					case SENTRY_BATT:
/*
						if (!Manufacture_EEPROM(POLLUX8956_BATT_DATA))
						{
							Serial.println();
							Serial.println(BattDone);
						}
*/
						not_sup();
						break;
					case PPP_V2:
						ManufV2();
						break;
					default:
						not_sup();
					}
				}
				break;
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
			case SAVE_AUTH_CONFIG:  // Save auth chip config
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
			case READ_TEMP_CHIP:  // Read temp chip temp
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].TempAddress)
					not_sup();
				else if (CheckVoltage())
					break;
				else // ok!
				{
					StartWire();
					Serial.printf("Battery temp: %d\n\n",Temp_Read());
				}
				break;
			case READ_EEPROM:  // Read EEPROM into buffer
				ClearValid(); // Clear valid data flags
				if (!BatteryType)
					NoType();
				else if (!Batts[BatteryType].EEPROM_Address)
					not_sup();
				else if (CheckVoltage())
					break;
				else // ok!
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
				else if (!Batts[BatteryType].EEPROM_Address || COMET_BATT == BatteryType)
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
					Serial.print("Update ");
					Serial.print(Batts[BatteryType].Name);
					Serial.println(" EEPROM data (y/n)?");
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
			case UPDATE_HEX_DATA: // Auto update hex data from file
				switch (BatteryType)
				{
				case 0:
					NoType();
					break;
				case PPP_V2:
					if (CheckVoltage(true))  // Good voltage?
						break;
					// Update the battery data
					StartWire();
					Auth_Hex_Data_Update();
					break;
				default:
					not_sup();
					break;
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
					Serial.print("Update ");
					Serial.print(Batts[BatteryType].Name);
					Serial.println(" auth chip data (y/n)?");
					while(!Serial.available())
					;
					inch = Serial.read();
					if (inch != 'y' && inch != 'Y')
						break;

					StartWire();
					AUTH_Update();
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
				case COMET_BATT:
					ShowMeteor(COMET_BATT_DATA);
					break;
				case IRONMAN_BATT:
				case POLLUX_BATT:
				case FALCON_BATT:
				case SENTRY_BATT:
//				case FROZONE_EP_BATT:
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
//					case FROZONE_EP_BATT:
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
			case SHOW_DRIVE_STRENGTH: // Display/set I2C drive strength on V2 gauges
				StartWire();
				switch (BatteryType)
				{
				case PPP_V2:
					if (CheckVoltage(true))  // Good voltage?
						break;

					// Check set the drive strength
					GG_DriveStrength();
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
			case RECOVER_ROM_MODE:  // Try to recover from ROM mode
				if (!BatteryType)
					NoType();
				else if (BatteryType != PPP_V2)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
					GG_Recover();
				break;
			case FORCE_CAL:  // Set firmware runtimer to force a cal
				if (!BatteryType)
					NoType();
				else if (BatteryType != PPP_V2)
					not_sup();
				else if (CheckVoltage())
					break;
				else // Good to go
					GG_ForceCal();
				break;
#endif  // End of Zebra mode
			case LOAD_FILE:  // Load buffer from file
				ClearValid(); // Clear valid data flags
				if (!BatteryType)
					NoType();
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
				ListDir();
				break;
			default:
				Serial.println("Unknown command");
				break;
			}

NoCmd:
			FlushSerial();
			Serial.println(); // Print prompt
			Serial.print("> ");
		}
	}
}

// Do firmware update
void FWupdate(void)
{
	uint8_t inch;

	FlushSerial();
	Serial.print("Restart to load firmware (y/n)?");
	while(!Serial.available())
	;
	inch = Serial.read();
	if (inch != 'y' && inch != 'Y')
		return;
	Serial.println("\r\n\r\nRestarting to load firmware...\r\n");
	delay(2000);
	*(uint32_t *)(0x20000000 + 196608 -4) = 0xf01669ef;     // Store special flag value in last word in RAM.
	NVIC_SystemReset();
}

// Chek battery voltage
// Returns true if error for low voltage
// False if voltage is OK
bool CheckVoltage(bool ForceCheck)
{
	if (NoCheckVoltage && !ForceCheck)
		return (0);

	if ((!Batts[BatteryType].ActiveClk // If passive clk drive
				|| Batts[BatteryType].GasGaugeAddress // or using a gas gauge
				|| Batts[BatteryType].CryptoAddress) // or using an auth chip
				&& BattVoltage() <= Batts[BatteryType].MinVoltage) // And battery is <= minimum voltage
	{
		LowBatt();
		return (true);
	}

	return (false);
}

// Function to scan I2C bus for devices
void ScanBus(void)
{
	uint8_t Addr, err, found = 0;

	Serial.println("Scanning I2C bus...");

	for (Addr = 1; Addr <= 127; ++Addr)
	{
		Wire.beginTransmission(Addr);
		err = Wire.endTransmission();

		switch (err)
		{
		case 0:  // No error, found one!
			if (LOCAL_AUTH_ADDR != (Addr<<1))
			{
				if (!found)
				{
					Serial.println();
					Serial.println("Addr  Device");
				}

				found = 1;
				Serial.printf("0x%02X  ",Addr<<1);

				switch (Addr)
				{
				case GG_ROM_MODE:
					Serial.println("Gas Gauge stuck in ROM mode!");
					break;
				case MPA2_EEPROM_ADDR:
				case MPA2_EEPROM_ADDR|1:
					Serial.println("MPA2/Pollux EEPROM");
					break;
				case MPA2_TMP_ADDR:
					Serial.println("MPA2 Temp chip");
					break;
				case MPA2_BM_ADDR:
					Serial.println("MPA2 Battery Micro");
					break;
				case MPA3_EEPROM_ADDR:
				case MPA3_EEPROM_ADDR|1:
					Serial.println("PP+ EEPROM");
					break;
				case MPA3_TMP_ADDR:
					Serial.println("MPA3 Temp chip");
					break;
				case MPA3_GG_ADDR:
					Serial.println("Gas Gauge");
					break;
				case AUTH_CHIP_ADDR>>1:
					Serial.println("Authentication IC");
					break;
				default:
					Serial.println("Unknown");
					break;
				}
			}
			break;
		case 2:  // Nothing there
			break;
		case 1:  // Data too long???
		case 3:  // NAK on data transmit???
		case 4:  // Other error
		default:
			Serial.printf("Unexpected error at address: 0x%02X\r\n",Addr<<1);
			break;
		}
	}

	Serial.println();
	if (found)
		Serial.println("Done");
	else
		Serial.println("Nothing found");
}

void ShowLink(void)
{
#ifdef ZEBRA_MODE
	Serial.printf("\r\n\r\nFor more information, including manusls, frimware updates, etc., pleas see:\r\n");
	Serial.printf("\r\nhttps://zebra.sharepoint.com/sites/BatteryProgrammer\r\n\r\n");
#else
	Serial.printf("\r\n\r\n");
#endif
}

void ShowVersion(void)
{
	Serial.printf("\r\nEEPROM Programmer V2 Version %s",VERSION);
#ifdef ZEBRA_MODE
	Serial.printf("\r\n");
#else
	Serial.printf("J\r\n");
#endif
	Serial.printf("SN: %d    HW: %d.%d    CPU: %d",AuthSN,AuthHW>>8,AuthHW&0xff,CPU_Type);
	ShowLink();
}

// Clear valid data flags
void ClearValid(void)
{
	PP_Valid = 0;
	PPP_Valid = 0;
	VT_Valid = 0;
	PPPV2_Valid = 0;
}

void StartWire(void)
{
	// Setup buffers
	Wire.setRxBuffer(rxBuf,I2C_BUFSIZE);
	Wire.setTxBuffer(txBuf,I2C_BUFSIZE);

	// Make timeout longer than GG hold time
	Wire.setTimeout_ms(400);

	// Change I2C clock to ~32KHz
	Wire.setClock(32000);

	Wire.begin();  // Start/restart TWI
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
	Serial.println("Currently Available Commands:");
	Serial.println();
	Serial.println("Battery Commands:");
	Serial.println("    B    - Select battery type");
	if (BatteryType) // If we have selected a battery
	{
		Serial.println("    VBD  - Validate battery data");
		if (Batts[BatteryType].EEPROM_Address)
		{ // Using EEPROM for data
			Serial.println("    RE   - Read battery EEPROM into buffer");
			Serial.println("    WE   - Write battery EEPROM from buffer");
			Serial.println("    UE   - Update battery EEPROM from buffer");
			Serial.println("    VE   - Verify battery EEPROM against buffer");
		}

		if (Batts[BatteryType].CryptoAddress)
		{ // Using an auth chip
			Serial.println("    A    - Check battery authentication chip");
			Serial.println("    RA   - Read battery Auth chip data into buffer");
			Serial.println("    WA   - Write battery Auth chip data from buffer");
			Serial.println("    UA   - Update battery Auth chip data from buffer");
			Serial.println("    VA   - Verify battery Auth chip data against buffer");
			Serial.println("    UHD  - Auto update battery data from hex file");
		}
		if (Batts[BatteryType].TempAddress)
		{ // Using a temp chip
			Serial.println("    RTC  - Read battery temp chip");
		}
#ifdef ZEBRA_MODE
		Serial.println("    MB   - Manufacture Battery");

		if (PP_Valid)
			Serial.println("    DPP  - Display PP battery data");

		if (PPP_Valid)
			Serial.println("    DPP+ - Display PP+ battery data");
#endif
		if (Batts[BatteryType].GasGaugeAddress)
		{
			Serial.println();
			Serial.println("Gas Gauge Commands:");
#ifdef ZEBRA_MODE
			Serial.println("    DGG  - Display Gas Gauge registers");
			Serial.println("    DGGH - Display Gas Gauge registers in hex");
			if (PPP_V2 != BatteryType)
			{
				Serial.println("    HGG  - Put Gas Gauge into hibernate mode");
				Serial.println("    FSGG - Set FULLSLEEP bit");
			}
			Serial.println("    RGG  - Reset Gas Gauge");
			Serial.println("    SGG  - Seal Gas Gauge");
			Serial.println("    USGG - Unseal Gas Gauge");
			Serial.println("    DGGF - Dump Gas Gauge Flash data");
			Serial.println("    SGGF - Save Gas Gauge Flash data in hex file");
#endif
			Serial.println("    UGG  - Update gas gauge from .fs file");
		}
	}
	Serial.println();
	Serial.println("File & Data Buffer Commands:");
	Serial.println("    E   - Erase buffer");
	Serial.println("    LS  - Load hex file over the serial connection into buffer");
	Serial.println("    DS  - Dump buffer over the serial connection as hex file");
	Serial.println("    LF  - Load a hex file from the internal drive");
	Serial.println("    SF  - Save buffer to file on internal drive");
	Serial.println("    DIR - Directory of internal drive");

	Serial.println();
	Serial.println("Other Commands:");
#ifdef ZEBRA_MODE
	Serial.println("    VERB - Toggle verbose mode");
#endif
	Serial.println("    SB   - Scan I2C bus for devices");
	Serial.println("    VER  - Show version information");
	Serial.println("    STAT - Show current status");
	Serial.println("    UF   - Update the firmware");
	Serial.println("    ?    - help menu");
	ShowLink();
}

void Status(void)
{
	int therm, volts;

	if (BatteryType)
	{
		Serial.print("Battery: ");
		Serial.println(Batts[BatteryType].Name);
		if (Batts[BatteryType].EEPROM_Address)
		{
			Serial.print(Batts[BatteryType].Size);
			Serial.println(" Bytes of EEPROM storage");
		}
		if (Batts[BatteryType].CryptoAddress)
			Serial.println("632 Bytes of Auth chip storage");
	}
	else
		Serial.println("No Battery Selected!");

#ifdef ZEBRA_MODE
	if (Verbose)
		Serial.println("Verbose: On");
	else
		Serial.println("Verbose: Off");

	if (NoCheckVoltage)
		Serial.println("Warning: Now skipping low voltage check!");
#endif

	PrintVoltage();
	PrintResistor();
	PrintClock();

	if (BatteryType && HaveBattery)
	{
		volts = BattVoltage();
		Serial.printf("Battery Voltage: %d.%03dV\r\n",volts/1000,volts%1000);
		if (Batts[BatteryType].HasTherm)  // Need to check for thermistor?
		{
			therm = ThermResistance();
			if (therm > 1000000)
				Serial.println("No thermistor found!");
			else
				Serial.printf("Thermistor: %d.%03dK\r\n",therm/1000,therm%1000);
		}
	}
}

// Function to read the battery voltage
// Returns voltage in mv
int BattVoltage(void)
{
	int voltage, volts;
	int AtD;

	// Average four readings
	AtD = analogRead(BATT_PLUS);
	delay(50);
	AtD += analogRead(BATT_PLUS);
	delay(50);
	AtD += analogRead(BATT_PLUS);
	delay(50);
	AtD += analogRead(BATT_PLUS);
	AtD /= 4;

	volts = REF_VOLT * AtD / FULL_SCALE;  // Get voltage on pin in mv
	voltage = volts * (R1+R2) / R1;  // Scale to battery voltage
	return (voltage);
}

// Function to read the thermistor resistance
// Returns resistance in ohms
int ThermResistance(void)
{
	int resistance;
	int AtD;

	// Average four readings
	AtD = analogRead(THERM_PIN);
	delay(50);
	AtD += analogRead(THERM_PIN);
	delay(50);
	AtD += analogRead(THERM_PIN);
	delay(50);
	AtD += analogRead(THERM_PIN);
	AtD /= 4;
	
	if (AtD >= FULL_SCALE) // Prevent div by 0 error
		AtD = FULL_SCALE - 1;
	resistance = (RT * AtD) / (FULL_SCALE - AtD); // Get thermistor resistance
	return (resistance);
}

// Relay control functions
void RelayOn(void)
{
	digitalWrite(RELAY_CTRL,1);
	delay(50);
}

void RelayOff(void)
{
	digitalWrite(RELAY_CTRL,0);
	delay(50);
}

// Function to update battery status
// Returns true if status changed, false otherwise
uint8_t CheckBattery(void)
{
	if (HaveBattery) // Do we have a battery?
	{  // Check for removal
		if (BattVoltage() < 500)
		{
			Serial.println();  
			Serial.println("Battery removed!");  
			HaveBattery = 0;  // No battery now
			RelayOff();
			GetBatteryType = 0;  // Don't ask for the battery type
			return (1);
		}
	}
	else
	{  // Check for battery insertion
		if (BattVoltage() > 600)
		{
			Serial.println();
			Serial.printf("Battery detected!");
			if (BatteryType)  // Only turn on relay when we have a valid battery type
			{
				RelayOn();
				delay(100);
			}
			HaveBattery = 1;  // Have battery now
			if (!BatteryType)
				GetBatteryType = 1;  // Trigger asking for the battery type
			return (1);
		}
	}

	return (0);
}

// Function to print message for no selected battery type
void NoType(void)
{
	Serial.println("Please select a battery type first!");
}

// Function to print message for low battery voltage
void LowBatt(void)
{
	Serial.printf("Battery voltage must be > %d.%03d\r\n",Batts[BatteryType].MinVoltage/1000,Batts[BatteryType].MinVoltage%1000);
}

// Function to print message for not supported operation
void not_sup(void)
{
	Serial.println("Function not supported on this battery type");
}

// Function to print message for no data in buffer
void NoData(void)
{
	Serial.println("The data buffer is blank!");
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

// Funtion to manually set the battery type
void SetType(void)
{
	int cnt;

	Serial.println();
	FlushSerial();

	for (cnt = 0; cnt < MAX_BATTERY_TYPES; ++cnt)  // Loop thru battery types
	{
		Serial.printf("%d - ",cnt);
		Serial.println((Batts[cnt].Name));
	}

	// Get value
	Serial.println();
	Serial.print("Select battery type: ");
	cnt = GetInt();

	if (cnt >= 0) // Got something?
	{ // yes
		if (cnt >= MAX_BATTERY_TYPES)
			Serial.println("Not a valid choice, battery type not changed!");
		else
		{
			BatteryType = cnt;
			if (HaveBattery)  // Relay on if we have a battery
				RelayOn();
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
			Serial.println("**** For MC95 Batts supply 3.3V ****");
			Serial.println("**** to batt external power pin ****");
			Serial.println("**** and Batt+ on this box!     ****");
			Serial.println();
			break;
		default:
			break;
		}

		// Switch relays and LED
		if (BatteryType)
		{
			STATUS_LED_READY
			if (HaveBattery)
				RelayOn();
		}
		else
		{
			RelayOff();
			STATUS_LED_ONLINE
		}
	}
	else
	{ // Nope
		Serial.println("Battery type not changed!");
	}
}

// Function to read in hex file
// Returns 0 if ok, 1 on error
int HEX_Read(uint16_t size)
{
	unsigned char x;
	bool done;
	unsigned int Address;		// Record offset/address
	unsigned char RecLen;		// Record length
	unsigned char RecType;		// Record type
	unsigned char DataBuf[MAX_REC_LEN];	// Buffer for hex data, we only support 16 byte records max.

	if (!BatteryType)  // Check for valid battery type
		return(1);

	// Setup segment, address, etc.
	Address = 0;

	FlushBuffer(1);	// Clear buffer if needed/wanted

	FlushSerial();

	done = false;	// Clear finished flag
	HadError = 0;  // Clear error flag

	Serial.println("Send file now, ESC to cancel...");

	do {	// Loop to do hex records
		// Get next hex record
		do {			// Wait for a colon
			while (!Serial.available())
			;
			RecLen = Serial.read();
			if (RecLen == ESC) { // Check for user abort
				Serial.println("Stopped by user!");
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
			Serial.print("Record > ");
			Serial.print(MAX_REC_LEN);
			Serial.println(" Bytes");
			FlushSerial();
			return(1);
		}

		for (x = 0; x < RecLen; ++x)	// Get data
			DataBuf[x] = GetByte();

		GetByte();			// Get Checksum

		if (CSum) {
			Serial.println(ChecksumError);
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
					Serial.println("ERROR: Address past end of part!");
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
			Serial.println("Bad record type");
			FlushSerial();
			return(1);
		}
	} while (!done);

	FlushSerial();
	Serial.println("Done with file load.");
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
	} while((millis() - StartTime) < 50); // Make sure no data for a while
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

	Serial.printf("Bad Character: %02x '%c'\r\n",c,c);	// Bad character
	HadError = 1;  // Set error flag
	return (0);
}

// Function to reset the EEPROM buffer
void FlushBuffer(int ask)
{
	uint16_t cnt;
	uint8_t inch;

	FlushSerial();
	if (ask)
	{
		Serial.println("Erase data buffer (y/n)?");
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

	Serial.println("Buffer erased");

	BufferBlank = 1;
	return;
}

// Function to reset the EEPROM buffer
void TestPattern(void)
{
	int cnt;
	uint8_t inch;

	Serial.println("Overwrite data buffer (y/n)?");
	while(!Serial.available())
	;
	inch = Serial.read();
	if (inch != 'y' && inch != 'Y')
		return;

	// Generate EEPROM test data
	for (cnt = 0; cnt < MAX_EEPROM_SIZE; ++cnt)
		EEPROM_Data[cnt] = (cnt >> 4) ;

	Serial.println("Test pattern in buffer!");
	BufferBlank = 0;
}

// Function to write hex file from buffer
void HEX_Write(uint16_t size, uint8_t *buf)
{
	uint8_t inch, *buffer = NULL;
	uint16_t cnt, cnt2, reclen;
	uint8_t csum;  // Record checksum

	if (!BatteryType)  // Check for valid battery type
		return;

	FlushSerial();
	Serial.println("Hit any key to start hex file output, then hit another key when done");
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
		Serial.printf(":%02X%04X00",reclen,cnt);

		csum = reclen; // Startup checksum
		csum += cnt & 0xff;
		csum += cnt  >> 8;

		for (cnt2 = 0; cnt2 < reclen; ++cnt2)  // Loop thru 16 byte record
		{
			inch = buffer[cnt+cnt2]; // Get next byte
			csum += inch;
			Serial.printf("%02X",inch);
		}
		Serial.printf("%02X\r\n",(~csum+1)&0xff);  // Ouput checksum

		// Check for user hitting ESC
		if (Serial.available() && ESC == Serial.read())
		{
			Serial.println("Interrupted by user");
			goto ReadError;
		}
	}
	Serial.println(":00000001FF");
	WaitForKey();
	Serial.println("Hex file output done.");
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
				Serial.print("\010 \010"); // Clear character on screen
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
	char name[MAX_NAME_LEN+1];

	STATUS_LED_BUSY;

	ResetFS(); // Reset file system first

	Serial.println("                        Name                       Date             Size");
	dir = fatfs.open("/");  // Open root dir

	while ((file = dir.openNextFile()))
	{
		if (!file.isDir()) {
			file.getName(name,MAX_NAME_LEN);
			Serial.printf("%40.40s   ",name);
			file.printModifyDateTime(&Serial);
			file.printFileSize(&Serial);
			Serial.println();
		}
		file.close();
	}

	dir.close();
	Serial.println();
	STATUS_LED_READY;
}

// Routine to save buffer to file
void SaveFile(uint16_t size, uint8_t *buf)
{
	File file;
	char name[MAX_NAME_LEN+1], msg[100];
	uint8_t inch, *buffer;
	uint16_t cnt, cnt2, reclen;
	uint8_t csum;  // Record checksum
	uint16_t Year, Month, Day, Hour, Minute, Second;

	if (!BatteryType)  // Check for valid battery type
		return;

	Year = 1999;
	Month = 1;
	Day = 1;
	Hour = 11;
	Minute = 59;
	Second = 0;

	ResetFS(); // Reset file system first

	// Get file name
	FlushSerial();
	Serial.println("Enter file name for save");
	Serial.printf("%d chars max, no extension: ",MAX_NAME_LEN);
	name[0] = 0;  // Terminate buffer
	if (!GetText(name,MAX_NAME_LEN))
	{
		Serial.println();
		Serial.println("Canceled");
		Serial.println();
		return;
	}
	sprintf(msg,"%s.hex",name);

	// Check for file already there
	if (file = fatfs.open(msg, O_READ))
	{
		file.close(); // Close file
		Serial.println("File exists, overwrite (y/n)?");
		while(!Serial.available())
		;
		inch = Serial.read();
		if (inch != 'y' && inch != 'Y')
			return;
	}

	STATUS_LED_BUSY;

	// Open/Create file
	if (!(file = fatfs.open(msg, O_CREAT | O_WRITE | O_TRUNC)))
	{
		STATUS_LED_READY;
		Serial.println("Error opening file!");
		return;
	}
  
	// set creation date time
	if (!file.timestamp(T_CREATE, Year, Month, Day, Hour, Minute, Second))
		Serial.println("Set create time failed.");
	// set write/modification date time
	if (!file.timestamp(T_WRITE, Year, Month, Day, Hour, Minute, Second))
		Serial.println("Set write time failed.");
	// set access date
	if (!file.timestamp(T_ACCESS, Year, Month, Day, Hour, Minute, Second))
		Serial.println("Set access time failed.");

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
	file.println(":00000001FF");
	file.close(); // Close file
	RefreshDrive();
	Serial.println("File save done.");

	STATUS_LED_READY;
}

// Routine to get file name from serial port
// Input is buffer to hold up to 8 character name and terminating zero
uint8_t GetText(char *buf,uint8_t MaxLen,uint8_t NoFilter)
{
	char inch;
	int done = 0, pos = 0;

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
				Serial.print("\010 \010"); // Clear character on screen
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
// Returns 0 if ok, 1 on error
uint8_t HEX_ReadFile(uint16_t size)
{
	char name[MAX_NAME_LEN+1], msg[MAX_NAME_LEN+5];

	ResetFS(); // Reset file system first

	if (!BatteryType)  // Check for valid battery type
		return (1);

	// Get file name
	FlushSerial();
	Serial.println("Enter file name for read");
	Serial.printf("%d chars max, no extension: ",MAX_NAME_LEN);
	name[0] = 0;  // Terminate buffer
	if (!GetText(name,MAX_NAME_LEN))
	{
		Serial.println();
		Serial.println("Canceled");
		Serial.println();
		return (0);
	}
	sprintf(msg,"%s.hex",name);

	FlushBuffer(1);  // Clear buffer if needed/wanted

	FlushSerial();

	if (GetHexFile(msg,size))
		return (1);

	Serial.println("Done with load from SD.");
	BufferBlank = 0;  // Stuff in buffer
	return (0);
}

// Read a hex file from SD card, given the name
uint8_t GetHexFile(char *name, uint16_t size, bool doStatus)
{
	unsigned char x;
	bool done;
	unsigned int Address;		// Record offset/address
	unsigned char RecLen;		// Record length
	unsigned char RecType;		// Record type
	unsigned char DataBuf[MAX_REC_LEN];	// Buffer for hex data, we only support 16 byte records max.
	File file;

	STATUS_LED_BUSY;

	// Open file
	if (!(file = fatfs.open(name, O_READ)))
	{
		Serial.println("Error opening file!");
		STATUS_LED_READY;
		return (1);
	}

	// Setup segment, address, etc.
	Address = 0;

	done = false;	// Clear finished flag
	HadError = 0;  // Clear error flag

	do {	// Loop to do hex records
		// Get next hex record
		do {			// Wait for a colon
			if (1 != file.read(&RecLen,1) || HadError)  // Get next byte from file and check for read error
			{
				Serial.println(ReadError);
				file.close();
				STATUS_LED_READY;
				return(1);
			}
		} while (RecLen != ':');

		CSum = 0;		// Clear checksum

		RecLen = GetFileByte(&file);	// Get Reord Length
		if (HadError)
		{
			file.close();
			STATUS_LED_READY;
			return(1);
		}

		Address = GetFileByte(&file);	// Get Offset (Address)
		if (HadError)
		{
			file.close();
			STATUS_LED_READY;
			return(1);
		}
		Address = Address << 8;
		Address |= GetFileByte(&file);
		if (HadError)
		{
			file.close();
			STATUS_LED_READY;
			return(1);
		}

		RecType = GetFileByte(&file);	// Get Record Type
		if (HadError)
		{
			file.close();
			STATUS_LED_READY;
			return(1);
		}

		if (RecLen > MAX_REC_LEN) {	// Check for record to big
			Serial.println("Record > 32 bytes");
			file.close();
			STATUS_LED_READY;
			return(1);
		}

		for (x = 0; x < RecLen; ++x)	// Get data
		{
			DataBuf[x] = GetFileByte(&file);
			if (HadError)
			{
				file.close();
				STATUS_LED_READY;
				return(1);
			}
		}

		GetFileByte(&file);			// Get Checksum
		if (HadError)
		{
			file.close();
			STATUS_LED_READY;
			return(1);
		}

		if (CSum)
		{
			Serial.println(ChecksumError);
			file.close();
			STATUS_LED_READY;
			return(1);
		}

		// Handle record type
		switch (RecType) {
		case DATA_REC:	// Data record
			// Load data to buffer
			for (x = 0; x < RecLen; ++x, ++Address)	// Move data
				if (Address >= size)
				{
					Serial.println("ERROR: Address past end of part!");
					file.close();
					STATUS_LED_READY;
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
			Serial.println("Bad record type");
			file.close();
			STATUS_LED_READY;
			return(1);
		}
	} while (!done);

	file.close();

	if (doStatus)
		STATUS_LED_READY;

	return(0);
}

// Routine to get a hex byte from a file
unsigned char GetFileByte(File *inFile)
{
	uint8_t val;
	int inch;

	if ((inch = inFile->read()) < 0)  // Get next byte from file
	{
		Serial.println(ReadError);
		HadError = 1;
		return (0);
	}

	val = ConvHex(inch);	// Convert from hex
	val = val << 4;		// Shift it over

	if ((inch = inFile->read()) < 0)  // Get next byte from file
	{
		Serial.println(ReadError);
		HadError = 1;
		return (0);
	}

	val |= ConvHex(inch);	// Convert from hex and add in

	CSum += val;		// Add to checksum

	return (val);
}

uint8_t CheckClk(void)
{
	if (!digitalRead(SCL_PIN))
	{
		Serial.println("Error: I2C clock is low!");
		return (1);
	}

	return (0);
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
		Serial.print("'E'EPROM or 'A'uth chip data? ");
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
