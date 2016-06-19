//---------------------------------------------------------------------
//
//	File:	TID3Dialog.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.14.99
//
//	Desc:	ID3 Settings Dialog
//
//	Copyright ©1999 mediapede Software
//
//---------------------------------------------------------------------

// Includes
#include <Application.h>
#include <MenuField.h>
#include <Menu.h>
#include <MenuItem.h>
#include <TextControl.h>

#include "AppConstants.h"
#include "DebugBuild.h"
#include "TProgressDialog.h"
#include "TCDListItem.h"
#include "id3tag.h"

#include "TID3Dialog.h"

//	Constants
char *kGenreTable[] = {"Blues",
                     "Classic Rock",
                     "Country",
                     "Dance",
                     "Disco",
                     "Funk",
                     "Grunge",
                     "Hip-Hop",
                     "Jazz",
                     "Metal",
                     "New Age",
                     "Oldies",
                     "Other",
                     "Pop",
                     "R&B",
                     "Rap",
                     "Reggae",
                     "Rock",
                     "Techno",
                     "Industrial",
                     "Alternative",
                     "Ska",
                     "Death Metal",
                     "Pranks",
                     "Soundtrack",
                     "Euro-Techno",
                     "Ambient",
                     "Trip-Hop",
                     "Vocal",
                     "Jazz+Funk",
                     "Fusion",
                     "Trance",
                     "Classical",
                     "Instrumental",
                     "Acid",
                     "House",
                     "Game",
                     "Sound Clip",
                     "Gospel",
                     "Noise",
                     "Alt. Rock",
                     "Bass",
                     "Soul",
                     "Punk",
                     "Space",
                     "Meditative",
                     "Instrum. Pop",
                     "Instrum. Rock",
                     "Ethnic",
                     "Gothic",
                     "Darkwave",
                     "Techno-Indust.",
                     "Electronic",
                     "Pop-Folk",
                     "Eurodance",
                     "Dream",
                     "Southern Rock",
                     "Comedy",
                     "Cult",
                     "Gangsta",
                     "Top 40",
                     "Christian Rap",
                     "Pop/Funk",
                     "Jungle",
                     "Native American",
                     "Cabaret",
                     "New Wave",
                     "Psychadelic",
                     "Rave",
                     "Showtunes",
                     "Trailer",
                     "Lo-Fi",
                     "Tribal",
                     "Acid Punk",
                     "Acid Jazz",
                     "Polka",
                     "Retro",
                     "Musical",
                     "Rock & Roll",
                     "Hard Rock",
                     "Folk",
                     "Folk/Rock",
                     "National Folk",
                     "Swing",
                     "Fusion",
                     "Bebob",
                     "Latin",
                     "Revival",
                     "Celtic",
                     "Bluegrass",
                     "Avantgarde",
                     "Gothic Rock",
                     "Progress. Rock",
                     "Psychadel. Rock",
                     "Symphonic Rock",
                     "Slow Rock",
                     "Big Band",
                     "Chorus",
                     "Easy Listening",
                     "Acoustic",
                     "Humour",
                     "Speech",
                     "Chanson",
                     "Opera",
                     "Chamber Music",
                     "Sonata",
                     "Symphony",
                     "Booty Bass",
                     "Primus",
                     "Porn Groove",
                     "Satire"
                     };

const int genre_count = 111;
	
	
//---------------------------------------------------------------------
//	Constructor
//---------------------------------------------------------------------
//
//

TID3Dialog::TID3Dialog(TProgressDialog *parent, TCDListItem *item, ID3_tag *tag, BMessage *message) :
				BWindow(message),
				m_Parent(parent),
				m_Item(item),
				m_Tag(tag)
{
	// Default initialization
	Init();	
}


//---------------------------------------------------------------------
//	Destructor
//---------------------------------------------------------------------
//
//

TID3Dialog::~TID3Dialog()
{		
}



//---------------------------------------------------------------------
//	Init
//---------------------------------------------------------------------
//
//

