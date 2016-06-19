//-------------------------------------------------------------------
//
//	File:	InexactMatchWindow.cpp
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

#include "InexactMatchWindow.h"

// Vanilla constructor, just give me a size.
InexactMatchWindow::InexactMatchWindow(BMessage *message) : BWindow(message)
{
	//	Locate views
	
	BRect boundsRect = frame;
	boundsRect.OffsetTo(B_ORIGIN);
	BView *mainView = new BView(boundsRect,"mainView",B_FOLLOW_ALL,B_WILL_DRAW);
	mainView->SetViewColor(220,220,220);
	AddChild(mainView);
	const BFont *font = be_plain_font;
	const char* theString = "Please select the closest match:";
	font_height fh;
	font->GetHeight(&fh);
	float btn_height = fh.leading+20;
	float btn_width = font->StringWidth("No Match")+20;
	float text_width = font->StringWidth(theString);
	
	boundsRect = frame;
	boundsRect.OffsetTo(B_ORIGIN);
	boundsRect.top +=4;
	boundsRect.bottom =boundsRect.top+btn_height;
	boundsRect.left += 8;
	boundsRect.right = boundsRect.left + text_width;
	
	titleString = new BStringView(boundsRect,"titleString",theString,B_FOLLOW_TOP|B_FOLLOW_LEFT,B_WILL_DRAW);
	mainView->AddChild(titleString);
	
	boundsRect = frame;
	boundsRect.OffsetTo(B_ORIGIN);
	boundsRect.bottom -= 12;
	boundsRect.top = boundsRect.bottom-btn_height;
	boundsRect.right -= 8;
	boundsRect.left = boundsRect.right -btn_width;
	
	btnNoMatch = new BButton(boundsRect,"CancelButton","No Match",new BMessage(CANCEL_B),B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT,B_WILL_DRAW);
	mainView->AddChild(btnNoMatch);
	
	boundsRect = frame;
	boundsRect.OffsetTo(B_ORIGIN);
	boundsRect.bottom -= 12;
	boundsRect.top = boundsRect.bottom-btn_height;
	boundsRect.left += 8;
	boundsRect.right = boundsRect.left+btn_width;
	btnSave = new BButton(boundsRect,"SaveButton","Ok",new BMessage(SAVE_B),B_FOLLOW_BOTTOM|B_FOLLOW_LEFT,B_WILL_DRAW);
	mainView->AddChild(btnSave);
	SetDefaultButton(btnSave);
		
	boundsRect = frame;
	boundsRect.OffsetTo(B_ORIGIN);
	boundsRect.left += 8;
	boundsRect.top += 8+btn_height;
	boundsRect.right -= 20;
	boundsRect.bottom -= 18+btn_height;
	
	cdList = new BListView(boundsRect,"cdList",B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL,B_WILL_DRAW);
	
	aView = new BScrollView("",(BView *)cdList,B_FOLLOW_ALL,B_WILL_DRAW,false,true, B_FANCY_BORDER);
	mainView->AddChild(aView);

	// Set the minimum window size
	float minWidth,maxWidth,minHeight,maxHeight;
	GetSizeLimits(&minWidth,&maxWidth,&minHeight,&maxHeight);
	minWidth = 3*btn_width;
	minHeight = 6*btn_height;
	SetSizeLimits(minWidth,maxWidth,minHeight,maxHeight);
}


// Handle incoming messages.  The CDDBLooper will send
//   ADD_CD messages as it gets server names from the
//   master server (cddb.cddb.com)

void InexactMatchWindow::MessageReceived(BMessage *msg) 
{
	char *info_str;
	
	switch (msg->what) 
	{
		case ADD_CD:
			msg->FindString("info",&info_str);
			if (info_str != NULL)
				cdList->AddItem(new BStringItem(info_str));
			replyTo = msg->ReturnAddress();
			break;
		
		case CANCEL_MSG:
			Quit();
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
void InexactMatchWindow::Save() 
{
	const char *selectedText;	
	int selection = cdList->CurrentSelection();
	if (selection < 0) 
	{
		beep();
		return;
	}
	
	// Copy the selected string.
	BStringItem *item; 
	item = (BStringItem *) cdList->ItemAt(selection); 
	selectedText = item->Text();
	char *parseStr= new char[strlen(selectedText)+1];
	strcpy(parseStr,selectedText);

	char *disc_id;
	char *category;
	 
	//	Send the response to the CDDB looper
	category = strtok(parseStr," ");
	disc_id = strtok(NULL," ");
	printf("Category is %s.\n",category);
	printf("New ID is %s.\n",disc_id);
	BMessage *msg = new BMessage(CDDB_READ);
	msg->AddString("category",category);
	msg->AddString("disc_id",disc_id);
	replyTo.SendMessage(msg);
	delete parseStr;
	PostMessage(B_QUIT_REQUESTED);
}
