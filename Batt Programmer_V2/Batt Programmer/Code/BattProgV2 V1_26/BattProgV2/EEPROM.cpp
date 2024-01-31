// Battery programmer EEPROM/Temp chip functions

#include <Arduino.h>
#include "Adafruit_SPIFlash.h"
#include <SoftWire.h>	// I2C lib
extern SoftWire Wire;

#include "pwmbatt.h"
#include "Batts.h"
#include "BattProg.h"
#include "Manufacture.h"
#include "EEPROM.h"

// Function to read an MPA2/3 temp chip
int8_t Temp_Read(void)
{
	int8_t ret = -42; // Set to error temp
	uint8_t inch;
	int BytesRead;

	if (!BatteryType)  // Check for valid battery type
		return (ret);

	if (!Batts[BatteryType].TempAddress)  // Check for a temp chip address
		return (ret);

	// Check for chip in shutdown mode
	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
	{
		STATUS_LED_READY;
		return (ret);
	}

	Wire.beginTransmission((uint8_t)Batts[BatteryType].TempAddress); // Set pointer to config register
	if (Wire.write(1) != 1) // Send address
	{
		Serial.println("Error writing config address");
		Wire.endTransmission();    // Stop transmitting
		return (ret);
	}
	Wire.endTransmission();    // stop transmitting

	// Try I2C read of 1 byte
	BytesRead = Wire.requestFrom((uint8_t)Batts[BatteryType].TempAddress, (uint8_t)1, DO_STOP);

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	if (1 != BytesRead)
	{
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			inch = Wire.read();
		return (ret);
	}

	// Check for chip in shutdown mode
	inch = Wire.read();
	if (!(inch & 1))
		Serial.println("ERROR: Temp chip was not in shutdown mode!");

	// Enable temp reads
	Wire.beginTransmission((uint8_t)Batts[BatteryType].TempAddress); // Set pointer to config register
	if (Wire.write(1) != 1 || Wire.write(0) != 1) // Send address and config
	{
		Serial.println("Error setting config");
		Wire.endTransmission();    // Stop transmitting
		return (ret);
	}
	Wire.endTransmission();    // stop transmitting

	delay(500); // Wait a bit for temp reads

	// Get temp
	Wire.beginTransmission((uint8_t)Batts[BatteryType].TempAddress); // Set pointer to temp reg
	if (Wire.write(0) != 1) // Send address
	{
		Serial.println("Error writing config address");
		Wire.endTransmission();    // Stop transmitting
		return (ret);
	}
	Wire.endTransmission();    // stop transmitting

	// Try I2C read of 1 byte
	BytesRead = Wire.requestFrom((uint8_t)Batts[BatteryType].TempAddress, (uint8_t)1, DO_STOP);

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	if (1 != BytesRead)
	{
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			inch = Wire.read();
		goto ErrorExit;
	}

	// Get temp
	ret = Wire.read();

ErrorExit:
	// Put chip back into shutdown mode
	Wire.beginTransmission((uint8_t)Batts[BatteryType].TempAddress); // Set pointer to config register
	if (Wire.write(1) != 1 || Wire.write(1) != 1) // Send address and config
	{
		Serial.println("Error setting shutdown");
		Wire.endTransmission();    // Stop transmitting
		return (ret);
	}
	Wire.endTransmission();    // stop transmitting

	// Return temp
	return (ret);
}

// Function to validate data in the EEPROM
uint8_t Validate_EEPROM(uint8_t type)
{
	uint8_t ret;

	if (!BatteryType)  // Check for valid battery type
		return (1);

	switch (type)
	{
	case GIFTED_BATT_DATA:
		ret = ValidatePPP(EEPROM_Data);
		break;
	case COMBINED_BATT_DATA:
		if ((ret = ValidatePollux(EEPROM_Data,POLLUX8956_BATT_DATA)))
			ValidatePPP(EEPROM_Data+512);
		else
			ret = ValidatePPP(EEPROM_Data+512);
		break;
	case POLLUX_BATT_DATA:
		ret = ValidatePollux(EEPROM_Data,POLLUX_BATT_DATA);
		break;
	case SMART_BATT_DATA:
		ret = ValidatePP(EEPROM_Data);
		break;
	case COMET_BATT_DATA:
		ret = ValidateVTeeprom(EEPROM_Data);
		break;
	default:
		Serial.println(Unsupported);
		ret = 1;
		break;
	}

	return (ret);
}

