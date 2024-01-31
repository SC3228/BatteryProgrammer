// Code to display battery data

#include <Arduino.h>
#include "Adafruit_SPIFlash.h"

#include "pwmbatt.h"
#include "Batts.h"
#include "BattProg.h"
#include "GasGauge.h"

// MPA2 battery part number stuff
#define DEFAULT_BATTERY_PREFIX_NOS		82		// Default prefix for battery family numbers

#ifdef ZEBRA_MODE

void ShowMPA2(void)
{
	uint16_t Year;
	SMART_BATT_DATA_t *SB = (SMART_BATT_DATA_t *)EEPROM_Data;
	// Used in part number conversions
	uint8_t dwBattPrefixNo = DEFAULT_BATTERY_PREFIX_NOS;
	uint32_t dwBattFamilyNo;

	if (!PP_Valid)
	{
		Serial.println("No valid PP data to show");
		return;
	}

	// Show rev
	if (4 != SB->BattDataType)
	{
		Serial.println("Unknown data type!!!");
		return;
	}
	Serial.print("DataType: ");
	Serial.println(SB->BattDataType);

	// Show part/serial numbers
	//Battery Family Number
	dwBattFamilyNo = SB->BattPartNum/10000;
	switch (dwBattFamilyNo)
	{
	case 60112:
	case 60117:
	case 166:
		dwBattPrefixNo = 55;
		break;
	case 65587:
	case 61261:
	case 62960:
		dwBattPrefixNo = 21;
		break;
	}

	// Find out the correct battery family number lookup table by searching the corresponding incorrect battery family number.
	switch (dwBattFamilyNo)
	{
	case 27909:
		dwBattFamilyNo = 127909;
		break;
	case 27912:
		dwBattFamilyNo = 127912;
		break;
	case 7172:
		dwBattFamilyNo = 107172;
		break;
	case 1094:
		dwBattFamilyNo = 111094;
		break;
	case 1606:
		dwBattFamilyNo = 101606;
		break;
	case 11734:
		dwBattFamilyNo = 111734;
		break;
	case 11636:
		dwBattFamilyNo = 111636;
		break;
	case 5:
		dwBattFamilyNo = 90005;
		break;
	}

	// See 70-62284-01 Smart Programming Guide for format of part number stored in E2
	// Typical data in battery E2: 713630101 or 1279090121
	// Typical data format to convert to: 82-71363-01 Rev.01 or 82-127909-01 Rev. A resp.
	// From Document 70-62284-01: 	If the last 2 digits <  21 then numeric revision 00 to 20
	//								If the last 2 digits >= 21 then numeric alpha, 21=A, 22=B etc.
	Serial.print("Part number: ");
	if (SB->BattPartNum%100 < 21)
		Serial.printf("%02d-%06ld-%02ld Rev.%02ld", dwBattPrefixNo, dwBattFamilyNo, (SB->BattPartNum/100)%100, SB->BattPartNum%100);
	else
		Serial.printf("%02d-%06ld-%02ld Rev. %c", dwBattPrefixNo, dwBattFamilyNo, (SB->BattPartNum/100)%100, (char)(SB->BattPartNum%100-21+'A'));

	Serial.printf("  Serial Num: %d\r\n",SB->BattID);

	// Date made
	Serial.print("Date Made: ");
	Serial.print(SB->BattDate[2]>>4);
	Serial.print("/");
	Serial.print(SB->BattDate[0]);
	Serial.print("/");
	Year = ((SB->BattDate[2] & 0x0f) << 8) | SB->BattDate[1];
	Serial.println(Year);

	// Capacities
	Serial.print("Capacity: ");
	Serial.print(SB->BattRated);
	Serial.print("maHr   Faire: ");
	Serial.print(SB->BattFaireLevel);
	Serial.print("%   Unfaire: ");
	Serial.print(SB->BattUnfaireLevel);
	Serial.print("%   Low: ");
	Serial.print(SB->BattLowLevel);
	Serial.println("%");

	// Charger control block
	Serial.println("Charger control Data");
	Serial.print("    Slow Chrg TO: ");
	Serial.print(SB->SlowChrgTO);
	Serial.print("  Fast Chrg TO: ");
	Serial.println(SB->FastChrgTO);
	Serial.print("    Slow/Fast Volt: ");
	Serial.print(SB->SlowFastThreshold);
	Serial.print("mv  Recharge Volt: ");
	Serial.print(SB->RechargeVoltage);
	Serial.print("mv");
	Serial.print("mv  Absent Batt Volt: ");
	Serial.print(SB->AbsentBattVoltage);
	Serial.println("mv");
	Serial.print("    Abnormal Curr: ");
	Serial.print(SB->AbnormalCurrent);
	Serial.print("ma  SC High Fault: ");
	Serial.print(SB->SlowChrgHighFC);
	Serial.print("ma  SC Low Fault: ");
	Serial.print(SB->SlowChrgLowFC);
	Serial.println("ma");
	Serial.print("    Nearly Done: ");
	Serial.print(SB->NearlyDoneCurrent);
	Serial.print("ma  Done: ");
	Serial.print(SB->DoneCurrent);
	Serial.println("ma");

	// Agg charge
	Serial.print("Agg charge 1: ");
	Serial.print(SB->BattLife1);
	Serial.print("maHrs   Agg charge 2: ");
	Serial.print(SB->BattLife2);
	Serial.println("maHrs");

	// Show 1725 data
	if (SB->_1725_DataType != 1)
		Serial.println("Unknown 1725 data type!!!");
	else
	{
		Serial.println("1725 Data");
		Serial.print("    Discharge Temp Min: ");
		Serial.print(SB->BattMinDTemp);
		Serial.print("C   Max: ");
		Serial.print(SB->BattMaxDTemp);
		Serial.println("C");
		Serial.print("    Max charge volt: ");
		Serial.print(SB->BattMaxCVolt);
		Serial.print("mv (Ignored)   Fast chrg curr: ");
		Serial.print(SB->BattFastCurr);
		Serial.println("ma");
	}

	// Health records
	if (SB->Health_DataType != 1)
		Serial.println("Unknown health data type!!!");
	else
	{
		Serial.print("Date of first use: ");
		Serial.print(SB->DateFirstUse[2]>>4);
		Serial.print("/");
		Serial.print(SB->DateFirstUse[0]);
		Serial.print("/");
		Year = ((SB->DateFirstUse[2] & 0x0f) << 8) | SB->DateFirstUse[1];
		Serial.print(Year);
		Serial.print("   Health byte: ");
		Serial.println(SB->BattHealth);
	}
}

