//-------------------------------------------------------------------- 
//      
//      RipperApp.cpp 
// 
//      
//-------------------------------------------------------------------- 

#include <stdio.h> 

#include <Debug.h> 
#include <Alert.h> 
#include <Screen.h> 
#include <Directory.h> 
#include <FindDirectory.h> 
#include <Path.h> 
#include <Volume.h>
#include <VolumeRoster.h>
#include <NodeInfo.h>
#include <string.h>

#include "DebugBuild.h"
#include "AppUtils.h"
#include "AppAlerts.h"
#include "ResourceManager.h"
#include "TProgressDialog.h"
#include "TAboutBox.h"

#include "RipperApp.h"

//	Defines
//#define TIMEOUT 1

//	Constants
const char *kSettingsFolder = "UltraEncode Folder";
const char	*kAppName = "UltraEncode";
	
//==================================================================== 

int main() 
{       
        //	Dump build to terminal
		printf("UltraEncode: Built on %s, at %s\n", __DATE__, __TIME__);

        RipperApp *app = new RipperApp(); 
        app->Run(); 

        delete app; 
        return B_NO_ERROR; 
} 

//-------------------------------------------------------------------- 

RipperApp::RipperApp() : BApplication("application/x-vnd.mediapede-ultraencode") 
{
} 

//-------------------------------------------------------------------- 

void RipperApp::ReadyToRun()
{	
	status_t retVal;
	
	//	Check for time out
	#if defined(TIMEOUT)
	{
		//	Check and see if we have timed out
		tm timeOut;
		timeOut.tm_sec		= 0;
		timeOut.tm_min		= 0;
		timeOut.tm_hour		= 0;
		timeOut.tm_mday		= 14;
		timeOut.tm_mon		= 8;	// 0-11
		timeOut.tm_year		= 99;
		timeOut.tm_isdst 	= -1;
		
		if (time(NULL) > mktime(&timeOut))
		{	
			ErrorAlert("This copy of UltraEncode™ has expired.  Please contact www.mediapede.com for an updated version.");
			be_app->PostMessage(B_QUIT_REQUESTED);
			return;
		}
	}
	#endif	
	
	char error[128] = "There is no CD-ROM drive connected or the drivers for the CD-ROM drive are not installed.";
	
	//	Find CD_ROM drive
	status_t count = TryDir("/dev/disk", m_CDPath);

	// If there are no drives, alert the user and leave... 
	if (count == B_ERROR) 
	{ 
		BAlert *myAlert = new BAlert("", error, "Continue", "Quit"); 
		int32 i = myAlert->Go(); 
		if (i == 1) 
		{ 
			be_app->PostMessage(B_QUIT_REQUESTED);
			return; 
		}
	}
	
	//	Check for and create prefs folder if needed
	BDirectory	dir; 
	BPath 		path;
	BEntry		entry;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path); 
	dir.SetTo(path.Path()); 
	if (dir.FindEntry(kSettingsFolder, &entry) != B_OK)
	{
		retVal = dir.CreateDirectory(kSettingsFolder, &dir);
		if (retVal != B_OK) 
		{ 
			ERROR("RipperApp::ReadyToRun() - Unable to create directory '%s': %s -\n", kSettingsFolder, strerror(retVal)); 
		} 
	}
	
	//	Save path to folder
	if (dir.FindEntry(kSettingsFolder, &m_PrefsEntry) != B_OK)
	{
		ERROR("RipperApp::ReadyToRun() - Unable to stash prefs directory -\n"); 
	}
	
	
	//	Register attributes
	RegisterMp3Attributes();
			
	// Create the status dialog from a resource archive
	BMessage *theMessage = GetWindowFromResource("InfoWindow");
	TProgressDialog *theDialog = new TProgressDialog(theMessage);
	ASSERT(theDialog);
	delete(theMessage);
	
	// Show the dialog
	theDialog->Show();
}


//-------------------------------------------------------------------
//	TryDir
//-------------------------------------------------------------------
//
//

