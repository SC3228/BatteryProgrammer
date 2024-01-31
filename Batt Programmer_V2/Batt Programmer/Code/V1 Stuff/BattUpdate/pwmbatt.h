// Virtual Power Micro Battery stuff Include File

#ifndef PWRM_BATT_H_
#define PWRM_BATT_H_

// Battery chip defs
#define MPA2EEPROM	0xA0	// MPA2 EEPROM address
#define MPA2TEMP	0x90	// MPA2 Temp chip address
#define BM_ADD		0x42	// Battery micro address
#define MPA3EEPROM	0xA4	// MPA3 EEPROM address
#define MPA3TEMP	0x94	// MPA3 Temp chip address
#define MPA3GG		0xAA	// MPA3 gas gauge address
#define TPB_TEMP	0x92	// Third party battery temp chip address
#define AUTH_CHIP	0xD0	// Auth chip address

// Other battery defines
#define MPA3_BATT_PN_LEN 22
#define MPA2_BATT_PN_LEN 22
#define MIN_BATT_VOLTS 2900   // Minimum battery voltage in mv to be able to talk to battery

// Gas Gauge commands/registers (MPA3 Batteries)
#define CNTL_CMD		0x00
#define AR_CMD			0x02	// Command to read/write the at rate current
#define FLAGS_CMD		0x0A	// Command to read the current gas gauge flags
#define SOC_CMD			0x2A	// Command to read the state of charge %
#define SOH_CMD			0x28	// Command to read the state of health %
#define LCCA_CMD		0x34	// Command to read the charge current accumulator
#define CHGCC_CMD		0x36	// Command to read the charge cycle count
#define DCAP_CMD		0x72	// Design capacity registers
#define MFGID_CMD		0x6a	// Manuf ID
#define DFCLS			0x3e	// DataFlashClass()
#define DFBLK_CMD		0x3f	// DataFlashBlock()
#define A_DF_CMD		0x40	// Authenticate()/BlockData()
#define DFDCKS			0x60	// BlockDataCheckSum()
#define DFDCNTL			0x61	// BlockDataControl()

// New gas gauge commands/registers
#define DCAP_CMD_NEW	0x3c	// Design capacity registers, new gas gauge
#define ALT_MANF_ACC	0x3e	// Alt manufaturer access, new gas gauge

// Control() sub commands (MPA3 Batteries)
#define GG_DEVICE_TYPE			0x0001	// Get device type
#define GG_FW_VERSION			0x0002	// Get firmware version
#define GG_HW_VERSION			0x0003	// Get hardware version
#define GG_ENABLE_RMFCC			0x000c	// Enables automatic RM update based on FCC
#define GG_DISABLE_IBAW			0x0010	// Disables access to manf block A
#define GG_SET_FULLSLEEP		0x0012	// Forces CONTROL_STATUS[FULLSLEEP] to 1
#define GG_SET_HIBERNATE		0x0013	// Forces CONTROL_STATUS[HIBERNATE] to 1
#define GG_CLEAR_HIBERNATE		0x0014	// Forces CONTROL_STATUS[HIBERNATE] to 0
#define GG_READ_QMAX_DAY		0x0017	// Get QMAX_DAY value
#define GG_READ_CONTROL_STATUS	0x0000	// Read CONTROL_STATUS flags
#define GG_SEAL_CHIP			0x0020	// Puts the gas gauge into sealed mode
#define GG_RESET_CHIP			0x0041	// Reset the gas gauge

// Data flash sub class values
#define SC_SAFETY			2	// Safety sub class
#define SC_CHRG_INHIB_CFG	32	// Charge Inhibit Cfg sub class
#define SC_CHARGE			34	// Charge sub class
#define SC_CHRG_TERM		36	// Charge Termination sub class
#define SC_CFG_DATA			48	// Configuration data sub class
#define SC_DISCHARGE		49	// Discharge sub class
#define SC_MANF_INFO		58	// Manufacturer Info sub class
#define SC_INTEG_DATA		57	// Integrity Data sub class
#define SC_REGISTERS		64	// Registers sub class
#define SC_POWER			68	// Power sub class
#define SC_IT_CFG			80	// IT Cfg sub class
#define SC_CURR_THRES		81	// Current Thresholds sub class
#define SC_STATE			82	// State sub class
#define SC_OCV_TABLE		83	// OCVa Table sub class
#define SC_PACK_RA0			88	// Pack Ra0 sub class
#define SC_PACK_RA0X		89	// Pack Ra0x sub class
#define SC_CAL_DATA			104	// Calibration data sub class
#define SC_CURRENT			107	// Current sub class