// Show PP+ V2 data
void ShowPPPV2(void)
{
	uint8_t cnt, cnt1;
	uint16_t Year;

	if (!PPPV2_Valid)
	{
		Serial.println("No valid PPP V2 data to show");
		return;
	}

	// Show revs
	Serial.print("Block 1 Rev: ");
	Serial.print(BD.NG.B1FormatRev);
	Serial.print("   Block 2 Rev: ");
	Serial.print(BD.NG.B2FormatRev);
	Serial.print("   Block 3 Rev: ");
	Serial.println(BD.NG.B3FormatRev);

	// Show part/serial numbers
	Serial.print("Part number: ");
	Serial.print((char *)BD.NG.PartNumber);
	Serial.print("   Serial Num: ");
	Serial.printf("%c%04d",BD.NG.ManufName[0],BD.NG.SerialNum);

	// Date made
	Serial.print("   Date Made: ");
	Serial.print((BD.NG.DateMade&0x1e0)>>5);
	Serial.print("/");
	Serial.print(BD.NG.DateMade&0x1f);
	Serial.print("/");
	Year = ((BD.NG.DateMade>>9)+1980);
	Serial.println(Year);

	// Capacities
	Serial.print("Capacity: ");
	Serial.print(BD.NG.ManfCapacity_ma);
	Serial.print("maHr   ");
	Serial.print(BD.NG.ManfCapacity_mw);
	Serial.print("mwHr   Low: ");
	Serial.print(BD.NG.BatteryLow);
	Serial.print("%   Very Low: ");
	Serial.print(BD.NG.BatteryVeryLow);
	Serial.print("%   Critical: ");
	Serial.print(BD.NG.BatteryCritical);
	Serial.println("%");

	// Show 1725 data
	Serial.print("Discharge Temp Min: ");
	Serial.print(BD.NG._1725DiscLowLimit);
	Serial.print("C   Max: ");
	Serial.print(BD.NG._1725DiscHighLimit);
	Serial.println("C");

	// Charger control block
	Serial.println();
	Serial.println("Charger control Data");
	Serial.print("    Slow Chrg TO: ");
	Serial.print(BD.NG.ChargerCtrl.SlowChrgTimeout);
	Serial.print("  Fast Chrg TO: ");
	Serial.print(BD.NG.ChargerCtrl.FastChrgTimeout);
	Serial.print("  Slow/Fast Volt: ");
	Serial.print(BD.NG.ChargerCtrl.SlowFastVolt);
	Serial.print("mv  Recharge Volt Delta: ");
	Serial.print(BD.NG.ChargerCtrl.RechargeVoltDelta);
	Serial.println("mv");
	Serial.print("    Abnormal Curr: ");
	Serial.print(BD.NG.ChargerCtrl.AbnormalCurr);
	Serial.print("ma  Slow Charge Curr: ");
	Serial.print(BD.NG.ChargerCtrl.SlowChrgCurr);	
	Serial.print("  Nearly Done: ");
	Serial.print(BD.NG.ChargerCtrl.NearlyDoneCurr);
	Serial.print("ma  Fallback Term Curr: ");
	Serial.print(BD.NG.ChargerCtrl.FallBackTermCurr);
	Serial.println("ma");
	Serial.print("    Hysteresis: ");
	Serial.println(BD.NG.ChargerCtrl.Hysteresis);

	// Ship mode
	Serial.println();
	// Is there data?
	if (BD.NG.ShipMode.LowVoltage != 0
		|| BD.NG.ShipMode.HighVoltage != 0
		|| BD.NG.ShipMode.LowCapacity != 0
		|| BD.NG.ShipMode.HighCapacity != 0)
	{  // Yup
		Serial.print("Ship mode Voltage Low: ");
		Serial.print(BD.NG.ShipMode.LowVoltage);
		Serial.print("mv  Voltage High: ");
		Serial.print(BD.NG.ShipMode.HighVoltage);
		Serial.print("mv  Capacity Low: ");
		Serial.print(BD.NG.ShipMode.LowCapacity);
		Serial.print("%  Capacity High: ");
		Serial.print(BD.NG.ShipMode.HighCapacity);
		Serial.println("%");
	}
	else // Nope
		Serial.println("No ship mode data found");

	// 660 Data
	Serial.println();
	Serial.println("660 Data Block");
	Serial.print("    Battery Flags: ");
	Serial.println(BD.NG._660Data.ElimFlag);
	Serial.print("    Min startup volt: ");
	Serial.print(BD.NG._660Data.MinStartupVolt);
	Serial.print("  Therm cof: 0x");
	Serial.print(BD.NG._660Data.ThermCoff[2],HEX);
	Serial.print(BD.NG._660Data.ThermCoff[1],HEX);
	Serial.println(BD.NG._660Data.ThermCoff[0],HEX);
	Serial.print("    UVLO: ");
	Serial.print(BD.NG._660Data.UVLO);
	Serial.print("mv  OVLO: ");
	Serial.print(BD.NG._660Data.OVLO);
	Serial.print("mv  Cutoff: ");
	Serial.print(BD.NG._660Data.CutoffVolt);
	Serial.println("mv");

	// JEITA data
	Serial.println();
	Serial.println("JEITA Data block");
	Serial.print("    Data type: ");
	Serial.print(BD.NG.JEITA.DataType);
	Serial.print("   Stop temp: ");
	Serial.print(BD.NG.JEITA.StopTemp);
	Serial.print("   Rec min temp: ");
	Serial.print(BD.NG.JEITA.RecMinTemp);
	Serial.print("   Rec max temp: ");
	Serial.println(BD.NG.JEITA.RecMaxTemp);

	for (cnt = 0; cnt < 5; ++cnt)
	{
		Serial.print("    Range");
		Serial.print(cnt);
		Serial.print(":  Start temp: ");
		Serial.print(BD.NG.JEITA.TempRange[cnt].StartTemp);

		for (cnt1 = 0; cnt1 < 3; ++cnt1)
		{
			Serial.print("   V");
			Serial.print(cnt1);
			Serial.print(": ");
			Serial.print(BD.NG.JEITA.TempRange[cnt].Voltage[cnt1]);
			Serial.print(" C");
			Serial.print(cnt1);
			Serial.print(": ");
			Serial.print(BD.NG.JEITA.TempRange[cnt].Current[cnt1]);
		}

		Serial.println();
	}

	// Gas gauge calibration data
	Serial.println();
	Serial.println("Gas gauge calibration data");
	Serial.print("    MaxOCVPRED_Mins: ");
	Serial.print(BD.NG.Cal.MaxOCVPREDmins);
	Serial.print("  OCVPRED_PrepCurrMin: ");
	Serial.print(BD.NG.Cal.OCVPRED_PrepCurrMin);
	Serial.print("  OCVPRED_PrepTimeSec: ");
	Serial.println(BD.NG.Cal.OCVPRED_PrepTimeSec);
	Serial.print("    RSOC_Delta: ");
	Serial.print(BD.NG.Cal.RSOC_Delta);
	Serial.print("  RSOC_CalMax: ");
	Serial.println(BD.NG.Cal.RSOC_CalMax);
	Serial.print("    MaxRecalCycles: ");
	Serial.print(BD.NG.Cal.MaxRecalCycles);
	Serial.print("  MaxRecalDays: ");
	Serial.println(BD.NG.Cal.MaxRecalDays);

	// Health records
	Serial.printf("\r\nHealth record type: ");
	Serial.print(BD.NG.Health1.RecordType);
	Serial.print("  Health percentage: ");
	Serial.println(BD.NG.Health1.Health);

	// Agg charge
	Serial.printf("Agg charge 1: %d   2: %d\r\n\r\n",BD.NG.AggCharge1.AcumChg,BD.NG.AggCharge2.AcumChg);

	// Initial times
	Serial.println("                        OT      HT     STH      RT     STL      LT      UT");
	Serial.printf("Inital Times (days): %5ld   %5ld   %5ld   %5ld   %5ld   %5ld   %5ld"
					,BD.NG.IT.Time[6]/86400,BD.NG.IT.Time[5]/86400,BD.NG.IT.Time[4]/86400,BD.NG.IT.Time[3]/86400,BD.NG.IT.Time[2]/86400,BD.NG.IT.Time[1]/86400,BD.NG.IT.Time[0]/86400);
	Serial.println();
	Serial.println();

	// Initial data
	Serial.printf("Temps: T1: %d  T2: %d  T5: %d  T6: %d  T3: %d  T4:%d"
					,BD.NG.InitData.Temps[0],BD.NG.InitData.Temps[1],BD.NG.InitData.Temps[2],BD.NG.InitData.Temps[3],BD.NG.InitData.Temps[4],BD.NG.InitData.Temps[5]);
	Serial.println();
	Serial.println();

	// Cell identifing data
	cnt1 = 0;
	for (cnt = 0; cnt < 84; ++cnt)  // Check if we have any cell indent data
		if (BD.NG.CellIdentData[cnt])
			cnt1 = 1;
	if (!cnt1)
		Serial.print("No cell ident data");
	else
	{
		Serial.print("Cell ident data: ");
		for (cnt = 0; cnt < 84; ++cnt)
			if (9 == BD.NG.CellIdentData[cnt])  // Handle seperator
				Serial.print("  ");
			else
				Serial.print(BD.NG.CellIdentData[cnt]);
	}
	Serial.println();
	Serial.println();
}

