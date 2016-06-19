//-------------------------------------------------------------------
//
//	File:	TInexactMatchWindow.cpp
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

//	Includes
#include <string.h>
#include <stdio.h>

#include <Button.h>
#include <ListView.h>
#include <StringView.h>
#include <ScrollView.h>
#include <Beep.h>

#include "DebugBuild.h"
#include "AppMessages.h"

#include "CDDBLooper.h"

#include "TInexactMatchWindow.h"



// Vanilla constructor, just give me a size.
TInexactMatchWindow::TInexactMatchWindow(BMessage *message) : BWindow(message)
{
	//	Locate views	
	m_MatchScrollView 	= (BScrollView *) FindView("MatchScrollView");
	m_MatchListView 	= (BListView *) FindView("MatchListView");
	m_OKButton 			= (BButton *) FindView("OK");
	m_CancelButton 		= (BButton *) FindView("Cancel");
	
	//	Set up list messages
	BMessage *selectMessage = new BMessage(MATCH_SELECT_MSG);
	m_MatchListView->SetSelectionMessage(selectMessage);
	
	BMessage *invokeMessage = new BMessage(MATCH_INVOKE_MSG);
	m_MatchListView->SetInvocationMessage(invokeMessage);
	
	//	OK is disabled at start
	m_OKButton->SetEnabled(false);

}



// Handle incoming messages.  The CDDBLooper will send
//   ADD_CD messages as it gets server names from the
//   master server (cddb.cddb.com)

void TInexactMatchWindow::MessageReceived(BMessage *msg) 
{
	const char *info_str;
	
	switch (msg->what) 
	{
		case ADD_CD:
			msg->FindString("info",&info_str);
			if (info_str != NULL)
				m_MatchListView->AddItem(new BStringItem(info_str));
			replyTo = msg->ReturnAddress();
			break;
		
		case CANCEL_MSG:
			Quit();
			break;
			
		//	Enable/disable OK
		case MATCH_SELECT_MSG:
			{
				if (m_MatchListView->CurrentSelection() < 0)
					m_OKButton->SetEnabled(false);
				else
					m_OKButton->SetEnabled(true);
			}
			break;
		
		//	Send current selection
		case MATCH_INVOKE_MSG:
			{
				if (m_MatchListView->CurrentSelection() >= 0)
					Save();
			}
			break;
			
		case OK_MSG:
			Save();
			break;
			
		default:
			BWindow::MessageReceived(msg);
			break;
	}
}


// Gets the user's selection, and parses it up to get the server,
//   then formats a BMessage and sends it to whomever opened the window.
void TInexactMatchWindow::Save() 
{
	const char *selectedText;	
	
	//	Make sure we have a selected item
	int selection = m_MatchListView->CurrentSelection();
	if (selection < 0) 
		return;
	
	// Copy the selected string.
	BStringItem *item; 
	item = (BStringItem *) m_MatchListView->ItemAt(selection); 
	selectedText = item->Text();
	char *parseStr= new char[strlen(selectedText)+1];
	strcpy(parseStr,selectedText);

	char *disc_id;
	char *category;
	 
	//	Send the response to the CDDB looper
	category = strtok(parseStr," ");
	disc_id = strtok(NULL," ");
	//printf("Category is %s.\n",category);
	//printf("New ID is %s.\n",disc_id);
	BMessage *msg = new BMessage(CDDB_READ);
	msg->AddString("category",category);
	msg->AddString("disc_id",disc_id);
	replyTo.SendMessage(msg);
	delete parseStr;
	PostMessage(B_QUIT_REQUESTED);
}
