// Tests for battery programmer V2

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "BattProg.h"
#include "Auth.h"

const char *Passed = "Passed\r\n";
static const char *ContESC = "    Hit any key to continue, or ESC to exit with failure...";
static const char *Cont = "    Hit any key to continue...";
static const char *BadVolt = "Check value of resistors R10 & R11, and the 3.3V supply";

void TestAll(void)
{
	RelayOff();
	PassiveClk();  // Disable active drive
	SetVoltage(PULLUP_VBATT);  // VBatt pullup voltage
	SetResistor(PULLUP_NONE); // No pullup resistors

	Serial.println("1. Connect a 6.8K 1% resistor between SCL and gnd terminals");
	Serial.println("2. Connect a 6.8K 1% resistor between SDA and gnd terminals");
	Serial.println("3. Connect a 6.8K 1% resistor between Therm and gnd terminals");
	Serial.println("4. Connect a variable power supply between the Batt+ and Gnd terminals");
	Serial.println("5. Set the power supply to 0V");
	Serial.println();
	Serial.println(Cont);
	WaitForKey();

	// Test thermistor
	if (!TestThermistor(true))
		return;

	// Test battery input
	if (!TestBattInput(true))
		return;

	// Test pull up resistors/voltage selection
	if (!TestPullups(true))
		return;

	// Test status LED
	if (!TestStatusLED())
		return;

	// Test auth chip
	if (!TestAuth())
		return;
	Serial.println();

	// Format drive
	if (!FormatDisk(true))
		return;
}

bool TestThermistor(bool TestAllMode)
{
	int therm;

	if (!TestAllMode)
	{
		Serial.println("Connect a 6.8K 1% resistor between the Therm and Gnd terminals");
		Serial.println();
		Serial.println(Cont);
		WaitForKey();
	}

	Serial.print("\r\nChecking open thermistor connection: ");
	RelayOff();
	therm = ThermResistance();  // Get thermisitor resistance when open
	if (therm < MAX_THERM)
	{
		Serial.print("Failed!!\r\n        ");
		Serial.printf("Open termistor shows %d ohms of resistance\r\nShould be > %d ohms\r\n",MAX_THERM);
		return (false);
	}
	else
		Serial.print(Passed);


	Serial.print("Checking thermistor measurement: ");
	RelayOn();
	therm = ThermResistance();  // Get thermisitor resistance
	RelayOff();
	if (therm > 6460 && therm < 7140) // Check for within 5% of 6.8K ohms
	{
		Serial.print(Passed);
		return (true);
	}
	else
	{
		Serial.print("Failed!!\r\n        ");
		Serial.printf("Termistor shows %d ohms of resistance\r\nShould be 6800 +-5%\r\n",therm);
		Serial.println("Check value of resistor R14");
		return (false);
	}
	return (false);
}

