//---------------------------------------------------------------------
//
//	File:	AppAlerts.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	02.14.98
//
//	Desc:	Alert Handling Routines
//
//	Copyright ©1998 mediapede Software
//
//---------------------------------------------------------------------

// Includes
#include <Application.h>
#include <InterfaceDefs.h>
#include <Debug.h>
#include <Alert.h>

#include <string.h>

#include "AppAlerts.h"
#include "AppUtils.h"


//---------------------------------------------------------------------
//	UnhandledImageFormat
//---------------------------------------------------------------------
//
//	Inform user that we cannot read the data in the image
//

void UnhandledImageFormat(entry_ref *ref)
{
	char msg[1024];
	sprintf(msg, "No Translation Kit add-on found for '%s'.", ref->name);
	BAlert *theAlert = new BAlert("UltraEncode", msg, "OK");
	CenterWindow(theAlert);
	theAlert->Go();
}

//---------------------------------------------------------------------
//	UnhandledImageFormat
//---------------------------------------------------------------------
//
//	Inform user that we cannot read the data in the image
//

void UnhandledMIDIFormat(entry_ref *ref)
{
	char msg[1024];
	sprintf(msg, "Unable to read '%s'.  Invalid MIDI file.", ref->name);
	BAlert *theAlert = new BAlert("UltraEncode", msg, "OK");
	CenterWindow(theAlert);
	theAlert->Go();
}


//---------------------------------------------------------------------
//	SaveAlert
//---------------------------------------------------------------------
//
//	Inform user that the document is dirty and should be saved
//

int32 SaveAlert(const char* documentName)
{
	int32 result;
	char str[1024];
	sprintf(str, "The document '%s' has been changed since the last save.  Save before quiting?", documentName);
	BAlert *theAlert = new BAlert( "UltraEncode", str, 
								   "Don't Save", "Cancel", "Save", B_WIDTH_FROM_WIDEST, B_WARNING_ALERT);
											 
	theAlert->SetShortcut(0, B_ESCAPE); 	 							
	CenterWindow(theAlert);		
	result = theAlert->Go();
	
	return (result);
}


//---------------------------------------------------------------------
//	QuitAlert
//---------------------------------------------------------------------
//
//	Inform user that there are processes occurring
//

int32 QuitAlert()
{
	int32 result;
	BAlert *theAlert = new BAlert( "UltraEncode", "You are extracting and/or encoding files.  Really quit?", 
								   "Quit", "Don't Quit", NULL, B_WIDTH_FROM_WIDEST, B_WARNING_ALERT);
											 
	theAlert->SetShortcut(0, B_ESCAPE); 	 							
	CenterWindow(theAlert);		
	result = theAlert->Go();
	
	return (result);
}



//---------------------------------------------------------------------
//	ErrorAlert
//---------------------------------------------------------------------
//
//	General purpose error dialog
//

int32 ErrorAlert(char *theError, status_t err)
{
	char msg[256];
	int32 result;

	//	Check for error code
	if (err != 0)	
		sprintf(msg, "%s\n%s [%x]", theError, strerror(err), err);
	else
		sprintf(msg, "%s", theError);
	
	BAlert *theAlert = new BAlert( "UltraEncode", msg, "OK", NULL, NULL, B_WIDTH_FROM_WIDEST, B_WARNING_ALERT);
											 
	theAlert->SetShortcut(0, B_ESCAPE); 	 							
	CenterWindow(theAlert);		
	result = theAlert->Go();
	
	return (result);
}
