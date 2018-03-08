//-------------------------------------------------------------------
//
//	File:	TMP3Writer.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <File.h>
#include <MediaFile.h>
#include <MediaTrack.h>
#include <MediaFormats.h>
#include <MediaWriter.h>

#include "TMP3Writer.h"

#if NDEBUG
 #define FPRINTF (void)
#else
 #define FPRINTF fprintf
#endif


const double MAX_VALUE = 1024.0*1024.0*1024.0*2.0-1.0;

enum {
	OUTPUT_BUFFER_SIZE = 2048
};


//	Code
MediaWriter *instantiate_mediawriter() 
{
	return new TMP3Writer();
}

status_t get_mediawriter_info(media_file_format *mfi) 
{
	strcpy(mfi->mime_type,       "audio/x-mpeg");
	strcpy(mfi->pretty_name,    "MP3 Audio File Format (MP3)");
	strcpy(mfi->short_name,     "mp3");
	strcpy(mfi->file_extension, "mp3");

	mfi->capabilities = media_file_format::B_KNOWS_RAW_AUDIO |
		                media_file_format::B_KNOWS_ENCODED_AUDIO |
		                media_file_format::B_WRITABLE;
	mfi->family       = B_MPEG_FORMAT_FAMILY;

	return B_OK;
}

status_t accepts_format(media_format *fmt) {
	status_t					err;
	BMediaFormats				formatObject;
	media_format_description	fd;

	if (fmt->type == B_MEDIA_ENCODED_AUDIO) 
	{
		formatObject.Lock();
		err = formatObject.GetCodeFor(*fmt, B_MPEG_FORMAT_FAMILY, &fd);
		formatObject.Unlock();

		if (err != B_OK)
			return err;
			
		if (fd.family != B_MPEG_FORMAT_FAMILY)
			return B_BAD_TYPE;
			
		return B_OK;
	}
	else if (fmt->type == B_MEDIA_RAW_AUDIO)
		return B_OK;

	return B_BAD_TYPE;
}


TMP3Writer::TMP3Writer():
	m_Track(NULL),
	m_HeaderCommitted(false),
	m_OutFile(NULL)
{
}


TMP3Writer::~TMP3Writer() 
{
}


status_t TMP3Writer::SetSource(BDataIO *source) 
{
	int fd;	

	BFile *file = dynamic_cast<BFile *>(source);
	if (file == NULL)
		return B_BAD_TYPE;

	FPRINTF(stderr, "#### SetRef In\n");
	if (file->InitCheck() != B_OK)
		return file->InitCheck();

	fd = file->Dup();
	m_OutFile = fdopen(fd, "w+");
	FPRINTF(stderr, "#### SetRef Out\n");
	m_HeaderCommitted = false;
	
	return B_OK;
}


status_t TMP3Writer::AddTrack(BMediaTrack *track) 
{
	media_format				mf;
	media_format_description	fd;

	//	Check if we didn't alreday add a audio track
	FPRINTF(stderr, "#### AddTrack In\n");
	if (m_Track != NULL)
		return B_BAD_INDEX;
		
	if(m_HeaderCommitted)
		return B_NOT_ALLOWED;
		
	track->EncodedFormat(&mf);
	
	//	Check if it's an encoded format that we know how to support
	if (mf.type == B_MEDIA_ENCODED_AUDIO)
	{
		FPRINTF(stderr, "TMP3Writer::AddTrack() - Bad format type -\n");
		return B_BAD_TYPE;
	}
	
	if (mf.type == B_MEDIA_RAW_AUDIO) 
	{
		FPRINTF(stderr, "TMP3Writer::AddTrack() - Bad format type -\n");
		return B_OK;
	}
		
	//	You can't add anything else than an audio track
	FPRINTF(stderr, "TMP3Writer::AddTrack() - Bad format type -\n");
	return B_BAD_TYPE;
}


status_t TMP3Writer::AddChunk(int32 type, const char *data, size_t size) 
{
	return B_ERROR;
}

status_t TMP3Writer::AddCopyright(const char *data) 
{
	if(m_HeaderCommitted)
		return B_NOT_ALLOWED;

	return B_OK;
}

status_t TMP3Writer::AddTrackInfo(int32 track, uint32 code, const char *data, size_t size) 
{
	if(m_HeaderCommitted)
		return B_NOT_ALLOWED;
	
	return B_OK;
}


status_t TMP3Writer::CommitHeader() 
{
	m_HeaderCommitted = true;
	return B_OK;
}



status_t TMP3Writer::WriteData(	int32 tracknum, media_type type, const void *data,
								size_t size, media_encode_info *info) 
{
	if (tracknum != 0)
		return B_BAD_INDEX;
	
	if(!m_HeaderCommitted)
		return B_NOT_ALLOWED;

	FPRINTF(stderr, "#### WriteData In\n");
	if (fwrite((char*)data, 1, size, m_OutFile) <= 0) 
	{
		FPRINTF(stderr, "TMP3Writer::WriteData() - fwrite error -\n");
		return B_ERROR;
	}

	return B_OK;
}


status_t TMP3Writer::CloseFile() 
{
	if(!m_HeaderCommitted)
		return B_NOT_ALLOWED;
		
	FPRINTF(stderr, "#### CloseFile In\n");
	
	//	Just close the file...
	if (fclose(m_OutFile) != 0)
	{
		FPRINTF(stderr, "TMP3Writer::CloseFile() - fclose error -\n");
		return B_ERROR;
	}
		
	return B_OK;
}

