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


#ifndef __CRYPT_MD5_H__
#define __CRYPT_MD5_H__


#ifndef	uint8
#define	uint8  unsigned	char
#endif

#ifndef	uint32
#define	uint32 unsigned	long int
#endif

#define MD5_MAC_LEN 16

typedef struct _MD5_CTX {
    ULONG   Buf[4];             /* buffers of four states */
	UCHAR   Input[64];          /* input message */
	ULONG   LenInBitCount[2];   /* length counter for input message, 0 up to 64 bits */
}   MD5_CTX;

VOID MD5Init(MD5_CTX *pCtx);
VOID MD5Update(MD5_CTX *pCtx, UCHAR *pData, ULONG LenInBytes);
VOID MD5Final(UCHAR Digest[16], MD5_CTX *pCtx);
VOID MD5Transform(ULONG Buf[4], ULONG Mes[16]);

void md5_mac(u8 *key, size_t key_len, u8 *data, size_t data_len, u8 *mac);
void hmac_md5(u8 *key, size_t key_len, u8 *data, size_t data_len, u8 *mac);

/* */
/* SHA context */
/* */
typedef	struct _SHA_CTX
{
	ULONG   Buf[5];             /* buffers of five states */
	UCHAR   Input[80];          /* input message */
	ULONG   LenInBitCount[2];   /* length counter for input message, 0 up to 64 bits */
	
}	SHA_CTX;

VOID SHAInit(SHA_CTX *pCtx);
UCHAR SHAUpdate(SHA_CTX *pCtx, UCHAR *pData, ULONG LenInBytes);
VOID SHAFinal(SHA_CTX *pCtx, UCHAR Digest[20]);
VOID SHATransform(ULONG Buf[5], ULONG Mes[20]);

#define SHA_DIGEST_LEN 20

VOID HMAC_SHA1(
	IN UCHAR * text,
	IN UINT text_len,
	IN UCHAR * key,
	IN UINT key_len,
	IN UCHAR * digest);

#define RT_HMAC_MD5(Key, KeyL, Meg, MegL, MAC, MACL) \
    hmac_md5((Key), (KeyL), (Meg), (MegL), (MAC))

#define RT_HMAC_SHA1(Key, KeyL, Meg, MegL, MAC, MACL) \
    HMAC_SHA1((Meg), (MegL), (Key), (KeyL), (MAC))


#endif /* __CRYPT_MD5_H__ */
