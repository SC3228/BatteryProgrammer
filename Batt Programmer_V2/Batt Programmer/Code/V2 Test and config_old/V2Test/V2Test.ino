// Battery programmer/reader V2 Test program

#define VERSION "1.2"
/*
1.2		Fixed failed thermistor test was showing MAX_THERM
		instead of the measured bad value.

1.1		Moved the auth test to just before the format disk
		test in do all tests.
		Changed pull up and battery voltage command names to
		make things clearer.

1.0		Initial version
*/

// Includes
#include "Adafruit_TinyUSB.h"
#include "Adafruit_SPIFlash.h"
#include <SdFat.h>
#include "ff.h"
#include "diskio.h"
#include <Adafruit_NeoPixel.h>
#include <SoftWire.h>	// I2C lib

#include "BattProg.h"
#include "Auth.h"

extern uint8_t GotSerial;

// I2C stuff
#define BUFSIZE 64
#define SDA_PIN 21
#define SCL_PIN 22
static uint8_t txBuf[BUFSIZE], rxBuf[BUFSIZE];  // RX/TX buffers
SoftWire Wire(SDA_PIN,SCL_PIN);  // SoftWire Object

// Status LED stuff
#define NEO_PIN 8
#define NUMPIXELS 1
Adafruit_NeoPixel statusLED(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

// A to D stuff
#define REF_VOLT	3300  // ADC ref voltage in mv
#define FULL_SCALE	1023  // ADC full scale reading
#define R1	3650  // Battery voltage resistor divider values in ohms
#define R2	6810
#define RT	5100  // Thermistor divider resistor

// Pin defines
#define BATT_PLUS	A0
#define THERM_PIN	A1
#define LED_PIN 13

// Disk stuff
// Volume name, up to 11 characters
#define DISK_LABEL    "BATTPROG"
// On-board external flash (QSPI or SPI) macros should already
// defined in your board variant if supported
// - EXTERNAL_FLASH_USE_QSPI
// - EXTERNAL_FLASH_USE_CS/EXTERNAL_FLASH_USE_SPI
#if defined(EXTERNAL_FLASH_USE_QSPI)
Adafruit_FlashTransport_QSPI flashTransport;

#elif defined(EXTERNAL_FLASH_USE_SPI)
Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

#else
#error No QSPI/SPI flash are defined on your board variant.h !
#endif
Adafruit_SPIFlash flash(&flashTransport);
// file system object from SdFat
FatFileSystem fatfs;
// Elm Cham's fatfs objects
FATFS elmchamFatfs;
uint8_t workbuf[4096]; // Working buffer for f_fdisk function.
bool HaveFlash; // True if we could init the flash chip.
bool IsFormated; // True if flash chip was already formatted

// Set to true when connected to serial
bool SerialConnect;

// List of commands
const char *Cmds[] = { "?","RC","RO"
						,"P10","P6","P5","PN"
						,"PBV","P3V"
						,"AC","PC"
						,"UF","DAT"
						,"TPR","TT","TPV","TBV","TS", "TA"
						,"ST","SBV"
						,"FD", "DAC"
						};

enum Command { HELP_MENU, RELAY_CLOSE, RELAY_OPEN
				, PULL_10K, PULL_6p8K, PULL_5p1K, PULL_NONE
				, PULL_VBATT, PULL_3p3V
				, SET_ACTIVE, SET_PASSIVE
				, UPDATE_FIRMWARE,DO_ALL_TESTS
				, TEST_PULLUPS, TEST_THERM, TEST_VOLT_SEL, TEST_VBATT, TEST_STATUS, TEST_AUTH
				, THERM_RES, BATT_VOLT
				, FORMAT_DISK, DUMP_AUTH_CONFIG
				, NO_CMD };

// Auth serial number string
char AuthSN[32] = "";

void setup()
{
	HaveFlash = false;  // Default to no flash chip
	IsFormated = false; // Default to flash not formatted yet

	// Start up the status NeoPixel
	statusLED.begin();
	delay(50);
	STATUS_LED_WAITING;

	// Setup relay control pin
	digitalWrite(RELAY_CTRL,LOW);
	pinMode(RELAY_CTRL,OUTPUT);

	// Set flags
	SerialConnect = 0;

	// Setup pullup control GPIOs
	InitPullupPins();

	SetVoltage(PULLUP_V3p3);  // Default to VBatt pullup voltage
	SetResistor(PULLUP_10K); // Default to no pullup resistors
	PassiveClk();  // Default to passive clock drive

	// Setup serial port
	Serial.begin(115200);
	while (!Serial.dtr())
		delay(500);   // wait for usb
	delay(500);

	// Good to go!
	STATUS_LED_ONLINE
	StartWire(); // Setup I2C

	// Initialize flash library
	if (flash.begin())
		HaveFlash = true;

	Serial.printf("\r\nBattProg2 Unit Test Ver: %s\r\n\r\n> ",VERSION); // Print version and prompt
}

void loop()
{
	static uint8_t inch, done = 0, pos = 0;
	static char buf[10] = {0};
	enum Command which;
	int therm;

	// Check for connected serial port
	if (!Serial.dtr())
	{
		STATUS_LED_WAITING  // Show no comm status
		pos = 0; // Flush command buffer
		while (!Serial.dtr())
			delay(200);   // wait for usb serial to come back

		delay(500);
		STATUS_LED_ONLINE

		Serial.println();
		Serial.print(F("> ")); // Print prompt
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
			case DUMP_AUTH_CONFIG:
				AUTH_Dump();
				break;
			case FORMAT_DISK:
				if (IsFormated)
				{
					Serial.println("Flash chip was already formatted this session!");
					break;
				}
				if (HaveFlash)
					FormatDisk(false);
				else
					Serial.println("No flash chip was found on startup, unable to format!");
				break;
			case DO_ALL_TESTS:
				TestAll();
				break;
			case TEST_VBATT:
				TestBattInput(false);
				break;
			case TEST_VOLT_SEL:
				TestVoltageSelect();
				break;
			case TEST_THERM:
				TestThermistor(false);
				break;
			case TEST_PULLUPS:  // Run thru pull up value & active drive check
				TestPullups(false);
				break;
			case TEST_AUTH:  // Check Auth chip
				TestAuth();
				break;
			case TEST_STATUS:
				TestStatusLED();
				break;
			case UPDATE_FIRMWARE:
				Serial.println("Restarting to load firmware...");
				delay(1000);
				*(uint32_t *)(0x20000000 + 196608 -4) = 0xf01669ef;     // Store special flag value in last word in RAM.
				NVIC_SystemReset();
				break;
			case RELAY_CLOSE: // Relay on
				RelayOn();
				Serial.println("Relays Closed");
				break;
			case RELAY_OPEN: // Relay off
				RelayOff();
				Serial.println("Relays Open");
				break;
			case SET_ACTIVE:
				ActiveClk();
				Serial.println("Active clock");
				break;
			case SET_PASSIVE:
				PassiveClk();
				Serial.println("Passive clock");
				break;
			case PULL_VBATT:
				SetVoltage(PULLUP_VBATT);
				Serial.println("Pullup to VBatt");
				break;
			case PULL_3p3V:
				SetVoltage(PULLUP_V3p3);
				Serial.println("Pullup to 3.3V");
				break;
			case PULL_10K:
				SetResistor(PULLUP_10K);
				Serial.println("Pullup resistors set to 10K");
				break;
			case PULL_6p8K:
				SetResistor(PULLUP_6p8K);
				Serial.println("Pullup resistors set to 6.8K");
				break;
			case PULL_5p1K:
				SetResistor(PULLUP_5p1K);
				Serial.println("Pullup resistors set to 5.1K");
				break;
			case PULL_NONE:
				SetResistor(PULLUP_NONE);
				Serial.println("No pullup resistors");
				break;
			case BATT_VOLT:
				Serial.println();
				do {  // Loop on displaying battery voltage
					Serial.printf("\rBattery Voltage: %4dmv ",BattVoltage());
					delay(500);
				} while (!Serial.available() && Serial.dtr());  // Wait for keystroke or disconnect
				Serial.println();
				FlushSerial();
				break;
			case THERM_RES:
				RelayOn();
				Serial.println();
				do {  // Loop on displaying thermistor resistance
					Serial.print("\rThermistor resistance: ");
					therm = ThermResistance();
					if (therm > MAX_THERM)
						Serial.print("No Thermistor");
					else
						Serial.printf( "%6d ohms  ",therm);
					delay(500);
				} while (!Serial.available() && Serial.dtr());  // Wait for keystroke or disconnect
				RelayOff();
				Serial.println();
				FlushSerial();
				break;
			case HELP_MENU:  // Help menu
				DoMenu();
				break;
			default:
				Serial.println("Unknown command");
				break;
			}

NoCmd:
			FlushSerial();
			Serial.println(); // Print prompt
			Serial.print(F("> "));
		}
	}
}