// Show Meteor data
void ShowMeteor(uint8_t Format)
{
	char buf[31];
	uint8_t cnt, cnt1;
	uint16_t Year;
	VALUE_TIER_BATT_DATA_t *VT = (VALUE_TIER_BATT_DATA_t *)EEPROM_Data;

	if (!VT_Valid)
	{
		Serial.println("No valid PP data to show");
		return;
	}

	// Show rev
	Serial.print("Format Rev: ");
	Serial.println(VT->FormatRev);

	// Show part/serial numbers
	Serial.print("Part number: ");
	Serial.println((char *)VT->PartNumber);
	Serial.print("Serial Num: ");
	Serial.print((char *)VT->SerialNumber);

	// Date made
	Serial.print("   Date Made: ");
	Serial.print(VT->Date[2]>>4);
	Serial.print("/");
	Serial.print(VT->Date[0]);
	Serial.print("/");
	Year = ((VT->Date[2] & 0x0f) << 8) | VT->Date[1];
	Serial.println(Year);

	// Capacities
	Serial.print("Capacity: ");
	Serial.print(VT->ManfCapacity);
	Serial.print("maHr   Low: ");
	Serial.print(VT->BatteryLow);
	Serial.print("%   Very Low: ");
	Serial.print(VT->BatteryVeryLow);
	Serial.print("%   Critical: ");
	Serial.print(VT->BatteryCritical);
	Serial.println("%");

	// Show 1725 data
	if (VT->_1725RecordType != 0)
		Serial.println("Unknown 1725 record type!!!");
	else
	{
		Serial.print("Discharge Temp Min: ");
		Serial.print(VT->_1725DiscLowLimit);
		Serial.print("C   Max: ");
		Serial.print(VT->_1725DiscHighLimit);
		Serial.println("C");
	}

	// Charger control block
	Serial.println("Charger control Data");
	Serial.print("    Slow Chrg TO: ");
	Serial.print(VT->ChargerCtrl.SlowChrgTimeout);
	Serial.print("  Fast Chrg TO: ");
	Serial.print(VT->ChargerCtrl.FastChrgTimeout);
	Serial.print("  Slow/Fast Volt: ");
	Serial.print(VT->ChargerCtrl.SlowFastVolt);
	Serial.print("mv  Recharge Volt: ");
	Serial.print(VT->ChargerCtrl.RechargeVolt);
	Serial.println("mv");
	Serial.print("    Abnormal Curr: ");
	Serial.print(VT->ChargerCtrl.AbnormalCurr);
	Serial.print("ma  SC High Fault: ");
	Serial.print(VT->ChargerCtrl.SlowChrgHighFaultCurr);
	Serial.print("ma  SC Low Fault: ");
	Serial.print(VT->ChargerCtrl.SlowChrgLowFaultCurr);
	Serial.println("ma");
	
	Serial.print("Nearly Done: ");
	Serial.print(VT->ChargerCtrl.NearlyDone);
	Serial.print("ma  Done: ");
	Serial.print(VT->ChargerCtrl.Done);
	Serial.print("ma  Hysteresis: ");
	Serial.println(VT->ChargerCtrl.Hysteresis);

	// JEITA data
	Serial.println("JEITA Data block");

	Serial.print("    Data type: ");
	Serial.print(VT->JEITA.DataType);
	Serial.print("   Stop temp: ");
	Serial.print(VT->JEITA.StopTemp);
	Serial.print("   Rec min temp: ");
	Serial.print(VT->JEITA.RecMinTemp);
	Serial.print("   Rec max temp: ");
	Serial.println(VT->JEITA.RecMaxTemp);

	for (cnt = 0; cnt < 5; ++cnt)
	{
		Serial.print("    Range");
		Serial.print(cnt);
		Serial.print(":  Start temp: ");
		Serial.print(VT->JEITA.TempRange[cnt].StartTemp);

		for (cnt1 = 0; cnt1 < 3; ++cnt1)
		{
			Serial.print("   V");
			Serial.print(cnt1);
			Serial.print(": ");
			Serial.print(VT->JEITA.TempRange[cnt].Voltage[cnt1]);
			Serial.print(" C");
			Serial.print(cnt1);
			Serial.print(": ");
			Serial.print(VT->JEITA.TempRange[cnt].Current[cnt1]);
		}

		Serial.println();
	}

	// 660 Data
	Serial.println("660 Data Block");
	if (VT->_660Data.ElimFlag)
		Serial.print("    Is");
	else
		Serial.print("    Isn't");
	Serial.println(" a battery eliminator");
	Serial.print("    Min startup volt: ");
	Serial.print(VT->_660Data.MinStartupVolt);
	Serial.print("  Therm cof: 0x");
	Serial.print(VT->_660Data.ThermCoff[2],HEX);
	Serial.print(VT->_660Data.ThermCoff[1],HEX);
	Serial.println(VT->_660Data.ThermCoff[0],HEX);
	Serial.print("    UVLO: ");
	Serial.print(VT->_660Data.UVLO);
	Serial.print("mv  OVLO: ");
	Serial.print(VT->_660Data.OVLO);
	Serial.print("mv  Cutoff: ");
	Serial.print(VT->_660Data.CutoffVolt);
	Serial.println("mv");

	// Cell identifing data
	Serial.print("Cell ident vendor code: ");
	Serial.write(VT->Ident.Vendor);
	Serial.print("  Data: ");
	for (cnt = 0; cnt < 30; ++cnt)
		if ((VT->Ident.IdentData[cnt] < 32 || VT->Ident.IdentData[cnt] > 126) && VT->Ident.IdentData[cnt])
			buf[cnt] = '?';
		else
		buf[cnt] = VT->Ident.IdentData[cnt];
	buf[cnt] = 0;
	Serial.println(buf);

	// New cell ident data
	if (COMET_BATT_DATA == Format)
	{
		Serial.println("Extended Cell Ident data");
		for (cnt = 0; cnt < 84 && VT->SDM660DTSI[cnt]; ++cnt)
			// Check for printable
			if (VT->SDM660DTSI[cnt] >= ' ' && VT->SDM660DTSI[cnt] <= 126)
			{
				if (10 == VT->SDM660DTSI[cnt])
					Serial.println();
				else
					Serial.print(VT->SDM660DTSI[cnt]);
			}
			else
				Serial.print("?");
		Serial.println();
	}

	// Ship mode
	// Is there data?
	if (VT->ShipMode.LowVoltage != 0
		|| VT->ShipMode.HighVoltage != 0
		|| VT->ShipMode.LowCapacity != 0
		|| VT->ShipMode.HighCapacity != 0)
	{  // Yup
		Serial.print("Ship mode Voltage Low: ");
		Serial.print(VT->ShipMode.LowVoltage);
		Serial.print("mv  Voltage High: ");
		Serial.print(VT->ShipMode.HighVoltage);
		Serial.print("mv  Capacity Low: ");
		Serial.print(VT->ShipMode.LowCapacity);
		Serial.print("%  Capacity High: ");
		Serial.print(VT->ShipMode.HighCapacity);
		Serial.println("%");
	}
	else // Nope
		Serial.println("No ship mode data found");

	// Health records
	Serial.print("Health 1 record type: ");
	Serial.print(VT->Health1.RecordType);
	Serial.print("  Health byte: ");
	Serial.println(VT->Health1.Health);
	Serial.print("Health 2 record type: ");
	Serial.print(VT->Health1.RecordType);
	Serial.print("  Health byte: ");
	Serial.println(VT->Health1.Health);

	// Agg charge
	Serial.print("Agg charge 1 Partial: ");
	Serial.print(VT->AggCharge1.Partial);
	Serial.print("%  Cycles: ");
	Serial.println(VT->AggCharge1.Cycles);
	Serial.print("Agg charge 2 Partial: ");
	Serial.print(VT->AggCharge2.Partial);
	Serial.print("%  Cycles: ");
	Serial.println(VT->AggCharge2.Cycles);
}

