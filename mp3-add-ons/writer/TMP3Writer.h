//-------------------------------------------------------------------
//
//	File:	TMP3Writer.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	09.07.99
//
//	Desc:	MP3 file format writer
//
//	Copyright ©1999	Mediapede, Inc.
//
//---------------------------------------------------------------------


#ifndef _MP3_WRITER_H
#define _MP3_WRITER_H

#include <OS.h>
#include <SupportDefs.h>
#include <Locker.h>
#include <MediaDefs.h>
#include <MediaWriter.h>

class BFile;

namespace BPrivate 
{
	class __declspec(dllexport) TMP3Writer;
}


namespace BPrivate 
{

	// TMP3Writer is the base virtual class that must be subclassed to implement
	// specific encapsulation format. Each flavour lives in its own add-on.
	
	class TMP3Writer : public MediaWriter 
	{
		public:
		
			TMP3Writer();
			~TMP3Writer();
		
			status_t	SetSource(BDataIO *source);
			
			status_t	AddTrack(BMediaTrack *track);
			status_t	AddCopyright(const char *data);
			status_t	AddTrackInfo(int32 track, uint32 code, const char *data,size_t sz);
			status_t	AddChunk(int32 type, const char *data, size_t size);
			status_t	CommitHeader();
		
			status_t	WriteData(int32 			tracknum,
								  media_type 		type,
								  const void 		*data,
								  size_t 			size,
								 media_encode_info	*info);
		
			status_t	CloseFile();
	
	
		private:
			BMediaTrack 	*m_Track;		  // we only support 1 audio track
			bool			m_HeaderCommitted;
			FILE			*m_OutFile;
	};

}	//	namespace


#endif
