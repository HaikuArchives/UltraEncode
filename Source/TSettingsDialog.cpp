//---------------------------------------------------------------------
//
//	File:	TSettingsDialog.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.10.99
//
//	Desc:	Encoder Settings
//
//	Copyright ©1999 mediapede Software
//
//---------------------------------------------------------------------

// Includes
#include <Application.h>
#include <CheckBox.h>
#include <Directory.h>
#include <MenuField.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Path.h>
#include <Roster.h>
#include <StringView.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <fs_info.h>

#include "DebugBuild.h"
#include "AppConstants.h"
#include "AppUtils.h"
#include "ResourceManager.h"
#include "RipperApp.h"
#include "TRefFilter.h"

#include "TSettingsDialog.h"

//---------------------------------------------------------------------
//	Constructor
//---------------------------------------------------------------------
//
//

TSettingsDialog::TSettingsDialog(TProgressDialog *parent, BMessage *message) :
				BWindow(message),
				m_Parent(parent),
				m_FileOpenPanel(NULL),
				m_CDCount(0)
{
	// Default initialization
	Init();	
}


//---------------------------------------------------------------------
//	Destructor
//---------------------------------------------------------------------
//
//

TSettingsDialog::~TSettingsDialog()
{
	// Clean up
	if (m_FileOpenPanel)
	{
		BRefFilter* f = m_FileOpenPanel->RefFilter();
		if (f)
			delete f;  
		delete m_FileOpenPanel;
	}	
}



//---------------------------------------------------------------------
//	Init
//---------------------------------------------------------------------
//
//

bool TSettingsDialog::Init()
{		
	//	Find items	
	m_ModeMenu 			= (BMenuField *)FindView("ModeMenu");
	m_BitRateMenu 		= (BMenuField *)FindView("BitRateMenu");
	m_DeemphasisMenu 	= (BMenuField *)FindView("DeemphasisMenu");
	m_FlagsMenu	 		= (BMenuField *)FindView("FlagsMenu");
	m_ExtractFromMenu	= (BMenuField *)FindView("ExtractFromMenu");
	m_AutoCheck			= (BCheckBox *)FindView("AutostartCheckBox");	
	m_DeleteSourceCheck	= (BCheckBox *)FindView("DeleteCheckBox");
	
	m_DeleteSourceCheck	= (BCheckBox *)FindView("DeleteCheckBox");
	m_DeleteSourceCheck	= (BCheckBox *)FindView("DeleteCheckBox");
	m_DeleteSourceCheck	= (BCheckBox *)FindView("DeleteCheckBox");

	m_TrackBeepCheck	= (BCheckBox *)FindView("TrackBeepCheckBox");
	m_CDBeepCheck		= (BCheckBox *)FindView("CDBeepCheckBox");
	m_EncodeBeepCheck	= (BCheckBox *)FindView("EncodeBeepCheckBox");

	m_ExtractToButton	= (BButton *)FindView("ExtractToButton");	
	m_ExtractToString	= (BStringView *)FindView("ExtractToString");
	
	m_AddArtistCheck 	= (BCheckBox *)FindView("ArtistCheckBox");
	m_AddAlbumCheck		= (BCheckBox *)FindView("AlbumCheckBox");
	m_AddTrackCheck		= (BCheckBox *)FindView("TrackCheckBox");


	//	Set up default control settings
	m_DeleteSourceCheck->SetValue(m_Parent->m_DeleteSource);
	m_TrackBeepCheck->SetValue(m_Parent->m_TrackDoneBeep);
	m_CDBeepCheck->SetValue(m_Parent->m_CDDoneBeep);
	m_EncodeBeepCheck->SetValue(m_Parent->m_EncodeDoneBeep);	
	m_ExtractToString->SetText(m_Parent->ExtractPathName());
	m_AddArtistCheck->SetValue(m_Parent->m_AddArtist);
	m_AddAlbumCheck->SetValue(m_Parent->m_AddAlbum);
	m_AddTrackCheck->SetValue(m_Parent->m_AddTrack);

	//
	//	Menus
	//
	
	BMenuItem *item;
	
	//	Stereo
	item = m_ModeMenu->Menu()->ItemAt(m_Parent->LayerInfo()->mode);
	if(item)
		item->SetMarked(true);
	
	//	Bit Rate
	item = m_BitRateMenu->Menu()->ItemAt(m_Parent->LayerInfo()->bitrate_index - 1);
	if(item)
		item->SetMarked(true);

	//	De-emphasis
	switch(m_Parent->LayerInfo()->emphasis)
	{
		case 0:
			item = m_DeemphasisMenu->Menu()->ItemAt(0);
			break;
		
		case 1:
			item = m_DeemphasisMenu->Menu()->ItemAt(1);
			break;
		
		case 3:
			item = m_DeemphasisMenu->Menu()->ItemAt(2);
			break;
			
		default:
			item = NULL;
			break;
	}
	if(item)
		item->SetMarked(true);
		
	//	Flags
	
	//	ExtractFrom device menu
	InitExtractFromMenu(m_ExtractFromMenu);
	
	//	Set menu targets
	m_ModeMenu->Menu()->SetTargetForItems(this);
	m_BitRateMenu->Menu()->SetTargetForItems(this);
	m_DeemphasisMenu->Menu()->SetTargetForItems(this);
	m_FlagsMenu->Menu()->SetTargetForItems(this);
	m_FlagsMenu->Menu()->SetTargetForItems(this);
	m_ExtractFromMenu->Menu()->SetTargetForItems(this);
	
	return true;
}

