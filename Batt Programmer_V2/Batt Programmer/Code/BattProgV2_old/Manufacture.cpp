// Battery manufacture stuff

#include <Arduino.h>
#include "Adafruit_SPIFlash.h"

#include "pwmbatt.h"
#include "BattProg.h"
#include "aes.h"
#include "GasGauge.h"
#include "Auth.h"

// ***FIX***
#define UNUSED(x) (void)(x)

// Manufacture PP+ data
uint8_t ManufacturePPP(uint8_t *Buf)
{
// ***FIX***
UNUSED(Buf);
	return (1);
}

// Manufacture Pollux data
uint8_t ManufacturePollux(uint8_t *Buf, uint8_t Format)
{
// ***FIX***
UNUSED(Buf);
UNUSED(Format);
	// Get needed data

	return (1);
}

// Manufacture Hawkeye data
uint8_t ManufactureHawkeye(uint8_t *Buf)
{
// ***FIX***
UNUSED(Buf);
	return (1);
}

// Function to set the serial number/date/etc. into a V2 PP+ batt
void ManufV2(void)
{
	uint16_t SN, date, year, month, day;
	uint8_t name[32], cnt, csum, *pnt;
	char buf[12];
	uint8_t regs[NEW_GB_CMD_FULL_READ_SIZE];

	STATUS_LED_BUSY
	// Get the current serial number
	if (!GG_BlockRead(NGG_SERIAL_NUM,(uint8_t *)&SN,2))
	{
		STATUS_LED_READY
		return;
	}

	// Get the current manuf name
	if (!GG_BlockRead(NGG_MANF_NAME,name))
	{
		STATUS_LED_READY
		return;
	}

	// Show the current serial number
	Serial.printf("Current serial number: %c%04d\r\n",name[0],SN);

	// Get the new number
	Serial.print("Enter new serial number: ");
	buf[0] = 0;  // Terminate buffer
	GetText(buf,5);

	// Validate number
	if (strlen(buf) != 5)
	{
		Serial.print("SM must be 5 chars!");
		STATUS_LED_READY
		return;
	}
	if (buf[0] < 'A' || buf[0] > 'Z')
	{
		Serial.print("First char must be an uppercase letter!");
		STATUS_LED_READY
		return;
	}
	for (cnt = 1; cnt < 5; ++cnt)
		if (buf[cnt] < '0' || buf[cnt] > '9')
		{
			Serial.print("Last 4 chars must be numbers!");
			STATUS_LED_READY
			return;
		}

	// Make up new serial number/manf name data;
	sscanf(buf+1,"%hd",&SN);
	name[2] = SN & 0xff;
	name[3] = (SN >> 8) & 0xff;
	name[4] = 1;  // Name string length
	name[5] = buf[0];
	for (cnt = 6; cnt < 25; ++cnt)
		name[cnt] = 0;

	// Get the current date
	if (!GG_BlockRead(NGG_MANF_DATE,(uint8_t *)&date,2))
	{
		STATUS_LED_READY
		return;
	}

	// Show the current date
	month = (date&0x1e0)>>5;
	day = date&0x1f;
	year = (date>>9)+1980;
	Serial.printf("Manufacture date: %d/%d/%d\r\n",month,day,year);

	// Get the new number
	Serial.print("Enter new date: ");
	buf[0] = 0;  // Terminate buffer
	GetText(buf,10,1);

	// Validate date
	sscanf(buf,"%hd/%hd/%hd",&month,&day,&year);
	if (month < 1 || month > 12)
	{
		Serial.println("Invalid month!");
		STATUS_LED_READY
		return;
	}
	if (day < 1 || day > 31)
	{
		Serial.println("Invalid day!");
		STATUS_LED_READY
		return;
	}
	if (year < 2020 || year > 2107)
	{
		Serial.println("Invalid year!");
		STATUS_LED_READY
		return;
	}

	// Make up new date
	date = day + (month << 5) + ((year-1980)<<9);
	name[0] = date & 0xff;
	name[1] = (date >> 8) & 0xff;

	// Get initial data
	Serial.println("Get initial data");
	if (newGG_ReadReg(regs))
	{
		STATUS_LED_READY
		return;
	}

	STATUS_LED_BUSY  // newGG_ReadReg() sets it to green when done

	BD.NG.InitData.Voltage = regs[NGG_VOLTAGE] | (regs[NGG_VOLTAGE+1] << 8);
	BD.NG.InitData.RSOC = regs[NGG_RSOC];
	BD.NG.InitData.Rev = 1;

	// Unseal the gauge
	Serial.println("Unsealing");
	if (GG_FullUnseal())
	{
		STATUS_LED_READY
		return;
	}

	// Get temp range data
	GG_BlockRead(NGG_TEMP_RANGES_ADD,BD.NG.InitData.Temps,6);

	// Do initial data checksum
	csum = 0;
	pnt = (uint8_t *)&BD.NG.InitData;
	++pnt;
	for (cnt = 1; cnt < 11; ++cnt)
		csum += *pnt++;
	BD.NG.InitData.Checksum = ~csum + 1;

	// Flush cell ident data
	for (cnt = 0; cnt < 84; ++cnt)
		BD.NG.CellIdentData[cnt] = 0;
	BD.NG.CellIdentCsum = 0; // // Flush checksum byte

	// Write out date and SN
	Serial.println("Write date/SN");
	GG_BlockWrite(NGG_MANF_DATE_ADD,name,25);
	delay(100);

	Serial.print("Write manf blocks ");
	// Write out the manuf blocks
	pnt = (uint8_t *)&BD.NG.InitData;
	GG_BlockWrite(NGG_MANF_BLKA_ADD,pnt,32);
	delay(200);
	Serial.print(".");
	pnt += 32;
	GG_BlockWrite(NGG_MANF_BLKB_ADD,pnt,32);
	delay(200);
	Serial.print(".");
	pnt += 32;
	GG_BlockWrite(NGG_MANF_BLKC_ADD,pnt,32);
	delay(200);
	Serial.println(".");

	// Set I2C drive strength
	Serial.println("Setting drive strength");
	buf[0] = 0x18;
	GG_BlockWrite(0x807f,(uint8_t *)buf,1);
	delay(100);
	buf[0] = 42; // Make sure it don't match!
	if (!GG_BlockRead(0x807f,(uint8_t *)buf,1))
		Serial.println("Error checking drive strength!");
	if (buf[0] != 0x18)
		Serial.println("Error setting drive strength!");

	Serial.println("Sealing");
	GG_Seal();
	Serial.println("Done!   Running VBD command to check battery is ok");

	// Do validate
	FlushBuffer(0);
	AUTH_Read(BD.Buf);
	if ((cnt = Validate_AUTH(PPP_V2_DATA)))
		CheckAuthChip();
	else
		cnt = CheckAuthChip();
	Serial.println();
	if (!cnt)
		Serial.println(BattOK);
	else
		Serial.println(FailedVal);

	STATUS_LED_READY
}
