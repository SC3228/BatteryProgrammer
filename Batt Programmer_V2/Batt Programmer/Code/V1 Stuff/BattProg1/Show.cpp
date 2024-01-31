// Code to display battery data

#include <Arduino.h>

#include "pwmbatt.h"
#include "BattProg.h"
#include "GasGauge.h"

// MPA2 battery part number stuff
#define DEFAULT_BATTERY_PREFIX_NOS		82		// Default prefix for battery family numbers

void ShowMPA2(void)
{
#ifdef ZEBRA_MODE
	char buf[31];
	uint16_t Year;
	SMART_BATT_DATA_t *SB = (SMART_BATT_DATA_t *)EEPROM_Data;
	// Used in part number conversions
	uint8_t dwBattPrefixNo = DEFAULT_BATTERY_PREFIX_NOS;
	uint32_t dwBattFamilyNo;

	if (!PP_Valid)
	{
		Serial.println(F("No valid PP data to show"));
		return;
	}

	// Show rev
	if (4 != SB->BattDataType)
	{
		Serial.println(F("Unknown data type!!!"));
		return;
	}
	Serial.print(F("DataType: "));
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
	if (SB->BattPartNum%100 < 21)
		sprintf(buf,"%02d-%06ld-%02ld Rev.%02ld", dwBattPrefixNo, dwBattFamilyNo, (SB->BattPartNum/100)%100, SB->BattPartNum%100);
	else
		sprintf(buf,"%02d-%06ld-%02ld Rev. %c", dwBattPrefixNo, dwBattFamilyNo, (SB->BattPartNum/100)%100, (char)(SB->BattPartNum%100-21+'A'));

	Serial.print(F("Part number: "));
	Serial.print(buf);
	Serial.print(F("  Serial Num: "));
	Serial.println(SB->BattID);

	// Date made
	Serial.print(F("Date Made: "));
	Serial.print(SB->BattDate[2]>>4);
	Serial.print(F("/"));
	Serial.print(SB->BattDate[0]);
	Serial.print(F("/"));
	Year = ((SB->BattDate[2] & 0x0f) << 8) | SB->BattDate[1];
	Serial.println(Year);

	// Capacities
	Serial.print(F("Capacity: "));
	Serial.print(SB->BattRated);
	Serial.print(F("maHr   Faire: "));
	Serial.print(SB->BattFaireLevel);
	Serial.print(F("%   Unfaire: "));
	Serial.print(SB->BattUnfaireLevel);
	Serial.print(F("%   Low: "));
	Serial.print(SB->BattLowLevel);
	Serial.println(F("%"));

	// Charger control block
	Serial.println(F("Charger control Data"));
	Serial.print(F("    Slow Chrg TO: "));
	Serial.print(SB->SlowChrgTO);
	Serial.print(F("  Fast Chrg TO: "));
	Serial.println(SB->FastChrgTO);
	Serial.print(F("    Slow/Fast Volt: "));
	Serial.print(SB->SlowFastThreshold);
	Serial.print(F("mv  Recharge Volt: "));
	Serial.print(SB->RechargeVoltage);
	Serial.print(F("mv"));
	Serial.print(F("mv  Absent Batt Volt: "));
	Serial.print(SB->AbsentBattVoltage);
	Serial.println(F("mv"));
	Serial.print(F("    Abnormal Curr: "));
	Serial.print(SB->AbnormalCurrent);
	Serial.print(F("ma  SC High Fault: "));
	Serial.print(SB->SlowChrgHighFC);
	Serial.print(F("ma  SC Low Fault: "));
	Serial.print(SB->SlowChrgLowFC);
	Serial.println(F("ma"));
	Serial.print(F("    Nearly Done: "));
	Serial.print(SB->NearlyDoneCurrent);
	Serial.print(F("ma  Done: "));
	Serial.print(SB->DoneCurrent);
	Serial.println(F("ma"));

	// Agg charge
	Serial.print(F("Agg charge 1: "));
	Serial.print(SB->BattLife1);
	Serial.print(F("maHrs   Agg charge 2: "));
	Serial.print(SB->BattLife2);
	Serial.println(F("maHrs"));

	// Show 1725 data
	if (SB->_1725_DataType != 1)
		Serial.println(F("Unknown 1725 data type!!!"));
	else
	{
		Serial.println(F("1725 Data"));
		Serial.print(F("    Discharge Temp Min: "));
		Serial.print(SB->BattMinDTemp);
		Serial.print(F("C   Max: "));
		Serial.print(SB->BattMaxDTemp);
		Serial.println(F("C"));
		Serial.print(F("    Max charge volt: "));
		Serial.print(SB->BattMaxCVolt);
		Serial.print(F("mv (Ignored)   Fast chrg curr: "));
		Serial.print(SB->BattFastCurr);
		Serial.println(F("ma"));
	}

	// Health records
	if (SB->Health_DataType != 1)
		Serial.println(F("Unknown health data type!!!"));
	else
	{
		Serial.print(F("Date of first use: "));
		Serial.print(SB->DateFirstUse[2]>>4);
		Serial.print(F("/"));
		Serial.print(SB->DateFirstUse[0]);
		Serial.print(F("/"));
		Year = ((SB->DateFirstUse[2] & 0x0f) << 8) | SB->DateFirstUse[1];
		Serial.print(Year);
		Serial.print(F("   Health byte: "));
		Serial.println(SB->BattHealth);
	}

#endif
}

