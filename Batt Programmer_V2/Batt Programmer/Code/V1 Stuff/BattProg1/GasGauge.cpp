// Zebra battery programmer gas gauge stuff

#include <arduino.h>
#include <wire.h>
//#include <stdint.h>

#include "pwmbatt.h"
#include "BattProg.h"
#include "GasGauge.h"
#include "Auth.h"

#define SwapBytes(A) A = (int16_t)((uint16_t)A<<8 | (uint16_t)A>>8);

uint16_t DesignCap = 1000;  // Default gas gauge design capacity value
uint16_t CurTemp = -70;  // Current battery temp
uint16_t CurIntTemp = -70;  // Current internal gauge temp
uint32_t dwSecSinceFirstUse;  // Time since first use

uint8_t GasGaugeOK = 0;  // Data was OK Flag

// Function to read all gas gauge registers
uint8_t GG_ReadReg(void)
{
	int BytesRead; // Bytes read from I2C
	uint8_t Total = 0;
	uint8_t data[32], *cpnt, cnt;
	int16_t tmp;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear OK flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NO_GAUGE);
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

	// Read all reg in 32 byte chunks
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
			Serial.println(BAD_READ);
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read reg data
		cpnt = (uint8_t *)&BD.Gifted.CONTROL_STATUS;
		cpnt += Total;
		for (cnt = 0; cnt < BytesRead; ++cnt)
			*cpnt++ = Wire.read();

		Total += BytesRead;
	} while (GB_CMD_FULL_READ_SIZE > Total);

	// Convert temp
	if (BD.Gifted.TEMP < 2032 || BD.Gifted.TEMP > 3732) // Out of range (-70C to +100C), set to invalid
	{
		CurTemp =  -70;
		Serial.println(F("Bad temp!!!"));
		ret = 1;
		goto ReadError;
	}
	else
	{  // Convert to deg C
		tmp = (int16_t)BD.Gifted.TEMP - 2732;  // Convert to .1 deg C
		tmp = tmp / 10;  // Convert to deg C

		CurTemp = tmp; // Set new temp
	}

	// Get time since first use
	dwSecSinceFirstUse = ((uint32_t)BD.Gifted.ETU << 16) | (uint32_t)BD.Gifted.ETL;

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

// Function to read all new gas gauge registers
uint8_t newGG_ReadReg(uint8_t *buffer)
{
	int BytesRead; // Bytes read from I2C
	uint8_t Total = 0;
	uint8_t data[3], cnt;
	int16_t tmp;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear OK flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NO_GAUGE);
		return (ret);
	}

	if (!NEW_GAUGE)
	{
		Serial.println(F("Old gas gauge?"));
		return (ret);
	}

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Read all reg in 32 byte chunks
	Total = 0;
	do {
		delay(1);

		// Figure bytes to read
		if ((NEW_GB_CMD_FULL_READ_SIZE - Total) >= 32)
			cnt = 32;
		else
			cnt = NEW_GB_CMD_FULL_READ_SIZE - Total;

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
			Serial.println(BAD_READ);
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read reg data
		for (cnt = 0; cnt < BytesRead; ++cnt)
			buffer[Total+cnt] = Wire.read();

		Total += BytesRead;
	} while (NEW_GB_CMD_FULL_READ_SIZE > Total);

	// Convert temps
	CurTemp = buffer[NGG_TEMP] | (buffer[NGG_TEMP+1] << 8);
	if ( CurTemp < 2032 || CurTemp > 3732) // Out of range (-70C to +100C), set to invalid
	{
		CurTemp =  -70;
		Serial.println(F("Bad temp!!!"));
		ret = 1;
		goto ReadError;
	}
	else
	{  // Convert to deg C
		tmp = CurTemp - 2732;  // Convert to .1 deg C
		tmp = tmp / 10;  // Convert to deg C

		CurTemp = tmp; // Set new temp
	}
	CurIntTemp = buffer[NGG_INT_TEMP] | (buffer[NGG_INT_TEMP+1] << 8);
	if ( CurIntTemp < 2032 || CurIntTemp > 3732) // Out of range (-70C to +100C), set to invalid
	{
		CurIntTemp =  -70;
		Serial.println(F("Bad internal temp!!!"));
		ret = 1;
		goto ReadError;
	}
	else
	{  // Convert to deg C
		tmp = CurIntTemp - 2732;  // Convert to .1 deg C
		tmp = tmp / 10;  // Convert to deg C

		CurIntTemp = tmp; // Set new temp
	}

