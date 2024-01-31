#include <Arduino.h>
#include "BattProg.h"

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
	digitalWrite(PULL_ENB,HIGH);  // Disable pullup voltage

	switch (Voltage)
	{
		case PULLUP_V5:  // Set to 5V
			digitalWrite(PULL_SEL1,HIGH);
			digitalWrite(PULL_SEL0,LOW);
			Serial.println(F("5V pullup voltage"));
			break;
		case PULLUP_VBATT:  // Set to VBatt
			digitalWrite(PULL_SEL1,LOW);
			digitalWrite(PULL_SEL0,HIGH);
			Serial.println(F("VBatt pullup voltage"));
			break;
		case PULLUP_V3p3:  // Set to 3.3V
			digitalWrite(PULL_SEL1,LOW);
			digitalWrite(PULL_SEL0,LOW);
			Serial.println(F("3.3V pullup voltage"));
			break;
		case PULLUP_V0:  // Set to 0V
		default:
			digitalWrite(PULL_SEL1,HIGH);
			digitalWrite(PULL_SEL0,HIGH);
			Serial.println(F("0V pullup voltage"));
			break;
	}

	digitalWrite(PULL_ENB,LOW);  // Enable pullup voltage
}

void SetResistor(uint8_t Resistor)
{
	if (0 == Resistor)
	{
		Serial.println(F("All reistors off!"));
		digitalWrite(SDA_EN0,LOW);
		digitalWrite(SDA_EN1,LOW);
		digitalWrite(SDA_EN2,LOW);
		return;
	}

	if (Resistor & 1)
	{
		digitalWrite(SDA_EN0,HIGH);
		digitalWrite(SCL_EN0,HIGH);
		Serial.println(F("10K reistor on"));
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
		Serial.println(F("6.8K reistor on"));
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
		Serial.println(F("5.1K reistor on"));
	}
	else
	{
		digitalWrite(SDA_EN2,LOW);
		digitalWrite(SCL_EN2,LOW);
	}
}

void ActiveClk(void)
{
	digitalWrite(PAS_CLK_EN,LOW);  // Disable passive clock drive
	digitalWrite(ACT_CLK_EN,HIGH);  // Enable active clock drive
	Serial.println(F("Active clock drive on"));
}

void NoClk(void)
{
	digitalWrite(PAS_CLK_EN,LOW);  // Disable passive clock drive
	digitalWrite(ACT_CLK_EN,LOW);  // Disable active clock drive
	Serial.println(F("No clock drive on"));
}

void PassiveClk(void)
{
	digitalWrite(ACT_CLK_EN,LOW);  // Disable active clock drive
	digitalWrite(PAS_CLK_EN,HIGH);  // Enable passive clock drive
	Serial.println(F("Passive clock drive on"));
}