#pragma mark -
#pragma mark === Message Handling === 


//---------------------------------------------------------------------
//	MessageReceived
//---------------------------------------------------------------------
//
//

void TSettingsDialog::MessageReceived(BMessage* message)
{
	switch (message->what)
	{
		// User pressed OK button. 
		case OK_MSG:
			{											
				// Close the dialog 
				Hide();
				
				//	Get settings
				m_Parent->m_DeleteSource 	= m_DeleteSourceCheck->Value();
				m_Parent->m_TrackDoneBeep 	= m_TrackBeepCheck->Value();
				m_Parent->m_CDDoneBeep 		= m_CDBeepCheck->Value();
				m_Parent->m_EncodeDoneBeep 	= m_EncodeBeepCheck->Value();				
				m_Parent->m_AddArtist 		= m_AddArtistCheck->Value();
				m_Parent->m_AddAlbum 		= m_AddAlbumCheck->Value();
				m_Parent->m_AddTrack 		= m_AddTrackCheck->Value();
			}
			break;
			
		// User has canceled the dialog
		case CANCEL_MSG:
			{
				//	Hide dialog
				Hide();
				
				//	Return controls to original settings
				m_DeleteSourceCheck->SetValue(m_Parent->m_DeleteSource);
				m_TrackBeepCheck->SetValue(m_Parent->m_TrackDoneBeep);
				m_CDBeepCheck->SetValue(m_Parent->m_CDDoneBeep);
				m_EncodeBeepCheck->SetValue(m_Parent->m_EncodeDoneBeep);				
				m_AddArtistCheck->SetValue(m_Parent->m_AddArtist);
				m_AddAlbumCheck->SetValue(m_Parent->m_AddAlbum);
				m_AddTrackCheck->SetValue(m_Parent->m_AddTrack);
			}
			break;
		
		case EXTRACT_TO_MSG:
			ShowPathDialog();
			break;

		case B_REFS_RECEIVED:
			SetExtractPath(message);			
			break;
					
		case AUTOSTART_MSG:
			PROGRESS("TSettingsDialog::MessageReceived() - AUTOSTART_MSG -\n");
			break;

		case TRACK_BEEP_MSG:
			{
				if (m_TrackBeepCheck->Value())
					PlayStatusSound("TrackDone");
			}
			break;

		case CD_BEEP_MSG:
			{
				if (m_CDBeepCheck->Value())
					PlayStatusSound("CDDone");
			}
			break;

		case ENCODE_BEEP_MSG:
			{
				if (m_EncodeBeepCheck->Value())
					PlayStatusSound("EncodeDone");
			}
			break;
		
		case ADD_ARTIST_MSG:
			PROGRESS("TSettingsDialog::MessageReceived() - ADD_ARTIST_MSG -\n");
			break;
			
		case ADD_ALBUM_MSG:
			PROGRESS("TSettingsDialog::MessageReceived() - ADD_ALBUM_MSG -\n");
			break;
			
		case ADD_TRACK_MSG:
			PROGRESS("TSettingsDialog::MessageReceived() - ADD_TRACK_MSG -\n");
			break;

		//	Menu Messages
		case STEREO_MSG:
			m_Parent->LayerInfo()->mode = MPG_MD_STEREO;
			break;

		case JOINT_STEREO_MSG:
			m_Parent->LayerInfo()->mode = MPG_MD_JOINT_STEREO;
			break;

		case MONO_MSG:
			m_Parent->LayerInfo()->mode = MPG_MD_DUAL_CHANNEL;
			break;

		case FORCE_MSG:
			m_Parent->LayerInfo()->mode = MPG_MD_MONO;
			break;
			
		case DE_NONE_MSG:
			m_Parent->LayerInfo()->emphasis = 0;
			break;
			
		case DE_50_MSG:
			m_Parent->LayerInfo()->emphasis = 1;
			break;
			
		case DE_CCIT_MSG:
			m_Parent->LayerInfo()->emphasis = 3;
			break;

		case PRIVATE_MSG:
			PROGRESS("PRIVATE_MSG\n");
			break;

		case PROTECTED_MSG:
			PROGRESS("PROTECTED_MSG\n");
			break;

		case COPYRIGHT_MSG:
			PROGRESS("COPYRIGHT_MSG\n");
			break;

		case ORIGINAL_MSG:
			PROGRESS("ORIGINAL_MSG\n");
			break;

		case BR_32_MSG:
			m_Parent->LayerInfo()->bitrate_index = 1;
			break;

		case BR_48_MSG:
			m_Parent->LayerInfo()->bitrate_index = 2;
			break;

		case BR_54_MSG:
			m_Parent->LayerInfo()->bitrate_index = 3;
			break;

		case BR_56_MSG:
			m_Parent->LayerInfo()->bitrate_index = 4;
			break;

		case BR_64_MSG:
			m_Parent->LayerInfo()->bitrate_index = 5;
			break;

		case BR_80_MSG:
			m_Parent->LayerInfo()->bitrate_index = 6;
			break;

		case BR_96_MSG:
			m_Parent->LayerInfo()->bitrate_index = 7;
			break;

		case BR_112_MSG:
			m_Parent->LayerInfo()->bitrate_index = 8;
			break;

		case BR_128_MSG:
			m_Parent->LayerInfo()->bitrate_index = 9;
			break;

		case BR_160_MSG:
			m_Parent->LayerInfo()->bitrate_index = 10;
			break;

		case BR_192_MSG:
			m_Parent->LayerInfo()->bitrate_index = 11;
			break;

		case BR_224_MSG:
			m_Parent->LayerInfo()->bitrate_index = 12;
			break;

		case BR_256_MSG:
			m_Parent->LayerInfo()->bitrate_index = 13;
			break;

		case BR_320_MSG:
			m_Parent->LayerInfo()->bitrate_index = 14;
			break;

		case BR_384_MSG:
			m_Parent->LayerInfo()->bitrate_index = 15;
			break;
		
		case EXTRACT_FROM_DISK_MSG:
			{
				//	Find device path and update prefs
				const char *cdName;
				if (message->FindString("CDPath", &cdName) == B_OK)
				{
					if (strcmp( ((RipperApp *)(be_app))->CDPath(), cdName) != 0)
					{
						((RipperApp *)(be_app))->CDPath(cdName);
					
						//	Have progress dialog update for new drive
						m_Parent->PostMessage(CANCEL_MSG);
					}
				}
			}
			break;
												
		default:
			BWindow::MessageReceived(message);						
			break;
	}		
}	


