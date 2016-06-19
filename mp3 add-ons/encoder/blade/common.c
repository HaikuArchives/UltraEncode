/*
			(c) Copyright 1998, 1999 - Tord Jansson
			=======================================

		This file is part of the BladeEnc MP3 Encoder, based on
		ISO's reference code for MPEG Layer 3 compression, and might
		contain smaller or larger sections that are directly taken
		from ISO's reference code.

		All changes to the ISO reference code herein are either
		copyrighted by Tord Jansson (tord.jansson@swipnet.se)
		or sublicensed to Tord Jansson by a third party.

	BladeEnc is free software; you can redistribute this file
	and/or modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

*/

/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/

#include    "common.h"
#include	<string.h> /* 1995-07-11 shn */
#include	<ctype.h>
#include <stdlib.h>

/***********************************************************************
*
*  Global Variable Definitions
*
***********************************************************************/

/* 1: MPEG-1, 0: MPEG-2 LSF, 1995-07-11 shn */
double  s_freq[2][4] = {{22.05, 24, 16, 0}, {44.1, 48, 32, 0}};

/* 1: MPEG-1, 0: MPEG-2 LSF, 1995-07-11 shn */
int     bitratex[2][15] = {
          {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160},
					{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320}
        };



/***********************************************************************
*
* Using the decoded info the appropriate possible quantization per
* subband table is loaded
*
**********************************************************************/


void hdr_to_frps(frame_params *fr_ps) /* interpret data in hdr str to fields in fr_ps */
{
layer *hdr = fr_ps->header;     /* (or pass in as arg?) */

    fr_ps->actual_mode = hdr->mode;
    fr_ps->stereo = (hdr->mode == MPG_MD_MONO) ? 1 : 2;
		fr_ps->sblimit = SBLIMIT;
    fr_ps->jsbound = fr_ps->sblimit;
}






/*******************************************************************************
*
*  Allocate number of bytes of memory equal to "block".
*
*******************************************************************************/

void  *mem_alloc(unsigned long block, char *item)
{

    void    *ptr;

    ptr = (void *) malloc(block*2);

    memset(ptr, 0, block*2);

    return(ptr);
}


/****************************************************************************
*
*  Free memory pointed to by "*ptr_addr".
*
*****************************************************************************/

void    mem_free( void **ptr_addr)
{
  if (*ptr_addr != NULL)
	{
    free(*ptr_addr);
    *ptr_addr = NULL;
  }
}



/*****************************************************************************
*
*  bit_stream.c package
*  Author:  Jean-Georges Fritsch, C-Cube Microsystems
*
*****************************************************************************/


extern	char		* pEncodedOutput;
extern	int				outputBit;


/*empty and close the buffer */

int putmask[9]={0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};


/*write n bits into the bit stream */

void putbits(unsigned int val, int n)
{
	int k, tmp;

  while (n > 0) 
  {
		k = MIN(n, outputBit);
		n -= k;
		tmp = val >> n;
		outputBit -= k;
		* pEncodedOutput |= (tmp&putmask[k]) << outputBit;
		if (!outputBit)
		{
			outputBit = 8;
			pEncodedOutput++;
			* pEncodedOutput = 0;
		}
  }
}




/*****************************************************************************
*
*  End of bit_stream.c package
*
*****************************************************************************/

/*****************************************************************************
*
*  CRC error protection package
*
*****************************************************************************/



void update_CRC(unsigned int data, unsigned int length, unsigned int * crc)
{
  unsigned int  masking, carry;

  masking = 1 << length;

  while((masking >>= 1))
	{
    carry = *crc & 0x8000;
    *crc <<= 1;
    if (!carry ^ !(data & masking))
      *crc ^= CRC16_POLYNOMIAL;
  }
  *crc &= 0xffff;
}
