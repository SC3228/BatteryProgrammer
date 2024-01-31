// AES Encryption header file

// Adapted from the following:
/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.

 LICENSE TERMS

 The redistribution and use of this software (with or without changes)
 is allowed without the payment of fees or royalties provided that:

  1. source code distributions include the above copyright notice, this
     list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
     of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
     built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 */

#ifndef AES_H
#define AES_H

#define AES_ENC_PREKEYED  /* AES encryption with a precomputed key schedule  */
#define AES_DEC_PREKEYED  /* AES decryption with a precomputed key schedule  */

#define N_ROW                   4
#define N_COL                   4
#define N_BLOCK   (N_ROW * N_COL)
#define N_MAX_ROUNDS           14

typedef unsigned char uint_8t;

typedef uint_8t return_type;

/*  Warning: The key length for 256 bit keys overflows a byte
    (see comment below)
*/

typedef uint_8t length_type;

typedef struct
{   uint_8t ksch[(N_MAX_ROUNDS + 1) * N_BLOCK];
    uint_8t rnd;
} aes_context;

// Wrapper function
extern void  MovI2Cdata(uint8_t *buffer1, uint8_t *buffer2, const uint8_t* buffer3, int dir);

#endif