/* ***FIX***
	// Get time since first use
	dwSecSinceFirstUse = ((uint32_t)BD.Gifted.ETU << 16) | (uint32_t)BD.Gifted.ETL;
*/

	ret = 0;
	GasGaugeOK = 1; // Set OK flag

ReadError:
	delay(1);

	return (ret);
}

// Function to read the Alt manufacturer block
// Returns 0 on error, size of block otherwise
uint8_t GG_BlockRead(uint16_t cmd, uint8_t data[], uint8_t size)
{
	int BytesRead; // Bytes read from I2C
	uint8_t cnt, cnt1, csum;
	uint8_t buf[36];

	if (CheckClk()) // Check for clock not "stuck"
		return (0);

	// Send command
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup command
	buf[0] = CNTL_CMD;
	buf[1] = cmd & 0xff;
	buf[2] = (cmd >> 8) & 0xff;
	if (Wire.write(buf,3) != 3) // Send command
	{
		Serial.println(F("Error writing block read command"));
		Wire.endTransmission();    // Stop transmitting
		return (0);
	}
	Wire.endTransmission();    // stop transmitting

	delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		return (0);

	buf[0] = ALT_MANF_ACC;  // Set to MACData address
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(buf,1) != 1) // Send request for MACData read
	{
		Serial.println(F("Error setting up block read"));
		Wire.endTransmission();    // Stop transmitting
		return (0);
	}
	Wire.endTransmission();    // stop transmitting

	for (cnt1 = 0; cnt1 <= 1; ++cnt1)
	{
		if (CheckClk()) // Check for clock not "stuck"
			return (0);

		// Try I2C read of 18 bytes (Need 36, max buffer is 32)
		BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)18, DO_STOP);

		if (18 != BytesRead)
		{
			Serial.println();
			Serial.println(BAD_READ);
			while(Wire.available())  // flush buffer
				Wire.read();
			return (0);
		}

		// Read 18 bytes
		for (cnt = 0; cnt < 18; ++cnt)
			buf[cnt1*18+cnt] = Wire.read();
	}

	// Check length
	if (buf[BLK_READ_LEN] < 5 || buf[BLK_READ_LEN] > 36)
	{
		Serial.println();
		Serial.print(F("Bad block read len"));
		return (0);
	}

	// Check checksum
	csum = buf[BLK_READ_CSUM] + 1;
	for (cnt = 0; cnt < (buf[BLK_READ_LEN]-2); ++cnt)
		csum += buf[cnt];
	if (csum)
	{
		Serial.println();
		Serial.println(F("Bad block read csum!"));
		return (0);
	}

	// Check command
	if (buf[0] != (cmd & 0xff) || buf[1] != ((cmd >> 8) & 0xff))
	{
		Serial.println();
		Serial.println(F("Block read cmd mismatch!"));
		return (0);
	}

	// Copy over data
	for (cnt = 0; (cnt < (buf[BLK_READ_LEN]-4)) && (cnt < size); ++cnt)
		data[cnt] = buf[cnt+2];

	return (buf[BLK_READ_LEN]);
}

