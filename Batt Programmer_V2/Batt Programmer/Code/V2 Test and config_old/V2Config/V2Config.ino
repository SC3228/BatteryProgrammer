// Battery programmer/reader V2 set serial numer and HW rev values program

#define VERSION "1.0"

// Includes
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include "Adafruit_SPIFlash.h"
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
#define MAX_THERM	500000  // Max alowable thermistor resistance value

// Pin defines
#define BATT_PLUS	A0
#define THERM_PIN	A1
#define LED_PIN 13

// Set to true when connected to serial
bool SerialConnect;

// List of commands
const char *Cmds[] = { "?"
						,"UF"
						,"ES"
						,"EH" };

enum Command { HELP_MENU
				, UPDATE_FIRMWARE
				, ENTER_SERIAL, ENTER_HWREV
				, NO_CMD };

// Auth serial number string
char AuthSN[32] = "";

void setup()
{
	// Start up the status NeoPixel
	statusLED.begin();
	delay(50);
	STATUS_LED_WAITING;

	// Clear got serial number flag
	GotSerial = 0;

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

	Serial.printf("\r\nBattProg2 Config Ver: %s\r\n\r\n> ",VERSION); // Print version and prompt
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
			case UPDATE_FIRMWARE:
				Serial.println("Restarting to load firmware...");
				delay(1000);
				*(uint32_t *)(0x20000000 + 196608 -4) = 0xf01669ef;     // Store special flag value in last word in RAM.
				NVIC_SystemReset();
				break;
			case ENTER_HWREV:  // Enter hardware revision
				EnterHWrev();
				break;
			case ENTER_SERIAL:  // Enter serial number
				EnterSerial();
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

	// Remove SCL/SDA internal pullups
// ***FIX***	digitalWrite(SCL,LOW);
//	***FIX*** digitalWrite(SDA,LOW);

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
	Serial.println("Configuration Commands:");
	Serial.println("ES - Enter device serial number");
	Serial.println("EH - Enter device hardware revision");

	Serial.println();
	Serial.println("Other Commands:");
	Serial.println("UF - Update the firmware");
	Serial.println("?  - help menu");
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

void EnterSerial(void)
{
	uint8_t LockValue, LockConfig;
	int16_t sn, sn2;

	// Is config locked?
	Serial.print("Checking for locked config: ");
	if (!AUTH_GetLockBytes(&LockValue,&LockConfig)) // Get lock bytes
		return;

	if (0x55 == LockValue || 0x55 == LockConfig)
	{  // Not locked, do config
		Serial.println("Unlocked\r\n        Unable to put serial number into unlocked chip!");
		return;
	}
	else
		Serial.println("Locked");

	do
	{
		// Get serial number
		Serial.print("\r\nEnter serial number, must be 100-999 for Zebra internal units\r\n  and 1000-1099 for JDM units: ");
		if (-1 ==(sn = GetInt()))
			return;
		if (sn < 100 || sn > 1099)
		{
			Serial.printf("\r\nYou entered %d which is out of range!\r\n",sn);
			sn = -1;
		}
		else
		{
			// Re-enter serial number
			Serial.print("\r\nRe-enter serial number to verify: ");
			if (-1 ==(sn2 = GetInt()))
				return;
			if (sn2 != sn)
			{
				Serial.printf("\r\nYou entered %d which does match %d!\r\n",sn2,sn);
				sn = -1;
			}
			else
			{
				// Double check we are good to go
				Serial.printf("\r\nAbout to lock slot 0 with SN: %d\r\n",sn);
				Serial.print("\r\nThis operation cannot be undone, are you sure (Y/N)? ");
				if (!GetYesNo())
					return;
				else
					Serial.println();
			}
		}
	} while (-1 == sn);

	if (!AUTH_WriteSerial(0,sn))
		return;

	Serial.print("Reading back serial number: ");
	if (!(sn2 = AUTH_ReadSerial(0)))
		return;

	// Check for match
	if (sn2 != sn)
	{
		Serial.printf("Missmatch!! SN: %d, read back: %d\r\n\r\n",sn,sn2);
		return;
	}
	else
		Serial.print("Data matched\r\n");

	if (!AUTH_LockSlot(0))
		return;

	Serial.print("Reading back serial number: ");
	if (!(sn2 = AUTH_ReadSerial(0)))
		return;

	// Check for match
	if (sn2 != sn)
	{
		Serial.printf("Missmatch!! SN: %d, read back: %d\r\n\r\n",sn,sn2);
		return;
	}
	else
	{
		Serial.print("Data matched\r\n");
		Serial.print("\r\nSN write & slot lock success!\r\n\r\n");
	}
}

void EnterHWrev(void)
{
	int16_t major,minor, tmp;
	uint8_t LockValue, LockConfig;

	// Is config locked?
	Serial.print("Checking for locked config: ");
	if (!AUTH_GetLockBytes(&LockValue,&LockConfig)) // Get lock bytes
		return;

	if (0x55 == LockValue || 0x55 == LockConfig)
	{  // Not locked, do config
		Serial.println("Unlocked\r\n        Unable to put HW rev into unlocked chip!");
		return;
	}
	else
		Serial.println("Locked");

	do
	{
		// Get HW rev
		Serial.print("\r\nEnter HW rev major number: ");
		if (-1 ==(major = GetInt()))
			return;
		if (major < 1 || major > 255)
		{
			Serial.printf("\r\nYou entered %d which is out of range!\r\n",major);
			major = -1;
		}
		else
		{
			// Enter minor number
			Serial.print("\r\nEnter HW minor rev number: ");
			if (-1 ==(minor = GetInt()))
				return;
			if (minor < 0 || minor > 255)
			{
				Serial.printf("\r\nYou entered %d which is out of range!\r\n",minor);
				minor = -1;
			}
			else
			{
				// Double check we are good to go
				Serial.printf("\r\nAbout to write HW rev: %d.%d\r\n",major,minor);
				Serial.print("\r\nAre you sure (Y/N)? ");
				if (!GetYesNo())
					return;
				else
					Serial.println();
			}
		}
	} while (-1 == major || -1 == minor);

	if (!AUTH_WriteSerial(1,(major<<8)|minor))
		return;

	Serial.print("Reading back HW rev: ");
	if (!(tmp = AUTH_ReadSerial(1)))
		return;

	// Check for match
	if (tmp != ((major<<8)|minor))
	{
		Serial.printf("Missmatch!! HW rev: %d.%d, read back: %d.%d\r\n\r\n",major,minor,tmp>>8,tmp&0xff);
		return;
	}
	else
		Serial.print("Data matched\r\n");
}

// Funtion waits for a 'Y' or 'N' to be entered.
// Returns 'true' on 'Y'
bool GetYesNo(void)
{
	int inch;

	while (1)  // Loop till we get a Y or N
	{
		FlushSerial();
		while(!Serial.available())
		;
		inch = Serial.read();
		if (inch != 'y' && inch != 'Y' && inch != 'n' && inch != 'N')
			Serial.print("\r\n\r\nPlease enter Y or N only: ");
		else // See which we got
		{
			FlushSerial();
			if (inch == 'y' || inch == 'Y')
				return (true);
			else
				return (false);
		}
	}
}