// Function to update EEPROM
void EEPROM_Update(uint8_t type)
{
	uint8_t LocalBuf[MAX_EEPROM_SIZE];

	if (!BatteryType)  // Check for valid battery type
		return;

	// Get data from the battery
	if (EEPROM_Read(LocalBuf))
		return;

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
		Serial.println(Unsupported);
		return;
		break;
	}

	// Write data back to battery
	EEPROM_Write(LocalBuf);
}

// Function to validate data in the EEPROM
uint8_t Manufacture_EEPROM(uint8_t type)
{
	uint8_t ret;

	if (!BatteryType)  // Check for valid battery type
		return (1);

	switch (type)
	{
	case GIFTED_BATT_DATA:
		ret = ManufacturePPP(EEPROM_Data);
		break;
	case COMBINED_BATT_DATA:
		if ((ret = ManufacturePollux(EEPROM_Data,POLLUX8956_BATT_DATA)))
			break;
		ret = ManufacturePPP(EEPROM_Data+512);
		break;
	case POLLUX_BATT_DATA:
		ret = ManufacturePollux(EEPROM_Data,POLLUX_BATT_DATA);
		break;
	default:
		Serial.println(Unsupported);
		ret = 1;
		break;
	}

	return (ret);
}

// Function to write EEPROM
void EEPROM_Write(uint8_t *buffer)
{
	int BytesWrote; // Bytes wrote to I2C
	uint8_t inch;
	uint16_t cnt;
	uint8_t *data;

	if (NULL == buffer)  // Check for which buffer
		data = EEPROM_Data;
	else
		data = buffer;

	if (!BatteryType)  // Check for valid battery type
		return;

	FlushSerial();
	Serial.println("WARNING: Overwriting all the EEPROM data can un-manufacture");
	Serial.println("           a battery making it non-operational!!!!!");
	Serial.println();
	Serial.print("Overwrite ");
	Serial.print(Batts[BatteryType].Name);
	Serial.println(" EEPROM (y/n)?");
	while(!Serial.available())
	;
	inch = Serial.read();
	if (inch != 'y' && inch != 'Y')
		return;

	STATUS_LED_BUSY;
	Serial.println("Starting write...");

	if (BatteryType == COMET_BATT)  // Check for 16K EEPROM
	{
		Wire.beginTransmission((uint8_t)(Batts[BatteryType].EEPROM_Address)); // transmit to EEPROM
		if (Wire.write(0) != 1 || Wire.write(0) != 1) // Send address bytes for 0
		{
			Serial.println("Error writing address");
			Wire.endTransmission();    // Stop transmitting
			goto WriteError;
		}

		// loop thru write page size bytes at a time
		for (cnt = 0; cnt < Batts[BatteryType].Size; cnt += Batts[BatteryType].Page)
		{
			if (CheckClk()) // Check for clock not "stuck"
			{
				STATUS_LED_READY;
				return;
			}

			// Write page size bytes
			BytesWrote = Wire.write(data+cnt,Batts[BatteryType].Page);
			Wire.endTransmission();    // Stop transmitting

			if (CheckClk()) // Check for clock not "stuck"
			{
				STATUS_LED_READY;
				return;
			}

			Wire.endTransmission();    // stop transmitting

			delay(1);

			// Check for count of bytes written
			if (BytesWrote != Batts[BatteryType].Page)
			{
				Serial.println("Error writing data");
				goto WriteError;
			}

			// Check for user hitting ESC
			if (Serial.available() && ESC == Serial.read())
			{
				Serial.println();
				Serial.println("Interrupted by user");
				STATUS_LED_READY;
				return;
			}
		}
	}
	else
	{
		// loop thru write page size bytes at a time
		for (cnt = 0; cnt < Batts[BatteryType].Size; cnt += Batts[BatteryType].Page)
		{
			if (SwitchClk) // Disable clock drive if needed
				PassiveClk();

			if (CheckClk()) // Check for clock not "stuck"
			{
				STATUS_LED_READY;
				return;
			}

			Wire.beginTransmission((uint8_t)(Batts[BatteryType].EEPROM_Address|(cnt >> 8))); // transmit to EEPROM
			if ((inch = Wire.write(cnt&0xff)) != 1) // Send address byte
			{
				Serial.println("Error writing address");
				Wire.endTransmission();    // Stop transmitting
				goto WriteError;
			}

			// Write page size bytes
			BytesWrote = Wire.write(data+cnt,Batts[BatteryType].Page);
			Wire.endTransmission();    // Stop transmitting

			if (SwitchClk) // Disable clock drive if needed
				PassiveClk();

			if (CheckClk()) // Check for clock not "stuck"
			{
				STATUS_LED_READY;
				return;
			}

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
				Serial.println("Error writing data");
				goto WriteError;
			}

			// Check for user hitting ESC
			if (Serial.available() && ESC == Serial.read())
			{
				Serial.println();
				Serial.println("Interrupted by user");
				STATUS_LED_READY;
				return;
			}
		}
	}
	Serial.println();
	Serial.println("Write Done ok!");

	EEPROM_Verify(buffer);  // Verify data