// Show PP+ V2 data
void ShowPPPV2(void)
{
#ifdef ZEBRA_MODE
	char buf[31];
	uint8_t cnt, cnt1;
	uint16_t Year;

	if (!PPPV2_Valid)
	{
		Serial.println(F("No valid PPP V2 data to show"));
		return;
	}

	// Show revs
	Serial.print(F("Block 1 Rev: "));
	Serial.print(BD.NG.B1FormatRev);
	Serial.print(F("   Block 2 Rev: "));
	Serial.print(BD.NG.B2FormatRev);
	Serial.print(F("   Block 3 Rev: "));
	Serial.println(BD.NG.B3FormatRev);

	// Show part/serial numbers
	Serial.print(F("Part number: "));
	Serial.print((char *)BD.NG.PartNumber);
	Serial.print(F("   Serial Num: "));
	sprintf(buf,"%c%04d",BD.NG.ManufName[0],BD.NG.SerialNum);
	Serial.print(buf);

	// Date made
	Serial.print(F("   Date Made: "));
	Serial.print((BD.NG.DateMade&0x1e0)>>5);
	Serial.print(F("/"));
	Serial.print(BD.NG.DateMade&0x1f);
	Serial.print(F("/"));
	Year = ((BD.NG.DateMade>>9)+1980);
	Serial.println(Year);

	// Capacities
	Serial.print(F("Capacity: "));
	Serial.print(BD.NG.ManfCapacity_ma);
	Serial.print(F("maHr   "));
	Serial.print(BD.NG.ManfCapacity_mw);
	Serial.print(F("mwHr   Low: "));
	Serial.print(BD.NG.BatteryLow);
	Serial.print(F("%   Very Low: "));
	Serial.print(BD.NG.BatteryVeryLow);
	Serial.print(F("%   Critical: "));
	Serial.print(BD.NG.BatteryCritical);
	Serial.println(F("%"));

	// Show 1725 data
	Serial.print(F("Discharge Temp Min: "));
	Serial.print(BD.NG._1725DiscLowLimit);
	Serial.print(F("C   Max: "));
	Serial.print(BD.NG._1725DiscHighLimit);
	Serial.println(F("C"));

	// Charger control block
	Serial.println();
	Serial.println(F("Charger control Data"));
	Serial.print(F("    Slow Chrg TO: "));
	Serial.print(BD.NG.ChargerCtrl.SlowChrgTimeout);
	Serial.print(F("  Fast Chrg TO: "));
	Serial.print(BD.NG.ChargerCtrl.FastChrgTimeout);
	Serial.print(F("  Slow/Fast Volt: "));
	Serial.print(BD.NG.ChargerCtrl.SlowFastVolt);
	Serial.print(F("mv  Recharge Volt Delta: "));
	Serial.print(BD.NG.ChargerCtrl.RechargeVoltDelta);
	Serial.println(F("mv"));
	Serial.print(F("    Abnormal Curr: "));
	Serial.print(BD.NG.ChargerCtrl.AbnormalCurr);
	Serial.print(F("ma  Slow Charge Curr: "));
	Serial.print(BD.NG.ChargerCtrl.SlowChrgCurr);	
	Serial.print(F("  Nearly Done: "));
	Serial.print(BD.NG.ChargerCtrl.NearlyDoneCurr);
	Serial.print(F("ma  Fallback Term Curr: "));
	Serial.print(BD.NG.ChargerCtrl.FallBackTermCurr);
	Serial.println(F("ma"));
	Serial.print(F("    Hysteresis: "));
	Serial.println(BD.NG.ChargerCtrl.Hysteresis);

	// Ship mode
	Serial.println();
	// Is there data?
	if (BD.NG.ShipMode.LowVoltage != 0
		|| BD.NG.ShipMode.HighVoltage != 0
		|| BD.NG.ShipMode.LowCapacity != 0
		|| BD.NG.ShipMode.HighCapacity != 0)
	{  // Yup
		Serial.print(F("Ship mode Voltage Low: "));
		Serial.print(BD.NG.ShipMode.LowVoltage);
		Serial.print(F("mv  Voltage High: "));
		Serial.print(BD.NG.ShipMode.HighVoltage);
		Serial.print(F("mv  Capacity Low: "));
		Serial.print(BD.NG.ShipMode.LowCapacity);
		Serial.print(F("%  Capacity High: "));
		Serial.print(BD.NG.ShipMode.HighCapacity);
		Serial.println(F("%"));
	}
	else // Nope
		Serial.println(F("No ship mode data found"));

	// 660 Data
	Serial.println();
	Serial.println(F("660 Data Block"));
	Serial.print(F("    Battery Flags: "));
	Serial.println(BD.NG._660Data.ElimFlag);
	Serial.print(F("    Min startup volt: "));
	Serial.print(BD.NG._660Data.MinStartupVolt);
	Serial.print(F("  Therm cof: 0x"));
	Serial.print(BD.NG._660Data.ThermCoff[2],HEX);
	Serial.print(BD.NG._660Data.ThermCoff[1],HEX);
	Serial.println(BD.NG._660Data.ThermCoff[0],HEX);
	Serial.print(F("    UVLO: "));
	Serial.print(BD.NG._660Data.UVLO);
	Serial.print(F("mv  OVLO: "));
	Serial.print(BD.NG._660Data.OVLO);
	Serial.print(F("mv  Cutoff: "));
	Serial.print(BD.NG._660Data.CutoffVolt);
	Serial.println(F("mv"));

	// JEITA data
	Serial.println();
	Serial.println(F("JEITA Data block"));
	Serial.print(F("    Data type: "));
	Serial.print(BD.NG.JEITA.DataType);
	Serial.print(F("   Stop temp: "));
	Serial.print(BD.NG.JEITA.StopTemp);
	Serial.print(F("   Rec min temp: "));
	Serial.print(BD.NG.JEITA.RecMinTemp);
	Serial.print(F("   Rec max temp: "));
	Serial.println(BD.NG.JEITA.RecMaxTemp);

	for (cnt = 0; cnt < 5; ++cnt)
	{
		Serial.print(F("    Range"));
		Serial.print(cnt);
		Serial.print(F(":  Start temp: "));
		Serial.print(BD.NG.JEITA.TempRange[cnt].StartTemp);

		for (cnt1 = 0; cnt1 < 3; ++cnt1)
		{
			Serial.print(F("   V"));
			Serial.print(cnt1);
			Serial.print(F(": "));
			Serial.print(BD.NG.JEITA.TempRange[cnt].Voltage[cnt1]);
			Serial.print(F(" C"));
			Serial.print(cnt1);
			Serial.print(F(": "));
			Serial.print(BD.NG.JEITA.TempRange[cnt].Current[cnt1]);
		}

		Serial.println();
	}

	// Gas gauge calibration data
	Serial.println();
	Serial.println(F("Gas gauge calibration data"));
	Serial.print(F("    MaxOCVPRED_Mins: "));
	Serial.print(BD.NG.Cal.MaxOCVPREDmins);
	Serial.print(F("  OCVPRED_PrepCurrMin: "));
	Serial.print(BD.NG.Cal.OCVPRED_PrepCurrMin);
	Serial.print(F("  OCVPRED_PrepTimeSec: "));
	Serial.println(BD.NG.Cal.OCVPRED_PrepTimeSec);
	Serial.print(F("    RSOC_Delta: "));
	Serial.print(BD.NG.Cal.RSOC_Delta);
	Serial.print(F("  RSOC_CalMax: "));
	Serial.println(BD.NG.Cal.RSOC_CalMax);
	Serial.print(F("    MaxRecalCycles: "));
	Serial.print(BD.NG.Cal.MaxRecalCycles);
	Serial.print(F("  MaxRecalDays: "));
	Serial.println(BD.NG.Cal.MaxRecalDays);
/*  ***FIX***
	// Cell identifing data
	Serial.print(F("Cell ident vendor code: "));
	Serial.write(VT->Ident.Vendor);
	Serial.print(F("  Data: "));
	for (cnt = 0; cnt < 30; ++cnt)
		if ((VT->Ident.IdentData[cnt] < 32 || VT->Ident.IdentData[cnt] > 126) && VT->Ident.IdentData[cnt])
			buf[cnt] = '?';
		else
		buf[cnt] = VT->Ident.IdentData[cnt];
	buf[cnt] = 0;
	Serial.println(buf);

	// Health records
	Serial.print(F("Health 1 record type: "));
	Serial.print(VT->Health1.RecordType);
	Serial.print(F("  Health byte: "));
	Serial.println(VT->Health1.Health);
	Serial.print(F("Health 2 record type: "));
	Serial.print(VT->Health1.RecordType);
	Serial.print(F("  Health byte: "));
	Serial.println(VT->Health1.Health);

	// Agg charge
	Serial.print(F("Agg charge 1 Partial: "));
	Serial.print(VT->AggCharge1.Partial);
	Serial.print(F("%  Cycles: "));
	Serial.println(VT->AggCharge1.Cycles);
	Serial.print(F("Agg charge 2 Partial: "));
	Serial.print(VT->AggCharge2.Partial);
	Serial.print(F("%  Cycles: "));
	Serial.println(VT->AggCharge2.Cycles);
*/
#endif
}