void StartWire(void)
{
	// Setup buffers
	Wire.setRxBuffer(rxBuf,BUFSIZE);
	Wire.setTxBuffer(txBuf,BUFSIZE);

	Wire.begin();  // Start TWI

	// Change I2C clock to ~32KHz
	Wire.setClock(32000);

	// Make timeout longer than GG hold time
	Wire.setTimeout_ms(400);
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
	Serial.println();
	Serial.println("Test Commands:");
	Serial.println("DAT - Do all tests");
	Serial.println();
	Serial.println("   Individual Test Commands:");
	Serial.println("   TA  - Test auth chip");
	Serial.println("   TPR - Test pullup resistors/Active clock drive");
	Serial.println("   TT  - Test thermistor measurement");
	Serial.println("   TPV - Test pullup voltage select/Active clock drive select");
	Serial.println("   TBV - Test battery voltage measurment");
	Serial.println("   TS  - Test status LED");

	Serial.println();
	Serial.println("Debug Commands:");
	Serial.println("SBV - Show battery voltage");
	Serial.println("ST  - Show thermistor resistance");
	Serial.println("RC  - Close relay contacts");
	Serial.println("RO  - Open relay contacts");
	Serial.println("AC  - Set to active clock drive");
	Serial.println("PC  - Set to passive clock drive");
	Serial.println("PBV - Pullup voltage to VBatt");
	Serial.println("P3V - Pullup voltage to 3.3V");
	Serial.println("P10 - Set 10K pullups");
	Serial.println("P6  - Set 6.8K pullups");
	Serial.println("P5  - Set 5.1K pullups");
	Serial.println("PN  - Set to no pullups");

	Serial.println();
	Serial.println("Other Commands:");
	Serial.println("DAC - Dump aurh chip config");
	Serial.println("FD  - Format flash drive");
	Serial.println("UF  - Update the firmware");
	Serial.println("?   - help menu");
}

