//-------------------------------------------------------------------
//
//	File:	TInfoBox.cpp
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


// Includes


#include <app/Application.h>
#include <support/Debug.h>

#include "DebugBuild.h"
#include "AppConstants.h"
#include "AppUtils.h"
#include "ResourceManager.h"

#include "TInfoDialog.h"

#include "TInfoBox.h"


//-------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------
//
//

TInfoBox::TInfoBox(TProgressDialog *parent, BRect rect, const char *title) : 
		BBox(rect, title),
		m_Parent(parent)
{
	
}


//-------------------------------------------------------------------
//	MouseDown
//-------------------------------------------------------------------
//
//

void TInfoBox::MouseDown(BPoint where)
{
	//FUNCTION("TInfoBox::MouseDown() \n");	
	
	//	Check for double click
	int32 clicks = 0;
	BMessage *message = Window()->CurrentMessage();	
	message->FindInt32("clicks", &clicks);
	
	if (clicks == 2)
		ShowCDInfo();
}


//-------------------------------------------------------------------
//	ShowCDInfo
//-------------------------------------------------------------------
//
//	Display CD info dialog
//

void TInfoBox::ShowCDInfo()
{
	// Create the dialog from a resource archive
	BMessage *theMessage = GetWindowFromResource("AlbumInfo");
	TInfoDialog *theDialog = new TInfoDialog(m_Parent, theMessage);
	ASSERT(theDialog);
	delete(theMessage);
	
	CenterWindow(theDialog);
	theDialog->Show();
}
