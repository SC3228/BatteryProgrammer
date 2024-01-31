// Include file for battery programmer

// Status LED defines
extern Adafruit_NeoPixel statusLED;
#define STATUS_LED_ONLINE	{statusLED.setPixelColor(0,0,0,200);statusLED.show();}
#define STATUS_LED_WAITING	{statusLED.setPixelColor(0,200,200,0);statusLED.show();}
#define STATUS_LED_READY	{statusLED.setPixelColor(0,0,200,0);statusLED.show();}
#define STATUS_LED_BUSY		{statusLED.setPixelColor(0,200,0,0);statusLED.show();}
#define STATUS_LED_OFF		{statusLED.setPixelColor(0,0,0,0);statusLED.show();}
#define STATUS_LED_RED		{statusLED.setPixelColor(0,255,0,0);statusLED.show();}
#define STATUS_LED_GREEN	{statusLED.setPixelColor(0,0,255,0);statusLED.show();}
#define STATUS_LED_BLUE		{statusLED.setPixelColor(0,0,0,255);statusLED.show();}

// Function prototypes
void USBdiskInit(void);
void RefreshDrive(void);
uint8_t GetText(char *buf,uint8_t MaxLen=8,uint8_t NoFilter = 0);
void InitPullupPins(void);
void SetVoltage(uint8_t Voltage);
void PrintVoltage();
void SetResistor(uint8_t Resistor);
void PrintResistor();
void ActiveClk(void);
void PassiveClk(void);
void PrintClock(void);
void FlushSerial(void);
int WaitForKey(void);

// Common strings in FLASH
//const char ok[] PROGMEM = "OK";
//#define OK (const __FlashStringHelper*)ok
//const char Passed[] PROGMEM = "Passed";
//#define PASSED (const __FlashStringHelper*)Passed

// Serial character defines
#define ESC		27	// ESC character
#define CR		13	// Carrage return
#define BKSP	8	// Backspace

// Auth serial number string
extern char AuthSN[32];

// Pin defines
#define RELAY_CTRL	12	// Relay control pin, high activates relays
#define I2C_CLK		22	// I2C clock pin

#define PULL_ENB	6	// Enable pull up voltages, active low

#define PULL_SEL	4	// Pull up voltage select line, 0 = VBatt, 1 = 3.3V

#define PU_EN0		5	// Enable for 10K pullup, active high
#define PU_EN1		6	// Enable for 6.8K pullup, active high
#define PU_EN2		9	// Enable for 5.1K pullup, active high
#define PAS_CLK_EN	11	// SCL enable for passive clock drive, active high
#define ACT_CLK_EN	10	// SCL enable for active clock drive, active high

#define BATT_PLUS	A0	// Analog input to read battery voltage
#define THERM		A1	// Analog input for thermistor

// Pullup voltage selects
#define PULLUP_V3p3		0	// 3.3 Volts
#define PULLUP_VBATT	1	// Battery voltage

// Pullup resistor selects
#define PULLUP_2p3K		7	// 2.3K pullup resistor
#define PULLUP_2p9K		6	// 2.9K pullup resistor
#define PULLUP_3p4K		5	// 2.3K pullup resistor
#define PULLUP_4K		3	// 4K pullup resistor
#define PULLUP_5p1K		4	// 5.1K pullup resistor
#define PULLUP_6p8K		2	// 6.8K pullup resistor
#define PULLUP_10K		1	// 10K pullup resistor
#define PULLUP_NONE		0	// No pull up resistor

// Key defines
#define NO_KEYS			0	// No keys needed
#define MPA3_KEYS		1	// Use MPA3 keys
#define FALCON_KEYS		2	// Use Falcon keys
#define IRONMAN_KEYS	3	// Use IronMan keys

#define LOCAL_AUTH_ADDR		0xc0	// Local auth chip address