void Status(void)
{
	PrintVoltage();
	PrintResistor();
	PrintClock();
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

// Routine to wait for a keypress
int WaitForKey(void)
{
	FlushSerial();
	while (!Serial.available() && Serial.dtr())
	;
	if (!Serial.dtr())
		return (ESC);
	else
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
	FatFile file, dir;
	char name[15];

	Serial.println(F("Name        Date                      Size"));
	dir.open("/");  // Open root dir

	while (file.openNext(&dir,O_RDONLY))
	{
		if (!file.isDir()) {
			file.getName(name,15);
			Serial.printf("%13s ",name);
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

uint8_t CheckClk(void)
{
	if (!digitalRead(I2C_CLK))
	{
		Serial.println(F("Error: I2C clock is low!"));
		return (1);
	}

	return (0);
}

// Routine to get a string from the serial port
int GetString(uint8_t *buf, uint8_t maxlen)
{
	char inch;
	int done = 0, pos = 0;

	buf[pos] = 0;  // Terminate string
	FlushSerial();

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
		else if (inch >= ' ' && inch <= '~') // Is it a valid char?
		{  // Yes
			if (pos < maxlen)  // Room to add?
			{
				buf[pos] = inch; // Add to buffer
				++pos;  // Inc pointer
				buf[pos] = 0;  // Terminate string
				Serial.write(inch); // Echo to screen
			}
		}
		else if (ESC == inch)  // Cancel?
		{
			pos = 0;
			done = 1;
		}
	} while(!done);

	Serial.println();  // New line

	return(pos);
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

//--------------------------------------------------------------------+
// fatfs diskio
//--------------------------------------------------------------------+
extern "C"
{

	DSTATUS disk_status ( BYTE pdrv )
	{
	  (void) pdrv;
		return 0;
	}

	DSTATUS disk_initialize ( BYTE pdrv )
	{
	  (void) pdrv;
		return 0;
	}

	DRESULT disk_read (
		BYTE pdrv,		/* Physical drive nmuber to identify the drive */
		BYTE *buff,		/* Data buffer to store read data */
		DWORD sector,	/* Start sector in LBA */
		UINT count		/* Number of sectors to read */
	)
	{
	  (void) pdrv;
		return flash.readBlocks(sector, buff, count) ? RES_OK : RES_ERROR;
	}

	DRESULT disk_write (
		BYTE pdrv,			/* Physical drive nmuber to identify the drive */
		const BYTE *buff,	/* Data to be written */
		DWORD sector,		/* Start sector in LBA */
		UINT count			/* Number of sectors to write */
	)
	{
	  (void) pdrv;
	  return flash.writeBlocks(sector, buff, count) ? RES_OK : RES_ERROR;
	}

	DRESULT disk_ioctl (
		BYTE pdrv,		/* Physical drive nmuber (0..) */
		BYTE cmd,		/* Control code */
		void *buff		/* Buffer to send/receive control data */
	)
	{
	  (void) pdrv;

	  switch ( cmd )
	  {
		case CTRL_SYNC:
		  flash.syncBlocks();
		  return RES_OK;

		case GET_SECTOR_COUNT:
		  *((DWORD*) buff) = flash.size()/512;
		  return RES_OK;

		case GET_SECTOR_SIZE:
		  *((WORD*) buff) = 512;
		  return RES_OK;

		case GET_BLOCK_SIZE:
		  *((DWORD*) buff) = 8;    // erase block size in units of sector size
		  return RES_OK;

		default:
		  return RES_PARERR;
	  }
	}

}

bool FormatDisk(bool TestAllMode)
{
	FRESULT r;

	if (IsFormated)
	{
		if (!TestAllMode)
			Serial.println("Flash chip was already formatted this session!");
		goto exit;
	}

	// Call fatfs begin and passed flash object to initialize file system
	if (TestAllMode)
		Serial.print("\r\nFormatting flash chip: ");
	else
		Serial.println("Creating and formatting FAT filesystem...");

	if (!HaveFlash)
	{
		if (TestAllMode)
			Serial.print("Failed!!\r\n        ");
		Serial.println("No flash chip was found on startup, unable to format!\r\n");
		return (false);
	}

	// Make filesystem.
	r = f_mkfs("", FM_FAT | FM_SFD, 0, workbuf, sizeof(workbuf));
	if (r != FR_OK)
	{
		if (TestAllMode)
			Serial.print("Failed!!\r\n        ");
		Serial.print("Error, f_mkfs failed with error code: ");
		Serial.println(r, DEC);
		return (false);
	}

	// mount to set disk label
	r = f_mount(&elmchamFatfs, "0:", 1);
	if (r != FR_OK)
	{
		if (TestAllMode)
			Serial.print("Failed!!\r\n        ");
		Serial.print("Error, f_mount failed with error code: ");
		Serial.println(r, DEC);
		return (false);
	}

	// Setting label
	if (!TestAllMode)
		Serial.println("Setting disk label to: " DISK_LABEL);
	r = f_setlabel(DISK_LABEL);
	if (r != FR_OK)
	{
		if (TestAllMode)
			Serial.print("Failed!!\r\n        ");
		Serial.print("Error, f_setlabel failed with error code: ");
		Serial.println(r, DEC);
		return (false);
	}

	// unmount
	f_unmount("0:");

	// sync to make sure all data is written to flash
	flash.syncBlocks();

	if (!TestAllMode)
		Serial.println("Formatted flash, checking filesystem...");

	// Check new filesystem
	if (!fatfs.begin(&flash))
	{
		if (TestAllMode)
			Serial.print("Failed!!\r\n        ");
		Serial.println("Error, failed to mount newly formatted filesystem!");
		return (false);
	}

	IsFormated = true;

exit:
	// Done!
	if (TestAllMode)
		Serial.println(Passed);

	return (true);
}
