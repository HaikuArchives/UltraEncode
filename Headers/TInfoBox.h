//-------------------------------------------------------------------
//
//	File:	TInfoBox.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.20.99
//
//	Desc:	Custom BBox
//
//	Copyright ©1999 mediapede Software
//
//---------------------------------------------------------------------

#ifndef __TINFOBOX_H__
#define __TINFOBOX_H__

// Class Definition
#include <Box.h>

class TInfoBox : public BBox
{
	public:
		// Member Functions
		TInfoBox(TProgressDialog *parent, BRect rect, const char *title);
		
		void	MouseDown(BPoint where);
		
		void 	ShowCDInfo();
		
	private:
	
	protected:
		TProgressDialog	*m_Parent;


};

#endif
