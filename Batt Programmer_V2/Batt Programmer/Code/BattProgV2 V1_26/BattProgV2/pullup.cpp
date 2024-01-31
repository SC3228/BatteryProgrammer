// Battery programer pullup resistance/voltage control

#include <Arduino.h>
#include "Adafruit_SPIFlash.h"

#include "pwmbatt.h"
#include "BattProg.h"
static uint8_t CurVolt = PULLUP_VBATT;  // Current voltage selection
static uint8_t CurRes = PULLUP_NONE;  // Current resistor selection
static uint8_t CurClk = 0;  // Current clock drive type

void InitPullupPins(void)
{
	digitalWrite(PAS_CLK_EN,HIGH);  // Enable passive clk drive
	pinMode(PAS_CLK_EN,OUTPUT);
	digitalWrite(ACT_CLK_EN,LOW);  // Disable active clk drive
	pinMode(ACT_CLK_EN,OUTPUT);
	digitalWrite(PULL_SEL,LOW);  // Init pullup slect to VBatt
	pinMode(PULL_SEL,OUTPUT);
	digitalWrite(PU_EN0,LOW);  // No pullup resistors
	digitalWrite(PU_EN1,LOW);
	digitalWrite(PU_EN2,LOW);
	pinMode(PU_EN0,OUTPUT);
	pinMode(PU_EN1,OUTPUT);
	pinMode(PU_EN2,OUTPUT);
}

void SetVoltage(uint8_t Voltage)
{
	CurVolt = Voltage; // Save new voltage

	switch (Voltage)
	{
		case PULLUP_V3p3:  // Set to 3.3V
			digitalWrite(PULL_SEL,HIGH);
			break;
		case PULLUP_VBATT:  // Set to VBatt
		default:
			digitalWrite(PULL_SEL,LOW);
			break;
	}
}

void PrintVoltage()
{
	switch (CurVolt)
	{
		case PULLUP_VBATT:  // Set to VBatt
			Serial.println("Pullup to VBatt");
			break;
		case PULLUP_V3p3:  // Set to 3.3V
			Serial.println("Pullup to 3.3V");
			break;
		default:
			Serial.println("Pullup to ???V");
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
		digitalWrite(PU_EN0,HIGH);
	else
		digitalWrite(PU_EN0,LOW);

	if (Resistor & 2)
		digitalWrite(PU_EN1,HIGH);
	else
		digitalWrite(PU_EN1,LOW);

	if (Resistor & 4)
		digitalWrite(PU_EN2,HIGH);
	else
		digitalWrite(PU_EN2,LOW);
}

void PrintResistor()
{
	switch (CurRes)
	{
		case PULLUP_NONE:
			Serial.println("No Pullups ");
			break;
		case PULLUP_2p3K:
			Serial.println("2.3K Pullups");
			break;
		case PULLUP_3p4K:
			Serial.println("3.4K Pullups");
			break;
		case PULLUP_2p9K:
			Serial.println("2.9K Pullups");
			break;
		case PULLUP_4K:
			Serial.println("4K Pullups");
			break;
		case PULLUP_5p1K:
			Serial.println("5.1K Pullups");
			break;
		case PULLUP_6p8K:
			Serial.println("6.8K Pullups");
			break;
		case PULLUP_10K:
			Serial.println("10K Pullups");
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
		Serial.println("Active clock drive");
	else
		Serial.println("Passive clock drive");
}