bool TID3Dialog::Init()
{		
	//	Find items
	m_TitleText 	= (BTextControl *)FindView("TitleTextControl");	
	m_ArtistText 	= (BTextControl *)FindView("ArtistTextControl");
	m_AlbumText 	= (BTextControl *)FindView("AlbumTextControl");
	m_TrackText 	= (BTextControl *)FindView("TrackTextControl");
	m_YearText 		= (BTextControl *)FindView("YearTextControl");
	m_CommentsText 	= (BTextControl *)FindView("CommentsTextControl");	
	m_GenreMenu 	= (BMenuField *)FindView("GenreMenuField");
			
	//	Set up default control settings	
	m_TitleText->SetText(m_Tag->songname);
	m_ArtistText->SetText(m_Tag->artist);
	m_AlbumText->SetText(m_Tag->album);
	m_CommentsText->SetText(m_Tag->comment);
	m_YearText->SetText(m_Tag->year);

	char id[4];
	sprintf(id, "%d", m_Item->ID());
	m_TrackText->SetText(id);

	//	Genre
	for (int32 index = 0; index < genre_count; index++)
	{
		BMenuItem *item = new BMenuItem(kGenreTable[index], NULL);
		m_GenreMenu->Menu()->AddItem(item);
	}
	m_GenreMenu->Menu()->ItemAt(m_Tag->genre)->SetMarked(true);
	
	//	Set menu targets
	m_GenreMenu->Menu()->SetTargetForItems(this);

	return true;
}

#pragma mark -
#pragma mark === Message Handling === 


//---------------------------------------------------------------------
//	MessageReceived
//---------------------------------------------------------------------
//
//

void TID3Dialog::MessageReceived(BMessage* message)
{
	switch (message->what)
	{
		// User pressed OK button. 
		case OK_MSG:
			{											
				//	Save settings
				strncpy(m_Tag->songname, m_TitleText->Text(), TAGLEN_SONG);				
				strncpy(m_Tag->artist, m_ArtistText->Text(), TAGLEN_ARTIST);
				strncpy(m_Tag->album, m_AlbumText->Text(), TAGLEN_ALBUM);				
				strncpy(m_Tag->year, m_YearText->Text(), TAGLEN_YEAR);
				strncpy(m_Tag->comment, m_CommentsText->Text(), TAGLEN_COMMENT);				
				
				BMenuItem *item = m_GenreMenu->Menu()->FindMarked();
				if (item)
					m_Tag->genre = m_GenreMenu->Menu()->IndexOf(item);
					
				//	Have item update settings
				m_Item->UpdateInfo();
				
				/*
				PROGRESS("%s\n", m_TitleText->Text());
				PROGRESS("%s\n", m_Tag->songname);
				
				PROGRESS("%s\n", m_ArtistText->Text());
				PROGRESS("%s\n", m_Tag->artist);
				
				PROGRESS("%s\n", m_AlbumText->Text());
				PROGRESS("%s\n", m_Tag->album);
				
				PROGRESS("%s\n", m_YearText->Text());
				PROGRESS("%s\n", m_Tag->year);
				
				PROGRESS("%s\n", m_CommentsText->Text());
				PROGRESS("%s\n", m_Tag->comment);
				*/
				
				// Close the dialog 
				Lock();
				Quit();
			}
			break;
			
		// User has canceled the dialog
		case CANCEL_MSG:
			{

				Lock();
				Quit();
			}
			break;
													
		default:
			BWindow::MessageReceived(message);						
			break;
	}		
}	


#pragma mark -
#pragma mark === Quit Routines === 

//-------------------------------------------------------------------
//	QuitRequested
//-------------------------------------------------------------------
//
//

bool TID3Dialog::QuitRequested()
{
	return true;
}

//---------------------------------------------------------------------
//	Quit
//---------------------------------------------------------------------
//
//	Tell our parent item we have been closed

void TID3Dialog::Quit()
{
	m_Item->SetDialog(NULL);
	
	//	Pass to parent
	BWindow::Quit();
}

