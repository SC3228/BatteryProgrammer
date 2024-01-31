// Include file for battery programmer

// Function prototypes
void InitPullupPins(void);
void SetVoltage(uint8_t Voltage);
void SetResistor(uint8_t Resistor);
void ActiveClk(void);
void PassiveClk(void);
void NoClk(void);
void AUTH_Read(void);
void AUTH_Config(void);
void AUTH_Dump(void);
int GetString(uint8_t *buf, uint8_t maxlen);
void AUTH_EnterSerial();

// Pin defines
#define PULL_ENB	7	// Enable pull up voltages, active low

#define PULL_SEL0	9	// Pull up voltage select lines
#define PULL_SEL1	8	// 00 - 3.3V, 01 - Batt+, 10 - 5V, 11 - 0V

#define SCL_EN0		17	// SCL enable for 10K pullup, active high
#define SCL_EN1		32	// SCL enable for 6.8K pullup, active high
#define SCL_EN2		34	// SCL enable for 5.1K pullup, active high
#define PAS_CLK_EN	18	// SCL enable for passive clock drive, active high

#define SDA_EN0		5	// SDA enable for 10K pullup, active high
#define SDA_EN1		3	// SDA enable for 6.8K pullup, active high
#define SDA_EN2		2	// SDA enable for 5.1K pullup, active high
#define ACT_CLK_EN	4	// SCL enable for active clock drive, active high

#define SD_CS		53	// SD chip select pin
#define WP			12	// SD card write protect signal, active low
#define CD			11	// SD card detect signal, active low

#define BATT_PLUS	A0	// Analog input to read battery voltage

// Pullup voltage selects
#define PULLUP_V0		3	// 0 Volts
#define PULLUP_V3p3		0	// 3.3 Volts
#define PULLUP_VBATT	1	// Battery voltage
#define PULLUP_V5		2	// 5 volts

// Pullup resistor selects
#define PULLUP_2p3K		7	// 2.3K pullup resistor
#define PULLUP_2p9K		6	// 2.9K pullup resistor
#define PULLUP_3p4K		5	// 2.3K pullup resistor
#define PULLUP_4K		3	// 4K pullup resistor
#define PULLUP_5p1K		4	// 5.1K pullup resistor
#define PULLUP_6p8K		2	// 6.8K pullup resistor
#define PULLUP_10K		1	// 10K pullup resistor

// Serial character defines
#define ESC		27	// ESC character
#define CR		13	// Carrage return
#define BKSP	8	// Backspace