// Unseal keys
#define UNSEAL_KEY1	0xe1b2
#define UNSEAL_KEY2	0x3e84

// Battery gas gauge defs (MPA3 Batteries)
#define MPA3_GG_DEVICE_TYPE	0x0541	// Supported device type
#define MPA3_GG_FW_VERSION	0x0100	// Supported firmware version

// Gas gauge status register bit defines
#define STATUS_LOW_HIBERNATE_BIT	0x40	// Hibernate bit in lower status byte
#define STATUS_LOW_FULLSLEEP_BIT	0x20	// Fullsleep bit in lower status byte
#define STATUS_HIGH_SS_BIT			0x20	// Sealed bit

// Flag register defs (MPA3 Batteries)
#define FLAG_FC		0x0200	// Full-charged condition reached. True when set.
#define OCV_TAKEN	0x0040	// Open-Circuit-Voltage taken. True when set.
#define OCV_PRED	0x0020	// Open-Circuit-Voltage predicted. True when set.

// Block select defs
#define AUTH_BLK	0	// Auth block
#define MAN_BLK_A	1	// Manuf block A
#define MAN_BLK_B	2	// Manuf block B
#define MAN_BLK_C	3	// Manuf block C

// New gauge block read defines
#define BLK_READ_SIZE	36	// Number of bytes in a block read
#define BLK_READ_CSUM	34	// Offset to block checksum
#define BLK_READ_LEN	35	// Offset to block length

// Smart battery data
#define	RTs_SIZE	11	// Size of RTs data in RT_DATA_ts
#define	VOCs_SIZE	11	// Size of Vocs data in VOC_DATA_ts

#define BATTERY_CHEMISTRY_LION  0x04 // Battery type define

#define MIN_SLOW_FAST_CURRENT	350	// Minimum setting for slow charge over current level

/* MPA2 Battery data defines */
#define	SB_DATA_ADD 0			// Address of battery data area in EEPROM
#define	SB_DATA_SIZE 156		// Size of battery data area in EEPROM

#define	SB_LIFE_ADD1	160		// Address of battery life data area 1
#define	SB_LIFE_ADD2	168		// Address of battery life data area 2
#define	SB_LIFE_SIZE	 5		// Size of battery life data

#define	SB_SUPPORTED_TYPE 4		// Currently supported battery data type

#define	SB_1725_DATA_ADD 208	// Address of 1725 data record in battery
#define	SB_1725_DATA_SIZE 8		// Size of 1725 data record
#define	SUPPORTED_1725_TYPE 1		// Currently supported 1725 data record type

#define	SB_HEALTH_DATA_ADD 216	// Address of health data record in battery
#define	SB_HEALTH_DATA_SIZE 6	// Size of health data record
#define	SUPPORTED_HEALTH_TYPE 1	// Currently supported health data record type
#define BAD_LIFE_DATA 0xffffffff// Bad TC data value

/* MPA3 Battery defines */
#define GB_DYN_BLK_SIZE 16			// Size of MPA3 dynamic blocks
#define GB_LOCK_BLK_ADD 0x30		// Address of lock block
#define GB_DYN_BLK1_ADD 0x40		// Address of dynamic block 1
#define GB_DYN_BLK2_ADD 0x50		// Address of dynamic block 2
#define GB_DYN_BLK3_ADD 0x60		// Address of dynamic block 3
#define GB_DYN_BLK4_ADD 0x70		// Address of dynamic block 4
#define GB_STATIC_DATA_SIZE 0x2f	// Size of static area in MPA3 EEPROM
#define	GB_SupportedType 1			// Currently supported battery data type
#define GB_MANF_BLK_SIZE 32			// Size of Manf blocks
#define GB_MANF_SupportedType 1		// Currently supported manf block data type
#define GB_BQ_ALLOC 0xcf			// Pattern for currently allocated blocks in the GG flash. Unused blocks are '1'
#define GB_CMD_PARTIAL_READ_SIZE 20	// Number of bytes to read at one time to fill
									// the GG registers in the MPA3 battery structure