// Show Meteor data
void ShowMeteor(void)
{
#ifdef ZEBRA_MODE
	char buf[31];
	uint8_t cnt, cnt1;
	uint16_t Year;
	VALUE_TIER_BATT_DATA_t *VT = (VALUE_TIER_BATT_DATA_t *)EEPROM_Data;

	if (!VT_Valid)
	{
		Serial.println(F("No valid PP data to show"));
		return;
	}

	// Show rev
	Serial.print(F("Format Rev: "));
	Serial.println(VT->FormatRev);

	// Show part/serial numbers
	Serial.print(F("Part number: "));
	Serial.println((char *)VT->PartNumber);
	Serial.print(F("Serial Num: "));
	Serial.print((char *)VT->SerialNumber);

	// Date made
	Serial.print(F("   Date Made: "));
	Serial.print(VT->Date[2]>>4);
	Serial.print(F("/"));
	Serial.print(VT->Date[0]);
	Serial.print(F("/"));
	Year = ((VT->Date[2] & 0x0f) << 8) | VT->Date[1];
	Serial.println(Year);

	// Capacities
	Serial.print(F("Capacity: "));
	Serial.print(VT->ManfCapacity);
	Serial.print(F("maHr   Low: "));
	Serial.print(VT->BatteryLow);
	Serial.print(F("%   Very Low: "));
	Serial.print(VT->BatteryVeryLow);
	Serial.print(F("%   Critical: "));
	Serial.print(VT->BatteryCritical);
	Serial.println(F("%"));

	// Show 1725 data
	if (VT->_1725RecordType != 0)
		Serial.println(F("Unknown 1725 record type!!!"));
	else
	{
		Serial.print(F("Discharge Temp Min: "));
		Serial.print(VT->_1725DiscLowLimit);
		Serial.print(F("C   Max: "));
		Serial.print(VT->_1725DiscHighLimit);
		Serial.println(F("C"));
	}

	// Charger control block
	Serial.println(F("Charger control Data"));
	Serial.print(F("    Slow Chrg TO: "));
	Serial.print(VT->ChargerCtrl.SlowChrgTimeout);
	Serial.print(F("  Fast Chrg TO: "));
	Serial.print(VT->ChargerCtrl.FastChrgTimeout);
	Serial.print(F("  Slow/Fast Volt: "));
	Serial.print(VT->ChargerCtrl.SlowFastVolt);
	Serial.print(F("mv  Recharge Volt: "));
	Serial.print(VT->ChargerCtrl.RechargeVolt);
	Serial.println(F("mv"));
	Serial.print(F("    Abnormal Curr: "));
	Serial.print(VT->ChargerCtrl.AbnormalCurr);
	Serial.print(F("ma  SC High Fault: "));
	Serial.print(VT->ChargerCtrl.SlowChrgHighFaultCurr);
	Serial.print(F("ma  SC Low Fault: "));
	Serial.print(VT->ChargerCtrl.SlowChrgLowFaultCurr);
	Serial.println(F("ma"));
	
	Serial.print(F("Nearly Done: "));
	Serial.print(VT->ChargerCtrl.NearlyDone);
	Serial.print(F("ma  Done: "));
	Serial.print(VT->ChargerCtrl.Done);
	Serial.print(F("ma  Hysteresis: "));
	Serial.println(VT->ChargerCtrl.Hysteresis);

	// JEITA data
	Serial.println(F("JEITA Data block"));

	Serial.print(F("    Data type: "));
	Serial.print(VT->JEITA.DataType);
	Serial.print(F("   Stop temp: "));
	Serial.print(VT->JEITA.StopTemp);
	Serial.print(F("   Rec min temp: "));
	Serial.print(VT->JEITA.RecMinTemp);
	Serial.print(F("   Rec max temp: "));
	Serial.println(VT->JEITA.RecMaxTemp);

	for (cnt = 0; cnt < 5; ++cnt)
	{
		Serial.print(F("    Range"));
		Serial.print(cnt);
		Serial.print(F(":  Start temp: "));
		Serial.print(VT->JEITA.TempRange[cnt].StartTemp);

		for (cnt1 = 0; cnt1 < 3; ++cnt1)
		{
			Serial.print(F("   V"));
			Serial.print(cnt1);
			Serial.print(F(": "));
			Serial.print(VT->JEITA.TempRange[cnt].Voltage[cnt1]);
			Serial.print(F(" C"));
			Serial.print(cnt1);
			Serial.print(F(": "));
			Serial.print(VT->JEITA.TempRange[cnt].Current[cnt1]);
		}

		Serial.println();
	}

	// 660 Data
	Serial.println(F("660 Data Block"));
	if (VT->_660Data.ElimFlag)
		Serial.print(F("    Is"));
	else
		Serial.print(F("    Isn't"));
	Serial.println(F(" a battery eliminator"));
	Serial.print(F("    Min startup volt: "));
	Serial.print(VT->_660Data.MinStartupVolt);
	Serial.print(F("  Therm cof: 0x"));
	Serial.print(VT->_660Data.ThermCoff[2],HEX);
	Serial.print(VT->_660Data.ThermCoff[1],HEX);
	Serial.println(VT->_660Data.ThermCoff[0],HEX);
	Serial.print(F("    UVLO: "));
	Serial.print(VT->_660Data.UVLO);
	Serial.print(F("mv  OVLO: "));
	Serial.print(VT->_660Data.OVLO);
	Serial.print(F("mv  Cutoff: "));
	Serial.print(VT->_660Data.CutoffVolt);
	Serial.println(F("mv"));

	// Cell identifing data
	Serial.print(F("Cell ident vendor code: "));
	Serial.write(VT->Ident.Vendor);
	Serial.print(F("  Data: "));
	for (cnt = 0; cnt < 30; ++cnt)
		if ((VT->Ident.IdentData[cnt] < 32 || VT->Ident.IdentData[cnt] > 126) && VT->Ident.IdentData[cnt])
			buf[cnt] = '?';
		else
		buf[cnt] = VT->Ident.IdentData[cnt];
	buf[cnt] = 0;
	Serial.println(buf);

	// Ship mode
	// Is there data?
	if (VT->ShipMode.LowVoltage != 0
		|| VT->ShipMode.HighVoltage != 0
		|| VT->ShipMode.LowCapacity != 0
		|| VT->ShipMode.HighCapacity != 0)
	{  // Yup
		Serial.print(F("Ship mode Voltage Low: "));
		Serial.print(VT->ShipMode.LowVoltage);
		Serial.print(F("mv  Voltage High: "));
		Serial.print(VT->ShipMode.HighVoltage);
		Serial.print(F("mv  Capacity Low: "));
		Serial.print(VT->ShipMode.LowCapacity);
		Serial.print(F("%  Capacity High: "));
		Serial.print(VT->ShipMode.HighCapacity);
		Serial.println(F("%"));
	}
	else // Nope
		Serial.println(F("No ship mode data found"));

	// Health records
	Serial.print(F("Health 1 record type: "));
	Serial.print(VT->Health1.RecordType);
	Serial.print(F("  Health byte: "));
	Serial.println(VT->Health1.Health);
	Serial.print(F("Health 2 record type: "));
	Serial.print(VT->Health1.RecordType);
	Serial.print(F("  Health byte: "));
	Serial.println(VT->Health1.Health);

	// Agg charge
	Serial.print(F("Agg charge 1 Partial: "));
	Serial.print(VT->AggCharge1.Partial);
	Serial.print(F("%  Cycles: "));
	Serial.println(VT->AggCharge1.Cycles);
	Serial.print(F("Agg charge 2 Partial: "));
	Serial.print(VT->AggCharge2.Partial);
	Serial.print(F("%  Cycles: "));
	Serial.println(VT->AggCharge2.Cycles);
#endif
}

