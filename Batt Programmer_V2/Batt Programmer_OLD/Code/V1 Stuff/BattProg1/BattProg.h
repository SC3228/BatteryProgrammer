// Include file for battery programmer

#define ZEBRA_MODE	// Comment out for JDM devices
//#define TEST_MODE	// Uncomment for testing new features

// Function prototypes
uint8_t GetText(char *buf,uint8_t MaxLen=8,uint8_t NoFilter = 0);
void NoData(void);
uint8_t DoChecksum(uint8_t *Start, uint8_t * End);
void FlushBuffer(int ask);
void HEX_Write(uint16_t size, uint8_t *buf = NULL);
void SaveFile(uint16_t size, uint8_t *buf = NULL);
void DoGiftedUpdate(int StartAddr, uint8_t *Buf);
uint8_t DoVTupdate(uint8_t *Buf);
uint8_t DoPolluxUpdate(uint8_t *Buf);
uint8_t DoHawkeyeUpdate(uint8_t *Buf);
void InitPullupPins(void);
void SetVoltage(uint8_t Voltage);
void PrintVoltage();
void SetResistor(uint8_t Resistor);
void PrintResistor();
void ActiveClk(void);
void PassiveClk(void);
void PrintClock(void);
void FlushSerial(void);
void printfSerial(char *fmt, ... );
uint8_t CheckClk(void);
uint8_t ValidatePPP(uint8_t *Buf);
uint8_t ValidatePP(uint8_t *Buf);
uint8_t ValidatePollux(uint8_t *Buf, uint8_t Format);
uint8_t ValidateHawkeye(uint8_t *Buf);
uint8_t ValidateVT(VALUE_TIER_BATT_DATA_t *VT);
uint8_t ValidatePPP_V2(void);
uint8_t KeepGoing();
void ShowMPA2(void);
void ShowPPPV2(void);
void ShowMeteor(void);
void ShowPollux(void);
void ShowHawkeye(void);
void ShowPPP(void);
void ShowGGregs(void);
void ShowGGregsHex(void);

// Common strings in FLASH
const char Unseal[] PROGMEM = "Unseal gas gauge? ARE YOU SURE??? (y/n)?";
#define UNSEAL (const __FlashStringHelper*)Unseal
const char HibErr[] PROGMEM = "Error setting hibernate";
#define HIB_ERR (const __FlashStringHelper*)HibErr
const char BadRead[] PROGMEM = "Bad read!";
#define BAD_READ (const __FlashStringHelper*)BadRead
const char NoGauge[] PROGMEM = "No gas gauge?";
#define NO_GAUGE (const __FlashStringHelper*)NoGauge
const char BadRev[] PROGMEM = "Bad revision!";
#define BAD_REV (const __FlashStringHelper*)BadRev
const char ReadError[] PROGMEM = "Error reading from file!";
#define READ_ERR (const __FlashStringHelper*)ReadError
const char BattDone[] PROGMEM = "Battery is done!";
#define BATT_DONE (const __FlashStringHelper*)BattDone
const char FailedVal[] PROGMEM = "Battery failed validation!!!";
#define FAILED_VAL (const __FlashStringHelper*)FailedVal
const char BattOK[] PROGMEM = "Battery is OK!";
#define BATT_OK (const __FlashStringHelper*)BattOK
const char Unsupported[] PROGMEM = "Unsupported data type!";
#define UNSUPPORTED (const __FlashStringHelper*)Unsupported
const char ok[] PROGMEM = "OK";
#define OK (const __FlashStringHelper*)ok
const char Passed[] PROGMEM = "Passed";
#define PASSED (const __FlashStringHelper*)Passed
const char ChecksumError[] PROGMEM = "Checksum Error";
#define BAD_CHECKSUM (const __FlashStringHelper*)ChecksumError

// Hex file download global variable
extern uint8_t HadError;  // Error flag
extern uint8_t CSum; // Checksum