#define GB_CMD_FULL_READ_SIZE (uint8_t)62	// Number of bytes to read all the gas gauge registers

/* Value tier battery defines */
#define VT_ASD1_SIZE 226	// Size of first architecture specific data
#define VT_ASD2_SIZE 184	// Size of second architecture specific data

#pragma pack(1)  // Byte align structure to match EEPROMM data

// MPA2 battery data
/*---------------------------------------------------------------------
 * 
 *  RT Table structure
 * 
 *--------------------------------------------------------------------*/
typedef struct tagRT_DATA
{
	u8	Temp;		// Battery temp in deg C
	u16	Slope;		// Slope of this segment
	u16	Resistance;	// Battery resisteance (in ohms * 1.024) at start of region
} RT_DATA_t;
/*---------------------------------------------------------------------
 * 
 *  VOC Table structure
 * 
 *--------------------------------------------------------------------*/
typedef struct tagVOC_DATA
{
	u16	Voltage;	// Battery V open circuit in mv
	u16	Slope;		// Slope of this segment
	u8	Capacity;	// Battery capacity at start of region
} VOC_DATA_t;

/*---------------------------------------------------------------------
 * 
 *  MPA2 Battery data structure
 * 
 *--------------------------------------------------------------------*/
typedef struct tagSMART_BATT_DATA
{
	// Battery data
	char	BattDataCsum;		// Checksum of battery data
	char	BattDataType;		// Smart battery data type
	u32	BattPartNum;		// Smart battery part number
	u32	BattID;				// Smart battery ID number
	char	BattDate[3];		// Smart battery date made
	u16	BattRated;			// Rated battery capacity in maHr's
	u8	BattFaireLevel;		// Battery capacity levels
	u8	BattUnfaireLevel;
	u8	BattLowLevel;
	u16	VocFudge;			// Slope of Voc below 0 fudge
	char	VocFudgeTempStart;	// Temperature below which Voc correction is made
	char	VocFudgeTempStop;	// Temperature below which no additional Voc correction is made

	// Gas Gauge Data
	RT_DATA_t	RTs[RTs_SIZE];		// Battery resistance by temp table
	VOC_DATA_t	VOCs[VOCs_SIZE];	// Battery Voc table

	// Charge control data
	u8	SlowChrgTO;			// Slow charge timeout in 10 min intervals
	u8	FastChrgTO;			// Fast charge timeout in 10 min intervals
	u16	SlowFastThreshold;	// Slow to fast charge voltage threshold in mv
	u16	RechargeVoltage;	// Recharge a charged battery voltage in mv
	u16	AbsentBattVoltage;	// Absent battery voltage in mv
	u16	AbnormalCurrent;	// Abnormal battery current in ma
	u16	SlowChrgHighFC;		// Slow charge high fault current in ma
	u16	SlowChrgLowFC;		// Slow charge low fault current in ma
	u16	NearlyDoneCurrent;	// Nearly done current in ma
	u16	DoneCurrent;		// Done current in ma
	int8_t	MinChrgTemp;		// Minimum charge temperature in deg C
	int8_t	ColdOnTemp;			// Turn on charger temp for cold battery
	int8_t	HotOnTemp;			// Turn on charger temp for hot battery
	int8_t	MaxChrgTemp;		// Maximum charge temperature in deg C
	int8_t	NoBattTemp;			// No battery temperature
	char	ChangeWaitCnt;		// The temperature delta above/below from 'too low' or 'too high' to recover
	char	Unused1[4];
	
	// Battery life data1
	u32	BattLife1;			// Battery life time total charge
	char	BattLifeCsum1;		// Checksum of battery life data
	char	Unused2[3];

	// Battery life data2
	u32	BattLife2;			// Battery life time total charge
	char	BattLifeCsum2;		// Checksum of battery life data
	char	Unused3[3];

	// Aging data (Not used)
	char	AgingData[28];
	char	Unused4[4];

	// 1725 data
	char	_1725_CSum;			// Check sum for the 1725 data record
	char	_1725_DataType;		// Data type for the 1725 data record
	char	BattMinDTemp;		// Battery min discharge temp
	char	BattMaxDTemp;		// Battery max discharge temp
	u16	BattMaxCVolt;		// Battery max charging voltage
	u16	BattFastCurr;		// Battery fast charge current

	// Health data
	char	Health_CSum;		// Check sum for the 1725 data record
	char	Health_DataType;	// Data type for the 1725 data record
	char	DateFirstUse[3];	// Smart battery date made
	u8	BattHealth;			// Battery health indicator
	char 	pad[34];		// Pad to get to 256 bytes
} SMART_BATT_DATA_t;

