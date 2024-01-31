// Data file validation stuff

#include <Arduino.h>

#include "pwmbatt.h"
#include "BattProg.h"
#include "aes.h"
#include "keys.h"
#include "GasGauge.h"

uint8_t PPP_Valid = 0; // Battery PP+ data valid flag
uint8_t PP_Valid = 0; // Battery PP data valid flag

// Shared battery data
uint16_t myear;
uint8_t	mmonth;
uint8_t mday;
uint16_t batteryRatedCapacity;
char batteryID[SER_NUM_LENGTH+1];		// Serial number
char batteryPartNumber[PART_NUMBER_LENGTH+1];	// Symbol's part number
char batteryPartNumberEx[PART_NUMBER_LENGTH_EX]; // New Symbol's part number to accomadate 6 digit battery family number
char batteryPartNumberNew[PART_NUMBER_LENGTH_NEW]; // Latest part number length for gifted batts
GIFTED_BATT_DATA_t g_GiftedBattData;	// Gifted battery data

// Routine to decrypt a PP+ buffer
void DecryptBuffer(uint8_t *src, uint8_t *dest)
{
	uint16_t cnt;

	// Loop thru the blocks
	for (cnt = 0; cnt < 32; ++cnt)
	{  // Do decryption
		if (MC18_BATT == BatteryType || ROGUE_BATT == BatteryType || FRENZY_BATT == BatteryType)
			MovI2Cdata(src+(cnt*16),dest+(cnt*16),cnt == 3 ? Sunrise_Key2 : Sunrise_Key1,0);
		else if (IRONMAN_BATT == BatteryType)
			MovI2Cdata(src+(cnt*16),dest+(cnt*16),cnt == 3 ? IronMan_Key2 : IronMan_Key1,0);
		else if (FRENZY_BATT == BatteryType)
			MovI2Cdata(src+(cnt*16),dest+(cnt*16),cnt == 3 ? Freeport_Key2 : Freeport_Key1,0);
		else
			MovI2Cdata(src+(cnt*16),dest+(cnt*16),cnt == 3 ? Falcon_Key2 : Falcon_Key1,0);
	}
}

