//-------------------------------------------------------------------
//
//	File:	TCDDBInfoWindow.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	08.30.99
//
//	Desc:	Display CDDB status
//
//	Copyright ©1999	Mediapede, Inc.
//
//---------------------------------------------------------------------

//	Includes
#include <string.h>
#include <stdio.h>
#include <Button.h>
#include <TextControl.h>

#include "DebugBuild.h"
#include "AppMessages.h"

#include "CDDBLooper.h"
#include "TProgressDialog.h"

#include "TCDDBInfoWindow.h"



TCDDBInfoWindow::TCDDBInfoWindow(BMessage *archive, TProgressDialog *parent) : 
		BWindow(archive),
		m_Parent(parent)
{
	//	Locate views	
	m_CDDBServerString 	= (BTextControl *) FindView("ServerTextControl");
	ASSERT(m_CDDBServerString);
	
	//	Set up text
	m_CDDBServerString->SetText(m_Parent->CDDBServerName());	
	m_CDDBServerString->MakeFocus(true);
	
//			inline const char			*CDDBServerName(){ return m_CDDBServerName; }
//		inline void					CDDBServerName(const char *path){ strcpy(m_CDDBServerName, path); }

}


//-------------------------------------------------------------------
//	QuitRequested
//-------------------------------------------------------------------
//
//

void TCDDBInfoWindow::MessageReceived(BMessage *msg) 
{
	switch (msg->what) 
	{			
		// User pressed OK button. 
		case OK_MSG:
			{											
				//	Save settings
				m_Parent->CDDBServerName(m_CDDBServerString->Text());
				m_Parent->UpdateCDDBServer();
				
				// Close the dialog 
				PostMessage(B_QUIT_REQUESTED);
			}
			break;
			
		// User has cancelled the dialog
		case CANCEL_MSG:
			{
				//	Restore text
				m_CDDBServerString->SetText(m_Parent->CDDBServerName());
				
				//	Close				
				PostMessage(B_QUIT_REQUESTED);
			}
			break;
			
		default:
			BWindow::MessageReceived(msg);
			break;
	}
}


//-------------------------------------------------------------------
//	QuitRequested
//-------------------------------------------------------------------
//
//

bool TCDDBInfoWindow::QuitRequested()
{
	//	Just hide
	Hide();
	return false;
}