WriteError:
	STATUS_LED_READY;
	if (SwitchClk)  // Just in case we had an error
		ActiveClk();
}

// Function to verify EEPROM data
void EEPROM_Verify(uint8_t *buffer)
{
	int BytesRead; // Bytes read from I2C
	uint8_t inch;
	uint16_t cnt, cnt2;
	uint8_t *data;

	if (NULL == buffer)  // Check for which buffer
		data = EEPROM_Data;
	else
		data = buffer;

	if (!BatteryType)  // Check for valid battery type
		return;

	STATUS_LED_BUSY;
	Serial.print("Starting ");
	Serial.print(Batts[BatteryType].Name);
	Serial.println(" EEPROM Verify...");

	if (BatteryType == COMET_BATT)  // Check for 16K EEPROM
	{
		Wire.beginTransmission((uint8_t)(Batts[BatteryType].EEPROM_Address)); // transmit to EEPROM
		if (Wire.write(0) != 1 || Wire.write(0) != 1) // Send address bytes for 0
		{
			Serial.println("Error writing address");
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}

		Wire.endTransmission();    // stop transmitting

		// loop thru 32 bytes at a time
		for (cnt = 0; cnt < Batts[BatteryType].Size; cnt += 32)
		{
			if (CheckClk()) // Check for clock not "stuck"
			{
				STATUS_LED_READY;
				return;
			}

			// Try I2C read of 32 bytes
			BytesRead = Wire.requestFrom((uint8_t)(Batts[BatteryType].EEPROM_Address), (uint8_t)32, DO_STOP);

			if (32 != BytesRead)
			{
				Serial.println(BadRead);
				while(Wire.available())  // flush buffer
					inch = Wire.read();
				goto ReadError;
			}

			cnt2 = 0; // Reset index into record
			while(Wire.available())  // Loop thru bytes
			{
				inch = Wire.read(); // Get next byte
				if (data[cnt+cnt2] != inch) // Check for match
				{
					Serial.println();
					Serial.print("ERROR at ");
					Serial.printf("%04X",cnt+cnt2);
					Serial.print(" Buffer: ");
					Serial.printf("%02X",data[cnt+cnt2]);
					Serial.print(" EEPROM: ");
					Serial.printf("%02X\n",inch);
					STATUS_LED_READY;
					return;
				}
				++cnt2;
			}

			// Check for user hitting ESC
			if (Serial.available() && ESC == Serial.read())
			{
				Serial.println();
				Serial.println("Interrupted by user");
				goto ReadError;
			}
		}
	}
	else
	{
		// loop thru 16 bytes at a time
		for (cnt = 0; cnt < Batts[BatteryType].Size; cnt += 16)
		{
			if (SwitchClk) // Disable clock drive if needed
				PassiveClk();

			if (CheckClk()) // Check for clock not "stuck"
			{
				STATUS_LED_READY;
				return;
			}

			Wire.beginTransmission((uint8_t)(Batts[BatteryType].EEPROM_Address|(cnt >> 8))); // transmit to EEPROM
			if ((inch = Wire.write(cnt&0xff)) != 1) // Send address byte
			{
				Serial.println("Error writing address");
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
				Serial.println(BadRead);
				while(Wire.available())  // flush buffer
					inch = Wire.read();
				goto ReadError;
			}

			cnt2 = 0; // Reset index into record
			while(Wire.available())  // Loop thru bytes
			{
				inch = Wire.read(); // Get next byte
				if (data[cnt+cnt2] != inch) // Check for match
				{
					Serial.println();
					Serial.print("ERROR at ");
					Serial.printf("%04X",cnt+cnt2);
					Serial.print(" Buffer: ");
					Serial.printf("%02X",data[cnt+cnt2]);
					Serial.print(" EEPROM: ");
					Serial.printf("%02X\n",inch);
					STATUS_LED_READY;
					return;
				}
				++cnt2;
			}

			// Check for user hitting ESC
			if (Serial.available() && ESC == Serial.read())
			{
				Serial.println();
				Serial.println("Interrupted by user");
				goto ReadError;
			}
		}
	}
	Serial.println();
	Serial.println("Verify ok!");

ReadError:
	STATUS_LED_READY;
	if (SwitchClk)  // Just in case we had an error
		ActiveClk();
}

