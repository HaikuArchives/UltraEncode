//---------------------------------------------------------------------
//
//	File:	TExtractListView.cpp
//
//	Author:	Mike Ost
//
//	Date:	08.11.98
//
//	Desc:	Sub-class of TElementsView. Handles the tab labelled 'Media'
//
//	Copyright ©1998 mediapede Software
//
//---------------------------------------------------------------------

// Includes
#include <ctype.h>
#include <assert.h>				// For assert()
#include <Region.h>
#include <ScrollBar.h>
#include <ScrollView.h>

#include "AppConstants.h"		// for kScrollHeight
#include "AppUtils.h"			// IsOptionKeyDown(), etc.
#include "DebugBuild.h"
#include "ResourceManager.h"	// for GetIcon16FromResource

#include "TProgressDialog.h"

#include "TExtractListView.h"

// Local constants
#define MEDIA_TAB_LIST_VIEW_MSG			'mtLV'
#define MEDIA_TAB_THUMBNAIL_VIEW_MSG	'mtTH'
#define MEDIA_TAB_ICON_VIEW_MSG			'mtIC'

		
//---------------------------------------------------------------------
//	Constructor
//---------------------------------------------------------------------
//
//

TExtractListView::TExtractListView(TProgressDialog *parent, BRect bounds, BScrollView **containerView) :
			   		ColumnListView( bounds, containerView, "ExtractList", B_FOLLOW_ALL_SIDES,
								B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST,
								false, true, true, B_NO_BORDER),
					m_Parent(parent)
{	 
	Init();
}


//---------------------------------------------------------------------
//	Destructor
//---------------------------------------------------------------------
//
//

TExtractListView::~TExtractListView()
{
}

//---------------------------------------------------------------------
//	Init
//---------------------------------------------------------------------
//
//

void TExtractListView::Init()
{
	//	Item Number and checkbox
	AddColumn( new CLVColumn( NULL, 30.0, CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));
	
	//	Title
	AddColumn( new CLVColumn("Title", 108.0, CLV_SORT_KEYABLE, 50.0));
	
	//	Duration
	AddColumn( new CLVColumn("Duration", 131.0, CLV_SORT_KEYABLE));
	
	//	Progress
	AddColumn( new CLVColumn("Progress", 180.0, CLV_SORT_KEYABLE));
		
	//	Set our sorting function
	//SetSortFunction((CLVCompareFuncPtr)TExtractListItem::MyCompare);
		
}


#pragma mark -
#pragma mark === Message Handling ===

//---------------------------------------------------------------------
//	MessageReceived
//---------------------------------------------------------------------
//
//

void TExtractListView::MessageReceived(BMessage* message)
{

	switch (message->what)
	{
			
		default:
			BView::MessageReceived(message);
			break;

	}
}	

#pragma mark -
#pragma mark === Mouse Handling ===

//---------------------------------------------------------------------
//	MouseDown
//---------------------------------------------------------------------
//
//	Handle mouse down events
//

void TExtractListView::MouseDown(BPoint where)
{			
	FUNCTION("TExtractListView::MouseDown() \n");
	
	//	Get item clicked
	int32 itemIndex = IndexOf(where);
		
	/*
	if(itemIndex >= 0)
	{
		CLVListItem* ClickedItem = (CLVListItem*)BListView::ItemAt(ItemIndex);
		if(ClickedItem->fSuperItem)
			if(ClickedItem->fExpanderButtonRect.Contains(point))
			{
				if(ClickedItem->IsExpanded())
					Collapse(ClickedItem);
				else
					Expand(ClickedItem);
				return;
			}
	}
	*/
	
	//	Select this item
	Select(itemIndex);

	//	Wait a while for a mouse up
	snooze(100 * 1000);
	
	// Check to see which button is down
	uint32 	buttons = 0;				
	Window()->CurrentMessage()->FindInt32("buttons", (long *)&buttons);
	
	// Is this a double click?  If so, open editor
	uint32 		type;
	int32		count = 0;
	BMessage 	*message = Window()->CurrentMessage();
	
	// Determine which button has been clicked
	switch(buttons)
	{
		case B_PRIMARY_MOUSE_BUTTON:
			if (B_OK == message->GetInfo("clicks", &type, &count) )
			{
				int32 clickCount = message->FindInt32("clicks", count-1);				
				if (clickCount == 2)
				{
				}
				else
				{
					// If shift key is down the user is group selecting/deselecting
					if (IsShiftKeyDown())
					{
					}
					// Otherwise, select the item
					else
					{			
						//BListView::MouseDown(where);
						InitiateDrag(where, itemIndex, true);
					}			
				}		
			}
			break;
				
		// If the second mouse button is down, open cue menu
		case B_SECONDARY_MOUSE_BUTTON:				
			break;

		default:
			break;
			
	}		
}


//---------------------------------------------------------------------
//	KeyDown
//---------------------------------------------------------------------
//
//	Handle key down event
//

void TExtractListView::KeyDown(const char *bytes, int32 numBytes)
{
	if (numBytes == 1)
	{
		switch(bytes[0])
		{	
			default:
				ColumnListView::KeyDown(bytes, numBytes);
				break;
		}
	}
	else
		ColumnListView::KeyDown(bytes, numBytes);
}


#pragma mark -
#pragma mark === Resizing Routines ===

//---------------------------------------------------------------------
//	FrameResized
//---------------------------------------------------------------------
//
//

void TExtractListView::FrameResized(float width, float height)
{
	FUNCTION("TExtractListView::FrameResized()\n");
	
	//	Adjust scrollbar
	BScrollView *scrollView = m_Parent->ScrollView();
	BScrollBar *scrollBar 	= scrollView->ScrollBar(B_VERTICAL);
	const BRect scrollRect 	= scrollView->Bounds();
	const BRect bounds 		= Bounds();
	
	AdjustScrollBar(scrollBar, scrollRect.Height(), 5.0, bounds.Height(), scrollRect.top);
		
	BListView::FrameResized(width, height);	
}

