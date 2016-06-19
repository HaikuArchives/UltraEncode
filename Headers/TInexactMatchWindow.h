//-------------------------------------------------------------------
//
//	File:	TInexactMatchWindow.h
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

#ifndef __TINEXACT_MATCH_WINDOW_H__
#define __TINEXACT_MATCH_WINDOW_H__


//	Constants
#define ADD_CD				'adcd'
#define MATCH_SELECT_MSG	'maSE'
#define MATCH_INVOKE_MSG	'maIN'

//	Includes
#include <Window.h>

//	Class Definition
class TInexactMatchWindow : public BWindow 
{
	public:
			TInexactMatchWindow(BMessage *message); 
			void	MessageReceived(BMessage *msg);

	private:
		void	Save();
		
		//	Member variables
		BScrollView		*m_MatchScrollView;
		BListView		*m_MatchListView;
		BButton			*m_OKButton;
		BButton			*m_CancelButton;
		BMessenger		replyTo;
		
		
	
};

#endif
