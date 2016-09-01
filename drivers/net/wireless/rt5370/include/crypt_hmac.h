/*
 *************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                       *
 *************************************************************************/


#ifndef __CRYPT_HMAC_H__
#define __CRYPT_HMAC_H__

#include "rt_config.h"


#if defined(__cplusplus)
extern "C" {
#endif

#define USE_SHA256

#if !defined(USE_SHA1) && !defined(USE_SHA256)
#error define USE_SHA1 or USE_SHA256 to set the HMAC hash algorithm
#endif

#ifdef USE_SHA1
#define HASH_INPUT_SIZE     SHA1_BLOCK_SIZE
#define HASH_OUTPUT_SIZE    SHA1_DIGEST_SIZE
#define sha_ctx             sha1_ctx
#define sha_begin           sha1_begin
#define sha_hash            sha1_hash
#define sha_end             sha1_end
#endif

#ifdef USE_SHA256
#define HASH_INPUT_SIZE     SHA256_BLOCK_SIZE
#define HASH_OUTPUT_SIZE    SHA256_DIGEST_SIZE
#define sha_ctx             sha256_ctx
#define sha_begin           sha256_begin
#define sha_hash            sha256_hash
#define sha_end             sha256_end
#endif

#define HMAC_OK                0
#define HMAC_BAD_MODE         -1
#define HMAC_IN_DATA  0xffffffff

typedef struct
{   unsigned char   key[HASH_INPUT_SIZE];
    sha_ctx         ctx[1];
    unsigned int   klen;
} hmac_ctx;

void hmac_sha_begin(hmac_ctx cx[1]);
int  hmac_sha_key(const unsigned char key[], unsigned int key_len, hmac_ctx cx[1]);
void hmac_sha_data(const unsigned char data[], unsigned int data_len, hmac_ctx cx[1]);
void hmac_sha_end(unsigned char mac[], unsigned int mac_len, hmac_ctx cx[1]);
void hmac_sha(const unsigned char key[], unsigned int key_len,
          const unsigned char data[], unsigned int data_len,
          unsigned char mac[], unsigned int mac_len);

#define RT_HMAC_SHA256(Key, KeyL, Data, DataL, Mac, MacL) \
    hmac_sha((Key), (KeyL), (Data), (DataL), (Mac), (MacL))

#if defined(__cplusplus)
}
#endif

#endif /* __CRYPT_HMAC_H__ */
