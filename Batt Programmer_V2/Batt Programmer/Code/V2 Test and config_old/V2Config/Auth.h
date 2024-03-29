//-----------------------------------------------------------------------------
// FILENAME:  BattAuth.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Battery authentication related definitions are here
//
// %IF Zebra_Internal
//
// NOTES:
//
// AUTHOR:   		Wasath Mudalige
// CREATION DATE: 	04/01/2016
// DERIVED FROM: 	New file
//
// EDIT HISTORY:
//
// JEC
// renamed to Auth.h and Removed cradle specific stuff.
// Added chip type values
//
//
// %End
//-----------------------------------------------------------------------------

#ifndef AUTH_H_
#define AUTH_H_

enum AuthChipType {
	AUTH_NO_TYPE = 0,
	AUTH_508A,
	AUTH_608A,
};

void AUTH_Dump(void);
AuthChipType GetChipType(void);
bool AUTH_GetLockBytes(uint8_t *Value, uint8_t *Config);
uint16_t AUTH_ReadSerial(int which);
bool AUTH_WriteSerial(int which, uint16_t SN);
uint16_t AUTH_ReadHWrev(void);
bool AUTH_WriteHWrev(uint16_t HW);
bool AUTH_LockSlot(uint8_t SlotNum);

// Auth defines
#define MAX_AUTH_CMD_SIZE	135

#define MAX_RESP_SIZE		64
#define KEY_LEN				64
#define SIG_LEN				64
#define ATCA_SN_SIZE		9					//!< number of bytes in the device serial number
#define DEVICE_PRIV			0					// Decvice private key slot
#define RANDOM_NUM_SIZE		((uint8_t)0x20)     //!< Number of bytes in the data packet of a random command

#define ROOT_PUB_KEY "\xd6\xb7\x42\xb0\x29\xae\x01\x77\x5a\x20\x23\x5a\xd4\x63\xf2\x02\xa6\xc9\xb1\xef\x03\xf4\xfd\xfb\x6f\x17\xfd\xa9\x30\x5a\xb7\x77\xea\x83\x1c\x27\x9f\x3d\x03\x71\x91\x07\x8a\x1b\x98\x33\x04\xb2\x1f\x10\x61\xbf\x1e\xb3\x37\xdf\x3f\xea\x04\x21\xaa\x8a\x70\x55"

#define AUTH_SN0			0x01
#define AUTH_SN1			0x23
#define AUTH_SN8			0xEE

#define GENKEY_MODE_PUBLIC  0x00         //!< GenKey mode: public key calculation
#define ATCA_SHA_DIGEST_SIZE        (32)
#define ATCA_BLOCK_SIZE             (32)                                //!< size of a block
#define ATCA_COUNT_SIZE             ((uint8_t)1)                        //!< Number of bytes in the command packet Count
#define ATCA_CRC_SIZE               ((uint8_t)2)                        //!< Number of bytes in the command packet CRC
#define ATCA_PACKET_OVERHEAD        (ATCA_COUNT_SIZE + ATCA_CRC_SIZE)   //!< Number of bytes in the command packet

#define AUTH_WAKEUP_DELAY	1	// 1 mSec
#define AUTH_SLEEP_DELAY	1	// 1 mSec
#define AUTH_SLEEP_REG		0x01
#define AUTH_CMD_REG		0x03

//! minimum number of bytes in command (from count byte to second CRC byte)
#define ATCA_CMD_SIZE_MIN       ((uint8_t)7)
//! maximum size of command packet (Verify)
#define ATCA_CMD_SIZE_MAX       ((uint8_t)4 * 36 + 7)
//! status byte for success
#define CMD_STATUS_SUCCESS      ((uint8_t)0x00)
//! status byte after wake-up
#define CMD_STATUS_WAKEUP       ((uint8_t)0x11)
//! command parse error
#define CMD_STATUS_BYTE_PARSE   ((uint8_t)0x03)
//! command ECC error
#define CMD_STATUS_BYTE_ECC     ((uint8_t)0x05)
//! command execution error
#define CMD_STATUS_BYTE_EXEC    ((uint8_t)0x0F)
//! communication error
#define CMD_STATUS_BYTE_COMM    ((uint8_t)0xFF)