// Show Pollux data
void ShowPollux(void)
{
	uint32_t PN;
	uint16_t SN, letter;
	uint16_t Rev, Suffix;

	if (!PP_Valid)
	{
		Serial.println("No valid PP data to show");
		return;
	}

	Serial.print("Format Rev: ");
	Serial.println(EEPROM_Data[1]);

	// Calculate Symbol part number
	PN = (uint32_t)EEPROM_Data[2] << 24;
	PN |= (uint32_t)EEPROM_Data[3] << 16;
	PN |= (uint32_t)EEPROM_Data[4] << 8;
	PN |= (uint32_t)EEPROM_Data[5];

	if (PN)
	{ // Use 4 byte part number
		Rev = PN % 100;
		PN /= 100;
		Suffix = PN % 100;
		PN /= 100;

		Serial.print("Symbol Part number: ");
		Serial.print(PN);
		Serial.print("-");
		Serial.print(Suffix);
		Serial.print(" Rev:");
		if (Rev <= 20)
			Serial.println(Rev);
		else if (Rev <= 46)
			Serial.println((char)('A'+(Rev-21)));
		else
			Serial.println("???");
	}
	else
	{ // Use Extended part number
		Serial.print("Extended Part number: ");
		Serial.println((char *)(EEPROM_Data+304));
	}

	// Calculate serial number
	PN = (uint32_t)EEPROM_Data[9] << 24;
	PN |= (uint32_t)EEPROM_Data[8] << 16;
	PN |= (uint32_t)EEPROM_Data[7] << 8;
	PN |= (uint32_t)EEPROM_Data[6];
	letter = PN / 10000;
	SN = PN - (10000*letter);
	Serial.printf("Serial Num: %c%04d",letter+'A',SN);

	Serial.print("   Date Made: ");
	Serial.print(EEPROM_Data[12]>>4);
	Serial.print("/");
	Serial.print(EEPROM_Data[10]);
	Serial.print("/");
	Rev = EEPROM_Data[12] & 0x0f;
	Rev <<= 8;
	Rev |= EEPROM_Data[11];
	Serial.println(Rev);

	Rev = (uint16_t)EEPROM_Data[13] << 8;
	Rev |= (uint16_t)EEPROM_Data[14];
	Serial.print("Capacity: ");
	Serial.print(Rev);
	Serial.print("maHr   Low: ");
	Serial.print(EEPROM_Data[15]);
	Serial.print("%   Very Low: ");
	Serial.print(EEPROM_Data[16]);
	Serial.print("%   Critical: ");
	Serial.print(EEPROM_Data[226]);
	Serial.println("%");

	// Show 1725 data
	if (EEPROM_Data[289] != 0)
		Serial.println("Unknown 1725 record type!!!");
	else
	{
		Serial.print("Discharge Temp Min: ");
		Serial.print((int8_t)EEPROM_Data[290]);
		Serial.print("C   Max: ");
		Serial.print((int8_t)EEPROM_Data[291]);
		Serial.println("C");
	}
}