// Show Pollux data
void ShowPollux(void)
{
#ifdef ZEBRA_MODE
	uint32_t PN;
	uint16_t SN, letter;
	uint16_t Rev, Suffix;
	char SNbuf[10];

	if (!PP_Valid)
	{
		Serial.println(F("No valid PP data to show"));
		return;
	}

	Serial.print(F("Format Rev: "));
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

		Serial.print(F("Symbol Part number: "));
		Serial.print(PN);
		Serial.print(F("-"));
		Serial.print(Suffix);
		Serial.print(F(" Rev:"));
		if (Rev <= 20)
			Serial.println(Rev);
		else if (Rev <= 46)
			Serial.println((char)('A'+(Rev-21)));
		else
			Serial.println(F("???"));
	}
	else
	{ // Use Extended part number
		Serial.print(F("Extended Part number: "));
		Serial.println((char *)(EEPROM_Data+304));
	}

	// Calculate serial number
	PN = (uint32_t)EEPROM_Data[9] << 24;
	PN |= (uint32_t)EEPROM_Data[8] << 16;
	PN |= (uint32_t)EEPROM_Data[7] << 8;
	PN |= (uint32_t)EEPROM_Data[6];
	letter = PN / 10000;
	SN = PN - (10000*letter);
	Serial.print(F("Serial Num: "));
	sprintf(SNbuf,"%c%04d",letter+'A',PN);
	Serial.print(SNbuf);

	Serial.print(F("   Date Made: "));
	Serial.print(EEPROM_Data[12]>>4);
	Serial.print(F("/"));
	Serial.print(EEPROM_Data[10]);
	Serial.print(F("/"));
	Rev = EEPROM_Data[12] & 0x0f;
	Rev <<= 8;
	Rev |= EEPROM_Data[11];
	Serial.println(Rev);

	Rev = (uint16_t)EEPROM_Data[13] << 8;
	Rev |= (uint16_t)EEPROM_Data[14];
	Serial.print(F("Capacity: "));
	Serial.print(Rev);
	Serial.print(F("maHr   Low: "));
	Serial.print(EEPROM_Data[15]);
	Serial.print(F("%   Very Low: "));
	Serial.print(EEPROM_Data[16]);
	Serial.print(F("%   Critical: "));
	Serial.print(EEPROM_Data[226]);
	Serial.println(F("%"));

	// Show 1725 data
	if (EEPROM_Data[289] != 0)
		Serial.println(F("Unknown 1725 record type!!!"));
	else
	{
		Serial.print(F("Discharge Temp Min: "));
		Serial.print((int8_t)EEPROM_Data[290]);
		Serial.print(F("C   Max: "));
		Serial.print((int8_t)EEPROM_Data[291]);
		Serial.println(F("C"));
	}
#endif
}

