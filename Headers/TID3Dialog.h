//---------------------------------------------------------------------
//
//	File:	TID3Dialog.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.14.99
//
//
//	Copyright ©1999 mediapede Software
//
//---------------------------------------------------------------------

#ifndef __TID3DIALOG_H__
#define __TID3DIALOG_H__

//	Includes
#include <Window.h>
#include "id3tag.h"

//	Forward declarations
class TProgressDialog;
class TCDListItem;

//	Class Definition
class TID3Dialog : public BWindow
{
	public:
		TID3Dialog(TProgressDialog *parent, TCDListItem *item, ID3_tag *id3Tag, BMessage *message);
		~TID3Dialog();
		
		void	MessageReceived(BMessage* message);
		
		bool 	QuitRequested();
		void 	Quit();
		
	private:
	
	protected:
		bool Init();
		
		TProgressDialog *m_Parent;
		TCDListItem 	*m_Item;
		
		BTextControl	*m_TitleText;
		BTextControl	*m_ArtistText;
		BTextControl	*m_AlbumText;
		BTextControl	*m_TrackText;
		BTextControl	*m_YearText;
		BTextControl	*m_CommentsText;		
		BMenuField		*m_GenreMenu;
		
		ID3_tag			*m_Tag;
				
};

#endif