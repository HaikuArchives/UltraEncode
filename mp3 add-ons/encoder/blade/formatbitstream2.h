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

#ifndef	FORMATBITSTREAM2_DOT_H
#define	FORMATBITSTREAM2_DOT_H


#define MAX_CHANNELS 2
#define MAX_GRANULES 2


#ifndef				_SYS_TYPES_H
typedef	unsigned int uint;
#endif

/*____ Structure Definitions ________________________________________________*/

typedef struct
{
  uint value;
  int	 length;
} BitHolderElement;


typedef struct
{
  int								max_elements;
  int								nrEntries;
  BitHolderElement	*element;
} BitHolder;


typedef struct BF_FrameData
{
    int         frameLength;
    int         nGranules;
    int         nChannels;
    BitHolder		header;
    BitHolder		frameSI;
    BitHolder		channelSI[MAX_CHANNELS];
    BitHolder		spectrumSI[MAX_GRANULES][MAX_CHANNELS];
    BitHolder		scaleFactors[MAX_GRANULES][MAX_CHANNELS];
    BitHolder		codedData[MAX_GRANULES][MAX_CHANNELS];
    BitHolder		userSpectrum[MAX_GRANULES][MAX_CHANNELS];
    BitHolder		userFrameData;
} BF_FrameData;


typedef struct BF_FrameResults
{
    int SILength;
    int mainDataLength;
    int nextBackPtr;
} BF_FrameResults;


/*____ Function Prototypes __________________________________________________*/

BitHolder * initBitHolder( BitHolder * wp, int elements );
void				exitBitHolder( BitHolder * wp );
void				addBits( BitHolder * wp, uint value, int length );
void				writeFrame( BF_FrameData *psFrame, BF_FrameResults *results );


#endif
