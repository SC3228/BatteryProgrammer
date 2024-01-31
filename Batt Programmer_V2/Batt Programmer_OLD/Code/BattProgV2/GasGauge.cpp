// Zebra battery programmer gas gauge stuff

#include <arduino.h>
#include "Adafruit_SPIFlash.h"
#include <SoftWire.h>
extern SoftWire Wire;

//#include <stdint.h>

#include "pwmbatt.h"
#include "Batts.h"
#include "BattProg.h"
#include "GasGauge.h"
#include "Auth.h"

#define SwapBytes(A) A = (int16_t)((uint16_t)A<<8 | (uint16_t)A>>8);

#define MAX_CHUNKS 50	// Max number of chunks of stuff to save doing GG update
#define GG_SAVE_SIZE 2000  // Max bytes to save during update

uint16_t DesignCap = 1000;  // Default gas gauge design capacity value
int16_t CurTemp = -70;  // Current battery temp
int16_t CurIntTemp = -70;  // Current internal gauge temp
uint32_t dwSecSinceFirstUse;  // Time since first use
uint32_t NGGsecSinceMade;  // Time since manufacture (New gauge)

uint8_t GasGaugeOK = 0;  // Data was ok Flag

// Functon to toss the colon following thw command
// return false on error, true if OK
bool SkipColon(File *file, int line)
{
	uint8_t inch[2];

	// Get colon character
	if (2 != file->read(&inch,2))  // Get next byte from file and check for read error
	{
		Serial.printf(OnLine,line);
		Serial.println(ReadError);
		return (false);
	}

	// Check for colon
	if (':' != inch[0])
	{
		Serial.printf("Bad format on line# %d, no ':' after command\r\n",line);
		return (false);
	}

	return (true);
}

// Function to read a hex byte from a .fs file
// Returns byte if OK, -1 on EOL, -2 on error
int GetHexByte(File *file, int line)
{
	char inch[3] = {0};
	int val;

	// Toss white space
	do
	{
		if (1 != file->read(inch,1))  // Get next byte from file and check for read error
		{
			Serial.printf(OnLine,line);
			Serial.println(HexReadErr);
			return (-2);
		}
	} while (' ' == inch[0]);

	if (LF == inch[0]) // Check for EOL
		return (-1);

	if (1 != file->read(inch+1,1))  // Get next byte from file and check for read error
	{
		Serial.printf(OnLine,line);
		Serial.println(HexReadErr);
		return (-2);
	}

	// Check for invalid hex digits
	if (((inch[0] < '0' || inch[0] > '9')
		     && (inch[0] < 'A' || inch[0] > 'F')
		     && (inch[0] < 'a' || inch[0] > 'f'))
		 || (('0' < inch[1] && inch[1] > '9')
		     && (inch[1] < 'A' || inch[1] > 'F')
		     && (inch[1] < 'a' || inch[1] > 'f')))
	{
		Serial.printf("Bad hex number on line# %d\r\n",line);
		return (-2);
	}

	// Get value
	val = -2;
	sscanf(inch,"%x",&val);

	return (val);
}

// Function to read an integer from end of line in a .fs file
// Returns int if OK, -2 on error
int GetInteger(File *file, int line)
{
	char inch;
	int val, cnt;

	// Toss white space
	do
	{
		if (1 != file->read(&inch,1))  // Get next byte from file and check for read error
		{
			Serial.printf(OnLine,line);
			Serial.println(HexReadErr);
			return (-2);
		}
	} while (' ' == inch);

	if (LF == inch) // Check for EOL
	{
		Serial.printf(OnLine,line);
		Serial.println("Missing value");
		return (-2);
	}

	cnt = 0;
	val = 0;
	do
	{
		// Check for an invalid digit
		if (inch < '0' || inch > '9')
		{
			Serial.printf("Bad number on line# %d\r\n",line);
			return (-2);
		}

		// Check for room
		if (cnt > 5)
		{
			Serial.printf("Number to long on line# %d\r\n",line);
			return (-2);
		}

		// Shift Val one place and add in the new digit
		val = (val * 10) + inch - '0';
		++cnt;

		if (1 != file->read(&inch,1))  // Get next byte from file and check for read error
		{
			Serial.printf(OnLine,line);
			Serial.println(HexReadErr);
			return (-2);
		}
	} while (inch != LF);

	return (val);
}

// Reads an FS file command line and returns the command character and any data
// The count of data is the return value, < 0 on errors, or the delay value for delay commands
// Data bytes are put into the 'data' array
int ReadFSline(File *file, int lcnt, uint8_t *cmd, uint8_t data[MAX_FS_DATA])
{
	int Val = 0, dcnt = 0;
	uint8_t inch;

		// Get command character
		if (1 != file->read(cmd,1))  // Get next byte from file and check for read error
		{
			Serial.printf(OnLine,lcnt);
			Serial.println(ReadError);
			return (-1);
		}

		// Check for valid command
		switch (*cmd)
		{
		case ';':  // Comment
			break;
		case 'W':  // Write data
		case 'C':  // Compare data
		case 'X':  // Delay
			if (!SkipColon(file,lcnt)) // Skip colon
				return (-1);
			break;
		default:
			Serial.printf(UnknownCmd,cmd,lcnt);
			return (-1);
			break;
		}

		// Get command data
		switch (*cmd)
		{
		case ';':  // Comment
			// Read till EOL (Toss data)
			do
			{
				if (1 != file->read(&inch,1))  // Get next byte from file and check for read error
				{
					Serial.printf(OnLine,lcnt);
					Serial.println(ReadError);
					return (-1);
				}
			} while (inch != LF);
			break;
		case 'W':  // Write data
		case 'C':  // Compare data
			// loop thru reading hex bytes till EOL
			do
			{
				Val = GetHexByte(file,lcnt);  // Get next hex byte
				if (-2 == Val) // Check for error
					return (-1);
				if (Val >= 0)
				{
					if (dcnt >= MAX_FS_DATA)
					{
						Serial.printf(OnLine,lcnt);
						Serial.printf(TooMuchData);
						return (-1);
					}

					data[dcnt] = Val;
					++dcnt;
				}
			} while (-1 != Val); // Loop till EOL
			if ('C' == *cmd && dcnt < 3)
			{
				Serial.printf(OnLine,lcnt);
				Serial.println("Too little data for compare");
				return (-1);
			}
			if ('W' == *cmd && dcnt < 3)
			{
				Serial.printf(OnLine,lcnt);
				Serial.println("Too little data for write");
				return (-1);
			}
			break;
		case 'X':  // Delay
			// Get int for delay
			Val = GetInteger(file,lcnt);  // Get next hex byte
			if (Val < 0) // Check for error
				return (-1);
			return (Val);
			break;
		}
		return (dcnt);
}

// Function to read in a config file
int ReadCfgFile(char *name, uint16_t SaveCfg[][2])
{
	File file;
	char line[100];
	int lcnt, p1, p2;
	int chunks=0, bytes=0;
	bool HadTotal=false;

	if (!(file = fatfs.open(name, O_READ)))  // Open file
	{
		Serial.println(FSopenErr);
		return(-1);
	}

	// Loop thru file lines
	lcnt = 1;
	while(file.fgets(line,100)>0)
	{
		// Process command
		switch(line[0])
		{
		case '#': // Comment
		case CR:  // Blank line
		case LF:
			break;  // Just toss
		case 'P': // Preserve
			if (HadTotal || 2 != sscanf(line+1," %d , %d",&p1,&p2))
			{
				Serial.printf(SyntaxError,lcnt);
				file.close();
				return (-1);
			}
			if (chunks >= MAX_CHUNKS)
			{
				Serial.println("Too many memeory chunks");
				file.close();
				return (-1);
			}
			SaveCfg[chunks][0] = p1;
			SaveCfg[chunks][1] = p2;
			++chunks;
			bytes += p2;
			if (bytes > GG_SAVE_SIZE)
			{
				Serial.println("Too many bytes saved");
				file.close();
				return (-1);
			}
			break;
		case 'T': // Totals
			if (HadTotal || 2 != sscanf(line+1," %d , %d",&p1,&p2))
			{
				Serial.printf(SyntaxError,lcnt);
				file.close();
				return (-1);
			}
			HadTotal = true;
			if (p1 != chunks || p2 != bytes)
			{
				Serial.printf("Incorrect Totals on line: %d\r\n",lcnt);
				file.close();
				return (-1);
			}
			break;
		default:  // Unknown character
			Serial.printf(SyntaxError,lcnt);
			file.close();
			return (-1);
		}
		++lcnt;
	}

	file.close();

	// Check if we validated the totals
	if (!HadTotal)
	{
		Serial.println("No totals in file");
		return(-1);
	}

	return (chunks);
}

