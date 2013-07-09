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


#include "rt_config.h"


/**
 * md5_mac:
 * @key: pointer to	the	key	used for MAC generation
 * @key_len: length	of the key in bytes
 * @data: pointer to the data area for which the MAC is	generated
 * @data_len: length of	the	data in	bytes
 * @mac: pointer to	the	buffer holding space for the MAC; the buffer should
 * have	space for 128-bit (16 bytes) MD5 hash value
 *
 * md5_mac() determines	the	message	authentication code	by using secure	hash
 * MD5(key | data |	key).
 */
void md5_mac(u8 *key, size_t key_len, u8 *data, size_t data_len, u8 *mac)
{
	MD5_CTX	context;

	MD5Init(&context);
	MD5Update(&context,	key, key_len);
	MD5Update(&context,	data, data_len);
	MD5Update(&context,	key, key_len);
	MD5Final(mac, &context);
}

/**
 * hmac_md5:
 * @key: pointer to	the	key	used for MAC generation
 * @key_len: length	of the key in bytes
 * @data: pointer to the data area for which the MAC is	generated
 * @data_len: length of	the	data in	bytes
 * @mac: pointer to	the	buffer holding space for the MAC; the buffer should
 * have	space for 128-bit (16 bytes) MD5 hash value
 *
 * hmac_md5() determines the message authentication	code using HMAC-MD5.
 * This	implementation is based	on the sample code presented in	RFC	2104.
 */