/*---------------------------------------------------------------------
 * 
 *  MPA3 Gifted Battery data structures
 * 
 *--------------------------------------------------------------------*/

/*---------------------------------------------------------------------
 * 
 *  Dynamic block structure 1
 * 
 *--------------------------------------------------------------------*/
typedef struct tagDYNAMIC_BLK1_DATA
{
	u8	MaxRecalPd_Days;		// Maximum number of days after the last recalibration that a
									// new recalibration should occur.  A value of 0x00 inhibits
									// duration-demanded recalibration.
	u8	HealthPct;				// StateOfHealth( ) value below which the battery is declared unhealthy.
	u32	BatteryLifeData;		// Total maHr put into the battery
	u8	CopiedManufactureID[8];	// This value shall be set initially to all ones. Upon seeing a value of
									// all ones, terminal and charger code shall replace this value with that
									// stored in the block located at hex address 0x30. Thereafter, code
									// shall check upon insertion of the battery pack into a unit that this
									// ID matches that stored in the block at 0x30.
	u8	Checksum_x;				// Checksum of this block (not including random pad)
	char	PadByte;				// Random Pad
} DYNAMIC_BLK1_DATA_t;


/*---------------------------------------------------------------------
 * 
 *  Dynamic block structure 2
 * 
 *--------------------------------------------------------------------*/
typedef struct tagDYNAMIC_BLK2_DATA 
{
	u16	EEFixedPattRewriteCtr;	// Counter which keeps track of the number of times that the Reserved
									// EEPROM blocks have been rewritten.
	u16	BQFixedPattRewriteCtrB;	// Counter which keeps track of the number of times that the BQ27541
									// Manufacturing Block B has been rewritten.
	u16	BQFixedPattRewriteCtrA;	// Counter which keeps track of the number of times that the BQ27541
									// Manufacturing Block A has been rewritten.

	u8	CopiedManufactureID[8];	// This value shall be set initially to all ones. Upon seeing a value of
									// all ones, terminal and charger code shall replace this value with that
									// stored in the block located at hex address 0x30. Thereafter, code
									// shall check upon insertion of the battery pack into a unit that this
									// ID matches that stored in the block at 0x30.
	u8	Checksum_x;				// Checksum of this block (not including random pad)
	char	PadByte;				// Random Pad
} DYNAMIC_BLK2_DATA_t;


/*---------------------------------------------------------------------
 * 
 *  Fixed block structure
 * 
 *--------------------------------------------------------------------*/
typedef struct tagFIXED_BLK_DATA
{
	u8	BlockNum[15];			// Block number of this block
	char	PadByte;
} FIXED_BLK_DATA_t;

/*---------------------------------------------------------------------
 * 
 *  Extended temp range structure
 * 
 *--------------------------------------------------------------------*/
typedef struct tagEX_TEMP_RANGE_EP
{
	int8_t StartTemp;		// Lower end of this temp range
	uint16_t Voltage[3];	// Charge to voltages for this temp range
	uint16_t Current[3];	// Max charge currents for this temp range
	uint8_t pad[3];
} EX_TEMP_RANGE_EP_t;

/*---------------------------------------------------------------------
 * 
 *  Extended temp charging structure
 * 
 *--------------------------------------------------------------------*/
