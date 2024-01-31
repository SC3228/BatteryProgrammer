// Battery programmer/reader


/*
2.0		Initial release with auth chip setup
*/

// Includes
#include <SdFat.h>	// SD card stuff
#include <Wire.h>	// I2C lib
#include "BattProg.h"

// TWI defs
#define	I2C_PRESCALE	0x01	// /4 prescaler
#define	I2C_RATE		60		// Setting for 31.25K SCL rate with /4 prescaler

SdFat SD;	// SD card file system object
uint8_t HaveSD = 0;  // Have an SD card flag
extern uint8_t GotSerial;

void setup()
{
	// Setup GPIOs
	InitPullupPins();
	digitalWrite(WP,LOW);  // Set WP to input, no pull up
	pinMode(WP,INPUT);
	digitalWrite(CD,LOW);  // Set CD to input, no pull up
	pinMode(CD,INPUT);

	// Clear got serial number flag
	GotSerial = 0;

	// Setup serial port
//	Serial.begin(115200);
	Serial.begin(9600);
	Serial.println("");
	Serial.println(F("EEPROM Programmer Test Version 2.0"));
	Serial.println("");

	// Setup I2C
	SetVoltage(PULLUP_V0);  // Default to 0V pullup voltage
	SetResistor(PULLUP_10K); // Default to 10K pullup resistors
	PassiveClk();  // Default to passive clock drive
	Wire.begin();

	// Remove SCL/SDA internal pullups
	digitalWrite(SCL,LOW);
	digitalWrite(SDA,LOW);

	// Change I2C clock to ~32KHz
	Wire.setClock(32250);


	CheckSD();  // Check for change in SD card status

	Serial.println("");
	Serial.print(F("> ")); // Print prompt
}

void loop()
{
	uint8_t inch;
	char msg[2];

	CheckSD();  // Check for change in SD card status

	if (Serial.available())
	{
		inch = Serial.read();
		msg[0] = inch;
		msg[1] = 0;
		Serial.println(msg);

		// Do command
		switch (inch)
		{
		case '?':  // Help menu
			Serial.println(F("Commands:"));
			Serial.println(F("0->7 Set pullup resistors"));
			Serial.println(F("B - Select battery voltage"));
			Serial.println(F("F - Select 5V"));
			Serial.println(F("T - Select 3.3V"));
			Serial.println(F("Z - Select 0V"));
			Serial.println(F("A - Active clock drive"));
			Serial.println(F("P - Passive clock drive"));
			Serial.println(F("N - No clock drive"));
			Serial.println(F("C - SD card directory"));
			Serial.println(F("V - Read battery voltage"));
			Serial.println(F("G - Setup auth chip"));
			Serial.println(F("S - Enter Serial number and rev"));
			Serial.println(F("E - Check Auth chip"));
			Serial.println(F("D - Dump Auth chip config and SN"));
			break;
		case CR:  // Enter key
			break;
		case 's':  // Enter serial number string
		case 'S':
			SetVoltage(PULLUP_V5);  // 5V pullup voltage
			SetResistor(PULLUP_5p1K); // 5.1K pullup resistors
			PassiveClk();  // Passive clock drive
			AUTH_EnterSerial();
			break;
		case 'g':  // Setup auth chip
		case 'G':
			SetVoltage(PULLUP_V5);  // 5V pullup voltage
			SetResistor(PULLUP_5p1K); // 5.1K pullup resistors
			PassiveClk();  // Passive clock drive
			AUTH_Config();
			break;
		case 'd':  // Dump auth chip config
		case 'D':
			SetVoltage(PULLUP_V5);  // 5V pullup voltage
			SetResistor(PULLUP_5p1K); // 5.1K pullup resistors
			PassiveClk();  // Passive clock drive
			AUTH_Dump();
			break;
		case 'e':  // Check Auth chip
		case 'E':
			SetVoltage(PULLUP_V5);  // 5V pullup voltage
			SetResistor(PULLUP_5p1K); // 5.1K pullup resistors
			PassiveClk();  // Passive clock drive
			AUTH_Read();
			break;
		case 'c': // SD card directory
		case 'C':
			ListDir();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			SetResistor(inch-'0');
			break;
		case 'b':
		case 'B':
			SetVoltage(PULLUP_VBATT);
			break;
		case 'f':
		case 'F':
			SetVoltage(PULLUP_V5);
			break;
		case 't':
		case 'T':
			SetVoltage(PULLUP_V3p3);
			break;
		case 'z':
		case 'Z':
			SetVoltage(PULLUP_V0);
			break;
		case 'a':
		case 'A':
			ActiveClk();
			break;
		case 'p':
		case 'P':
			PassiveClk();
			break;
		case 'n':
		case 'N':
			NoClk();
			break;
		case 'v':
		case 'V':
			Serial.print(F("Battery voltage: "));
			Serial.print(BattVoltage());
			Serial.println("V");
			break;
		case 'i':
		case 'I':
			Serial.print(Wire.requestFrom((uint8_t)0x52, (uint8_t)16, (uint8_t)true));
			Serial.println(F(" Bytes read."));
			break;
		default:
			Serial.println(F("Unknown command"));
			break;
		}

		Serial.println(""); // Print prompt
		Serial.print(F("> "));
	}
}

// Function to read the battery voltage
// Returns voltage in mv
float BattVoltage(void)
{
	float voltage;
	int AtD;

	AtD = analogRead(BATT_PLUS);
	voltage = AtD * (5.0 / 1023.0);
	return (voltage);
}

// Function to update SD card status
void CheckSD(void)
{
	if (HaveSD) // Do we have a card?
	{  // Check for removal
		if (digitalRead(CD))
		{
			Serial.println(F("SD removed!"));  
			HaveSD = 0;  // No card now
		}
	}
	else
	{  // Check for card insertion
		if (!digitalRead(CD))
		{
			if (!SD.begin(SD_CS, SPI_HALF_SPEED))  // see if the card is present and can be initialized
			{
				Serial.println(F("Bad SD card!"));
			}
			else
			{
				if (!digitalRead(WP))
					Serial.println(F("Found SD card!"));
				else
					Serial.println(F("Found write protected SD card!"));
				HaveSD = 1;  // Have card now			
			}
		}
	}
}

// Function to print message for no SD card
void NoCard(void)
{
	Serial.println(F("Please insert an SD card first!"));
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
	} while((millis() - StartTime) < 1500); // Make sure no data for a while
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

// Routine to get a string from the serial port
int GetString(uint8_t *buf, uint8_t maxlen)
{
	char inch;
	int retval, done = 0, pos = 0;

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

