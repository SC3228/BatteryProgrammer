// Include file for battery programmer
#include <Adafruit_NeoPixel.h>

//#define ZEBRA_MODE	// Comment out for JDM devices
//#define TEST_MODE	// Uncomment for testing new features

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
#define COMET_BATT_DATA			10	// Value tier data format
#define PPP_V2_DATA				11	// PP+ version 2 data format

// Function prototypes
void not_sup(void);
void ClearValid(void);
void SyncFS(void);
void USBdiskInit(void);
void RefreshDrive(void);
void FormatDisk(void);
void FlushSerial(void);
uint8_t GetText(char *buf,uint8_t MaxLen=8,uint8_t NoFilter = 0);
void NoData(void);
uint8_t DoChecksum(uint8_t *Start, uint8_t * End);
void FlushBuffer(int ask);
uint8_t GetHexFile(char *name, uint16_t size, bool doStatus=true);
void HEX_Write(uint16_t size, uint8_t *buf = NULL);
void SaveFile(uint16_t size, uint8_t *buf = NULL);
void DoGiftedUpdate(int StartAddr, uint8_t *Buf);
uint8_t DoV2update(uint8_t *Buf);
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
uint8_t CheckClk(void);
uint8_t ValidatePPP(uint8_t *Buf);
uint8_t ValidatePP(uint8_t *Buf);
uint8_t ValidateVTeeprom(uint8_t *Buf);
uint8_t ValidatePollux(uint8_t *Buf, uint8_t Format);
uint8_t ValidateHawkeye(uint8_t *Buf);
uint8_t ValidateVT(VALUE_TIER_BATT_DATA_t *VT, uint8_t Format=METEOR_BATT_DATA);
uint8_t ValidatePPP_V2(bool HaveGauge=true);
uint8_t KeepGoing();
void ShowMPA2(void);
void ShowPPPV2(void);
void ShowMeteor(uint8_t Format=METEOR_BATT_DATA);
void ShowPollux(void);
void ShowHawkeye(void);
void ShowPPP(void);
void ShowGGregs(void);
void ShowGGregsHex(void);
int GG_UpdateStatus(void);
void GG_Recover();
void ResetFS(void);

// Common strings in FLASH
const char OnLine[] = "\r\nOn line# %d - ";
const char Unseal[] = "Unseal gas gauge? ARE YOU SURE??? (y/n)?";
const char HibErr[] = "Error setting hibernate";
const char BadRead[] = "Bad read!";
const char NoGauge[] = "No gas gauge?";
const char BadRev[] = "Bad revision!";
const char ReadError[] = "Error reading from file!";
const char BattDone[] = "Battery is done!";
const char FailedVal[] = "Battery failed validation!!!";
const char FailedEEPROM[] = "Battery EEPROM failed validation!!!";
const char BattOK[] = "Battery is ok!";
const char EEPROMok[] = "EEPROM data is ok!";
const char Unsupported[] = "Unsupported data type!";
const char ok[] = "ok";
const char Passed[] = "Passed";
const char ChecksumError[] = "Checksum Error";
const char HexReadErr[] = "Error reading hex digits!";
const char FSopenErr[] = "Error opening FS file!";
const char UnknownCmd[] = "Unknown command %02X on line %d\r\n";
const char TooMuchData[] = "Too much data!\r\n";
const char SyntaxError[] = "Syntax error on line %d\r\n";

// File stuff
// Onboard flash file system
extern FatFileSystem fatfs;
#define MAX_NAME_LEN 42
#define LF 10  // Line feed code for Unix line endings
#define MAX_FS_DATA 42  // Max number of data bytes for a .fs file line

// Serial character defines
#define ESC		27	// ESC character
#define CR		13	// Carrage return
#define BKSP	8	// Backspace

// Status LED defines
extern Adafruit_NeoPixel statusLED;
#define STATUS_LED_ONLINE	{statusLED.setPixelColor(0,0,0,128);statusLED.show();}
#define STATUS_LED_WAITING	{statusLED.setPixelColor(0,128,128,0);statusLED.show();}
#define STATUS_LED_READY	{statusLED.setPixelColor(0,0,128,0);statusLED.show();}
#define STATUS_LED_BUSY		{statusLED.setPixelColor(0,128,0,0);statusLED.show();}
#define STATUS_LED_OFF		{statusLED.setPixelColor(0,0,0,0);statusLED.show();}

// Auth serial number, hardware rev
extern uint16_t AuthSN, AuthHW;

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
#define RELAY_CTRL	12	// Relay control pin, high activates relays
#define I2C_CLK		22	// I2C clock pin

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
#define VT_EEPROM_SIZE		16384	// Size of VT 6490 EEPROM
#define VT_PAGE_SIZE		32		// Size of VT 6490 EEPROM write page
#define AUTH_CHIP_ADDR		0xd0	// Battery pack auth chip address
#define LOCAL_AUTH_ADDR		0xc0	// Local auth chip address
#define AUTH_EEPROM_SIZE	416		// Auth chip eeprom size
#define MAX_AUTH_SIZE		632		// Auth chip eeprom size
#define AUTH_PAGE_SIZE		32		// Auth chip page size
#define GG_ROM_MODE			0x0B	// Gas gauge stuck in ROM mode
#define MIN_VOLTAGE			3000	// Minimum battery voltage for non-parasitic power in mv

#define NEW_GAUGE (PPP_V2 == BatteryType)

// EEPROM data buffer
#define MAX_EEPROM_SIZE  16400 // 16K VT EEPROM
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
extern uint8_t VTeeprom_Valid;  // VT EEPROM battery data valid flag

extern uint8_t Verbose;  // Use verbose output flag