typedef struct tagEX_TEMP_CHARGING_EP
{
	uint8_t DataType;	// Structure version byte.
	uint8_t csum;  // Checksum, bytewise total of struct (including csum) should be zero
	int8_t StopTemp;  // High end of last range, if needed.
	int8_t RecMinTemp, RecMaxTemp;  // JEITA recommended temperature range
	uint8_t pad[11];
	EX_TEMP_RANGE_EP_t TempRange[5]; // Charging temp ranges
} EX_TEMP_CHARGING_EP_t;

/*---------------------------------------------------------------------
 * 
 *  Ship mode structure
 * 
 *--------------------------------------------------------------------*/
typedef struct tagSHIP_MODE_DATA
{
	uint16_t LowVoltage;	// Low end of shipmode voltage range
	uint16_t HighVoltage;	// High end of shipmode voltage range
	uint8_t LowCapacity;	// Low end of shipmode capacity range
	uint8_t HighCapacity;	// High end of shipmode capacity range
	uint8_t CheckSum;		// Checksum of above bytes
	uint8_t pad[9];
} SHIP_MODE_DATA_t;

// Bit mask for blocks used by the EX_TEMP_CHARGING_EP_t struct
#define EX_CHARGE_TEMP_BLOCKS 0x00003F00

// Non encrypted block defs
#define SD660_BLOCK (uint32_t)0x00004000  // Bit mask for SD660 block used
#define SD660blknum 0x0E  // SD660 data block number
#define SD660blkaddr 0xE0  // SD660 data block address
#define CELL_IDENT_BLOCKS (uint32_t)0x00018000  // Bit mask for cell identifying blocks used
#define CELL_IDENT1blknum 0x0F  // Cell identifying block numbers
#define CELL_IDENT2blknum 0x10
#define CELL_IDENT1blkaddr 0xF0  // Cell identifying block address
#define SHIP_MODE_BLOCK (uint32_t)0x00020000  // Bit mask for ship mode block used
#define SHIP_MODEblknum 0x11  // Ship mode block number
#define SHIP_MODEblkaddr 0x110  // Ship mode block address

/*---------------------------------------------------------------------
 * 
 * Gifted battery EEPROM structure
 * 
 *--------------------------------------------------------------------*/
