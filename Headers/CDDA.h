/*
	Originally public interface to CDDA device, later modified
	to abstract the API to BeOS R4.1 CDDA capabilities.
	Copyright 1997-1999 Peter Urbanec. All Rights Reserved.
	
	Version 4.1

	This code can be freely used for non-profit or for commercial purposes.

	If you wish to use any portion of this code you must include the following
	disclaimer in documentation associated with the product as well as any
	"about boxes" or other places where acknowledgements are normally listed,
	in a typeface which is no less prominent than the typeface used for any
	other credits.

	"Contains software components developed by Peter Urbanec"

*/

#ifndef _CDDA_H
#define _CDDA_H

/* Flags for TOC entries */
#define	CDDA_TOC_FLAG_AUDIO_PREEMPHASIS	0x01
#define	CDDA_TOC_FLAG_DIGITAL_COPY_OK	0x02
#define	CDDA_TOC_FLAG_DATA_TRACK		0x04
#define	CDDA_TOC_FLAG_AUDIO_FOUR_CHAN	0x08

/* MSF struct for CDDA addresses */
typedef struct
{
	uint8	reserved;
	uint8	m; /* Minute */
	uint8	s; /* Second */
	uint8	f; /* Frame */
} CDDA_MSF;

/* Each entry in the Table Of Contents has the following info */
typedef struct
{
	uint8	reserved1;
	uint8	flags;
	uint8	track_number;
	uint8	reserved2;
	CDDA_MSF	address;
} CDDA_TOC_ENTRY;

/* Table Of Contents for an entire CD */
typedef struct
{
	uint16	toc_length;		/* Number of bytes that follows */
	uint8	first_track;	/* The track number which will appear in track[0] */
	uint8	last_track;		/* The last valid entry in the toc */
	CDDA_TOC_ENTRY	track[100];
} CDDA_TOC;

/* One CDDA sample point */
typedef struct
{
	int16	left;	/* Note that CDDA data comes in */
	int16	right;	/* low byte, high byte order */
} CDDA_SAMPLE;

/* One CDDA block */
typedef struct
{
	CDDA_SAMPLE		sample[588];	/* 588 L+R samples */
} CDDA_BLOCK;

#endif  /* _CDDA_H */
