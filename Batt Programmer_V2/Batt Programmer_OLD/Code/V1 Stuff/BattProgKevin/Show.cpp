// Code to display battery data

#include <Arduino.h>

#include "pwmbatt.h"
#include "BattProg.h"
#include "GasGauge.h"

void ShowPP(void)
{
#ifdef ZEBRA_MODE
	uint32_t PN;
	uint16_t Rev, Suffix;

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
	Serial.print(F("Serial Num: "));
	Serial. print(PN);

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
#endif
}

void ShowPPP(void)
{
#ifdef ZEBRA_MODE
	uint8_t *cpnt;
	uint16_t tmp;
	char msg[10];

	if (!PPP_Valid)
	{
		Serial.println(F("No valid PP+ data to show"));
		return;
	}

	Serial.print(F("Rated: "));
	Serial.print(batteryRatedCapacity);
	Serial.print(F("ma  Faire: "));
	Serial.print(g_GiftedBattData.Faire);
	Serial.print(F("%  Unfaire: "));
	Serial.print(g_GiftedBattData.Unfaire);
	Serial.print(F("%%  Low: "));
	Serial.print(g_GiftedBattData.Low);
	Serial.println(F("%"));

	Serial.print(F("SlowCur: "));
	Serial.print(g_GiftedBattData.SlowCharge_mA);
	Serial.print(F("ma  Slow/Fast: "));
	Serial.print(g_GiftedBattData.SlowFastCharge_mV);
	Serial.print(F("mv  FastCur: "));
	Serial.print(g_GiftedBattData.FastCharge_mA);
	Serial.print(F("ma  MaxCVolt: "));
	Serial.print(g_GiftedBattData.ChargeUp_mV);
	Serial.println(F("mv"));

	Serial.print(F("Nearly: "));
	Serial.print(g_GiftedBattData.NearlyCharged_mA);
	Serial.print(F("ma  Done: "));
	Serial.print(g_GiftedBattData.ChargeTerm_mA);
	Serial.print(F("ma  SlowMin: "));
	Serial.print(g_GiftedBattData.ChargeSlowMins);
	Serial.print(F("   FastMin: "));
	Serial.println(g_GiftedBattData.ChargeFastMins);

	Serial.print(F("SOH Threshold: "));
	Serial.print(g_GiftedBattData.Dyn1Block[0].HealthPct);
	Serial.print(F("%  SOH: "));
	Serial.print(g_GiftedBattData.SOH);
	Serial.println(F("%"));

	Serial.print(F("Cycles: "));
	Serial.print(g_GiftedBattData.CHGCC);
	Serial.print(F("   TC: "));
	Serial.print(g_GiftedBattData.CC_Threshold * g_GiftedBattData.CHGCC + g_GiftedBattData.LCCA);
	Serial.print(F("maH   ZTC: "));
	Serial.print(g_GiftedBattData.Dyn1Block[0].BatteryLifeData);
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
	Serial.print(g_GiftedBattData.SOC);
	Serial.print(F("%   Temperature: "));
	Serial.println(CurTemp);

	Serial.print(F("Secs since first use: "));
	Serial.print(dwSecSinceFirstUse);
	Serial.print(F("  In days: "));
	Serial.println(dwSecSinceFirstUse/24/3600);


	// Check for SD660 block being used
	if (!(g_GiftedBattData.Block_Allocation_EEPROM & SD660_BLOCK))
	{  // block is used
		Serial.println();
		Serial.println(F("SD660 Data block"));

		cpnt = (uint8_t *)&g_GiftedBattData;
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
	if (!(g_GiftedBattData.Block_Allocation_EEPROM & CELL_IDENT_BLOCKS))
	{  // blocks are used
		Serial.println();
		Serial.println(F("Cell identifying data"));

		cpnt = (uint8_t *)&g_GiftedBattData;
		cpnt += CELL_IDENT1blkaddr; // Set to beginning of ident blocks

		Serial.print(F("Cell vendor code: "));
		msg[0] = cpnt[1];
		msg[1] = 0;
		Serial.println(msg);

		Serial.print(F("Identifying Data: "));
		Serial.println((char *)(cpnt+2));
	}
#endif
}

void ShowGGregs(void)
{
#ifdef ZEBRA_MODE
	uint8_t *cpnt;
	uint8_t tmp, cnt;
	char msg[10];

	if (GG_ReadReg())
		return;

	Serial.println(F("     Control/Status"));
	Serial.println(F("High Byte"));
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			tmp = 0;
		else
			tmp = 1;

		Serial.print(tmp ? F("High:  ") : F(" Low:  "));

		if (((g_GiftedBattData.CONTROL_STATUS & 0x8000) ^ (cnt << 15)))
			Serial.print(F("SE "));
		else
			Serial.print(F("   "));

		if (((g_GiftedBattData.CONTROL_STATUS & 0x4000) ^ (cnt << 14)))
			Serial.print(F("FAS "));
		else
			Serial.print(F("    "));

		if (((g_GiftedBattData.CONTROL_STATUS & 0x2000) ^ (cnt << 13)))
			Serial.print(F("SS "));
		else
			Serial.print(F("   "));

		if (((g_GiftedBattData.CONTROL_STATUS & 0x1000) ^ (cnt << 12)))
			Serial.print(F("CSV "));
		else
			Serial.print(F("    "));

		if ((g_GiftedBattData.CONTROL_STATUS & 0x800) ^ (cnt << 11))
			Serial.print(F("CCA "));
		else
			Serial.print(F("    "));

		if ((g_GiftedBattData.CONTROL_STATUS & 0x400) ^ (cnt << 10))
			Serial.print(F("BCA "));
		else
			Serial.print(F("    "));

		if ((g_GiftedBattData.CONTROL_STATUS & 0x200) ^ (cnt << 9))
			Serial.print(F("IBAW "));
		else
			Serial.print(F("     "));

		Serial.println();
	}

	Serial.println(F("Low Byte"));
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			tmp = 0;
		else
			tmp = 1;

		Serial.print(tmp ? F("High:  ") : F(" Low:  "));

		if (((g_GiftedBattData.CONTROL_STATUS & 0x80) ^ (cnt << 7)))
			Serial.print(F("SHUTDOWN "));
		else
			Serial.print(F("         "));

		if (((g_GiftedBattData.CONTROL_STATUS & 0x40) ^ (cnt << 6)))
			Serial.print(F("HIBERNATE "));
		else
			Serial.print(F("          "));

		if (((g_GiftedBattData.CONTROL_STATUS & 0x20) ^ (cnt << 5)))
			Serial.print(F("FULLSLEEP "));
		else
			Serial.print(F("          "));

		if (((g_GiftedBattData.CONTROL_STATUS & 0x10) ^ (cnt << 4)))
			Serial.print(F("SLEEP "));
		else
			Serial.print(F("      "));

		if ((g_GiftedBattData.CONTROL_STATUS & 0x08) ^ (cnt << 3))
			Serial.print(F("LDMD "));
		else
			Serial.print(F("     "));

		if ((g_GiftedBattData.CONTROL_STATUS & 0x04) ^ (cnt << 2))
			Serial.print(F("RUP_DIS "));
		else
			Serial.print(F("        "));

		if ((g_GiftedBattData.CONTROL_STATUS & 0x02) ^ (cnt << 1))
			Serial.print(F("VOK "));
		else
			Serial.print(F("    "));

		if ((g_GiftedBattData.CONTROL_STATUS & 0x01) ^ cnt)
			Serial.print(F("QEN "));
		else
			Serial.print(F("    "));

		Serial.println();
	}
	Serial.println();
	Serial.print(F("Temp: "));
	Serial.print(CurTemp);
	Serial.print(F("Deg C   Volt: "));
	Serial.print(g_GiftedBattData.VOLT);
	Serial.println(F("mv"));
	Serial.println();
	Serial.println("     Flags");
	Serial.println(F("High Byte"));
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			tmp = 0;
		else
			tmp = 1;

		Serial.print(tmp ? F("High:  ") : F(" Low:  "));

		if (((g_GiftedBattData.FLAGS & 0x8000) ^ (cnt << 15)))
			Serial.print(F("OTC "));
		else
			Serial.print(F("    "));

		if (((g_GiftedBattData.FLAGS & 0x4000) ^ (cnt << 14)))
			Serial.print(F("OTD "));
		else
			Serial.print(F("    "));

		if ((g_GiftedBattData.FLAGS & 0x800) ^ (cnt << 11))
			Serial.print(F("CHG_INH "));
		else
			Serial.print(F("        "));

		if ((g_GiftedBattData.FLAGS & 0x400) ^ (cnt << 10))
			Serial.print(F("XCHG "));
		else
			Serial.print(F("     "));

		if ((g_GiftedBattData.FLAGS & 0x200) ^ (cnt << 9))
			Serial.print(F("FC "));
		else
			Serial.print(F("   "));

		if ((g_GiftedBattData.FLAGS & 0x100) ^ (cnt << 8))
			Serial.print(F("CHG "));
		else
			Serial.print(F("    "));

		Serial.println();
	}

	Serial.println(F("Low Byte"));
	for (cnt = 0; cnt < 2; ++cnt)
	{
		if (cnt)
			tmp = 0;
		else
			tmp = 1;

		Serial.print(tmp ? F("High:  ") : F(" Low:  "));

		if (((g_GiftedBattData.FLAGS & 0x80) ^ (cnt << 7)))
			Serial.print(F("RMFCC_EN "));
		else
			Serial.print(F("         "));

		if (((g_GiftedBattData.FLAGS & 0x40) ^ (cnt << 6)))
			Serial.print(F("OCV_TAKEN "));
		else
			Serial.print(F("          "));

		if (((g_GiftedBattData.FLAGS & 0x20) ^ (cnt << 5)))
			Serial.print(F("OCV_PRED "));
		else
			Serial.print(F("         "));

		if ((g_GiftedBattData.FLAGS & 0x04) ^ (cnt << 2))
			Serial.print(F("SOC1 "));
		else
			Serial.print(F("     "));

		if ((g_GiftedBattData.FLAGS & 0x02) ^ (cnt << 1))
			Serial.print(F("SOCF "));
		else
			Serial.print(F("     "));

		if ((g_GiftedBattData.FLAGS & 0x01) ^ cnt)
			Serial.print(F("DSG "));
		else
			Serial.print(F("    "));

		Serial.println();
	}
	Serial.println();

	Serial.println(F("    Capacities"));
	Serial.print(F("Nominal Avail:"));
	Serial.print(g_GiftedBattData.NAC);
	Serial.print(F("  Full Avail:"));
	Serial.print(g_GiftedBattData.NAC);
	Serial.print(F("  Remaining:"));
	Serial.print(g_GiftedBattData.RM);
	Serial.print(F("  Full Chrg:"));
	Serial.println(g_GiftedBattData.FCC);
	Serial.println();

	Serial.print(F("Health:"));
	Serial.print(g_GiftedBattData.SOH);
	Serial.print(F("%  Capacity:"));
	Serial.print(g_GiftedBattData.SOC);
	Serial.println(F("%"));
	Serial.println();

	Serial.print(F("Secs since first use: "));
	Serial.print(dwSecSinceFirstUse);
	Serial.print(F("  In days: "));
	Serial.println(dwSecSinceFirstUse/24/3600);

	Serial.print(F("Valid Qmax cycles:"));
	Serial.println(g_GiftedBattData.QVC);
	Serial.println();
#endif
}

void ShowGGregsHex(void)
{
#ifdef ZEBRA_MODE
	uint16_t *tmp;
	uint8_t cnt;
	char msg[10];

	if (GG_ReadReg())
		return;

	tmp = &g_GiftedBattData.CONTROL_STATUS;

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
