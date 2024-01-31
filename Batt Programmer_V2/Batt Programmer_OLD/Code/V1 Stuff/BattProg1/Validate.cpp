// Data file validation stuff

#include <Arduino.h>

#include "pwmbatt.h"
#include "BattProg.h"
#include "aes.h"
#include "F:\Includes\ArduinoKeys.h"
#include "GasGauge.h"

uint8_t PPP_Valid = 0; // PP+ Battery data valid flag
uint8_t PPPV2_Valid; // PP+ V2 Battery data valid flag
uint8_t PP_Valid = 0; // PP Battery data valid flag
uint8_t VT_Valid = 0; // VT Battery data valid flag

// Shared battery data
uint16_t myear;
uint8_t	mmonth;
uint8_t mday;
uint16_t batteryRatedCapacity;
char batteryID[SER_NUM_LENGTH+1];		// Serial number
char batteryPartNumber[PART_NUMBER_LENGTH+1];	// Symbol's part number
char batteryPartNumberEx[PART_NUMBER_LENGTH_EX]; // New Symbol's part number to accomadate 6 digit battery family number
char batteryPartNumberNew[PART_NUMBER_LENGTH_NEW]; // Latest part number length for gifted batts
BatteryData BD;	// Battery data

// Routine to decrypt a PP+ buffer
void DecryptBuffer(uint8_t *src, uint8_t *dest)
{
	uint16_t cnt;

	// Loop thru the blocks
	for (cnt = 0; cnt < 32; ++cnt)
	{  // Do decryption
		if (MC18_BATT == BatteryType || ROGUE_BATT == BatteryType)
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
	uint8_t *cpnt, cCSum, dwBcnt, ret=0, tmp, tmp2;
	uint16_t wCSum;
	uint32_t dwCnt;
	char sz_PN[MPA3_BATT_PN_LEN];


	PPP_Valid = 0;  // Set data to not valid

	if (Verbose)
	{
		Serial.println();
		Serial.println(F("Validating PP+ battery data"));
		Serial.print(F("Checking Gas Gauge: "));
	}

	if (GG_GetStuff(&BD.Gifted))
		ret = 1;

	if (GasGaugeOK)
	{
		// Check for valid device type
		if (MPA3_GG_DEVICE_TYPE != BD.Gifted.GG_DeviceType)
		{  // invalid device type
			Serial.println(F("Bad Device Type"));
			ret = 1;
		}

		// Check for valid firmware version
		if ((BD.Gifted.GG_FirmwareVer & 0xff00) != MPA3_GG_FW_VERSION)
		{  // Invalid firmware version
			Serial.println(F("Bad Firmware Version"));
			ret = 1;
		}

	// Read GG regs??? ***FIX***
	// Check hibernate bit??? ***FIX***

		// Verify fixed manf blocks
		for (dwCnt = 0; dwCnt < GB_MANF_BLK_SIZE; ++dwCnt)  // Check fixed blocks
		{
			if (pgm_read_byte(BlockA_Fixed+dwCnt) != BD.Gifted.BlockA[dwCnt] || pgm_read_byte(BlockB_Fixed+dwCnt) != BD.Gifted.BlockB[dwCnt])
			{
				Serial.println(F("Bad manf fixed block"));
				ret = 1;
				break;
			}
		}

		// Check Manf block C checksum
		cpnt = (uint8_t *)&BD.Gifted.Checksum_c;
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
		if (BD.Gifted.DataFormat541 != GB_MANF_SupportedType) // Check data type
		{
			Serial.println(F("Bad manf data type"));
			ret = 1;
		}

		if (!ret && Verbose)
			Serial.println(PASSED);
	}
	else
		Serial.println(F("Skipping Gas Gauge tests!!!"));

	if (Verbose)
		Serial.println(F("Checking EEPROM/Auth chip data"));

	tmp = 0;
	// Decrypt the EEPROM data
	cpnt = (uint8_t *)&BD.Gifted;
	DecryptBuffer(Buf,cpnt);

	// Get static data
	myear = ((BD.Gifted.DateMade[2] & 0x0f) << 8) | BD.Gifted.DateMade[1];
	mmonth = BD.Gifted.DateMade[2] >> 4;
	mday = BD.Gifted.DateMade[0];

	// Do longer New part number
	memcpy(sz_PN,BD.Gifted.BatteryPartNumber,21);
	sz_PN[21] = 0;  // Add terminator at max gifted part number length
	if (BD.Gifted.Revision >= 0xc9)
		sprintf(batteryPartNumberNew,"%s R.%02d", sz_PN, BD.Gifted.Revision-0xc8);
	else if (BD.Gifted.Revision <= 26)
		sprintf(batteryPartNumberNew,"%s R.%c", sz_PN, '@'+BD.Gifted.Revision);
	else
		sprintf(batteryPartNumberNew,"%s R.%c%c", sz_PN, '@'+((BD.Gifted.Revision-1)/26),'A'+((BD.Gifted.Revision-1)%26));

	// Do older Ex part number
	// Added support for longer part number "ex" for CQ138814
	sz_PN[18] = 0;  // Add terminator at max Symbol part number length to fit into 24 char string w/rev.

	if (BD.Gifted.Revision >= 0xc9)
		sprintf(batteryPartNumberEx,"%s R.%02d", sz_PN, BD.Gifted.Revision-0xc8);
	else if (BD.Gifted.Revision <= 26)
		sprintf(batteryPartNumberEx,"%s R.%c", sz_PN, '@'+BD.Gifted.Revision);
	else
		sprintf(batteryPartNumberEx,"%s R.%c%c", sz_PN, '@'+((BD.Gifted.Revision-1)/26),'A'+((BD.Gifted.Revision-1)%26));

	// Do clipped part number field
	sz_PN[13] = 0;  // Add terminator at max Moto part number length to fit into 19 char string w/rev.

	if (BD.Gifted.Revision >= 0xc9)
		sprintf(batteryPartNumber,"%s R.%02d", sz_PN, BD.Gifted.Revision-0xc8);
	else if (BD.Gifted.Revision <= 26)
		sprintf(batteryPartNumber,"%s R.%c", sz_PN, '@'+BD.Gifted.Revision);
	else
		sprintf(batteryPartNumber,"%s R.%c%c", sz_PN, '@'+((BD.Gifted.Revision-1)/26),'A'+((BD.Gifted.Revision-1)%26));

	memcpy(batteryID,BD.Gifted.SerialNumber,5);
	batteryID[5] = 0;  // Add terminator

	BD.Gifted.PadByte1 = 0;  // Clear pad bytes
	BD.Gifted.PadByte2 = 0;

	// Check Static section checksum
	if (Verbose)
		Serial.print(F("Static data area checksum: "));
	cpnt = (uint8_t *)&BD.Gifted;   // NOTE: Also used in a loop down a bit
	cCSum = 0;
	for (dwCnt = 0; dwCnt < GB_STATIC_DATA_SIZE; ++dwCnt)  // Check checksum
	{
		cCSum = cCSum + *(cpnt+dwCnt);
	}
	if (cCSum) // Bad checksum?
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
		tmp = 1;
	}
	if (!tmp && Verbose)
		Serial.println(PASSED);

	// Verify gifted battery static data area type
	if (Verbose)
		Serial.print(F("Static data area data type: "));
	tmp = 0;
	if (BD.Gifted.DataType != GB_SupportedType) // Check data type
	{
		Serial.println(F("Bad static data area data type"));
		ret = 1;
		tmp = 1;
	}
	if (!tmp && Verbose)
		Serial.println(PASSED);
		
	// Verify gifted battery BQ block allocation byte
	if (Verbose)
		Serial.print(F("GG block allocation: "));
	tmp = 0;
	if (BD.Gifted.Block_Allocation_27541 != GB_BQ_ALLOC)
	{
		Serial.println(F("Bad GG block allocation byte"));
		ret = 1;
		tmp = 1;
	}
	if (!tmp && Verbose)
		Serial.println(PASSED);

	// Verify blank EEPROM blocks
	if (Verbose)
		Serial.print(F("EEPROM/Auth block allocation: "));
	tmp = 0;  // Clear had an error flag
	for (dwCnt = 0; dwCnt < (Batts[BatteryType].UseAuthData ? 26 : 32); ++dwCnt)  // Check blocks
	{
		if (BD.Gifted.Block_Allocation_EEPROM & ((uint32_t)1 << dwCnt))
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
		Serial.println(PASSED);

	// Verify Lock Block
	cCSum = 0;
	tmp = 0;
	if (Verbose)
		Serial.print(F("Static factory block: "));
	// Start at the end of block and go backwards to work
	// around an optimization error cause by the compiler knowing
	// that BD.Gifted.Manufacture_ID is 8 bytes long and preventing
	// a loop cnt of 15 from being used.
	cpnt = (uint8_t *)&BD.Gifted.Checksum_y;  
	for (dwCnt = 0; dwCnt < 15; ++dwCnt)  // Check checksum
	{
		cCSum = cCSum + *(cpnt-dwCnt);
	}
	if (cCSum) // Bad checksum?
	{
			Serial.println(BAD_CHECKSUM);
			ret = 1;
			tmp = 1;
	}
	else // Check sum OK check ID
		for (dwCnt = 0; dwCnt < 14; ++dwCnt)  // loop thru block data
		{
			if (dwCnt < 8)
			{ // Check ID
				if (BD.Gifted.Manufacture_ID[dwCnt] != BD.Gifted.M200_Manf_ID[dwCnt])
				{
					Serial.println(F("Manufacture ID mismatch"));
					ret = 1;
					tmp = 1;
					break;
				}
			}
			else
			{ // Check for zero
				if (BD.Gifted.DummyData[dwCnt-8])
				{
					Serial.println(F("Manufacture ID Non-zero pad bytes"));
					ret = 1;
					tmp = 1;
					break;
				}
			}
		}

	if (!tmp && Verbose)
		Serial.println(PASSED);

	if (DesignCap > BD.Gifted.CC_Threshold)  // Use higher of design cap and CC_Threshold
		batteryRatedCapacity = DesignCap;
	else
		batteryRatedCapacity = BD.Gifted.CC_Threshold;

	// Verify Dynamic block 1
	tmp = 0;
	if (Verbose)
		Serial.print(F("Dynamic blocks: "));
	cpnt = (uint8_t *)&BD.Gifted.Dyn1Block[0];
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
	cpnt = (uint8_t *)&BD.Gifted.Dyn1Block[1];
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
		if (BD.Gifted.Dyn1Block[0].CopiedManufactureID[dwCnt] != BD.Gifted.M200_Manf_ID[dwCnt])
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
			if (0xff != BD.Gifted.Dyn1Block[0].CopiedManufactureID[dwCnt])
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
		if (BD.Gifted.Dyn1Block[1].CopiedManufactureID[dwCnt] != BD.Gifted.M200_Manf_ID[dwCnt])
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
			if (0xff != BD.Gifted.Dyn1Block[1].CopiedManufactureID[dwCnt])
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
		cpnt = (uint8_t *)&BD.Gifted.Dyn2Block[dwBcnt];
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
		Serial.println(PASSED);
	// Check extended temp charging data if it's there
	if (EX_CHARGE_TEMP_BLOCKS != (BD.Gifted.Block_Allocation_EEPROM & EX_CHARGE_TEMP_BLOCKS))  // Check if any of those blocks are marked as used
	{
		tmp = 0;
		if (Verbose)
			Serial.print(F("JEITA charging data: "));

		// Zero the random pad bytes
		BD.Gifted.ExCharge.pad[10] = 0;
		for (dwCnt = 0; dwCnt < 5; ++dwCnt)
			BD.Gifted.ExCharge.TempRange[dwCnt].pad[2] = 0;

		// Check the checksum
		cpnt = (uint8_t *)&BD.Gifted.ExCharge;
		cCSum = 0;
		for (dwCnt = 0; dwCnt < sizeof(EX_TEMP_CHARGING_EP_t); ++dwCnt)  // Check checksum
		{
			cCSum = cCSum + *(cpnt+dwCnt);
		}

		if (cCSum) // Bad checksum?
		{
			Serial.println(BAD_CHECKSUM);
			ret = 1;
			tmp = 1;
		}
		if (!tmp && Verbose)
			Serial.println(PASSED);

	}
	else if (Verbose)
		Serial.println(F("Note: No JEITA charging data"));

	// Check for SD660 block being used
	if (!(BD.Gifted.Block_Allocation_EEPROM & SD660_BLOCK))
	{  // block is used
		tmp = 0;
		if (Verbose)
			Serial.print(F("SD660 data: "));

		// Copy over unencrypted data
		memcpy((uint8_t *)&BD.Gifted+SD660blkaddr, Buf+SD660blkaddr, 16);

		// Check the checksum
		cpnt = (uint8_t *)&BD.Gifted;
		cpnt += SD660blkaddr; // Set to beginning of SD660 block
		cCSum = 0;

		for (dwCnt = 0; dwCnt < 16; ++dwCnt)  // Check checksum
		{
			cCSum = cCSum + *(cpnt+dwCnt);
		}
		if (cCSum) // Bad checksum?
		{
			Serial.println(BAD_CHECKSUM);
			ret = 1;
			tmp = 1;
		}

		if (!tmp && Verbose)
			Serial.println(PASSED);
	}
	else if (Verbose)
		Serial.println(F("Note: No SD660 data"));

	// Check for cell identity blocks being used
	if (!(BD.Gifted.Block_Allocation_EEPROM & CELL_IDENT_BLOCKS))
	{  // block is used
		if (Verbose)
			Serial.print(F("Cell identity data: "));
		tmp = 0;

		// Copy over unencrypted data
		memcpy((uint8_t *)&BD.Gifted+CELL_IDENT1blkaddr, Buf+CELL_IDENT1blkaddr, 32);

		// Check the checksum
		cpnt = (uint8_t *)&BD.Gifted;
		cpnt += CELL_IDENT1blkaddr; // Set to beginning of ident blocks
		cCSum = 0;

		tmp2 = 0;  // Clear using cell ident data flag
		for (dwCnt = 0; dwCnt < 32; ++dwCnt)  // Check checksum
		{
			cCSum = cCSum + *(cpnt+dwCnt);
			if (*(cpnt+dwCnt))
				tmp2 = 1;  // Set using cell ident data flag
		}
		if (cCSum) // Bad checksum?
		{
			Serial.println(BAD_CHECKSUM);
			ret = 1;
			tmp = 1;
		}

		if (!tmp && Verbose)
			Serial.println(PASSED);

		if (!tmp2 && Verbose)
			Serial.println(F("Note: No cell ident data found"));
	}
	else if (Verbose)
		Serial.println(F("Note: No cell identity data"));

	// Check for ship mode data being used
	if (!(BD.Gifted.Block_Allocation_EEPROM & SHIP_MODE_BLOCK))
	{  // block is used
		if (Verbose)
			Serial.print(F("Ship mode data: "));
		tmp = 0;

		// Copy over unencrypted data
		memcpy((uint8_t *)&BD.Gifted+SHIP_MODEblkaddr, Buf+SHIP_MODEblkaddr, 16);

		// Check the checksum
		cpnt = (uint8_t *)&BD.Gifted;
		cpnt += SHIP_MODEblkaddr; // Set to beginning of ship mode block
		cCSum = 0;

		tmp2 = 0;  // Clear using ship mode data flag
		for (dwCnt = 0; dwCnt < 7; ++dwCnt)  // Check checksum
		{
			cCSum = cCSum + *(cpnt+dwCnt);
			if (*(cpnt+dwCnt))
				tmp2 = 1;  // Set using ship mode data flag
		}

		if (cCSum) // Bad checksum?
		{
			Serial.println(BAD_CHECKSUM);
			ret = 1;
			tmp = 1;
		}

		if (!tmp && Verbose)
			Serial.println(PASSED);

		if (!tmp2 && Verbose)
			Serial.println(F("Note: No ship mode data found"));
	}
	else if (Verbose)
		Serial.println(F("Note: No ship mode data"));

	// Check for PP 660 data being used
	if (!(BD.Gifted.Block_Allocation_EEPROM & SD660_PP_DATA_BLOCK))
	{  // block is used
		if (Verbose)
			Serial.print(F("PP SD660 Data: "));
		tmp = 0;

		// Copy over unencrypted data
		memcpy((uint8_t *)&BD.Gifted+SD660_PP_DATAblkaddr, Buf+SD660_PP_DATAblkaddr, SD660_PP_DATAblkSize);

		// Check the checksum
		cpnt = (uint8_t *)&BD.Gifted;
		cpnt += SD660_PP_DATAblkaddr; // Set to beginning of PP SD660 data
		wCSum = (uint16_t)(*cpnt) + (uint16_t)(*(cpnt+1)<<8);
		for (dwCnt = 2; dwCnt < SD660_PP_DATAblkSize; ++dwCnt)  // Check checksum
			wCSum = wCSum + *(cpnt+dwCnt);

		if (wCSum) // Bad checksum?
		{
			Serial.println(BAD_CHECKSUM);
			ret = 1;
			tmp = 1;
		}

		if (!tmp && Verbose)
			Serial.println(PASSED);
	}
	else if (Verbose)
		Serial.println(F("Note: No PP SD660 Data"));

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
	{
		Serial.println();
		Serial.println(F("Validating Pollux PP data"));
	}

	// Check first checksum
	if (Verbose)
		Serial.print(F("0-239 checksum: "));
	csum = 0;
	for (cnt = 0; cnt <= 239; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check first aggregate charge block
	if (Verbose)
		Serial.print(F("First aggregate charge block: "));
	csum = 0;
	for (cnt = 240; cnt <= 244; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check second aggregate charge block
	if (Verbose)
		Serial.print(F("Second aggregate charge block: "));
	csum = 0;
	for (cnt = 248; cnt <= 252; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Charger control area
	if (Verbose)
		Serial.print(F("Charger control area: "));
	csum = 0;
	for (cnt = 256; cnt <= 279; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Start up voltage
	if (Verbose)
		Serial.print(F("Start up voltage: "));
	csum = 0;
	for (cnt = 280; cnt <= 282; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check 1725 data
	if (Verbose)
		Serial.print(F("1725 data: "));
	csum = 0;
	for (cnt = 288; cnt <= 295; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check health data
	if (Verbose)
		Serial.print(F("Health data: "));
	csum = 0;
	for (cnt = 296; cnt <= 303; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Buf[297] != 0)
	{
		Serial.print(F("Invalid record type in PP health data: 0x"));
		Serial.println(Buf[297],HEX);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Extended part number
	if (Verbose)
		Serial.print(F("Extended part number: "));
	csum = 0;
	for (cnt = 304; cnt <= 323; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Do 8956 area stuff if needed
	if (POLLUX8956_BATT_DATA == Format)
	{
		if (Verbose)
			Serial.print(F("8956 data: "));
		csum = 0;
		for (cnt = 352; cnt <= 480; ++cnt)
			csum += Buf[cnt];

		if (csum)
		{
			Serial.println(BAD_CHECKSUM);
			ret = 1;
		}
		else if (Verbose)
			Serial.println(PASSED);
	}

	// Fast charge area
	if (Verbose)
		Serial.print(F("Fast charge data: "));
	csum = 0;
	for (cnt = 504; cnt <= 511; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	PP_Valid = 1;  // Set data to valid
	return (ret);
}


// Checkout MPA2 data
uint8_t ValidatePP(uint8_t *Buf)
{
	uint16_t cnt;
	uint8_t csum, ret=0;

	PP_Valid = 0;  // Set data to not valid

	if (Verbose)
	{
		Serial.println();
		Serial.println(F("Check MPA2 PP data"));
	}

	// Check first checksum
	if (Verbose)
		Serial.print(F("0-156 Checksum: "));
	csum = 0;
	for (cnt = 0; cnt <= 156; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check first aggregate charge block
	if (Verbose)
		Serial.print(F("First battery life block: "));
	csum = 0;
	for (cnt = 160; cnt <= 164; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check second aggregate charge block
	if (Verbose)
		Serial.print(F("Second battery life block: "));
	csum = 0;
	for (cnt = 168; cnt <= 172; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check 1725 data
	if (Verbose)
		Serial.print(F("1725 Data: "));
	csum = 0;
	for (cnt = 208; cnt <= 215; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check health data
	if (Verbose)
		Serial.print(F("Health Data: "));
	csum = 0;
	for (cnt = 216; cnt <= 221; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	PP_Valid = 1;  // Set data to valid
	return (ret);
}

// Checkout Hawkeye data
uint8_t ValidateHawkeye(uint8_t *Buf)
{
	uint16_t cnt;
	uint8_t csum, ret=0;

	PP_Valid = 0;  // Set data to not valid

	if (Verbose)
	{
		Serial.println();
		Serial.println(F("Validate QC PP data"));
	}

	// Check first checksum
	if (Verbose)
		Serial.print(F("0-199 Checksum: "));
	csum = 0;
	for (cnt = 0; cnt <= 199; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check first aggregate charge block
	if (Verbose)
		Serial.print(F("First aggregate charge block: "));
	csum = 0;
	for (cnt = 200; cnt <= 204; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check second aggregate charge block
	if (Verbose)
		Serial.print(F("Second aggregate charge block: "));
	csum = 0;
	for (cnt = 208; cnt <= 212; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Charger control area
	if (Verbose)
		Serial.print(F("Charger control data: "));
	csum = 0;
	for (cnt = 216; cnt <= 239; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check startup voltage
	if (Verbose)
		Serial.print(F("Startup voltage: "));
	csum = 0;
	for (cnt = 240; cnt <= 242; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check 1725 data
	if (Verbose)
		Serial.print(F("1725 Data: "));
	csum = 0;
	for (cnt = 244; cnt <= 251; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check health data
	if (Verbose)
		Serial.print(F("Health Data: "));
	csum = 0;
	for (cnt = 252; cnt <= 259; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// JEITA data
	if (Verbose)
		Serial.print(F("JEITA Data: "));
	csum = 0;
	for (cnt = 288; cnt <= 383; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	PP_Valid = 1;  // Set data to valid
	return (ret);
}

// Checkout value tier data
uint8_t ValidateVT(VALUE_TIER_BATT_DATA_t *VT)
{
	uint16_t cnt;
	uint8_t csum, ret=0;
	uint8_t *Buf;

	VT_Valid = 0;  // Set data to not valid

	if (Verbose)
	{
		Serial.println();
		Serial.println(F("Validate VT data"));
	}

	// Swap capacity bytes
	VT->ManfCapacity = (VT->ManfCapacity >> 8) | (VT->ManfCapacity << 8);

	// Verify main checksum
	if (Verbose)
		Serial.print(F("Main checksum (0-431): "));

	csum = 0;
	Buf = (uint8_t *)VT;
	for (cnt = 0; cnt <= 431; ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check revision
	if (Verbose)
		Serial.print(F("Format revision: "));

	if (VT->FormatRev < 1 || VT->FormatRev > 2)
	{
		Serial.print(F("Unsupported format revision: "));
		Serial.println(VT->FormatRev);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Verify second DTSI block checksum
	if (Verbose)
		Serial.print(F("Second DTSI block checksum: "));

	if (VT->FormatRev != 1)  // Do we need to check this? (Not supported in rev 1 format)
	{
		csum = VT->ASD2_Checksum;  // Start with block checksum
		for (cnt = 0; cnt < VT_ASD2_SIZE; ++cnt)
			csum += VT->ASD2[cnt];

		if (csum)
		{
			Serial.println(BAD_CHECKSUM);
			ret = 1;
		}

		if (!csum && Verbose)
			Serial.println(PASSED);
	}
	else if (Verbose)
		Serial.println(F("Skipped (Not supported in Rev 1 format)"));

	// Check charger control data
	if (Verbose)
		Serial.print(F("Charger control data: "));

	csum = 0;
	Buf = (uint8_t *)&VT->ChargerCtrl;
	for (cnt = 0; cnt < sizeof(CHARGER_CTRL_VT_t); ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check JEITA data
	if (Verbose)
		Serial.print(F("JEITA data: "));

	csum = 0;
	Buf = (uint8_t *)&VT->JEITA;
	for (cnt = 0; cnt < sizeof(EX_TEMP_CHARGING_EP_VT_t); ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check cell ident data
	if (Verbose)
		Serial.print(F("Cell ident data: "));

	csum = 0;
	Buf = (uint8_t *)&VT->Ident;
	for (cnt = 0; cnt < sizeof(CELL_IDENT_t); ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check first health block
	if (Verbose)
		Serial.print(F("First health block: "));

	csum = 0;
	Buf = (uint8_t *)&VT->Health1;
	for (cnt = 0; cnt < sizeof(BATT_HEALTH_VT_t); ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check second health block
	if (Verbose)
		Serial.print(F("Second health block: "));

	csum = 0;
	Buf = (uint8_t *)&VT->Health2;
	for (cnt = 0; cnt < sizeof(BATT_HEALTH_VT_t); ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check first aggregate charge block
	if (Verbose)
		Serial.print(F("First aggregate charge block: "));

	csum = 0;
	Buf = (uint8_t *)&VT->AggCharge1;
	for (cnt = 0; cnt < sizeof(AGG_CHARGE_VT_t); ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check Second aggregate charge block
	if (Verbose)
		Serial.print(F("Second aggregate charge block: "));

	csum = 0;
	Buf = (uint8_t *)&VT->AggCharge2;
	for (cnt = 0; cnt < sizeof(AGG_CHARGE_VT_t); ++cnt)
		csum += Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	VT_Valid = 1;  // Set data to valid
	return (ret);
}

// Checkout PP+ V2 data
uint8_t ValidatePPP_V2(void)
{
	uint16_t cnt;
	uint8_t csum, ret=0, *pnt;

	PPPV2_Valid = 0;  // Set data to not valid

	if (Verbose)
	{
		Serial.println();
		Serial.println(F("Validate PP+ V2 data"));
	}

	// Verify block 1 checksum
	if (Verbose)
		Serial.print(F("Block 1 checksum (0-51): "));

	csum = 0;
	for (cnt = 0; cnt <= 51; ++cnt)
		csum += BD.Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check revision
	if (Verbose)
		Serial.print(F("Block 1 format revision: "));

	if (BD.NG.B1FormatRev < 1 || BD.NG.B1FormatRev > 1)
	{
		Serial.print(F("Unsupported format revision: "));
		Serial.println(BD.NG.B1FormatRev);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Verify block 2 checksum
	if (Verbose)
		Serial.print(F("Block 2 checksum (52-147): "));

	csum = 0;
	for (cnt = 52; cnt <= 147; ++cnt)
		csum += BD.Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check revision
	if (Verbose)
		Serial.print(F("Block 2 format revision: "));

	if (BD.NG.B2FormatRev < 1 || BD.NG.B2FormatRev > 1)
	{
		Serial.print(F("Unsupported format revision: "));
		Serial.println(BD.NG.B2FormatRev);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Verify block 3 checksum
	if (Verbose)
		Serial.print(F("Block 3 checksum (200-631): "));

	csum = 0;
	for (cnt = 200; cnt <= 631; ++cnt)
		csum += BD.Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check revision
	if (Verbose)
		Serial.print(F("Block 3 format revision: "));

	if (BD.NG.B3FormatRev < 1 || BD.NG.B3FormatRev > 1)
	{
		Serial.print(F("Unsupported format revision: "));
		Serial.println(BD.NG.B3FormatRev);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check JEITA revision
	if (Verbose)
		Serial.print(F("JEITA Data type: "));

	if (BD.NG.JEITA.DataType < 1 || BD.NG.JEITA.DataType > 1)
	{
		Serial.print(F("Unsupported data type: "));
		Serial.println(BD.NG.JEITA.DataType);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check first aggregate charge block
	if (Verbose)
		Serial.print(F("First aggregate charge block: "));

	csum = 0;
	for (cnt = 148; cnt < 156; ++cnt)
		csum += BD.Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check Second aggregate charge block
	if (Verbose)
		Serial.print(F("Second aggregate charge block: "));

	csum = 0;
	for (cnt = 156; cnt < 164; ++cnt)
		csum += BD.Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check initial times block
	if (Verbose)
		Serial.print(F("Initial Times block: "));

	csum = 0;
	for (cnt = 164; cnt < 196; ++cnt)
		csum += BD.Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check health block
	if (Verbose)
		Serial.print(F("Health block: "));

	csum = 0;
	for (cnt = 196; cnt < 200; ++cnt)
		csum += BD.Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	// Check cap drop block
	if (Verbose)
		Serial.print(F("Capacity Drop Data block: "));

	csum = 0;
	for (cnt = 204; cnt < 244; ++cnt)
		csum += BD.Buf[cnt];

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else
	{
		if (BD.NG.CapDrop.Format < 0 || BD.NG.CapDrop.Format > 0)
		{
			Serial.print(F("Unsupported data format: "));
			Serial.println(BD.NG.CapDrop.Format);
			ret = 1;
		}
		else if (Verbose)
			Serial.println(PASSED);
	}

	// Check intial data block
	if (Verbose)
		Serial.print(F("Initial data: "));

	pnt = (uint8_t *)&BD.NG.InitData;
	csum = 0;
	for (cnt = 0; cnt < 11; ++cnt)
		csum += *pnt++;

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else
	{
		if (BD.NG.InitData.Rev < 1 || BD.NG.InitData.Rev > 1)
		{
			Serial.print(F("Unsupported data format: "));
			Serial.println(BD.NG.InitData.Rev);
			ret = 1;
		}
		else if (Verbose)
			Serial.println(PASSED);
	}

// ***FIX*** Get format from Brad!!!
	// Check cell ident data
	if (Verbose)
		Serial.print(F("Cell Ident data: "));

	csum = 0;
	for (cnt = 11; cnt < 96; ++cnt)
		csum += *pnt++;

	if (csum)
	{
		Serial.println(BAD_CHECKSUM);
		ret = 1;
	}
	else if (Verbose)
		Serial.println(PASSED);

	PPPV2_Valid = 1;  // Set data to valid
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