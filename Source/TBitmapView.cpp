//-------------------------------------------------------------------
//
//	File:	TBitmapView.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	11.02.97
//
//-------------------------------------------------------------------

// Includes
#include "TBitmapView.h"

#include <Window.h>



//-------------------------------------------------------------------
//
//	Function:	Constructor
//
//	Desc:		
//
//-------------------------------------------------------------------
//
//

TBitmapView::TBitmapView(BRect area, const char *name, BBitmap *bitmap, 
		bool own, uint32 resizeFlags, bool eventsToParent) : 
	BView(area, name, resizeFlags, B_WILL_DRAW),
	m_EventsToParent(eventsToParent),
	m_Bitmap(bitmap),
	m_Ownership(own)
{ 
	// We don't need a background color
	SetViewColor(B_TRANSPARENT_32_BIT);
}

TBitmapView::TBitmapView(BRect area, const char *name, const BBitmap *bitmap, 
		uint32 resizeFlags, bool eventsToParent) : 
	BView(area, name, resizeFlags, B_WILL_DRAW),
	m_EventsToParent(eventsToParent),
	// NOTE: this is safe, since the class doesn't expect to own the bitmap
	// and everything else done in m_Bitmap is safely 'const'.
	m_Bitmap(const_cast<BBitmap*>(bitmap)),
	m_Ownership(false)
{ 
	// We don't need a background color
	SetViewColor(B_TRANSPARENT_32_BIT);
}

//-------------------------------------------------------------------
//	~TBitmapView
//-------------------------------------------------------------------
//
//

TBitmapView::~TBitmapView() 
{
	if (m_Bitmap)
	{
		if (m_Ownership)
			delete m_Bitmap; 
	}
}

#pragma mark -
#pragma mark === Mouse Handling ===

//-------------------------------------------------------------------
//	MouseDown
//-------------------------------------------------------------------
//
//

void TBitmapView::MouseDown(BPoint mousePt)
{
	if (m_EventsToParent)
	{
		if (LockLooper())
		{
			//	Convert to parent
			BPoint newPt = ConvertToParent(mousePt);
			
			if (Parent())
				Parent()->MouseDown(newPt);
			else
				Window()->PostMessage(B_MOUSE_DOWN);
			UnlockLooper();
		}	
	}
}


//-------------------------------------------------------------------
//	MouseUp
//-------------------------------------------------------------------
//
//

void TBitmapView::MouseUp(BPoint mousePt)
{
	if (m_EventsToParent)
	{
		if (LockLooper())
		{
			//	Convert to parent
			BPoint newPt = ConvertToParent(mousePt);
			if (Parent())
				Parent()->MouseUp(newPt);
			UnlockLooper();
		}	
	}
}


//-------------------------------------------------------------------
//	MouseUp
//-------------------------------------------------------------------
//
//

void TBitmapView::MouseMoved(BPoint mousePt, uint32 code, const BMessage *a_message)
{
	if (m_EventsToParent)
	{
		if (LockLooper())
		{
			//	Convert to parent
			BPoint newPt = ConvertToParent(mousePt);
			if (Parent())
				Parent()->MouseMoved(newPt, code, a_message);
			UnlockLooper();
		}	
	}
}


#pragma mark -
#pragma mark === Drawing Routines ===

//-------------------------------------------------------------------
//	Draw
//-------------------------------------------------------------------
//
//	Draws the bitmap into the view
//

void TBitmapView::Draw( BRect area)
{
	if (m_Bitmap)
		DrawBitmap(m_Bitmap, m_Bitmap->Bounds(), Bounds());
};
