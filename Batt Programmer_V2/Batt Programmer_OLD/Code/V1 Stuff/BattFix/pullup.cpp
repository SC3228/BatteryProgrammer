// Battery programer pullup resistance/voltage control

#include <Arduino.h>

#include "pwmbatt.h"
#include "BattProg.h"
static uint8_t CurVolt = PULLUP_V0;  // Current voltage selection
static uint8_t CurRes = PULLUP_NONE;  // Current resistor selection
static uint8_t CurClk = 0;  // Current clock drive type

void InitPullupPins(void)
{
	digitalWrite(PULL_ENB,HIGH);  // No pullup voltage
	pinMode(PULL_ENB,OUTPUT);
	digitalWrite(PAS_CLK_EN,HIGH);  // Enable passive clk drive
	pinMode(PAS_CLK_EN,OUTPUT);
	digitalWrite(ACT_CLK_EN,LOW);  // Disable active clk drive
	pinMode(ACT_CLK_EN,OUTPUT);
	digitalWrite(PULL_SEL0,HIGH);  // Init pullup slect to 0V
	digitalWrite(PULL_SEL1,HIGH);
	pinMode(PULL_SEL0,OUTPUT);
	pinMode(PULL_SEL1,OUTPUT);
	digitalWrite(SCL_EN0,LOW);  // No SCL resistors
	digitalWrite(SCL_EN1,LOW);
	digitalWrite(SCL_EN2,LOW);
	pinMode(SCL_EN0,OUTPUT);
	pinMode(SCL_EN1,OUTPUT);
	pinMode(SCL_EN2,OUTPUT);
	digitalWrite(SDA_EN0,LOW);  // No SDA resistors
	digitalWrite(SDA_EN1,LOW);
	digitalWrite(SDA_EN2,LOW);
	pinMode(SDA_EN0,OUTPUT);
	pinMode(SDA_EN1,OUTPUT);
	pinMode(SDA_EN2,OUTPUT);
}

void SetVoltage(uint8_t Voltage)
{
	CurVolt = Voltage; // Save new voltage

	digitalWrite(PULL_ENB,HIGH);  // Disable pullup voltage

	switch (Voltage)
	{
		case PULLUP_V5:  // Set to 5V
			digitalWrite(PULL_SEL1,HIGH);
			digitalWrite(PULL_SEL0,LOW);
			break;
		case PULLUP_VBATT:  // Set to VBatt
			digitalWrite(PULL_SEL1,LOW);
			digitalWrite(PULL_SEL0,HIGH);
			break;
		case PULLUP_V3p3:  // Set to 3.3V
			digitalWrite(PULL_SEL1,LOW);
			digitalWrite(PULL_SEL0,LOW);
			break;
		case PULLUP_V0:  // Set to 0V
		default:
			digitalWrite(PULL_SEL1,HIGH);
			digitalWrite(PULL_SEL0,HIGH);
			break;
	}

	digitalWrite(PULL_ENB,LOW);  // Enable pullup voltage
}

void PrintVoltage()
{
	switch (CurVolt)
	{
		case PULLUP_V5:  // Set to 5V
			Serial.println(F("Pullup to 5V"));
			break;
		case PULLUP_VBATT:  // Set to VBatt
			Serial.println(F("Pullup to VBatt"));
			break;
		case PULLUP_V3p3:  // Set to 3.3V
			Serial.println(F("Pullup to 3.3V"));
			break;
		case PULLUP_V0:  // Set to 0V
		default:
			Serial.println(F("Pullup to 0V"));
			break;
	}
}

void SetResistor(uint8_t Resistor)
{
	CurRes = Resistor; // Save new resistor

	if (Resistor < PULLUP_10K)
		Resistor = PULLUP_10K;
	else if (Resistor > PULLUP_2p3K)
		Resistor = PULLUP_10K;

	if (Resistor & 1)
	{
		digitalWrite(SDA_EN0,HIGH);
		digitalWrite(SCL_EN0,HIGH);
	}
	else
	{
		digitalWrite(SDA_EN0,LOW);
		digitalWrite(SCL_EN0,LOW);
	}

	if (Resistor & 2)
	{
		digitalWrite(SDA_EN1,HIGH);
		digitalWrite(SCL_EN1,HIGH);
	}
	else
	{
		digitalWrite(SDA_EN1,LOW);
		digitalWrite(SCL_EN1,LOW);
	}

	if (Resistor & 4)
	{
		digitalWrite(SDA_EN2,HIGH);
		digitalWrite(SCL_EN2,HIGH);
	}
	else
	{
		digitalWrite(SDA_EN2,LOW);
		digitalWrite(SCL_EN2,LOW);
	}
}

void PrintResistor()
{
	switch (CurRes)
	{
		case PULLUP_NONE:
			Serial.println(F("No Pullups "));
			break;
		case PULLUP_2p3K:
			Serial.println(F("2.3K Pullups"));
			break;
		case PULLUP_3p4K:
			Serial.println(F("3.4K Pullups"));
			break;
		case PULLUP_2p9K:
			Serial.println(F("2.9K Pullups"));
			break;
		case PULLUP_4K:
			Serial.println(F("4K Pullups"));
			break;
		case PULLUP_5p1K:
			Serial.println(F("5.1K Pullups"));
			break;
		case PULLUP_6p8K:
			Serial.println(F("6.8K Pullups"));
			break;
		case PULLUP_10K:
			Serial.println(F("10K Pullups"));
			break;
	}
}

void ActiveClk(void)
{
	CurClk = 1;  // Save current clock drive type

	digitalWrite(PAS_CLK_EN,LOW);  // Disable passive clock drive
	digitalWrite(ACT_CLK_EN,HIGH);  // Enable active clock drive
}

void PassiveClk(void)
{
	CurClk = 0;  // Save current clock drive type

	digitalWrite(ACT_CLK_EN,LOW);  // Disable active clock drive
	digitalWrite(PAS_CLK_EN,HIGH);  // Enable passive clock drive
}

void PrintClock(void)
{
	if (CurClk)
		Serial.println(F("Active clock drive"));
	else
		Serial.println(F("Passive clock drive"));
}