// Show Hawkeye data
void ShowHawkeye(void)
{
#ifdef ZEBRA_MODE
	uint16_t Rev;
	int cnt, cnt1;

	if (!PP_Valid)
	{
		Serial.println(F("No valid PP data to show"));
		return;
	}

	Serial.print(F("Format Rev: "));
	Serial.println(EEPROM_Data[1]);

	Serial.print(F("Symbol Part number: "));
	Serial.println((char *)(EEPROM_Data+4));
	Serial.print(F("Serial Num: "));
	Serial.println((char *)(EEPROM_Data+24));

	Serial.print(F("Date Made: "));
	Serial.print(EEPROM_Data[38]>>4);
	Serial.print(F("/"));
	Serial.print(EEPROM_Data[36]);
	Serial.print(F("/"));
	Rev = EEPROM_Data[38] & 0x0f;
	Rev <<= 8;
	Rev |= EEPROM_Data[37];
	Serial.println(Rev);

	Rev = (uint16_t)EEPROM_Data[40] << 8;
	Rev |= (uint16_t)EEPROM_Data[41];
	Serial.print(F("Capacity: "));
	Serial.print(Rev);
	Serial.print(F("maHr   Low: "));
	Serial.print(EEPROM_Data[42]);
	Serial.print(F("%   Very Low: "));
	Serial.print(EEPROM_Data[43]);
	Serial.print(F("%   Critical: "));
	Serial.print(EEPROM_Data[197]);
	Serial.println(F("%"));
	Serial.println();

	// Show 1725 data
	if (EEPROM_Data[245] != 0)
		Serial.println(F("Unknown 1725 record type!!!"));
	else
	{
		Serial.print(F("1725 data: Discharge Temp Min: "));
		Serial.print((int8_t)EEPROM_Data[246]);
		Serial.print(F("C   Max: "));
		Serial.print((int8_t)EEPROM_Data[247]);
		Serial.println(F("C"));
	}

	// Health records
	Serial.print(F("Health record type: "));
	Serial.print(EEPROM_Data[253]);
	Serial.print(F("  Health byte: "));
	Serial.println(EEPROM_Data[257]);

	// Show cycles
	Serial.print(F("Agg charge1 cycles: "));
	Rev = (uint16_t)EEPROM_Data[201] << 8;
	Rev |= (uint16_t)EEPROM_Data[200];
	Serial.print(Rev);
	Serial.print(F("   Agg charge2 cycles: "));
	Rev = (uint16_t)EEPROM_Data[209] << 8;
	Rev |= (uint16_t)EEPROM_Data[208];
	Serial.println(Rev);

	Serial.println();
	Serial.println(F("JEITA Data block"));

	Serial.print(F("    Data type: "));
	Serial.print(EEPROM_Data[288]);
	Serial.print(F("   Stop temp: "));
	Serial.print(EEPROM_Data[290]);
	Serial.print(F("   Rec min temp: "));
	Serial.print(EEPROM_Data[291]);
	Serial.print(F("   Rec max temp: "));
	Serial.println(EEPROM_Data[292]);

	for (cnt = 0; cnt < 5; ++cnt)
	{
		Serial.print(F("    Range"));
		Serial.print(cnt);
		Serial.print(F(":  Start temp: "));
		Serial.print(EEPROM_Data[304+16*cnt]);

		for (cnt1 = 0; cnt1 < 3; ++cnt1)
		{
			Serial.print(F("   V"));
			Serial.print(cnt1);
			Serial.print(F(": "));
			Serial.print(*(uint16_t *)(EEPROM_Data+305+16*cnt+2*cnt1));
			Serial.print(F(" C"));
			Serial.print(cnt1);
			Serial.print(F(": "));
			Serial.print(*(uint16_t *)(EEPROM_Data+311+16*cnt+2*cnt1));
		}
		Serial.println();
	}
#endif
}

void ShowPPP(void)
{
#ifdef ZEBRA_MODE
	uint8_t *cpnt, bak;
	uint16_t tmp;
	char msg[33];

	if (!PPP_Valid)
	{
		Serial.println(F("No valid PP+ data to show"));
		return;
	}

	Serial.print(F("Rated: "));
	Serial.print(batteryRatedCapacity);
	Serial.print(F("ma  Faire: "));
	Serial.print(BD.Gifted.Faire);
	Serial.print(F("%  Unfaire: "));
	Serial.print(BD.Gifted.Unfaire);
	Serial.print(F("%%  Low: "));
	Serial.print(BD.Gifted.Low);
	Serial.println(F("%"));

	Serial.print(F("SlowCur: "));
	Serial.print(BD.Gifted.SlowCharge_mA);
	Serial.print(F("ma  Slow/Fast: "));
	Serial.print(BD.Gifted.SlowFastCharge_mV);
	Serial.print(F("mv  FastCur: "));
	Serial.print(BD.Gifted.FastCharge_mA);
	Serial.print(F("ma  MaxCVolt: "));
	Serial.print(BD.Gifted.ChargeUp_mV);
	Serial.println(F("mv"));

	Serial.print(F("Discharge Temp Min: "));
	Serial.print(BD.Gifted.DischargeMin_C);
	Serial.print(F("C   Max: "));
	Serial.print(BD.Gifted.DischargeMax_C);
	Serial.println(F("C"));

	Serial.print(F("Nearly: "));
	Serial.print(BD.Gifted.NearlyCharged_mA);
	Serial.print(F("ma  Done: "));
	Serial.print(BD.Gifted.ChargeTerm_mA);
	Serial.print(F("ma  SlowMin: "));
	Serial.print(BD.Gifted.ChargeSlowMins);
	Serial.print(F("   FastMin: "));
	Serial.println(BD.Gifted.ChargeFastMins);

	Serial.print(F("SOH Threshold: "));
	Serial.print(BD.Gifted.Dyn1Block[0].HealthPct);
	Serial.print(F("%  SOH: "));
	Serial.print(BD.Gifted.SOH);
	Serial.println(F("%"));

	Serial.print(F("Cycles: "));
	Serial.print(BD.Gifted.CHGCC);
	Serial.print(F("   TC: "));
	Serial.print(BD.Gifted.CC_Threshold * BD.Gifted.CHGCC + BD.Gifted.LCCA);
	Serial.print(F("maH   ZTC: "));
	Serial.print(BD.Gifted.Dyn1Block[0].BatteryLifeData);
	Serial.println(F("maHr"));

	Serial.print(F("Part#: "));
	Serial.println(batteryPartNumber);
	Serial.print(F("PartEx#: "));
	Serial.println(batteryPartNumberEx);
	Serial.print(F("PartNew#: "));
	Serial.println(batteryPartNumberNew);
	Serial.print(F("Serial#: "));
	Serial.println(batteryID);
	Serial.print(F("Manf Date: "));
	Serial.print(mmonth);
	Serial.print(F("/"));
	Serial.print(mday);
	Serial.print(F("/"));
	Serial.println(myear);

	Serial.print(F("Capacity: "));
	Serial.print(BD.Gifted.SOC);
	Serial.print(F("%   Temperature: "));
	Serial.println(CurTemp);

	Serial.print(F("Secs since first use: "));
	Serial.print(dwSecSinceFirstUse);
	Serial.print(F("  In days: "));
	Serial.println(dwSecSinceFirstUse/24/3600);

	// Check extended temp charging data if it's there
	if (EX_CHARGE_TEMP_BLOCKS != (BD.Gifted.Block_Allocation_EEPROM & EX_CHARGE_TEMP_BLOCKS))  // Check if any of those blocks are marked as used
	{
		uint8_t cnt, cnt1;

		Serial.println();
		Serial.println(F("JEITA Data block"));

		Serial.print(F("    Data type: "));
		Serial.print(BD.Gifted.ExCharge.DataType);
		Serial.print(F("   Stop temp: "));
		Serial.print(BD.Gifted.ExCharge.StopTemp);
		Serial.print(F("   Rec min temp: "));
		Serial.print(BD.Gifted.ExCharge.RecMinTemp);
		Serial.print(F("   Rec max temp: "));
		Serial.println(BD.Gifted.ExCharge.RecMaxTemp);

		for (cnt = 0; cnt < 5; ++cnt)
		{
			Serial.print(F("    Range"));
			Serial.print(cnt);
			Serial.print(F(":  Start temp: "));
			Serial.print(BD.Gifted.ExCharge.TempRange[cnt].StartTemp);

			for (cnt1 = 0; cnt1 < 3; ++cnt1)
			{
				Serial.print(F("   V"));
				Serial.print(cnt1);
				Serial.print(F(": "));
				Serial.print(BD.Gifted.ExCharge.TempRange[cnt].Voltage[cnt1]);
				Serial.print(F(" C"));
				Serial.print(cnt1);
				Serial.print(F(": "));
				Serial.print(BD.Gifted.ExCharge.TempRange[cnt].Current[cnt1]);
			}

			Serial.println();
		}
	}

	// Check for SD660 block being used
	if (!(BD.Gifted.Block_Allocation_EEPROM & SD660_BLOCK))
	{  // block is used
		Serial.println();
		Serial.println(F("SD660 Data block"));

		cpnt = (uint8_t *)&BD.Gifted;
		cpnt += SD660blkaddr; // Set to beginning of SD660 block

		if (cpnt[0])
			Serial.println(F("Is a battery eliminator"));
		else
			Serial.println(F("Not a battery eliminator"));

		Serial.print(F("Minimum startup voltage: "));
		tmp = *(uint16_t *)(cpnt+1);
		Serial.print(tmp);
		Serial.println(F("mv"));

		Serial.print(F("UVLO: "));
		tmp = *(uint16_t *)(cpnt+6);
		Serial.print(tmp);
		Serial.println(F("mv"));

		Serial.print(F("OVLO: "));
		tmp = *(uint16_t *)(cpnt+8);
		Serial.print(tmp);
		Serial.println(F("mv"));

		Serial.print(F("Cutoff voltage: "));
		tmp = *(uint16_t *)(cpnt+10);
		Serial.print(tmp);
		Serial.println(F("mv"));
	}

	// Check for cell identifying blocks being used
	if (!(BD.Gifted.Block_Allocation_EEPROM & CELL_IDENT_BLOCKS))
	{  // blocks are used
		Serial.println();
		Serial.println(F("Cell identifying data"));

		cpnt = (uint8_t *)&BD.Gifted;
		cpnt += CELL_IDENT1blkaddr; // Set to beginning of ident blocks
		
		// Mask non-printable chars
		for (tmp = 1; tmp < 32; ++tmp)
			if ((cpnt[tmp] < 32 || cpnt[tmp] > 126) && cpnt[tmp])
				cpnt[tmp] = '?';
		bak = cpnt[tmp];  // Save first byte of Ship mode data
		cpnt[tmp] = 0;  // Terminate ident string just in case

		Serial.print(F("Cell vendor code: "));
		msg[0] = cpnt[1];
		msg[1] = 0;
		Serial.println(msg);

		Serial.print(F("Identifying Data: "));
		Serial.println((char *)(cpnt+2));

		cpnt[tmp] = bak;  // Restore first byte of Ship mode data
	}

	// Check for ship mode block being used
	if (!(BD.Gifted.Block_Allocation_EEPROM & SHIP_MODE_BLOCK))
	{  // block is used
		Serial.println();
		Serial.println(F("Ship mode data"));

		// Is there data?
		if (BD.Gifted.ShipMode.LowVoltage != 0
			|| BD.Gifted.ShipMode.HighVoltage != 0
			|| BD.Gifted.ShipMode.LowCapacity != 0
			|| BD.Gifted.ShipMode.HighCapacity != 0)
		{  // Yup
			Serial.print(F("Voltage Low: "));
			Serial.print(BD.Gifted.ShipMode.LowVoltage);
			Serial.print(F("mv  Voltage High: "));
			Serial.print(BD.Gifted.ShipMode.HighVoltage);
			Serial.println(F("mv"));
			Serial.print(F("Capacity Low: "));
			Serial.print(BD.Gifted.ShipMode.LowCapacity);
			Serial.print(F("%  Capacity High: "));
			Serial.print(BD.Gifted.ShipMode.HighCapacity);
			Serial.println(F("%"));
		}
		else // Nope
			Serial.println(F("No ship mode data found"));
	}

#endif
}