//-------------------------------------------------------------------
//	QuitRequested
//-------------------------------------------------------------------
//
//

bool TSettingsDialog::QuitRequested()
{
	//	Just hide
	Hide();
	return false;
}


#pragma mark -
#pragma mark == Drive Location ===

//-------------------------------------------------------------------
//	InitExtractFromMenu
//-------------------------------------------------------------------
//
//	Fill menu with CD device paths
//

void TSettingsDialog::InitExtractFromMenu(BMenuField *menuField)
{
	BMenuItem *item;
	
	/*
	char name[1024];
	status_t count = FindCDDirectory("/dev/disk", name);
	
	// If there are no drives, set up menu item to show no drives
	if (count == B_ERROR) 
	{
		item = new BMenuItem("None", NULL); 
		item->SetEnabled(false); 
		menuField->Menu()->AddItem(item);
	}
	//	We found a drive.  Add it to the menu
	else
	{
		//	Create menu message
		BMessage *message = new BMessage(EXTRACT_FROM_DISK_MSG);
		message->AddString("CDPath", name);				
		menuField->Menu()->AddItem(item = new BMenuItem(name, message)); 
	}
	*/
	
	SetupCDMenu("/dev/disk", menuField->Menu());
	if (m_CDCount == 0)
	{
		item = new BMenuItem("None", NULL); 
		item->SetEnabled(false); 
		menuField->Menu()->AddItem(item);	
	}
	
	//	Check proper item
	const char *compName;
	char *cdName = ((RipperApp *)(be_app))->CDPath();	
	for (int32 index = 0; index < menuField->Menu()->CountItems(); index++)
	{
		item =  menuField->Menu()->ItemAt(index);
		if (item)
		{
			if (item->Message()->FindString("CDPath", &compName) == B_OK)
			{
				if (strcmp(compName, cdName) == 0)
				{
					item->SetMarked(true);
					return;
				}
			}	
		}
	}
}