typedef struct tagGIFTED_BATT_DATA 
{
	// Battery data
	char	Checksum_z;				// Checksum of static battery data
	char	DataType;				// Battery data type
	u8	Faire;					// Battery capacity levels
	u8	Unfaire;
	u8	Low;

	// Charge control data
	u8	ChargeSlowMins;			// Slow charging time out, minutes.
	u16	ChargeFastMins;			// Fast charging time out, minutes. 
	u16	ChargeUp_mV;			// Charge-up voltage, mV.
	u16	SlowFastCharge_mV;		// Slow/fast charge voltage threshold, mV.
	u16	CC_Threshold;			// Capacity that must charge the battery before the BQ27541 advances ChgCycleCount().
									// This parameter is identical in name and value to the one stored in the BQ27541 data flash.
									// The BQ parameter cannot be read, however, hence this copy. (mAhrs)
	char	FillerByte1;			// Should be 0xff
	char	PadByte1;				// Random Pad

	u16	AbnormalCharge_mA;		// Abnormal battery charge current, mA.
	u16	SlowCharge_mA;			// Slow charge current, mA.
	u16	NearlyCharged_mA;		// Current at which charge completion is indicated, mA.
	u16	FastCharge_mA;			// Fast Charge Current, mA.
	u16	ChargeTerm_mA;			// Minimum Charge Termination Current, mA (below BQ27541 value.) 
	u8	Block_Allocation_27541;	// Bits refer to a block of 16 contiguous bytes in the 27541, for a total of 6.
									// The LSb refers to the first block of Manu Block A, then the second of A,
									// then the first of B, etc.  If set to 0, the bit indicates that the block
									// contains useful data.  If set to 1, the block is reserved, is expected to
									// contain the pattern shown in the BQ27541 map in the document PackMakerMemSpec.
	u32	Block_Allocation_EEPROM;// Each bit refers to a block of 16 contiguous bytes in this EEPROM, a total of 32.
									// The LSb of the LSB refers to block 0, etc.  If set to 0, the bit indicates that the
									// block contains useful data.  If set to 1, the block is reserved and is treated as described in ????????
	char	PadByte2;				// Random Pad

	int8_t	ChargeColdOff_C;		// Highest cold temp at which battery is never charged, signed, °C.
	int8_t	ChargeColdOn_C;			// Hysteretic cold charging turn on temp, signed, °C.
	int8_t	ChargeHotOn_C;			// Hysteretic hot charging turn on temp, signed, °C.
	int8_t	ChargeHotOff_C;			// Lowest hot temp at which battery is never charged, signed, °C.
	int8_t	DischargeMin_C;			// Discharge Temperature, minimum allowed, signed, °C.
	int8_t	DischargeMax_C;			// Discharge Temperature, maximum allowed, signed, °C.
	u16	MaxOCV_PREDmins;		// A value greater than the maximum period in minutes starting
									// from entering BQ27541 Relaxation to seeing OCV_PRED set.
	u16	OCV_PRED_Prep_Curr_mA;	// The minimum charge or discharge current that must flow before
									// the Relaxation mode can be entered if OCV_PRED is to be set.
	u16	OCV_PRED_Prep_Time_Sec;	// The minimum time that OCV_PRED_Prep_Curr_mA must flow before
									// the Relaxation mode can be entered if OCV_PRED is to be set.
	u8	SOC_DELTA;				// The minimum amount in percent that the BQ27541 SoC must change
									// between OCV readings before OCV_PRED can be set. Typically 40%. 
	u8	SOC_CAL_MAX;			// The maximum SOC percentage allowed before the calibration is
									// initiated.  Typically, SOC_CAL_MAX <= 100% - SOC_DELTA ? 
									// ( OCV_PRED_Prep_Curr_mA * OCV_PRED_Prep_Time_Sec )  / 
									// ( 3600 * CC_Threshold )
	u8	MaxRecal_Cycle;			// The maximum number of cycles between recalibrations.  A value
									// of 0x00 inhibits cycle number-demanded recalibration.
	char	PadByte3;				// Random Pad

	// Lock Block
	u8	Manufacture_ID[8];	// Copy of manufacture ID of the battery's SN27541-M200
	u8	DummyData[6];		// Set to 0x00
	u8	Checksum_y;			// Checksum of this block of data (random pad set to 0)
	char	PadByte4;			// Random Pad

	// Dynamic blocks 1-2
	DYNAMIC_BLK1_DATA_t	Dyn1Block[2];

	// Dynamic blocks 3-4
	DYNAMIC_BLK2_DATA_t	Dyn2Block[2];

	// Extended temp charging
	EX_TEMP_CHARGING_EP_t ExCharge;

	// Fixed blocks
	FIXED_BLK_DATA_t FixBlockA[3];

	// Ship mode data
	SHIP_MODE_DATA_t ShipMode;

	// Fixed blocks
	FIXED_BLK_DATA_t FixBlockB[16];

	// Manf data block A
	u8 BlockA[32];

	// Manf data block B
	u8 BlockB[32];

	// Manf data block C
	char	Checksum_c;				// Checksum of manf block C data
	char	DataFormat541;			// Battery data type (should be 1)
	char	BatteryPartNumber[21];	// Battery part number in ASCII string
	u8		Revision;				// Battery pack revision. 0x00 or 0xff are unassigned
	char	SerialNumber[5];		// Battery pack serial number in ASCII string
	uint8_t	DateMade[3];			// Battery date made

	// Other data
	u16	GG_DeviceType;		// Device type of GG
	u16	GG_FirmwareVer;		// Firmware version of GG code
	u16 GG_HardwareVer;		// Hardware version of GG
	u8	M200_Manf_ID[8];		// Manufacture ID read from the battery?s SN27541-M200
	u16	QMAX_DAY;				// Days of last calibration
	u16	Last_CHGCC;				// Last read values of cycle count and charge accumulator
	u16	Last_LCCA;
	u16 CONTROL_STATUS;			// CONTROL_STATUS flags

	// The following are read periodically from the gas gauge registers
	u16	AR;						// AtRate( )
	u16	ARTTE;					// AtRateTimeToEmpty( )
	u16	TEMP;					// Temperature( )
	u16	VOLT;					// Voltage( )
	u16	FLAGS;					// Flags( )
	u16	NAC;					// NominalAvailableCapacity( )
	u16	FAC;					// FullAvailableCapacity( )
	u16	RM;						// RemainingCapacity( )
	u16	FCC;					// FullChargeCapacity( )
	int16_t	AI;					// AverageCurrent( )
	u16	TTE;					// TimeToEmpty( )
	u16	TTF;					// TimeToFull( )
	u16	SI;						// StandbyCurrent( )
	u16	STTE;					// StandbyTimeToEmpty( )
	u16	MLI;					// MaxLoadCurrent( )
	u16	MLTTE;					// MaxLoadTimeToEmpty( )
	u16	AE;						// AvailableEnergy( )
	u16	AP;						// AveragePower( )
	u16	TTECP;					// TTEatConstantPower( )
	u16	SOH;					// StateOfHealth( )
	u16	SOC;					// StateOfCharge( )
	u16	QVC;					// ValidQmaxCycles()
	u16	ETU;					// ElapsedTimeUpper()
	u16	ETL;					// ElapsedTimeLower()
	u16	TV;						// TerminateVoltage()
	u16	LCCA;					// ChargeCurrentAccumulator()
	u16	CHGCC;					// ChgCycleCount( )
	u16	LDCA;					// DischargeCurrentAccumulatot()
	u16	DSGCC;					// DsgCycleCount( )
	u16	SOCZ;					// StateOfChargeZone( )
} GIFTED_BATT_DATA_t;

