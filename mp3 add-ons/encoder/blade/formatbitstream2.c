/*
			(c) Copyright 1998, 1999 - Tord Jansson
			=======================================

		This file is part of the BladeEnc MP3 Encoder, based on
		ISO's reference code for MPEG Layer 3 compression.

		This file doesn't contain any of the ISO reference code and
		is copyright Tord Jansson (tord.jansson@swipnet.se).

	BladeEnc is free software; you can redistribute this file
	and/or modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<malloc.h>
#include	"formatbitstream2.h"
#include	"common.h"

static int bitHolderBits( BitHolder * psBH );
static int writeMainDataBits( BF_FrameData * psFrame, BitHolder *psBH );


static	int BitsRemaining = 0;
static	int	sideInfoSize;					/* Keeping this global is just a small optimization. */


/*____ initBitHolder() ______________________________________________________*/

BitHolder * initBitHolder( BitHolder * wp, int elements )
{
	BitsRemaining = 0;								/* Shouldn't really be here, just a quick patch... */
	wp->element = (BitHolderElement *) malloc( sizeof( BitHolderElement ) * elements );
	wp->max_elements = elements;
	wp->nrEntries = 0;
	return	wp;
}

/*____ exitBitHolder() ______________________________________________________*/

void exitBitHolder( BitHolder * wp )
{
	if( wp->element != NULL )
		free( wp->element );
	wp->element = NULL;
}


/*____ addBits() ____________________________________________________________*/

void addBits( BitHolder * wp, uint value, int length )
{
	if( length != 0 )
	{
		if( wp->nrEntries == wp->max_elements )
		{
			printf( "ERROR: BitHolder overflow!\n" );
			exit( -1 );
		}
		wp->element[wp->nrEntries].value = value;
		wp->element[wp->nrEntries].length = length;
		wp->nrEntries++;
	}
}


/*____ writeFrame() _________________________________________________________*/

void writeFrame( BF_FrameData *psFrame, BF_FrameResults *results )
{
	int	bits;
	int	ch, gr;


	// Compute and return size of SideInfo.

	bits = bitHolderBits( &psFrame->header );
	bits += bitHolderBits( &psFrame->frameSI );

	for ( ch = 0; ch < psFrame->nChannels; ch++ )
		bits += bitHolderBits( &psFrame->channelSI[ch] );

  for ( gr = 0; gr < psFrame->nGranules; gr++ )
		for ( ch = 0; ch < psFrame->nChannels; ch++ )
			bits += bitHolderBits( &psFrame->spectrumSI[gr][ch] );
	
  results->SILength = sideInfoSize = bits;


	// Put the bits and compute size of mainData

	bits = 0;
  for ( gr = 0; gr < psFrame->nGranules; gr++ )
		for ( ch = 0; ch < psFrame->nChannels; ch++ )
		{
			bits += writeMainDataBits( psFrame, &psFrame->scaleFactors[gr][ch] );
			bits += writeMainDataBits( psFrame, &psFrame->codedData[gr][ch] );
			bits += writeMainDataBits( psFrame, &psFrame->userSpectrum[gr][ch] );
		}
  bits += writeMainDataBits( psFrame, &psFrame->userFrameData );

  results->mainDataLength = bits;


  /* calculate nextBackPointer */

  results->nextBackPtr = (BitsRemaining / 8);	/* BitsRemaining must be dividable by 8 */

}


/*=============================================================================
									>>> Static Functions <<<
=============================================================================*/

/*____ bitHolderBits() ______________________________________________________*/

static int bitHolderBits( BitHolder * psBH )
{
	BitHolderElement * psElem = psBH->element;
	int i, bits = 0;

	for( i = 0 ; i < psBH->nrEntries ; i++, psElem++ )
		bits += psElem->length;

	return	bits;
}

//____ writeBitHolder() _______________________________________________________

static void writeBitHolder( BitHolder * part )
{
  BitHolderElement * ep;
  int i;

  ep = part->element;
  for ( i = 0; i < part->nrEntries; i++, ep++ )
		putbits( ep->value, ep->length );
}

//____ calcCRC() _____________________________________________________________

static int	calcCRC( char * pData, int size )
{
	int		i;
	int		crc = 0xffff;
	int		masking;
	int		carry;
	int		data;
			
	for( i = 2 ; i < size ; i++ )
	{
		if( i != 4 && i != 5 )
		{
			masking = 1 << 8;
			data = pData[i];
			while( masking >>= 1 )
			{
				carry = crc & 0x8000;
		    crc <<= 1;
		    if (!carry ^ !(data & masking))
					crc ^= 0x8005;
      }			
		}
	}
	
	crc &= 0xffff;
	return crc;
}

//____ writeSideInfo() ________________________________________________________

extern	char		* pEncodedOutput;


static void writeSideInfo( BF_FrameData * psFrame )
{
  int ch, gr;
	int	crc;
	char	* pHeader = pEncodedOutput;

  writeBitHolder( &psFrame->header );
  writeBitHolder( &psFrame->frameSI );

  for ( ch = 0; ch < psFrame->nChannels; ch++ )
		writeBitHolder( &psFrame->channelSI[ch] );

  for ( gr = 0; gr < psFrame->nGranules; gr++ )
		for ( ch = 0; ch < psFrame->nChannels; ch++ )
	    writeBitHolder( &psFrame->spectrumSI[gr][ch] );

	// Checksum generation (if CRC enabled).

	if( !(pHeader[1] & 0x1) )
	{
		crc = calcCRC( pHeader, pEncodedOutput - pHeader );
		pHeader[4] = (char) (crc >> 8);
		pHeader[5] = (char) crc;
	}
}


/*____ writeMainDataBits() __________________________________________________*/

static int writeMainDataBits( BF_FrameData * psFrame, BitHolder *psBH )
{
  BitHolderElement * psElem = psBH->element;
  int i, bits = 0;
	unsigned int	val;
	int nBits;

  for ( i = 0; i < psBH->nrEntries; i++, psElem++ )
  {
		val = psElem->value;
		nBits = psElem->length;

		if ( BitsRemaining == 0 )
		{
			writeSideInfo( psFrame );
			BitsRemaining = psFrame->frameLength - sideInfoSize;
		}

		if ( nBits > BitsRemaining )
		{
			nBits -= BitsRemaining;
			putbits( val >> nBits, BitsRemaining );
			writeSideInfo( psFrame );
			BitsRemaining = psFrame->frameLength - sideInfoSize;
			putbits( val, nBits );
		}
		else
			putbits( val, nBits );
		BitsRemaining -= nBits;
		
		bits += psElem->length;
  }
  return	bits;
}
