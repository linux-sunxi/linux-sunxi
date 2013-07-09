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


#ifndef __CRYPT_SHA2_H__
#define __CRYPT_SHA2_H__


#define EXIT_SUCCESS    0
#define EXIT_FAILURE    1

typedef unsigned char uint_8t;
typedef unsigned short uint_16t;
typedef unsigned int uint_32t;
typedef unsigned long long uint_64t;

#define void_ret  void
#define int_ret   int


/* define the hash functions that you need  */
#define SHA_2			/* for dynamic hash length  */
#define SHA_224
#define SHA_256


#if defined(__cplusplus)
extern "C" {
#endif

/* Note that the following function prototypes are the same */
/* for both the bit and byte oriented implementations.  But */
/* the length fields are in bytes or bits as is appropriate */
/* for the version used.  Bit sequences are arrays of bytes */
/* in which bit sequence indexes increase from the most to  */
/* the least significant end of each byte                   */

#define SHA1_BLOCK_SIZE    64	/* 512 bits = 64 bytes */
#define SHA1_DIGEST_SIZE   20	/* 160 bits = 20 bytes */
#define SHA224_DIGEST_SIZE  28
#define SHA224_BLOCK_SIZE   64
#define SHA256_DIGEST_SIZE  32
#define SHA256_BLOCK_SIZE   64

/* type to hold the SHA256 (and SHA224) context */

typedef struct
{   uint_32t count[2];
    uint_32t hash[8];
    uint_32t wbuf[16];
} sha256_ctx;

typedef sha256_ctx  sha224_ctx;

void_ret sha256_compile(sha256_ctx ctx[1]);

void_ret sha224_begin(sha224_ctx ctx[1]);
#define sha224_hash sha256_hash
void_ret sha224_end(unsigned char hval[], sha224_ctx ctx[1]);
void_ret sha224(unsigned char hval[], const unsigned char data[], unsigned int len);

void_ret sha256_begin(sha256_ctx ctx[1]);
void_ret sha256_hash(const unsigned char data[], unsigned int len, sha256_ctx ctx[1]);
void_ret sha256_end(unsigned char hval[], sha256_ctx ctx[1]);
void_ret sha256(unsigned char hval[], const unsigned char data[], unsigned int len);
 

#ifndef SHA_64BIT

typedef struct
{   union
    { sha256_ctx  ctx256[1];
    } uu[1];
    uint_32t    sha2_len;
} sha2_ctx;

#define SHA2_MAX_DIGEST_SIZE    SHA256_DIGEST_SIZE

#else

#define SHA384_DIGEST_SIZE  48
#define SHA384_BLOCK_SIZE  128
#define SHA512_DIGEST_SIZE  64
#define SHA512_BLOCK_SIZE  128
#define SHA2_MAX_DIGEST_SIZE    SHA512_DIGEST_SIZE

/* type to hold the SHA384 (and SHA512) context */

typedef struct
{   uint_64t count[2];
    uint_64t hash[8];
    uint_64t wbuf[16];
} sha512_ctx;

typedef sha512_ctx  sha384_ctx;

typedef struct
{   union
    { sha256_ctx  ctx256[1];
      sha512_ctx  ctx512[1];
    } uu[1];
    uint_32t    sha2_len;
} sha2_ctx;

void_ret sha512_compile(sha512_ctx ctx[1]);

void_ret sha384_begin(sha384_ctx ctx[1]);
#define sha384_hash sha512_hash
void_ret sha384_end(unsigned char hval[], sha384_ctx ctx[1]);
void_ret sha384(unsigned char hval[], const unsigned char data[], unsigned int len);

void_ret sha512_begin(sha512_ctx ctx[1]);
void_ret sha512_hash(const unsigned char data[], unsigned int len, sha512_ctx ctx[1]);
void_ret sha512_end(unsigned char hval[], sha512_ctx ctx[1]);
void_ret sha512(unsigned char hval[], const unsigned char data[], unsigned int len);

int_ret  sha2_begin(unsigned int size, sha2_ctx ctx[1]);
void_ret sha2_hash(const unsigned char data[], unsigned int len, sha2_ctx ctx[1]);
void_ret sha2_end(unsigned char hval[], sha2_ctx ctx[1]);
int_ret  sha2(unsigned char hval[], unsigned int size, const unsigned char data[], unsigned int len);
#endif

#define RT_SHA256(Message, MessageL, DMessage) \
    sha256((DMessage), (Message), (MessageL));

#if defined(__cplusplus)
}
#endif

#endif /* __CRYPT_SHA2_H__ */