/*---------------------------------------------------------------------
 * 
 *  Cell identifing data structure
 * 
 *--------------------------------------------------------------------*/
typedef struct tagCELL_IDENT
{
	int8_t Checksum;// Checksum of ident data strct
	uint8_t Vendor;	// Cell vendor code
	uint8_t IdentData[30];	// Cell identifing data
} CELL_IDENT_t;

/*---------------------------------------------------------------------
 * 
 *  Charger control structure for value tier
 * 
 *--------------------------------------------------------------------*/
typedef struct tagCHARGER_CTRL_VT
{
	uint8_t Checksum;
	uint8_t SlowChrgTimeout;
	uint8_t FastChrgTimeout;
	uint16_t SlowFastVolt;
	uint16_t RechargeVolt;
	uint16_t AbnormalCurr;
	uint16_t SlowChrgHighFaultCurr;
	uint16_t SlowChrgLowFaultCurr;
	uint16_t NearlyDone;
	uint16_t Done;
	uint8_t Hysteresis;
} CHARGER_CTRL_VT_t;

/*---------------------------------------------------------------------
 * 
 *  Aggregate charge structure for value tier
 * 
 *--------------------------------------------------------------------*/
typedef struct tagAGG_CHARGE_VT
{
	uint8_t Checksum;  // Structure checksum
	uint8_t Partial;  // Partial charge cycle percentage
	uint16_t Cycles;  // Full charge cycle count
} AGG_CHARGE_VT_t;

/*---------------------------------------------------------------------
 * 
 *  Ship mode structure for value tier
 * 
 *--------------------------------------------------------------------*/
typedef struct tagSHIP_MODE_DATA_VT
{
	uint16_t LowVoltage;  // Low end of shipmode voltage range
	uint16_t HighVoltage; // High end of shipmode voltage range
	uint8_t LowCapacity;  // Low end of shipmode capacity range
	uint8_t HighCapacity;	  // High end of shipmode capacity range
} SHIP_MODE_DATA_VT_t;

/*---------------------------------------------------------------------
 * 
 *  Battery health data structure for value tier
 * 
 *--------------------------------------------------------------------*/
typedef struct tagBATT_HEALTH_VT
{
	uint8_t Checksum;  // Structure checksum
	uint8_t RecordType;	// Health record type, currently 0
	uint8_t Health;	// Battery health byte
	uint8_t Pad;
} BATT_HEALTH_VT_t;