// Checkout PP+ data
uint8_t ValidatePPP(uint8_t *Buf)
{
	uint8_t *cpnt, cCSum, dwBcnt, ret=0, tmp;
	uint32_t dwCnt;
	char sz_PN[MPA3_BATT_PN_LEN];


	PPP_Valid = 0;  // Set data to not valid

	if (Verbose)
		Serial.print(F("Check Gas Gauge:"));

	// Get data from the GG
	if (GG_GetStuff(&g_GiftedBattData))
			ret = 1;
	if (GasGaugeOK)
	{
		// Check for valid device type
		if (MPA3_GG_DEVICE_TYPE != g_GiftedBattData.M200_DeviceType)
		{  // invalid device type
			Serial.println(F("Bad Device Type"));
			ret = 1;
		}

		// Check for valid firmware version
		if ((g_GiftedBattData.M200_FirmwareVer & 0xff00) != MPA3_GG_FW_VERSION)
		{  // Invalid firmware version
			Serial.println(F("Bad Firmware Version"));
			ret = 1;
		}

	// Read GG regs??? ***FIX***
	// Check hibernate bit??? ***FIX***

		// Verify fixed manf blocks
		for (dwCnt = 0; dwCnt < GB_MANF_BLK_SIZE; ++dwCnt)  // Check fixed blocks
		{
			if (pgm_read_byte(BlockA_Fixed+dwCnt) != g_GiftedBattData.BlockA[dwCnt] || pgm_read_byte(BlockB_Fixed+dwCnt) != g_GiftedBattData.BlockB[dwCnt])
			{
				Serial.println(F("Bad manf fixed block"));
				ret = 1;
			}
		}

		// Check Manf block C checksum
		cpnt = (uint8_t *)&g_GiftedBattData.Checksum_c;
		cCSum = 0;
		for (dwCnt = 0; dwCnt < GB_MANF_BLK_SIZE; ++dwCnt)  // Check checksum
		{
			cCSum = cCSum + *(cpnt+dwCnt);
		}
		if (cCSum) // Bad checksum?
		{
			Serial.println(F("Bad manf checksum"));
			ret = 1;
		}

		// Verify gifted battery Manf area type
		if (g_GiftedBattData.DataFormat541 != GB_MANF_SupportedType) // Check data type
		{
			Serial.println(F("Bad manf data type"));
			ret = 1;
		}

		if (!ret && Verbose)
			Serial.println(F("Passed"));
	}
	else
		Serial.println(F("Skipping Gas Gauge tests!!!"));

	if (Verbose)
		Serial.println(F("Checking EEPROM/Auth chip data"));

	tmp = 0;
	// Decrypt the EEPROM data
	cpnt = (uint8_t *)&g_GiftedBattData;
	DecryptBuffer(Buf,cpnt);

	// Get static data
	myear = ((g_GiftedBattData.DateMade[2] & 0x0f) << 8) | g_GiftedBattData.DateMade[1];
	mmonth = g_GiftedBattData.DateMade[2] >> 4;
	mday = g_GiftedBattData.DateMade[0];

	// Do longer New part number
	memcpy(sz_PN,g_GiftedBattData.BatteryPartNumber,21);
	sz_PN[21] = 0;  // Add terminator at max gifted part number length
	if (g_GiftedBattData.Revision >= 0xc9)
		sprintf(batteryPartNumberNew,"%s R.%02d", sz_PN, g_GiftedBattData.Revision-0xc8);
	else if (g_GiftedBattData.Revision <= 26)
		sprintf(batteryPartNumberNew,"%s R.%c", sz_PN, '@'+g_GiftedBattData.Revision);
	else
		sprintf(batteryPartNumberNew,"%s R.%c%c", sz_PN, '@'+((g_GiftedBattData.Revision-1)/26));

	// Do older Ex part number
	// Added support for longer part number "ex" for CQ138814
	sz_PN[18] = 0;  // Add terminator at max Symbol part number length to fit into 24 char string w/rev.

	if (g_GiftedBattData.Revision >= 0xc9)
		sprintf(batteryPartNumberEx,"%s R.%02d", sz_PN, g_GiftedBattData.Revision-0xc8);
	else if (g_GiftedBattData.Revision <= 26)
		sprintf(batteryPartNumberEx,"%s R.%c", sz_PN, '@'+g_GiftedBattData.Revision);
	else
		sprintf(batteryPartNumberEx,"%s R.%c%c", sz_PN, '@'+((g_GiftedBattData.Revision-1)/26));

	// Do clipped part number field
	sz_PN[13] = 0;  // Add terminator at max Moto part number length to fit into 19 char string w/rev.

	if (g_GiftedBattData.Revision >= 0xc9)
		sprintf(batteryPartNumber,"%s R.%02d", sz_PN, g_GiftedBattData.Revision-0xc8);
	else if (g_GiftedBattData.Revision <= 26)
		sprintf(batteryPartNumber,"%s R.%c", sz_PN, '@'+g_GiftedBattData.Revision);
	else
		sprintf(batteryPartNumber,"%s R.%c%c", sz_PN, '@'+((g_GiftedBattData.Revision-1)/26));

	memcpy(batteryID,g_GiftedBattData.SerialNumber,5);
	batteryID[5] = 0;  // Add terminator

	g_GiftedBattData.PadByte1 = 0;  // Clear pad bytes
	g_GiftedBattData.PadByte2 = 0;

	// Check Static section checksum
	if (Verbose)
		Serial.print(F("Static data area checksum: "));
	cpnt = (uint8_t *)&g_GiftedBattData;   // NOTE: Also used in a loop down a bit
	cCSum = 0;
	for (dwCnt = 0; dwCnt < GB_STATIC_DATA_SIZE; ++dwCnt)  // Check checksum
	{
		cCSum = cCSum + *(cpnt+dwCnt);
	}
	if (cCSum) // Bad checksum?
	{
		Serial.println(F("Bad static area checksum"));
		ret = 1;
		tmp = 1;
	}
	if (!tmp && Verbose)
		Serial.println(F("Passed"));

	// Verify gifted battery static data area type
	if (Verbose)
		Serial.print(F("Static data area data type: "));
	tmp = 0;
	if (g_GiftedBattData.DataType != GB_SupportedType) // Check data type
	{
		Serial.println(F("Bad static data area data type"));
		ret = 1;
		tmp = 1;
	}
	if (!tmp && Verbose)
		Serial.println(F("Passed"));
		
	// Verify gifted battery BQ block allocation byte
	if (Verbose)
		Serial.print(F("GG block allocation: "));
	tmp = 0;
	if (g_GiftedBattData.Block_Allocation_27541 != GB_BQ_ALLOC)
	{
		Serial.println(F("Bad GG block allocation byte"));
		ret = 1;
	}

	// Verify blank EEPROM blocks
	tmp = 0;  // Clear had an error flag
	for (dwCnt = 0; dwCnt < (Batts[BatteryType].UseAuthData ? 26 : 32); ++dwCnt)  // Check blocks
	{
		if (g_GiftedBattData.Block_Allocation_EEPROM & ((uint32_t)1 << dwCnt))
		{  // block is free, check for default pattern
			if (Verbose)
			{
				Serial.print(dwCnt);
				Serial.print(F(" "));
			}
			for (cCSum = 0; cCSum < 15; ++cCSum)  // Check first 15 bytes
			{
				if (*(cpnt+(16*dwCnt)+cCSum) != dwCnt) // Check for block number in each byte
				{
					if (Verbose)
						Serial.println();
					Serial.print(F("Bad free data block: "));
					Serial.println(dwCnt);
					ret = 1;
					tmp = 1;
					break;
				}
			}
		}
	}
	if (!tmp && Verbose)
		Serial.println(F("Passed"));

	// Verify Lock Block
	cCSum = 0;
	tmp = 0;
	if (Verbose)
		Serial.print(F("Static factory block: "));
	tmp = 0;
	// Start at the end of block and go backwards to work
	// around an optimization error cause by the compiler knowing
	// that g_GiftedBattData.Manufacture_ID is 8 bytes long and preventing
	// a loop cnt of 15 from being used.
	cpnt = (uint8_t *)&g_GiftedBattData.Checksum_y;  
	for (dwCnt = 0; dwCnt < 15; ++dwCnt)  // Check checksum
	{
		cCSum = cCSum + *(cpnt-dwCnt);
	}
	if (cCSum) // Bad checksum?
	{
		Serial.println(F("Bad static factory block checksum"));
			ret = 1;
			tmp = 1;
	}
	if (!tmp && Verbose)
		Serial.println(F("Passed"));

	if (Verbose)
		Serial.print(F("Manufacture ID block: "));
	tmp = 0;
	// Check ID
	for (dwCnt = 0; dwCnt < 14; ++dwCnt)  // loop thru block data
	{
		if (dwCnt < 8)
		{ // Check ID
			if (g_GiftedBattData.Manufacture_ID[dwCnt] != g_GiftedBattData.M200_Manf_ID[dwCnt])
			{
				Serial.println(F("Manufacture ID mismatch"));
				ret = 1;
				tmp = 1;
				break;
			}
		}
		else
		{ // Check for zero
			if (g_GiftedBattData.DummyData[dwCnt-8])
			{
				Serial.println(F("Manufacture ID Non-zero pad bytes"));
				ret = 1;
				tmp = 1;
				break;
			}
		}
	}

	if (!tmp && Verbose)
		Serial.println(F("Passed"));

	if (DesignCap > g_GiftedBattData.CC_Threshold)  // Use higher of design cap and CC_Threshold
		batteryRatedCapacity = DesignCap;
	else
		batteryRatedCapacity = g_GiftedBattData.CC_Threshold;

	// Verify Dynamic block 1
	tmp = 0;
	if (Verbose)
		Serial.print(F("Dynamic blocks: "));
	cpnt = (uint8_t *)&g_GiftedBattData.Dyn1Block[0];
	cCSum = 0;
	for (dwCnt = 0; dwCnt < 15; ++dwCnt)  // Check checksum
	{
		cCSum = cCSum + *(cpnt+dwCnt);
	}

	if (cCSum) // Bad checksum?
	{
		Serial.println(F("Bad dyn1 checksum"));
		ret = 1;
		tmp = 1;
	}

	// Verify Dynamic block 2
	cpnt = (uint8_t *)&g_GiftedBattData.Dyn1Block[1];
	cCSum = 0;
	for (dwCnt = 0; dwCnt < 15; ++dwCnt)  // Check checksum
	{
		cCSum = cCSum + *(cpnt+dwCnt);
	}

	if (cCSum) // Bad checksum?
	{
		Serial.println(F("Bad dyn2 checksum"));
		ret = 1;
		tmp = 1;
	}

	// Check ID in block1
	for (dwCnt = 0; dwCnt < 8; ++dwCnt)  // loop thru ID data
	{
		if (g_GiftedBattData.Dyn1Block[0].CopiedManufactureID[dwCnt] != g_GiftedBattData.M200_Manf_ID[dwCnt])
		{
			break; // Stop on error
		}
	}

	if (dwCnt < 8)  // Ended too soon?
	{ // Had an error
		// Check for all ones, only allowable error (unprogrammed part)
		for (dwCnt = 0; dwCnt < 8; ++dwCnt)  // loop thru ID data
		{
			// Make sure it's all 0xff's
			if (0xff != g_GiftedBattData.Dyn1Block[0].CopiedManufactureID[dwCnt])
			{
				Serial.println(F("Bad MID1"));
				ret = 1;
				tmp = 1;
				break;
			}
		}

		Serial.println(F("Note: blk1 ID not set "));
	}

	// Check ID in block2
	for (dwCnt = 0; dwCnt < 8; ++dwCnt)  // loop thru ID data
	{
		if (g_GiftedBattData.Dyn1Block[1].CopiedManufactureID[dwCnt] != g_GiftedBattData.M200_Manf_ID[dwCnt])
		{
			break; // Stop on error
		}
	}

	if (dwCnt < 8)  // Ended too soon?
	{ // Had an error
		// Check for all ones, only alowable error (unprogrammed part)
		for (dwCnt = 0; dwCnt < 8; ++dwCnt)  // loop thru ID data
		{
			// Make sure it's all 0xff's
			if (0xff != g_GiftedBattData.Dyn1Block[1].CopiedManufactureID[dwCnt])
			{
				Serial.println(F("Bad MID in blk2"));
				ret = 1;
				tmp = 1;
				break;
			}
		}

		Serial.print(F("Note: blk2 not set "));
	}

	// Verify Dynamic blocks 3 & 4
	for (dwBcnt = 0; dwBcnt < 2; ++dwBcnt)
	{
		cpnt = (uint8_t *)&g_GiftedBattData.Dyn2Block[dwBcnt];
		cCSum = 0;
		for (dwCnt = 0; dwCnt < 15; ++dwCnt)  // Check checksum
		{
			cCSum = cCSum + *(cpnt+dwCnt);
		}

		if (cCSum) // Bad checksum?
		{
			if (dwBcnt)
				Serial.println(F("Bad dyn3 checksum"));
			else
				Serial.println(F("Bad dyn2 checksum"));
			ret = 1;
			tmp = 1;
		}
	}

	if (!tmp && Verbose)
		Serial.println(F("Passed"));
	// Check extended temp charging data if it's there
	if (EX_CHARGE_TEMP_BLOCKS != (g_GiftedBattData.Block_Allocation_EEPROM & EX_CHARGE_TEMP_BLOCKS))  // Check if any of those blocks are marked as used
	{
		tmp = 0;
		if (Verbose)
			Serial.print(F("JEITA charging data: "));

		// Zero the random pad bytes
		g_GiftedBattData.ExCharge.pad[10] = 0;
		for (dwCnt = 0; dwCnt < 5; ++dwCnt)
			g_GiftedBattData.ExCharge.TempRange[dwCnt].pad[2] = 0;

		// Check the checksum
		cpnt = (uint8_t *)&g_GiftedBattData.ExCharge;
		cCSum = 0;
		for (dwCnt = 0; dwCnt < sizeof(EX_TEMP_CHARGING_EP_t); ++dwCnt)  // Check checksum
		{
			cCSum = cCSum + *(cpnt+dwCnt);
		}

		if (cCSum) // Bad checksum?
		{
			Serial.println(F("Bad JEITA checksum"));
			ret = 1;
			tmp = 1;
		}
		if (!tmp && Verbose)
			Serial.println(F("Passed"));

	}
	else if (Verbose)
		Serial.println(F("Note: No JEITA charging data"));

	// Check for SD660 block being used
	if (!(g_GiftedBattData.Block_Allocation_EEPROM & SD660_BLOCK))
	{  // block is used
		tmp = 0;
		if (Verbose)
			Serial.print(F("SD660 data: "));
		tmp = 0;

		// Copy over unencrypted data
		memcpy((uint8_t *)&g_GiftedBattData+SD660blkaddr, Buf+SD660blkaddr, 16);

		// Check the checksum
		cpnt = (uint8_t *)&g_GiftedBattData;
		cpnt += SD660blkaddr; // Set to beginning of SD660 block
		cCSum = 0;

		for (dwCnt = 0; dwCnt < 16; ++dwCnt)  // Check checksum
		{
			cCSum = cCSum + *(cpnt+dwCnt);
		}
		if (cCSum) // Bad checksum?
		{
			Serial.println(F("Bad SDM660 data checksum"));
			ret = 1;
			tmp = 1;
		}

		if (!tmp && Verbose)
			Serial.println(F("Passed"));
	}
	else if (Verbose)
		Serial.println(F("Note: No SD660 data"));

	// Check for cell identity blocks being used
	if (!(g_GiftedBattData.Block_Allocation_EEPROM & CELL_IDENT_BLOCKS))
	{  // block is used
		if (Verbose)
			Serial.print(F("Cell identity data: "));
		tmp = 0;

		// Copy over unencrypted data
		memcpy((uint8_t *)&g_GiftedBattData+CELL_IDENT1blkaddr, Buf+CELL_IDENT1blkaddr, 32);

		// Check the checksum
		cpnt = (uint8_t *)&g_GiftedBattData;
		cpnt += CELL_IDENT1blkaddr; // Set to beginning of ident blocks
		cCSum = 0;

		for (dwCnt = 0; dwCnt < 32; ++dwCnt)  // Check checksum
		{
			cCSum = cCSum + *(cpnt+dwCnt);
		}
		if (cCSum) // Bad checksum?
		{
			Serial.print(F("Bad cell identity data checksum"));
			ret = 1;
			tmp = 1;
		}

		if (!tmp && Verbose)
			Serial.println(F("Passed"));
	}
	else if (Verbose)
		Serial.println(F("Note: No cell identity data"));

	// Get data from the GG
	if (GG_ReadReg())
		ret = 1;

	PPP_Valid = 1;  // Set data to valid

	return (ret);
}