// Function to write the Alt manufacturer block
// Returns 0 on error, 1 otherwise.
uint8_t GG_BlockWrite(uint16_t cmd, uint8_t buf[], uint8_t len)
{
	uint8_t data[37];
	int BytesRead; // Bytes read from I2C
	uint8_t cnt, csum;

	// Check that write will fit into 32 bytes
	if (len > 29)
	{
		Serial.println(F("Too many bytes in block write"));
		return (0);
	}
	data[0] = ALT_MANF_ACC;  // Set to MACData address
	data[1] = cmd & 0xff;  // Set up command
	csum = data[1];
	data[2] = (cmd >> 8) & 0xff;
	csum += data[2];

	// Move data to buffer
	for (cnt = 0; cnt < len; ++cnt)
	{
		data[cnt+3] = buf[cnt];
		csum += data[cnt+3];
	}

	// Write data
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,len+3) != (len+3)) // Send block write
	{
		Serial.println(F("Error writing block!"));
		Wire.endTransmission();    // Stop transmitting
		return (0);
	}
	Wire.endTransmission();    // stop transmitting

	// Write checksum/len
	// Set checksum and length
	data[0] = MAC_DATA_CSUM;  // Set to checksum address
	data[1] = ~csum;
	data[2] = len + 4;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send block write
	{
		Serial.println(F("Error in block write!"));
		Wire.endTransmission();    // Stop transmitting
		return (0);
	}
	Wire.endTransmission();    // stop transmitting

	return (1);
}