// Function to read EEPROM into buffer
uint8_t EEPROM_Read(uint8_t *buffer)
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
		Serial.print("Read ");
		Serial.print(Batts[BatteryType].Name);
		Serial.println(" EEPROM into buffer (y/n)?");
		while(!Serial.available())
		;
		inch = Serial.read();
		if (inch != 'y' && inch != 'Y')
			return(ret);
	}
	else
		data = buffer;

	STATUS_LED_BUSY;
	Serial.println("Starting read...");

	if (BatteryType == COMET_BATT)  // Check for 16K EEPROM
	{
		Wire.beginTransmission((uint8_t)(Batts[BatteryType].EEPROM_Address)); // transmit to EEPROM
		if (Wire.write(0) != 1 || Wire.write(0) != 1) // Send address bytes for 0
		{
			Serial.println("Error writing address");
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		// loop thru 32 bytes at a time
		for (cnt = 0; cnt < Batts[BatteryType].Size; cnt += 32)
		{
			if (CheckClk()) // Check for clock not "stuck"
			{
				STATUS_LED_READY;
				return(ret);
			}

			// Try I2C read of 32 bytes
			BytesRead = Wire.requestFrom((uint8_t)(Batts[BatteryType].EEPROM_Address), (uint8_t)32, DO_STOP);

			if (32 != BytesRead)
			{
				Serial.println();
				Serial.println(BadRead);
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
				Serial.println("Interrupted by user");
				goto ReadError;
			}
		}
	}
	else
	{
		// loop thru 16 bytes at a time
		for (cnt = 0; cnt < Batts[BatteryType].Size; cnt += 16)
		{
			if (SwitchClk) // Disable clock drive if needed
				PassiveClk();

			if (CheckClk()) // Check for clock not "stuck"
			{
				STATUS_LED_READY;
				return(ret);
			}

			Wire.beginTransmission((uint8_t)(Batts[BatteryType].EEPROM_Address|(cnt >> 8))); // transmit to EEPROM
			if (Wire.write(cnt&0xff) != 1) // Send address byte
			{
				Serial.println("Error writing address");
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
				Serial.println(BadRead);
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
				Serial.println("Interrupted by user");
				goto ReadError;
			}
		}
	}
	Serial.println();
	Serial.println("Read ok!");
	ret = 0;

ReadError:
	STATUS_LED_READY;

	if (SwitchClk)  // Just in case we had an error
		ActiveClk();
	return(ret);
}

