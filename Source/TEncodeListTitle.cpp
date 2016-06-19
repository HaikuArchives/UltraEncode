//---------------------------------------------------------------------
//
//	File:	TEncodeListTitle.cpp
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

#include "TQueueListView.h"

#include "TEncodeListTitle.h"

// Constants

//---------------------------------------------------------------------
//	Constructor
//---------------------------------------------------------------------
//
//

TEncodeListTitle::TEncodeListTitle(TQueueListView *parent, BRect bounds) : 
					BView(bounds, "EncodeTitleListView", B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP , B_WILL_DRAW|B_FRAME_EVENTS),
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

TEncodeListTitle::~TEncodeListTitle()
{
}


//---------------------------------------------------------------------
//	Init
//---------------------------------------------------------------------
//
//	Perform default initialization tasks

void TEncodeListTitle::Init()
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

void TEncodeListTitle::MessageReceived(BMessage* message)
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

void TEncodeListTitle::Draw(BRect updateRect)
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

void TEncodeListTitle::FrameResized(float width, float height)
{
	//FUNCTION("TEncodeListTitle::FrameResized()\n");
	
	//	Force redraw
	Invalidate();
	
	BView::FrameResized(width, height);	
}