// Serial character defines
#define ESC		27	// ESC character
#define CR		13	// Carrage return
#define BKSP	8	// Backspace

// Auth serial number string
extern char AuthSN[32];

// JDM/Zebra stuff
extern uint8_t JDM_Mode;

// Selected battery type
extern uint8_t BatteryType;

// Clock control
extern uint8_t SwitchClk;

// Skip auth serial number check flag
extern uint8_t NoCheckSerialNum;

// Intel hex record types
#define	DATA_REC	0	// Data record
#define	EOF_REC		1	// End of file record
#define START_REC	3	// Start segment address record

// Pin defines
#define I2C_CLK		21	// I2C clock pin

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
#define PULLUP_NONE		0	// No pull up resistor

// Key defines
#define NO_KEYS			0	// No keys needed
#define MPA3_KEYS		1	// Use MPA3 keys
#define FALCON_KEYS		2	// Use Falcon keys
#define IRONMAN_KEYS	3	// Use IronMan keys

// Battery chip defs
#define MPA2_EEPROM_ADDR	0x50	// I2C Slave address of MPA2 battery EEPROM
#define MPA2_TMP_ADDR		0x48	// I2C Slave address of MPA2 battery temperature chip
#define MPA2_BM_ADDR		0x15	// I2C Slave address of MPA2 battery Micro
#define MPA2_EEPROM_SIZE	256		// Size of MPA2 EEPROM
#define MPA2_PAGE_SIZE		8		// Write page size
#define POLLUX_EEPROM_SIZE	512		// Size of Pollux EEPROM
#define POLLUX_PAGE_SIZE	8		// Write page size
#define MPA3_EEPROM_ADDR	0x52	// I2C Slave address of MPA3 battery EEPROM
#define MPA3_TMP_ADDR		0x4A	// I2C Slave address of MPA3 battery temperature chip
#define MPA3_GG_ADDR		0x55	// I2C Slave address of MPA3 gas gauge
#define MPA3_EEPROM_SIZE	512		// Size of MPA3 EEPROM
#define MPA3_PAGE_SIZE		16		// Write page size
#define QC_EEPROM_SIZE		1024	// Size of QC combined EEPROM
#define QC_PAGE_SIZE		16		// Write page size
#define AUTH_CHIP_ADDR		0xd0	// Battery pack auth chip address
#define LOCAL_AUTH_ADDR		0xc0	// Local auth chip address
#define MAX_AUTH_SIZE		632		// Auth chip eeprom size
#define AUTH_PAGE_SIZE		32		// Auth chip page size
#define MIN_VOLTAGE			3.3		// Minimum battery voltage for non-parasitic power

// NOTE: Keep the below in sync with the defs in BattProg1.ino!!!
#define NO_BATT				0	// No battery
#define MPA2_BATT			1	// MPA2 smart battery
#define MC95_BATT			2	// MC 95 battery (with external power)
#define MC18_BATT			3	// MC18 battery (MPA3 PP+)
#define ROGUE_BATT			4	// Rouge battery
#define FRENZY_BATT			5	// Frenzy battery
#define IRONMAN_BATT		6	// IronMan battery
#define POLLUX_BATT			7	// Pollux battery
#define	FALCON_BATT			8	// Falcon battery
#define HAWKEYE_BATT		9	// Hawkeye battery
#define SENTRY_BATT			10	// Sentry battery
#define GALACTUS_BATT		11	// Galactus battery
#define FROZONE_EP_BATT		12	// Frozone EEPROM
#define METEOR_BATT			13	// VT battery
#define PPP_V2				14	// New gas gauge battery

#define NEW_GAUGE (PPP_V2 == BatteryType)

