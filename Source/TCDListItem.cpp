//-------------------------------------------------------------------
//
//	File:	TCDListItem.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.10.99
//
//	Desc:	Custom lsit item for CD list selection
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
#include "TProgressListView.h"

#include "TCDListItem.h"

//-------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------
//
//

TCDListItem::TCDListItem(TProgressListView *parent, int32 id, const char *text, uint32 outlineLevel, bool expanded) :
		BStringItem(text, outlineLevel, expanded),
		m_Parent(parent),
		m_ID(id),
		m_Selected(true),
		m_Locked(false),
		m_Pending(true),
		m_Dialog(NULL),
		m_EditMode(false)
{

	//	Set up ID3 Tag info
	m_ID3Tag.tag[0] 		= '\0';
	m_ID3Tag.songname[0] 	= '\0';
	m_ID3Tag.artist[0] 		= '\0';
	m_ID3Tag.album[0] 		= '\0';
	m_ID3Tag.year[0] 		= '\0';
	m_ID3Tag.comment[0] 	= '\0';
	m_ID3Tag.genre 			= 0;
  
					
	strncpy(m_ID3Tag.artist, m_Parent->GetParent()->Artist()->String(), TAGLEN_ARTIST);
	strncpy(m_ID3Tag.album, m_Parent->GetParent()->Album()->String(), TAGLEN_ALBUM);				
	strncpy(m_ID3Tag.year, m_Parent->GetParent()->Year()->String(), TAGLEN_YEAR);

	track_info *trackData = (track_info *)m_Parent->GetParent()->TitleList()->ItemAt(id);
	if (trackData)
		strncpy( m_ID3Tag.songname, trackData->title, TAGLEN_SONG);
	else
		strncpy( m_ID3Tag.songname, "Untitled", TAGLEN_SONG);
	
	strncpy(m_ID3Tag.artist, m_Parent->GetParent()->Artist()->String(), TAGLEN_ARTIST);
	strncpy(m_ID3Tag.album, m_Parent->GetParent()->Album()->String(), TAGLEN_ALBUM);

  	//char 	year[TAGLEN_YEAR+1];
  	//char 	comment[TAGLEN_COMMENT+1];
}


//-------------------------------------------------------------------
//	Destructor
//-------------------------------------------------------------------
//
//

TCDListItem::~TCDListItem()
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

void TCDListItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	if (owner->LockLooper())
	{		
		owner->PushState();
			
		//	Clear out current frame
		owner->SetHighColor(kWhite);
		owner->FillRect(frame);
		
		//	Set up select rect
		BRect selectRect = frame;
		selectRect.InsetBy(0, 2);
		selectRect.left += 5;
		selectRect.right = selectRect.left + selectRect.Height();
		owner->SetHighColor(kBlack);
		owner->StrokeRect(selectRect);
				
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
		
		//	Draw check
		if (m_Selected)
		{
			owner->SetHighColor(kBlack);
			BPoint startPt, endPt;
			startPt.Set(selectRect.left, selectRect.top);
			endPt.Set(selectRect.right, selectRect.bottom);
			owner->StrokeLine(startPt, endPt);
			startPt.Set(selectRect.right, selectRect.top);
			endPt.Set(selectRect.left, selectRect.bottom);
			owner->StrokeLine(startPt, endPt);			
		}
		
		//	Draw text.  Set color if currently being extracted
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
		owner->DrawString(m_IDString, BPoint(selectRect.right + 5, selectRect.bottom));

		//	Draw Title
		clipper.left  = clipper.right;
		clipper.right = m_Parent->TimeOffset();
		newClip.Set(clipper);
		owner->ConstrainClippingRegion(&newClip);
		/*if (m_EditMode)
		{
			//	Fill bounding box
			owner->SetHighColor(kBlack);
			clipper.InsetBy(1, 1);
			owner->FillRect(clipper);
			
			//	Draw text
			owner->SetHighColor(kWhite);
			owner->SetLowColor(kBlack);
			owner->DrawString(m_TitleString, BPoint(selectRect.right + m_Parent->TitleOffset(), selectRect.bottom));
			
			//	Restore color
			owner->SetHighColor(kBlack);
			owner->SetLowColor(kWhite);
		}
		else*/
			owner->DrawString(m_TitleString, BPoint(selectRect.right + m_Parent->TitleOffset(), selectRect.bottom));

		//	Draw Time
		clipper.left  = clipper.right;
		clipper.right = m_Parent->StatusOffset();
		newClip.Set(clipper);
		owner->ConstrainClippingRegion(&newClip);
		owner->DrawString(m_TimeString, BPoint(selectRect.right + m_Parent->TimeOffset(), selectRect.bottom));
						
		//	Draw Status
		if (m_Pending && !m_Locked)
		{
			const rgb_color darkGreen = {0, 125, 0,   255};
			owner->SetHighColor(darkGreen);
		}
		else if(m_Pending)
			owner->SetHighColor(kBeFocusBlue);
		else
		{
			const rgb_color darkRed = {125, 0, 0,   255};
			owner->SetHighColor(darkRed);
		}
			
		clipper.left  = clipper.right;
		clipper.right = frame.right;
		newClip.Set(clipper);
		owner->ConstrainClippingRegion(&newClip);
		owner->DrawString(m_StatusString, BPoint(selectRect.right + m_Parent->StatusOffset(), selectRect.bottom));
				
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

void TCDListItem::UpdateInfo()
{
	//	Update title
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
	}
}