// Show Hawkeye data
void ShowHawkeye(void)
{
	uint16_t Rev;
	int cnt, cnt1;

	if (!PP_Valid)
	{
		Serial.println("No valid PP data to show");
		return;
	}

	Serial.print("Format Rev: ");
	Serial.println(EEPROM_Data[1]);

	Serial.print("Symbol Part number: ");
	Serial.println((char *)(EEPROM_Data+4));
	Serial.print("Serial Num: ");
	Serial.println((char *)(EEPROM_Data+24));

	Serial.print("Date Made: ");
	Serial.print(EEPROM_Data[38]>>4);
	Serial.print("/");
	Serial.print(EEPROM_Data[36]);
	Serial.print("/");
	Rev = EEPROM_Data[38] & 0x0f;
	Rev <<= 8;
	Rev |= EEPROM_Data[37];
	Serial.println(Rev);

	Rev = (uint16_t)EEPROM_Data[40] << 8;
	Rev |= (uint16_t)EEPROM_Data[41];
	Serial.print("Capacity: ");
	Serial.print(Rev);
	Serial.print("maHr   Low: ");
	Serial.print(EEPROM_Data[42]);
	Serial.print("%   Very Low: ");
	Serial.print(EEPROM_Data[43]);
	Serial.print("%   Critical: ");
	Serial.print(EEPROM_Data[197]);
	Serial.println("%");
	Serial.println();

	// Show 1725 data
	if (EEPROM_Data[245] != 0)
		Serial.println("Unknown 1725 record type!!!");
	else
	{
		Serial.print("1725 data: Discharge Temp Min: ");
		Serial.print((int8_t)EEPROM_Data[246]);
		Serial.print("C   Max: ");
		Serial.print((int8_t)EEPROM_Data[247]);
		Serial.println("C");
	}

	// Health records
	Serial.print("Health record type: ");
	Serial.print(EEPROM_Data[253]);
	Serial.print("  Health byte: ");
	Serial.println(EEPROM_Data[257]);

	// Show cycles
	Serial.print("Agg charge1 cycles: ");
	Rev = (uint16_t)EEPROM_Data[201] << 8;
	Rev |= (uint16_t)EEPROM_Data[200];
	Serial.print(Rev);
	Serial.print("   Agg charge2 cycles: ");
	Rev = (uint16_t)EEPROM_Data[209] << 8;
	Rev |= (uint16_t)EEPROM_Data[208];
	Serial.println(Rev);

	Serial.println();
	Serial.println("JEITA Data block");

	Serial.print("    Data type: ");
	Serial.print(EEPROM_Data[288]);
	Serial.print("   Stop temp: ");
	Serial.print(EEPROM_Data[290]);
	Serial.print("   Rec min temp: ");
	Serial.print(EEPROM_Data[291]);
	Serial.print("   Rec max temp: ");
	Serial.println(EEPROM_Data[292]);

	for (cnt = 0; cnt < 5; ++cnt)
	{
		Serial.print("    Range");
		Serial.print(cnt);
		Serial.print(":  Start temp: ");
		Serial.print(EEPROM_Data[304+16*cnt]);

		for (cnt1 = 0; cnt1 < 3; ++cnt1)
		{
			Serial.print("   V");
			Serial.print(cnt1);
			Serial.print(": ");
			Serial.print(*(uint16_t *)(EEPROM_Data+305+16*cnt+2*cnt1));
			Serial.print(" C");
			Serial.print(cnt1);
			Serial.print(": ");
			Serial.print(*(uint16_t *)(EEPROM_Data+311+16*cnt+2*cnt1));
		}
		Serial.println();
	}
}

void ShowPPP(void)
{
	uint8_t *cpnt, bak;
	uint16_t tmp;
	char msg[33];

	if (!PPP_Valid)
	{
		Serial.println("No valid PP+ data to show");
		return;
	}

	Serial.print("Rated: ");
	Serial.print(batteryRatedCapacity);
	Serial.print("ma  Faire: ");
	Serial.print(BD.Gifted.Faire);
	Serial.print("%  Unfaire: ");
	Serial.print(BD.Gifted.Unfaire);
	Serial.print("%%  Low: ");
	Serial.print(BD.Gifted.Low);
	Serial.println("%");

	Serial.print("SlowCur: ");
	Serial.print(BD.Gifted.SlowCharge_mA);
	Serial.print("ma  Slow/Fast: ");
	Serial.print(BD.Gifted.SlowFastCharge_mV);
	Serial.print("mv  FastCur: ");
	Serial.print(BD.Gifted.FastCharge_mA);
	Serial.print("ma  MaxCVolt: ");
	Serial.print(BD.Gifted.ChargeUp_mV);
	Serial.println("mv");

	Serial.print("Discharge Temp Min: ");
	Serial.print(BD.Gifted.DischargeMin_C);
	Serial.print("C   Max: ");
	Serial.print(BD.Gifted.DischargeMax_C);
	Serial.println("C");

	Serial.print("Nearly: ");
	Serial.print(BD.Gifted.NearlyCharged_mA);
	Serial.print("ma  Done: ");
	Serial.print(BD.Gifted.ChargeTerm_mA);
	Serial.print("ma  SlowMin: ");
	Serial.print(BD.Gifted.ChargeSlowMins);
	Serial.print("   FastMin: ");
	Serial.println(BD.Gifted.ChargeFastMins);

	Serial.print("SOH Threshold: ");
	Serial.print(BD.Gifted.Dyn1Block[0].HealthPct);
	Serial.print("%  SOH: ");
	Serial.print(BD.Gifted.SOH);
	Serial.println("%");

	Serial.print("Cycles: ");
	Serial.print(BD.Gifted.CHGCC);
	Serial.print("   TC: ");
	Serial.print(BD.Gifted.CC_Threshold * BD.Gifted.CHGCC + BD.Gifted.LCCA);
	Serial.print("maH   ZTC: ");
	Serial.print(BD.Gifted.Dyn1Block[0].BatteryLifeData);
	Serial.println("maHr");

	Serial.print("Part#: ");
	Serial.println(batteryPartNumber);
	Serial.print("PartEx#: ");
	Serial.println(batteryPartNumberEx);
	Serial.print("PartNew#: ");
	Serial.println(batteryPartNumberNew);
	Serial.print("Serial#: ");
	Serial.println(batteryID);
	Serial.print("Manf Date: ");
	Serial.print(mmonth);
	Serial.print("/");
	Serial.print(mday);
	Serial.print("/");
	Serial.println(myear);

	Serial.print("Capacity: ");
	Serial.print(BD.Gifted.SOC);
	Serial.print("%   Temperature: ");
	Serial.println(CurTemp);

	Serial.print("Secs since first use: ");
	Serial.print(dwSecSinceFirstUse);
	Serial.print("  In days: ");
	Serial.println(dwSecSinceFirstUse/24/3600);

	// Check extended temp charging data if it's there
	if (EX_CHARGE_TEMP_BLOCKS != (BD.Gifted.Block_Allocation_EEPROM & EX_CHARGE_TEMP_BLOCKS))  // Check if any of those blocks are marked as used
	{
		uint8_t cnt, cnt1;

		Serial.println();
		Serial.println("JEITA Data block");

		Serial.print("    Data type: ");
		Serial.print(BD.Gifted.ExCharge.DataType);
		Serial.print("   Stop temp: ");
		Serial.print(BD.Gifted.ExCharge.StopTemp);
		Serial.print("   Rec min temp: ");
		Serial.print(BD.Gifted.ExCharge.RecMinTemp);
		Serial.print("   Rec max temp: ");
		Serial.println(BD.Gifted.ExCharge.RecMaxTemp);

		for (cnt = 0; cnt < 5; ++cnt)
		{
			Serial.print("    Range");
			Serial.print(cnt);
			Serial.print(":  Start temp: ");
			Serial.print(BD.Gifted.ExCharge.TempRange[cnt].StartTemp);

			for (cnt1 = 0; cnt1 < 3; ++cnt1)
			{
				Serial.print("   V");
				Serial.print(cnt1);
				Serial.print(": ");
				Serial.print(BD.Gifted.ExCharge.TempRange[cnt].Voltage[cnt1]);
				Serial.print(" C");
				Serial.print(cnt1);
				Serial.print(": ");
				Serial.print(BD.Gifted.ExCharge.TempRange[cnt].Current[cnt1]);
			}

			Serial.println();
		}
	}

	// Check for SD660 block being used
	if (!(BD.Gifted.Block_Allocation_EEPROM & SD660_BLOCK))
	{  // block is used
		Serial.println();
		Serial.println("SD660 Data block");

		cpnt = (uint8_t *)&BD.Gifted;
		cpnt += SD660blkaddr; // Set to beginning of SD660 block

		if (cpnt[0])
			Serial.println("Is a battery eliminator");
		else
			Serial.println("Not a battery eliminator");

		Serial.print("Minimum startup voltage: ");
		tmp = *(uint16_t *)(cpnt+1);
		Serial.print(tmp);
		Serial.println("mv");

		Serial.print("UVLO: ");
		tmp = *(uint16_t *)(cpnt+6);
		Serial.print(tmp);
		Serial.println("mv");

		Serial.print("OVLO: ");
		tmp = *(uint16_t *)(cpnt+8);
		Serial.print(tmp);
		Serial.println("mv");

		Serial.print("Cutoff voltage: ");
		tmp = *(uint16_t *)(cpnt+10);
		Serial.print(tmp);
		Serial.println("mv");
	}

	// Check for cell identifying blocks being used
	if (!(BD.Gifted.Block_Allocation_EEPROM & CELL_IDENT_BLOCKS))
	{  // blocks are used
		Serial.println();
		Serial.println("Cell identifying data");

		cpnt = (uint8_t *)&BD.Gifted;
		cpnt += CELL_IDENT1blkaddr; // Set to beginning of ident blocks
		
		// Mask non-printable chars
		for (tmp = 1; tmp < 32; ++tmp)
			if ((cpnt[tmp] < 32 || cpnt[tmp] > 126) && cpnt[tmp])
				cpnt[tmp] = '?';
		bak = cpnt[tmp];  // Save first byte of Ship mode data
		cpnt[tmp] = 0;  // Terminate ident string just in case

		Serial.print("Cell vendor code: ");
		msg[0] = cpnt[1];
		msg[1] = 0;
		Serial.println(msg);

		Serial.print("Identifying Data: ");
		Serial.println((char *)(cpnt+2));

		cpnt[tmp] = bak;  // Restore first byte of Ship mode data
	}

	// Check for ship mode block being used
	if (!(BD.Gifted.Block_Allocation_EEPROM & SHIP_MODE_BLOCK))
	{  // block is used
		Serial.println();
		Serial.println("Ship mode data");

		// Is there data?
		if (BD.Gifted.ShipMode.LowVoltage != 0
			|| BD.Gifted.ShipMode.HighVoltage != 0
			|| BD.Gifted.ShipMode.LowCapacity != 0
			|| BD.Gifted.ShipMode.HighCapacity != 0)
		{  // Yup
			Serial.print("Voltage Low: ");
			Serial.print(BD.Gifted.ShipMode.LowVoltage);
			Serial.print("mv  Voltage High: ");
			Serial.print(BD.Gifted.ShipMode.HighVoltage);
			Serial.println("mv");
			Serial.print("Capacity Low: ");
			Serial.print(BD.Gifted.ShipMode.LowCapacity);
			Serial.print("%  Capacity High: ");
			Serial.print(BD.Gifted.ShipMode.HighCapacity);
			Serial.println("%");
		}
		else // Nope
			Serial.println("No ship mode data found");
	}
}

