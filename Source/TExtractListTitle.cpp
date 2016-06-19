//---------------------------------------------------------------------
//
//	File:	TExtractListTitle.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.17.99
//
//	Desc:	Draw extract title items
//
//	Copyright ©1998 mediapede Software
//
//---------------------------------------------------------------------

// Includes
#include <app/Application.h>
#include <support/Debug.h>

#include "AppConstants.h"
#include "AppMessages.h"
#include "AppUtils.h"
#include "ResourceManager.h"

#include "TProgressDialog.h"
#include "TProgressListView.h"

#include "TExtractListTitle.h"

// Constants

//---------------------------------------------------------------------
//	Constructor
//---------------------------------------------------------------------
//
//

TExtractListTitle::TExtractListTitle(TProgressListView *parent, BRect bounds) : 
					BView(bounds, "TitleListView", B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP , B_WILL_DRAW|B_FRAME_EVENTS),
					m_Parent(parent)
{
	// Perform default initialization
	Init();
}


//---------------------------------------------------------------------
//	Destructor
//---------------------------------------------------------------------
//
//

TExtractListTitle::~TExtractListTitle()
{
}


//---------------------------------------------------------------------
//	Init
//---------------------------------------------------------------------
//
//	Perform default initialization tasks

void TExtractListTitle::Init()
{
	// We don't need a background color
	SetViewColor(B_TRANSPARENT_32_BIT);
					
}

#pragma mark -
#pragma mark === Message Handling ===

//------------------------------------------------------------------
//	MessageReceived
//------------------------------------------------------------------
//
//

void TExtractListTitle::MessageReceived(BMessage* message)
{	
	
	switch (message->what)
	{
			
		default:
			BView::MessageReceived(message);
			break;
	}

}	


#pragma mark -
#pragma mark === Drawing Routines ===

//---------------------------------------------------------------------
//	Draw
//---------------------------------------------------------------------
//
//

void TExtractListTitle::Draw(BRect updateRect)
{
	BPoint startPt, endPt;
		
	BRect bounds = Bounds();
	
	// Fill
	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	FillRect(bounds);
	
	//	Frame
	rgb_color frameShadow = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_4_TINT);
	SetHighColor(frameShadow);
	StrokeRect(bounds);
	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	startPt.Set(bounds.right, bounds.bottom-1);
	endPt.Set(bounds.right, bounds.top+1);
	StrokeLine(startPt, endPt);
	
	//	Highlight
	SetHighColor(kWhite);
	startPt.Set(bounds.left+1, bounds.bottom-1);
	endPt.Set(bounds.left+1, bounds.top+1);
	StrokeLine(startPt, endPt);
	startPt.Set(bounds.left+1, bounds.top+1);
	endPt.Set(bounds.right, bounds.top+1);
	StrokeLine(startPt, endPt);
	
	//	Shadow
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));
	startPt.Set(bounds.right, bounds.bottom-1);
	endPt.Set(bounds.left+2, bounds.bottom-1);
	StrokeLine(startPt, endPt);
	
	//	Draw title line
	int32 start = m_Parent->TitleOffset();
	SetHighColor(frameShadow);
	startPt.Set(bounds.left + start, bounds.top+1);	
	endPt.Set(bounds.left + start, bounds.bottom-1);
	StrokeLine(startPt, endPt);
	start++;
	startPt.Set(bounds.left + start, bounds.top+1);	
	endPt.Set(bounds.left + start, bounds.bottom-1);
	StrokeLine(startPt, endPt);
	SetHighColor(kWhite);
	start++;
	startPt.Set(bounds.left + start, bounds.top+1);	
	endPt.Set(bounds.left + start, bounds.bottom-1);
	StrokeLine(startPt, endPt);
	
	//	Draw "Title" String
	SetHighColor(kBlack);
	start += 10;
	DrawString("Title", BPoint(start, bounds.bottom - 3));
	
	//	Draw time line
	start = m_Parent->TimeOffset();
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));
	startPt.Set(bounds.left + start, bounds.top+1);	
	endPt.Set(bounds.left + start, bounds.bottom-1);
	StrokeLine(startPt, endPt);
	start++;
	SetHighColor(frameShadow);
	startPt.Set(bounds.left + start, bounds.top+1);	
	endPt.Set(bounds.left + start, bounds.bottom-1);
	StrokeLine(startPt, endPt);
	SetHighColor(kWhite);
	start++;
	startPt.Set(bounds.left + start, bounds.top+1);	
	endPt.Set(bounds.left + start, bounds.bottom-1);
	StrokeLine(startPt, endPt);
	
	//	Draw "Time" String
	SetHighColor(kBlack);
	start += 10;
	DrawString("Time", BPoint(start, bounds.bottom - 3));
	
	//	Draw status line
	start = m_Parent->StatusOffset();
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));
	startPt.Set(bounds.left + start, bounds.top+1);	
	endPt.Set(bounds.left + start, bounds.bottom-1);
	StrokeLine(startPt, endPt);
	start++;
	SetHighColor(frameShadow);
	startPt.Set(bounds.left + start, bounds.top+1);	
	endPt.Set(bounds.left + start, bounds.bottom-1);
	StrokeLine(startPt, endPt);
	SetHighColor(kWhite);
	start++;
	startPt.Set(bounds.left + start, bounds.top+1);	
	endPt.Set(bounds.left + start, bounds.bottom-1);
	StrokeLine(startPt, endPt);
	
	//	Draw "Status" String
	SetHighColor(kBlack);
	start += 10;
	DrawString("Status", BPoint(start, bounds.bottom - 3));


}


//---------------------------------------------------------------------
//	FrameResized
//---------------------------------------------------------------------
//
//

void TExtractListTitle::FrameResized(float width, float height)
{
	//FUNCTION("TExtractListTitle::FrameResized()\n");
	
	//	Force redraw
	Invalidate();
	
	BView::FrameResized(width, height);	
}


#pragma mark -
#pragma mark === MouseHandling ===

//---------------------------------------------------------------------
//	MouseDown
//---------------------------------------------------------------------
//
//	Handle mouse down event
//

void TExtractListTitle::MouseDown(BPoint where)
{
	//	Do nothing if we are extracting
	if (m_Parent->GetParent()->Extracting())
		return;
	
	//	Check for double click
	//int32 clicks = 0;
	//BMessage *message = Window()->CurrentMessage();	
	//message->FindInt32("clicks", &clicks);
	
	//	Check for button
	BPoint mousePt;
	uint32 buttons;
	GetMouse(&mousePt, &buttons);
	
	switch(buttons)
	{
		case B_PRIMARY_MOUSE_BUTTON:
			m_Parent->GetParent()->SelectAllExtractorItems();
			break;
			
		case B_SECONDARY_MOUSE_BUTTON:
			m_Parent->GetParent()->DeselectAllExtractorItems();
			break;
	
		default:
			break;
	}
	
}