// Function to recover a GG from ROM mode
void GG_Recover()
{
	File file;
	char name[MAX_NAME_LEN+3];
	uint8_t cmd, buf[5];
	int lcnt, cnt, dcnt;
	uint8_t data[MAX_FS_DATA];

	if (!BatteryType)  // Check for valid battery type
		return;

	lcnt = 1; // Clear line count
	ResetFS(); // Reset file system first

	// Check for gauge already in ROM mode
	Wire.beginTransmission(GG_ROM_MODE);  // ROM mode address
	if (!Wire.endTransmission())
	{
		FlushSerial();
		Serial.println("Found gauge in ROM mode. Enter name of fs file: ");
		name[0] = 0;  // Terminate buffer
		if (!GetText(name,MAX_NAME_LEN))
		{
			Serial.println();
			Serial.println("Canceled");
			Serial.println();
			return;
		}
	}
	else
	{  // Not in ROM mode
		Serial.println("No gauge found in ROM mode!");
		return;
	}

	STATUS_LED_BUSY;

	Serial.println();

	ClearValid(); // Clear valid data flags

	// Validate all input files
	// Check FS file
	Serial.println("Checking FS file");
	if (!(file = fatfs.open(name, O_READ)))  // Open file
	{
		Serial.println(FSopenErr);
		GG_Seal();
		STATUS_LED_READY;
		return;
	}

	while (file.available())  // Loop to check FS commands until EOF
	{
		// Get command character and data
		if ((dcnt = ReadFSline(&file,lcnt,&cmd,data)) < 0)
		{
			Serial.printf(OnLine,lcnt);
			Serial.println(ReadError);
			file.close();
			STATUS_LED_READY;
			return;
		}

		++lcnt;
	}

	file.close();  // Close FS file

	Serial.println();
	Serial.println("DO NOT REMOVE BATTERY TILL UPDATE DONE!!!");

	// Process FS file commands
	Serial.println("Writing FS file to gauge");
	if (!(file = fatfs.open(name, O_READ)))  // Open FS file
	{
		Serial.println(FSopenErr);
		GG_Seal();
		STATUS_LED_READY;
		return;
	}

	Serial.println();
	Serial.println("Processing line");
	lcnt = 1;  // Reset line counter

	while (file.available())  // Loop to do FS commands until EOF
	{
		// Get command character and data
		if ((dcnt = ReadFSline(&file,lcnt,&cmd,data)) < 0)
		{
			Serial.printf(OnLine,lcnt);
			Serial.println(ReadError);
			file.close();
			STATUS_LED_READY;
			return;
		}

		Serial.printf("\r%d",lcnt);

		// Do command
		switch (cmd)
		{
		case ';':  // Comment
			break;
		case 'W':  // Write data
			Wire.beginTransmission(data[0]>>1); // transmit to address in command
			if ((int)Wire.write(data+1,dcnt-1) != (dcnt-1)) // Write data
			{
				Serial.printf(OnLine,lcnt);
				Serial.println("Error queueing data for gauge");
				file.close();
				STATUS_LED_READY;
				return;
			}
			if ((cnt = Wire.endTransmission()))  // Do transmitting
			{
				Serial.printf(OnLine,lcnt);
				Serial.printf("Error code:%d writing to gauge\r\n",cnt);
				file.close();
				STATUS_LED_READY;
				return;
			}
			break;
		case 'C':  // Compare data
			Wire.beginTransmission(data[0]>>1); // transmit to address in command
			if (Wire.write(data+1,1) != 1) // Write address for read
			{
				Serial.printf(OnLine,lcnt);
				Serial.println("Error queueing data for compare");
				file.close();
				STATUS_LED_READY;
				return;
			}
			if ((cnt = Wire.endTransmission()))  // Do transmitting
			{
				Serial.printf(OnLine,lcnt);
				Serial.printf("Error code:%d writing to gauge\r\n",cnt);
				file.close();
				STATUS_LED_READY;
				return;
			}
			if ((int)Wire.requestFrom((uint8_t)(data[0]>>1),(uint8_t)(dcnt-2)) != (dcnt-2)) // Read data
			{
				Serial.printf(OnLine,lcnt);
				Serial.println("Error reading from gauge");
				file.close();
				STATUS_LED_READY;
				return;
			}
			// Do compare
			for (cnt = 2; cnt < dcnt; ++cnt)
			{
				if (Wire.read() != data[cnt])
				{
					Serial.printf(OnLine,lcnt);
					Serial.println("Error comparing FS file data");
					file.close();
					STATUS_LED_READY;
					return;
				}
			}
			break;
		case 'X':  // Delay
			delay(dcnt+4);
			break;
		}
		++lcnt;
	}

	file.close();
	--lcnt;
	Serial.printf("\r\nProcessed %d lines\r\n",lcnt);

	// Unseal GG
	Serial.println("Unsealing gauge");
	if (GG_FullUnseal())
	{
		STATUS_LED_READY;
		return;
	}

	// Set I2C drive strength
	Serial.println("Setting drive strength");
	buf[0] = 0x18;
	GG_BlockWrite(0x807f,buf,1);
	buf[0] = 42; // Make sure it don't match!
	Serial.println("Checking drive strength");
	if (!GG_BlockRead(0x807f,buf,1))
		Serial.println("Error checking drive strength!");
	if (buf[0] != 0x18)
		Serial.println("Error setting drive strength!");

	// Enable the gauge and data collection
	buf[0] = 0x28;  // Set the value for manufacture status init
	buf[1] = 0;  // to enable gauging and data collection
	if (!GG_BlockWrite(NGG_MFG_STATUS_INIT, buf, 2))
	{
		Serial.println("Error setting enables");
		GG_Seal();
		STATUS_LED_READY;
		return;
	}

	// Check for Update Status & 6 == 6
	Serial.println("Checking gauge status");
	if ((cnt = GG_UpdateStatus()) < 0)
	{
		STATUS_LED_READY;
		GG_Seal();
		return;
	}
	if (0x06 != (0x06 & cnt))
	{
		Serial.println("Error: enables not set!");
		GG_Seal();
		STATUS_LED_READY;
		return;
	}

	Serial.println("Sealing gauge");
	GG_Seal();

	Serial.println();
	Serial.println("Battery GG recover success!");
	Serial.println();

	STATUS_LED_READY;
}