// Opcodes for ATATECC Commands
#define ATCA_CHECKMAC                   ((uint8_t)0x28)         //!< CheckMac command op-code
#define ATCA_DERIVE_KEY                 ((uint8_t)0x1C)         //!< DeriveKey command op-code
#define ATCA_INFO                       ((uint8_t)0x30)         //!< Info command op-code
#define ATCA_GENDIG                     ((uint8_t)0x15)         //!< GenDig command op-code
#define ATCA_GENKEY                     ((uint8_t)0x40)         //!< GenKey command op-code
#define ATCA_HMAC                       ((uint8_t)0x11)         //!< HMAC command op-code
#define ATCA_LOCK                       ((uint8_t)0x17)         //!< Lock command op-code
#define ATCA_MAC                        ((uint8_t)0x08)         //!< MAC command op-code
#define ATCA_NONCE                      ((uint8_t)0x16)         //!< Nonce command op-code
#define ATCA_PAUSE                      ((uint8_t)0x01)         //!< Pause command op-code
#define ATCA_PRIVWRITE                  ((uint8_t)0x46)         //!< PrivWrite command op-code
#define ATCA_RANDOM                     ((uint8_t)0x1B)         //!< Random command op-code
#define ATCA_READ                       ((uint8_t)0x02)         //!< Read command op-code
#define ATCA_SIGN                       ((uint8_t)0x41)         //!< Sign command op-code
#define ATCA_UPDATE_EXTRA               ((uint8_t)0x20)         //!< UpdateExtra command op-code
#define ATCA_VERIFY                     ((uint8_t)0x45)         //!< GenKey command op-code
#define ATCA_WRITE                      ((uint8_t)0x12)         //!< Write command op-code
#define ATCA_ECDH                       ((uint8_t)0x43)         //!< ECDH command op-code
#define ATCA_COUNTER                    ((uint8_t)0x24)         //!< Counter command op-code
#define ATCA_SHA                        ((uint8_t)0x47)         //!< SHA command op-code

/** \name Definitions for Zone and Address Parameters
   @{ */
#define ATCA_ZONE_CONFIG                ((uint8_t)0x00)         //!< Configuration zone
#define ATCA_ZONE_OTP                   ((uint8_t)0x01)         //!< OTP (One Time Programming) zone
#define ATCA_ZONE_DATA                  ((uint8_t)0x02)         //!< Data zone
#define ATCA_ZONE_MASK                  ((uint8_t)0x03)         //!< Zone mask
#define ATCA_ZONE_ENCRYPTED             ((uint8_t)0x40)         //!< Zone bit 6 set: Write is encrypted with an unlocked data zone.
#define ATCA_ZONE_READWRITE_32          ((uint8_t)0x80)         //!< Zone bit 7 set: Access 32 bytes, otherwise 4 bytes.
#define ATCA_ADDRESS_MASK_CONFIG        (0x001F)                //!< Address bits 5 to 7 are 0 for Configuration zone.
#define ATCA_ADDRESS_MASK_OTP           (0x000F)                //!< Address bits 4 to 7 are 0 for OTP zone.
#define ATCA_ADDRESS_MASK               (0x007F)                //!< Address bit 7 to 15 are always 0.
/** @} */

#define SHA_MODE_SHA256_START			((uint8_t)0x00)		//!< Initialization, does not accept a message
#define SHA_MODE_SHA256_UPDATE          ((uint8_t)0x01)		//!< Add 64 bytes in the meesage to the SHA context
#define SHA_MODE_SHA256_END             ((uint8_t)0x02)		//!< Complete the calculation and return the digest

#define NONCE_MODE_SEED_UPDATE          ((uint8_t)0x00)     //!< Nonce mode: update seed
#define NONCE_MODE_NO_SEED_UPDATE       ((uint8_t)0x01)     //!< Nonce mode: do not update seed
#define NONCE_MODE_INVALID              ((uint8_t)0x02)     //!< Nonce mode 2 is invalid.
#define NONCE_MODE_PASSTHROUGH          ((uint8_t)0x03)     //!< Nonce mode: pass-through

#define VERIFY_MODE_EXTERNAL			((uint8_t)0x02)     //!< Verify mode: external
#define VERIFY_KEY_P256					((uint8_t)0x04)		//!< Verify key type: P256

#define RANDOM_SEED_UPDATE              ((uint8_t)0x00)     //!< Random mode for automatic seed update
#define SIGN_MODE_EXTERNAL				((uint8_t)0x80)     //!< Sign mode bit 7: external

#endif /* AUTH_H_ */