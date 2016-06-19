//-------------------------------------------------------------------
//
//	File:	TAboutBox.cpp
//
//	Author:	Gene Z. Ragan
//
//	Data:	5.23.97
//
//-------------------------------------------------------------------

// Includes
#include <Resources.h>
#include <TranslationUtils.h>
#include <Debug.h>
#include <Button.h>

#include "DebugBuild.h"
#include "RipperApp.h"
#include "TBitmapView.h"

#include "TAboutBox.h"

//-------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------
//
//

const BRect aboutFrame(200, 100, 703, 409); // 504 310

TAboutBox::TAboutBox() : BWindow(aboutFrame, "About UltraEncode™", B_MODAL_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	Init();
}


//-------------------------------------------------------------------
//	Destructor
//-------------------------------------------------------------------
//
//

TAboutBox::~TAboutBox()
{
}


//-------------------------------------------------------------------
//	Init
//-------------------------------------------------------------------
//
//

void TAboutBox::Init()
{
	
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);
	if (file.InitCheck())
		return;
	
	size_t 		size;
	BBitmap 	*data;
	{
		BResources res(&file);
		data = (BBitmap *)res.FindResource('bits', "Splash", &size);
		if (!data)			
			return;
	}
	
	//	Copy data into BMemoryIO object
	BMemoryIO ioData(data, size);
	
	//	Use BTranslationUtils to load data
	BBitmap *bitmap = BTranslationUtils::GetBitmap(&ioData);
	ASSERT(bitmap);
		
	TBitmapView *bm = new TBitmapView(Bounds(), "AboutBitmap", bitmap, true, B_FOLLOW_ALL, true);

	// Add it to the window
	AddChild(bm);
	
}


//-------------------------------------------------------------------
//
//	Function:	MessageReceived
//
//	Desc:		
//
//-------------------------------------------------------------------
//
//

void TAboutBox::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case B_MOUSE_DOWN:
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