void ShowM200regs(void)
{
#ifdef ZEBRA_MODE
	uint8_t *cpnt;
	uint8_t tmp, cnt;
	char msg[10];

	GG_GetStuff(&BD.Gifted);

	Serial.print(F("GG Device Type: "));
	sprintf(msg,"0x%04X",BD.Gifted.GG_DeviceType);
	Serial.println(msg);
	Serial.print(F("GG Firmware Version: "));
	sprintf(msg,"0x%04X",BD.Gifted.GG_FirmwareVer);
	Serial.println(msg);
	Serial.print(F("GG Hardware Version: "));
	sprintf(msg,"0x%04X",BD.Gifted.GG_HardwareVer);
	Serial.println(msg);
	Serial.println();

	if (GG_ReadReg())
		return;

	Serial.println(F("     Control/Status"));
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			tmp = 0;
		else
			tmp = 1;

		Serial.print(tmp ? F("High:  ") : F(" Low:  "));

		if (((BD.Gifted.CONTROL_STATUS & 0x8000) ^ (cnt << 15)))
			Serial.print(F("SE "));
		else
			Serial.print(F("   "));

		if (((BD.Gifted.CONTROL_STATUS & 0x4000) ^ (cnt << 14)))
			Serial.print(F("FAS "));
		else
			Serial.print(F("    "));

		if (((BD.Gifted.CONTROL_STATUS & 0x2000) ^ (cnt << 13)))
			Serial.print(F("SS "));
		else
			Serial.print(F("   "));

		if (((BD.Gifted.CONTROL_STATUS & 0x1000) ^ (cnt << 12)))
			Serial.print(F("CSV "));
		else
			Serial.print(F("    "));

		if ((BD.Gifted.CONTROL_STATUS & 0x800) ^ (cnt << 11))
			Serial.print(F("CCA "));
		else
			Serial.print(F("    "));

		if ((BD.Gifted.CONTROL_STATUS & 0x400) ^ (cnt << 10))
			Serial.print(F("BCA "));
		else
			Serial.print(F("    "));

		if ((BD.Gifted.CONTROL_STATUS & 0x200) ^ (cnt << 9))
			Serial.print(F("IBAW "));
		else
			Serial.print(F("     "));

		if (((BD.Gifted.CONTROL_STATUS & 0x80) ^ (cnt << 7)))
			Serial.print(F("SHUTDOWN "));
		else
			Serial.print(F("         "));

		if (((BD.Gifted.CONTROL_STATUS & 0x40) ^ (cnt << 6)))
			Serial.print(F("HIBERNATE "));
		else
			Serial.print(F("          "));

		if (((BD.Gifted.CONTROL_STATUS & 0x20) ^ (cnt << 5)))
			Serial.print(F("FULLSLEEP "));
		else
			Serial.print(F("          "));

		if (((BD.Gifted.CONTROL_STATUS & 0x10) ^ (cnt << 4)))
			Serial.print(F("SLEEP "));
		else
			Serial.print(F("      "));

		if ((BD.Gifted.CONTROL_STATUS & 0x08) ^ (cnt << 3))
			Serial.print(F("LDMD "));
		else
			Serial.print(F("     "));

		if ((BD.Gifted.CONTROL_STATUS & 0x04) ^ (cnt << 2))
			Serial.print(F("RUP_DIS "));
		else
			Serial.print(F("        "));

		if ((BD.Gifted.CONTROL_STATUS & 0x02) ^ (cnt << 1))
			Serial.print(F("VOK "));
		else
			Serial.print(F("    "));

		if ((BD.Gifted.CONTROL_STATUS & 0x01) ^ cnt)
			Serial.print(F("QEN "));
		else
			Serial.print(F("    "));

		Serial.println();
	}
	Serial.println();
	Serial.print(F("Temp: "));
	Serial.print(CurTemp);
	Serial.print(F("Deg C   Volt: "));
	Serial.print(BD.Gifted.VOLT);
	Serial.print(F("mv   Curr: "));
	Serial.print(BD.Gifted.AI);
	Serial.println(F("ma"));
	Serial.println();
	Serial.println("     Flags");
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			tmp = 0;
		else
			tmp = 1;

		Serial.print(tmp ? F("High:  ") : F(" Low:  "));

		if (((BD.Gifted.FLAGS & 0x8000) ^ (cnt << 15)))
			Serial.print(F("OTC "));
		else
			Serial.print(F("    "));

		if (((BD.Gifted.FLAGS & 0x4000) ^ (cnt << 14)))
			Serial.print(F("OTD "));
		else
			Serial.print(F("    "));

		if ((BD.Gifted.FLAGS & 0x800) ^ (cnt << 11))
			Serial.print(F("CHG_INH "));
		else
			Serial.print(F("        "));

		if ((BD.Gifted.FLAGS & 0x400) ^ (cnt << 10))
			Serial.print(F("XCHG "));
		else
			Serial.print(F("     "));

		if ((BD.Gifted.FLAGS & 0x200) ^ (cnt << 9))
			Serial.print(F("FC "));
		else
			Serial.print(F("   "));

		if ((BD.Gifted.FLAGS & 0x100) ^ (cnt << 8))
			Serial.print(F("CHG "));
		else
			Serial.print(F("    "));

		if (((BD.Gifted.FLAGS & 0x80) ^ (cnt << 7)))
			Serial.print(F("RMFCC_EN "));
		else
			Serial.print(F("         "));

		if (((BD.Gifted.FLAGS & 0x40) ^ (cnt << 6)))
			Serial.print(F("OCV_TAKEN "));
		else
			Serial.print(F("          "));

		if (((BD.Gifted.FLAGS & 0x20) ^ (cnt << 5)))
			Serial.print(F("OCV_PRED "));
		else
			Serial.print(F("         "));

		if ((BD.Gifted.FLAGS & 0x04) ^ (cnt << 2))
			Serial.print(F("SOC1 "));
		else
			Serial.print(F("     "));

		if ((BD.Gifted.FLAGS & 0x02) ^ (cnt << 1))
			Serial.print(F("SOCF "));
		else
			Serial.print(F("     "));

		if ((BD.Gifted.FLAGS & 0x01) ^ cnt)
			Serial.print(F("DSG "));
		else
			Serial.print(F("    "));

		Serial.println();
	}
	Serial.println();

	Serial.println(F("    Capacities"));
	Serial.print(F("Nominal Avail:"));
	Serial.print(BD.Gifted.NAC);
	Serial.print(F("  Full Avail:"));
	Serial.print(BD.Gifted.FAC);
	Serial.print(F("  Remaining:"));
	Serial.print(BD.Gifted.RM);
	Serial.print(F("  Full Chrg:"));
	Serial.print(BD.Gifted.FCC);
	Serial.print(F("  Design Cap:"));
	Serial.println(DesignCap);

	Serial.print(F("Health:"));
	Serial.print(BD.Gifted.SOH);
	Serial.print(F("%  Capacity:"));
	Serial.print(BD.Gifted.SOC);
	Serial.println(F("%"));
	Serial.println();

	Serial.print(F("Secs since first use: "));
	Serial.print(dwSecSinceFirstUse);
	Serial.print(F("  In days: "));
	Serial.println(dwSecSinceFirstUse/24/3600);

	Serial.print(F("Valid Qmax cycles:"));
	Serial.println(BD.Gifted.QVC);
	Serial.println();