int32 RipperApp::TryDir(const char *directory, char *name)
{ 
	BDirectory dir; 
	dir.SetTo(directory); 
	if(dir.InitCheck() != B_NO_ERROR) 
	{ 
		return B_ERROR; 
	} 
	
	const char *pathName;
	
	dir.Rewind(); 
	BEntry entry; 
	while(dir.GetNextEntry(&entry) >= 0) 
	{ 
		BPath path;
		entry_ref e; 
		
		if(entry.GetPath(&path) != B_NO_ERROR) 
			continue; 
		pathName = path.Path(); 
		
		
		if(entry.GetRef(&e) != B_NO_ERROR) 
			continue; 

		if(entry.IsDirectory()) 
		{ 
			//	Ignore floppy (it is not silent) 
			if(strcmp(e.name, "floppy") == 0) 
				continue;
			
			int32 devfd = TryDir(pathName, name);
			if(devfd >= 0)
				return devfd;
		} 
		else 
		{ 
			int32 devfd; 
			device_geometry g; 

			//	Ignore partitions 
			if(strcmp(e.name, "raw") != 0) 
				continue;
				
			devfd = open(pathName, O_RDONLY); 
			if(devfd < 0) 
				continue; 

			if(ioctl(devfd, B_GET_GEOMETRY, &g, sizeof(g)) >= 0) 
			{
				if(g.device_type == B_CD)
				{ 
					//	Return first CD device found
					strcpy(name, pathName);
					return devfd;
				}
			}
			close(devfd);
		} 
	}
	
	return B_ERROR;
}


//-------------------------------------------------------------------- 

RipperApp::~RipperApp(void) 
{ 
} 

//-------------------------------------------------------------------- 

void RipperApp::AboutRequested(void) 
{ 
	/*
	char	about[256] = "UltraEncode™\n©1999 by Mediapede, Inc.";
	BAlert	*myAlert; 
	
	myAlert = new BAlert("", about, "OK"); 
	myAlert->Go(); 
	*/
	
	// Display the about dialog
	TAboutBox *aboutWindow = new TAboutBox();
	
	// Show the window
	CenterWindow(aboutWindow);
	aboutWindow->Show();
} 

//-------------------------------------------------------------------- 

void RipperApp::MessageReceived(BMessage* msg) 
{ 
	switch (msg->what) 
	{ 
		default: 
			break; 
	} 
} 


//-------------------------------------------------------------------
//	RegisterMp3Attributes
//-------------------------------------------------------------------
//
//	Register attributes associated with mp3
//