//-------------------------------------------------------------------
//	FindCDDirectory
//-------------------------------------------------------------------
//
//

int32 TSettingsDialog::FindCDDirectory(const char *directory, char *name)
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
			
			int32 devfd = FindCDDirectory(pathName, name);
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


//-------------------------------------------------------------------
//	SetupCDMenu
//-------------------------------------------------------------------
//
//	Scan a directory recursively for CD drives
//

int32 TSettingsDialog::SetupCDMenu(const char *directory, BMenu *menu) 
{
	const char	*name;
	int 		fd;
	BPath 		path;
	BDirectory 	dir;
	BEntry 		entry;
	device_geometry dg;
	
	int32 count = 0;
	
	// Set to the path given.
	dir.SetTo(directory);
	
	// Check for init errors
	if (dir.InitCheck() == B_NO_ERROR) 
	{
		// go to first ref.
		dir.Rewind();
		
		// Loop through all the refs we get.
		while (dir.GetNextEntry(&entry) >= 0) 
		{
			entry.GetPath(&path);
			name = path.Path();
			
			// Try to recurse
			if (entry.IsDirectory())
				SetupCDMenu(name, menu);
				
			// we want only the raw partitions here...
			else if (strstr(name,"/raw")) 
			{
				fd = open(name, 0);
				if (fd >= 0) 
				{
					ioctl(fd, B_GET_GEOMETRY, &dg);
					if (dg.device_type == B_CD) 
					{
						//	Create menu message
						BMessage *message = new BMessage(EXTRACT_FROM_DISK_MSG);
						message->AddString("CDPath", name);				
						menu->AddItem(new BMenuItem(name, message)); 
						m_CDCount++;	
					}
					close(fd);
				}
			}
		}
	}
	
	return count;
}


#pragma mark -
#pragma mark == Extract Path Handling ===

//-------------------------------------------------------------------
//	ShowPathDialog
//-------------------------------------------------------------------
//
//	Display extract path selection dialog
//

void TSettingsDialog::ShowPathDialog()
{
	if (m_FileOpenPanel)
	{
		m_FileOpenPanel->Show();
	}
	else
	{
		// Create a RefFilter for a "directory" type
		TRefFilter *refFilter = new TRefFilter(kCueSheetFilter);
		
		// Construct a file panel and set it to modal
		m_FileOpenPanel = new BFilePanel( B_OPEN_PANEL, NULL, NULL, B_DIRECTORY_NODE, false, NULL, refFilter, true, true );
		if (m_FileOpenPanel)
		{
		 	// Set it to application's home directory
		 	app_info appInfo;
		 	be_app->GetAppInfo(&appInfo); 	
		 	BEntry entry(&appInfo.ref);
		 	BDirectory parentDir;
		 	entry.GetParent(&parentDir);
		 	m_FileOpenPanel->SetPanelDirectory(&parentDir);
		 		
			//	We are the target	
			m_FileOpenPanel->SetTarget(this);
			
			// Center Panel
			CenterWindow(m_FileOpenPanel->Window());
						
			m_FileOpenPanel->Show();
		}
	}
}


//-------------------------------------------------------------------
//	SetExtractPath
//-------------------------------------------------------------------
//
//	Get ref from message and set it as default extract path
//

void TSettingsDialog::SetExtractPath(BMessage *message)
{
	//	Find refs
	entry_ref ref;
	if (message->FindRef("refs", &ref) == B_OK)
	{
		//	Get path
		BEntry entry(&ref);
		if (entry.InitCheck() == B_OK)
		{		
			BPath path(&entry);
			if (path.InitCheck() == B_OK)
			{
				//	Inform parent
				m_Parent->ExtractPathName(path.Path());
				
				//	Update status text
				m_ExtractToString->SetText(path.Path());
			}
			else
				ERROR("TSettingsDialog::SetExtractPath() - Invalid Path -\n");
		}
	}
}
