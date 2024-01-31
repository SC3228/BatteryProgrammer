// Zebra battery programmer gas gauge stuff

#include <arduino.h>
#include <wire.h>
//#include <stdint.h>

#include "pwmbatt.h"
#include "BattProg.h"
#include "GasGauge.h"

#define SwapBytes(A) A = (int16_t)((uint16_t)A<<8 | (uint16_t)A>>8);

uint16_t DesignCap = 1000;  // Default gas gauge design capacity value
uint16_t CurTemp = -70;  // Current battery temp
uint32_t dwSecSinceFirstUse;  // Time since first use

uint8_t GasGaugeOK = 0;  // Data was OK Flag

// Function to read all gas gauge registers
uint8_t GG_ReadReg(void)
{
	int BytesRead; // Bytes read from I2C
	uint8_t Total;
	uint8_t data[32], *cpnt, cnt;
	int16_t tmp;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear OK flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(F("No gas gauge?"));
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Setup read of control status
	// Setup data
	data[0] = CNTL_CMD+Total;
	data[1] = GG_READ_CONTROL_STATUS & 0xff;
	data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send request for regs
	{
		Serial.println(F("Error setting ctrl/status read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// read reg in 32 byte chunks
	Total = 0;
	do {
		if (SwitchClk) // Enable clock drive if needed
		{
			ActiveClk();
			delay(15);
			PassiveClk();
		}
		else
			delay(1);

		// Figure bytes to read
		if ((GB_CMD_FULL_READ_SIZE - Total) >= 32)
			cnt = 32;
		else
			cnt = GB_CMD_FULL_READ_SIZE - Total;

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Get data
		// Setup data
		data[0] = CNTL_CMD+Total;

		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,1) != 1) // Send request for regs
		{
			Serial.println(F("Error setting reg read"));
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		// Try I2C read of all regs bytes
		BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, cnt, DO_STOP);

		if (cnt != BytesRead)
		{
			Serial.println();
			Serial.println(F("Bad read!"));
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read reg data
		cpnt = (uint8_t *)&g_GiftedBattData.CONTROL_STATUS;
		cpnt += Total;
		for (cnt = 0; cnt < BytesRead; ++cnt)
			*cpnt++ = Wire.read();

		Total += BytesRead;
	} while (GB_CMD_FULL_READ_SIZE > Total);

	// Convert temp
	if (g_GiftedBattData.TEMP < 2032 || g_GiftedBattData.TEMP > 3732) // Out of range (-70C to +100C), set to invalid
	{
		CurTemp =  -70;
		Serial.println(F("Bad temp!!!"));
		ret = 1;
		goto ReadError;
	}
	else
	{  // Convert to deg C
		tmp = (int16_t)g_GiftedBattData.TEMP - 2732;  // Convert to .1 deg C
		tmp = tmp / 10;  // Convert to deg C

		CurTemp = tmp; // Set new temp
	}

	// Get time since first use
	dwSecSinceFirstUse = ((uint32_t)g_GiftedBattData.ETU << 16) | (uint32_t)g_GiftedBattData.ETL;

	ret = 0;
	GasGaugeOK = 1; // Set OK flag

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
}

// Function to get the startup data
uint8_t GG_GetStuff(GIFTED_BATT_DATA_t *BattData)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[32], *cpnt, cnt;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear OK flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(F("No gas gauge?"));
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get device type
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_DEVICE_TYPE & 0xff;
	data[2] = (GG_DEVICE_TYPE >> 8) & 0xff;
	if (Wire.write(data,3) != 3) // Setup read of device type
	{
		Serial.println(F("Error writing get device type command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for device type
	{
		Serial.println(F("Error setting up device type read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 2 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (2 != BytesRead)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read device type
	data[0] = Wire.read();
	data[1] = Wire.read();
	BattData->M200_DeviceType = (uint16_t)data[1] << 8 | (uint16_t)data[0];

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get firmware version
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_FW_VERSION & 0xff;
	data[2] = (GG_FW_VERSION >> 8) & 0xff;
	if (Wire.write(data,3) != 3) // Setup read of device type
	{
		Serial.println(F("Error writing get version command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for version
	{
		Serial.println(F("Error setting up version type read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 2 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (2 != BytesRead)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read firmware version
	data[0] = Wire.read();
	data[1] = Wire.read();
	BattData->M200_FirmwareVer = (uint16_t)data[1] << 8 | (uint16_t)data[0];

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get manf ID
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = MFGID_CMD;
	if (Wire.write(data,1) != 1) // Setup read of manf ID
	{
		Serial.println(F("Error writing manf ID command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// Try I2C read of 8 bytes
	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)8, DO_STOP);

	if (8 != BytesRead)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf ID
	for (cnt = 0; cnt < 8; ++cnt)
		BattData->M200_Manf_ID[cnt] = Wire.read();

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get design capacity
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DCAP_CMD;
	if (Wire.write(data,1) != 1) // Setup read of design cap
	{
		Serial.println(F("Error writing design cap command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// Try I2C read of 2 bytes
	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (2 != BytesRead)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read design cap
	data[0] = Wire.read();
	data[1] = Wire.read();
	DesignCap = (uint16_t)data[1] << 8 | (uint16_t)data[0];

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockA
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_A;
	if (Wire.write(data,2) != 2) // Setup read of manf ID
	{
		Serial.println(F("Error writing manf blockA command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println(F("Error setting up block read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockA data
	for (cnt = 0; cnt < 32; ++cnt)
		BattData->BlockA[cnt] = Wire.read();

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockB
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_B;
	if (Wire.write(data,2) != 2) // Setup read of manf ID
	{
		Serial.println(F("Error writing manf blockB command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println(F("Error setting up block read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockB data
	for (cnt = 0; cnt < 32; ++cnt)
		BattData->BlockB[cnt] = Wire.read();

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockC
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_C;
	if (Wire.write(data,2) != 2) // Setup read of manf ID
	{
		Serial.println(F("Error writing manf blockC command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println(F("Error setting up block read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockC data
	cpnt = (uint8_t *)&BattData->Checksum_c;
	for (cnt = 0; cnt < 32; ++cnt)
		cpnt[cnt] = Wire.read();

	ret = 0;
	GasGaugeOK = 1; // Set OK flag


ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return(ret);
}

uint8_t GG_EnterHibernate(void)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[10], *cpnt;
	uint8_t ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(F("No gas gauge?"));
		return (ret);
	}

	Serial.println(F("Put gas gauge into hibernate (y/n)?"));
	while(!Serial.available())
	;
	data[0] = Serial.read();
	if (data[0] != 'y' && data[0] != 'Y')
		return(ret);

	// Do reg read to get the current time
	if (GG_ReadReg())
		return(ret);

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	// Setup write of control status
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_SET_FULLSLEEP & 0xff;
	data[2] = (GG_SET_FULLSLEEP >> 8) & 0xff;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error setting hibernate"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// Setup write of control status
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_SET_HIBERNATE & 0xff;
	data[2] = (GG_SET_HIBERNATE >> 8) & 0xff;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error setting hibernate"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// read control status reg
	data[0] = CNTL_CMD;
	data[1] = GG_READ_CONTROL_STATUS & 0xff;
	data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error setting ctrl/status read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get data
	// Setup data
	data[0] = CNTL_CMD;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for regs
	{
		Serial.println(F("Error setting reg read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of control/status reg
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, 2, DO_STOP);

	if (BytesRead != 2)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read reg data
	cpnt = (uint8_t *)&g_GiftedBattData.CONTROL_STATUS;
	*cpnt++ = Wire.read();
	*cpnt = Wire.read();

	ret = 0;

	if (g_GiftedBattData.CONTROL_STATUS & STATUS_LOW_HIBERNATE_BIT)
		Serial.println("Hibernate bit was successfully set");
	else
	{
		Serial.println("ERROR: Hibernate bit was NOT set");
		ret = 1;
	}

	if (g_GiftedBattData.CONTROL_STATUS & STATUS_LOW_FULLSLEEP_BIT)
		Serial.println("Fullsleep bit was successfully set");
	else
	{
		Serial.println("ERROR: Fullsleep bit was NOT set");
		ret = 1;
	}

	Serial.print(F("Seconds since first use: "));
	Serial.println(dwSecSinceFirstUse);

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
#else
	return (0);
#endif
}

uint8_t GG_SetFullSleep(uint8_t ask=1)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[10], *cpnt;
	uint8_t ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(F("No gas gauge?"));
		return (ret);
	}

	if (ask)
	{
		Serial.println(F("Set FULLSLEEP bit (y/n)?"));
		while(!Serial.available())
		;
		data[0] = Serial.read();
		if (data[0] != 'y' && data[0] != 'Y')
			return(ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Setup write of control status
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_SET_FULLSLEEP & 0xff;
	data[2] = (GG_SET_FULLSLEEP >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error setting hibernate"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// read control status reg
	data[0] = CNTL_CMD;
	data[1] = GG_READ_CONTROL_STATUS & 0xff;
	data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error setting ctrl/status read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get data
	// Setup data
	data[0] = CNTL_CMD;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for regs
	{
		Serial.println(F("Error setting reg read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of control/status reg
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, 2, DO_STOP);

	if (BytesRead != 2)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read reg data
	cpnt = (uint8_t *)&g_GiftedBattData.CONTROL_STATUS;
	*cpnt++ = Wire.read();
	*cpnt = Wire.read();

	ret = 0;

	if (g_GiftedBattData.CONTROL_STATUS & STATUS_LOW_FULLSLEEP_BIT)
		Serial.println("Fullsleep bit was successfully set");
	else
	{
		Serial.println("ERROR: Fullsleep bit was NOT set");
		ret = 1;
	}

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
#else
	return (0);
#endif
}

uint8_t GG_Unseal(void)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[10], *cpnt;
	uint8_t ret = 1;

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send first unlock key
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = UNSEAL_KEY1 & 0xff;
	data[2] = (UNSEAL_KEY1 >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error sending first key"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send second unlock key
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = UNSEAL_KEY2 & 0xff;
	data[2] = (UNSEAL_KEY2 >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error sending first key"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// Check if unsealed
	data[0] = CNTL_CMD;
	data[1] = GG_READ_CONTROL_STATUS & 0xff;
	data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error setting ctrl/status read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get data
	// Setup data
	data[0] = CNTL_CMD;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for regs
	{
		Serial.println(F("Error setting reg read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// I2C read of reg
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, 2, DO_STOP);

	if (BytesRead != 2)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read reg data
	cpnt = (uint8_t *)&g_GiftedBattData.CONTROL_STATUS;
	*cpnt++ = Wire.read();
	*cpnt = Wire.read();

	ret = 0;

	// Check if it's unsealed
	if (g_GiftedBattData.CONTROL_STATUS & (STATUS_HIGH_SS_BIT << 8))
	{
		Serial.println("Unable to unseal gas gauge");
		goto ReadError;
	}

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
#else
	return (0);
#endif
}

uint8_t GG_Reset(uint8_t ask = 1)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[10], *cpnt;
	uint8_t ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(F("No gas gauge?"));
		return (ret);
	}

	if (ask)
	{
		Serial.println(F("Reset gas gauge? ARE YOU SURE??? (y/n)?"));
		while(!Serial.available())
		;
		data[0] = Serial.read();
		if (data[0] != 'y' && data[0] != 'Y')
			return(ret);

		// Assume since we asked, we are not unsealed
		if (GG_Unseal())
			goto ReadError;
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send reset command
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_RESET_CHIP & 0xff;
	data[2] = (GG_RESET_CHIP >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error sending reset"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	Serial.println(F("Gas gauge has been reset!"));

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
#else
	return (0);
#endif
}

uint8_t GG_Seal(void)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[10], *cpnt;
	uint8_t ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(F("No gas gauge?"));
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send seal command
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_SEAL_CHIP & 0xff;
	data[2] = (GG_SEAL_CHIP >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error sending seal"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	Serial.println(F("Gas gauge has been sealed"));

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
#else
	return (0);
#endif
}

uint8_t GG_GetManfBlocks(uint8_t *buffer)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[32], cnt;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear OK flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(F("No gas gauge?"));
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockA
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_A;
	if (Wire.write(data,2) != 2) // Setup read of manf ID
	{
		Serial.println(F("Error writing manf blockA command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println(F("Error setting up block read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockA data
	for (cnt = 0; cnt < 32; ++cnt)
		buffer[cnt] = Wire.read();

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockB
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_B;
	if (Wire.write(data,2) != 2) // Setup read of manf ID
	{
		Serial.println(F("Error writing manf blockB command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println(F("Error setting up block read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockB data
	for (cnt = 0; cnt < 32; ++cnt)
		buffer[cnt+32] = Wire.read();

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockC
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_C;
	if (Wire.write(data,2) != 2) // Setup read of manf ID
	{
		Serial.println(F("Error writing manf blockC command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println(F("Error setting up block read"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(F("Bad read!"));
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockC data
	for (cnt = 0; cnt < 32; ++cnt)
		buffer[cnt+64] = Wire.read();

	ret = 0;

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
#else
	return (0);
#endif
}

uint8_t GG_GetDataFlash(DATA_FLASH_t *df)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[32], cnt;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear OK flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(F("No gas gauge?"));
		return (ret);
	}

	// Unlock the gauge!
	if (GG_Unseal())
		return (ret);

	// Get Safety subclass
	if (GG_ReadSubClass(SC_SAFETY,sizeof(CFG_SAFETY_t),(uint8_t *)&df->CfgSafety))
		goto ReadError;

	// Convert data
	SwapBytes(df->CfgSafety.OT_Chg);
	SwapBytes(df->CfgSafety.OT_ChgRecovery);
	SwapBytes(df->CfgSafety.OT_Dsg);
	SwapBytes(df->CfgSafety.OT_DsgRecovery);

	// Get Charge Inhibit subclass
	if (GG_ReadSubClass(SC_CHRG_INHIB_CFG,sizeof(CFG_CHG_INHIBIT_t),(uint8_t *)&df->CfgChgInhibit))
		goto ReadError;

	// Get Charge subclass
	if (GG_ReadSubClass(SC_CHARGE,sizeof(CFG_CHARGE_t),(uint8_t *)&df->CfgCharge))
		goto ReadError;

	// Get Calibration data subclass
	if (GG_ReadSubClass(SC_CAL_DATA,sizeof(CAL_DATA_t),(uint8_t *)&df->CalData))
		goto ReadError;

	ret = 0;

ReadError:
	GG_Seal();  // return to sealed mode

	return (ret);
#else
	return (0);
#endif
}


// Not, this function assumes the gauge is unsealed!
uint8_t GG_ReadSubClass(uint8_t SubClass, uint8_t Size, uint8_t *buffer)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[3], cnt, Block = 0, bytes;
	uint8_t ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(F("No gas gauge?"));
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	do
	{
		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Set subclass
		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

		// Setup data
		data[0] = DFCLS;
		data[1] = SubClass;
		data[2] = Block;

		if (Wire.write(data,3) != 3) // Setup read of data flash
		{
			Serial.println(F("Error writing data flash block command"));
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		data[0] = A_DF_CMD;
		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,1) != 1) // Send request for block data
		{
			Serial.println(F("Error setting up block read"));
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Try I2C read data
		if (Size > 32)
			bytes = 32;
		else
			bytes = Size;

		BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, bytes, DO_STOP);

		if (bytes != BytesRead)
		{
			Serial.println();
			Serial.println(F("Bad read!"));
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read data
		for (cnt = 0; cnt < bytes; ++cnt)
			buffer[cnt+(Block*32)] = Wire.read();

		if (SwitchClk) // Enable clock drive if needed
		{
			ActiveClk();
			delay(15);
			PassiveClk();
		}
		else
			delay(1);

		Size -= bytes;
	} while (Size);

	ret = 0;

ReadError:
	if (SwitchClk) // Enable clock drive if needed
		ActiveClk();

	return (ret);
#else
	return (0);
#endif
}

void PrintFloatFromInt(int16_t val)
{
	if (val < 0)
	{
		Serial.print(F("-"));
		Serial.print(abs(val)/10);
		Serial.print(F("."));
		Serial.println(abs(val)%10);
	}
	else
	{
		Serial.print(val/10);
		Serial.print(F("."));
		Serial.println(val%10);
	}
}

float ConvFloat(uint32_t InData)
{
	uint8_t vMSByte, vMidHiByte, vMidLoByte, vLSByte;
	uint8_t bIsPositive = 0;
	float fExponent, fResult;

	vLSByte = InData >> 24;
	vMidLoByte = (InData >> 16) &0xff;
	vMidHiByte = (InData >> 8) &0xff;
	vMSByte =  InData & 0xff;

Serial.println();
Serial.print(vMSByte,HEX);

	// Get the sign, its in the 0x00 80 00 00 bit
	if (!(vMidHiByte & 128))
		bIsPositive = 1;

	// Get the exponent, it's 2^(MSbyte - 0x80)
	fExponent = pow(2.0,vMSByte-128);

	// Or in 0x80 to the MidHiByte
	vMidHiByte = vMidHiByte | 128;

	// get value out of midhi byte
	fResult = vMidHiByte * pow(2.0,16.0);

	// add in midlow byte
	fResult = fResult + (vMidLoByte * pow(2.0,8.0));

	// add in LS byte
	fResult = fResult + (uint64_t)vLSByte;

	// multiply by 2^-24 to get the actual fraction
	fResult = fResult * pow(2.0,-24.0);

	// multiply fraction by the ‘exponent’ part
	fResult = fResult * fExponent;

	// Make negative if necessary
	if (!bIsPositive)
		fResult = -fResult;

	return (fResult);

/*
Dim bIsPositive As Boolean
Dim fExponent As Double
Dim fResult As Double
// Get the sign, its in the 0x00 80 00 00 bit
If (vMidHiByte And 128) = 0 Then bIsPositive = True
// Get the exponent, it's 2^(MSbyte - 0x80)
fExponent = 2 ^ (vMSByte - 128)
// Or in 0x80 to the MidHiByte
vMidHiByte = vMidHiByte Or 128
// get value out of midhi byte
fResult = (vMidHiByte) * 2 ^ 16
// add in midlow byte
fResult = fResult + (vMidLoByte * 2 ^ 8)
// add in LS byte
fResult = fResult + vLSByte
// multiply by 2^-24 to get the actual fraction
fResult = fResult * 2 ^ -24
// multiply fraction by the ‘exponent’ part
fResult = fResult * fExponent
// Make negative if necessary
If False = bIsPositive Then fResult = -fResult
Xemics_Storage2Dec = fResult
End Function
*/
}

void DumpGGConfig(void)
{
	DATA_FLASH_t df;  // Data flash struct
	float tmp;

	if (GG_GetDataFlash(&df))
		return;

	Serial.println();
	Serial.println(F("[Safety(Configuration)]"));
	Serial.print(F("OT Chg = "));
	PrintFloatFromInt(df.CfgSafety.OT_Chg);
	Serial.print(F("OT Chg Time = "));
	Serial.println(df.CfgSafety.OT_ChgTime);
	Serial.print(F("OT Chg Recovery = "));
	PrintFloatFromInt(df.CfgSafety.OT_ChgRecovery);
	Serial.print(F("OT Dsg = "));
	PrintFloatFromInt(df.CfgSafety.OT_Dsg);
	Serial.print(F("OT Dsg Time = "));
	Serial.println(df.CfgSafety.OT_DsgTime);
	Serial.print(F("OT Dsg Recovery = "));
	PrintFloatFromInt(df.CfgSafety.OT_DsgRecovery);

	Serial.print(F("CC Gain = "));
	Serial.println(ConvFloat(df.CalData.CC_Gain));


	Serial.println();
}

void SaveGGConfig(void)
{
	Serial.println("Save GG config");
}