void ShowM200regs(void)
{
	uint8_t cnt;

	GG_GetStuff(&BD.Gifted);

	Serial.printf("GG Device Type: 0x%04X\r\n",BD.Gifted.GG_DeviceType);
	Serial.printf("GG Firmware Version: 0x%04X\r\n",BD.Gifted.GG_FirmwareVer);
	Serial.printf("GG Hardware Version: 0x%04X\r\n\n",BD.Gifted.GG_HardwareVer);

	if (GG_ReadReg())
		return;

	Serial.println("     Control/Status");
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			Serial.print(" Low:  ");
		else
			Serial.print("High:  ");

		if (((BD.Gifted.CONTROL_STATUS & 0x8000) ^ (cnt << 15)))
			Serial.print("SE ");
		else
			Serial.print("   ");

		if (((BD.Gifted.CONTROL_STATUS & 0x4000) ^ (cnt << 14)))
			Serial.print("FAS ");
		else
			Serial.print("    ");

		if (((BD.Gifted.CONTROL_STATUS & 0x2000) ^ (cnt << 13)))
			Serial.print("SS ");
		else
			Serial.print("   ");

		if (((BD.Gifted.CONTROL_STATUS & 0x1000) ^ (cnt << 12)))
			Serial.print("CSV ");
		else
			Serial.print("    ");

		if ((BD.Gifted.CONTROL_STATUS & 0x800) ^ (cnt << 11))
			Serial.print("CCA ");
		else
			Serial.print("    ");

		if ((BD.Gifted.CONTROL_STATUS & 0x400) ^ (cnt << 10))
			Serial.print("BCA ");
		else
			Serial.print("    ");

		if ((BD.Gifted.CONTROL_STATUS & 0x200) ^ (cnt << 9))
			Serial.print("IBAW ");
		else
			Serial.print("     ");

		if (((BD.Gifted.CONTROL_STATUS & 0x80) ^ (cnt << 7)))
			Serial.print("SHUTDOWN ");
		else
			Serial.print("         ");

		if (((BD.Gifted.CONTROL_STATUS & 0x40) ^ (cnt << 6)))
			Serial.print("HIBERNATE ");
		else
			Serial.print("          ");

		if (((BD.Gifted.CONTROL_STATUS & 0x20) ^ (cnt << 5)))
			Serial.print("FULLSLEEP ");
		else
			Serial.print("          ");

		if (((BD.Gifted.CONTROL_STATUS & 0x10) ^ (cnt << 4)))
			Serial.print("SLEEP ");
		else
			Serial.print("      ");

		if ((BD.Gifted.CONTROL_STATUS & 0x08) ^ (cnt << 3))
			Serial.print("LDMD ");
		else
			Serial.print("     ");

		if ((BD.Gifted.CONTROL_STATUS & 0x04) ^ (cnt << 2))
			Serial.print("RUP_DIS ");
		else
			Serial.print("        ");

		if ((BD.Gifted.CONTROL_STATUS & 0x02) ^ (cnt << 1))
			Serial.print("VOK ");
		else
			Serial.print("    ");

		if ((BD.Gifted.CONTROL_STATUS & 0x01) ^ cnt)
			Serial.print("QEN ");
		else
			Serial.print("    ");

		Serial.println();
	}
	Serial.println();
	Serial.print("Temp: ");
	Serial.print(CurTemp);
	Serial.print("Deg C   Volt: ");
	Serial.print(BD.Gifted.VOLT);
	Serial.print("mv   Curr: ");
	Serial.print(BD.Gifted.AI);
	Serial.println("ma");
	Serial.println();
	Serial.println("     Flags");
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			Serial.print(" Low:  ");
		else
			Serial.print("High:  ");

		if (((BD.Gifted.FLAGS & 0x8000) ^ (cnt << 15)))
			Serial.print("OTC ");
		else
			Serial.print("    ");

		if (((BD.Gifted.FLAGS & 0x4000) ^ (cnt << 14)))
			Serial.print("OTD ");
		else
			Serial.print("    ");

		if ((BD.Gifted.FLAGS & 0x800) ^ (cnt << 11))
			Serial.print("CHG_INH ");
		else
			Serial.print("        ");

		if ((BD.Gifted.FLAGS & 0x400) ^ (cnt << 10))
			Serial.print("XCHG ");
		else
			Serial.print("     ");

		if ((BD.Gifted.FLAGS & 0x200) ^ (cnt << 9))
			Serial.print("FC ");
		else
			Serial.print("   ");

		if ((BD.Gifted.FLAGS & 0x100) ^ (cnt << 8))
			Serial.print("CHG ");
		else
			Serial.print("    ");

		if (((BD.Gifted.FLAGS & 0x80) ^ (cnt << 7)))
			Serial.print("RMFCC_EN ");
		else
			Serial.print("         ");

		if (((BD.Gifted.FLAGS & 0x40) ^ (cnt << 6)))
			Serial.print("OCV_TAKEN ");
		else
			Serial.print("          ");

		if (((BD.Gifted.FLAGS & 0x20) ^ (cnt << 5)))
			Serial.print("OCV_PRED ");
		else
			Serial.print("         ");

		if ((BD.Gifted.FLAGS & 0x04) ^ (cnt << 2))
			Serial.print("SOC1 ");
		else
			Serial.print("     ");

		if ((BD.Gifted.FLAGS & 0x02) ^ (cnt << 1))
			Serial.print("SOCF ");
		else
			Serial.print("     ");

		if ((BD.Gifted.FLAGS & 0x01) ^ cnt)
			Serial.print("DSG ");
		else
			Serial.print("    ");

		Serial.println();
	}
	Serial.println();

	Serial.println("    Capacities");
	Serial.print("Nominal Avail:");
	Serial.print(BD.Gifted.NAC);
	Serial.print("  Full Avail:");
	Serial.print(BD.Gifted.FAC);
	Serial.print("  Remaining:");
	Serial.print(BD.Gifted.RM);
	Serial.print("  Full Chrg:");
	Serial.print(BD.Gifted.FCC);
	Serial.print("  Design Cap:");
	Serial.println(DesignCap);

	Serial.print("Health:");
	Serial.print(BD.Gifted.SOH);
	Serial.print("%  Capacity:");
	Serial.print(BD.Gifted.SOC);
	Serial.println("%");
	Serial.println();

	Serial.print("Secs since first use: ");
	Serial.print(dwSecSinceFirstUse);
	Serial.print("  In days: ");
	Serial.println(dwSecSinceFirstUse/24/3600);

	Serial.print("Valid Qmax cycles:");
	Serial.println(BD.Gifted.QVC);
	Serial.println();
}

