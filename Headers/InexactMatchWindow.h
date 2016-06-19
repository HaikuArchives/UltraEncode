/*
	File:	InexactMatchWindow.h
	Author:	Chip Paul (cpaul@resgen.com)
	Function:
		This window is popped up on the screen when a 211 CDDB Code
		"inexact match" is returned.  It contains a list of the 
		possible matches and allows the user to choose which one
		they think is the best match.
*/
#ifndef INEXACT_MATCH_WINDOW_H
#define INEXACT_MATCH_WINDOW_H

#define SAVE_B		'save'
#define CANCEL_B	'cncl'
#define ADD_CD		'adcd'

#include <Window.h>

class InexactMatchWindow : public BWindow {
private:
	BScrollView		*aView;
	BStringView	*titleString;
	BListView	*cdList;
	BButton		*btnSave;
	BButton		*btnNoMatch;
	BMessenger	replyTo;
	void Save();
	
public:
			InexactMatchWindow(BRect frame); 
	void	MessageReceived(BMessage *msg);
};

#endif