// Checkout Pollux data
uint8_t ValidatePollux(uint8_t *Buf, uint8_t Format)
{
	uint16_t cnt;
	uint8_t csum, ret=0;

	PP_Valid = 0;  // Set data to not valid

	if (Verbose)
		Serial.print(F("Check QC PP data: "));

	// Check first checksum
	csum = 0;
	for (cnt = 0; cnt <= 239; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in PP data from 0-239"));
		ret = 1;
	}

	// Check first aggregate charge block
	csum = 0;
	for (cnt = 240; cnt <= 244; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in first PP aggregate charge block"));
		ret = 1;
	}

	// Check second aggregate charge block
	csum = 0;
	for (cnt = 248; cnt <= 252; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in second PP aggregate charge block"));
		ret = 1;
	}

	// Charger control area
	csum = 0;
	for (cnt = 256; cnt <= 279; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in PP charger control data"));
		ret = 1;
	}

	// Check 1725 data
	csum = 0;
	for (cnt = 288; cnt <= 295; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in PP 1725 data"));
		ret = 1;
	}

	// Check health data
	csum = 0;
	for (cnt = 296; cnt <= 303; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in PP health data"));
		ret = 1;
	}

	// Extended part number
	csum = 0;
	for (cnt = 304; cnt <= 323; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in PP extended part number"));
		ret = 1;
	}

	// Do 8956 area stuff if needed
	if (POLLUX8956_BATT_DATA == Format)
	{
		csum = 0;
		for (cnt = 352; cnt <= 480; ++cnt)
			csum += Buf[cnt];

		if (csum)
		{
			Serial.println(F("Checksum error in PP 8956 DTSI data"));
			ret = 1;
		}
	}

	// Fast charge area
	csum = 0;
	for (cnt = 504; cnt <= 511; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in PP fast charge data"));
		ret = 1;
	}

	if (ret && Verbose)
		Serial.println(F("Failed"));
	else if (!ret && Verbose)
		Serial.println(F("Passed"));

	PP_Valid = 1;  // Set data to valid
	return (ret);
}

