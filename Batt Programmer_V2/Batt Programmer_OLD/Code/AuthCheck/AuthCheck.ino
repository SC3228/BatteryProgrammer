// Auth Chip Checker

// Includes
#include <Wire.h>	// I2C lib

#include "Auth.h"

void setup()
{
	uint8_t cnt = 0;

	// Setup serial port
//	Serial.begin(115200);
	Serial.begin(9600);

	// Setup I2C
	StartWire(); // restart Wire stuff
}

// Check for valid auth chip after keypress
void loop()
{
	Serial.println(F("Hit any key to test auth chip..."));
	
	WaitForKey();
	Serial.println();  // New line

	StartWire();
	if (!CheckAuthChip())
	{
		Serial.println();
		Serial.println(F("Auth chip is OK!"));
	}
}

// Function to start the I2C interface
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
