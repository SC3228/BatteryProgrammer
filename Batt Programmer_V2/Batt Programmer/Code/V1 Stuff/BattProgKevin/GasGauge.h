// Gas gauge stuff

extern uint16_t DesignCap;  // Gas gauge design capacity value
extern uint16_t CurTemp;  // Current battery temp
extern uint32_t dwSecSinceFirstUse;  // Time since first use
extern uint8_t GasGaugeOK;  // Data was OK Flag

uint8_t GG_ReadReg(void);
uint8_t GG_GetStuff(GIFTED_BATT_DATA_t *BattData);
uint8_t GG_EnterHibernate(void);
uint8_t GG_SetFullSleep(uint8_t ask=1);
uint8_t GG_Reset(uint8_t ask=1);
uint8_t GG_Seal(void);
uint8_t GG_GetManfBlocks(uint8_t *buffer);
uint8_t GG_GetDataFlash(uint8_t *buffer);
uint8_t GG_ReadSubClass(uint8_t SubClass, uint8_t Size, uint8_t *buffer);
void DumpGGConfig(void);
void SaveGGConfig(void);

#pragma pack(1);

// Config safety struct
typedef struct tagCFG_SAFETY
{
	int16_t	OT_Chg;
	uint8_t	OT_ChgTime;
	int16_t	OT_ChgRecovery;
	int16_t	OT_Dsg;
	uint8_t	OT_DsgTime;
	int16_t	OT_DsgRecovery;
} CFG_SAFETY_t;

// Config charge inhibit struct
typedef struct tagCFG_CHG_INHIBIT
{
	int16_t	TempLow;
	int16_t	TempHi;
	int16_t	TempHys;
} CFG_CHG_INHIBIT_t;

// Config charge struct
typedef struct tagCFG_CHARGE
{
	int16_t	ChargingVoltage;
	int16_t	DeltaTemp;
	int16_t	SuspendLowTemp;
	int16_t	SuspendHighTemp;
} CFG_CHARGE_t;

// Calibration data struct
typedef struct tagCAL_DATA
{
	uint32_t CC_Gain;
	uint32_t CC_Delta;
	int16_t	CC_Offset;
	int8_t BoardOffset;
	int8_t IntTempOffset;
	int8_t ExtTempOffset;
	int8_t PackV_Offset;
} CAL_DATA_t;

// Overall data flash struct
typedef struct tagDATA_FLASH
{
	CFG_SAFETY_t CfgSafety;
	CFG_CHG_INHIBIT_t CfgChgInhibit;
	CFG_CHARGE_t CfgCharge;
	CAL_DATA_t CalData;
} DATA_FLASH_t;

#pragma pack();