//---------------------------------------------------------------------
//	AttachedToWindow
//---------------------------------------------------------------------
//
//	Update scroll bar
//

void TExtractListView::AttachedToWindow()
{
	//	Adjust scrollbar
	BScrollView *scrollView = m_Parent->ScrollView();
	BScrollBar *scrollBar 	= scrollView->ScrollBar(B_VERTICAL);
	const BRect scrollRect 	= scrollView->Bounds();
	const BRect bounds 		= Bounds();
	
	AdjustScrollBar(scrollBar, scrollRect.Height(), 5.0, bounds.Height(), scrollRect.top);
	
	BListView::AttachedToWindow();
}


#pragma mark -
#pragma mark === Dragging Routines ===

//---------------------------------------------------------------------
//	InitiateDrag
//---------------------------------------------------------------------
//
//

bool TExtractListView::InitiateDrag(BPoint point, int32 index, bool wasSelected)
{	
	/*
	// Exit if item not selected
	if (wasSelected == false)
		return false;
			
	TExtractListItem *theItem = static_cast<TExtractListItem *>(ItemAt(index));
	if (theItem)
	{					
		// 	Get bitmap based on item type.
			
		//	Make bitmap transparent
		BBitmap *dragBitmap = MakeTransparentBitmap(theItem->LargeIcon(), 170);
		ASSERT(dragBitmap);
		
		// Center mouse over icon
		BPoint centerPt( theItem->LargeIcon()->Bounds().Width() / 2, theItem->LargeIcon()->Bounds().Height() / 2 );

		//	Start drag
		BMessage message(BROWSER_ICON_DRAG_MSG);
		message.AddRef("FileRef", theItem->EntryRef());	
		DragMessage(&message, dragBitmap, B_OP_ALPHA, centerPt);
				
		return true;
	}
	*/
	
	return false;	
}


#pragma mark -
#pragma mark === TExtractListItem Class ===


//------------------------------------------------------------------
//	Constructor
//------------------------------------------------------------------
//
//

TExtractListItem::TExtractListItem(int32 id, const char* text0, const char* text1, 
					 			   const char* text2, const char* text3) : 
					CLVListItem(0, false, true, 20.0),
					m_ID(id),
					m_Selected(true),
					m_Locked(false),
					m_Dialog(NULL)
{
	//	Setup text elements
	m_Text[0] = new char[strlen(text0)+1];
	strcpy(m_Text[0],text0);
	m_Text[1] = new char[strlen(text1)+1];
	strcpy(m_Text[1],text1);
	m_Text[2] = new char[strlen(text2)+1];
	strcpy(m_Text[2],text2);
	m_Text[3] = new char[strlen(text3)+1];
	strcpy(m_Text[3],text3);
}


TExtractListItem::~TExtractListItem()
{
	//	Free text
	for(int Counter = 0; Counter <= 4; Counter++)
		delete[] m_Text[Counter];
	
	//	Close ID3 dialog
	if (m_Dialog)
	{
		m_Dialog->Lock();
		m_Dialog->Quit();
		m_Dialog = NULL;
	}
	
}


void TExtractListItem::DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete)
{
	rgb_color color;
	bool selected = IsSelected();
	
	if(selected)
		color = BeListSelectGrey;
	else
		color = White;
		
	owner->SetHighColor(color);
	owner->SetLowColor(color);
	if(complete || selected)
		owner->FillRect(item_column_rect);
	BRegion Region;
	Region.Include(item_column_rect);
	owner->ConstrainClippingRegion(&Region);
	owner->SetDrawingMode(B_OP_OVER);
	if(column_index == 0)
	{
		item_column_rect.left += 2.0;
		item_column_rect.right = item_column_rect.left + 15.0;
		item_column_rect.top += 2.0;
		if(Height() > 20.0)
			item_column_rect.top += ceil((Height()-20.0)/2.0);
		item_column_rect.bottom = item_column_rect.top + 15.0;
	}
	else if(column_index >= 1)
	{
		owner->SetHighColor(Black);
		owner->DrawString(m_Text[column_index-1],
			BPoint(item_column_rect.left+2.0,item_column_rect.top + m_TextOffset));
	}
	owner->SetDrawingMode(B_OP_COPY);
	owner->ConstrainClippingRegion(NULL);
}


void TExtractListItem::Update(BView *owner, const BFont *font)
{
	CLVListItem::Update(owner,font);
	font_height FontAttributes;
	be_plain_font->GetHeight(&FontAttributes);
	float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);
	m_TextOffset = ceil(FontAttributes.ascent) + (Height()-FontHeight)/2.0;
}

int TExtractListItem::MyCompare(const TExtractListItem *Item1, const TExtractListItem *Item2, int32 KeyColumn)
{
	char* Temp = Item1->m_Text[KeyColumn-1];
	char* Text1 = new char[strlen(Temp)+1];
	
	for(int Counter = 0; Counter <= (int) strlen(Temp); Counter++)
		Text1[Counter] = tolower(Temp[Counter]);
		
	Temp = Item2->m_Text[KeyColumn-1];
	char* Text2 = new char[strlen(Temp)+1];
	
	for(int Counter = 0; Counter <= (int) strlen(Temp); Counter++)
		Text2[Counter] = tolower(Temp[Counter]);
		
	int Result = strcmp(Text1,Text2);
	delete[] Text1;
	delete[] Text2;
	
	if(Result <= -1)
		return -1;
		
	if(Result >= 1)
		return 1;
		
	return 0;
}
