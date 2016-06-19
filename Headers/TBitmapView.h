//-------------------------------------------------------------------
//
//	File:	TBitmapView.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	11.02.97
//
//-------------------------------------------------------------------

#ifndef __TBITMAPVIEW_H__
#define __TBITMAPVIEW_H__

#include <Bitmap.h>
#include <View.h>

class TBitmapView : public BView
{	
	public:
		TBitmapView( BRect area, const char *name, BBitmap *bitmap, bool own, uint32 resizeFlags = B_FOLLOW_NONE, bool eventsToParent = false);
			// A version where ownership is clear: we don't
		TBitmapView( BRect area, const char *name, const BBitmap *bitmap, uint32 resizeFlags, bool eventsToParent);
		virtual ~TBitmapView();
				
		virtual void MouseDown(BPoint where);
		virtual void MouseUp(BPoint where);
		virtual void MouseMoved(BPoint where, uint32 code, const BMessage *a_message);

		virtual void Draw( BRect area);
					
	private:
		BBitmap *m_Bitmap;
		bool	m_Ownership;
		bool	m_EventsToParent;
					
};

#endif
