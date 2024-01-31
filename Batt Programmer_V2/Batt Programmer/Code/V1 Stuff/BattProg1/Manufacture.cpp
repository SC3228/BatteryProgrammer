// Battery manufacture stuff

#include <Arduino.h>

#include "pwmbatt.h"
#include "BattProg.h"
#include "aes.h"
#include "F:\Includes\ArduinoKeys.h"
#include "GasGauge.h"
#include "Auth.h"

// Manufacture PP+ data
uint8_t ManufacturePPP(uint8_t *Buf)
{
	return (1);
}

// Manufacture Pollux data
uint8_t ManufacturePollux(uint8_t *Buf, uint8_t Format)
{
	// Get needed data

	return (1);
}

// Manufacture Hawkeye data
uint8_t ManufactureHawkeye(uint8_t *Buf)
{
	return (1);
}

// Function to set the serial number/date/etc. into a V2 PP+ batt
void ManufV2(void)
{
	uint16_t SN, date, year, month, day;
	uint8_t name[32], cnt, csum, *pnt;
	uint8_t buf[12];
	uint8_t regs[NEW_GB_CMD_FULL_READ_SIZE];

	// Get the current serial number
	if (!GG_BlockRead(NGG_SERIAL_NUM,(uint8_t *)&SN,2))
		return;

	// Get the current manuf name
	if (!GG_BlockRead(NGG_MANF_NAME,name))
		return;

	// Show the current serial number
	sprintf(buf,"%c%04d",name[0],SN);
	Serial.print(F("Current serial number: "));
	Serial.println((char *)buf);

	// Get the new number
	Serial.print(F("Enter new serial number: "));
	buf[0] = 0;  // Terminate buffer
	GetText(buf,5);

	// Validate number
	if (strlen(buf) != 5)
	{
		Serial.print(F("SM must be 5 chars!"));
		return;
	}
	if (buf[0] < 'A' || buf[0] > 'Z')
	{
		Serial.print(F("First char must be an uppercase letter!"));
		return;
	}
	for (cnt = 1; cnt < 5; ++cnt)
		if (buf[cnt] < '0' || buf[cnt] > '9')
		{
			Serial.print(F("Last 4 chars must be numbers!"));
			return;
		}

	// Make up new serial number/manf name data;
	sscanf(buf+1,"%d",&SN);
	name[2] = SN & 0xff;
	name[3] = (SN >> 8) & 0xff;
	name[4] = 1;  // Name string length
	name[5] = buf[0];
	for (cnt = 6; cnt < 25; ++cnt)
		name[cnt] = 0;

	// Get the current date
	if (!GG_BlockRead(NGG_MANF_DATE,(uint8_t *)&date,2))
		return;

	// Show the current date
	month = (date&0x1e0)>>5;
	day = date&0x1f;
	year = (date>>9)+1980;
	sprintf(buf,"%d/%d/%d",month,day,year);
	Serial.print(F("Manufacture date: "));
	Serial.println((char *)buf);

	// Get the new number
	Serial.print(F("Enter new date: "));
	buf[0] = 0;  // Terminate buffer
	GetText(buf,10,1);

	// Validate date
	sscanf(buf,"%d/%d/%d",&month,&day,&year);
	if (month < 1 || month > 12)
	{
		Serial.println(F("Invalid month!"));
		return;
	}
	if (day < 1 || day > 31)
	{
		Serial.println(F("Invalid day!"));
		return;
	}
	if (year < 2020 || year > 2107)
	{
		Serial.println(F("Invalid year!"));
		return;
	}

	// Make up new date
	date = day + (month << 5) + ((year-1980)<<9);
	name[0] = date & 0xff;
	name[1] = (date >> 8) & 0xff;

	// Get initial data
	Serial.println(F("Get initial data"));
	if (newGG_ReadReg(regs))
		return;

	BD.NG.InitData.Voltage = regs[NGG_VOLTAGE] | (regs[NGG_VOLTAGE+1] << 8);
	BD.NG.InitData.RSOC = regs[NGG_RSOC];
	BD.NG.InitData.Rev = 1;

	// Unseal the gauge
	Serial.println(F("Unsealing"));
	if (GG_FullUnseal())
		return;

	// Get temp range data
	GG_BlockRead(NGG_TEMP_RANGES_ADD,BD.NG.InitData.Temps,6);

	// Do initial data checksum
	csum = 0;
	pnt = (uint8_t *)&BD.NG.InitData;
	++pnt;
	for (cnt = 1; cnt < 11; ++cnt)
		csum += *pnt++;
	BD.NG.InitData.Checksum = ~csum + 1;

	// flush the rest of the manuf blocks ***FIX***
	for (cnt = 0; cnt < 85; ++cnt)
		BD.NG.Pad3[cnt] = 0;

	// Write out date and SN
	Serial.println(F("Write date/SN"));
	GG_BlockWrite(NGG_MANF_DATE_ADD,name,25);
	delay(100);

	Serial.print(F("Write manf blocks "));
	// Write out the manuf blocks
	pnt = (uint8_t *)&BD.NG.InitData;
	GG_BlockWrite(NGG_MANF_BLKA_ADD,pnt,16);
	delay(200);
	Serial.print(F("."));
	pnt += 16;
	GG_BlockWrite(NGG_MANF_BLKA_ADD+16,pnt,16);
	delay(200);
	Serial.print(F("."));
	pnt += 16;
	GG_BlockWrite(NGG_MANF_BLKB_ADD,pnt,16);
	delay(200);
	Serial.print(F("."));
	pnt += 16;
	GG_BlockWrite(NGG_MANF_BLKB_ADD+16,pnt,16);
	delay(200);
	Serial.print(F("."));
	pnt += 16;
	GG_BlockWrite(NGG_MANF_BLKC_ADD,pnt,16);
	delay(200);
	Serial.print(F("."));
	pnt += 16;
	GG_BlockWrite(NGG_MANF_BLKC_ADD+16,pnt,16);
	delay(200);
	Serial.println(F("."));

	// Set I2C drive strength
	Serial.println(F("Setting drive strength"));
	buf[0] = 0x18;
	GG_BlockWrite(0x807f,buf,1);
	delay(100);
	buf[0] = 42; // Make sure it don't match!
	if (!GG_BlockRead(0x807f,buf,1))
		Serial.println(F("Error checking drive strength!"));
	if (buf[0] != 0x18)
		Serial.println(F("Error setting drive strength!"));

	Serial.println(F("Sealing"));
	GG_Seal();
	Serial.println(F("Done!   Running VBD command to check battery is OK"));

	// Do validate
	FlushBuffer(0);
	AUTH_Read(BD.Buf);
	if (cnt = Validate_AUTH(PPP_V2_DATA))
		CheckAuthChip();
	else
		cnt = CheckAuthChip();
	Serial.println();
	if (!cnt)
		Serial.println(BATT_OK);
	else
		Serial.println(FAILED_VAL);
}
