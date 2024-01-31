// Battery programmer EEPROM functions

#include <Arduino.h>
#include <Wire.h>	// I2C lib

#include "pwmbatt.h"
#include "BattProg.h"
#include "EEPROM.h"

// Function to update EEPROM
void EEPROM_Update(uint8_t type)
{
	uint8_t LocalBuf[MAX_EEPROM_SIZE];

	if (!BatteryType)  // Check for valid battery type
		return;

	// Get data from the battery
	if (EEPROM_Read(LocalBuf))
		return (1);

	switch (type)
	{
	case GIFTED_BATT_DATA:
		DoGiftedUpdate(0,LocalBuf);
		break;
	case COMBINED_BATT_DATA:
		if (DoPolluxUpdate(LocalBuf))
			return;
		DoGiftedUpdate(512,LocalBuf);
		break;
	case POLLUX_BATT_DATA:
		DoPolluxUpdate(LocalBuf);
		break;
	default:
		Serial.println(F("Unsupported data type!"));
		return;
		break;
	}

	// Write data back to battery
	EEPROM_Write(LocalBuf);
}

// Function to write EEPROM
void EEPROM_Write(uint8_t *buffer = NULL)
{
	int BytesWrote; // Bytes wrote to I2C
	uint8_t inch;
	uint16_t cnt, cnt2;
	uint8_t csum;  // Record checksum
	uint8_t *data;

	if (NULL == buffer)  // Check for which buffer
		data = EEPROM_Data;
	else
		data = buffer;

	if (!BatteryType)  // Check for valid battery type
		return;

	FlushSerial();
	Serial.println(F("WARNING: Overwriting all the EEPROM data can un-manufacture"));
	Serial.println(F("           a battery making it non-operational!!!!!"));
	Serial.println();
	Serial.print(F("Overwrite "));
	Serial.print((__FlashStringHelper*)Batts[BatteryType].Name);
	Serial.println(F(" EEPROM (y/n)?"));
	while(!Serial.available())
	;
	inch = Serial.read();
	if (inch != 'y' && inch != 'Y')
		return;

	Serial.println(F("Starting write..."));

	// loop thru write page size bytes at a time
	for (cnt = 0; cnt < Batts[BatteryType].Size; cnt += Batts[BatteryType].Page)
	{
		if (SwitchClk) // Disable clock drive if needed
			PassiveClk();

		if (CheckClk()) // Check for clock not "stuck"
			return;

		Wire.beginTransmission((uint8_t)(Batts[BatteryType].EEPROM_Address|(cnt >> 8))); // transmit to EEPROM
		if ((inch = Wire.write(cnt&0xff)) != 1) // Send address byte
		{
			Serial.println(F("Error writing address"));
			Wire.endTransmission();    // Stop transmitting
			goto WriteError;
		}

		// Write page size bytes
		BytesWrote = Wire.write(data+cnt,Batts[BatteryType].Page);
		Wire.endTransmission();    // Stop transmitting

		if (SwitchClk) // Disable clock drive if needed
			PassiveClk();

		if (CheckClk()) // Check for clock not "stuck"
			return;

		Wire.endTransmission();    // stop transmitting

		if (SwitchClk) // Enable clock drive if needed
		{
			ActiveClk();
			delay(15);
		}
		else
			delay(1);

		// Check for count of bytes written
		if (BytesWrote != Batts[BatteryType].Page)
		{
			Serial.println(F("Error writing data"));
			goto WriteError;
		}

		// Check for user hitting ESC
		if (Serial.available() && ESC == Serial.read())
		{
			Serial.println();
			Serial.println(F("Interrupted by user"));
			return;
		}
	}
	Serial.println();
	Serial.println(F("Write Done OK!"));

	EEPROM_Verify(buffer);  // Verify data

WriteError:
	if (SwitchClk)  // Just in case we had an error
		ActiveClk();
}