// Mode: 0 - do all tests
//       1 - do open test
//       2 - do connected test
bool TestBattInput(bool TestAllMode)
{
	int volts;

	RelayOff();
	PassiveClk();  // Disable active drive
	SetVoltage(PULLUP_VBATT);  // VBatt pullup voltage
	SetResistor(PULLUP_NONE); // No pullup resistors

	if (!TestAllMode)
	{
		Serial.println("1. Connect a variable power supply between the Batt+ and Gnd terminals");
		Serial.println("2. Set the power supply to 0V");
		Serial.println();
		Serial.println(Cont);
		WaitForKey();
	}

	// Check 0V level
	Serial.print("\r\nChecking open battery connection: ");
	volts = BattVoltage();
	if (volts < 100) // Check for within 100mv of 0V
		Serial.println(Passed);
	else
	{
		Serial.print("Failed!!\r\n        ");
		Serial.printf("Battery voltage was %dmv, should be below 100mv\r\n",volts);
		Serial.println("Check value of resistor R11");
		return (false);
	}

	// Check 2.5V level
	Serial.println("\r\nAdjust supply to 2.5VDC");
	Serial.println();
	Serial.println(Cont);
	WaitForKey();
	Serial.print("\r\nChecking for 2.5V on battery terminal: ");
	volts = BattVoltage();
	if (volts > 2450 && volts < 2550) // Check for within 2% of 2.5V
		Serial.println(Passed);
	else
	{
		Serial.print("Failed!!\r\n        ");
		Serial.printf("Battery voltage was %d.%dV, should be 2.5V +- 2%\r\n",volts/1000,volts%1000);
		Serial.println(BadVolt);
		RelayOff();
		return (false);
	}

	// Check 9V level
	Serial.println("\r\nAdjust supply to 9.0VDC");
	Serial.println(Cont);
	WaitForKey();
	Serial.print("\r\nChecking for 9.0V on battery terminal: ");
	volts = BattVoltage();
	RelayOff();  // Last test
	Serial.printf("Battery voltage was %d.%dV, test ",volts/1000,volts%1000);
	if (volts > 8820 && volts < 9180) // Check for within 2% of 9V
		Serial.println(Passed);
	else
	{
		Serial.print("Failed!!\r\n        ");
		Serial.printf("Battery voltage was %d.%dV, should be 4V +- 2%\r\n",volts/1000,volts%1000);
		Serial.println(BadVolt);
		return (false);
	}

	// Check 4V level
	Serial.println("\r\nAdjust supply to 4.0VDC");
	Serial.println(Cont);
	WaitForKey();
	Serial.print("\r\nChecking for 4.0V on battery terminal: ");
	volts = BattVoltage();
	if (volts > 3920 && volts < 4080) // Check for within 2% of 4V
		Serial.println(Passed);
	else
	{
		Serial.print("Failed!!\r\n        ");
		Serial.printf("Battery voltage was %d.%dV, should be 4V +- 2%\r\n",volts/1000,volts%1000);
		Serial.println(BadVolt);
		RelayOff();
		return (false);
	}

	return (true);
}

bool TestAuth(void)
{
	AuthChipType Chip;
	uint8_t LockValue, LockConfig;

	RelayOff();
	SetVoltage(PULLUP_V3p3);  // 3V pullup voltage
	SetResistor(PULLUP_10K); // 10K pullup resistors
	PassiveClk();  // Passive clock drive

	// Get chip type
	Serial.print("Checking auth chip type: ");
	if (AUTH_NO_TYPE == (Chip = GetChipType())) // Check for valid chip type
		return (false);

	switch (Chip) {
	case AUTH_508A:
		Serial.println("ATECC508A");
		break;
	case AUTH_608A:
		Serial.println("ATECC608A");
		break;
	default:
		Serial.println("Unknown???");
		break;
	}

	// Is config locked?
	Serial.print("Checking for locked config: ");
	if (!GetLockBytes(&LockValue,&LockConfig)) // Get lock bytes
		return (false);

	if (0x55 == LockValue && 0x55 == LockConfig)
	{  // Not locked, do config
		Serial.println("Unlocked");

		if (!AUTH_Config(Chip))  // Config the part
			return (false);
	}
	else
		Serial.println("Locked");

	// Check config
	return (CheckCryptoConfig(1,Chip));
}