// Function to update GG from FS file
void GG_Update()
{
	File file;
	uint16_t SaveMap[MAX_CHUNKS][2];
	int Chunks;
	char name[50], hex[50], cfg[50];
	uint8_t inch, cmd, buf[5];
	int lcnt, cnt, dcnt, scnt;
	uint8_t data[MAX_FS_DATA], VerboseSaved;
	bool DoSave, InROMmode;
	uint8_t GGsave[GG_SAVE_SIZE];  // Storage for saved GG data

	if (!BatteryType)  // Check for valid battery type
		return;

	lcnt = 1; // Clear line count
	ResetFS(); // Reset file system first

	// Check for gauge already in ROM mode
	Wire.beginTransmission(GG_ROM_MODE);  // ROM mode address
	if (!Wire.endTransmission())
	{
		FlushSerial();
		Serial.print("Gauge already in ROM mode. cannot save data, procede anyway (y/n)?");
		while(!Serial.available())
		;
		inch = Serial.read();
		Serial.println();
		if (inch == 'y' || inch == 'Y')
		{
			InROMmode = true;
			DoSave = false; // Can't save data in ROM mode
		}
		else
			return;
	}
	else
	{  // Not in ROM mode, do save of data
		InROMmode = false;
		DoSave = true;
	}

	// Set to terse, save current state
	VerboseSaved = Verbose;
	Verbose = 0;

	STATUS_LED_BUSY;

	// Check if battery is OK
	Serial.println("Checking battery");

	// Get gauge data if we have a gauge we can talk to
	if (!InROMmode)
	{
		if (newGG_GetStuff(&BD.NG))
		{
			Serial.println("Invalid battery, can't update GG");
			STATUS_LED_READY;
			Verbose = VerboseSaved; // Restore verbose setting
			return;
		}
	}

	// Read auth chip data into the buffer
	AUTH_Read(BD.Buf);
	if (ValidatePPP_V2(!InROMmode))
	{
		Serial.println();
		Serial.println("Invalid battery, can't update GG");
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	Serial.println("Checking auth chip");
	if (CheckAuthChip(false))
	{
		Serial.println();
		Serial.println("Non-Zebra auth chip, can't update GG");
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	Serial.println();
	ClearValid(); // Clear valid data flags

	// Drop rev from part number string
	for (cnt = 0; BD.NG.PartNumber[cnt]; ++cnt)
		if ('%' == BD.NG.PartNumber[cnt])
			BD.NG.PartNumber[cnt] = 0;

	// Get part number, create file names
	sprintf(name,"GG/%s.fs",(char *)BD.NG.PartNumber);
	sprintf(hex,"GG/%s.hex",(char *)BD.NG.PartNumber);
	sprintf(cfg,"GG/%s.cfg",(char *)BD.NG.PartNumber);

	// Validate all input files
	// Check FS file
	Serial.println("Checking FS file");
	if (!(file = fatfs.open(name, O_READ)))  // Open file
	{
		Serial.println(FSopenErr);
		GG_Seal();
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	while (file.available())  // Loop to check FS commands until EOF
	{
		// Get command character and data
		if ((dcnt = ReadFSline(&file,lcnt,&cmd,data)) < 0)
		{
			Serial.printf(OnLine,lcnt);
			Serial.println(ReadError);
			file.close();
			STATUS_LED_READY;
			Verbose = VerboseSaved; // Restore verbose setting
			return;
		}

		++lcnt;
	}

	file.close();  // Close FS file

	// Check hex file
	Serial.println("Checking HEX file");
	if (GetHexFile(hex,MAX_AUTH_SIZE,false))
	{
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	// Import config file
	Serial.println("Checking config file");
	if ((Chunks = ReadCfgFile(cfg,SaveMap)) < 0)
	{
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	Serial.println();
	Serial.println("DO NOT REMOVE BATTERY TILL UPDATE DONE!!!");

	if (!InROMmode) // Have a gauge we can talk to?
	{ // Yes
		// Unseal GG
		Serial.println("Unsealing gauge");
		if (GG_FullUnseal())
		{
			GG_Seal();  // Just in case it's stuck unsealed
			STATUS_LED_READY;
			Verbose = VerboseSaved; // Restore verbose setting
			return;
		}
	}

	// Save data if needed
	if (DoSave)
	{
		Serial.println("Saving GG data");

		// Read V2 gauge data
		scnt = 0;  // Reset saved data pointer

		// Loop thru the map
		for (cnt = 0; cnt < Chunks; ++cnt)
		{
			// Loop thru the 32 byte blocks in the map section
			lcnt = 0; // Clear counts of bytes read
			do
			{
				// Figure bytes to read
				if ((SaveMap[cnt][1] - lcnt) > 32)  // Over a block?
					dcnt = 32; // Do full block read
				else
					dcnt = SaveMap[cnt][1] - lcnt;  // Partial read

				// Data block from current address
				if (GG_BlockRead(SaveMap[cnt][0]+lcnt+NGG_DATA_FLASH, GGsave+scnt, dcnt) < dcnt)
				{
					Serial.println("Error saving GG data");
					GG_Seal();
					STATUS_LED_READY;
					Verbose = VerboseSaved; // Restore verbose setting
					return;
				}
				scnt += dcnt;  // Update total bytes read
				lcnt += dcnt;  // Update bytes read in this blook
			} while (lcnt < SaveMap[cnt][1]); // Bytes left to read?
		}
	}

	// Process FS file commands
	Serial.println("Writing FS file to gauge");
	if (!(file = fatfs.open(name, O_READ)))  // Open FS file
	{
		Serial.println(FSopenErr);
		GG_Seal();
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	Serial.println();
	Serial.println("Processing line");
	lcnt = 1;  // Reset line counter

	while (file.available())  // Loop to do FS commands until EOF
	{
		// Get command character and data
		if ((dcnt = ReadFSline(&file,lcnt,&cmd,data)) < 0)
		{
			Serial.printf(OnLine,lcnt);
			Serial.println(ReadError);
			file.close();
			STATUS_LED_READY;
			Verbose = VerboseSaved; // Restore verbose setting
			return;
		}

		Serial.printf("\r%d",lcnt);

		// Do command
		switch (cmd)
		{
		case ';':  // Comment
			break;
		case 'W':  // Write data
			Wire.beginTransmission(data[0]>>1); // transmit to address in command
			if ((int)Wire.write(data+1,dcnt-1) != (dcnt-1)) // Write data
			{
				Serial.printf(OnLine,lcnt);
				Serial.println("Error queueing data for gauge");
				file.close();
				STATUS_LED_READY;
				Verbose = VerboseSaved; // Restore verbose setting
				return;
			}
			if ((cnt = Wire.endTransmission()))  // Do transmitting
			{
				Serial.printf(OnLine,lcnt);
				Serial.printf("Error code:%d writing to gauge\r\n",cnt);
				file.close();
				STATUS_LED_READY;
				Verbose = VerboseSaved; // Restore verbose setting
				return;
			}
			break;
		case 'C':  // Compare data
			Wire.beginTransmission(data[0]>>1); // transmit to address in command
			if (Wire.write(data+1,1) != 1) // Write address for read
			{
				Serial.printf(OnLine,lcnt);
				Serial.println("Error queueing data for compare");
				file.close();
				STATUS_LED_READY;
				Verbose = VerboseSaved; // Restore verbose setting
				return;
			}
			if ((cnt = Wire.endTransmission()))  // Do transmitting
			{
				Serial.printf(OnLine,lcnt);
				Serial.printf("Error code:%d writing to gauge\r\n",cnt);
				file.close();
				STATUS_LED_READY;
				Verbose = VerboseSaved; // Restore verbose setting
				return;
			}
			if ((int)Wire.requestFrom((uint8_t)(data[0]>>1),(uint8_t)(dcnt-2)) != (dcnt-2)) // Read data
			{
				Serial.printf(OnLine,lcnt);
				Serial.println("Error reading from gauge");
				file.close();
				STATUS_LED_READY;
				Verbose = VerboseSaved; // Restore verbose setting
				return;
			}
			// Do compare
			for (cnt = 2; cnt < dcnt; ++cnt)
			{
				if (Wire.read() != data[cnt])
				{
					Serial.printf(OnLine,lcnt);
					Serial.println("Error comparing FS file data");
					file.close();
					STATUS_LED_READY;
					Verbose = VerboseSaved; // Restore verbose setting
					return;
				}
			}
			break;
		case 'X':  // Delay
			delay(dcnt+4);
			break;
		}
		++lcnt;
	}

	file.close();
	--lcnt;
	Serial.printf("\r\nProcessed %d lines\r\n",lcnt);

	// Unseal GG
	Serial.println("Unsealing gauge");
	if (GG_FullUnseal())
	{
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	// Set I2C drive strength
	Serial.println("Setting drive strength");
	buf[0] = 0x18;
	GG_BlockWrite(0x807f,buf,1);
	buf[0] = 42; // Make sure it don't match!
	Serial.println("Checking drive strength");
	if (!GG_BlockRead(0x807f,buf,1))
		Serial.println("Error checking drive strength!");
	if (buf[0] != 0x18)
		Serial.println("Error setting drive strength!");

	// Restore saved data if needed
	if (DoSave)
	{
		Serial.println("Restoring GG data");

		scnt = 0;  // Reset saved data pointer

		// Loop thru the map
		for (cnt = 0; cnt < Chunks; ++cnt)
		{
			// Loop thru the 32 byte blocks in the map section
			lcnt = 0; // Clear counts of bytes read
			do
			{
				// Figure bytes to read
				if ((SaveMap[cnt][1] - lcnt) > 32)  // Over a block?
					dcnt = 32; // Do full block read
				else
					dcnt = SaveMap[cnt][1] - lcnt;  // Partial write

				// Data block from current address
				if (!GG_BlockWrite(SaveMap[cnt][0]+lcnt+NGG_DATA_FLASH, GGsave+scnt, dcnt))
				{
					Serial.println("Error restoring data");
					GG_Seal();
					STATUS_LED_READY;
					Verbose = VerboseSaved; // Restore verbose setting
					return;
				}
				scnt += dcnt;  // Update total bytes read
				lcnt += dcnt;  // Update bytes read in this blook
			} while (lcnt < SaveMap[cnt][1]); // Bytes left to read?
		}
	}

	// Enable the gauge and data collection
	buf[0] = 0x28;  // Set the value for manufacture status init
	buf[1] = 0;  // to enable gauging and data collection
	if (!GG_BlockWrite(NGG_MFG_STATUS_INIT, buf, 2))
	{
		Serial.println("Error setting enables");
		GG_Seal();
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	// Check for Update Status & 6 == 6
	Serial.println("Checking gauge status");
	if ((cnt = GG_UpdateStatus()) < 0)
	{
		STATUS_LED_READY;
		GG_Seal();
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}
	if (0x06 != (0x06 & cnt))
	{
		Serial.println("Error: enables not set!");
		GG_Seal();
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	Serial.println("Sealing gauge");
	GG_Seal();

	Serial.println();
	Serial.println("Updating Auth data");

	// Get file into buffer
	if (GetHexFile(hex,MAX_AUTH_SIZE,false))
	{
		STATUS_LED_READY;
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}
	AUTH_Update(false);  // Do update

	// Recheck battery
	Serial.println();
	Serial.println("Rechecking battery");

	// Read auth chip data into the buffer
	ClearValid(); // Clear valid data flags
	AUTH_Read(BD.Buf);
	if (Validate_AUTH(PPP_V2_DATA))
	{
		Serial.println();
		Serial.println("Invalid battery, failed update");
		Verbose = VerboseSaved; // Restore verbose setting
		return;
	}

	Serial.println();
	Serial.println("Battery GG update success!");
	Serial.println();

	Verbose = VerboseSaved; // Restore verbose setting
	STATUS_LED_READY;
}

// Function to read the manufacture status from the new gauge
// Returns 0 on error, Update status value on success
int GG_UpdateStatus(void)
{
	uint8_t tmp = 128;

	if (!GG_BlockRead(NGG_UPDATE_STATUS, &tmp, 1))
	{
		Serial.println("Error reading update status!");
		return (-1);
	}

	return(tmp);
}

// Function to read all gas gauge registers
uint8_t GG_ReadReg(void)
{
	int BytesRead; // Bytes read from I2C
	uint8_t Total = 0;
	uint8_t data[32], *cpnt, cnt;
	int16_t tmp;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear ok flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	STATUS_LED_BUSY

	// Setup read of control status
	// Setup data
	data[0] = CNTL_CMD+Total;
	data[1] = GG_READ_CONTROL_STATUS & 0xff;
	data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send request for regs
	{
		Serial.println("Error setting ctrl/status read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// Read all reg in 32 byte chunks
	Total = 0;
	do {
		if (SwitchClk) // Enable clock drive if needed
		{
			ActiveClk();
			delay(15);
			PassiveClk();
		}
		else
			delay(1);

		// Figure bytes to read
		if ((GB_CMD_FULL_READ_SIZE - Total) >= 32)
			cnt = 32;
		else
			cnt = GB_CMD_FULL_READ_SIZE - Total;

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Get data
		// Setup data
		data[0] = CNTL_CMD+Total;

		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,1) != 1) // Send request for regs
		{
			Serial.println("Error setting reg read");
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		// Try I2C read of all regs bytes
		BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, cnt, DO_STOP);

		if (cnt != BytesRead)
		{
			Serial.println();
			Serial.println(BadRead);
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read reg data
		cpnt = (uint8_t *)&BD.Gifted.CONTROL_STATUS;
		cpnt += Total;
		for (cnt = 0; cnt < BytesRead; ++cnt)
			*cpnt++ = Wire.read();

		Total += BytesRead;
	} while (GB_CMD_FULL_READ_SIZE > Total);

	// Convert temp
	if (BD.Gifted.TEMP < 2032 || BD.Gifted.TEMP > 3732) // Out of range (-70C to +100C), set to invalid
	{
		CurTemp =  -70;
		Serial.println("Bad temp!!!");
		ret = 1;
		goto ReadError;
	}
	else
	{  // Convert to deg C
		tmp = (int16_t)BD.Gifted.TEMP - 2732;  // Convert to .1 deg C
		tmp = tmp / 10;  // Convert to deg C

		CurTemp = tmp; // Set new temp
	}

	// Get time since first use
	dwSecSinceFirstUse = ((uint32_t)BD.Gifted.ETU << 16) | (uint32_t)BD.Gifted.ETL;

	ret = 0;
	GasGaugeOK = 1; // Set ok flag

ReadError:
	STATUS_LED_READY

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
}

// Function to read all new gas gauge registers
uint8_t newGG_ReadReg(uint8_t *buffer)
{
	int BytesRead; // Bytes read from I2C
	uint8_t Total = 0;
	uint8_t data[3], cnt;
	int16_t tmp;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear ok flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return (ret);
	}

	if (!NEW_GAUGE)
	{
		Serial.println("Old gas gauge?");
		return (ret);
	}

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	STATUS_LED_BUSY

	// Read all reg in 32 byte chunks
	Total = 0;
	do {
		delay(1);

		// Figure bytes to read
		if ((NEW_GB_CMD_FULL_READ_SIZE - Total) >= 32)
			cnt = 32;
		else
			cnt = NEW_GB_CMD_FULL_READ_SIZE - Total;

		// Get data
		// Setup data
		data[0] = CNTL_CMD+Total;

		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,1) != 1) // Send request for regs
		{
			Serial.println("Error setting reg read");
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		// Try I2C read of all regs bytes
		BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, cnt, DO_STOP);

		if (cnt != BytesRead)
		{
			Serial.println();
			Serial.println(BadRead);
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read reg data
		for (cnt = 0; cnt < BytesRead; ++cnt)
			buffer[Total+cnt] = Wire.read();

		Total += BytesRead;
	} while (NEW_GB_CMD_FULL_READ_SIZE > Total);

	// Convert temps
	CurTemp = buffer[NGG_TEMP] | (buffer[NGG_TEMP+1] << 8);
	if ( CurTemp < 2032 || CurTemp > 3732) // Out of range (-70C to +100C), set to invalid
	{
		CurTemp =  -70;
		Serial.println("Bad temp!!!");
		goto ReadError;
	}
	else
	{  // Convert to deg C
		tmp = CurTemp - 2732;  // Convert to .1 deg C
		tmp = tmp / 10;  // Convert to deg C

		CurTemp = tmp; // Set new temp
	}
	CurIntTemp = buffer[NGG_INT_TEMP] | (buffer[NGG_INT_TEMP+1] << 8);
	if ( CurIntTemp < 2032 || CurIntTemp > 3732) // Out of range (-70C to +100C), set to invalid
	{
		CurIntTemp =  -70;
		Serial.println("Bad internal temp!!!");
		goto ReadError;
	}
	else
	{  // Convert to deg C
		tmp = CurIntTemp - 2732;  // Convert to .1 deg C
		tmp = tmp / 10;  // Convert to deg C

		CurIntTemp = tmp; // Set new temp
	}

	// Get gauging status
	if (!GG_BlockRead(NGG_GAUGING_STATUS,(uint8_t *)&BD.NG.GaugingStatus,4))
		goto ReadError;

/* ***FIX***
	// Get time since first use
	dwSecSinceFirstUse = ((uint32_t)BD.Gifted.ETU << 16) | (uint32_t)BD.Gifted.ETL;
*/

	ret = 0;
	GasGaugeOK = 1; // Set ok flag

ReadError:
	STATUS_LED_READY
	delay(1);

	return (ret);
}

// Function to read the Alt manufacturer block
// Returns 0 on error, size of block otherwise
uint8_t GG_BlockRead(uint16_t cmd, uint8_t data[], uint8_t size)
{
	int BytesRead; // Bytes read from I2C
	uint8_t cnt, csum;
	uint8_t buf[36];

	if (CheckClk()) // Check for clock not "stuck"
		return (0);

	// Send command
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup command
	buf[0] = CNTL_CMD;
	buf[1] = cmd & 0xff;
	buf[2] = (cmd >> 8) & 0xff;
	if (Wire.write(buf,3) != 3) // Send command
	{
		Serial.println("Error writing block read command");
		Wire.endTransmission();    // Stop transmitting
		return (0);
	}
	Wire.endTransmission();    // stop transmitting

	delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		return (0);

	buf[0] = ALT_MANF_ACC;  // Set to MACData address
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(buf,1) != 1) // Send request for MACData read
	{
		Serial.println("Error setting up block read");
		Wire.endTransmission();    // Stop transmitting
		return (0);
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		return (0);

	// I2C read of 36 bytes, must read full block
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)36, DO_STOP);

	if (36 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		return (0);
	}

	// Read 36 bytes
	for (cnt = 0; cnt < 36; ++cnt)
		buf[cnt] = Wire.read();

	// Check length
	if (buf[BLK_READ_LEN] < 5 || buf[BLK_READ_LEN] > 36)
	{
		Serial.println();
		Serial.print("Bad block read len");
		return (0);
	}

	// Check checksum
	csum = buf[BLK_READ_CSUM] + 1;
	for (cnt = 0; cnt < (buf[BLK_READ_LEN]-2); ++cnt)
		csum += buf[cnt];
	if (csum)
	{
		Serial.println();
		Serial.println("Bad block read csum!");
		return (0);
	}

	// Check command
	if (buf[0] != (cmd & 0xff) || buf[1] != ((cmd >> 8) & 0xff))
	{
		Serial.println();
		Serial.println("Block read cmd mismatch!");
		Serial.println(buf[0],HEX);
		Serial.println(buf[1],HEX);
		return (0);
	}

	// Copy over data
	for (cnt = 0; (cnt < (buf[BLK_READ_LEN]-4)) && (cnt < size); ++cnt)
		data[cnt] = buf[cnt+2];

	return (buf[BLK_READ_LEN]);
}

// Function to write the Alt manufacturer block
// Returns 0 on error, 1 otherwise.
uint8_t GG_BlockWrite(uint16_t cmd, uint8_t buf[], uint8_t len)
{
	uint8_t data[37];
	uint8_t cnt, csum;

	// Check that write will fit into 32 bytes
	if (len > 32)
	{
		Serial.println("Too many bytes in block write");
		return (0);
	}
	data[0] = ALT_MANF_ACC;  // Set to MACData address
	data[1] = cmd & 0xff;  // Set up command
	csum = data[1];
	data[2] = (cmd >> 8) & 0xff;
	csum += data[2];

	// Move data to buffer
	for (cnt = 0; cnt < len; ++cnt)
	{
		data[cnt+3] = buf[cnt];
		csum += data[cnt+3];
	}

	// Write data
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,len+3) != (len+3u)) // Send block write
	{
		Serial.println("Error writing block!");
		Wire.endTransmission();    // Stop transmitting
		return (0);
	}
	Wire.endTransmission();    // stop transmitting

	// Write checksum/len
	// Set checksum and length
	data[0] = MAC_DATA_CSUM;  // Set to checksum address
	data[1] = ~csum;
	data[2] = len + 4;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send block write
	{
		Serial.println("Error in block write!");
		Wire.endTransmission();    // Stop transmitting
		return (0);
	}
	Wire.endTransmission();    // stop transmitting

	return (1);
}

// Function to get the gas gauge data
uint8_t GG_GetStuff(GIFTED_BATT_DATA_t *BattData)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[36], *cpnt, cnt;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear ok flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	STATUS_LED_BUSY

	// Get device type
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_DEVICE_TYPE & 0xff;
	data[2] = (GG_DEVICE_TYPE >> 8) & 0xff;
	if (Wire.write(data,3) != 3) // Setup read of device type
	{
		Serial.println("Error writing get device type command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for device type
	{
		Serial.println("Error setting up device type read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 2 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (2 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read device type
	data[0] = Wire.read();
	data[1] = Wire.read();
	BattData->GG_DeviceType = (uint16_t)data[1] << 8 | (uint16_t)data[0];

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get firmware version
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_FW_VERSION & 0xff;
	data[2] = (GG_FW_VERSION >> 8) & 0xff;
	if (Wire.write(data,3) != 3) // Setup read of device type
	{
		Serial.println("Error writing get version command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for version
	{
		Serial.println("Error setting up version type read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 2 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (2 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read firmware version
	data[0] = Wire.read();
	data[1] = Wire.read();
	BattData->GG_FirmwareVer = (uint16_t)data[1] << 8 | (uint16_t)data[0];

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get hardware version
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_HW_VERSION & 0xff;
	data[2] = (GG_HW_VERSION >> 8) & 0xff;
	if (Wire.write(data,3) != 3) // Setup read of hardware version
	{
		Serial.println("Error writing get hardware version command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for hardware version
	{
		Serial.println("Error setting up hardware version read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 2 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (2 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read hardware version
	data[0] = Wire.read();
	data[1] = Wire.read();
	BattData->GG_HardwareVer = (uint16_t)data[1] << 8 | (uint16_t)data[0];

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get manf ID
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = MFGID_CMD;
	if (Wire.write(data,1) != 1) // Setup read of manf ID
	{
		Serial.println("Error writing manf ID command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// Try I2C read of 8 bytes
	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)8, DO_STOP);

	if (8 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf ID
	for (cnt = 0; cnt < 8; ++cnt)
		BattData->M200_Manf_ID[cnt] = Wire.read();

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get design capacity
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	data[0] = DCAP_CMD;
	if (Wire.write(data,1) != 1) // Setup read of design cap
	{
		Serial.println("Error writing design cap command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// Try I2C read of 2 bytes
	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (2 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read design cap
	data[0] = Wire.read();
	data[1] = Wire.read();
	DesignCap = (uint16_t)data[1] << 8 | (uint16_t)data[0];

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Read manf blocks
	// Get Manf blockA
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_A;
	if (Wire.write(data,2) != 2) // Setup read of manf block A
	{
		Serial.println("Error writing manf block A command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println("Error setting up block read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockA data
	for (cnt = 0; cnt < 32; ++cnt)
		BattData->BlockA[cnt] = Wire.read();

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockB
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_B;
	if (Wire.write(data,2) != 2) // Setup read of manf ID
	{
		Serial.println("Error writing manf blockB command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println("Error setting up block read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockB data
	for (cnt = 0; cnt < 32; ++cnt)
		BattData->BlockB[cnt] = Wire.read();

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockC
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_C;
	if (Wire.write(data,2) != 2) // Setup read of manf ID
	{
		Serial.println("Error writing manf blockC command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println("Error setting up block read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockC data
	cpnt = (uint8_t *)&BattData->Checksum_c;
	for (cnt = 0; cnt < 32; ++cnt)
		cpnt[cnt] = Wire.read();

	ret = 0;
	GasGaugeOK = 1; // Set ok flag

ReadError:
	STATUS_LED_READY

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return(ret);
}

// Function to get the new gas gauge data
uint8_t newGG_GetStuff(NEW_GIFTED_BATT_DATA_t *BattData)
{
	uint8_t data[36], cnt, cnt2;
	uint8_t ret = 1;
	uint8_t regs[NEW_GB_CMD_FULL_READ_SIZE];

	GasGaugeOK = 0; // Clear ok flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return (ret);
	}

	if (!NEW_GAUGE)
	{
		Serial.println("Old gas gauge?");
		return (ret);
	}

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	STATUS_LED_BUSY
	// Get device type
	if (!GG_BlockRead(GG_DEVICE_TYPE,(uint8_t *)&BattData->GG_DeviceType))
		goto ReadError;

	// Get firmware version
	if (!GG_BlockRead(GG_FW_VERSION,BattData->GG_FirmwareVer))
		goto ReadError;

	// Get hardware version
	if (!GG_BlockRead(GG_HW_VERSION,(uint8_t *)&BattData->GG_HardwareVer))
		goto ReadError;

	// Get Qmax day
	if (!GG_BlockRead(GG_QMAX_DAY,(uint8_t *)&BattData->QMAX_DAY,2))
		goto ReadError;

	// Get Qmax DOD
	if (!GG_BlockRead(NGG_ITSTATUS3,data,14))
		goto ReadError;

	BattData->QMAXDOD0 = *((uint16_t *)(data+2));

	delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get time in cap ranges over temps
	for (cnt = 0; cnt < NUM_TEMPS; ++cnt)
	{
		if (!GG_BlockRead(NGG_LIFE_TIME_CAPS+cnt,data))
			goto ReadError;

		// Copy over time at caps
		for (cnt2 = 0; cnt2 < NUM_CAPS; ++cnt2)
			BD.NG.times[cnt][cnt2] = (uint32_t)data[(cnt2*4)+3] << 24 | (uint32_t)data[(cnt2*4)+2] << 16 | (uint32_t)data[(cnt2*4)+2] << 8 | (uint32_t)data[(cnt2*4)];
	}

	delay(1);

	if(!GG_BlockRead(NGG_MANF_NAME,BattData->ManufName))
		goto ReadError;

	if (CheckClk()) // FCheck for clock not "stuck"
		goto ReadError;

	// Get date made
	if (!GG_BlockRead(NGG_MANF_DATE,(uint8_t *)&BattData->DateMade))
		goto ReadError;

	delay(1);

	if (!GG_BlockRead(NGG_SERIAL_NUM,(uint8_t *)&BattData->SerialNum))
		goto ReadError;

	// Read manf blocks
	if (!GG_BlockRead(NGG_MANF_BLKA,(uint8_t *)&BattData->InitData))
		goto ReadError;
	if (!GG_BlockRead(NGG_MANF_BLKB,((uint8_t *)&BattData->InitData)+32))
		goto ReadError;
	if (!GG_BlockRead(NGG_MANF_BLKC,((uint8_t *)&BattData->InitData)+64))
		goto ReadError;

	if (newGG_ReadReg(regs))
		goto ReadError;
	NGGsecSinceMade = *(uint32_t *)(regs+NGG_TIMESTAMP);

	ret = 0;
	GasGaugeOK = 1; // Set ok flag

ReadError:
	STATUS_LED_READY
	delay(1);

	return(ret);
}

// Function to set timestamp to 30
// days past qmax day to force a cal
uint8_t GG_ForceCal(void)
{
	uint8_t ret;
	uint32_t NewTime;
	uint8_t data[5];
	uint16_t qmax;


	// Ask about unseal
	Serial.println("Forcing cal requiers an unseal of the gauge, continue (Y/N)? ");
	while(!Serial.available())
	;
	ret = Serial.read();
	if (ret != 'y' && ret != 'Y')
		return(0);

	STATUS_LED_BUSY;
	// Get QMAX_DAY
	Serial.println("Geting QMAX_DAY");
	if (!GG_BlockRead(GG_QMAX_DAY,(uint8_t *)&qmax,2))
		goto Error;

	// Add 31 days
	NewTime = (qmax * 3600 * 24) + (31 * 3600 * 24);

	// Unseal the gauge
	Serial.println("Unseal gauge");
	if (GG_Unseal())
	{ // Error full unsealing
		GG_Seal();  // Just in case it's stuck unsealed
		goto Error;
	}

	// Save to timestamp
	Serial.println("Save new timestamp");

	// Setup data
	data[0] = NGG_TIMESTAMP;
	data[1] = NewTime & 0xff;
	data[2] = (NewTime >> 8) & 0xff;
	data[3] = (NewTime >> 16) & 0xff;
	data[4] = (NewTime >> 24) & 0xff;

	if (CheckClk()) // Check for clock not "stuck"
		goto Error;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,5) != 5) // Send write of timestamp
		Serial.println("Error updating the timestamp");
	else
		ret = 0;
	Wire.endTransmission();    // stop transmitting

Error:
	// reseal the gauge
	Serial.println("Sealing gauge");
	if (GG_Seal())
		ret = 1;
	STATUS_LED_READY;

	return(ret);
}

uint8_t GG_EnterHibernate(void)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[10], *cpnt;
	uint8_t ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return (ret);
	}

	Serial.println("Put gas gauge into hibernate (y/n)?");
	while(!Serial.available())
	;
	data[0] = Serial.read();
	if (data[0] != 'y' && data[0] != 'Y')
		return(ret);

	// Do reg read to get the current time
	if (GG_ReadReg())
		return(ret);

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	// Setup write of control status
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_SET_FULLSLEEP & 0xff;
	data[2] = (GG_SET_FULLSLEEP >> 8) & 0xff;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(HibErr);
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// Setup write of control status
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_SET_HIBERNATE & 0xff;
	data[2] = (GG_SET_HIBERNATE >> 8) & 0xff;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(HibErr);
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// read control status reg
	data[0] = CNTL_CMD;
	data[1] = GG_READ_CONTROL_STATUS & 0xff;
	data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error setting ctrl/status read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get data
	// Setup data
	data[0] = CNTL_CMD;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for regs
	{
		Serial.println("Error setting reg read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of control/status reg
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (BytesRead != 2)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read reg data
	cpnt = (uint8_t *)&BD.Gifted.CONTROL_STATUS;
	*cpnt++ = Wire.read();
	*cpnt = Wire.read();

	ret = 0;

	if (BD.Gifted.CONTROL_STATUS & STATUS_LOW_HIBERNATE_BIT)
		Serial.println("Hibernate bit was successfully set");
	else
	{
		Serial.println("ERROR: Hibernate bit was NOT set");
		ret = 1;
	}

	if (BD.Gifted.CONTROL_STATUS & STATUS_LOW_FULLSLEEP_BIT)
		Serial.println("Fullsleep bit was successfully set");
	else
	{
		Serial.println("ERROR: Fullsleep bit was NOT set");
		ret = 1;
	}

	Serial.print("Seconds since first use: ");
	Serial.println(dwSecSinceFirstUse);

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
}

uint8_t GG_SetFullSleep(uint8_t ask)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[10], *cpnt;
	uint8_t ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return (ret);
	}

	if (ask)
	{
		Serial.println("Set FULLSLEEP bit (y/n)?");
		while(!Serial.available())
		;
		data[0] = Serial.read();
		if (data[0] != 'y' && data[0] != 'Y')
			return(ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Setup write of control status
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_SET_FULLSLEEP & 0xff;
	data[2] = (GG_SET_FULLSLEEP >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println(HibErr);
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	// read control status reg
	data[0] = CNTL_CMD;
	data[1] = GG_READ_CONTROL_STATUS & 0xff;
	data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error setting ctrl/status read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get data
	// Setup data
	data[0] = CNTL_CMD;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for regs
	{
		Serial.println("Error setting reg read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of control/status reg
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (BytesRead != 2)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read reg data
	cpnt = (uint8_t *)&BD.Gifted.CONTROL_STATUS;
	*cpnt++ = Wire.read();
	*cpnt = Wire.read();

	ret = 0;

	if (BD.Gifted.CONTROL_STATUS & STATUS_LOW_FULLSLEEP_BIT)
		Serial.println("Fullsleep bit was successfully set");
	else
	{
		Serial.println("ERROR: Fullsleep bit was NOT set");
		ret = 1;
	}

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
}

uint8_t GG_CheckSeal(void)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[6];
	uint8_t ret = 1;
	uint16_t Status;

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Check if unsealed
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_READ_CONTROL_STATUS & 0xff;
	data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error sending control/status read!");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Setup data
	data[0] = CNTL_CMD;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for regs
	{
		Serial.println("Error setting reg read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// I2C read of reg
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

	if (BytesRead != 2)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read reg data
	Status = Wire.read();
	Status |= (Wire.read()<<8);

	// Check if it's full unsealed
	if (!(Status & (STATUS_HIGH_FAS_BIT << 8)))
	{
		Serial.println("Error: Gas Gauge is full unsealed!");
		goto ReadOK;
	}

	// Check if it's unsealed
	if (!(Status & (STATUS_HIGH_SS_BIT << 8)))
		Serial.println("Error: Gas Gauge is unsealed!");
	else
		ret = 0;

	goto ReadOK;

ReadError:
	Serial.println("Unable to check gas gauge seal state");
ReadOK:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
}

uint8_t GG_Unseal(void)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[6];
	uint8_t ret = 1, cnt;
	uint16_t Status;

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	delay(4000);  // TI says we need this

	// Send first unlock key
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = UNSEAL_KEY1 & 0xff;
	data[2] = (UNSEAL_KEY1 >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error sending first key");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send second unlock key
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = UNSEAL_KEY2 & 0xff;
	data[2] = (UNSEAL_KEY2 >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error sending second key");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	for (cnt = 0; cnt < 4; ++cnt)
	{
		delay (1000);  // Wait a bit

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Check if unsealed
		// Setup data
		data[0] = CNTL_CMD;
		data[1] = GG_READ_CONTROL_STATUS & 0xff;
		data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,3) != 3) // Send write of control/status
		{
			Serial.println("Error sending control/status read!");
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Setup data
		data[0] = CNTL_CMD;

		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,1) != 1) // Send request for regs
		{
			Serial.println("Error setting reg read");
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// I2C read of reg
		BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

		if (BytesRead != 2)
		{
			Serial.println();
			Serial.println(BadRead);
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read reg data
		Status = Wire.read();
		Status |= (Wire.read()<<8);

		// Check if it's unsealed
		if (!(Status & (STATUS_HIGH_SS_BIT << 8)))
		{
			ret = 0;
			goto ReadOK;
		}
	}

ReadError:
	Serial.println("Unable to unseal gas gauge");
ReadOK:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
}

uint8_t GG_FullUnseal(void)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[6];
	uint8_t ret = 1, cnt;
	uint16_t Status;

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	delay(4000); // TI says we need this

	// Send first unlock key
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = UNSEAL_KEY1 & 0xff;
	data[2] = (UNSEAL_KEY1 >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error sending first key");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	delay (100);

	// Send second unlock key
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = UNSEAL_KEY2 & 0xff;
	data[2] = (UNSEAL_KEY2 >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error sending first key");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (NEW_GAUGE)
		delay(1000); // Wait a bit, only needed for the new gauge

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send third unlock key
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = UNSEAL_KEY3 & 0xff;
	data[2] = (UNSEAL_KEY3 >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error sending first key");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send forth unlock key
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = UNSEAL_KEY4 & 0xff;
	data[2] = (UNSEAL_KEY4 >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error sending first key");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	for (cnt = 0; cnt < 4; ++cnt)
	{
		delay (1000);  // Wait a bit

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Check if unsealed
		// Setup data
		data[0] = CNTL_CMD;
		data[1] = GG_READ_CONTROL_STATUS & 0xff;
		data[2] = (GG_READ_CONTROL_STATUS >> 8) & 0xff;

		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,3) != 3) // Send write of control/status
		{
			Serial.println("Error sending control/status read!");
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Setup data
		data[0] = CNTL_CMD;

		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,1) != 1) // Send request for regs
		{
			Serial.println("Error setting reg read");
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// I2C read of reg
		BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)2, DO_STOP);

		if (BytesRead != 2)
		{
			Serial.println();
			Serial.println(BadRead);
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read reg data
		Status = Wire.read();
		Status |= (Wire.read()<<8);

		// Check if it's unsealed
		if (!(Status & ((STATUS_HIGH_SS_BIT << 8) | (STATUS_HIGH_FAS_BIT << 8))))
		{
			ret = 0;
			goto ReadOK;
		}
	}

ReadError:
	Serial.println("Unable to full unseal gas gauge");
ReadOK:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
}

void GG_Reset(uint8_t ask)
{
	uint8_t data[10];

	if (!BatteryType)  // Check for valid battery type
		return;

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return;
	}

	if (ask)
	{
		Serial.println("Reset gas gauge? ARE YOU SURE??? (y/n)?");
		while(!Serial.available())
		;
		data[0] = Serial.read();
		if (data[0] != 'y' && data[0] != 'Y')
			return;
		STATUS_LED_BUSY;
		// Assume since we asked, we are not unsealed
		if (GG_Unseal())
			goto ReadError;
	}

	STATUS_LED_BUSY;
	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send reset command
	// Setup data
	data[0] = CNTL_CMD;
	data[1] = GG_RESET_CHIP & 0xff;
	data[2] = (GG_RESET_CHIP >> 8) & 0xff;

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error sending reset");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	Serial.println("Gas gauge has been reset!");

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	STATUS_LED_READY;
}

void GG_do_Unseal(void)
{
	uint8_t data;

	Serial.println(Unseal);
	while(!Serial.available())
	;
	data = Serial.read();
	if (data != 'y' && data != 'Y')
		return;

	STATUS_LED_BUSY;
	if (!GG_Unseal())
		Serial.println("Gas gauge has been unsealed!");
	STATUS_LED_READY;

	return;
}

void GG_do_FullUnseal(void)
{
	uint8_t data;

	Serial.println(Unseal);
	while(!Serial.available())
	;
	data = Serial.read();
	if (data != 'y' && data != 'Y')
		return;

	STATUS_LED_BUSY;
	if (!GG_FullUnseal())
		Serial.println("Gas gauge has been fully unsealed!");
	STATUS_LED_READY;
}

void GG_do_Seal(void)
{
	if (!GG_Seal())
		Serial.println("Gas gauge has been sealed!");
}

uint8_t GG_Seal(void)
{
	uint8_t data[10];
	uint8_t ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Send seal command
	// Setup data
	data[0] = CNTL_CMD;
	if (!NEW_GAUGE)
	{  // Do M200 stuff
		data[1] = GG_SEAL_CHIP & 0xff;
		data[2] = (GG_SEAL_CHIP >> 8) & 0xff;
	}
	else // New gauge
	{
		data[1] = NGG_SEAL_CHIP & 0xff;
		data[2] = (NGG_SEAL_CHIP >> 8) & 0xff;
	}

	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,3) != 3) // Send write of control/status
	{
		Serial.println("Error sending seal");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	ret = 0;

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
}

uint8_t GG_GetNewManfBlocks(uint8_t *buffer)
{
	// Check the data
	GG_BlockRead(NGG_MANF_BLKA,buffer,32);
	GG_BlockRead(NGG_MANF_BLKB,buffer+32,32);
	GG_BlockRead(NGG_MANF_BLKC,buffer+64,32);
	return (0);
}

uint8_t GG_GetManfBlocks(uint8_t *buffer)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[32], cnt;
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear ok flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockA
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_A;
	if (Wire.write(data,2) != 2) // Setup read of manf ID
	{
		Serial.println("Error writing manf blockA command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println("Error setting up block read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockA data
	for (cnt = 0; cnt < 32; ++cnt)
		buffer[cnt] = Wire.read();

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockB
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_B;
	if (Wire.write(data,2) != 2) // Setup read of block B
	{
		Serial.println("Error writing manf block B command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println("Error setting up block read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockB data
	for (cnt = 0; cnt < 32; ++cnt)
		buffer[cnt+32] = Wire.read();

	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
		PassiveClk();
	}
	else
		delay(1);

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Get Manf blockC
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

	// Setup data
	data[0] = DFBLK_CMD;
	data[1] = MAN_BLK_C;
	if (Wire.write(data,2) != 2) // Setup read of manf ID
	{
		Serial.println("Error writing manf blockC command");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	data[0] = A_DF_CMD;
	Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
	if (Wire.write(data,1) != 1) // Send request for block data
	{
		Serial.println("Error setting up block read");
		Wire.endTransmission();    // Stop transmitting
		goto ReadError;
	}
	Wire.endTransmission();    // stop transmitting

	if (CheckClk()) // Check for clock not "stuck"
		goto ReadError;

	// Try I2C read of 32 bytes
	BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, (uint8_t)32, DO_STOP);

	if (32 != BytesRead)
	{
		Serial.println();
		Serial.println(BadRead);
		while(Wire.available())  // flush buffer
			Wire.read();
		goto ReadError;
	}

	// Read Manf BlockC data
	for (cnt = 0; cnt < 32; ++cnt)
		buffer[cnt+64] = Wire.read();

	ret = 0;

ReadError:
	if (SwitchClk) // Enable clock drive if needed
	{
		ActiveClk();
		delay(15);
	}
	else
		delay(1);

	return (ret);
}

uint8_t GG_GetDataFlash(DATA_FLASH_t *df)
{
	uint8_t ret = 1;

	GasGaugeOK = 0; // Clear ok flag

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return (ret);
	}

	// Unlock the gauge!
	if (GG_Unseal())
		return (ret);

	// Get Safety subclass
	if (GG_ReadSubClass(SC_SAFETY,sizeof(CFG_SAFETY_t),(uint8_t *)&df->CfgSafety))
		goto ReadError;

	// Convert data
	SwapBytes(df->CfgSafety.OT_Chg);
	SwapBytes(df->CfgSafety.OT_ChgRecovery);
	SwapBytes(df->CfgSafety.OT_Dsg);
	SwapBytes(df->CfgSafety.OT_DsgRecovery);

	// Get Charge Inhibit subclass
	if (GG_ReadSubClass(SC_CHRG_INHIB_CFG,sizeof(CFG_CHG_INHIBIT_t),(uint8_t *)&df->CfgChgInhibit))
		goto ReadError;

	// Get Charge subclass
	if (GG_ReadSubClass(SC_CHARGE,sizeof(CFG_CHARGE_t),(uint8_t *)&df->CfgCharge))
		goto ReadError;

	// Get Calibration data subclass
	if (GG_ReadSubClass(SC_CAL_DATA,sizeof(CAL_DATA_t),(uint8_t *)&df->CalData))
		goto ReadError;

	ret = 0;

ReadError:
	GG_Seal();  // return to sealed mode

	return (ret);
}

// Note, this function assumes the gauge is unsealed!
uint8_t GG_ReadSubClass(uint8_t SubClass, uint8_t Size, uint8_t *buffer)
{
	int BytesRead; // Bytes read from I2C
	uint8_t data[3], cnt, Block = 0, bytes;
	uint8_t ret = 1;

	if (!BatteryType)  // Check for valid battery type
		return(ret);

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return (ret);
	}

	if (SwitchClk) // Disable clock drive if needed
		PassiveClk();

	do
	{
		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Set subclass
		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge

		// Setup data
		data[0] = DFCLS;
		data[1] = SubClass;
		data[2] = Block;

		if (Wire.write(data,3) != 3) // Setup read of data flash
		{
			Serial.println("Error writing data flash block command");
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		data[0] = A_DF_CMD;
		Wire.beginTransmission((uint8_t)Batts[BatteryType].GasGaugeAddress); // transmit to gas gauge
		if (Wire.write(data,1) != 1) // Send request for block data
		{
			Serial.println("Error setting up block read");
			Wire.endTransmission();    // Stop transmitting
			goto ReadError;
		}
		Wire.endTransmission();    // stop transmitting

		if (CheckClk()) // Check for clock not "stuck"
			goto ReadError;

		// Try I2C read data
		if (Size > 32)
			bytes = 32;
		else
			bytes = Size;

		BytesRead = Wire.requestFrom(Batts[BatteryType].GasGaugeAddress, bytes, DO_STOP);

		if (bytes != BytesRead)
		{
			Serial.println();
			Serial.println(BadRead);
			while(Wire.available())  // flush buffer
				Wire.read();
			goto ReadError;
		}

		// Read data
		for (cnt = 0; cnt < bytes; ++cnt)
			buffer[cnt+(Block*32)] = Wire.read();

		if (SwitchClk) // Enable clock drive if needed
		{
			ActiveClk();
			delay(15);
			PassiveClk();
		}
		else
			delay(1);

		Size -= bytes;
	} while (Size);

	ret = 0;

ReadError:
	if (SwitchClk) // Enable clock drive if needed
		ActiveClk();

	return (ret);
}

void PrintFloatFromInt(int16_t val)
{
	if (val < 0)
	{
		Serial.print("-");
		Serial.print(abs(val)/10);
		Serial.print(".");
		Serial.println(abs(val)%10);
	}
	else
	{
		Serial.print(val/10);
		Serial.print(".");
		Serial.println(val%10);
	}
}

float ConvFloat(uint32_t InData)
{
	uint8_t vMSByte, vMidHiByte, vMidLoByte, vLSByte;
	uint8_t bIsPositive = 0;
	float fExponent, fResult;

	vLSByte = InData >> 24;
	vMidLoByte = (InData >> 16) &0xff;
	vMidHiByte = (InData >> 8) &0xff;
	vMSByte =  InData & 0xff;

//Serial.println();
//Serial.print("vMSByte,HEX);

	// Get the sign, its in the 0x00 80 00 00 bit
	if (!(vMidHiByte & 128))
		bIsPositive = 1;

	// Get the exponent, it's 2^(MSbyte - 0x80)
	fExponent = pow(2.0,vMSByte-128);

	// Or in 0x80 to the MidHiByte
	vMidHiByte = vMidHiByte | 128;

	// get value out of midhi byte
	fResult = vMidHiByte * pow(2.0,16.0);

	// add in midlow byte
	fResult = fResult + (vMidLoByte * pow(2.0,8.0));

	// add in LS byte
	fResult = fResult + (uint64_t)vLSByte;

	// multiply by 2^-24 to get the actual fraction
	fResult = fResult * pow(2.0,-24.0);

	// multiply fraction by the ‘exponent’ part
	fResult = fResult * fExponent;

	// Make negative if necessary
	if (!bIsPositive)
		fResult = -fResult;

	return (fResult);
}

void DumpNewGGConfig(void)
{
	// ***FIX***
	Serial.println("Dump GG config, coming soon!");
}

void SaveNewGGConfig(void)
{
	// ***FIX***
	Serial.println("Save new GG config, coming soon!");
}

void DumpGGConfig(void)
{
	DATA_FLASH_t df;  // Data flash struct

	if (GG_GetDataFlash(&df))
		return;

	Serial.println();
	Serial.println("[Safety(Configuration)]");
	Serial.print("OT Chg = ");
	PrintFloatFromInt(df.CfgSafety.OT_Chg);
	Serial.print("OT Chg Time = ");
	Serial.println(df.CfgSafety.OT_ChgTime);
	Serial.print("OT Chg Recovery = ");
	PrintFloatFromInt(df.CfgSafety.OT_ChgRecovery);
	Serial.print("OT Dsg = ");
	PrintFloatFromInt(df.CfgSafety.OT_Dsg);
	Serial.print("OT Dsg Time = ");
	Serial.println(df.CfgSafety.OT_DsgTime);
	Serial.print("OT Dsg Recovery = ");
	PrintFloatFromInt(df.CfgSafety.OT_DsgRecovery);

	Serial.print("CC Gain = ");
	Serial.println(ConvFloat(df.CalData.CC_Gain));

	Serial.println();
}

void SaveGGConfig(void)
{
	// ***FIX***
	Serial.println("Save GG config, coming soon!");
}

void GG_DriveStrength()
{
	uint8_t buf[2], inch;

	if (!BatteryType)  // Check for valid battery type
		return;

	if (!Batts[BatteryType].GasGaugeAddress)
	{
		Serial.println(NoGauge);
		return;
	}

	STATUS_LED_BUSY;

	Serial.println("Unsealing gauge");
	if (GG_FullUnseal())
	{
		STATUS_LED_READY;
		return;
	}

	// Check I2C drive strength
	buf[0] = 42; // Make sure it don't match!
	Serial.println("Checking drive strength");
	if (!GG_BlockRead(0x807f,buf,1))
	{
		Serial.println("Error checking drive strength!");
		GG_Seal();
		STATUS_LED_READY;
		return;
	}

	if (0x18 == buf[0])
	{
		Serial.println("Drive strength set");
		GG_Seal();
		STATUS_LED_READY;
		return;
	}

	Serial.println("Drive strength not set! Fix now (y/n) ");
	while(!Serial.available())
	;
	inch = Serial.read();
	Serial.println();
	if (inch != 'y' && inch != 'Y')
	{
		GG_Seal();
		STATUS_LED_READY;
		return;
	}

	Serial.println("Setting drive strength");
	buf[0] = 0x18;
	GG_BlockWrite(0x807f,buf,1);
	buf[0] = 42; // Make sure it don't match!
	Serial.println("Checking drive strength");
	if (!GG_BlockRead(0x807f,buf,1))
		Serial.println("Error checking drive strength!");
	else if (buf[0] != 0x18)
		Serial.println("Error setting drive strength!");
	else
		Serial.println("Drive strength set!");

		GG_Seal();
		STATUS_LED_READY;
}
