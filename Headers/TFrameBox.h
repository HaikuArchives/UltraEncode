//-------------------------------------------------------------------
//
//	File:	TFrameBox.h
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

#ifndef __TFRAMEBOX_H__
#define __TFRAMEBOX_H__

// Class Definition
#include <Box.h>

class TFrameBox : public BBox
{
	public:
		// Member Functions
		TFrameBox(BRect rect, const char *title);
		
		void	Draw(BRect updateRect);
		
		inline void Bevel(BRect rect){ m_BevelRect = rect; }	
	private:
	
	protected:
		BRect 	m_BevelRect;
		
};

#endif