bool TestPullups(bool TestAllMode)
{
	bool ret = true;

	if (!TestAllMode)
	{
		Serial.println("1. Connect a 6.8K 1% resistor from SCL to Gnd");
		Serial.println("2. Connect a second 6.8K 1% resistor from SDA to Gnd");
		Serial.println("3. Connect a variable power supply set to 4VDC to the Batt+ input");
		Serial.println();
		Serial.println(Cont);
		WaitForKey();
	}
	RelayOn();

	// Check active clock drive
	SetVoltage(PULLUP_V3p3);  // 3V pullup voltage
	SetResistor(PULLUP_NONE); // No pullup resistors
	ActiveClk(); // Enable active drive
	Serial.println("\r\nMeasure voltage on SCL. Should be 3.3V +/- .1V");
	Serial.println(ContESC);
	if (WaitForKey() == ESC)
	{
		ret = false;
		goto exit;
	}

	// Check no pullups
	PassiveClk();  // Disable active drive
	Serial.println("\r\nMeasure voltage on SCL & SDA. Should be 0.84V +/- .1V");
	Serial.println(ContESC);
	if (WaitForKey() == ESC)
	{
		ret = false;
		goto exit;
	}

	// Check 10K pullups
	SetResistor(PULLUP_10K);
	Serial.println("\r\nMeasure voltage on SCL & SDA. Should be 1.67V +/- .1V");
	Serial.println(ContESC);
	if (WaitForKey() == ESC)
	{
		ret = false;
		goto exit;
	}

	// Check 6.8K pullups
	SetResistor(PULLUP_6p8K);
	Serial.println("\r\nMeasure voltage on SCL & SDA. Should be 1.89V +/- .1V");
	Serial.println(ContESC);
	if (WaitForKey() == ESC)
	{
		ret = false;
		goto exit;
	}

	// Check 5.1K pullups
	SetResistor(PULLUP_5p1K);
	Serial.println("\r\nMeasure voltage on SCL & SDA. Should be 2.07V +/- .1V");
	Serial.println(Cont);
	if (WaitForKey() == ESC)
	{
		ret = false;
		goto exit;
	}

	// Check for switch to VBatt
	SetVoltage(PULLUP_VBATT);
	Serial.println("\r\nMeasure voltage on SCL & SDA. Should be 2.41V +/- .1V");
	Serial.println(Cont);
	if (WaitForKey() == ESC)
	{
		ret = false;
		goto exit;
	}

exit:
	RelayOff();
	PassiveClk();  // Disable active drive
	SetVoltage(PULLUP_VBATT);  // Default to VBatt pullup voltage
	SetResistor(PULLUP_NONE); // Default to no pullup resistors

	return (ret);
}

bool TestVoltageSelect(void)
{
	bool ret = true;

	Serial.println("1. Connect a variable power supply set to 4VDC to the Batt+ input");
	Serial.println("2. Disconnect any external resistors from the SDA and SCL pins");
	Serial.println();
	Serial.println(ContESC);
	if (WaitForKey() == ESC)
	{
		ret = false;
		goto exit;
	}

	SetResistor(PULLUP_10K);  // set to 10K pullups and 3.3V pull up voltage
	SetVoltage(PULLUP_V3p3);  // 3V pullup voltage
	RelayOn();

	// Check passive clock drive
	PassiveClk();
	Serial.println("Measure the voltage on both SCL and SDA. It should be 3.3V +/- .1V");
	Serial.println(ContESC);
	if (WaitForKey() == ESC)
	{
		ret = false;
		goto exit;
	}

	SetVoltage(PULLUP_VBATT);
	Serial.println("Measure the voltage on both SCL and SDA. It should be 4V +/- .1V");
	Serial.println(ContESC);
	if (WaitForKey() == ESC)
	{
		ret = false;
		goto exit;
	}

exit:
	RelayOff();
	PassiveClk();  // Disable active drive
	SetVoltage(PULLUP_VBATT);  // Default to VBatt pullup voltage
	SetResistor(PULLUP_NONE); // Default to no pullup resistors

	return (ret);
}

bool TestStatusLED(void)
{
	bool ret = true;

	Serial.println("Checking status LED: ");
	Serial.println("        Verify that the status LED is cycling thru red, green, and blue colors");
	Serial.println();
	Serial.println(ContESC);
	do
	{
		STATUS_LED_RED
		delay(700);
		STATUS_LED_GREEN
		delay(700);
		STATUS_LED_BLUE
		delay(700);
	} while(!Serial.available() && Serial.dtr());

	STATUS_LED_ONLINE
	if (ESC == Serial.read())
		ret = false;

	FlushSerial();
	return (ret);
}

