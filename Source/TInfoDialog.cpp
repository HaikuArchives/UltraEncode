//---------------------------------------------------------------------
//
//	File:	TInfoDialog.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.20.99
//
//	Desc:	Album Info Settings Dialog
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

#include "TInfoDialog.h"

//	Constants
char *kInfoGenreTable[] = {"Blues",
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

TInfoDialog::TInfoDialog(TProgressDialog *parent, BMessage *message) :
				BWindow(message),
				m_Parent(parent)
{
	// Default initialization
	Init();	
}


//---------------------------------------------------------------------
//	Destructor
//---------------------------------------------------------------------
//
//

TInfoDialog::~TInfoDialog()
{		
}



//---------------------------------------------------------------------
//	Init
//---------------------------------------------------------------------
//
//

bool TInfoDialog::Init()
{		
	//	Find items
	m_ArtistText 	= (BTextControl *)FindView("ArtistTextControl");
	m_AlbumText 	= (BTextControl *)FindView("AlbumTextControl");
	m_YearText 		= (BTextControl *)FindView("YearTextControl");
	m_GenreMenu 	= (BMenuField *)FindView("GenreMenuField");

	//	Set up default control settings	
	m_ArtistText->SetText(m_Parent->Artist()->String());
	m_AlbumText->SetText(m_Parent->Album()->String());
	m_YearText->SetText(m_Parent->Year()->String());

	//	Genre
	for (int32 index = 0; index < genre_count; index++)
	{
		BMenuItem *item = new BMenuItem(kInfoGenreTable[index], NULL);
		m_GenreMenu->Menu()->AddItem(item);
	}
	m_GenreMenu->Menu()->ItemAt(m_Parent->Genre())->SetMarked(true);
		
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

void TInfoDialog::MessageReceived(BMessage* message)
{
	switch (message->what)
	{
		// User pressed OK button. 
		case OK_MSG:
			{											
				//	Save settings
				m_Parent->Artist(m_ArtistText->Text());
				m_Parent->Album(m_AlbumText->Text());
				m_Parent->Year(m_YearText->Text());
				
				BMenuItem *item = m_GenreMenu->Menu()->FindMarked();
				if (item)
					m_Parent->Genre(m_GenreMenu->Menu()->IndexOf(item));
				
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

//---------------------------------------------------------------------
//	Quit
//---------------------------------------------------------------------
//
//	Tell our parent item we have been closed

void TInfoDialog::Quit()
{
	//m_Item->SetDialog(NULL);
	
	//	Pass to parent
	BWindow::Quit();
}