// Function to get the gas gauge data
uint8_t GG_GetStuff(GIFTED_BATT_DATA_t *BattData)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[36], *cpnt, cnt;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear OK flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NO_GAUGE);
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
		Serial.println(BAD_READ);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read device type
	data[0] = Wire.read();
	data[1] = Wire.read();
	BattData->GG_DeviceType = (uint16_t)data[1] << 8 | (uint16_t)data[0];

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
		Serial.println(BAD_READ);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read firmware version
	data[0] = Wire.read();
	data[1] = Wire.read();
	BattData->GG_FirmwareVer = (uint16_t)data[1] << 8 | (uint16_t)data[0];

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

	// Get hardware version
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_HW_VERSION & 0xff;
	data[2] = (GG_HW_VERSION >> 8) & 0xff;
	if (Wire.write(data,3) != 3) // Setup read of hardware version
	{
		Serial.println(F("Error writing get hardware version command"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for hardware version
	{
		Serial.println(F("Error setting up hardware version read"));
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
		Serial.println(BAD_READ);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read hardware version
	data[0] = Wire.read();
	data[1] = Wire.read();
	BattData->GG_HardwareVer = (uint16_t)data[1] << 8 | (uint16_t)data[0];

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
		Serial.println(BAD_READ);
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
		Serial.println(BAD_READ);
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

	// Read manf blocks
	// Get Manf blockA
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_A;
	if (Wire.write(data,2) != 2) // Setup read of manf block A
	{
		Serial.println(F("Error writing manf block A command"));
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
		Serial.println(BAD_READ);
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
		Serial.println(BAD_READ);
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
		Serial.println(BAD_READ);
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

// Function to get the new gas gauge data
uint8_t newGG_GetStuff(NEW_GIFTED_BATT_DATA_t *BattData)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[36], *cpnt, cnt, cnt2;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear OK flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NO_GAUGE);
		return (ret);
	}

	if (!NEW_GAUGE)
	{
		Serial.println(F("Old gas gauge?"));
		return (ret);
	}

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get device type
	if (!GG_BlockRead(GG_DEVICE_TYPE,(uint8_t *)&BattData->GG_DeviceType))
		goto ReadError;

	// Get firmware version
	if (!GG_BlockRead(GG_FW_VERSION,BattData->GG_FirmwareVer))
		goto ReadError;

	// Get hardware version
	if (!GG_BlockRead(GG_HW_VERSION,(uint8_t *)&BattData->GG_HardwareVer))
		goto ReadError;

	delay(1);
	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get time in cap ranges over temps
	for (cnt = 0; cnt < NUM_TEMPS; ++cnt)
	{
		if (!GG_BlockRead(NGG_LIFE_TIME_CAPS+cnt,data))
			goto ReadError;

		// Copy over time at caps
		for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
			BD.NG.times[cnt][cnt2] = (uint32_t)data[(cnt2*4)+5] << 24 | (uint32_t)data[(cnt2*4)+4] << 16 | (uint32_t)data[(cnt2*4)+3] << 8 | (uint32_t)data[(cnt2*4)+2];
	}

	delay(1);

	if(!GG_BlockRead(NGG_MANF_NAME,BattData->ManufName))
		goto ReadError;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get date made
	if (!GG_BlockRead(NGG_MANF_DATE,(uint8_t *)&BattData->DateMade))
		goto ReadError;

	delay(1);

	if (!GG_BlockRead(NGG_SERIAL_NUM,(uint8_t *)&BattData->SerialNum))
		goto ReadError;

	// Read manf blocks
	if (!GG_BlockRead(NGG_MANF_BLKA,(uint8_t *)&BattData->InitData))
		goto ReadError;
	if (!GG_BlockRead(NGG_MANF_BLKB,((uint8_t *)&BattData->InitData)+32))
		goto ReadError;
	if (!GG_BlockRead(NGG_MANF_BLKC,((uint8_t *)&BattData->InitData)+64))
		goto ReadError;

	ret = 0;
	GasGaugeOK = 1; // Set OK flag

ReadError:
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
		Serial.println(NO_GAUGE);
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
		Serial.println(HIB_ERR);
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
		Serial.println(HIB_ERR);
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
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (BytesRead != 2)
	{
		Serial.println();
		Serial.println(BAD_READ);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read reg data
	cpnt = (uint8_t *)&BD.Gifted.CONTROL_STATUS;
	*cpnt++ = Wire.read();
	*cpnt = Wire.read();

	ret = 0;

	if (BD.Gifted.CONTROL_STATUS & STATUS_LOW_HIBERNATE_BIT)
		Serial.println("Hibernate bit was successfully set");
	else
	{
		Serial.println("ERROR: Hibernate bit was NOT set");
		ret = 1;
	}

	if (BD.Gifted.CONTROL_STATUS & STATUS_LOW_FULLSLEEP_BIT)
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

uint8_t GG_SetFullSleep(uint8_t ask)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[10], *cpnt;
	uint8_t ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NO_GAUGE);
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
		Serial.println(HIB_ERR);
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
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (BytesRead != 2)
	{
		Serial.println();
		Serial.println(BAD_READ);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read reg data
	cpnt = (uint8_t *)&BD.Gifted.CONTROL_STATUS;
	*cpnt++ = Wire.read();
	*cpnt = Wire.read();

	ret = 0;

	if (BD.Gifted.CONTROL_STATUS & STATUS_LOW_FULLSLEEP_BIT)
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
	uint8_t data[6];
	uint8_t ret = 1, cnt;
	uint16_t Status;

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
		Serial.println(F("Error sending second key"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	for (cnt = 0; cnt < 20; ++cnt)
	{
		delay (100);  // Wait a bit

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Check if unsealed
		// Setup data
		data[0] = CNTL_CMD;
		data[1] = GG_READ_CONTROL_STATUS & 0xff;
		data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,3) != 3) // Send write of control/status
		{
			Serial.println(F("Error sending control/status read!"));
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

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
		BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

		if (BytesRead != 2)
		{
			Serial.println();
			Serial.println(BAD_READ);
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read reg data
		Status = Wire.read();
		Status |= (Wire.read()<<8);

		// Check if it's unsealed
		if (!(Status & (STATUS_HIGH_SS_BIT << 8)))
		{
			ret = 0;
			goto ReadOK;
		}
	}

ReadError:
	Serial.println("Unable to unseal gas gauge");
ReadOK:
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

uint8_t GG_FullUnseal(void)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[6];
	uint8_t ret = 1, cnt;
	uint16_t Status;

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

	if (NEW_GAUGE)
		delay(1000); // Wait a bit, only needed for the new gauge

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send third unlock key
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = UNSEAL_KEY3 & 0xff;
	data[2] = (UNSEAL_KEY3 >> 8) & 0xff;

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

	// Send forth unlock key
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = UNSEAL_KEY4 & 0xff;
	data[2] = (UNSEAL_KEY4 >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error sending first key"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	for (cnt = 0; cnt < 20; ++cnt)
	{
		delay (100);  // Wait a bit

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Check if unsealed
		// Setup data
		data[0] = CNTL_CMD;
		data[1] = GG_READ_CONTROL_STATUS & 0xff;
		data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,3) != 3) // Send write of control/status
		{
			Serial.println(F("Error sending control/status read!"));
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

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
		BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

		if (BytesRead != 2)
		{
			Serial.println();
			Serial.println(BAD_READ);
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read reg data
		Status = Wire.read();
		Status |= (Wire.read()<<8);

		// Check if it's unsealed
		if (!(Status & ((STATUS_HIGH_SS_BIT << 8) | (STATUS_HIGH_FAS_BIT << 8))))
		{
			ret = 0;
			goto ReadOK;
		}
	}

ReadError:
	Serial.println("Unable to full unseal gas gauge");
ReadOK:
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

void GG_Reset(uint8_t ask)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data[10], *cpnt;

	if (!BatteryType)  // Check for valid battery type
		return;

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NO_GAUGE);
		return;
	}

	if (ask)
	{
		Serial.println(F("Reset gas gauge? ARE YOU SURE??? (y/n)?"));
		while(!Serial.available())
		;
		data[0] = Serial.read();
		if (data[0] != 'y' && data[0] != 'Y')
			return;

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
#endif
}

void GG_do_Unseal(void)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data;

	Serial.println(UNSEAL);
	while(!Serial.available())
	;
	data = Serial.read();
	if (data != 'y' && data != 'Y')
		return;

	if (!GG_Unseal())
		Serial.println(F("Gas gauge has been unsealed!"));

#endif
	return;
}

void GG_do_FullUnseal(void)
{
#ifdef ZEBRA_MODE
	int BytesRead; // Bytes read from I2C
	uint8_t data;

	Serial.println(UNSEAL);
	while(!Serial.available())
	;
	data = Serial.read();
	if (data != 'y' && data != 'Y')
		return;

	if (!GG_FullUnseal())
		Serial.println(F("Gas gauge has been fully unsealed!"));

#endif
	return;
}

void GG_do_Seal(void)
{
#ifdef ZEBRA_MODE
	uint8_t data;

	if (!GG_Seal())
		Serial.println(F("Gas gauge has been sealed!"));

#endif
	return;
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
		Serial.println(NO_GAUGE);
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send seal command
	// Setup data
	data[0] = CNTL_CMD;
	if (!NEW_GAUGE)
	{  // Do M200 stuff
		data[1] = GG_SEAL_CHIP & 0xff;
		data[2] = (GG_SEAL_CHIP >> 8) & 0xff;
	}
	else // New gauge
	{
		data[1] = NGG_SEAL_CHIP & 0xff;
		data[2] = (NGG_SEAL_CHIP >> 8) & 0xff;
	}

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(F("Error sending seal"));
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

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

uint8_t GG_GetNewManfBlocks(uint8_t *buffer)
{
#ifdef ZEBRA_MODE
	// Check the data
	GG_BlockRead(NGG_MANF_BLKA,buffer,32);
	GG_BlockRead(NGG_MANF_BLKB,buffer+32,32);
	GG_BlockRead(NGG_MANF_BLKC,buffer+64,32);
	return (0);
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
		Serial.println(NO_GAUGE);
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
		Serial.println(BAD_READ);
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
	if (Wire.write(data,2) != 2) // Setup read of block B
	{
		Serial.println(F("Error writing manf block B command"));
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
		Serial.println(BAD_READ);
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
		Serial.println(BAD_READ);
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
		Serial.println(NO_GAUGE);
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

// Note, this function assumes the gauge is unsealed!
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
		Serial.println(NO_GAUGE);
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
			Serial.println(BAD_READ);
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

//Serial.println();
//Serial.print(vMSByte,HEX);

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
}

void DumpNewGGConfig(void)
{
	// ***FIX***
	Serial.println("Dump GG config, coming soon!");
}

void SaveNewGGConfig(void)
{
	// ***FIX***
	Serial.println("Save new GG config, coming soon!");
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
	// ***FIX***
	Serial.println("Save GG config, coming soon!");
}

