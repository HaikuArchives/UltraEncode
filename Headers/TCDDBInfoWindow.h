//-------------------------------------------------------------------
//
//	File:	TCDDBInfoWindow.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.20.99
//
//	Desc:	This window is popped up on the screen when a 211 CDDB Code
//			"inexact match" is returned.  It contains a list of the 
//			possible matches and allows the user to choose which one
//			they think is the best match.
//
//	Copyright ©1999	Mediapede, Inc.
//
//---------------------------------------------------------------------

#ifndef __TCDDBINFO_WINDOW_H__
#define __TCDDBINFO_WINDOW_H__

//	Constants

//	Forward Declarations
class TProgressDialog;

//	Includes
#include <Window.h>

//	Class Definition
class TCDDBInfoWindow : public BWindow 
{
	public:
			TCDDBInfoWindow(BMessage *archive, TProgressDialog *parent); 
			void	MessageReceived(BMessage *msg);
			bool 	QuitRequested();

	private:
		
		//	Member variables
		TProgressDialog	*m_Parent;
		BTextControl 	*m_CDDBServerString;			
	
};

#endif
