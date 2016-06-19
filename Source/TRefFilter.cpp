//---------------------------------------------------------------------
//
//	File:	TRefFilter.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	05.25.98
//
//	Desc:	General Purpose Ref Filter.  See header file for
//			types of filters
//
//	Copyright ©1998 mediapede Software
//
//---------------------------------------------------------------------

// Includes
#include <app/Application.h>
#include <assert.h>
#include <NodeInfo.h>

#include "DebugBuild.h"
#include "AppUtils.h"

#include "TRefFilter.h"

//---------------------------------------------------------------------
//	Constructor
//---------------------------------------------------------------------
//
//

TRefFilter::TRefFilter(FilterType filterType) : BRefFilter()
{	
	m_FilterType = filterType;
}


//---------------------------------------------------------------------
//	Constructor
//---------------------------------------------------------------------
//
//

bool TRefFilter::Filter(const entry_ref *theRef, BNode *theNode, struct stat *theStat, const char *mimetype)
{
	//	Create BEntry and traverse to get source ref
	BEntry entry(theRef, true);
	if (entry.InitCheck() != B_OK)
	{
		ERROR("TRefFilter::Filter() - Error creating BEntry -\n");
		return false;
	}
		
	//	Create a node from ref
	BNode localNode(theRef);
	if (localNode.InitCheck() != B_OK)
	{
		ERROR("TRefFilter::Filter() - Error creating BNode -\n");
		return false;
	}
		
	//	Get node info
	BNodeInfo nodeInfo(&localNode);	 
	if (nodeInfo.InitCheck() != B_OK)
	{
		ERROR("TRefFilter::Filter() - Error getting BNodeInfo -\n");
		return false;
	}
	
	//	Get stat info
	struct stat st;
	if (entry.GetStat(&st) != B_OK)
	{
		ERROR("TRefFilter::Filter() - Error getting stat info -\n");
		return false;	
	}
	
	switch(m_FilterType)
	{
		case kAudioFilter:
		{
			// Allow directories
			if (S_ISDIR(st.st_mode)) 
				return true;
					
			// Allow audio
			if (IsAudio(nodeInfo))
				return true;
		}
		break;
	
		case kAudioAiffFilter:
		{
			// Allow directories
			if (S_ISDIR(st.st_mode)) 
				return true;
					
			// Allow audio
			if (IsAudioAiff(nodeInfo))
				return true;
		}
		break;
			
		case kImageFilter:
			{
				// Allow directories
				if (S_ISDIR(st.st_mode)) 
					return true;
					
				// Allow images
				if (IsImage(nodeInfo))
					return true;							
			}
			break;
			
		case kTextFilter:
			{
				// Allow directories
				if (S_ISDIR(st.st_mode)) 
					return true;
					
				// Allow text
				if (IsText(nodeInfo))
					return true;
			}
			break;
			
		case kVideoFilter:
			{
				// Allow directories
				if (S_ISDIR(st.st_mode)) 
					return true;
					
				// Allow video
				if (IsVideo(nodeInfo))
					return true;
			}
			break;
			
		case kCueSheetFilter:
			{
				// Allow directories
				if (S_ISDIR(st.st_mode)) 
					return true;
					
				// Allow CueSheets
				if (IsCueSheet(nodeInfo))
					return true;
			}
			break;
		
		case kDirectoryFilter:
		{
			// Allow directories
			if (S_ISDIR(st.st_mode)) 
				return true;					
		}
		break;
		
		default:
			return true;
	}	
	
	// Fail if we get here
	return false;
}