// Battery format types for "update"
#define SMART_BATT_DATA			0	// MPA2 style smart/PP battery data
#define GIFTED_BATT_DATA		1	// MPA3 style gifted/PP+ battery data
#define POLLUX_BATT_DATA		2	// QC style PP data
#define POLLUX8956_BATT_DATA	3	// Combined Pollux and 8956 data
#define COMBINED_BATT_DATA		4	// Combined gifted and pollux data
#define HAWKEYE_BATT_DATA		5	// Hawkeye style battery data
#define SD660_BATT_DATA			6	// SD660 QC data format
#define GALACTUS_BATT_DATA		7	// Galactus auth chip only data format
#define METEOR_BATT_DATA		8	// Value tier battery data format
#define SENTRY_BATT_DATA		9	// Sentry mixed EEPROM/Auth chip data format
#define VALUE_TIER_DATA			10	// Value tier data format
#define PPP_V2_DATA				11	// PP+ version 2 data format

#pragma pack(1)
// Battery type data struct
typedef struct
{
	uint8_t EEPROM_Address; 	// 7 Bit address of EEPROM
	uint8_t TempAddress;		// 7 Bit address of the temp chip, if any
	uint8_t GasGaugeAddress;	// 7 Bit address of the gas gauge, if any
	uint8_t CryptoAddress;		// 7 Bit addresss of the crypto chip, if any
	uint8_t UseAuthData;		// Flag for supports auth chip data
	uint16_t Size;				// Size in bytes of the EEPROM data
	uint8_t Page;				// Write page size
	uint8_t Pullup_volt;		// Pullup voltage
	uint8_t Pullup_resistor;	// Pullup resistor values
	uint8_t	ActiveClk;			// Use active clock drive flag
	uint8_t Keys;				// Which encryption keys to use (if any)
	const PROGMEM char* const Name;	// Descriptive name
	float MinVoltage;			// Minimum battery voltage to access EEPROM
} BattData;
#pragma pack()

#define MAX_BATTERY_TYPES 15  // Number of supported battery types

extern BattData Batts[MAX_BATTERY_TYPES];

// EEPROM data buffer
#define MAX_EEPROM_SIZE 1144 // Max size of EEPROM + 120 bytes for extra auth data past 512 bytes
extern uint8_t EEPROM_Data[MAX_EEPROM_SIZE];

// Buffer blank flag
extern uint8_t BufferBlank;

#define MAX_REC_LEN 64  // Handle up to 64 byte records

// I2C defines
#define DO_STOP	(uint8_t)true  // Flag to do a stop after transaction

// Static/Shared data defines
#define PART_NUMBER_LENGTH 		19		// typical Symbol part number 21-65587-01 Rev.05  (18 chars + termination)
#define SER_NUM_LENGTH		 	17		// battery serial number is 16 chars + termination
#define PART_NUMBER_LENGTH_EX   24      // new Symbol part number xx-xxxxxx-xx Rev.xx (19 chars + termination + 3 reserved) To solve SPR 19650
#define PART_NUMBER_LENGTH_NEW	40		// Latest part number, 21 char from gifted plus " Rev:xxx" plus buffer.

extern uint16_t myear;
extern uint8_t	mmonth;
extern uint8_t mday;
extern uint16_t batteryRatedCapacity;
extern char batteryID[SER_NUM_LENGTH+1];		// Serial number
extern char batteryPartNumber[PART_NUMBER_LENGTH+1];	// Symbol's part number
extern char batteryPartNumberEx[PART_NUMBER_LENGTH_EX]; // New Symbol's part number to accomadate 6 digit battery family number
extern char batteryPartNumberNew[PART_NUMBER_LENGTH_NEW]; // Latest part number length for gifted batts
extern BatteryData BD;	// Battery data

extern uint8_t PPP_Valid; // PP+ Battery data valid flag
extern uint8_t PPPV2_Valid; // PP+ V2 Battery data valid flag
extern uint8_t PP_Valid; // PP Battery data valid flag
extern uint8_t VT_Valid;  // VT battery data valid flag

extern uint8_t Verbose;  // Use verbose output flag


