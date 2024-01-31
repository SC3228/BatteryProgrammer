// Battery data table include file

// NOTE: Keep the below in sync with the defs in Batts.cpp!!!
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
//#define FROZONE_EP_BATT		12	// Frozone EEPROM
#define METEOR_BATT			12	// VT battery
#define COMET_BATT			13	// VT battery with EEPROM
#define PPP_V2				14	// New gas gauge battery

// Which key to use defines
#define	NO_KEYS			0
#define SUNRISE_KEYS	1
#define IRONMAN_KEYS	2
#define FREEPORT_KEYS	3
#define FALCON_KEYS		4

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
	uint16_t Asize;				// Size in bytes of the auth chip data
	uint8_t Pullup_volt;		// Pullup voltage
	uint8_t Pullup_resistor;	// Pullup resistor values
	uint8_t	ActiveClk;			// Use active clock drive flag
	uint8_t Keys;				// Which encryption keys to use (if any)
	const char *Name;			// Descriptive name
	int MinVoltage;				// Minimum battery voltage to access EEPROM in mv
	uint8_t HasTherm;			// Battery has a thermistor flag
} BattData;
#pragma pack()

#define MAX_BATTERY_TYPES 15  // Number of supported battery types

extern const BattData Batts[MAX_BATTERY_TYPES];