// Checkout Hawkeye data
uint8_t ValidateHawkeye(uint8_t *Buf)
{
	uint16_t cnt;
	uint8_t csum, ret=0;

	PP_Valid = 0;  // Set data to not valid

	Serial.print(F("Check QC PP data: "));

	// Check first checksum
	csum = 0;
	for (cnt = 0; cnt <= 199; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in data from 0-199"));
		if (!KeepGoing())
			return (1);
		else
			ret = 1;
	}

	// Check first aggregate charge block
	csum = 0;
	for (cnt = 200; cnt <= 204; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in first aggregate charge block"));
		if (!KeepGoing())
			return (1);
		else
			ret = 1;
	}

	// Check second aggregate charge block
	csum = 0;
	for (cnt = 208; cnt <= 212; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in second aggregate charge block"));
		if (!KeepGoing())
			return (1);
		else
			ret = 1;
	}

	// Charger control area
	csum = 0;
	for (cnt = 216; cnt <= 239; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in charger control data"));
		if (!KeepGoing())
			return (1);
		else
			ret = 1;
	}

	// Check startup voltage
	csum = 0;
	for (cnt = 240; cnt <= 242; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in startup voltage"));
		if (!KeepGoing())
			return (1);
		else
			ret = 1;
	}

	// Check 1725 data
	csum = 0;
	for (cnt = 244; cnt <= 251; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in 1725 data"));
		if (!KeepGoing())
			return (1);
		else
			ret = 1;
	}

	// Check health data
	csum = 0;
	for (cnt = 252; cnt <= 259; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in health data"));
		if (!KeepGoing())
			return (1);
		else
			ret = 1;
	}

	// JEITA data
	csum = 0;
	for (cnt = 288; cnt <= 383; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(F("Checksum error in JEITA data"));
		if (!KeepGoing())
			return (1);
		else
			ret = 1;
	}

	if (ret)
		Serial.println(F("Failed"));
	else
		Serial.println(F("Passed"));

	PP_Valid = 1;  // Set data to valid
	return (ret);
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