void hmac_md5(u8 *key, size_t key_len, u8 *data, size_t data_len, u8 *mac)
{
/*	MD5_CTX	context;*/
	MD5_CTX	*pcontext = NULL;
    u8 k_ipad[65]; /* inner padding - key XORd with ipad */
    u8 k_opad[65]; /* outer padding - key XORd with opad */
    u8 tk[16];
	int	i;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pcontext, sizeof(MD5_CTX));
	if (pcontext == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	/*assert(key != NULL && data != NULL && mac != NULL);*/

	/* if key is longer	than 64	bytes reset	it to key =	MD5(key) */
	if (key_len	> 64) {
		MD5Init(pcontext);
		MD5Update(pcontext, key, key_len);
		MD5Final(tk, pcontext);
		/*key=(PUCHAR)ttcontext.buf;*/
		key	= tk;
		key_len	= 16;
	}

	/* the HMAC_MD5	transform looks	like:
	 *
	 * MD5(K XOR opad, MD5(K XOR ipad, text))
	 *
	 * where K is an n byte	key
	 * ipad	is the byte	0x36 repeated 64 times
	 * opad	is the byte	0x5c repeated 64 times
	 * and text	is the data	being protected	*/

	/* start out by	storing	key	in pads	*/
	NdisZeroMemory(k_ipad, sizeof(k_ipad));
	NdisZeroMemory(k_opad,	sizeof(k_opad));
	/*assert(key_len < sizeof(k_ipad));*/
	NdisMoveMemory(k_ipad, key,	key_len);
	NdisMoveMemory(k_opad, key,	key_len);

	/* XOR key with	ipad and opad values */
	for	(i = 0;	i <	64;	i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	/* perform inner MD5 */
	MD5Init(pcontext);					 /*	init context for 1st pass */
	MD5Update(pcontext,	k_ipad,	64);	 /*	start with inner pad */
	MD5Update(pcontext,	data, data_len); /*	then text of datagram */
	MD5Final(mac, pcontext);			 /*	finish up 1st pass */

	/* perform outer MD5 */
	MD5Init(pcontext);					 /*	init context for 2nd pass */
	MD5Update(pcontext,	k_opad,	64);	 /*	start with outer pad */
	MD5Update(pcontext,	mac, 16);		 /*	then results of	1st	hash */
	MD5Final(mac, pcontext);			 /*	finish up 2nd pass */

	if (pcontext != NULL)
		os_free_mem(NULL, pcontext);
}

#ifndef RT_BIG_ENDIAN
#define byteReverse(buf, len)   /* Nothing */
#else
void byteReverse(unsigned char *buf, unsigned longs);
void byteReverse(unsigned char *buf, unsigned longs)
{
    do {
        *(ULONG *)buf = SWAP32(*(ULONG *)buf);
        buf += 4;
    } while (--longs);
}
#endif


/* ==========================  MD5 implementation =========================== */ 
/* four base functions for MD5 */
#define MD5_F1(x, y, z) (((x) & (y)) | ((~x) & (z))) 
#define MD5_F2(x, y, z) (((x) & (z)) | ((y) & (~z))) 
#define MD5_F3(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_F4(x, y, z) ((y) ^ ((x) | (~z)))
#define CYCLIC_LEFT_SHIFT(w, s) (((w) << (s)) | ((w) >> (32-(s))))

#define	MD5Step(f, w, x, y,	z, data, t, s)	\
	( w	+= f(x,	y, z) +	data + t,  w = (CYCLIC_LEFT_SHIFT(w, s)) & 0xffffffff, w +=	x )


/*
 *  Function Description:
 *      Initiate MD5 Context satisfied in RFC 1321
 *
 *  Arguments:
 *      pCtx        Pointer	to MD5 context
 *
 *  Return Value:
 *      None	    
 */
VOID MD5Init(MD5_CTX *pCtx)
{
    pCtx->Buf[0]=0x67452301;
    pCtx->Buf[1]=0xefcdab89;
    pCtx->Buf[2]=0x98badcfe;
    pCtx->Buf[3]=0x10325476;

    pCtx->LenInBitCount[0]=0;
    pCtx->LenInBitCount[1]=0;
}


/*
 *  Function Description:
 *      Update MD5 Context, allow of an arrary of octets as the next portion 
 *      of the message
 *      
 *  Arguments:
 *      pCtx		Pointer	to MD5 context
 * 	    pData       Pointer to input data
 *      LenInBytes  The length of input data (unit: byte)
 *
 *  Return Value:
 *      None
 *
 *  Note:
 *      Called after MD5Init or MD5Update(itself)   
 */
VOID MD5Update(MD5_CTX *pCtx, UCHAR *pData, ULONG LenInBytes)
{
    
    ULONG TfTimes;
    ULONG temp;
	unsigned int i;
    
    temp = pCtx->LenInBitCount[0];

    pCtx->LenInBitCount[0] = (ULONG) (pCtx->LenInBitCount[0] + (LenInBytes << 3));
 
    if (pCtx->LenInBitCount[0] < temp)
        pCtx->LenInBitCount[1]++;   /*carry in*/

    pCtx->LenInBitCount[1] += LenInBytes >> 29;

    /* mod 64 bytes*/
    temp = (temp >> 3) & 0x3f;  
    
    /* process lacks of 64-byte data */
    if (temp) 
    {
        UCHAR *pAds = (UCHAR *) pCtx->Input + temp;
        
        if ((temp+LenInBytes) < 64)
        {
            NdisMoveMemory(pAds, (UCHAR *)pData, LenInBytes);   
            return;
        }
        
        NdisMoveMemory(pAds, (UCHAR *)pData, 64-temp);               
        byteReverse(pCtx->Input, 16);
        MD5Transform(pCtx->Buf, (ULONG *)pCtx->Input);

        pData += 64-temp;
        LenInBytes -= 64-temp; 
    } /* end of if (temp)*/
    
     
    TfTimes = (LenInBytes >> 6);

    for (i=TfTimes; i>0; i--)
    {
        NdisMoveMemory(pCtx->Input, (UCHAR *)pData, 64);
        byteReverse(pCtx->Input, 16);
        MD5Transform(pCtx->Buf, (ULONG *)pCtx->Input);
        pData += 64;
        LenInBytes -= 64;
    } /* end of for*/

    /* buffering lacks of 64-byte data*/
    if(LenInBytes)
        NdisMoveMemory(pCtx->Input, (UCHAR *)pData, LenInBytes);   
   
}


/*
 *  Function Description:
 *      Append padding bits and length of original message in the tail 
 *      The message digest has to be completed in the end  
 *  
 *  Arguments:
 *      Digest		Output of Digest-Message for MD5
 *  	pCtx        Pointer	to MD5 context
 * 	
 *  Return Value:
 *      None
 *  
 *  Note:
 *      Called after MD5Update  
 */
VOID MD5Final(UCHAR Digest[16], MD5_CTX *pCtx)
{
    UCHAR Remainder;
    UCHAR PadLenInBytes;
    UCHAR *pAppend=0;
    unsigned int i;
    
    Remainder = (UCHAR)((pCtx->LenInBitCount[0] >> 3) & 0x3f);

    PadLenInBytes = (Remainder < 56) ? (56-Remainder) : (120-Remainder);
    
    pAppend = (UCHAR *)pCtx->Input + Remainder;

    /* padding bits without crossing block(64-byte based) boundary*/
    if (Remainder < 56)
    {
        *pAppend = 0x80;
        PadLenInBytes --;
        
        NdisZeroMemory((UCHAR *)pCtx->Input + Remainder+1, PadLenInBytes); 
		
		/* add data-length field, from low to high*/
       	for (i=0; i<4; i++)
        {
        	pCtx->Input[56+i] = (UCHAR)((pCtx->LenInBitCount[0] >> (i << 3)) & 0xff);
        	pCtx->Input[60+i] = (UCHAR)((pCtx->LenInBitCount[1] >> (i << 3)) & 0xff);
      	}
      	
        byteReverse(pCtx->Input, 16);
        MD5Transform(pCtx->Buf, (ULONG *)pCtx->Input);
    } /* end of if*/
    
    /* padding bits with crossing block(64-byte based) boundary*/
    else
    {
        /* the first block ===*/
        *pAppend = 0x80;
        PadLenInBytes --;
       
        NdisZeroMemory((UCHAR *)pCtx->Input + Remainder+1, (64-Remainder-1)); 
        PadLenInBytes -= (64 - Remainder - 1);
        
        byteReverse(pCtx->Input, 16);
        MD5Transform(pCtx->Buf, (ULONG *)pCtx->Input);
        

        /* the second block ===*/
        NdisZeroMemory((UCHAR *)pCtx->Input, PadLenInBytes); 

        /* add data-length field*/
        for (i=0; i<4; i++)
        {
        	pCtx->Input[56+i] = (UCHAR)((pCtx->LenInBitCount[0] >> (i << 3)) & 0xff);
        	pCtx->Input[60+i] = (UCHAR)((pCtx->LenInBitCount[1] >> (i << 3)) & 0xff);
      	}

        byteReverse(pCtx->Input, 16);
        MD5Transform(pCtx->Buf, (ULONG *)pCtx->Input);
    } /* end of else*/


    NdisMoveMemory((UCHAR *)Digest, (ULONG *)pCtx->Buf, 16); /* output*/
    byteReverse((UCHAR *)Digest, 4);
    NdisZeroMemory(pCtx, sizeof(pCtx)); /* memory free */
}


/*
 *  Function Description:
 *      The central algorithm of MD5, consists of four rounds and sixteen 
 *  	steps per round
 * 
 *  Arguments:
 *      Buf     Buffers of four states (output: 16 bytes)		
 * 	    Mes     Input data (input: 64 bytes) 
 *  
 *  Return Value:
 *      None
 *  	
 *  Note:
 *      Called by MD5Update or MD5Final
 */
VOID MD5Transform(ULONG Buf[4], ULONG Mes[16])
{  
    ULONG Reg[4], Temp; 
	unsigned int i;
    
    static UCHAR LShiftVal[16] = 
    { 	
        7, 12, 17, 22, 	
		5, 9 , 14, 20, 
		4, 11, 16, 23, 
 		6, 10, 15, 21, 
 	};

	
	/* [equal to 4294967296*abs(sin(index))]*/
    static ULONG MD5Table[64] = 
	{ 
		0xd76aa478,	0xe8c7b756,	0x242070db,	0xc1bdceee,	
		0xf57c0faf,	0x4787c62a,	0xa8304613, 0xfd469501,	
		0x698098d8,	0x8b44f7af,	0xffff5bb1,	0x895cd7be,
    	0x6b901122,	0xfd987193,	0xa679438e,	0x49b40821,
    	
    	0xf61e2562,	0xc040b340,	0x265e5a51,	0xe9b6c7aa,
    	0xd62f105d,	0x02441453,	0xd8a1e681,	0xe7d3fbc8,
    	0x21e1cde6,	0xc33707d6,	0xf4d50d87,	0x455a14ed,
    	0xa9e3e905,	0xfcefa3f8,	0x676f02d9,	0x8d2a4c8a,
    	           
    	0xfffa3942,	0x8771f681,	0x6d9d6122,	0xfde5380c,
    	0xa4beea44,	0x4bdecfa9,	0xf6bb4b60,	0xbebfbc70,
    	0x289b7ec6,	0xeaa127fa,	0xd4ef3085,	0x04881d05,
    	0xd9d4d039,	0xe6db99e5,	0x1fa27cf8,	0xc4ac5665,
    	           
    	0xf4292244,	0x432aff97,	0xab9423a7,	0xfc93a039,
   		0x655b59c3,	0x8f0ccc92,	0xffeff47d,	0x85845dd1,
    	0x6fa87e4f,	0xfe2ce6e0,	0xa3014314,	0x4e0811a1,
    	0xf7537e82,	0xbd3af235,	0x2ad7d2bb,	0xeb86d391
	};
 
				
    for (i=0; i<4; i++)
        Reg[i]=Buf[i];
			
				
    /* 64 steps in MD5 algorithm*/
    for (i=0; i<16; i++)                    
    {
        MD5Step(MD5_F1, Reg[0], Reg[1], Reg[2], Reg[3], Mes[i],               
                MD5Table[i], LShiftVal[i & 0x3]);

        /* one-word right shift*/
        Temp   = Reg[3]; 
        Reg[3] = Reg[2];
        Reg[2] = Reg[1];
        Reg[1] = Reg[0];
        Reg[0] = Temp;            
    }
    for (i=16; i<32; i++)                    
    {
        MD5Step(MD5_F2, Reg[0], Reg[1], Reg[2], Reg[3], Mes[(5*(i & 0xf)+1) & 0xf], 
                MD5Table[i], LShiftVal[(0x1 << 2)+(i & 0x3)]);    

        /* one-word right shift*/
        Temp   = Reg[3]; 
        Reg[3] = Reg[2];
        Reg[2] = Reg[1];
        Reg[1] = Reg[0];
        Reg[0] = Temp;           
    }
    for (i=32; i<48; i++)                    
    {
        MD5Step(MD5_F3, Reg[0], Reg[1], Reg[2], Reg[3], Mes[(3*(i & 0xf)+5) & 0xf], 
                MD5Table[i], LShiftVal[(0x1 << 3)+(i & 0x3)]);        

        /* one-word right shift*/
        Temp   = Reg[3]; 
        Reg[3] = Reg[2];
        Reg[2] = Reg[1];
        Reg[1] = Reg[0];
        Reg[0] = Temp;          
    }
    for (i=48; i<64; i++)                    
    {
        MD5Step(MD5_F4, Reg[0], Reg[1], Reg[2], Reg[3], Mes[(7*(i & 0xf)) & 0xf], 
                MD5Table[i], LShiftVal[(0x3 << 2)+(i & 0x3)]);   

        /* one-word right shift*/
        Temp   = Reg[3]; 
        Reg[3] = Reg[2];
        Reg[2] = Reg[1];
        Reg[1] = Reg[0];
        Reg[0] = Temp;           
    }
    
      
    /* (temporary)output*/
    for (i=0; i<4; i++)
        Buf[i] += Reg[i];

}



/* =========================  SHA-1 implementation ========================== */
/* four base functions for SHA-1*/
#define SHA1_F1(b, c, d)    (((b) & (c)) | ((~b) & (d)))         
#define SHA1_F2(b, c, d)    ((b) ^ (c) ^ (d)) 
#define SHA1_F3(b, c, d)    (((b) & (c)) | ((b) & (d)) | ((c) & (d)))


#define SHA1Step(f, a, b, c, d, e, w, k)    \
    ( e	+= ( f(b, c, d) + w + k + CYCLIC_LEFT_SHIFT(a, 5)) & 0xffffffff, \
      b = CYCLIC_LEFT_SHIFT(b, 30) )

/*Initiate SHA-1 Context satisfied in RFC 3174  */
VOID SHAInit(SHA_CTX *pCtx)
{
    pCtx->Buf[0]=0x67452301;
    pCtx->Buf[1]=0xefcdab89;
    pCtx->Buf[2]=0x98badcfe;
    pCtx->Buf[3]=0x10325476;
    pCtx->Buf[4]=0xc3d2e1f0;
    
    pCtx->LenInBitCount[0]=0;
    pCtx->LenInBitCount[1]=0;
}

/*
 *  Function Description:
 *      Update SHA-1 Context, allow of an arrary of octets as the next
 *      portion of the message
 *      
 *  Arguments:
 *      pCtx		Pointer	to SHA-1 context
 * 	    pData       Pointer to input data
 *      LenInBytes  The length of input data (unit: byte)
 *
 *  Return Value:
 *      error       indicate more than pow(2,64) bits of data  
 *
 *  Note:
 *      Called after SHAInit or SHAUpdate(itself)   
 */
UCHAR SHAUpdate(SHA_CTX *pCtx, UCHAR *pData, ULONG LenInBytes)
{
    ULONG TfTimes;
    ULONG temp1,temp2;
	unsigned int i;
	UCHAR err=1;
    
    temp1 = pCtx->LenInBitCount[0];
    temp2 = pCtx->LenInBitCount[1];

    pCtx->LenInBitCount[0] = (ULONG) (pCtx->LenInBitCount[0] + (LenInBytes << 3));
    if (pCtx->LenInBitCount[0] < temp1)
        pCtx->LenInBitCount[1]++;   /*carry in*/


    pCtx->LenInBitCount[1] = (ULONG) (pCtx->LenInBitCount[1] +(LenInBytes >> 29));
    if (pCtx->LenInBitCount[1] < temp2)
        return (err);   /*check total length of original data*/
 

    /* mod 64 bytes*/
    temp1 = (temp1 >> 3) & 0x3f;  
    
    /* process lacks of 64-byte data */
    if (temp1) 
    {
        UCHAR *pAds = (UCHAR *) pCtx->Input + temp1;
        
        if ((temp1+LenInBytes) < 64)
        {
            NdisMoveMemory(pAds, (UCHAR *)pData, LenInBytes);   
            return (0);
        }
        
        NdisMoveMemory(pAds, (UCHAR *)pData, 64-temp1);              
        byteReverse((UCHAR *)pCtx->Input, 16);               
        
        NdisZeroMemory((UCHAR *)pCtx->Input + 64, 16);
        SHATransform(pCtx->Buf, (ULONG *)pCtx->Input);

        pData += 64-temp1;
        LenInBytes -= 64-temp1; 
    } /* end of if (temp1)*/
    
     
    TfTimes = (LenInBytes >> 6);

    for (i=TfTimes; i>0; i--)
    {
        NdisMoveMemory(pCtx->Input, (UCHAR *)pData, 64);
        byteReverse((UCHAR *)pCtx->Input, 16);
        
        NdisZeroMemory((UCHAR *)pCtx->Input + 64, 16);
        SHATransform(pCtx->Buf, (ULONG *)pCtx->Input);
        pData += 64;
        LenInBytes -= 64;
    } /* end of for*/

    /* buffering lacks of 64-byte data*/
    if(LenInBytes)
        NdisMoveMemory(pCtx->Input, (UCHAR *)pData, LenInBytes);

	return (0);

}

/* Append padding bits and length of original message in the tail */
/* The message digest has to be completed in the end */
VOID SHAFinal(SHA_CTX *pCtx, UCHAR Digest[20])
{
    UCHAR Remainder;
    UCHAR PadLenInBytes;
    UCHAR *pAppend=0;
    unsigned int i;

    Remainder = (UCHAR)((pCtx->LenInBitCount[0] >> 3) & 0x3f);

    pAppend = (UCHAR *)pCtx->Input + Remainder;

    PadLenInBytes = (Remainder < 56) ? (56-Remainder) : (120-Remainder);
    
    /* padding bits without crossing block(64-byte based) boundary*/
    if (Remainder < 56)
    {       
        *pAppend = 0x80;
        PadLenInBytes --;
        
        NdisZeroMemory((UCHAR *)pCtx->Input + Remainder+1, PadLenInBytes); 
		 
		/* add data-length field, from high to low*/
        for (i=0; i<4; i++)
        {
        	pCtx->Input[56+i] = (UCHAR)((pCtx->LenInBitCount[1] >> ((3-i) << 3)) & 0xff);
        	pCtx->Input[60+i] = (UCHAR)((pCtx->LenInBitCount[0] >> ((3-i) << 3)) & 0xff);
      	}
      	
        byteReverse((UCHAR *)pCtx->Input, 16);
        NdisZeroMemory((UCHAR *)pCtx->Input + 64, 14);
        SHATransform(pCtx->Buf, (ULONG *)pCtx->Input);
    } /* end of if*/
    
    /* padding bits with crossing block(64-byte based) boundary*/
    else
    {
        /* the first block ===*/
        *pAppend = 0x80;
        PadLenInBytes --;
        
        NdisZeroMemory((UCHAR *)pCtx->Input + Remainder+1, (64-Remainder-1)); 
        PadLenInBytes -= (64 - Remainder - 1);
        
        byteReverse((UCHAR *)pCtx->Input, 16);
        NdisZeroMemory((UCHAR *)pCtx->Input + 64, 16);
        SHATransform(pCtx->Buf, (ULONG *)pCtx->Input);


        /* the second block ===*/
        NdisZeroMemory((UCHAR *)pCtx->Input, PadLenInBytes); 
			
		/* add data-length field*/
		for (i=0; i<4; i++)
        {
        	pCtx->Input[56+i] = (UCHAR)((pCtx->LenInBitCount[1] >> ((3-i) << 3)) & 0xff);
        	pCtx->Input[60+i] = (UCHAR)((pCtx->LenInBitCount[0] >> ((3-i) << 3)) & 0xff);
      	}
      	
        byteReverse((UCHAR *)pCtx->Input, 16);
        NdisZeroMemory((UCHAR *)pCtx->Input + 64, 16); 
        SHATransform(pCtx->Buf, (ULONG *)pCtx->Input);
    } /* end of else*/
	
		
    /*Output, bytereverse*/
    for (i=0; i<20; i++)
    {
        Digest [i] = (UCHAR)(pCtx->Buf[i>>2] >> 8*(3-(i & 0x3)));
    }
    
    NdisZeroMemory(pCtx, sizeof(pCtx)); /* memory free */
}


/* The central algorithm of SHA-1, consists of four rounds and */
/* twenty steps per round*/
VOID SHATransform(ULONG Buf[5], ULONG Mes[20])
{
    ULONG Reg[5],Temp; 
	unsigned int i;
/*    ULONG W[80]; */
	ULONG *W = NULL;
    static ULONG SHA1Table[4] = { 0x5a827999, 0x6ed9eba1, 
                                  0x8f1bbcdc, 0xca62c1d6 };

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&W, sizeof(ULONG)*80);
	if (W == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

    Reg[0]=Buf[0];
	Reg[1]=Buf[1];
	Reg[2]=Buf[2];
	Reg[3]=Buf[3];
	Reg[4]=Buf[4];

    /*the first octet of a word is stored in the 0th element, bytereverse*/
	for(i = 0; i < 16; i++)
    { 
    	W[i]  = (Mes[i] >> 24) & 0xff;
        W[i] |= (Mes[i] >> 8 ) & 0xff00;
        W[i] |= (Mes[i] << 8 ) & 0xff0000;
        W[i] |= (Mes[i] << 24) & 0xff000000;
    }
    
		 
    for	(i = 0; i < 64; i++)
	    W[16+i] = CYCLIC_LEFT_SHIFT(W[i] ^ W[2+i] ^ W[8+i] ^ W[13+i], 1);
	    
    
    /* 80 steps in SHA-1 algorithm*/
    for (i=0; i<80; i++)                    
    {
        if (i<20)
            SHA1Step(SHA1_F1, Reg[0], Reg[1], Reg[2], Reg[3], Reg[4], 
                     W[i], SHA1Table[0]);
        
        else if (i>=20 && i<40)
            SHA1Step(SHA1_F2, Reg[0], Reg[1], Reg[2], Reg[3], Reg[4], 
                     W[i], SHA1Table[1]);
			
		else if (i>=40 && i<60)
            SHA1Step(SHA1_F3, Reg[0], Reg[1], Reg[2], Reg[3], Reg[4], 
                      W[i], SHA1Table[2]);
			
        else
            SHA1Step(SHA1_F2, Reg[0], Reg[1], Reg[2], Reg[3], Reg[4], 
                     W[i], SHA1Table[3]);
			

       /* one-word right shift*/
		Temp   = Reg[4];
        Reg[4] = Reg[3];
        Reg[3] = Reg[2];
        Reg[2] = Reg[1];
        Reg[1] = Reg[0];
        Reg[0] = Temp;       
  
    } /* end of for-loop*/


    /* (temporary)output*/
    for (i=0; i<5; i++)
        Buf[i] += Reg[i];

	if (W != NULL)
		os_free_mem(NULL, W);
}


/*
	========================================================================
	
	Routine Description:
		SHA1 function 

	Arguments:
		
	Return Value:

	Note:
		
	========================================================================
*/
VOID	HMAC_SHA1(
	IN	UCHAR	*text,
	IN	UINT	text_len,
	IN	UCHAR	*key,
	IN	UINT	key_len,
	IN	UCHAR	*digest)
{
/*	SHA_CTX	context;*/
	SHA_CTX	*pcontext = NULL;
	UCHAR	k_ipad[65]; /* inner padding - key XORd with ipad	*/
	UCHAR	k_opad[65]; /* outer padding - key XORd with opad	*/
	INT		i;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pcontext, sizeof(SHA_CTX));
	if (pcontext == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	/* if key is longer	than 64	bytes reset	it to key=SHA1(key)	*/
	if (key_len	> 64) 
	{
		SHAInit(pcontext);
		SHAUpdate(pcontext, key, key_len);
		SHAFinal(pcontext,	key);
		key_len	= 20;
	}
	NdisZeroMemory(k_ipad, sizeof(k_ipad));
	NdisZeroMemory(k_opad, sizeof(k_opad));
	NdisMoveMemory(k_ipad, key,	key_len);
	NdisMoveMemory(k_opad, key,	key_len);

	/* XOR key with	ipad and opad values  */
	for	(i = 0;	i <	64;	i++) 
	{	
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	/* perform inner SHA1 */
	SHAInit(pcontext); 						/* init context for 1st pass */
	SHAUpdate(pcontext,	k_ipad,	64);		/*	start with inner pad */
	SHAUpdate(pcontext,	text, text_len);	/*	then text of datagram */
	SHAFinal(pcontext, digest);				/* finish up 1st pass */

	/*perform outer	SHA1  */
	SHAInit(pcontext);					/* init context for 2nd pass */
	SHAUpdate(pcontext,	k_opad,	64);	/*	start with outer pad */
	SHAUpdate(pcontext,	digest,	20);	/*	then results of	1st	hash */
	SHAFinal(pcontext, digest);			/* finish up 2nd pass */

	if (pcontext != NULL)
		os_free_mem(NULL, pcontext);
}


/* End of crypt_md5.c */