void ShowM200regsHex(void)
{
	uint16_t *tmp;
	uint8_t cnt;

	if (GG_ReadReg())
		return;

	tmp = &BD.Gifted.CONTROL_STATUS;

	for (cnt = 0; cnt <= 30; ++cnt)
	{
		Serial.printf("%04X ",*(tmp+cnt));
		if ((cnt & 0x07) == 0x07)
		Serial.println();
	}
	Serial.println();
}

void Show27Z561regs(void)
{
	uint8_t cnt, cnt2;
	uint8_t regs[NEW_GB_CMD_FULL_READ_SIZE];
	uint32_t val;

	newGG_GetStuff(&BD.NG);

	Serial.printf("GG Device Type: 0x%04X\r\n",BD.NG.GG_DeviceType);
	Serial.println("GG Firmware: ");
	Serial.printf("    Device: %02X%02X\r\n",BD.NG.GG_FirmwareVer[0],BD.NG.GG_FirmwareVer[1]);
	Serial.printf("    Version: %02X%02X\r\n",BD.NG.GG_FirmwareVer[2],BD.NG.GG_FirmwareVer[3]);
	Serial.printf("    Build: %02X%02X\r\n",BD.NG.GG_FirmwareVer[4],BD.NG.GG_FirmwareVer[5]);
	Serial.printf("    Type: %02X\r\n",BD.NG.GG_FirmwareVer[6]);
	Serial.printf("    IT Ver: %02X%02X\r\n",BD.NG.GG_FirmwareVer[7],BD.NG.GG_FirmwareVer[8]);
	Serial.printf("GG Hardware Version: 0x%04X\r\n\n",BD.NG.GG_HardwareVer);

	if (newGG_ReadReg(regs))
		return;

	Serial.println("     Control/Status");
	val = regs[0] | (regs[1] << 8);
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			Serial.print(" Low:  ");
		else
			Serial.print("High:  ");

		if (((val & 0x4000) ^ (cnt << 14)))
			Serial.print("FAS ");
		else
			Serial.print("    ");

		if (((val & 0x2000) ^ (cnt << 13)))
			Serial.print("SS ");
		else
			Serial.print("   ");

		if ((val & 0x200) ^ (cnt << 9))
			Serial.print("CSum ");
		else
			Serial.print("     ");

		if (((val & 0x80) ^ (cnt << 7)))
			Serial.print("LDMD ");
		else
			Serial.print("     ");

		if (((val & 0x40) ^ (cnt << 6)))
			Serial.print("RDIS ");
		else
			Serial.print("     ");

		if ((val & 0x02) ^ (cnt << 1))
			Serial.print("VOK ");
		else
			Serial.print("    ");

		if ((val & 0x01) ^ cnt)
			Serial.print("QEN ");
		else
			Serial.print("    ");

		Serial.println();
	}
	Serial.println();

	Serial.println("     Battery Status");
	val = regs[NGG_BATTERY_STATUS] | (regs[NGG_BATTERY_STATUS+1] << 8);
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			Serial.print(" Low:  ");
		else
			Serial.print("High:  ");

		if (((val & 0x4000) ^ (cnt << 14)))
			Serial.print("TCA ");
		else
			Serial.print("    ");

		if ((val & 0x800) ^ (cnt << 11))
			Serial.print("TDA ");
		else
			Serial.print("    ");

		if ((val & 0x200) ^ (cnt << 9))
			Serial.print("RCA ");
		else
			Serial.print("    ");

		if (((val & 0x80) ^ (cnt << 7)))
			Serial.print("INIT ");
		else
			Serial.print("     ");

		if (((val & 0x40) ^ (cnt << 6)))
			Serial.print("DSG ");
		else
			Serial.print("    ");

		if (((val & 0x20) ^ (cnt << 5)))
			Serial.print("FC ");
		else
			Serial.print("   ");

		if ((val & 0x10) ^ (cnt << 4))
			Serial.print("FD ");
		else
			Serial.print("   ");

		Serial.println();
	}

	Serial.println();
	Serial.println("     Gauging Status");
	val = BD.NG.GaugingStatus;
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			Serial.print(" Low:  ");
		else
			Serial.print("High:  ");

		if (((val & 0x200000) ^ (cnt << 21)))
			Serial.print("QMAXDODOK ");
		else
			Serial.print("          ");

		if ((val & 0x100000) ^ (cnt << 20))
			Serial.print("OCVFR ");
		else
			Serial.print("      ");

		if ((val & 0x80000) ^ (cnt << 19))
			Serial.print("LDMD ");
		else
			Serial.print("     ");

		if (((val & 0x40000) ^ (cnt << 18)))
			Serial.print("RX ");
		else
			Serial.print("   ");

		if (((val & 0x20000) ^ (cnt << 17)))
			Serial.print("QMAX ");
		else
			Serial.print("     ");

		if (((val & 0x10000) ^ (cnt << 16)))
			Serial.print("VDQ ");
		else
			Serial.print("    ");

		if ((val & 0x8000) ^ (cnt << 15))
			Serial.print("NSFM ");
		else
			Serial.print("     ");

		if ((val & 0x4000) ^ (cnt << 14))
			Serial.print("OCVPRED ");
		else
			Serial.print("        ");

		if ((val & 0x2000) ^ (cnt << 13))
			Serial.print("SLPQMax ");
		else
			Serial.print("        ");

		if ((val & 0x1000) ^ (cnt << 12))
			Serial.print("QEN ");
		else
			Serial.print("    ");

		if ((val & 0x800) ^ (cnt << 11))
			Serial.print("VOK ");
		else
			Serial.print("    ");

		if ((val & 0x400) ^ (cnt << 10))
			Serial.print("RDIS ");
		else
			Serial.print("     ");

		if ((val & 0x100) ^ (cnt << 8))
			Serial.print("REST ");
		else
			Serial.print("        ");

		if ((val & 0x40) ^ (cnt << 6))
			Serial.print("DSG ");
		else
			Serial.print("    ");

		if ((val & 0x20) ^ (cnt << 5))
			Serial.print("EDV ");
		else
			Serial.print("    ");

		if ((val & 0x8) ^ (cnt << 3))
			Serial.print("TC ");
		else
			Serial.print("   ");

		if ((val & 0x4) ^ (cnt << 2))
			Serial.print("TD ");
		else
			Serial.print("   ");

		if ((val & 0x2) ^ (cnt << 1))
			Serial.print("FC ");
		else
			Serial.print("   ");

		if ((val & 0x1) ^ (cnt << 0))
			Serial.print("FD ");
		else
			Serial.print("   ");

		Serial.println();
	}

	Serial.println();
	Serial.print("Batt Temp: ");
	Serial.print(CurTemp);
	Serial.print("Deg C   Int Temp: ");
	Serial.print(CurIntTemp);
	Serial.print("Deg C   Volt: ");
	Serial.print(regs[NGG_VOLTAGE] | (regs[NGG_VOLTAGE+1] << 8));
	Serial.print("mv   Curr: ");
	Serial.print((int16_t)(regs[NGG_CURRENT] | (regs[NGG_CURRENT+1] << 8)));
	Serial.println("ma");

	Serial.println();
	Serial.println("    Capacities");
	Serial.print("Remaining: ");
	Serial.print(regs[NGG_RM] | (regs[NGG_RM+1] << 8));
	Serial.print("  Full Chrg: ");
	Serial.print(regs[NGG_FCC] | (regs[NGG_FCC+1] << 8));
	Serial.print("  Design Cap: ");
	Serial.println(regs[NGG_DCAP] | (regs[NGG_DCAP+1] << 8));

	Serial.print("Health: ");
	Serial.print(regs[NGG_SOH] | (regs[NGG_SOH+1] << 8));
	Serial.print("%  Capacity: ");
	Serial.print(regs[NGG_RSOC] | (regs[NGG_RSOC+1] << 8));
	Serial.print("%  Cycles: ");
	Serial.println(regs[NGG_CYCLES] | (regs[NGG_CYCLES+1] << 8));


	Serial.print("Calibration: ");
	Serial.print("  QMAX Cycles: ");
	Serial.print(regs[NGG_QMAX_CYCLES] | (regs[NGG_QMAX_CYCLES+1] << 8));
	Serial.print("  QMAX Day: ");
	Serial.print(BD.NG.QMAX_DAY);
	Serial.print("  QMAXDOD0: ");
	Serial.print(BD.NG.QMAXDOD0);
	if (!BD.NG.QMAXDOD0)
		Serial.println(" (0%)");
	else
	{
		Serial.print("  (");
		Serial.print(BD.NG.QMAXDOD0*(uint32_t)100/(uint32_t)0x7fff);
		Serial.println("%)");
	}
	Serial.println();

	Serial.print("Secs since manufacture: ");
	Serial.print(NGGsecSinceMade);
	Serial.print("  In days: ");
	Serial.println(NGGsecSinceMade/24/3600);
	Serial.println();

	// Time at temp/capacity
	Serial.println("Time in zones in days");
	Serial.println("     >=95%  >=90%  >=80%  >=50%  >=20%  >=10%   >=5%   >=0%");
	Serial.print("UT ");
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
		Serial.printf("%7ld",BD.NG.times[0][cnt2]/86400);
	Serial.println();
	Serial.print("LT ");
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
		Serial.printf("%7ld",BD.NG.times[1][cnt2]/86400);
	Serial.println();
	Serial.print("SLT");
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
		Serial.printf("%7ld",BD.NG.times[2][cnt2]/86400);
	Serial.println();
	Serial.print("RT ");
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
		Serial.printf("%7ld",BD.NG.times[3][cnt2]/86400);
	Serial.println();
	Serial.print("STH");
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
		Serial.printf("%7ld",BD.NG.times[4][cnt2]/86400);
	Serial.println();
	Serial.print("HT ");
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
		Serial.printf("%7ld",BD.NG.times[5][cnt2]/86400);
	Serial.println();
	Serial.print("OT ");
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
		Serial.printf("%7ld",BD.NG.times[6][cnt2]/86400);
	Serial.println();
}

void ShowGGregs(void)
{
	// Which gauge???
	if (NEW_GAUGE)
		Show27Z561regs();
	else
		ShowM200regs();
}

void ShowGGregsHex(void)
{
	// Which gauge???
	if (NEW_GAUGE)
		return;
	else
		ShowM200regsHex();
}
#endif // Zebra mode