void RipperApp::RegisterMp3Attributes()
{
	//	Make sure mime type is installed in system
	BMimeType mime("audio/x-mpeg"); 
	if (!mime.IsInstalled()) 
		mime.Install();
	
	BMessage curAttr;
	mime.GetAttrInfo(&curAttr);
	
	bool addName 	= true;
	bool addArtist 	= true;
	bool addAlbum 	= true;
	bool addYear 	= true;
	bool addGenre 	= true;
	bool addComment = true;
	
	const char *songName 	= "Audio:SongName";
	const char *artistName 	= "Audio:Artist";
	const char *albumName 	= "Audio:Album";
	const char *yearName 	= "Audio:Year";
	const char *genreName 	= "Audio:Genre";
	const char *commentName = "Audio:Comment";
		
	//	Check and see which attributes are already installed
	int32 totalNames = curAttr.CountNames(B_STRING_TYPE);
	
	//	Exit if we have a count.  We don't want to mess anyone up
	if (totalNames)
		return;
		
	/*
	curAttr.PrintToStream();
	
	for(int32 index = 0; index < totalNames; index++)
	{
		if ( strcmp( curAttr.FindString("attr:name", index), songName) == 0)
			addName = false;
		else if ( strcmp( curAttr.FindString("attr:name", index), artistName) == 0)
			addArtist = false;
		else if ( strcmp( curAttr.FindString("attr:name", index), albumName) == 0)
			addAlbum = false;
		else if ( strcmp( curAttr.FindString("attr:name", index), yearName) == 0)
			addYear = false;
		else if ( strcmp( curAttr.FindString("attr:name", index), genreName) == 0)
			addGenre = false;
		else if( strcmp( curAttr.FindString("attr:name", index), commentName) == 0)
			addComment = false;
	}	
	*/
	
	//	Inform Tracker about these attributes	
	
	//	SongName
	if (addName)
	{
		curAttr.AddString("attr:name", songName); 
		curAttr.AddString("attr:public_name", "Song Name"); 
		curAttr.AddInt32("attr:type", B_STRING_TYPE); 
		curAttr.AddBool("attr:viewable", true); 
		curAttr.AddInt32("attr:alignment", B_ALIGN_LEFT); 
		curAttr.AddBool("attr:editable", true); 
		curAttr.AddInt32("attr:width", 50); 
	}
			
	//	Artist
	if (addArtist)
	{
		curAttr.AddString("attr:name", artistName); 
		curAttr.AddString("attr:public_name", "Artist"); 
		curAttr.AddInt32("attr:type", B_STRING_TYPE); 
		curAttr.AddBool("attr:viewable", true); 
		curAttr.AddInt32("attr:alignment", B_ALIGN_LEFT); 
		curAttr.AddBool("attr:editable", true); 
		curAttr.AddInt32("attr:width", 50); 
	}
		
	//	Album
	if (addAlbum)
	{
		curAttr.AddString("attr:name", albumName); 
		curAttr.AddString("attr:public_name", "Album"); 
		curAttr.AddInt32("attr:type", B_STRING_TYPE); 
		curAttr.AddBool("attr:viewable", true); 
		curAttr.AddInt32("attr:alignment", B_ALIGN_LEFT); 
		curAttr.AddBool("attr:editable", true); 
		curAttr.AddInt32("attr:width", 50); 
	}
		
	//	Year
	if (addYear)
	{
		curAttr.AddString("attr:name", yearName); 
		curAttr.AddString("attr:public_name", "Year"); 
		curAttr.AddInt32("attr:type", B_STRING_TYPE); 
		curAttr.AddBool("attr:viewable", true); 
		curAttr.AddInt32("attr:alignment", B_ALIGN_LEFT); 
		curAttr.AddBool("attr:editable", true); 
		curAttr.AddInt32("attr:width", 50); 
	}
			
	//	Genre
	if (addGenre)
	{
		curAttr.AddString("attr:name", genreName); 
		curAttr.AddString("attr:public_name", "Genre"); 
		curAttr.AddInt32("attr:type", B_STRING_TYPE); 
		curAttr.AddBool("attr:viewable", true); 
		curAttr.AddInt32("attr:alignment", B_ALIGN_LEFT);
		curAttr.AddBool("attr:editable", true); 
		curAttr.AddInt32("attr:width", 50); 
	}
		
	//	Comment
	if (addComment)
	{
		curAttr.AddString("attr:name", commentName); 
		curAttr.AddString("attr:public_name", "Comment"); 
		curAttr.AddInt32("attr:type", B_STRING_TYPE); 
		curAttr.AddBool("attr:viewable", true); 
		curAttr.AddInt32("attr:alignment", B_ALIGN_LEFT);					
		curAttr.AddBool("attr:editable", true); 
		curAttr.AddInt32("attr:width", 50);
	}
		
	//	Update info
	mime.SetAttrInfo(&curAttr);
	
	// Replace the attributes (TODO: Only add these as needed/ untrivial? )
	BMessage msg;
	msg.AddString("extensions","mp3");
	msg.AddString("extensions","MP3");
	msg.AddString("extensions","mp2");
	msg.AddString("extensions","MP2");
	msg.AddString("extensions","Mp3");
	msg.AddString("extensions","mP3");
	msg.AddString("extensions","Mp2");
	msg.AddString("extensions","mP2");
	mime.SetFileExtensions(&msg);
	
	// A nice little description
	mime.SetShortDescription("MPEG Audio File");
	mime.SetLongDescription("MPEG Layer 1,2,3 Audio File");
}
