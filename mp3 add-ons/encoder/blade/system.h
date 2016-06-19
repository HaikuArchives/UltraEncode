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

/*==== High level defines =====================================================*/


#if !defined(system_h)
#define system_h

#define         WIN32_INTEL             0
#define         WIN32_ALPHA             1
#define         SOLARIS                 2
#define         LINUX_I386              3
#define         IBM_OS2                 4
#define         IRIX                    5
#define         ATARI_TOS               6
#define         UNIXWARE7               7
#define      		LINUX_SPARC    					8
#define         BEOS                    9

#if !defined(SYSTEM)                          /* so we can define in make file */
 #if defined(__BEOS__)
  #define        SYSTEM         BEOS
  #include <ByteOrder.h>	/* need it before defines below */
  #include <SupportDefs.h>	/* ditto */
 #else
  #define        SYSTEM         LINUX_I386   /* Set current system here, select */
 #endif                                       /* from list above. */
#endif

					
 
/*==== Low level defines ======================================================*/
 
/*
  LIST OF DEFINES
  ===============

  BYTEORDER	[byteorder]     Should either be set to BIG_ENDIAN or LITTLE_ENDIAN depending on the processor.
  INLINE    [prefix]        Defines the prefix for inline functions, normally _inline.
                             Skip this define if your compiler doesn't support inline functions.
  DRAG_DROP                 Set if Drag-n-Drop operations are supported. If defined, the hint for drag and drop
                             is displayed in the help menu.
  PRIO                      Set if priority can be set with the -PRIO switch.
  MSWIN                     Set this for windows systems. Includes "windows.h" etc.
  WILDCARDS                 Set this if the program has to expand wildcards itself on your system.
  NO_ZERO_CALLOC            Set this to work around a bug when allocation 0 bytes memory with some compilers.
	DIRECTORY_SEPARATOR				Should either be '\\' or '/'.
	WAIT_KEY									Set this on systems where we as default want to wait for a keypress before quiting.
*/


/*  Most systems allready have these two defines, but some doesn't 
    so we have to put them here, before they are used. */

#ifndef BIG_ENDIAN
	#define			BIG_ENDIAN				4321
#endif

#ifndef LITTLE_ENDIAN
	#define			LITTLE_ENDIAN			1234
#endif



/*_____ Windows 95/98/NT Intel defines ________________________________________*/

#if     SYSTEM == WIN32_INTEL
				#define					BYTEORDER							LITTLE_ENDIAN
        #define         INLINE								_inline
        #define         DRAG_DROP
        #define         PRIO
        #define         MSWIN
        #define         WILDCARDS
				#define					DIRECTORY_SEPARATOR		'\\'
				#define					WAIT_KEY
#endif

/*_____ Windows NT DEC Alpha defines __________________________________________*/

#if SYSTEM == WIN32_ALPHA

        #define         BYTEORDER							BIG_ENDIAN
        #define         INLINE								_inline
        #define         DRAG_DROP
        #define         PRIO
        #define         MSWIN
        #define         WILDCARDS
				#define					DIRECTORY_SEPARATOR		'\\'
				#define					WAIT_KEY
#endif


/*____ Linux defines ________________________________________________________*/

#if SYSTEM == LINUX_I386

        #define         BYTEORDER							LITTLE_ENDIAN
				#define					DIRECTORY_SEPARATOR		'/'
#endif

/*____ Linux defines ________________________________________________________*/

#if SYSTEM == LINUX_SPARC

				#define        	BYTEORDER             BIG_ENDIAN
				#define         DIRECTORY_SEPARATOR   '/'
#endif

/*____ Solaris defines ________________________________________________________*/

#if SYSTEM == SOLARIS

				#define					BYTEORDER							BIG_ENDIAN
				#define					DIRECTORY_SEPARATOR		'/'
#endif


/*____ OS/2 ___________________________________________________________________*/

#if SYSTEM == IBM_OS2

				#define					BYTEORDER							LITTLE_ENDIAN
        #define         INLINE								inline
        #define         PRIO
        #define         WILDCARDS
				#define					DIRECTORY_SEPARATOR		'\\'
        #define         OS2
				#define					WAIT_KEY
#endif


/*____ IRIX defines ___________________________________________________________*/

#if SYSTEM == IRIX

				#define					BYTEORDER							BIG_ENDIAN
				#define					DIRECTORY_SEPARATOR		'/'
#endif


/*____ TOS ___________________________________________________________________*/

#if SYSTEM == ATARI_TOS

				#define					BYTEORDER							BIG_ENDIAN
        #define         INLINE								inline
				#define					DIRECTORY_SEPARATOR		'\\'
        #define         TOS
 #endif


/*____ UNIXWARE7 ______________________________________________________________*/

#if SYSTEM == UNIXWARE7

				#define					BYTEORDER							LITTLE_ENDIAN
				#define					DIRECTORY_SEPARATOR		'/'
#endif

#if SYSTEM == BEOS
 #if B_HOST_IS_LENDIAN
  #define BYTEORDER LITTLE_ENDIAN
 #else
  #define BYTEORDER BIG_ENDIAN
 #endif
 #define DIRECTORY_SEPARATOR '/'
 #define PRIO 1
#endif


/*____ To make sure that certain necessary defines are set... */

#ifndef INLINE
        #define INLINE
#endif

/*==== Other Global Definitions, placed here for convenience ==================*/

#if !defined(FALSE)
 #define         FALSE           0
#endif
#if !defined(TRUE)
 #define         TRUE            1
#endif

#ifndef _SUPPORT_DEFS_H
typedef         unsigned        char    uchar;
#endif

#ifndef _SYS_TYPES_H
typedef         unsigned        short   ushort;
typedef         unsigned        long    ulong;
typedef         unsigned        int     uint;
#endif

#endif	/* system_h */

