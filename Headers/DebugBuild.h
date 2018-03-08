//---------------------------------------------------------------------
//
//	File:	DebugBuild.h
//
//	Author:	Michael Ost
//
//	Date:	12.08.98
//
//	Desc:	Controls the DEBUG flag and includes Be's Debug facilities
//			via <support/Debug.h>
//
//	Copyright ©1998 mediapede software
//
//---------------------------------------------------------------------

#ifndef __DEBUGBUILD_H__
#define __DEBUGBUILD_H__

// Enable/disable debugging
#ifdef DEBUG
#undef DEBUG
#endif
// NOTE: the PPC compiler requires that this value be either 0 or
// 1. bool values (true/false) don't work.
#define DEBUG 0

#include <stdlib.h>

#include <MediaDefs.h>				// for media_format
//#include <Alert.h>					// for BAlert

// Include Be debug support (ASSERT etc.)
#include <support/Debug.h>

// Define some debug dependent messages
#ifdef DEBUG
	#define ERROR				printf
	#define MESSAGE				puts
	#define PRINTF				printf
	#define	FUNCTION			printf
	#define PROGRESS			printf
	#define LOOP				(void)0
#else
	#define MESSAGE				(void)0
	#define ERROR				(void)0
	#define PRINTF				(void)0
	#define	FUNCTION			(void)0
	#define PROGRESS			(void)0
	#define LOOP				(void)0
#endif

/*
// Some preformatted debug dumps
inline void DumpRect(BRect theRect)
{
	PRINTF("theRect = %f, %f, %f, %f\n", theRect.left, theRect.top, theRect.right, theRect.bottom  );
}

inline void DumpPoint(BPoint thePoint)
{
	PRINTF("thePoint = %f, %f\n", thePoint.x, thePoint.y);
}

inline void DumpMediaFormat(const media_format& f)
{
#if DEBUG
	if (f.type == B_MEDIA_RAW_VIDEO) {
		MESSAGE("type = B_MEDIA_RAW_VIDEO");
		MESSAGE("    .u.raw_video.");
		PRINTF( "    field_rate          %f\n", f.u.raw_video.field_rate);
		PRINTF( "    interlace           %d\n", f.u.raw_video.interlace);
		PRINTF( "    first_active        %d\n", f.u.raw_video.first_active);
		PRINTF( "    last_active         %d\n", f.u.raw_video.last_active);
		PRINTF( "    orientation         %d\n", f.u.raw_video.orientation);
		PRINTF( "    pixel_width_aspect  %d\n", f.u.raw_video.pixel_width_aspect);
		PRINTF( "    pixel_height_aspect %d\n", f.u.raw_video.pixel_height_aspect);
		MESSAGE("    .u.raw_video.display.");
		PRINTF( "    line_width          %d\n", f.u.raw_video.display.line_width);
		PRINTF( "    line_count          %d\n", f.u.raw_video.display.line_count);
		PRINTF( "    bytes_per_row       %d\n", f.u.raw_video.display.bytes_per_row);
		PRINTF( "    pixel_offset        %d\n", f.u.raw_video.display.pixel_offset);
		PRINTF( "    line_offset         %d\n", f.u.raw_video.display.line_offset);
	}
	else if (f.type == B_MEDIA_RAW_AUDIO) {
		MESSAGE("type = B_MEDIA_RAW_AUDIO");
		MESSAGE("    .u.raw_video.");
		PRINTF( "    frame_rate      %f\n", f.u.raw_audio.frame_rate);
		PRINTF( "    channel_count   %d\n", f.u.raw_audio.channel_count);
		PRINTF( "    format          %s\n", (f.u.raw_audio.format == media_raw_audio_format::B_AUDIO_UCHAR)? "B_AUDIO_UCHAR" :
											(f.u.raw_audio.format == media_raw_audio_format::B_AUDIO_SHORT)? "B_AUDIO_SHORT" :
											(f.u.raw_audio.format == media_raw_audio_format::B_AUDIO_FLOAT)? "B_AUDIO_FLOAT" :
											(f.u.raw_audio.format == media_raw_audio_format::B_AUDIO_INT)? "B_AUDIO_INT" :
											"UNKNOWN");
		PRINTF( "    byte_order      %s endian\n", (f.u.raw_audio.byte_order == 2)? "big" : "little");
		PRINTF( "    buffer_size     %d\n", f.u.raw_audio.buffer_size);
	}
	else
		MESSAGE("no details available for format");
#endif
}
*/

/*
// A debug only alert message
inline void DebugAlert(const char *theString)
{
#if DEBUG
//	BAlert *theAlert = new BAlert("Debug", theString, "Thanks");
//	theAlert->Go();
	MESSAGE(theString);
#endif
}
*/

#endif	// __DEBUGBUILD_H__
