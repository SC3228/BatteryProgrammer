// Battery programmer tester code
//
// Tests battery programer for correct voltages and pullups
// Needs a 10K resistor to SCL on D2, and another to SDA on D3
// Also, A0 goes to SCL and A1 to SDA
// And a .1uf cap on Aref

// Pin defs
#define SCLin 0
#define SCLout 2
#define SDAin 1
#define SDAout 3

// Resistor defs
#define R_CLK 9.79
#define R_SDA 9.91

float Vcc; // Calculated 5V supply

void setup()
{
	Serial.begin(9600);

	// Turn off the digital input buffer on A0 and A1
	DIDR0 = _BV(ADC1D) | _BV(ADC0D);
	
	// Setup the output pins
	pinMode(SCLout,INPUT);
	pinMode(SDAout,INPUT);
}

void loop()
{
	int reading;
	float temp, Vscl, Vsda;
	char msg[80], SCLv[6], SCLr[7], SDAv[6],SDAr[7];

	calibrate();

	// Read pullup voltages
	reading = analogRead(SCLin);
	Vscl = Vcc / 1023.0 * (float)reading;
	dtostrf(Vscl,4,2,SCLv);
	reading = analogRead(SDAin);
	Vsda = Vcc / 1023.0 * (float)reading;
	dtostrf(Vsda,4,2,SDAv);

	// Get pullup resistors
	pinMode(SCLout,OUTPUT);
	pinMode(SDAout,OUTPUT);

	reading = analogRead(SCLin);
	temp = Vcc / 1023.0 * (float)reading;
	temp = (Vscl * R_CLK / temp) - R_CLK;
	dtostrf(temp,5,2,SCLr);
	reading = analogRead(SDAin);
	temp = Vcc / 1023.0 * (float)reading;
	temp = (Vsda * R_SDA / temp) - R_SDA;
	dtostrf(temp,5,2,SDAr);

	pinMode(SCLout,INPUT);
	pinMode(SDAout,INPUT);

	sprintf(msg,"SCL: %sV, %sK      SDA: %sV, %sK",SCLv,SCLr,SDAv,SDAr);
	Serial.println(msg);
	delay(1000);
}

void calibrate()
{
	int reading;

	// Calibrate for 5V level
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

	delay(10); // Wait for Vref to settle

	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA,ADSC)) // measuring
	;

	// Toss the first one
	delay(10);
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA,ADSC)) // measuring
	;

	reading = ADC; // Get result

	// Calculate 5V level
	Vcc = 5.0 / 1023.0 * (float)reading;
	Vcc = 1.1 / Vcc * 5.0;
}