//-------------------------------------------------------------------
//
//	File:	TEncodeListItem.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.10.99
//
//	Desc:	Custom list item for encoder list selection
//
//	Copyright ©1999	Mediapede, Inc.
//
//---------------------------------------------------------------------

// Includes
#include <Application.h>
#include <Region.h>

#include "DebugBuild.h"
#include "AppConstants.h"

#include "TProgressDialog.h"
#include "TQueueListView.h"

#include "TEncodeListItem.h"

//-------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------
//
//

TEncodeListItem::TEncodeListItem(TQueueListView *parent, TProgressDialog *dialog, ID3_tag *tag, int32 id, const char *text, uint32 outlineLevel, bool expanded) :
		BStringItem(text, outlineLevel, expanded),
		m_Parent(parent),
		m_ID(id),
		m_Locked(false),
		m_Pending(true),
		m_Dialog(NULL)
{
	//	Copy over tag info
	strncpy(m_Tag.songname, tag->songname, TAGLEN_SONG);				
	strncpy(m_Tag.artist, tag->artist, TAGLEN_ARTIST);
	strncpy(m_Tag.album, tag->album, TAGLEN_ALBUM);				
	strncpy(m_Tag.year, tag->year, TAGLEN_YEAR);
	strncpy(m_Tag.comment, tag->comment, TAGLEN_COMMENT);							
	m_Tag.genre = tag->genre;
	
	//	Set up strings	
	TitleString(m_Tag.songname);
	StatusString("Pending");						
}


//-------------------------------------------------------------------
//	Destructor
//-------------------------------------------------------------------
//
//

TEncodeListItem::~TEncodeListItem()
{
	if (m_Dialog)
	{
		m_Dialog->Lock();
		m_Dialog->Quit();
		m_Dialog = NULL;
	}
}


//-------------------------------------------------------------------
//	DrawItem
//-------------------------------------------------------------------
//
//

void TEncodeListItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	if (owner->LockLooper())
	{		
		owner->PushState();
			
		//	Clear out current frame
		owner->SetHighColor(kWhite);
		owner->FillRect(frame);
		
		//	Draw selection frame
		if (IsSelected())
		{
			owner->SetHighColor(kBeFocusBlue);
			owner->StrokeRect(frame);
		}
		//	Erase selection frame
		else
		{
			owner->SetHighColor(kWhite);
			owner->StrokeRect(frame);		
		}
				
		//	Draw text.  Set color if currently being encoded
		if (m_Locked)
			owner->SetHighColor(kBeFocusBlue);
		else
			owner->SetHighColor(kBlack);
		
		//	Get ready to clip
		BRegion oldClip, newClip;
		BRect clipper;
		owner->GetClippingRegion(&oldClip);
		
		//	Draw ID String
		clipper = frame;
		clipper.right = m_Parent->TitleOffset();
		newClip.Set(clipper);
		owner->ConstrainClippingRegion(&newClip);
		
		char idStr[4];
		if (m_ID < 10)
			sprintf(idStr, " %d", m_ID);
		else
			sprintf(idStr, "%d", m_ID);
		owner->DrawString(idStr, BPoint(12, frame.bottom-2));

		//	Draw Title
		clipper.left  = clipper.right;
		clipper.right = m_Parent->TimeOffset();
		newClip.Set(clipper);
		owner->ConstrainClippingRegion(&newClip);
		owner->DrawString(m_TitleString, BPoint(12 + m_Parent->TitleOffset(), frame.bottom-2));

		//	Draw Time
		clipper.left  = clipper.right;
		clipper.right = m_Parent->StatusOffset();
		newClip.Set(clipper);
		owner->ConstrainClippingRegion(&newClip);
		owner->DrawString(m_TimeString, BPoint(12 + m_Parent->TimeOffset(), frame.bottom-2));
						
		//	Draw Status
		if (m_Pending && !m_Locked)
		{
			const rgb_color darkGreen = {0, 125, 0,   255};
			owner->SetHighColor(darkGreen);
		}
		else if(m_Pending)
			owner->SetHighColor(kBeFocusBlue);
		/*else
		{
			const rgb_color darkRed = {125, 0, 0,   255};
			owner->SetHighColor(darkRed);
		}*/
			
		clipper.left  = clipper.right;
		clipper.right = frame.right;
		newClip.Set(clipper);
		owner->ConstrainClippingRegion(&newClip);
		owner->DrawString(m_StatusString, BPoint(12 + m_Parent->StatusOffset(), frame.bottom-2));
				
		owner->ConstrainClippingRegion(&oldClip);
		owner->PopState();
		owner->UnlockLooper();
	}
}

#pragma mark -
#pragma mark === Info Setting ===

//-------------------------------------------------------------------
//	UpdateInfo
//-------------------------------------------------------------------
//
//	A change has occured in the track info.  Get info and display it 
//	properly.
//

void TEncodeListItem::UpdateInfo()
{
	/*//	Update title
	strncpy(m_TitleString, m_ID3Tag.songname, TAGLEN_SONG);
		
	//	Update list and force redraw
	if (m_Parent->LockLooper())
	{
		int32 index = m_Parent->IndexOf(this);
		if (index >= 0)
		{
			//	Update title list
			track_info *trackInfo = (track_info*)m_Parent->GetParent()->TitleList()->ItemAt(index + 1);
			if (trackInfo)
				sprintf(trackInfo->title, "%s", m_TitleString);
			
			//	Redraw		
			m_Parent->Invalidate(m_Parent->ItemFrame(index));			
		}
				
		m_Parent->UnlockLooper();
	}*/
}

