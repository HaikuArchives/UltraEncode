//-------------------------------------------------------------------
//
//	File:	TQueueListView.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	06.10.99
//
//	Desc:	Encoder Queue list
//
//	Copyright ©1999	Mediapede, Inc.
//
//---------------------------------------------------------------------

// Includes
#include <ScrollBar.h>
#include <ScrollView.h>

#include "DebugBuild.h"
#include "AppMessages.h"
#include "AppUtils.h"
#include "RipperApp.h"

#include "TProgressDialog.h"

#include "TQueueListView.h"

//	Constants

//-------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------
//
//

TQueueListView::TQueueListView(TProgressDialog *parent, BRect frame) : 
			BListView(frame, "QueueListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM),
			m_Parent(parent),
			m_TitleOffset(30),
			m_TimeOffset(325),
			m_StatusOffset(375)
{
}


//-------------------------------------------------------------------
//	Destructor
//-------------------------------------------------------------------
//
//

TQueueListView::~TQueueListView()
{
}


#pragma mark -
#pragma mark === Message Handling ===

//-------------------------------------------------------------------
//	MessageReceived
//-------------------------------------------------------------------
//
//

void TQueueListView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{						
		default:
			BListView::MessageReceived(message);
			break;			
	}
}


//---------------------------------------------------------------------
//	KeyDown
//---------------------------------------------------------------------
//
//	Handle key down event
//

void TQueueListView::KeyDown(const char *bytes, int32 numBytes)
{
	if (numBytes == 1)
	{
		switch(bytes[0])
		{	
			// Delete/Backspace key
			case B_DELETE:
			case B_BACKSPACE:
				//m_Parent->RemoveQueueItem();
				break;
				
			default:
				BListView::KeyDown(bytes, numBytes);
				break;
		
		}
	}
	else
		BListView::KeyDown(bytes, numBytes);
}


//---------------------------------------------------------------------
//	FrameResized
//---------------------------------------------------------------------
//
//

void TQueueListView::FrameResized(float width, float height)
{
	//	Calculate scroll amount
	UpdateScrollBar();
			
	BListView::FrameResized(width, height);
	
	//	Redraw selected item
	int32 curSel = CurrentSelection();
	if (curSel >= 0)
	{
		Invalidate(ItemFrame(curSel));
	}	
}


//---------------------------------------------------------------------
//	AttachedToWindow
//---------------------------------------------------------------------
//
//	Update scroll bar
//

void TQueueListView::AttachedToWindow()
{
	//	Calculate scroll amount
	UpdateScrollBar();
	
	BListView::AttachedToWindow();
}

#pragma mark -
#pragma mark === List Handling ===

//---------------------------------------------------------------------
//	AddItem
//---------------------------------------------------------------------
//
//	Add item and resize list view as neccessary
//

bool TQueueListView::AddItem(BListItem *item)
{	
	//FUNCTION("TQueueListView::AddItem()\n");
	
	//	Call parent
	bool retVal = BListView::AddItem(item);
	
	//	Calculate scroll amount
	UpdateScrollBar();
	
	return retVal;
}


//---------------------------------------------------------------------
//	AddItem
//---------------------------------------------------------------------
//
//	Add item and resize list view as neccessary
//

bool TQueueListView::AddItem(BListItem *item, int32 index)
{	
	//FUNCTION("TQueueListView::AddItem()\n");
	
	//	Call parent
	bool retVal = BListView::AddItem(item);
	
	//	Calculate scroll amount
	UpdateScrollBar();
	return retVal;
}


//---------------------------------------------------------------------
//	RemoveItem
//---------------------------------------------------------------------
//
//	Remove item and resize list view as neccessary
//

bool TQueueListView::RemoveItem(BListItem *item)
{
	//	Call parent
	bool retVal = BListView::RemoveItem(item);
	
	//	Calculate scroll amount
	UpdateScrollBar();
	
	return retVal;
}

//---------------------------------------------------------------------
//	RemoveItem
//---------------------------------------------------------------------
//
//	Remove item and resize list view as neccessary
//

BListItem *TQueueListView::RemoveItem(int32 index)
{
	//	Call parent
	BListItem *item = BListView::RemoveItem(index);
	
	//	Calculate scroll amount
	UpdateScrollBar();
	
	return item;
}


//---------------------------------------------------------------------
//	DeleteSelectedItems
//---------------------------------------------------------------------
//
//	Remove selected items from process list
//

void TQueueListView::DeleteSelectedItems()
{
 	BListItem *item; 
	int32 selected; 
	while ( (selected = CurrentSelection()) >= 0 ) 
   	{ 
		item = ItemAt(selected);
		if (item)
		{
			//	Remove item from list view if it not being processed
			RemoveItem(item);
			
			//	Remove item from process list
			//ProcessList()
		}
   }
}


#pragma mark -
#pragma mark === Dialog Routines ===

//---------------------------------------------------------------------
//	UpdateScrollBar
//---------------------------------------------------------------------
//
//

void TQueueListView::UpdateScrollBar()
{
	TQueueListView *scrollView  = m_Parent->EncoderListView();
	BScrollBar *scrollBar 		= m_Parent->EncoderScrollBar();
	
	const BRect scrollRect 	= scrollView->Bounds();	
	float totalHeight = (CountItems() + 2) * ItemFrame(0).Height();
	
	AdjustScrollBar(scrollBar, scrollRect.Height(), 5.0, totalHeight, scrollRect.top);
		
}
