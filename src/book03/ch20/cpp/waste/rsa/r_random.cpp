/* R_RANDOM.C - random objects for RSAREF
*/

/* Copyright (C) RSA Laboratories, a division of RSA Data Security,
Inc., created 1991. All rights reserved.
*/

#include "stdafx.hpp"

#include "global.hpp"
#include "rsaref.hpp"
#include "r_random.hpp"
#include "md5.hpp"

#define RANDOM_BYTES_NEEDED 256

/*
Params:
	R_RANDOM_STRUCT *randomStruct;	new random structure
*/
int R_RandomInit(R_RANDOM_STRUCT *randomStruct)
{
	randomStruct->bytesNeeded = RANDOM_BYTES_NEEDED;
	R_memset ((POINTER)randomStruct->state, 0, sizeof (randomStruct->state));
	randomStruct->outputAvailable = 0;
	return (0);
}

/*
Params:
	R_RANDOM_STRUCT *randomStruct;	random structure
	unsigned char *block;			block of values to mix in
	unsigned int blockLen;			length of block
*/
int R_RandomUpdate(R_RANDOM_STRUCT *randomStruct, unsigned char *block, unsigned int blockLen)
{
	MD5_CTX context;
	unsigned char digest[16];
	unsigned int i, x;

	MD5Init(&context);
	MD5Update(&context, block, blockLen);
	MD5Final(digest, &context);

	/* add digest to state */
	x = 0;
	for (i = 0; i < 16; i++) {
		x += randomStruct->state[15-i] + digest[15-i];
		randomStruct->state[15-i] = (unsigned char)(x&0xff);//HACK md5chap
		x >>= 8;
	};

	if (randomStruct->bytesNeeded < blockLen) randomStruct->bytesNeeded = 0;
	else randomStruct->bytesNeeded -= blockLen;

	//Zeroize sensitive information.
	R_memset ((POINTER)digest, 0, sizeof (digest));
	x = 0;

	return (0);
}

/*
Params:
	unsigned int *bytesNeeded;		number of mix-in bytes needed
	R_RANDOM_STRUCT *randomStruct;	random structure
*/
int R_GetRandomBytesNeeded(unsigned int *bytesNeeded, R_RANDOM_STRUCT *randomStruct)
{
	*bytesNeeded = randomStruct->bytesNeeded;
	return (0);
}

/*
Params:
	unsigned char *block;			block
	unsigned int blockLen;			length of block
	R_RANDOM_STRUCT *randomStruct;	random structure
*/
int R_GenerateBytes(unsigned char *block, unsigned int blockLen, R_RANDOM_STRUCT *randomStruct)
{
	MD5_CTX context;
	unsigned int available, i;

	if (randomStruct->bytesNeeded)
		return (RE_NEED_RANDOM);

	available = randomStruct->outputAvailable;

	while (blockLen > available) {
		R_memcpy
			((POINTER)block, (POINTER)&randomStruct->output[16-available],
			available);
		block += available;
		blockLen -= available;

		/* generate new output */
		MD5Init (&context);
		MD5Update (&context, randomStruct->state, sizeof(randomStruct->state));
		MD5Final (randomStruct->output, &context);
		available = 16;

		/* increment state */
		for (i = 0; i < 32; i++)
			if (randomStruct->state[31-i]++)
				break;
	};

	R_memcpy((POINTER)block, (POINTER)&randomStruct->output[16-available], blockLen);
	randomStruct->outputAvailable = available - blockLen;

	return (0);
}

/*
Params:
	R_RANDOM_STRUCT *randomStruct;	random structure
*/
void R_RandomFinal(R_RANDOM_STRUCT *randomStruct)
{
	R_memset((POINTER)randomStruct, 0, sizeof (*randomStruct));
}