#endif
}

void ShowM200regsHex(void)
{
#ifdef ZEBRA_MODE
	uint16_t *tmp;
	uint8_t cnt;
	char msg[10];

	if (GG_ReadReg())
		return;

	tmp = &BD.Gifted.CONTROL_STATUS;

	for (cnt = 0; cnt <= 30; ++cnt)
	{
		sprintf(msg,"%04X ",*(tmp+cnt));
		Serial.print(msg);
		if ((cnt & 0x07) == 0x07)
		Serial.println();
	}
	Serial.println();

#endif
}

void Show27Z561regs(void)
{
#ifdef ZEBRA_MODE
	uint8_t *cpnt;
	uint8_t tmp, cnt, cnt2;
	char msg[80];
	uint8_t regs[NEW_GB_CMD_FULL_READ_SIZE];
	uint16_t val;

	newGG_GetStuff(&BD.NG);

	Serial.print(F("GG Device Type: "));
	sprintf(msg,"0x%04X",BD.NG.GG_DeviceType);
	Serial.println(msg);
	Serial.println(F("GG Firmware: "));
	Serial.print(F("    Device: "));
	sprintf(msg,"%02X%02X",BD.NG.GG_FirmwareVer[0],BD.NG.GG_FirmwareVer[1]);
	Serial.println(msg);
	Serial.print(F("    Version: "));
	sprintf(msg,"%02X%02X",BD.NG.GG_FirmwareVer[2],BD.NG.GG_FirmwareVer[3]);
	Serial.println(msg);
	Serial.print(F("    Build: "));
	sprintf(msg,"%02X%02X",BD.NG.GG_FirmwareVer[4],BD.NG.GG_FirmwareVer[5]);
	Serial.println(msg);
	Serial.print(F("    Type: "));
	sprintf(msg,"%02X",BD.NG.GG_FirmwareVer[6]);
	Serial.println(msg);
	Serial.print(F("    IT Ver: "));
	sprintf(msg,"%02X%02X",BD.NG.GG_FirmwareVer[7],BD.NG.GG_FirmwareVer[8]);
	Serial.println(msg);
	
	Serial.print(F("GG Hardware Version: "));
	sprintf(msg,"0x%04X",BD.NG.GG_HardwareVer);
	Serial.println(msg);
	Serial.println();

	if (newGG_ReadReg(regs))
		return;

	Serial.println(F("     Control/Status"));
	val = regs[0] | (regs[1] << 8);
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			tmp = 0;
		else
			tmp = 1;

		Serial.print(tmp ? F("High:  ") : F(" Low:  "));

		if (((val & 0x4000) ^ (cnt << 14)))
			Serial.print(F("FAS "));
		else
			Serial.print(F("    "));

		if (((val & 0x2000) ^ (cnt << 13)))
			Serial.print(F("SS "));
		else
			Serial.print(F("   "));

		if ((val & 0x200) ^ (cnt << 9))
			Serial.print(F("CSum "));
		else
			Serial.print(F("     "));
		if (((val & 0x80) ^ (cnt << 7)))
			Serial.print(F("LDMD "));
		else
			Serial.print(F("     "));

		if (((val & 0x40) ^ (cnt << 6)))
			Serial.print(F("RDIS "));
		else
			Serial.print(F("     "));

		if ((val & 0x02) ^ (cnt << 1))
			Serial.print(F("VOK "));
		else
			Serial.print(F("    "));

		if ((val & 0x01) ^ cnt)
			Serial.print(F("QEN "));
		else
			Serial.print(F("    "));

		Serial.println();
	}
	Serial.println();
	Serial.println("     Battery Status");
	val = regs[NGG_FLAGS] | (regs[NGG_FLAGS+1] << 8);
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			tmp = 0;
		else
			tmp = 1;

		Serial.print(tmp ? F("High:  ") : F(" Low:  "));

		if (((val & 0x4000) ^ (cnt << 14)))
			Serial.print(F("TCA "));
		else
			Serial.print(F("    "));

		if ((val & 0x800) ^ (cnt << 11))
			Serial.print(F("TDA "));
		else
			Serial.print(F("    "));

		if ((val & 0x200) ^ (cnt << 9))
			Serial.print(F("RCA "));
		else
			Serial.print(F("    "));

		if (((val & 0x80) ^ (cnt << 7)))
			Serial.print(F("INIT "));
		else
			Serial.print(F("     "));

		if (((val & 0x40) ^ (cnt << 6)))
			Serial.print(F("DSG "));
		else
			Serial.print(F("    "));

		if (((val & 0x20) ^ (cnt << 5)))
			Serial.print(F("FC "));
		else
			Serial.print(F("   "));

		if ((val & 0x10) ^ (cnt << 4))
			Serial.print(F("FD "));
		else
			Serial.print(F("   "));

		Serial.println();
	}
	Serial.println();
	Serial.print(F("Batt Temp: "));
	Serial.print(CurTemp);
	Serial.print(F("Deg C   Int Temp: "));
	Serial.print(CurIntTemp);
	Serial.print(F("Deg C   Volt: "));
	Serial.print(regs[NGG_VOLTAGE] | (regs[NGG_VOLTAGE+1] << 8));
	Serial.print(F("mv   Curr: "));
	Serial.print(regs[NGG_CURRENT] | (regs[NGG_CURRENT+1] << 8));
	Serial.println(F("ma"));

	Serial.println();
	Serial.println(F("    Capacities"));
	Serial.print(F("Remaining: "));
	Serial.print(regs[NGG_RM] | (regs[NGG_RM+1] << 8));
	Serial.print(F("  Full Chrg: "));
	Serial.print(regs[NGG_FCC] | (regs[NGG_FCC+1] << 8));
	Serial.print(F("  Design Cap: "));
	Serial.println(regs[NGG_DCAP] | (regs[NGG_DCAP+1] << 8));

	Serial.print(F("Health: "));
	Serial.print(regs[NGG_SOH] | (regs[NGG_SOH+1] << 8));
	Serial.print(F("%  Capacity: "));
	Serial.print(regs[NGG_RSOC] | (regs[NGG_RSOC+1] << 8));
	Serial.print(F("%  Cycles: "));
	Serial.print(regs[NGG_CYCLES] | (regs[NGG_CYCLES+1] << 8));
	Serial.print(F("  QMAX Cycles: "));
	Serial.println(regs[NGG_QMAX_CYCLES] | (regs[NGG_QMAX_CYCLES+1] << 8));
	Serial.println();

	// Time at temp/capacity
	Serial.println(F("Time in zones in days"));
	Serial.println(F("     >=95%  >=90%  >=80%  >=50%  >=20%  >=10%   >=5%   >=0%"));
	Serial.print(F("UT "));
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
	{
		sprintf(msg,"%7ld",BD.NG.times[0][cnt2]/(3600*24));
		Serial.print(msg);
	}
	Serial.println();
	Serial.print(F("LT "));
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
	{
		sprintf(msg,"%7ld",BD.NG.times[1][cnt2]/(3600*24));
		Serial.print(msg);
	}
	Serial.println();
	Serial.print(F("SLT"));
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
	{
		sprintf(msg,"%7ld",BD.NG.times[2][cnt2]/(3600*24));
		Serial.print(msg);
	}
	Serial.println();
	Serial.print(F("RT "));
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
	{
		sprintf(msg,"%7ld",BD.NG.times[3][cnt2]/(3600*24));
		Serial.print(msg);
	}
	Serial.println();
	Serial.print(F("STH"));
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
	{
		sprintf(msg,"%7ld",BD.NG.times[4][cnt2]/(3600*24));
		Serial.print(msg);
	}
	Serial.println();
	Serial.print(F("HT "));
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
	{
		sprintf(msg,"%7ld",BD.NG.times[5][cnt2]/(3600*24));
		Serial.print(msg);
	}
	Serial.println();
	Serial.print(F("OT "));
	for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
	{
		sprintf(msg,"%7ld",BD.NG.times[6][cnt2]/(3600*24));
		Serial.print(msg);
	}
	Serial.println();

/* ***FIX***
	Serial.print(F("Secs since first use: "));
	Serial.print(dwSecSinceFirstUse);
	Serial.print(F("  In days: "));
	Serial.println(dwSecSinceFirstUse/24/3600);
*/
#endif
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
	if (NEW_GAUGE)  //*** FIX ***//
		return;
	else
		ShowM200regsHex();
}