/*---------------------------------------------------------------------
 * 
 *  Extended temp range structure for value tier
 * 
 *--------------------------------------------------------------------*/
typedef struct tagEX_TEMP_RANGE_EP_VT
{
	int8_t StartTemp;		// Lower end of this temp range
	uint16_t Voltage[3];	// Charge to voltages for this temp range
	uint16_t Current[3];	// Max charge currents for this temp range
} EX_TEMP_RANGE_EP_VT_t;

/*---------------------------------------------------------------------
 * 
 *  Extended temp charging structure for value tier
 * 
 *--------------------------------------------------------------------*/
typedef struct tagEX_TEMP_CHARGING_EP_VT
{
	uint8_t DataType;	// Structure version byte.
	uint8_t csum;  // Checksum, bytewise total of struct (including csum) should be zero
	int8_t StopTemp;  // High end of last range, if needed.
	int8_t RecMinTemp, RecMaxTemp;  // JEITA recommended temperature range
	EX_TEMP_RANGE_EP_VT_t TempRange[5]; // Charging temp ranges
} EX_TEMP_CHARGING_EP_VT_t;

/*---------------------------------------------------------------------
 * 
 *  QC660 data structure for value tier
 * 
 *--------------------------------------------------------------------*/
typedef struct tagQC660_DATA_VT
{
	uint8_t ElimFlag;	// Battery eliminator flag
	uint16_t MinStartupVolt;  // Minimum startup voltage in mv
	uint8_t ThermCoff[3];	// Thermistor coefficients
	uint16_t UVLO;  // Under voltage lockout voltage in mv
	uint16_t OVLO;  // Over voltage lockout voltage in mv
	uint16_t CutoffVolt;  // Cutoff voltage in mv
} tagQC660_DATA_VT_t;

/*---------------------------------------------------------------------
 * 
 *  Thermistor data structure for value tier
 * 
 *--------------------------------------------------------------------*/
typedef struct THERM_DATA_VT
{
	int8_t Temps[9];
	uint16_t Res[9];
	uint8_t addend;
} THERM_DATA_VT_t;

/*---------------------------------------------------------------------
 * 
 *  VT (Value tier) batttery data format
 * 
 *--------------------------------------------------------------------*/
typedef struct tagVALUE_TIER_BATT_DATA 
{
	// Battery data
	uint8_t Checksum;
	uint8_t FormatRev;
	uint8_t PartNumber[20];
	uint8_t SerialNumber[6];
	uint8_t Date[3];
	uint16_t ManfCapacity;  // Must be byte swapped before use
	// Battery capacity limits
	uint8_t BatteryLow;
	uint8_t BatteryVeryLow;
	uint8_t BatteryCritical;
	// 660 DTSI data
	uint8_t SDM660DTSI[VT_ASD1_SIZE];
	// 1725 Stuff
	uint8_t _1725RecordType;
	int8_t _1725DiscLowLimit;
	int8_t _1725DiscHighLimit;
	// Charger control
	CHARGER_CTRL_VT_t ChargerCtrl;
	// Thermistor data
	THERM_DATA_VT_t Therm;
	// JEITA data
	EX_TEMP_CHARGING_EP_VT_t JEITA;
	tagQC660_DATA_VT_t _660Data;
	// Cell ident stuff
	CELL_IDENT_t Ident;
	// Ship mode data
	SHIP_MODE_DATA_VT_t ShipMode;
	uint8_t ASD2_Checksum;
	// Battery health
	BATT_HEALTH_VT_t Health1;
	BATT_HEALTH_VT_t Health2;
	// Agregate charge
	AGG_CHARGE_VT_t AggCharge1;
	AGG_CHARGE_VT_t AggCharge2;
	// Currently free space
	uint8_t ASD2[VT_ASD2_SIZE];
} VALUE_TIER_BATT_DATA_t;

// Common struct to save RAM
union BatteryData
{
	SMART_BATT_DATA_t	Smart;
	GIFTED_BATT_DATA_t	Gifted;
	VALUE_TIER_BATT_DATA_t	VT;
};

#pragma pack()

#endif	// #ifndef PWRMBATT_H_
