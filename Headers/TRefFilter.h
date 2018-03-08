//---------------------------------------------------------------------
//
//	File:	TRefFilter.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	05.25.98
//
//
//	Copyright ©1998 mediapede Software
//
//---------------------------------------------------------------------

#ifndef __TREFFILTER_H__
#define __TREFFILTER_H__

#include <FilePanel.h>

typedef enum 
{
	kAudioFilter,
	kAudioAiffFilter,
	kImageFilter,
	kTextFilter,
	kVideoFilter,
	kCueSheetFilter,
	kDirectoryFilter
} FilterType;

// Class Definition
class TRefFilter : public BRefFilter 
{
	public:
		TRefFilter(FilterType filterType);
		
		virtual bool Filter(const entry_ref *, BNode *, struct stat_beos *, const char *mimetype);
		
	private:
		FilterType m_FilterType;

};

#endif
