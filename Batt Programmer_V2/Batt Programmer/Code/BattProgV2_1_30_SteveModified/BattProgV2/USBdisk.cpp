// USB flash drive code

#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include <SdFat.h>
#include "ff.h"
#include "diskio.h"

#include "pwmbatt.h"
#include "BattProg.h"

//  ************* Flash/USB drive stuff *************
// Callback prototypes
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize);
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize);
void msc_flush_cb (void);

// Disk stuff
// Volume name, up to 11 characters
#define DISK_LABEL    "BATTPROG"

// Uncomment to run with FRAM
// #define FRAM_CS   A5
// #define FRAM_SPI  SPI

#if defined(FRAM_CS) && defined(FRAM_SPI)
  Adafruit_FlashTransport_SPI flashTransport(FRAM_CS, FRAM_SPI);

#else
  // On-board external flash (QSPI or SPI) macros should already
  // defined in your board variant if supported
  // - EXTERNAL_FLASH_USE_QSPI
  // - EXTERNAL_FLASH_USE_CS/EXTERNAL_FLASH_USE_SPI
  #if defined(EXTERNAL_FLASH_USE_QSPI)
    Adafruit_FlashTransport_QSPI flashTransport;

  #elif defined(EXTERNAL_FLASH_USE_SPI)
    Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

  #else
    #error No QSPI/SPI flash are defined on your board variant.h !
  #endif
#endif

Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatFileSystem fatfs;

//--------------------------------------------------------------------+
// fatfs diskio
//--------------------------------------------------------------------+
extern "C"
{

	DSTATUS disk_status ( BYTE pdrv )
	{
	  (void) pdrv;
		return 0;
	}

	DSTATUS disk_initialize ( BYTE pdrv )
	{
	  (void) pdrv;
		return 0;
	}

	DRESULT disk_read (
		BYTE pdrv,		/* Physical drive nmuber to identify the drive */
		BYTE *buff,		/* Data buffer to store read data */
		DWORD sector,	/* Start sector in LBA */
		UINT count		/* Number of sectors to read */
	)
	{
	  (void) pdrv;
		return flash.readBlocks(sector, buff, count) ? RES_OK : RES_ERROR;
	}

	DRESULT disk_write (
		BYTE pdrv,			/* Physical drive nmuber to identify the drive */
		const BYTE *buff,	/* Data to be written */
		DWORD sector,		/* Start sector in LBA */
		UINT count			/* Number of sectors to write */
	)
	{
	  (void) pdrv;
	  return flash.writeBlocks(sector, buff, count) ? RES_OK : RES_ERROR;
	}

	DRESULT disk_ioctl (
		BYTE pdrv,		/* Physical drive nmuber (0..) */
		BYTE cmd,		/* Control code */
		void *buff		/* Buffer to send/receive control data */
	)
	{
	  (void) pdrv;

	  switch ( cmd )
	  {
		case CTRL_SYNC:
		  flash.syncBlocks();
		  return RES_OK;

		case GET_SECTOR_COUNT:
		  *((DWORD*) buff) = flash.size()/512;
		  return RES_OK;

		case GET_SECTOR_SIZE:
		  *((WORD*) buff) = 512;
		  return RES_OK;

		case GET_BLOCK_SIZE:
		  *((DWORD*) buff) = 8;    // erase block size in units of sector size
		  return RES_OK;

		default:
		  return RES_PARERR;
	  }
	}

}

// USB Mass Storage object
Adafruit_USBD_MSC usb_msc;

// Set to true when PC write to flash
bool DiskChanged;

void ResetFS(void)
{
	fatfs.begin(&flash);
}

void USBdiskInit(void)
{
	DiskChanged = false;

	flash.begin();

	// Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
	usb_msc.setID("Adafruit", "External Flash", "1.0");

	// Set callback
	usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

	// Set disk size, block size should be 512 regardless of spi flash page size
	usb_msc.setCapacity(flash.size()/512, 512);

	// MSC is ready for read/write
	usb_msc.setUnitReady(true);

	usb_msc.begin();

	delay(200);

	// Init file system on the flash
	fatfs.begin(&flash);
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and 
// return number of copied bytes (must be multiple of block size) 
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  digitalWrite(LED_BUILTIN, HIGH);

  // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
  // sync with flash
  flash.syncBlocks();

  // clear file system's cache to force refresh
  fatfs.cacheClear();

  DiskChanged = true;

  digitalWrite(LED_BUILTIN, LOW);
}

// Function to detact the USB drive
// from the PC to allow us to update it
void DetachDrive(void)
{
	USBDevice.detach();
	delay(200);
}

// Function to attach the USB drive
// to the PC to aforce it to re-read it
void AttachDrive(void)
{
	USBDevice.attach();
	while (!Serial.dtr())
		delay(200);   // wait for usb serial to reopen
	delay(500);
}

// Force PC to refresh the drive image
// By disconnect/reconnect of USB
void RefreshDrive(void)
{
	USBDevice.detach();
	delay(100);
	USBDevice.attach();
	while (!Serial.dtr())
		delay(200);   // wait for usb serial to reopen
	delay(500);
}

void FormatDisk(void)
{
	// file system object from SdFat
	FatFileSystem fatfs;
	// Elm Cham's fatfs objects
	FATFS elmchamFatfs;
	uint8_t workbuf[4096]; // Working buffer for f_fdisk function.
	FRESULT r;
	uint8_t inch;

	// Make sure we REALLY want to do this
	FlushSerial();
	Serial.println("Formatting internal drive will erase all data currently on it!!!");
	Serial.println();
	Serial.println(" Are you sure (y/n)?");
	while(!Serial.available())
	;
	inch = Serial.read();
	if (inch != 'y' && inch != 'Y')
		return;
	FlushSerial();
	Serial.println();
	Serial.println(" Are you REALLY sure (y/n)?");
	while(!Serial.available())
	;
	inch = Serial.read();
	if (inch != 'y' && inch != 'Y')
		return;

	STATUS_LED_BUSY;
	Serial.println();
	Serial.println("Formatting internal drive");

	// Call fatfs begin and passed flash object to initialize file system
	// Make filesystem.
	Serial.println("Create file system");
	r = f_mkfs("", FM_FAT | FM_SFD, 0, workbuf, sizeof(workbuf));
	if (r != FR_OK)
	{
		Serial.print("Error, f_mkfs failed with error code: ");
		Serial.println(r, DEC);
		STATUS_LED_READY;
		return;
	}

	// mount to set disk label
	Serial.println("Mounting drive");
	r = f_mount(&elmchamFatfs, "0:", 1);
	if (r != FR_OK)
	{
		Serial.print("Error, f_mount failed with error code: ");
		Serial.println(r, DEC);
		STATUS_LED_READY;
		return;
	}

	// Setting label
	Serial.println("Setting disk label to: " DISK_LABEL);
	r = f_setlabel(DISK_LABEL);
	if (r != FR_OK)
	{
		Serial.print("Error, f_setlabel failed with error code: ");
		Serial.println(r, DEC);
		STATUS_LED_READY;
		return;
	}

	// unmount
	f_unmount("0:");

	// sync to make sure all data is written to flash
	flash.syncBlocks();

	Serial.println("Formatted flash, checking filesystem...");

	// Check new filesystem
	if (!fatfs.begin(&flash))
	{
		Serial.println("Error, failed to mount newly formatted filesystem!");
		STATUS_LED_READY;
		return;
	}

	RefreshDrive();
	Serial.println("Format done.");
	STATUS_LED_READY;
}
