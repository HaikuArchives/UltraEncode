//---------------------------------------------------------------------
//
//	File:	TInfoDialog.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.20.99
//
//
//	Copyright ©1999 mediapede Software
//
//---------------------------------------------------------------------

#ifndef __TINFODIALOG_H__
#define __TINFODIALOG_H__

//	Includes
#include <Window.h>
#include <Message.h>

//	Forward declarations
class TProgressDialog;
class TCDListItem;

//	Class Definition
class TInfoDialog : public BWindow
{
	public:
		TInfoDialog(TProgressDialog *parent, BMessage *message);
		~TInfoDialog();
		
		void	MessageReceived(BMessage* message);
		
		void 	Quit();
		
	private:
	
	protected:
		bool Init();
		
		TProgressDialog *m_Parent;
		
		BTextControl	*m_ArtistText;
		BTextControl	*m_AlbumText;
		BTextControl	*m_YearText;
		BMenuField		*m_GenreMenu;
						
};

#endif