// Function to verify EEPROM data
// Returns 0 if OK, 1 on error
void EEPROM_Verify(uint8_t *buffer = NULL)
{
	int BytesRead; // Bytes read from I2C
	uint8_t inch;
	uint16_t cnt, cnt2;
	uint8_t csum;  // Record checksum
	uint8_t *data;

	if (NULL == buffer)  // Check for which buffer
		data = EEPROM_Data;
	else
		data = buffer;

	if (!BatteryType)  // Check for valid battery type
		return;

	Serial.print(F("Starting "));
	Serial.print((__FlashStringHelper*)Batts[BatteryType].Name);
	Serial.println(F(" EEPROM Verify..."));

	// loop thru 16 bytes at a time
	for (cnt = 0; cnt < Batts[BatteryType].Size; cnt += 16)
	{
		if (SwitchClk) // Disable clock drive if needed
			PassiveClk();

		if (CheckClk()) // Check for clock not "stuck"
			return;

		Wire.beginTransmission((uint8_t)(Batts[BatteryType].EEPROM_Address|(cnt >> 8))); // transmit to EEPROM
		if (Wire.write(cnt&0xff) != 1) // Send address byte
		{
			Serial.println(F("Error writing address"));
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		// Try I2C read of 16 bytes
		BytesRead = Wire.requestFrom((uint8_t)(Batts[BatteryType].EEPROM_Address|(cnt >> 8)), (uint8_t)16, DO_STOP);

		if (SwitchClk) // Enable clock drive if needed
		{
			ActiveClk();
			delay(15);
		}
		else
			delay(1);

		if (16 != BytesRead)
		{
			Serial.println(F("Bad read!"));
			while(Wire.available())  // flush buffer
				inch = Wire.read();
			goto ReadError;
		}

		cnt2 = 0; // Reset index into record
		while(Wire.available())  // Loop thru bytes
		{
			inch = Wire.read(); // Get next byte
			csum += inch;
			if (data[cnt+cnt2] != inch) // Check for match
			{
				Serial.println();
				Serial.print(F("ERROR at "));
				printfSerial("%04X",cnt+cnt2);
				Serial.print(F(" Buffer: "));
				printfSerial("%02X",data[cnt+cnt2]);
				Serial.print(F(" EEPROM: "));
				printfSerial("%02X\n",inch);
				return;
			}
			++cnt2;
		}

		// Check for user hitting ESC
		if (Serial.available() && ESC == Serial.read())
		{
			Serial.println();
			Serial.println(F("Interrupted by user"));
			goto ReadError;
		}
	}
	Serial.println();
	Serial.println(F("Verify OK!"));

ReadError:
	if (SwitchClk)  // Just in case we had an error
		ActiveClk();
}

// Function to read EEPROM into buffer
uint8_t EEPROM_Read(uint8_t *buffer = NULL)
{
	int BytesRead; // Bytes read from I2C
	uint8_t inch;
	uint16_t cnt, cnt2;
	uint8_t *data, ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(1);

	FlushSerial();
	if (NULL == buffer)  // Check for which buffer
	{
		data = EEPROM_Data;
		Serial.print(F("Read "));
		Serial.print((__FlashStringHelper*)Batts[BatteryType].Name);
		Serial.println(F(" EEPROM into buffer (y/n)?"));
		while(!Serial.available())
		;
		inch = Serial.read();
		if (inch != 'y' && inch != 'Y')
			return(ret);
	}
	else
		data = buffer;

	Serial.println(F("Starting read..."));

	// loop thru 16 bytes at a time
	for (cnt = 0; cnt < Batts[BatteryType].Size; cnt += 16)
	{
		if (SwitchClk) // Disable clock drive if needed
			PassiveClk();

		if (CheckClk()) // Check for clock not "stuck"
			return(ret);

		Wire.beginTransmission((uint8_t)(Batts[BatteryType].EEPROM_Address|(cnt >> 8))); // transmit to EEPROM
		if (Wire.write(cnt&0xff) != 1) // Send address byte
		{
			Serial.println(F("Error writing address"));
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		// Try I2C read of 16 bytes
		BytesRead = Wire.requestFrom((uint8_t)(Batts[BatteryType].EEPROM_Address|(cnt >> 8)), (uint8_t)16, DO_STOP);

		if (SwitchClk) // Enable clock drive if needed
		{
			ActiveClk();
			delay(15);
		}
		else
			delay(1);

		if (16 != BytesRead)
		{
			Serial.println();
			Serial.println(F("Bad read!"));
			while(Wire.available())  // flush buffer
				inch = Wire.read();
			goto ReadError;
		}

		cnt2 = 0; // Reset index into record
		while(Wire.available())  // Loop thru bytes
		{
			inch = Wire.read(); // Get next byte
			data[cnt+cnt2] = inch;
			++cnt2;
			BufferBlank = 0;
		}

		// Check for user hitting ESC
		if (Serial.available() && ESC == Serial.read())
		{
			Serial.println();
			Serial.println(F("Interrupted by user"));
			goto ReadError;
		}
	}
	Serial.println();
	Serial.println(F("Read OK!"));
	ret = 0;

ReadError:
	if (SwitchClk)  // Just in case we had an error
		ActiveClk();
	return(ret);
}

