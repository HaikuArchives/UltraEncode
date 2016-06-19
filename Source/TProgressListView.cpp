//-------------------------------------------------------------------
//
//	File:	TProgressListView.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.14.99
//
//	Desc:	Progress Queue list
//
//	Copyright ©1999	Mediapede, Inc.
//
//---------------------------------------------------------------------

// Includes
#include <ScrollView.h>

#include "DebugBuild.h"
#include "AppConstants.h"
#include "AppMessages.h"
#include "AppUtils.h"
#include "ResourceManager.h"

#include "RipperApp.h"
#include "TProgressDialog.h"
#include "TCDListItem.h"
#include "TID3Dialog.h"

#include "TProgressListView.h"

//	Constants

//-------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------
//
//

TProgressListView::TProgressListView(TProgressDialog *parent, BRect frame) : 
				    BListView(frame, "ProgressListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM),
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

TProgressListView::~TProgressListView()
{
}


#pragma mark -
#pragma mark === Message Handling ===

//-------------------------------------------------------------------
//	MessageReceived
//-------------------------------------------------------------------
//
//

void TProgressListView::MessageReceived(BMessage *message)
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

void TProgressListView::KeyDown(const char *bytes, int32 numBytes)
{
	//FUNCTION("TProgressListView::KeyDown()\n");
	
	if (numBytes == 1)
	{
		switch(bytes[0])
		{	
			// Delete/Backspace key
			//case B_DELETE:
			//case B_BACKSPACE:
			//	DeleteSelectedItems();
			//	break;
			
			//	Enter opens ID3 dialog of selected item
			case B_ENTER:
				{
					int32 curSel = CurrentSelection();
					if (curSel >= 0)
					{
						TCDListItem *item = (TCDListItem *)ItemAt(curSel);
						if (item)
						{
							if (item->EditMode())
							{
								//	Take out of edit mode
								item->EditMode(false);
								InvalidateItem(curSel);	
							}
							else
							{
								ShowID3Dialog();
							}
						}
					}
				}
				break;
				
			case B_UP_ARROW:
			case B_DOWN_ARROW:
				{
					int32 curSel = CurrentSelection();
					if (curSel >= 0)
					{
						TCDListItem *item = (TCDListItem *)ItemAt(curSel);												
						if (item && item->EditMode())
						{
						}						
						//	Pass to parent
						else
						{
							BListView::KeyDown(bytes, numBytes);
						}
					}
				}
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
//	MouseDown
//---------------------------------------------------------------------
//
//	Handle mouse down event
//

void TProgressListView::MouseDown(BPoint where)
{
	//	Do nothing if we are extracting
	if (m_Parent->Extracting())
		return;
	
	//	Set focus to list view
	MakeFocus(true);
	
	//	Check for double click
	int32  clicks  = 0;
	uint32 buttons = 0;				
	BMessage *message = Window()->CurrentMessage();	
	message->FindInt32("clicks", &clicks);
	message->FindInt32("buttons", (int32 *)&buttons);
	
	//	Find item at click point
	int32 index = IndexOf(where);
	TCDListItem *item = (TCDListItem *)ItemAt(index);
	if (item)
	{
		//	Select
		if (item->IsSelected() == false)
		{
			//	Deselect and redraw last selected item
			int32 curSel = CurrentSelection();
			if (curSel >= 0)
			{
				TCDListItem *lastItem = (TCDListItem *)ItemAt(curSel);
				if (lastItem)
				{
					lastItem->EditMode(false);
					Deselect(curSel);
				}
			}
						
			//	Select item
			Select(index);
		}
		else
		{
			//	Put in edit mode if not double click
			if (clicks < 2)
			{
				//	Check and see if click was in text area
				BRect textRect = ItemFrame(index);
				textRect.left  =  TitleOffset();
				textRect.right =  TimeOffset();
				if (textRect.Contains(where))
				{
					if (item->EditMode())
					{
					
					}
					//	Put in edit mode
					else
					{
						item->EditMode(true);
						InvalidateItem(index);
					}
				}
			}
		}
		
		//	Check and see if click ocurred in click rect
		BRect frame = ItemFrame(index);
		frame.right = frame.left + 15;
		
		if (frame.Contains(where))
		{
			//	Toggle selection state
			if (item->Selected())
			{
				item->Selected(false);
				item->Pending(false);
				item->StatusString("");
			}
			else
			{
				item->Selected(true);
				item->Pending(true);
				item->StatusString("Pending");				
			}
			
			//	Set extract flag to new state
			track_info *albumData = (track_info *)m_Parent->TitleList()->ItemAt(index + 1);
			if (albumData)
				albumData->extract = item->Selected();
				
			//	Redraw
			InvalidateItem(index);
		}
	}
	
	//	Double click
	if ( (clicks == 2) && ValidDoubleClick(where, m_LastClick))
	{		
		switch(buttons)
		{
			//	Open ID3 dialog
			case B_PRIMARY_MOUSE_BUTTON:
				{		
					//	Take out of edit mode
					item->EditMode(false);
					InvalidateItem(index);
					ShowID3Dialog();
				}
				break;
			
			//	Edit name
			default:
				{
					item->EditMode(true);
					InvalidateItem(index);
				}
				break;
		}
	}
}


//---------------------------------------------------------------------
//	MouseUp
//---------------------------------------------------------------------
//
//	Handle mouse up event
//

void TProgressListView::MouseUp(BPoint where)
{
	//	Save click location for later compare
	m_LastClick = where;
}


//---------------------------------------------------------------------
//	FrameResized
//---------------------------------------------------------------------
//
//

void TProgressListView::FrameResized(float width, float height)
{
	//FUNCTION("TProgressListView::FrameResized()\n");
	
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

void TProgressListView::AttachedToWindow()
{
	//	Calculate scroll amount
	UpdateScrollBar();
	
	BListView::AttachedToWindow();
}

//---------------------------------------------------------------------
//	Draw
//---------------------------------------------------------------------
//
//

void TProgressListView::Draw(BRect updateRect)
{
	//	Parent
	BListView::Draw(updateRect);
}


#pragma mark -
#pragma mark === List Handling ===

//---------------------------------------------------------------------
//	AddItem
//---------------------------------------------------------------------
//
//	Add item and resize list view as neccessary
//

bool TProgressListView::AddItem(BListItem *item)
{	
	//FUNCTION("TProgressListView::AddItem()\n");
	
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

bool TProgressListView::AddItem(BListItem *item, int32 index)
{	
	//FUNCTION("TProgressListView::AddItem()\n");
	
	//	Call parent
	bool retVal = BListView::AddItem(item, index);
	
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

bool TProgressListView::RemoveItem(BListItem *item)
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

BListItem *TProgressListView::RemoveItem(int32 index)
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

void TProgressListView::DeleteSelectedItems()
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
//	ShowID3Dialog
//---------------------------------------------------------------------
//
//	Load and diaply ID3 settings dialog
//

void TProgressListView::ShowID3Dialog()
{
	//	Get current selected item
	int32 curSel = CurrentSelection();
	if (curSel >= 0)
	{
		TCDListItem *item = (TCDListItem *)ItemAt(curSel);
		if(item)
		{
			//	Show dialog if created
			if (item->Dialog())
			{
				item->Dialog()->Show();
				item->Dialog()->Activate();
			}
			//	Create new dialog
			else
			{
				// Create the status dialog from a resource archive
				BMessage *theMessage = GetWindowFromResource("ID3Dialog");
				TID3Dialog *theDialog = new TID3Dialog(m_Parent, item, item->ID3Tag(), theMessage);
				ASSERT(theDialog);
				delete(theMessage);
				
				//	Set ownership to item
				item->SetDialog(theDialog);
				
				// Center it
				CenterWindow(theDialog);
				
				// Show the dialog
				theDialog->Show();
			}			
			
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

void TProgressListView::UpdateScrollBar()
{
	TProgressListView *scrollView   = m_Parent->TracksListView();
	BScrollBar *scrollBar 			= m_Parent->ExtractScrollBar();
	
	const BRect scrollRect 	= scrollView->Bounds();	
	float totalHeight = (CountItems() + 2) * ItemFrame(0).Height();
	
	AdjustScrollBar(scrollBar, scrollRect.Height(), 5.0, totalHeight, scrollRect.top);
		
}
