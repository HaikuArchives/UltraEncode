//-------------------------------------------------------------------
//
//	File:	TProgressDialog.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	05.20.99
//
//	Desc:	Generic progress dialog
//
//	Copyright ©1999	Mediapede, Inc.
//
//---------------------------------------------------------------------

// Includes
#include <stdio.h> 					// for sprintf()
#include <string.h> 				// for strerror()

#include <netdb.h>
#include <Application.h>
#include <Beep.h>
#include <Box.h>
#include <Button.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <FilePanel.h>
#include <Path.h>
#include <NodeInfo.h>
#include <scheduler.h>
#include <Roster.h>
#include <Screen.h>
#include <StatusBar.h>
#include <StringView.h>
#include <MediaFile.h>
#include <MediaTrack.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <ListView.h>
#include <View.h>
#include <VolumeRoster.h>
#include <ListView.h>
#include <ScrollBar.h>

#include "DebugBuild.h"
#include "AppAlerts.h"
#include "AppMessages.h"
#include "AppUtils.h"
#include "RipperApp.h"
#include "encodelib.h"
#include "ResourceManager.h"
#include "TCDListItem.h"
#include "TProgressListView.h"
#include "TQueueListView.h"
#include "TEncodeListItem.h"
#include "TEncodeListTitle.h"
#include "id3tag.h"
#include "TInfoBox.h"
#include "KnobSwitch.h"
#include "TRefFilter.h"
#include "TFrameBox.h"
#include "TCDDBInfoWindow.h"

//#include "TMP3Writer.h"

#include "TProgressDialog.h"

//	Constants
const float		kCollapsedHeight 	 = 175;
const float		kExtractorHeight 	 = 370;
const float		kEncoderHeight 	 	 = 585;
const float		kExtractorPaneHeight = 195;
const float		kEncoderPaneHeight 	 = 215;
const BPoint	kExtractorPos(5, 155);
const BPoint	kEncoderPos(5, 350);

const bigtime_t kLockTimeout = 15000;

//-------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------
//
//

TProgressDialog::TProgressDialog(BMessage *theMessage) : 
				BWindow(theMessage),
				m_CDDBLooper(0),
				m_TOC(NULL),
				m_ReadCommand(0),
				m_FrameBuffer(0),
				m_CDID(0),
				m_Changed(false),
				m_ProcessList(0),
				m_TitleList(0),
				m_CanUseCDDB(false),
				m_StartExtraction(false),
				m_MouseFilter(0),
				m_SettingsDialog(0),
				m_FileOpenPanel(0),
				m_Genre(0),
				m_ExtractorPanelOpen(false),
				m_EncoderPanelOpen(false),
				m_CDDBInfoWindow(0)
{
	//
	//	Locate all child views
	//
	
	// 	Set up menu bar
	BMessage *menuMsg = GetMenuFromResource("ProgressDialogMenuBar");
	ASSERT(menuMsg);	
	m_MenuBar = new BMenuBar(menuMsg);
	delete(menuMsg);
	
	//
	// 	Set up application menus.
	
	//	File menu
	BMenu *fileMenu = m_MenuBar->SubmenuAt(0);
	ASSERT(fileMenu);
	fileMenu->SetTargetForItems(be_app);
	
		//	Rio item
		BMenuItem *item = fileMenu->FindItem(MOUNT_RIO_MSG);
		if(item)
			item->SetTarget(this);
	
	//	Settings menu
	BMenu *settingsMenu = m_MenuBar->SubmenuAt(1);
	ASSERT(settingsMenu);
	settingsMenu->SetTargetForItems(this);
	
	AddChild(m_MenuBar);
	
	
	BView *view = FindView("BackgroundView");	
	view->SetFlags(B_WILL_DRAW);
		
	view = FindView("InfoBox");	
	view->SetFlags(0);
	view->SetFlags(B_WILL_DRAW);
	
	//	Find info text items
	m_AlbumString  = (BStringView *)FindView("AlbumUserString");
	m_ArtistString = (BStringView *)FindView("ArtistUserString");
	m_TracksString = (BStringView *)FindView("TracksUserString");
	m_CDDBString   = (BStringView *)FindView("CDDBUserString");
	
	// Find status bars
	m_ExtractTrackStatusBar = (BStatusBar *)FindView("ExtractTrackStatusBar");
	m_ExtractCDStatusBar 	= (BStatusBar *)FindView("ExtractCDStatusBar");
	m_EncodeStatusBar 		= (BStatusBar *)FindView("EncodeStatusBar");
	
	//	Find buttons
	m_CancelButton 	= (BButton *)FindView("CancelButton");
	m_StartButton 	= (BButton *)FindView("StartButton");
	m_CDDBButton 	= (BButton *)FindView("CDDBButton");
	
	m_EncoderStopButton 	= (BButton *)FindView("EncoderStopButton");
	m_EncoderStartButton 	= (BButton *)FindView("EncoderStartButton");
	m_EncoderAddButton 		= (BButton *)FindView("EncoderAddButton");
	m_EncoderRemoveButton	= (BButton *)FindView("EncoderRemoveButton");
	
	//
	//	Find and setup extract list view
	//
	
	m_ExtractListScrollBar 	= (BScrollBar *)FindView("ScrollBar");
	BView *tracksList 		=  FindView("ScrollView");
	m_TracksListView		= new TProgressListView(this, tracksList->Frame());
	tracksList->Parent()->AddChild(m_TracksListView);
	tracksList->RemoveSelf();
	delete tracksList;
	m_ExtractListScrollBar->SetTarget(m_TracksListView);
	
	//	Set list view messages
	BMessage *selectMessage = new BMessage(TRACKLIST_SELECT_MSG);
	m_TracksListView->SetSelectionMessage(selectMessage);
	
	BMessage *invokeMessage = new BMessage(TRACKLIST_INVOKE_MSG);
	m_TracksListView->SetInvocationMessage(invokeMessage);

	//	ExtractListTitle
	BView *titleView = FindView("ExtractListTitle");
	m_TracksListTitle = new TExtractListTitle(m_TracksListView, titleView->Frame());
	titleView->Parent()->AddChild(m_TracksListTitle);
	titleView->RemoveSelf();
	delete titleView;

	//
	//	Find and setup Encoder list view
	//
	
	m_EncoderListScrollBar 	= (BScrollBar *)FindView("EncodeScrollBar");
	tracksList 				=  FindView("EncodeScrollView");
	m_EncoderListView = new TQueueListView(this, tracksList->Frame());
	tracksList->Parent()->AddChild(m_EncoderListView);
	tracksList->RemoveSelf();
	delete tracksList;
	m_EncoderListScrollBar->SetTarget(m_EncoderListView);

	//	Set list view messages
	BMessage *encodeSelectMessage = new BMessage(ENCODER_LIST_SELECT_MSG);
	m_EncoderListView->SetSelectionMessage(encodeSelectMessage);
	
	BMessage *encodeInvokeMessage = new BMessage(ENCODER_LIST_INVOKE_MSG);
	m_EncoderListView->SetInvocationMessage(encodeInvokeMessage);
	
	//	EncodeListTitle
	titleView = FindView("EncodeListTitle");
	m_EncodeListTitle = new TEncodeListTitle(m_EncoderListView, titleView->Frame());
	titleView->Parent()->AddChild(m_EncodeListTitle);
	titleView->RemoveSelf();
	delete titleView;
	
	//
	//	Extractor Queue box
	//
	
	BBox *box = (BBox *)FindView("ExtractorBox");
	m_ExtractorPane = new TFrameBox(box->Frame(), box->Label());
	m_ExtractorPane->SetLabel(box->Label());
	box->Parent()->AddChild(m_ExtractorPane);
	
	//	Transfer children
	m_TracksListTitle->RemoveSelf();
	m_ExtractorPane->AddChild(m_TracksListTitle);
	m_TracksListView->RemoveSelf();
	m_ExtractorPane->AddChild(m_TracksListView);
	m_ExtractListScrollBar->RemoveSelf();
	m_ExtractorPane->AddChild(m_ExtractListScrollBar);
	
	//	Set bevel
	BRect bevel = m_TracksListTitle->Frame() | m_ExtractListScrollBar->Frame();
	bevel = m_TracksListView->Frame() | bevel;
	bevel.InsetBy(-2, -2);
	m_ExtractorPane->Bevel(bevel);

	//	Remove old box
	box->RemoveSelf();
	delete box;
	
	//
	//	Info box
	//
	
	box = (BBox *)FindView("InfoBox");
	m_InfoBox = new TInfoBox(this, box->Frame(), box->Label());
	m_InfoBox->SetLabel(box->Label());
	box->Parent()->AddChild(m_InfoBox);
	
	//	Transfer children
	view = FindView("AlbumString");
	view->RemoveSelf();
	m_InfoBox->AddChild(view);
	m_AlbumString->RemoveSelf();
	m_InfoBox->AddChild(m_AlbumString);

	view = FindView("ArtistString");
	view->RemoveSelf();
	m_InfoBox->AddChild(view);
	m_ArtistString->RemoveSelf();
	m_InfoBox->AddChild(m_ArtistString);

	view = FindView("TracksString");
	view->RemoveSelf();
	m_InfoBox->AddChild(view);
	m_TracksString->RemoveSelf();
	m_InfoBox->AddChild(m_TracksString);

	view = FindView("TracksString");
	view->RemoveSelf();
	m_InfoBox->AddChild(view);
	m_TracksString->RemoveSelf();
	m_InfoBox->AddChild(m_TracksString);
	
	view = FindView("CDDBString");
	view->RemoveSelf();
	m_InfoBox->AddChild(view);
	m_CDDBString->RemoveSelf();
	m_InfoBox->AddChild(m_CDDBString);
		
	//	Remove old box
	box->RemoveSelf();
	delete box;
	
	//
	//	Encoder box
	//
	
	box = (BBox *)FindView("EncoderBox");
	m_EncoderPane = new TFrameBox(box->Frame(), box->Label());
	m_EncoderPane->SetLabel(box->Label());
	box->Parent()->AddChild(m_EncoderPane);
	
	//	Transfer children
	m_EncodeListTitle->RemoveSelf();
	m_EncoderPane->AddChild(m_EncodeListTitle);
	m_EncoderListView->RemoveSelf();
	m_EncoderPane->AddChild(m_EncoderListView);
	m_EncoderListScrollBar->RemoveSelf();
	m_EncoderPane->AddChild(m_EncoderListScrollBar);
	m_EncoderStopButton->RemoveSelf();
	m_EncoderPane->AddChild(m_EncoderStopButton);
	m_EncoderStartButton->RemoveSelf();
	m_EncoderPane->AddChild(m_EncoderStartButton);
	m_EncoderAddButton->RemoveSelf();
	m_EncoderPane->AddChild(m_EncoderAddButton);
	m_EncoderRemoveButton->RemoveSelf();
	m_EncoderPane->AddChild(m_EncoderRemoveButton);
	
	//	Set bevel
	bevel = m_EncodeListTitle->Frame() | m_EncoderListScrollBar->Frame();
	bevel = m_EncoderListView->Frame() | bevel;
	bevel.InsetBy(-2, -2);
	m_EncoderPane->Bevel(bevel);

	//	Remove old box
	box->RemoveSelf();
	delete box;
		
	//	Extractor Switch
	view = FindView("SliderSwitch01");
	m_ExtractorKnob = new KnobSwitch( view->Frame(), "", B_FOLLOW_TOP | B_FOLLOW_RIGHT ); 
	m_ExtractorKnob->ResizeToPreferred(); 
	m_ExtractorKnob->SetMessage( new BMessage(SHOW_EXTRACTOR_MSG) ); 
	view->Parent()->AddChild(m_ExtractorKnob); 
	view->RemoveSelf();
	delete view;
	
	//	Encoder Switch
	view = FindView("SliderSwitch02");
	m_EncoderKnob = new KnobSwitch( view->Frame(), "", B_FOLLOW_TOP | B_FOLLOW_RIGHT ); 
	m_EncoderKnob->ResizeToPreferred(); 
	m_EncoderKnob->SetMessage( new BMessage(SHOW_ENCODER_MSG) ); 
	view->Parent()->AddChild(m_EncoderKnob); 
	view->RemoveSelf();
	delete view;
	
	//	Default initialization
	Init();
	
	//	Resize based on knob settings
	m_ExtractorKnob->SetValue(m_ExtractorPanelOpen);
	m_EncoderKnob->SetValue(m_EncoderPanelOpen);
	
	if (m_ExtractorPanelOpen)
		ShowExtractor(m_ExtractorPanelOpen);
	if (m_EncoderPanelOpen)
		ShowEncoder(m_EncoderPanelOpen);
	
	//	Move to desired location
	BScreen screen(B_MAIN_SCREEN_ID);
	if (screen.Frame().Contains(m_Position))
		MoveTo(m_Position);
	else
	{
		CenterWindow(this);
		m_Position.Set(Frame().left, Frame().top);
	}
}


//-------------------------------------------------------------------
//	Destructor
//-------------------------------------------------------------------
//
//

TProgressDialog::~TProgressDialog()
{
	//	Kill extractor thread
	kill_thread(m_ExtractorThread);

	//	Kill watcher thread
	kill_thread(m_WatcherThread);

	StopCompressor();
		
	//	Free access semaphore
	if (m_CDSem) 
		delete_sem(m_CDSem);

	//	Free memory
	if(m_FrameBuffer != NULL) 
		free(m_FrameBuffer);
		
	if(m_ReadCommand  != NULL) 
		free(m_ReadCommand);
		
	if(m_TOC != NULL) 
		free(m_TOC);
		
	//	Tell the looper to quit itself.
	if (m_CDDBLooper)
		m_CDDBLooper->PostMessage(B_QUIT_REQUESTED);
	
	//	Clear out list of entries
	if (m_ProcessList)
	{
		for (int32 index = 0; index < m_ProcessList->CountItems(); index++)
		{
			BEntry *entry = (BEntry *)m_ProcessList->ItemAt(index);
			if (entry)
				delete entry;
		}
		delete m_ProcessList;
		m_ProcessList = NULL;
	}
	
	//	Clear out title list
	if (m_TitleList)
	{
		for (int32 index = 0; index < m_TitleList->CountItems(); index++)
		{
			track_info *albumData = (track_info  *)m_TitleList->ItemAt(index);
			if (albumData)
				free(albumData);
		}		
		delete m_TitleList;
		m_TitleList = NULL;
	}
	
	//	Clear filters
	if (m_MouseFilter)
	{
		RemoveCommonFilter(m_MouseFilter);
		delete m_MouseFilter;
	}

	if (m_KeyboardFilter)
	{
		RemoveCommonFilter(m_KeyboardFilter);
		delete m_KeyboardFilter;
	}

}

#pragma mark -
#pragma mark === Quit Routines ===

//-------------------------------------------------------------------
//	QuitRequested
//-------------------------------------------------------------------
//
//

bool TProgressDialog::QuitRequested() 
{ 
	//FUNCTION("TProgressDialog::QuitRequested()\n");
	
	bool retVal = true;
	
	//	Check and see if we are extracting or encoding
	if (m_StartExtraction || (m_ProcessList->CountItems() > 0 && m_CompressorThread))
	{
		int32 userVal = QuitAlert();
		
		//	Check user response
		switch( userVal)
		{
			//	User does not want to save
			case 0:
				retVal = true;
				break;
			
			// 	User decided not to quit
			case 1:
				retVal = false;
				break;
			
			default:
				retVal = true;
				
		}
	}
				
	//	Return user response to application
	return retVal;
}

//-------------------------------------------------------------------
//	Quit
//-------------------------------------------------------------------
//
//	Only one window in app
//

void TProgressDialog::Quit() 
{ 
	//FUNCTION("TProgressDialog::Quit()\n");
	
	if (m_SettingsDialog)
	{
		m_SettingsDialog->Lock();
		m_SettingsDialog->Quit();	
	}
	
	if (m_CDDBInfoWindow)
	{
		m_CDDBInfoWindow->Lock();
		m_CDDBInfoWindow->Quit();	
	}

	// Clean up
	if (m_FileOpenPanel)
	{
		BRefFilter* f = m_FileOpenPanel->RefFilter();
		if (f)
			delete f;  
		delete m_FileOpenPanel;
	}

	//	Save out prefs.  We just archive into a BMessage and flatten it to disk
	SavePrefs();
	
	//	Quit app
	be_app->PostMessage(B_QUIT_REQUESTED);
	
	//	Tell parent
	BWindow::Quit();
}



#pragma mark -
#pragma mark === Initialization ===

//-------------------------------------------------------------------
//	Init
//-------------------------------------------------------------------
//
//

void TProgressDialog::Init() 
{
	//	Try to find prefs
	bool retVal = LoadPrefs();
	
	//	Set up default values
	if (retVal == false)
	{		
		//	Set up default extract volume to be boot volume if we failed
		BVolumeRoster 	volRoster;
		BVolume			bootVol;
		volRoster.GetBootVolume(&bootVol);
		
		//	Create path to "home" on capture device
		char volName[B_FILE_NAME_LENGTH];
		bootVol.GetName(volName);
		sprintf(m_ExtractPathName, "/%s/%s/", volName, "home");
				
		//	CDDB path
		strcpy(m_CDDBServerName, "us.cddb.com");
		
		//	Delete source file
		m_DeleteSource = 0;

		//	Alerts
		m_TrackDoneBeep  = 1;
		m_CDDoneBeep 	 = 1;
		m_EncodeDoneBeep = 1;
		
		m_AddArtist		= 0;
		m_AddAlbum		= 0;
		m_AddTrack		= 0;
		
		m_ExtractorPanelOpen = true;
		m_EncoderPanelOpen 	 = true;

		
		// Get screen settings
		BScreen theScreen(B_MAIN_SCREEN_ID);
		BRect screenBounds = theScreen.Frame();
		BRect windBounds   = Frame();
		
		m_Position.x = (screenBounds.Width() - windBounds.Width())/2;
		m_Position.y = 75;
		
		//	Init encoder settings
		m_LayerInfo.version				= MPEG_AUDIO_ID;
		m_LayerInfo.lay					= DFLT_LAY;
		m_LayerInfo.error_protection	= false;
		m_LayerInfo.bitrate_index		= 9;
		m_LayerInfo.padding				= 0;
		m_LayerInfo.extension			= 0;
		m_LayerInfo.mode				= MPG_MD_STEREO;
		m_LayerInfo.mode_ext			= 0;
		m_LayerInfo.copyright			= 0;
		m_LayerInfo.original			= 1;
		m_LayerInfo.emphasis			= 0;
		m_AcousticModel					= DFLT_PSY;
	}
	
	//	Verify that path loaded is valid
	BPath path(m_ExtractPathName);
	if (path.InitCheck() != B_OK)
	{
		//	Bad path.  Reset to /boot/home
		BVolumeRoster 	volRoster;
		BVolume			bootVol;
		volRoster.GetBootVolume(&bootVol);
		find_directory(B_USER_DIRECTORY, bootVol.Device(), true, m_ExtractPathName, B_PATH_NAME_LENGTH);
	}
		
	//	Init controls to starting states
	InitControls();

	//	Create track process list
	m_ProcessList = new BList();
		
	//	Create cd access semaphore
	m_CDSem = create_sem(1, "CD_SEM");
	
	//	Allocate all memory for CDDA operations
	if((m_TOC = (CDDA_TOC *)malloc(sizeof(scsi_toc))) == NULL)
	{
		ERROR("TProgressDialog::Init() - Error allocating TOC -\n");
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}
		
	if((m_ReadCommand = (scsi_read_cd *)malloc(sizeof(scsi_read_cd))) == NULL) 
	{
		ERROR("TProgressDialog::Init() - Error allocating read command -\n");
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}
		
	if((m_FrameBuffer = (CDDA_BLOCK *)malloc(sizeof(CDDA_BLOCK))) == NULL) 
	{
		ERROR("TProgressDialog::Init() - Error allocating frame buffer -\n");
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}

	// Spawn CD watcher thread	
	m_WatcherThread = spawn_thread( cd_watcher, "CDWatcher", B_NORMAL_PRIORITY, (void *)this);
	if( m_WatcherThread != B_NO_MORE_THREADS && m_WatcherThread != B_NO_MEMORY)
	{
		resume_thread(m_WatcherThread);
	}
	else
	{
		ERROR("TProgressDialog::Init() - Error creating CD watcher! -\n");
	}
	
	//	Start compressor thread
	StartCompressor();
		
	//	Spawn extractor thread
	m_ExtractorThread = spawn_thread( extractor, "extractor", B_NORMAL_PRIORITY, (void *)this);
	if( m_ExtractorThread != B_NO_MORE_THREADS && m_ExtractorThread != B_NO_MEMORY)
	{
		resume_thread(m_ExtractorThread);
	}
	else
	{
		ERROR("TProgressDialog::Init() - Error creating extractor! -\n");
	}
	
	//	Start the CDDB Lookup looper
	char cddbServer[MAXHOSTNAMELEN];
	strcpy(cddbServer, m_CDDBServerName);
	m_CDDBLooper = new CDDBLooper("CDDBLooper", B_NORMAL_PRIORITY, cddbServer, 888);
	
	//	Install filters
	//CreateMouseFilter();
	CreateKeyboardFilter();
	
	//	Update encoder controls
	UpdateEncoderControls();
		
	//	Set focus to list view
	m_TracksListView->MakeFocus(true);
}


//-------------------------------------------------------------------
//	InitControls
//-------------------------------------------------------------------
//
//

void TProgressDialog::InitControls() 
{
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		//	Set bar height
		m_ExtractTrackStatusBar->SetBarHeight(10.0);
		m_ExtractCDStatusBar->SetBarHeight(10.0);

		//	Info text
		m_Album.SetTo("");
		m_Artist.SetTo("");
		m_Year.SetTo("");
		m_Genre = 0;
		
		m_TracksString->SetText("");
		m_ArtistString->SetText(m_Artist.String());
		m_AlbumString->SetText(m_Album.String());
		m_CDDBString->SetText("");

		//	Update button enabled states
		m_StartButton->SetEnabled(false);
		m_CDDBButton->SetEnabled(false);
		m_CancelButton->SetEnabled(false);
		
		//	Status bars
		//	Clear out totally
		m_ExtractTrackStatusBar->Reset();
		m_ExtractCDStatusBar->Reset();
		m_EncodeStatusBar->Reset();
		
		//	Reset
		char leadText[512];
		sprintf(leadText, "Waiting...");
		m_ExtractTrackStatusBar->Reset(leadText, "");
		m_ExtractCDStatusBar->Reset(leadText, "");
		m_EncodeStatusBar->Reset(leadText, "");
		
		Unlock();
	}
}

#pragma mark -
#pragma mark === Message Handling ===

//-------------------------------------------------------------------
//	MessageReceived
//-------------------------------------------------------------------
//
//

void TProgressDialog::MessageReceived(BMessage *message) 
{ 
	//FUNCTION("TProgressDialog::MessageReceived() - ENTER -\n");

	switch(message->what)
	{
		//	We have received some file refs from a drag onto one of our sorters
		case B_SIMPLE_DATA:
			{
				if (message->WasDropped())
					HandleRefsMessage(message);
			}
			break;
			
		//	From BFilePanel
		case B_REFS_RECEIVED:
			HandleRefsMessage(message);
			break;
			
		case START_MSG:
			{
				//if (LockWithTimeout(kLockTimeout))
				if (Lock())
				{
				
					//	Deselect all list items
					m_TracksListView->DeselectAll();
					
					//	Set flag
					m_StartExtraction = true;
					
					//	Update button enabled states				
					m_StartButton->SetEnabled(false);
					m_CDDBButton->SetEnabled(false);
					m_CancelButton->SetEnabled(true);
					
					Unlock();
				}
			}
			break;

		case OPTIONS_MSG:
			ShowSettingsDialog();
			break;

		//	Look up CDDB info
		case CDDB_MSG:
			CheckCDDB();
			break;

		case CANCEL_MSG:
			{
				//if (LockWithTimeout(kLockTimeout))
				if (Lock())
				{				
					StopExtraction();
					
					//	Update button enabled states
					m_StartButton->SetEnabled(true);
					m_CDDBButton->SetEnabled(true);
					m_CancelButton->SetEnabled(false);
					
					Unlock();
				}
			}
			break;
			
		//	New cd information is in a cd_info structure contained in the message
		case CD_INFO:
			{
				cd_info *newInfo;
				ssize_t numBytes;
				message->FindData("cd_info", B_RAW_TYPE, (const void **)&newInfo, &numBytes);
				PRINTF("TProgressDialog::MessageReceived CD_INFO - returned from FindData newInfo = %08x, size = %08x\n", newInfo, numBytes);
				if (numBytes == sizeof(cd_info))
				{
					PRINTF("TProgressDialog::MessageReceived CD_INFO - Calling SetCDInfo\n");
					SetCDInfo(newInfo);
				}
				
				m_CanUseCDDB = true;
			}
			break;
			
		case STATUS_MSG:
			{
				//	Get status message
				const char *string;
				if (message->FindString("status_msg", &string) == B_OK)
					m_CDDBString->SetText(string);
			}
			break;
				
		case SHOW_EXTRACTOR_MSG:
			{
				BControl *control;
				if ( message->FindPointer("source", (void **)&control) == B_OK )
					ShowExtractor(control->Value());
			}
			break;
			
		case SHOW_ENCODER_MSG:
			{
				BControl *control;
				if ( message->FindPointer("source", (void **)&control) == B_OK )
					ShowEncoder(control->Value());
			}
			break;
		
		case ENCODER_LIST_SELECT_MSG:
			UpdateEncoderControls();
			break;
			
		case TRACKLIST_SELECT_MSG:
			break;

		case TRACKLIST_INVOKE_MSG:
			{
				//	Get pointer to list
				BListView *listView;
				if (message->FindPointer("source", (void **)&listView) == B_OK)				
				{
					//	Find selected item and set it's value
					int32 index;
					if (message->FindInt32("index", &index) == B_OK)
					{
						TCDListItem *listItem = (TCDListItem *)listView->ItemAt(index);
						if (listItem)
						{
							//	Toggle value
							listItem->Selected(!listItem->Selected());
							listView->Invalidate();
							
							//	Determine if we have to add or remove entry from track list
							if (listItem->Selected())
							{
								if (m_ProcessList)
								{
									//	Set extract flag to true
									track_info *albumData = (track_info *)m_TitleList->ItemAt(listItem->ID());
									if (albumData)
										albumData->extract = true;
								}
							}
							else
							{
								if (m_ProcessList)
								{
									//	Set extract flag to false
									track_info *albumData = (track_info *)m_TitleList->ItemAt(listItem->ID());
									if (albumData)
										albumData->extract = false;
								}
							}					
						}
					}
				}
			}
			break;
		
		//	Start encoder
		case START_ENCODE_MSG:
			{
				StartCompressor();
				UpdateEncoderControls();
				m_EncoderListView->Invalidate();
			}
			break;
					
		//	Stop encoder
		case STOP_ENCODE_MSG:
			{
				StopCompressor();
				UpdateEncoderControls();
				m_EncoderListView->Invalidate();
			}
			break;
			
		case ADD_ENCODE_MSG:
			ShowFileOpenPanel();
			break;
			
		case REMOVE_ENCODE_MSG:
			RemoveEncoderQueueItem();
			break;
		
		case CD_INFO_MSG:
			m_InfoBox->ShowCDInfo();
			break;
			
		case CDDB_INFO_MSG:
			{
				if (m_CDDBInfoWindow)
				{
					m_CDDBInfoWindow->Show();
				}
				else
				{
					BMessage *theMessage = GetWindowFromResource("CDDBSettings");
					m_CDDBInfoWindow = new TCDDBInfoWindow(theMessage, this);
					ASSERT(m_CDDBInfoWindow);
					delete(theMessage);
						
					CenterWindow(m_CDDBInfoWindow);
					m_CDDBInfoWindow->Show();
				}
			}
			break;

		case MOUNT_RIO_MSG:
			{
				execle("/boot/beos/bin/mount", "-t rio /dev/misc/rio/$file /rio");
			}
			break;
			
		default:
			break;
	}
}

#pragma mark -
#pragma mark === Filter Routines ===

//---------------------------------------------------------------------
//	CreateMouseFilter
//---------------------------------------------------------------------
//
//	Install message filter that checks for mouse events
//

void TProgressDialog::CreateMouseFilter()
{
	//FUNCTION("TProgressDialog::CreateMouseFilter() - ENTER -\n");
	
	m_MouseFilter = new BMessageFilter(B_MOUSE_DOWN, mouse_filter);
	AddCommonFilter(m_MouseFilter);
}

//---------------------------------------------------------------------
//	Mouse down hook function
//---------------------------------------------------------------------
//
//

filter_result mouse_filter(BMessage *message, BHandler **target, BMessageFilter *messageFilter)
{
	TProgressDialog *dialog = static_cast<TProgressDialog *>(messageFilter->Looper());
	if (dialog)
	{	
		//	Don't allow click in list view if we are extracting
		if ((dialog->Extracting() == true) && (*target == dialog->TracksListView()))
		{
			return B_SKIP_MESSAGE;
		}
		else
			return B_DISPATCH_MESSAGE;
	}
	
	return B_DISPATCH_MESSAGE;
}


//---------------------------------------------------------------------
//	CreateKeyboardFilter
//---------------------------------------------------------------------
//
//	Install message filter that handles transport hot keys
//

void TProgressDialog::CreateKeyboardFilter()
{
	//FUNCTION("TCueSheetWindow::CreateKeyboardFilter() - ENTER -\n");
	
	m_KeyboardFilter = new BMessageFilter(B_KEY_DOWN, keyboard_filter);
	AddCommonFilter(m_KeyboardFilter);
}

//---------------------------------------------------------------------
//	Keyboard hook function
//---------------------------------------------------------------------
//
//

filter_result keyboard_filter(BMessage *message, BHandler **target, BMessageFilter *messageFilter)
{
	TProgressDialog *dialog = static_cast<TProgressDialog *>(messageFilter->Looper());
	if (dialog)
	{
		//	Check for selection keys
		int8 byte;
		if ( message->FindInt8("byte", &byte) == B_OK )
		{
			switch(byte)
			{
				//	Select All
				case 'a':
					{													
						if (IsCommandKeyDown())
						{							
							//	Do nothing if we are extracting
							if (dialog->Extracting() == true)
								return B_SKIP_MESSAGE; 

							dialog->SelectAllExtractorItems();							
							return B_SKIP_MESSAGE;
						}
					}
					break;

				//	Deselect All
				case 'd':
					{
						if (IsCommandKeyDown())
						{
							
							if (dialog->Extracting() == true)
								return B_SKIP_MESSAGE; 

							dialog->DeselectAllExtractorItems();							
							return B_SKIP_MESSAGE;
						}
					}
					break;
					
				default:
					break;
			}
		}					
		else
			return B_DISPATCH_MESSAGE;
	}
	
	return B_DISPATCH_MESSAGE;
}



#pragma mark -
#pragma mark === Status Routines ===

//-------------------------------------------------------------------
//	Enable
//-------------------------------------------------------------------
//
//	Enable controls and prepare for extraction
//

void TProgressDialog::Enable() 
{ 
	//	Enable buttons
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		m_StartButton->SetEnabled(true);
		m_CDDBButton->SetEnabled(true);
		m_CancelButton->SetEnabled(false);
		Unlock();
	}
}

//-------------------------------------------------------------------
//	Disable
//-------------------------------------------------------------------
//
//

void TProgressDialog::Disable() 
{ 
	//	Reset tracks
	m_MinTrack = 1; 
	m_MaxTrack = 1; 
		
	//	Clear out title list
	if (m_TitleList)
	{
		for (int32 index = 0; index < m_TitleList->CountItems(); index++)
		{
			track_info *albumData = (track_info  *)m_TitleList->ItemAt(index);
			if (albumData)
				free(albumData);
		}		
		delete m_TitleList;
		m_TitleList = NULL;
	}


	//	Reset all controls expect encoder items
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		//	Clear out old items
		for (int32 index = m_TracksListView->CountItems(); index >= 0; index--)
		{
			TCDListItem *item = (TCDListItem *)m_TracksListView->RemoveItem((int32)0);
			if (item)
				delete item;
		}
		//m_TracksListView->MakeEmpty();

		//	Info text
		m_Album.SetTo("");
		m_Artist.SetTo("");
		m_Year.SetTo("");
		m_Genre = 0;
		
		m_TracksString->SetText("");		
		m_ArtistString->SetText(m_Artist.String());
		m_AlbumString->SetText(m_Album.String());
		m_CDDBString->SetText("");

		//	Update button enabled states
		m_StartButton->SetEnabled(false);
		m_CDDBButton->SetEnabled(false);
		m_CancelButton->SetEnabled(false);
		
		//	Status bars
		m_ExtractTrackStatusBar->Reset();
		m_ExtractCDStatusBar->Reset();
		
		//	Reset
		char leadText[512];
		sprintf(leadText, "Waiting...");
		m_ExtractTrackStatusBar->Reset(leadText, "");
		m_ExtractCDStatusBar->Reset(leadText, "");
				
		Unlock();
	}
}

//-------------------------------------------------------------------
//	GetCurrentTrackPercentComplete();
//-------------------------------------------------------------------
//
//	Return float indicating how complete track copy is
//

float TProgressDialog::GetTrackComplete() 
{
	float percent = 0.0;
	scsi_position 	pos;
	
	//	Make sure we have a CD
	if (!m_CDID)
		return percent;
	
	//	Get scsi position
	if (ioctl(m_CDID, B_SCSI_GET_POSITION, &pos) == B_OK) 
	{ 	
		
		//int32 start = (m_SCSITOC.toc_data[4 + (m_CurrentTrack * 8) + 5] * 60) + (m_SCSITOC.toc_data[4 + (m_CurrentTrack * 8) + 6]); 
		//int32 len = ((m_SCSITOC.toc_data[4 + ((m_CurrentTrack + 1) * 8) + 5] * 60) + (m_SCSITOC.toc_data[4 + ((m_CurrentTrack + 1) * 8) + 6])) - start; 

		//	Calculate percentage
		int32 length 	= ((m_SCSITOC.toc_data[4 + ((m_CurrentTrack) * 8) + 5] * 60) + 
							m_SCSITOC.toc_data[4 + ((m_CurrentTrack) * 8) + 6]) - 
							((m_SCSITOC.toc_data[4 + (m_CurrentTrack * 8) + 5] * 60) + 
							m_SCSITOC.toc_data[4 + (m_CurrentTrack * 8) + 6]);
		int32 position 	= (pos.position[13] * 60) + pos.position[14];
		
		//PROGRESS("TProgressDialog::GetTrackComplete() - start: %ld -\n", start);
		//PROGRESS("TProgressDialog::GetTrackComplete() - length: %ld -\n", len);
		//PROGRESS("TProgressDialog::GetTrackComplete() - m_CurrentTrack: %ld -\n", m_CurrentTrack);
		//PROGRESS("TProgressDialog::GetTrackComplete() - length: %ld -\n", length);
		//PROGRESS("TProgressDialog::GetTrackComplete() - position: %ld-\n", position);
		
		percent = (float)position / (float)length; 
	}
	else
	{
		ERROR("TProgressDialog::GetTrackComplete() - Unable to get SCSI Position. -\n");
	}
	
	return percent;
}

//-------------------------------------------------------------------
//	SetupTrackStatus
//-------------------------------------------------------------------
//
//	Setup track status bar based on seconds length
//

void TProgressDialog::SetupTrackStatus(track_info *trackData, float secs) 
{
	//if (m_ExtractTrackStatusBar->LockLooperWithTimeout(kLockTimeout))
	if (m_ExtractTrackStatusBar->LockLooper())
	{				
		m_ExtractTrackStatusBar->Reset();
		m_ExtractTrackStatusBar->SetMaxValue(secs);
		
		//	Update labels
		char leadText[512];
		sprintf(leadText, "Extracting: %s", trackData->title);
		m_ExtractTrackStatusBar->SetText(leadText);	
		m_ExtractTrackStatusBar->SetTrailingText("");	
		m_ExtractTrackStatusBar->Invalidate();
		m_ExtractTrackStatusBar->UnlockLooper();
	}
}

//-------------------------------------------------------------------
//	SetupEncoderStatus
//-------------------------------------------------------------------
//
//	Setup encoder status bar
//

void TProgressDialog::SetupEncoderStatus(BEntry *entry) 
{
	//if (m_EncodeStatusBar->LockLooperWithTimeout(kLockTimeout))
	if (m_EncodeStatusBar->LockLooper())
	{
		//	Get process filename
		char fileName[B_FILE_NAME_LENGTH];			 	
		entry->GetName(fileName);			 						
	
		//	Update labels
		char leadText[512];
		sprintf(leadText, "Encoding: %s", fileName);
		
		int32 items = m_ProcessList->CountItems() - 1;		
		if (items)
			sprintf(leadText, "%ld Files Remaining", items);
		else
			sprintf(leadText, "No Files Remaining");
			
		//	Set value
		m_EncodeStatusBar->Reset();
		m_EncodeStatusBar->SetMaxValue(0);
		
		//	Update labels			
		m_EncodeStatusBar->SetTrailingText(leadText);
		sprintf(leadText, "Encoding: %s", fileName);
		m_EncodeStatusBar->SetText(leadText);
		m_EncodeStatusBar->Invalidate();		
		m_EncodeStatusBar->UnlockLooper();
	}
}

#pragma mark -
#pragma mark === Thread Routines ===

//-------------------------------------------------------------------
//	cd_watcher
//-------------------------------------------------------------------
//
//	Static thread function
//

int32 TProgressDialog::cd_watcher(void *arg) 
{
	TProgressDialog *obj = (TProgressDialog *)arg;
	return(obj->CDWatcher() );		
}


//---------------------------------------------------------------------
//	CDWatcher
//---------------------------------------------------------------------
//
//	Watch cd and detect when an audio CD is mounted.  When a CD is 
//	mounted, extact audio tracks.
//
		
int32 TProgressDialog::CDWatcher()
{			
	//	Keep looping...
	while(true)
	{
		bool		gotone = false; 
		status_t   	media_status; 
		
		snooze(100 * 1000);
		
		if (!m_CDID) 
		{ 
			acquire_sem(m_CDSem);
			char *cdName = ((RipperApp *)(be_app))->CDPath();
			if ((m_CDID = open(cdName, 0)) <= 0) 
			{ 
				ERROR("TProgressDialog::CDWatcher() - Couldn't open cd drive. -\n");
				m_CDID = 0; 
			} 
			else 
			{ 
				if(ioctl(m_CDID, B_GET_MEDIA_STATUS, &media_status, sizeof(media_status)) >= 0 &&
				(media_status == B_DEV_NO_MEDIA || media_status == B_DEV_NOT_READY)) 
				{ 
					PRINTF("TProgressDialog::CDWatcher() - Busy or empty drive. -\n");
					close(m_CDID); 
					m_CDID = 0; 
					release_sem(m_CDSem); 
					goto done; 
				}
			
				if (ioctl(m_CDID, B_SCSI_GET_TOC, &m_SCSITOC) < B_NO_ERROR)
				{ 
					PRINTF("TProgressDialog::CDWatcher() - Could not read CD TOC. -\n");
					close(m_CDID); 
					m_CDID = 0; 
					release_sem(m_CDSem); 
					goto done; 
				} 
			
				m_MinTrack = m_SCSITOC.toc_data[2]; 
				m_MaxTrack = m_SCSITOC.toc_data[3]; 
			
				//	Clear out old title list
				if (m_TitleList)
				{
					PRINTF("Clearing title list -\n");
					for (int32 index = 0; index < m_TitleList->CountItems(); index++)
					{
						track_info *albumData = (track_info  *)m_TitleList->ItemAt(index);
						if (albumData)
							free(albumData);
					}		
					delete m_TitleList;
					m_TitleList = NULL;
				}
				
				//	Create new title list
				m_TitleList = new BList();
				
				//	Add album data	
				track_info *album_data = (track_info*)malloc(sizeof(track_info)); 
				album_data->flags = 0; 
				sprintf(album_data->title, "Audio CD");
				album_data->extract = true;
				m_TitleList->AddItem(album_data);
								
				//	Add track data			
				int32 	start; 
				int32	len;

				m_TotalTracks = 0;
				while (m_TotalTracks <= (m_SCSITOC.toc_data[3] - m_SCSITOC.toc_data[2]))
				{ 
					track_info *track_data = (track_info*)malloc(sizeof(track_info)); 
					track_data->flags = 0; 					
					sprintf(track_data->title, "Track %d", m_TotalTracks + 1);
					track_data->extract = true;
												
					start = (m_SCSITOC.toc_data[4 + (m_TotalTracks * 8) + 5] * 60) + (m_SCSITOC.toc_data[4 + (m_TotalTracks * 8) + 6]); 
					len = ((m_SCSITOC.toc_data[4 + ((m_TotalTracks + 1) * 8) + 5] * 60) + (m_SCSITOC.toc_data[4 + ((m_TotalTracks + 1) * 8) + 6])) - start; 
					track_data->length = len;
					
					//	Add data to list
					m_TitleList->AddItem(track_data);
					
					//	Increment total tracks
					m_TotalTracks++; 
				}
				
				//	Set status strings in dialog
				//if (LockWithTimeout(kLockTimeout))
				if (Lock())
				{
					char trackStr[32];
					sprintf(trackStr, "%ld", m_TotalTracks); 
					m_TracksString->SetText(trackStr);
					
					m_Artist.SetTo("Unknown");
					m_Album.SetTo("Untitled");
					m_Year.SetTo("");
					m_Genre = 0;
					
					m_ArtistString->SetText(m_Artist.String());
					m_AlbumString->SetText(m_Album.String());

					Unlock();
				}
				
				//	Add track names to ListView
				//if (LockWithTimeout(kLockTimeout))
				if (Lock())
				{
					for (int32 index = 1; index < m_TitleList->CountItems(); index++)
					{
						track_info *albumData = (track_info  *)m_TitleList->ItemAt(index);
						if (albumData)
						{										
							char id[10];
							
							//	Set up proper spacing
							if (index < 10)							
								sprintf(id, " %d.", index);
							else
								sprintf(id, "%d.", index);
								
							//	Create item and set strings
							TCDListItem *trackItem = new TCDListItem(m_TracksListView, index, id);
														
							//	Set ID
							trackItem->IDString(id);
							
							//	Set title
							trackItem->TitleString(albumData->title);
							
							//	Set time
							char str[255];
							sprintf(str, "%.2d:%.2d", albumData->length / 60, albumData->length % 60);
							trackItem->TimeString(str);
							
							//	Set status
							trackItem->StatusString("Pending");

							//	Add item to list
							m_TracksListView->AddItem(trackItem);
						}
					}
					m_TracksListView->UpdateScrollBar();
					Unlock();
				}				
							
				// Convert toc to cddb standard
				int32 numTracks = m_MaxTrack - m_MinTrack+1;
				for (int32 index = 9; index <9 + 8 * (numTracks + 1); index += 8)
				{
					m_CDDBTOC[(index-9)/8].min 	 = m_SCSITOC.toc_data[index];
					m_CDDBTOC[(index-9)/8].sec 	 = m_SCSITOC.toc_data[index+1];
					m_CDDBTOC[(index-9)/8].frame = m_SCSITOC.toc_data[index+2];
				}
				
				//	Set flag saying we are ready to use CDDB
				m_CanUseCDDB = true;
								
				//	Set flag indicating we found a CD
				gotone = true;
			} 
			release_sem(m_CDSem); 
		}
		//else
		//{
		//	PROGRESS("TProgressDialog::CDWatcher() - CD Loaded...\n");
		//}

	done: 
		if (m_CDID) 
		{ 
			//	Found new disc
			if (gotone) 
			{ 
				PROGRESS("TProgressDialog::CDWatcher() - Found a new CD -\n");
				Enable();
				m_Changed = true;
				
				//	Check CDDB cache
				if (m_CDDBLooper)
				{
					BMessage msg(CDDB_CACHE_LOOKUP);
					msg.AddInt8("numTracks", m_TotalTracks);
					msg.AddPointer("cddb_toc", m_CDDBTOC);
					m_CDDBLooper->PostMessage(&msg, NULL, this);
				}

			}
			//	Make sure valid disc is loaded
			else
			{
			 	scsi_position pos;
			 	if (ioctl(m_CDID, B_SCSI_GET_POSITION, &pos) != B_NO_ERROR) 
			 	{ 
					Disable();
					acquire_sem(m_CDSem); 
					m_CDID = 0; 
					release_sem(m_CDSem);
					m_Changed = false;
					StopExtraction();
                } 
			}
		} 
		else 
		{
			if (m_Changed)
			{
				PROGRESS("TProgressDialog::CDWatcher() - Did not find a new CD -\n");
				Disable();
				m_Changed = false;
				StopExtraction();
			} 
		}
	}
		
	return true;
}

//-------------------------------------------------------------------
//	compressor
//-------------------------------------------------------------------
//
//	Static thread function
//

int32 TProgressDialog::compressor(void *arg) 
{
	TProgressDialog *obj = (TProgressDialog *)arg;
	return(obj->Compressor() );		
}


//-------------------------------------------------------------------
//	Compressor
//-------------------------------------------------------------------
//
//	Watch process list for new items and compress them according
//	to user preferences.
//

int32 TProgressDialog::Compressor()
{			
	status_t err;
	
	BMediaFile 	*inputFile   = NULL;
	BMediaFile 	*mediaOut    = NULL;
	char 		*soundBuffer = NULL;
	
	//	Keep looping...
	while(m_Compress)
	{
		snooze(500 * 1000);
		
		//	Check for new item to process
		if (m_ProcessList && m_ProcessList->CountItems() > 0)
		{
			//PROGRESS("TProgressDialog::Compressor() - Found an item to compress. -\n");
			
			//	Get first item it list and encode it
			BEntry *fileEntry = (BEntry *)m_ProcessList->ItemAt(0);
			if (fileEntry)
			{			 					
				//	Get item and set mime type and ID3 info
				TEncodeListItem *encodeItem = (TEncodeListItem *)m_EncoderListView->FirstItem();				
				ASSERT(encodeItem);
				ID3_tag *tag = encodeItem->ID3Tag();
				
				//	Set status to "Encoding"
				//if (m_EncoderListView->LockLooperWithTimeout(kLockTimeout))
				if (m_Compress && m_EncoderListView->LockLooper())
				{
					encodeItem->Locked(true);
					encodeItem->Pending(false);
					encodeItem->StatusString("Encoding");
					m_EncoderListView->Invalidate(m_EncoderListView->ItemFrame(0));
					m_EncoderListView->UnlockLooper();
				}
			 						
			 	//	Get process file path
			 	BPath filePath;
			 	fileEntry->GetPath(&filePath);
			 	char inFile[1024];
			 	sprintf(inFile, "%s", filePath.Path());
			 	char outFileName[1024];
			 	sprintf(outFileName, "%s.mp3", filePath.Path());
			 	
			 	//	Get directory
			 	BDirectory dir;
			 	err = fileEntry->GetParent(&dir);
			 	if (err != B_OK)
			 	{
			 		ERROR("TProgressDialog::Compressor() - Error getting directory: %s\n", strerror(err));	
			 		break;
			 	}
			 	
			 	//	Create output file
			 	BFile outFile;			 	
			 	err = dir.CreateFile(outFileName, &outFile, false);
			 	if (err != B_OK)
			 	{
			 		ERROR("TProgressDialog::Compressor() - Error creating file: %s\n", strerror(err));	
			 		break;
			 	}
			 	
			 	const bool encodeMP3 = true;
			 	if (encodeMP3)
			 	{
				 	//	Update encode status bar
					SetupEncoderStatus(fileEntry);
					
					//	Create media file for data read
					entry_ref inputRef;
					err = fileEntry->GetRef(&inputRef);
					if (err != B_OK)
				 	{
				 		ERROR("TProgressDialog::Compressor() - Error getting input ref: %s\n", strerror(err));	
				 		break;
				 	}
					
					if (inputFile)
						delete inputFile;

					inputFile = new BMediaFile(&inputRef);
					err = inputFile->InitCheck();
					if (err != B_OK)
				 	{
				 		ERROR("TProgressDialog::Compressor() - Error creating input media file: %s\n", strerror(err));	
				 		break;
				 	}
				 	
				 	//	Get audio track and format
				 	media_format format;
				 	BMediaTrack *inTrack = GetAudioTrack(inputFile, format);
				 	if (!inTrack)
				 	{
				 		ERROR("TProgressDialog::Compressor() - Error getting input audio track -\n");	
				 		break;				 	
				 	}
				 	
				 	//	Get format and set up buffer
				 	inTrack->EncodedFormat(&format);
					inTrack->DecodedFormat(&format);
					
					if (soundBuffer)
						free(soundBuffer);
						
					soundBuffer = (char*) malloc(format.u.raw_audio.buffer_size);
					//int64 frameSize = (format.u.raw_audio.format&15)*format.u.raw_audio.channel_count;
				 						
					//	Find the right file format handler
					bool  foundWriter = false;
					int32 cookie = 0;
					media_file_format mfi;
					while(get_next_file_format(&cookie, &mfi) == B_OK) 
					{
						if (strcmp(mfi.short_name, "wav") == 0)
						{
							foundWriter = true;
							break;
						}
					}
				 				 
					//	Create output media file
					if (foundWriter)
					{
						if (mediaOut)
							delete mediaOut;
							
						mediaOut = new BMediaFile(&outFile, &mfi);
						err = mediaOut->InitCheck();
						if (err != B_OK) 
						{
							ERROR("TProgressDialog::Compressor() - Error creating output media file: %s\n", strerror(err));
							break;
						}
						
						//	Get encoder						
						media_codec_info    mci;						
						bool 				foundEncoder = false;
						media_format		outfmt;
						cookie = 0;
						while (get_next_encoder(&cookie, &mfi, &format, &outfmt, &mci) == B_OK) 
						{
							PROGRESS("found encoder %s (%d)\n", mci.pretty_name, mci.id);
							
							if (strcmp("mp3", mci.short_name) == 0)
							{
								PROGRESS("TProgressDialog::Compressor() - Found encoder -\n");
								foundEncoder = true;
								break;
							}
						}
												
						PROGRESS("Encoder used %s (%d)\n", mci.pretty_name, mci.id);
						
						//	Create track
						if (foundEncoder)
						{
							BMediaTrack *outTrack = mediaOut->CreateTrack(&format, &mci);
							if (!outTrack)
							{
								ERROR("TProgressDialog::Compressor() - Error creating media track -\n");
								break;
							}
							
							//	Commit header
							mediaOut->CommitHeader();
							
							//	Set up status bar
							media_header mh;
							int64 numFrames  = inTrack->CountFrames();
							int64 frameCount = 0;
							
							//if (m_EncodeStatusBar->LockLooperWithTimeout(kLockTimeout))
							if (m_Compress && m_EncodeStatusBar->LockLooper())
							{
								m_EncodeStatusBar->SetMaxValue(numFrames);
								m_EncodeStatusBar->UnlockLooper();
							}
								
							//	Decode and encode data
							for (int32 j = 0; j < numFrames; j += frameCount) 
							{
								err = inTrack->ReadFrames(soundBuffer, &frameCount, &mh);
								if (err) 
								{
									ERROR("TProgressDialog::Compressor() - GetNextChunk() error: %s\n", strerror(err));
									break;
								}
							
								err = outTrack->WriteFrames(soundBuffer, frameCount);
								if (err) 
								{
									ERROR("TProgressDialog::Compressor() - %s (0x%x) writing audio frame %Ld\n", strerror(err), err, j);
									break;
								}
								
								//	Mark encoding progress						
								//if (m_EncodeStatusBar->LockLooperWithTimeout(kLockTimeout))
								if (m_Compress && m_EncodeStatusBar->LockLooper())
								{
									m_EncodeStatusBar->Update(frameCount);
									m_EncodeStatusBar->UnlockLooper();
								}
								
								//	Make sure thread hasn't been killed
								if (m_Compress == false)
								{
									j = numFrames + 1;
								}
							}
						}
					}
										
					//	Delete source file if pref set					
					if (m_DeleteSource)
					{
						err = fileEntry->Remove();					
						if ( err != B_OK)
							ERROR("TProgressDialog::Compressor() - Error deleting source file: %s -\n", strerror(err));
					}
					
					//	Set file mime type
					BNodeInfo info(&outFile);
					if ( info.InitCheck() == B_OK )
						info.SetType("audio/x-mpeg");
					else
						ERROR("TProgressDialog::Compressor() - Unable to set type: %s -\n", strerror(info.InitCheck()));
						
					//	Set attributes
					PROGRESS("TProgressDialog::Compressor() - Writing attributes... -\n");
					outFile.WriteAttr("Audio:SongName", B_STRING_TYPE, 0, tag->songname, strlen(tag->songname)+1);
					outFile.WriteAttr("Audio:Artist", B_STRING_TYPE, 0, tag->artist,strlen(tag->artist)+1);
					outFile.WriteAttr("Audio:Album", B_STRING_TYPE, 0, tag->album,strlen(tag->album)+1);
					outFile.WriteAttr("Audio:Year", B_STRING_TYPE, 0, tag->year,strlen(tag->year)+1);
					outFile.WriteAttr("Audio:Comment", B_STRING_TYPE, 0, tag->comment,strlen(tag->comment)+1);
					//outFile.WriteAttr("Audio:GenreID", B_UINT32_TYPE, 0, (void *)tag->genre, 1);
					//if (tag->genre >= 0 && tag->genre < genre_count)
					//	outFile.WriteAttr("Audio:Genre", B_STRING_TYPE, 0, genre_table[tag->genre],strlen(genre_table[tag->genre])+1);
																
					//	Write ID3 tags
					write_tag(false, outFileName, tag);
	
					//	Remove item from list and queue list view
					RemoveFromEncoderQueue(fileEntry);
	
					//	Notify that track is encoded
					if (m_EncodeDoneBeep)
						PlayStatusSound("EncodeDone");
						
					//	Reset status bar if no items are left
				 	//if (m_ProcessList->CountItems() == 0 && m_EncodeStatusBar->LockLooperWithTimeout(kLockTimeout))
				 	if (m_ProcessList->CountItems() == 0 && m_EncodeStatusBar->LockLooper())
					{	
						m_EncodeStatusBar->Reset("Waiting...", "");
						m_EncodeStatusBar->UnlockLooper();
					}
					
					//	Clean up
					if (inputFile)
					{
						inputFile->CloseFile();
						delete inputFile;
						inputFile = 0;
					}

					if (mediaOut)
					{
						mediaOut->CloseFile();
						delete mediaOut;
						mediaOut = 0;
					}
					
					if (soundBuffer)
					{													
						free(soundBuffer);
						soundBuffer = 0;
					}
				}
			}
		}
	}
	
	//	Clean up
	if (inputFile)
	{
		inputFile->CloseFile();
		delete inputFile;
		inputFile = 0;
	}
	
	if (mediaOut)
	{
		inputFile->CloseFile();
		delete mediaOut;
		mediaOut = 0;
	}
	
	if (soundBuffer)
	{													
		free(soundBuffer);
		soundBuffer = 0;
	}
	
	return true;
}

//-------------------------------------------------------------------
//	extractor
//-------------------------------------------------------------------
//
//	Static extracort thread function
//

int32 TProgressDialog::extractor(void *arg) 
{
	TProgressDialog *obj = (TProgressDialog *)arg;
	return(obj->Extractor() );		
}

//-------------------------------------------------------------------
//	Extractor
//-------------------------------------------------------------------
//
//	Extract CD files

int32 TProgressDialog::Extractor()
{			
	bool retVal;
	
	while(true)
	{
		while (m_StartExtraction)
		{
			retVal = StartExtraction();
			StopExtraction();
			
			//	If retVal came back false, stop extraction.
			//	It probably means we are out of disk space
			if (retVal == false)
			{
				m_StartExtraction = false;
				
				//	Update button enabled states
				//if (LockWithTimeout(kLockTimeout))
				if (Lock())
				{
					m_StartButton->SetEnabled(true);
					m_CDDBButton->SetEnabled(true);
					m_CancelButton->SetEnabled(false);
					Unlock();
				}
			}
		}
		
		//	Sleep until it is time to start
		snooze(100 * 1000);
	}
}


//-------------------------------------------------------------------
//	StartCompressor
//-------------------------------------------------------------------
//
//	Start compression
//

void TProgressDialog::StartCompressor()
{
	//	Spawn compressor thread
	if (m_CompressorThread == 0)
	{
		//PROGRESS("TProgressDialog::StartCompressor() - Spawning thread -\n");		
		m_Compress = true;
		
		m_CompressorThread = spawn_thread( compressor, "compressor", B_NORMAL_PRIORITY, (void *)this);
		if( m_CompressorThread != B_NO_MORE_THREADS && m_CompressorThread != B_NO_MEMORY)
		{
			resume_thread(m_CompressorThread);
		}
		else
		{
			ERROR("TProgressDialog::StartCompressor() - Error creating compressor! -\n");
		}
	}
}

//-------------------------------------------------------------------
//	StopCompressor
//-------------------------------------------------------------------
//
//	Stop compression
//

void TProgressDialog::StopCompressor()
{
	//	Kill compressor thread
	if (m_CompressorThread)
	{
		status_t err;
		status_t retVal;
		
		m_Compress = false;
		err = wait_for_thread (m_CompressorThread, &retVal);
		//kill_thread(m_CompressorThread);
		m_CompressorThread = 0;
		
		//	Reset status
		m_EncodeStatusBar->Reset();
		m_EncodeStatusBar->Reset("Waiting", "");
	}
}


#pragma mark -
#pragma mark === Extraction Routines ===
								
//-------------------------------------------------------------------
//	StartExtraction
//-------------------------------------------------------------------
//
//	Extract tracks from CD
//

bool TProgressDialog::StartExtraction()
{
	int32 	 result;
	status_t err;
	
	//	Make sure we have a valid file descriptor
	if (!m_CDID)
	{
		ERROR("TProgressDialog::StartExtraction() - Invalid file descriptor -\n");
		ErrorAlert("Unable to access compact disc drive.");
		return false;
	}
	
	//	Read in device TOC
	if((result = ioctl(m_CDID, B_SCSI_GET_TOC, m_TOC)) != 0)
	{
		ERROR("TProgressDialog::StartExtraction() - Error reading table of contents -\n");
		ErrorAlert("Error reading compact disc.");
		return false;
	}
	
	//	Get total tracks to extract and total time
	float totalSecs = 0;
	int32 tracksToExtract = 0;
	CDDA_MSF current, end;
	
	//	The total time is based on tracks set for extraction
	for (int32 index = 1; index < m_TitleList->CountItems(); index++)
	{
		//	Get data from track list		
		track_info *trackData = (track_info *)m_TitleList->ItemAt(index);
		if (trackData && trackData->extract)
		{
			//	Get track time
			current.m = m_TOC->track[(index-1) + (m_TOC->first_track-1)].address.m;
			current.s = m_TOC->track[(index-1) + (m_TOC->first_track-1)].address.s;
			current.f = m_TOC->track[(index-1) + (m_TOC->first_track-1)].address.f;
		
			end.m = m_TOC->track[(index-1) + m_TOC->first_track].address.m;
			end.s = m_TOC->track[(index-1) + m_TOC->first_track].address.s;
			end.f = m_TOC->track[(index-1) + m_TOC->first_track].address.f;
						
			//	Increment total seconds
			totalSecs += (float)((end.m - current.m) * 60) + (end.s - current.s);

			//	Increment total track to extract
			tracksToExtract++;
		}
	}
			
	//	Set up track list
	BEntry 	entry;
	BFile 	file;
		
	//	Reset status and set up ExtractCDStatusBar progress bar
	//if (m_ExtractCDStatusBar->LockLooperWithTimeout(kLockTimeout))
	if (m_ExtractCDStatusBar->LockLooper())
	{
		m_ExtractCDStatusBar->Reset();
		m_ExtractCDStatusBar->SetMaxValue(totalSecs);
		m_ExtractCDStatusBar->UnlockLooper();
	}
	
	//	Extract tracks
	for (m_CurrentTrack = m_TOC->first_track; m_CurrentTrack <= m_TOC->last_track; m_CurrentTrack++)
	{
		//	Now we have the TOC for the CD in standard SCSI-2 format, even for
		//	ATAPI CD-ROMs, since they just do SCSI-2 and a bit over IDE bus anyway
		if((m_CurrentTrack > m_TOC->last_track) || (m_CurrentTrack < m_TOC->first_track) ||
			// Track numbers normally start at 1, but they do not have to...
		   (m_TOC->track[(m_CurrentTrack-1)+(m_TOC->first_track-1)].track_number != m_CurrentTrack))
		{
			ERROR("TProgressDialog::StartExtraction() - Error: Can't find track %d in the table of contents. -\n", m_CurrentTrack);
			ErrorAlert("Can't find track in the table of contents.");
			return false;
		}
				
		//	Get data from track list		
		char fileName[B_FILE_NAME_LENGTH];
		char directoryName[B_FILE_NAME_LENGTH];
		track_info *trackData = (track_info *)m_TitleList->ItemAt(m_CurrentTrack);
		ASSERT(trackData);
		
		//	Only proceed if track is flagged for extraction
		if (trackData->extract)
		{
			//	Lock track down
			TCDListItem *extractListItem = (TCDListItem *)m_TracksListView->ItemAt(m_CurrentTrack - 1);
			ASSERT(extractListItem);
			//if (m_TracksListView->LockLooperWithTimeout(kLockTimeout))
			if (m_TracksListView->LockLooper())
			{
				extractListItem->Locked(true);
				extractListItem->StatusString("Extracting");
				m_TracksListView->InvalidateItem(m_CurrentTrack - 1);
				m_TracksListView->UnlockLooper();
			}
			
			//	Create file and directory names
			sprintf(fileName, "%s", trackData->title);
			sprintf(directoryName, "%s", m_AlbumString->Text());
			
			//	Check for user naming prefs
			char appendStr[B_FILE_NAME_LENGTH];
			if (m_AddArtist)
				sprintf(appendStr, "%s - ", m_ArtistString->Text());	
			
			if (m_AddAlbum)
			{
				char copyStr[B_FILE_NAME_LENGTH];
				strcpy(copyStr, appendStr);
				sprintf(appendStr, "%s%s - ", copyStr, m_AlbumString->Text());	
			}

			if (m_AddTrack)
			{
				char copyStr[B_FILE_NAME_LENGTH];
				strcpy(copyStr, appendStr);
				sprintf(appendStr, "%s%d. ", copyStr, extractListItem->ID());
			}
			
			//	Append to file name
			if (m_AddArtist || m_AddAlbum || m_AddTrack )
			{
				char copyStr[B_FILE_NAME_LENGTH];
				strcpy(copyStr, fileName);
				sprintf(fileName, "%s%s", appendStr, copyStr);
			}
				
			//	Remove pesky '/' from song title.  Thanks Steve...
			for (char *c = fileName; *c != 0; c++)
			{
				if (*c == '/')
				{
					*c = ' ';
					ERROR("TProgressDialog::StartExtraction() - Error removing '/' from song title. -\n");
				}
			}
			
			//	Remove pesky '/' from album title.  Thanks Steve...
			for (char *c = directoryName; *c != 0; c++)
			{
				if (*c == '/')
				{
					*c = ' ';
					ERROR("TProgressDialog::StartExtraction() - Error removing '/' from album title. -\n");
				}
			}
			
			//	Create the file
			err = MakeFile(&entry, &file, fileName, directoryName);
			if (err != B_OK)
			{
				ERROR("TProgressDialog::StartExtraction() - Error: Can't create file. -\n");
				ErrorAlert("Unable to create file: ", err);
				return false;
			}
									
			//	Perform CDDA extraction on audio only
			if((m_TOC->track[(m_CurrentTrack-1)+(m_TOC->first_track-1)].flags & CDDA_TOC_FLAG_DATA_TRACK) != 0)
			{
				ERROR("TProgressDialog::StartExtraction() - Error: Track %d contains data, not audio\n", m_CurrentTrack);
				ErrorAlert("Track does not contain audio data.");
				return false;
			}
			
			//	Update ExtractCDStatusBar status bar
			//if (m_ExtractCDStatusBar->LockLooperWithTimeout(kLockTimeout))
			if (m_ExtractCDStatusBar->LockLooper())
			{
				PROGRESS("TProgressDialog::StartExtraction() - Reading %s, track %d -\n", fileName, m_CurrentTrack);
				
				//	Update lead text
				char leadText[512];
				sprintf(leadText, "Extracting Track %ld", m_CurrentTrack);
				m_ExtractCDStatusBar->SetText(leadText);	
				
				//	Update trailing text
				char trailText[512];
				if ((tracksToExtract - 1) == 0)
					sprintf(trailText, "No Tracks Remaining");
				else
					sprintf(trailText, "%d Tracks Remaining", tracksToExtract - 1);
				m_ExtractCDStatusBar->SetTrailingText(trailText);
				
				//	Force redraw				
				m_ExtractCDStatusBar->Invalidate();
				m_ExtractCDStatusBar->UnlockLooper();
			}
			
			current.m = m_TOC->track[(m_CurrentTrack-1)+(m_TOC->first_track-1)].address.m;
			current.s = m_TOC->track[(m_CurrentTrack-1)+(m_TOC->first_track-1)].address.s;
			current.f = m_TOC->track[(m_CurrentTrack-1)+(m_TOC->first_track-1)].address.f;
		
			end.m = m_TOC->track[(m_CurrentTrack-1)+(m_TOC->first_track-1)+1].address.m;
			end.s = m_TOC->track[(m_CurrentTrack-1)+(m_TOC->first_track-1)+1].address.s;
			end.f = m_TOC->track[(m_CurrentTrack-1)+(m_TOC->first_track-1)+1].address.f;
		
			//	Dump status
			//PRINTF("TProgressDialog::StartExtraction() - minutes: %ld\n", end.m - current.m);
			//PRINTF("TProgressDialog::StartExtraction() - seconds: %ld\n", end.s - current.s);
			//PRINTF("TProgressDialog::StartExtraction() - frames: %ld\n", end.f - current.f);
			
			//	Build the READ_CD command
			m_ReadCommand->start_m = current.m;
			m_ReadCommand->start_s = current.s;
			m_ReadCommand->start_f = current.f;
		
			m_ReadCommand->length_m = 0;
			m_ReadCommand->length_s = 0;
			m_ReadCommand->length_f = 1;
			
			m_ReadCommand->buffer_length = sizeof(CDDA_BLOCK);
			m_ReadCommand->buffer 		 = (char *)m_FrameBuffer;
			m_ReadCommand->play 		 = false;	// Obsolete flag
			
					
			//	Set up media_format for audio file		
			media_format mf;
			mf.type 						= B_MEDIA_RAW_AUDIO;		
			mf.u.raw_audio.format 			= media_raw_audio_format::B_AUDIO_SHORT;
			mf.u.raw_audio.frame_rate 		= 44100.000000;
			mf.u.raw_audio.channel_count 	= 2;
			mf.u.raw_audio.byte_order 		= 2;
	
			//	Create output media file
			BMediaFile *mediaFile = CreateOutputMediaFile(&file);
			if (!mediaFile)
			{
				ErrorAlert("Unable to create output file.");
				return false;
			}
			
			//	Create track		
			BMediaTrack *track = mediaFile->CreateTrack(&mf);
			mediaFile->CommitHeader();
			
			//	Reset and set up ExtractTrackStatusBar progress bar
			float tracksSecs = (float)((end.m - current.m) * 60) + (end.s - current.s);
			SetupTrackStatus(trackData, tracksSecs);
							
			//	Read one CDDA block at a time
			uint8 lastSec = current.s;
			while(m_StartExtraction)
			{
				if((result = ioctl(m_CDID, B_SCSI_READ_CD, m_ReadCommand)) != 0)
				{
					ERROR("TProgressDialog::StartExtraction() - Error: ioctl(B_SCSI_READ_CD) returned with %d. \n", result);				
					ErrorAlert("Unable to read compact disc.");
					return false;
				}
				
				//	Dump block to destination
				status_t retVal = track->WriteFrames(m_FrameBuffer, sizeof(CDDA_BLOCK));
				if (retVal == B_OK)
				{			
					//	Don't read the last block, it belongs to next track
					BumpMSF(&current);
					
					//	At the end of the track.  Break out...
					if(MSFEqual(current, end) == true) 
						break;
			
					m_ReadCommand->start_m = current.m;
					m_ReadCommand->start_s = current.s;
					m_ReadCommand->start_f = current.f;
				}
				else
				{
					ERROR("TProgressDialog::StartExtraction() - File Write() error %s. \n", strerror(retVal));
					ErrorAlert("Unable to write to output file: ", err);
					return false;
				}
				
				//	Mark progress every second
				if (lastSec != current.s)
				{
					//	Mark track progress
					//if (m_ExtractTrackStatusBar->LockLooperWithTimeout(kLockTimeout))
					if (m_ExtractTrackStatusBar->LockLooper())
					{
						m_ExtractTrackStatusBar->Update(1.0);
						m_ExtractTrackStatusBar->UnlockLooper();
					}
					
					//	Mark CD progress
					//if (m_ExtractCDStatusBar->LockLooperWithTimeout(kLockTimeout))
					if (m_ExtractCDStatusBar->LockLooper())
					{
						m_ExtractCDStatusBar->Update(1.0);
						m_ExtractCDStatusBar->UnlockLooper();
					}
					
					lastSec = current.s;		
				}
			}			
			
			//	Close and free media file
			mediaFile->CloseFile();
			delete mediaFile;
			mediaFile = 0;
			
			//	Make sure we are still extracting
			if (m_StartExtraction == false)
				return false;
					
			//	Done extracting track.  Add path to process list.
			BEntry *newEntry = new BEntry(entry);
			m_ProcessList->AddItem(newEntry);
			
			//	Add item to encoder list
			track_info *trackData = (track_info *)m_TitleList->ItemAt(m_CurrentTrack);
			char str[255];
			sprintf(str, "%.2d:%.2d", trackData->length / 60, trackData->length % 60);			
			AddToEncoderQueue(newEntry, extractListItem->ID3Tag(), str);
					
			//	Update encoder status bar
			//if (m_EncodeStatusBar->LockLooperWithTimeout(kLockTimeout))
			if (m_EncodeStatusBar->LockLooper())
			{
				char encodeText[512];
				int32 items = m_ProcessList->CountItems() - 1;		
				if (items)
					sprintf(encodeText, "%ld Files Remaining", items);
				else
					sprintf(encodeText, "No Files Remaining");												
	
				m_EncodeStatusBar->SetTrailingText(encodeText);	
				m_EncodeStatusBar->Invalidate();
				m_EncodeStatusBar->UnlockLooper();
			}
			
			//	Decrement tracksToExtract variable so status text is update properly
			tracksToExtract--;
			
			//	Unlock track
			if (extractListItem)
			{
				//if (m_TracksListView->LockLooperWithTimeout(kLockTimeout))
				if (m_TracksListView->LockLooper())
				{
					extractListItem->Locked(false);
					extractListItem->Pending(false);
					extractListItem->StatusString("Queued");
					m_TracksListView->InvalidateItem(m_CurrentTrack - 1);
					m_TracksListView->UnlockLooper();
				}
			}
			
			//	Audio alert that track is done
			if (m_TrackDoneBeep)
				PlayStatusSound("TrackDone");
		}		
	}
	
	//	Audio alert that CD is done
	if (m_CDDoneBeep)
		PlayStatusSound("CDDone");

	return true;
}


//-------------------------------------------------------------------
//	StopExtraction
//-------------------------------------------------------------------
//
//

void TProgressDialog::StopExtraction()
{
	//	Set stop flag	
	m_StartExtraction 	= false;
	m_CanUseCDDB		= false;
	
	//	Close CD and reopen without letting CDWatcher reset lists
	if (m_CDID)
	{
		//	Reload CD
		acquire_sem(m_CDSem); 
		close(m_CDID);
		char *cdName = ((RipperApp *)(be_app))->CDPath();
		m_CDID = open(cdName, 0);
		release_sem(m_CDSem);
		
		//	Update controls
		Enable();
	}
	
	//	Reset status bars and clear old items
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		//	Clear out totally
		m_ExtractTrackStatusBar->Reset();
		m_ExtractCDStatusBar->Reset();
		
		//	Reset
		char leadText[512];
		sprintf(leadText, "Waiting...");
		m_ExtractTrackStatusBar->Reset(leadText, "");
		m_ExtractCDStatusBar->Reset(leadText, "");
		
		//	Unlock all list items
		for (int32 index = 0; index < m_TracksListView->CountItems(); index++)
		{
			TCDListItem *item = (TCDListItem *)m_TracksListView->ItemAt(index);
			if (item)
				item->Locked(false);					
		}
		
		//	Redraw list view
		m_TracksListView->Invalidate();
		
		Unlock();
	}
}


//-------------------------------------------------------------------
//	BumpMSF
//-------------------------------------------------------------------
//
//	Utility function to increment an MSF struct by one F
//

void TProgressDialog::BumpMSF(CDDA_MSF *a)
{
	a->f++;
	if(a->f > 74)
	{
		a->f = 0;
		a->s++;
		if(a->s > 59)
		{
			a->s = 0;
			a->m++;
		}
	}
}


//-------------------------------------------------------------------
//	MSFEqual
//-------------------------------------------------------------------
//
//	See if two MSF structures are equal
//

bool TProgressDialog::MSFEqual(CDDA_MSF a, CDDA_MSF b)
{
	if(a.f != b.f) return false;
	if(a.s != b.s) return false;
	if(a.m != b.m) return false;
	
	return true;
}



#pragma mark -
#pragma mark === File I/O ===


//-------------------------------------------------------------------
//	MakeFile
//-------------------------------------------------------------------
//
//

status_t TProgressDialog::MakeFile(BEntry *item, BFile *file, const char *filename, const char *directoryname) 
{ 
	status_t        result;
	BDirectory      dir; 
	BEntry          entry; 
	BMessage        msg;
	BNodeInfo       *node; 
	BPath           path; 
	
	/*
	//	Look for 'home' directory
	if ((result = find_directory(B_USER_DIRECTORY, &path)) != B_NO_ERROR) 
	{ 
		ERROR("TProgressDialog::MakeFile() - find_directory failed: %s\n", strerror(result)); 
		return result; 
	} 
	
	dir.SetTo(path.Path()); 
	if ((result = dir.InitCheck()) != B_NO_ERROR) 
	{ 
		ERROR("TProgressDialog::MakeFile() - error setting user directory: %s -\n", strerror(result)); 
		return result; 
	}
	*/
	
	/*
	//	Create path to "home" on capture device
	char volName[B_FILE_NAME_LENGTH];
	m_ExtractVolume.GetName(volName);
	sprintf(m_ExtractPathName, "/%s/%s/", volName, "home");
	PROGRESS("TProgressDialog::MakeFile() - Extract path: %s\n", m_ExtractPathName);
	*/
	
	//	Set directory to path
	dir.SetTo(m_ExtractPathName);
	
	//	Find album directory	
	result = dir.FindEntry(directoryname, &entry);
	if ( result != B_NO_ERROR) 
	{ 
		//	Create directory
		PROGRESS("TProgressDialog::MakeFile() - Creating directory: %s -\n", directoryname); 
		result = dir.CreateDirectory(directoryname, &dir);	
		if (result != B_NO_ERROR) 
		{ 
			ERROR("TProgressDialog::MakeFile() - error creating cd directory (%s): %s\n", directoryname, strerror(result)); 
			return result; 
		} 
	} 
	else 
	{ 
		dir.SetTo(&entry); 
		if ((result = dir.InitCheck()) != B_NO_ERROR) 
		{ 
			ERROR("TProgressDialog::MakeFile() - Error setting directory: %x -\n", result); 
			return result; 
		} 
	} 
	
	while (1) 
	{
		if (dir.CreateFile(filename, file, false) == B_NO_ERROR) 
		{ 
			node = new BNodeInfo(file); 
			node->SetType("audio/x-aiff"); 
			delete node; 
			dir.FindEntry(filename, item); 
			break; 
		}
		else
			ERROR("TProgressDialog::MakeFile() - Error creating file: %s -\n", filename); 
	}
	 	
	return B_NO_ERROR; 
}

//-------------------------------------------------------------------
//	CreateOutputMediaFile
//-------------------------------------------------------------------
//
//	Create media file for audio frame output
//

BMediaFile *TProgressDialog::CreateOutputMediaFile(BFile *file) 
{	
	status_t err;
		
	//	Locate audio encoder
	media_file_format mfi;
	
	bool foundEncoder = false;
	int32 cookie = 0;
	while((err = get_next_file_format(&cookie, &mfi)) == B_OK) 
	{
		if (strcmp(mfi.short_name, "AIFF") == 0)
		{
			foundEncoder = true;
			break;
		}
	}
			
	//	Make sure we found encoder
	if (!foundEncoder)
	{
		ERROR("TProgressDialog::CreateOutputMediaFile() - Unable to find media encoder. -\n");
		return NULL;		
	}
	
	//	Create BMediafile		
	BMediaFile *mediaFile = new BMediaFile(file, &mfi);
	if (mediaFile->InitCheck() != B_OK)
	{
		ERROR("TProgressDialog::CreateOutputMediaFile() - Unable to create media file: '%s'\n", strerror(mediaFile->InitCheck()));
		return NULL;
	}
	
	return mediaFile;
}

#pragma mark -
#pragma mark === CDDB Routines ===

//-------------------------------------------------------------------
//	CheckCDDB
//-------------------------------------------------------------------
//
//	Check with CDDB for track info
//

bool TProgressDialog::CheckCDDB() 
{
	//FUNCTION("TProgressDialog::CheckCDDB() \n");
	
	//	Do we have a valid looper
	if (!m_CDDBLooper)
	{
		ERROR("TProgressDialog::CheckCDDB() - No valid CDDB looper. -\n");
		return false;
	}
		
	if (!m_CanUseCDDB)
	{
		ERROR("TProgressDialog::CheckCDDB() - CDDB track info not ready. -\n");
		return false;
	}
		
	//	Check with CDDB
	BMessage msg(CDDB_LOOKUP);
	msg.AddInt8("numTracks", m_TotalTracks);
	msg.AddPointer("cddb_toc", m_CDDBTOC);
	m_CDDBLooper->PostMessage(&msg, NULL, this);
	
	return true;																						
}


//-------------------------------------------------------------------
//	SetCDInfo
//-------------------------------------------------------------------
//
//	Called to set the track/disc information from a cd_info structure
//

void TProgressDialog::SetCDInfo(cd_info *newInfo) 
{	
	//FUNCTION("TProgressDialog::SetCDInfo\n");		
	ASSERT(m_TitleList);
	
	//	Bail if we are extracting
	if (Extracting())
	{
		ERROR("TProgressDialog::SetCDInfo() - Cannot set info while extracting -\n");	
		return;
	}
		
	//	Get album data from list and update it
	track_info *albumData = (track_info  *)m_TitleList->ItemAt(0);
	if (albumData)
		strcpy(albumData->title, newInfo->disc_title);
		
	PRINTF("ALBUM TITLE: %s\n", albumData->title);
	
	//	Update track data	
	for(int32 index = 1; index < m_TitleList->CountItems(); index++)
	{ 
		PRINTF("TProgressDialog::SetCDInfo #1a\n");	
		track_info *trackData = (track_info  *)m_TitleList->ItemAt(index);
		if (trackData)
		{			
			sprintf(trackData->title, "%s", newInfo->track[index-1].track_name);
			PRINTF("TRACK TITLE: %s\n", trackData->title);
		}
	}
	
	PRINTF("TProgressDialog::SetCDInfo #2\n");	
	//	Set status strings in dialog
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		m_Artist.SetTo(newInfo->disc_artist);
		m_Album.SetTo(albumData->title);
		
		m_ArtistString->SetText(m_Artist.String());
		m_AlbumString->SetText(m_Album.String());

		Unlock();
	}
	
	PRINTF("TProgressDialog::SetCDInfo #3\n");	
	//	Update tracks in list
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		PRINTF("TProgressDialog::SetCDInfo #4\n");
		
		//	Clear out old items
		for (int32 index = m_TracksListView->CountItems(); index >= 0; index--)
		{
			TCDListItem *item = (TCDListItem *)m_TracksListView->RemoveItem((int32)0);
			if (item)
				delete item;
		}
		//m_TracksListView->MakeEmpty();
		
		PRINTF("TProgressDialog::SetCDInfo #5\n");	
		//	Add new items		
		for (int32 index = 1; index < m_TitleList->CountItems(); index++)
		{
			track_info *trackInfo = (track_info  *)m_TitleList->ItemAt(index);
			if (trackInfo)
			{										
				char id[10];
				
				//	Set up proper number spacing
				if (index < 10)							
					sprintf(id, " %d.", index);
				else
					sprintf(id, "%d.", index);
				
				//	Create list item
				TCDListItem *trackItem = new TCDListItem(m_TracksListView, index, id);
				
				//	Set ID
				trackItem->IDString(id);
				
				//	Set title
				trackItem->TitleString(trackInfo->title);
				
				//	Set time
				char str[255];
				sprintf(str, "%.2d:%.2d", trackInfo->length / 60, trackInfo->length % 60);
				trackItem->TimeString(str);

				//	Set status
				trackItem->StatusString("Pending");

				//	Add item
				m_TracksListView->AddItem(trackItem);
			}
		}				
		m_TracksListView->UpdateScrollBar();
		Unlock();
	}

}


//-------------------------------------------------------------------
//	UpdateCDDBServer
//-------------------------------------------------------------------
//
//	Update CDDB Looper's server
//

void TProgressDialog::UpdateCDDBServer()
{
	if (m_CDDBLooper)
		m_CDDBLooper->UpdateServer(m_CDDBServerName);
} 



#pragma mark -
#pragma mark === Encode Queue Routines ===

//-------------------------------------------------------------------
//	AddToEncoderQueue
//-------------------------------------------------------------------
//
//	Add entry name to encoder queue list view
//

void TProgressDialog::AddToEncoderQueue(BEntry *entry, ID3_tag *tag, char *time) 
{
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		char encodeName[B_FILE_NAME_LENGTH];
		entry->GetName(encodeName);
		TEncodeListItem *encodeItem = new TEncodeListItem(m_EncoderListView, this, tag, m_EncoderListView->CountItems()+1, encodeName);
		m_EncoderListView->AddItem(encodeItem);
		encodeItem->TimeString(time);
		m_EncoderListView->Invalidate(m_EncoderListView->ItemFrame(m_EncoderListView->IndexOf(encodeItem)));
		Unlock();
	}
}


//-------------------------------------------------------------------
//	RemoveFromEncoderQueue
//-------------------------------------------------------------------
//
//	Remove first item from queue and free entry from process list
//

void TProgressDialog::RemoveFromEncoderQueue(BEntry *entry) 
{
	//	Remove from queue list
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		TEncodeListItem *encodeItem = (TEncodeListItem *)m_EncoderListView->FirstItem();
		if (encodeItem)
		{
			m_EncoderListView->RemoveItem(encodeItem);
			delete encodeItem;
		}
		
		//	Remove from process list
		m_ProcessList->RemoveItem(entry);
		entry->Unset();
		delete entry;
		
		//	Renumber items
		for (int32 index = 0; index < m_EncoderListView->CountItems(); index++)
		{
			TEncodeListItem *renumItem = (TEncodeListItem *)m_EncoderListView->ItemAt(index);
			if (renumItem)
				renumItem->ID(index + 1);
		}
		
		Unlock();
	}
}


//-------------------------------------------------------------------
//	ShowSettingsDialog
//-------------------------------------------------------------------
//
//	Open settings window
//

void TProgressDialog::ShowSettingsDialog() 
{
	//	Show existing dialog
	if(m_SettingsDialog)
	{
		m_SettingsDialog->Show();
		m_SettingsDialog->Activate(true);	
	}
	//	Create new dialog
	else
	{
		// Create the status dialog from a resource archive
		BMessage *theMessage = GetWindowFromResource("SettingsWindow");
		m_SettingsDialog = new TSettingsDialog(this, theMessage);
		ASSERT(m_SettingsDialog);
		delete(theMessage);
		
		// Center it
		CenterWindow(m_SettingsDialog);
		
		// Show the dialog
		m_SettingsDialog->Show();
		m_SettingsDialog->Activate(true);
	}			
}


//-------------------------------------------------------------------
//	GetAudioTrack
//-------------------------------------------------------------------
//
//	Return audio track
//

BMediaTrack *TProgressDialog::GetAudioTrack(BMediaFile *file, media_format &format) 
{
	//	Count number of media tracks.  We really should only have one...
	int32 numTracks = file->CountTracks();
	if (numTracks <= 0)
	{
		ERROR("TProgressDialog::LoadAudioTrack() - Erorr: No tracks. -\n");
		return NULL;
	}

	//	Make sure they are valid
	BMediaTrack *track;
	
	for (int32 i = 0; i < numTracks; i++) 
	{
		//	Get track
		track = file->TrackAt(i);
		if (!track) 
		{
			ERROR("TProgressDialog::LoadAudioTrack() - Cannot construct BMediaTrack object -\n");
			return NULL;
		}
		
		//	Get the encoded format
		status_t err = track->EncodedFormat(&format);
		if (err != B_OK) 
		{
			ERROR("TAudioCue::LoadAudioFile() - BMediaTrack::EncodedFormat error: %s\n", strerror(err));
			return NULL;
		}
		
		switch(format.type) 
		{
			//	Handle raw audio track
			case B_MEDIA_RAW_AUDIO:
			case B_MEDIA_ENCODED_AUDIO:
				return track;
				break;
				
			default:
				break;
					
		}				
	}
	
	//	No audio track found
	return NULL;
}

#pragma mark -
#pragma mark === Pref Routines ===

//-------------------------------------------------------------------
//	SavePrefs
//-------------------------------------------------------------------
//
//	Save out prefs
//

bool TProgressDialog::SavePrefs() 
{
	//
	//	Save out prefs.  We just archive into a BMessage and flatten it to disk
	//
	
	BMessage prefsMsg;
	
	//	Extraction To Path
	prefsMsg.AddString("ExtractPath", m_ExtractPathName);
	
	//	CDDB server name
	prefsMsg.AddString("CDDBName", m_CDDBServerName);
		
	//	Delete Source File
	prefsMsg.AddInt32("DeleteSource", m_DeleteSource);

	//	Track done beep setting
	prefsMsg.AddInt32("TrackDoneBeep", m_TrackDoneBeep);
	
	//	CD done beep setting
	prefsMsg.AddInt32("CDDoneBeep", m_CDDoneBeep);
	
	//	Encode done beep setting
	prefsMsg.AddInt32("EncodeDoneBeep", m_EncodeDoneBeep);
	
	//	Add artist to title
	prefsMsg.AddInt32("AddArtist", m_AddArtist);

	//	Add album to title
	prefsMsg.AddInt32("AddAlbum", m_AddAlbum);

	//	Add track to title
	prefsMsg.AddInt32("AddTrack", m_AddTrack);
	
	//	Extractor pane setting
	prefsMsg.AddBool("ExtractorPanelOpen", m_ExtractorPanelOpen);
	
	//	Encoder pane setting
	prefsMsg.AddBool("EncoderPanelOpen", m_EncoderPanelOpen);
	
	//	Encoder settings
	prefsMsg.AddData("LayerInfo", B_RAW_TYPE, &m_LayerInfo, sizeof(layer) );
	prefsMsg.AddInt32("AcousticModel", m_AcousticModel);
	
	//	Dialog location
	m_Position.Set(Frame().left, Frame().top);
	prefsMsg.AddPoint("Position", m_Position);

	//
	//	Write file out
	//
		
	//	Get prefs folder
	BDirectory	dir; 
	BPath 		path;
	BEntry		entry;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path); 
	dir.SetTo(path.Path()); 
	if (dir.FindEntry("UltraEncode Folder", &entry) == B_OK)
	{
		//	Create prefs file		
		dir.SetTo(&entry);
		BFile prefFile;
		status_t retVal = dir.CreateFile("UltraEncode Prefs", &prefFile);
		if (retVal == B_OK)
		{			
			//	Write out data
			prefsMsg.Flatten(&prefFile);
		}
		else
		{
			ERROR("TProgressDialog::SavePrefs() - CreateFile() Error: %s -\n", strerror(retVal));
			return false;
		}
	}
	
	return true;
}

//-------------------------------------------------------------------
//	LoadPrefs
//-------------------------------------------------------------------
//
//	Load in prefs
//

bool TProgressDialog::LoadPrefs() 
{
	//	Get prefs folder
	BDirectory	dir; 
	BPath 		path;
	BEntry		entry;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path); 
	dir.SetTo(path.Path()); 
	if (dir.FindEntry("UltraEncode Folder", &entry) == B_OK)
	{
		//	Locate prefs file		
		dir.SetTo(&entry);
		if (dir.FindEntry("UltraEncode Prefs", &entry) == B_OK)
		{			
			PROGRESS("TProgressDialog::LoadPrefs() - Founds prefs! -\n");
			BMessage prefsMsg;
			
			BFile prefsFile(&entry, B_READ_ONLY);
			if (prefsFile.InitCheck() == B_OK)
			{
				//	Unflatten and load in
				prefsMsg.Unflatten(&prefsFile);
				
				//	Set up extract volume
				const char *extractPath;
				if (prefsMsg.FindString("ExtractPath", &extractPath) == B_OK)
					strcpy(m_ExtractPathName, extractPath);
							
				//	Set up CDDB server name
				const char *cddbName;
				if (prefsMsg.FindString("CDDBName", &cddbName) == B_OK)
					strcpy(m_CDDBServerName, cddbName);
				else
					strcpy(m_CDDBServerName, "us.cddb.com");

				//	Get delete source setting
				if (prefsMsg.FindInt32("DeleteSource", &m_DeleteSource) != B_OK)
					m_DeleteSource = 0;

				//	Get track done beep setting
				if (prefsMsg.FindInt32("TrackDoneBeep", &m_TrackDoneBeep) != B_OK)
					m_TrackDoneBeep = 1;

				//	Get CD done beep setting
				if (prefsMsg.FindInt32("CDDoneBeep", &m_CDDoneBeep) != B_OK)
					m_CDDoneBeep = 1;

				//	Get encode done beep setting
				if (prefsMsg.FindInt32("EncodeDoneBeep", &m_EncodeDoneBeep) != B_OK)
					m_EncodeDoneBeep = 1;
					
				//	Get add artist to title
				if (prefsMsg.FindInt32("AddArtist", &m_AddArtist) != B_OK)
					m_AddArtist = 0;

				//	Get add album to title
				if (prefsMsg.FindInt32("AddAlbum", &m_AddAlbum) != B_OK)
					m_AddAlbum = 0;

				//	Get add track to title
				if (prefsMsg.FindInt32("AddTrack", &m_AddTrack) != B_OK)
					m_AddTrack = 0;
					
				//	Extractor pane setting
				if (prefsMsg.FindBool("ExtractorPanelOpen", &m_ExtractorPanelOpen) != B_OK)
					m_ExtractorPanelOpen = false;
					
				//	Encoder pane setting
				if (prefsMsg.FindBool("EncoderPanelOpen", &m_EncoderPanelOpen) != B_OK)
					m_EncoderPanelOpen = false;
					
				//	Dialog location
				if (prefsMsg.FindPoint("Position", &m_Position) != B_OK)
				{
					// Get screen settings
					BScreen theScreen(B_MAIN_SCREEN_ID);
					BRect screenBounds = theScreen.Frame();
					BRect windBounds   = Frame();
						
					m_Position.x = (screenBounds.Width() - windBounds.Width())/2;
					m_Position.y = (screenBounds.Height() - windBounds.Height())/2;
				}
				
				//	Encoder settings
				ssize_t numBytes;
				layer	*theLayer;	
				if (prefsMsg.FindData("LayerInfo", B_RAW_TYPE, (const void **)&theLayer, &numBytes ) != B_OK)
				{
					PROGRESS("didn't find encoder settings...\n");
					//	Init encoder settings
					m_LayerInfo.version				= MPEG_AUDIO_ID;
					m_LayerInfo.lay					= DFLT_LAY;
					m_LayerInfo.error_protection= false;
					m_LayerInfo.bitrate_index		= 9;
					m_LayerInfo.padding				= 0;
					m_LayerInfo.extension			= 0;
					m_LayerInfo.mode				= MPG_MD_STEREO;
					m_LayerInfo.mode_ext			= 0;
					m_LayerInfo.copyright			= 0;
					m_LayerInfo.original			= 1;
					m_LayerInfo.emphasis			= 0;					
				}
				else
				{
					m_LayerInfo = *theLayer;
					PROGRESS("Found encoder settings...\n");
				}
								
				if (prefsMsg.FindInt32("AcousticModel", &m_AcousticModel) != B_OK)
					m_AcousticModel = DFLT_PSY;
			}
			else
			{
				ERROR("TProgressDialog::LoadPrefs() - Unable to unflatten prefs -\n");
				return false;
			}
		}
		else
		{
			ERROR("TProgressDialog::LoadPrefs() - Unable to find prefs -\n");
			return false;
		}
	}
	
	return true;
}


#pragma mark -
#pragma mark === Ref Handling ===

//---------------------------------------------------------------------
//	HandleRefsMessage
//---------------------------------------------------------------------
//
//	Handle files dragged into window
//

void TProgressDialog::HandleRefsMessage(BMessage *theMessage)
{
	uint32 		theType; 
	int32 		theCount; 
    entry_ref 	theRef;	

	theMessage->GetInfo("refs", &theType, &theCount); 
       
	if ( theType != B_REF_TYPE )
	{
		ERROR("TProgressDialog::HandleRefsMessage() - Not B_REF_TYPE -\n");
		return;
	}
		
	for ( int32 i = --theCount; i >= 0; i-- ) 
	{ 
		if ( theMessage->FindRef("refs", i, &theRef) == B_OK ) 
		{ 			
			// Evaluate the ref and determine if we can deal with it
			// Currently we are only dealing with files
			status_t retVal = EvaluateRef(theRef);
			if(retVal != B_OK)
				ERROR("TProgressDialog::HandleRefsMessage() - Bad ref -\n");
		} 
	}
}	

//---------------------------------------------------------------------
//	EvaluateRef
//---------------------------------------------------------------------
//
//	Check ref and see if it is a type we can handle
//

status_t TProgressDialog::EvaluateRef(entry_ref &ref) 
{
	struct stat st; 
	BEntry 		entry; 
	
	// Can we create a BEntry?
	if (entry.SetTo(&ref, false) != B_OK)
	{
		ERROR("TProgressDialog::HandleRefsMessage() - BEntry SetTo() failure -\n");
		return B_ERROR; 
	}
		
	// Can we get a BStatable?
	if (entry.GetStat(&st) != B_OK) 
	{
		ERROR("TProgressDialog::HandleRefsMessage() - BEntry GetStat() failure -\n");
		return B_ERROR; 
	}
		
	// Is it a SymLink?
	if (S_ISLNK(st.st_mode)) 
		return HandleLink(ref, st);
	// How about a File?
	else if (S_ISREG(st.st_mode)) 
		return HandleFile(ref, st); 
	// A Directory?
	else if (S_ISDIR(st.st_mode)) 
	{
		BDirectory dir; 
		if (dir.SetTo(&ref) != B_OK)
		{
			return B_ERROR;
		}
			
		if (dir.IsRootDirectory()) 
			return HandleVolume(ref, st, dir); 
		else 
			return HandleDirectory(ref, st, dir); 
	} 
	
	// No luck
	return B_ERROR;
} 


//---------------------------------------------------------------------
//	HandleLink
//---------------------------------------------------------------------
//
//

status_t TProgressDialog::HandleLink(entry_ref &theRef, struct stat &st) 
{ 
	
	// Resolve possible symlink...
	BEntry entry(&theRef, true);
	if ( entry.InitCheck() == B_OK )
	{
		entry.GetRef(&theRef);
		
		return HandleFile(theRef, st);
	}
	
	return B_ERROR;
}

//---------------------------------------------------------------------
//	HandleFile
//---------------------------------------------------------------------
//
//

status_t TProgressDialog::HandleFile(entry_ref &theRef, struct stat &st) 
{ 
	//FUNCTION("TProgressDialog::HandleFile() - ENTER -\n");
	
	BFile theFile; 
	if ( theFile.SetTo(&theRef, B_READ_WRITE) == B_OK )
	{									
		// Create BEntry
		BEntry *theEntry = new BEntry(&theRef, true);
		
		// Create node
		BNodeInfo nodeInfo(&theFile);
		
		if (nodeInfo.InitCheck() == B_NO_ERROR)
		{
			if ( IsSupportedType(nodeInfo) && !AlreadyInEncoderList(theRef) )
			//if ( IsAudio(nodeInfo) && !AlreadyInEncoderList(theRef) )
			{									
				//	Replace with MediaFile checks...
				
				/*				
				//	One more check.  For now we only handle 16-bit, 44.1k
				//	Stereo AIFF files
				BSoundFile *soundFile = new BSoundFile(&theRef, B_READ_ONLY);
				if (soundFile->InitCheck() != B_OK)
				{
					ERROR("TProgressDialog::HandleFile() - BSoundFile Error: %s -\n", strerror(soundFile->InitCheck()));					
					return B_ERROR;
				}
					
				//	Make sure we are AIFF
				if (soundFile->FileFormat() != B_AIFF_FILE)
				{
					ERROR("TProgressDialog::HandleFile() - File not AIFF. -\n");
					return B_ERROR;
				}
				
				//	Make sure we are stereo
				if (soundFile->CountChannels() != 2)
				{
					ERROR("TProgressDialog::HandleFile() - File not stereo. Has %ld channels. -\n", soundFile->CountChannels());
					return B_ERROR;
				}

				//	Check bit depth
				if (soundFile->SamplingRate() != 44100)
				{
					ERROR("TProgressDialog::HandleFile() - Invalid sampling rate: %ld -\n", soundFile->SamplingRate());
					return B_ERROR;
				}

				//	Check sampling rate
				if (soundFile->SampleSize() != 2)
				{
					ERROR("TProgressDialog::HandleFile() - File not 16bit -\n");
					return B_ERROR;
				}
				
				//	Must be good!
				delete soundFile;
				*/
				
				//	Add item to list
				//if (LockWithTimeout(kLockTimeout))
				if (Lock())
				{
					AddItemToEncoderList(theEntry, nodeInfo);
					Unlock();
				}
									
				return B_NO_ERROR;
			}
			
			ERROR("TProgressDialog::HandleFile() - Not supported type -\n");
			return B_ERROR;				
		}
	}				

	ERROR("TProgressDialog::HandleFile() - BFile SetTo failure -\n");
	return B_ERROR;
}

//---------------------------------------------------------------------
//	HandleDirectory
//---------------------------------------------------------------------
//	iterate through the directory and pass the resulting
//	refs and attempt to add the resulting file
//
status_t TProgressDialog::HandleDirectory(entry_ref &ref, struct stat &st, BDirectory &dir) 
{
	struct stat s; 
	BEntry entry; 
	
	dir.Rewind();
	while (true)
	{
		if (dir.GetNextEntry(&entry) == B_OK)
		{
			entry.GetStat(&s);
			
			entry_ref eRef;
			entry.GetRef(&eRef);
//			HandleFile(eRef, s);
			EvaluateRef(eRef);
		} else
			break;
	}
		

	return B_ERROR;
}


//---------------------------------------------------------------------
//	HandleVolume
//---------------------------------------------------------------------
//
//

status_t TProgressDialog::HandleVolume(entry_ref &ref, struct stat &st, BDirectory &dir) 
{
	return B_ERROR;
	
	/*
	BVolumeRoster vol_roster; 
	BVolume       vol; 
	BDirectory    root_dir; 
	dev_t         device; 
	
	while (vol_roster.GetNextVolume(&vol) == B_NO_ERROR) 
	{ 
		vol.GetRootDirectory(&root_dir); 
		
		if (root_dir == dir) 
	  		break; 
	} 
    */

}

#pragma mark -
#pragma mark === Encoder List Functions ===

//------------------------------------------------------------------
//	AddItemToEncoderList
//------------------------------------------------------------------
//
//	Add relevant items to sorter list
//
//	Icon  Name  Date  Path  Type  Duration  Size  AudioInfo  VideoInfo
//  

void TProgressDialog::AddItemToEncoderList(BEntry *theEntry, BNodeInfo &nodeInfo)
{
	//
	//	Get name
	//

	char fileName[B_FILE_NAME_LENGTH];	
	theEntry->GetName(fileName);
	
	
	//
	//	Get Date
	//
	
	time_t	fileTime;
	struct	tm *localTime;
    char	timeStr[255];

	theEntry->GetModificationTime(&fileTime);
    localTime = localtime(&fileTime);
    strftime(timeStr, sizeof(timeStr), "%A, %B %d, %Y, %H:%M:%S %p", localTime);

	//
	//	Get Path
	//

	BPath filePath;
	theEntry->GetPath(&filePath);	

	//
	//	Get Type
	//
	
	char typeStr[B_MIME_TYPE_LENGTH];
	if ( nodeInfo.GetType(typeStr) != B_OK)
		strcpy(typeStr, "unknown");
		
	//
	//	Get Size
	//
	
	off_t fileSize;
	theEntry->GetSize(&fileSize);
	
	int32 kBytes = fileSize / 1024;
	char sizeStr[65];
	sprintf(sizeStr, "%dK", kBytes);

	
	//	Get entry_ref
	entry_ref ref;
	theEntry->GetRef(&ref);
	
	//	Set up ID3 Tag info
	ID3_tag tag;	
	tag.tag[0] 		= '\0';
	strncpy(tag.songname, fileName, TAGLEN_SONG);
	tag.artist[0] 	= '\0';
	tag.album[0] 	= '\0';
	tag.year[0] 	= '\0';
	tag.comment[0] 	= '\0';
	tag.genre 		= 0;
			
	//	Add the item to the list
	TEncodeListItem *item = new TEncodeListItem(m_EncoderListView, this, &tag, m_EncoderListView->CountItems()+1, fileName);
	m_EncoderListView->AddItem(item);
	
	//	Add item to master encode list
	m_ProcessList->AddItem(theEntry);
	
	//	Update controls based on list contents
	UpdateEncoderControls();
}

//------------------------------------------------------------------
//	AlreadyInList
//------------------------------------------------------------------
//
//	See if an entry_ref is already in our browser list

bool TProgressDialog::AlreadyInEncoderList(const entry_ref& newRef)
{
	//for (int i = 0; i < m_MediaTabView->CountItems(); i++) {
	//	MediaItem* item = dynamic_cast<MediaItem*>(m_MediaTabView->ItemAt(i));
	//	if (newRef == *item->EntryRef())
	//		return true;
	//}
	
	return false;
}


//---------------------------------------------------------------------
//	RemoveEncoderQueueItem
//---------------------------------------------------------------------
//
//	Remove currently selected item from list
//

void TProgressDialog::RemoveEncoderQueueItem()
{
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		//	Get current selection
		int32 curSel = m_EncoderListView->CurrentSelection();
		if (curSel >= 0)
		{
			bool restart = false;
			
			if (curSel == 0 && Compressing())
			{
				//	Be nice and stop encoder
				StopCompressor();
				restart = true;
			}
			
			//	Remove selected item from list
			TEncodeListItem *item = (TEncodeListItem *)m_EncoderListView->RemoveItem(curSel);
			if (item)
			{
				//	Free item
				delete item;
				
				//	Remove item from process list
				if (m_ProcessList)
				{
					BEntry *entry = (BEntry *)m_ProcessList->RemoveItem(curSel);
					if (entry)
						delete entry;
				}
				
				//	Renumber items
				for (int32 index = 0; index < m_EncoderListView->CountItems(); index++)
				{
					TEncodeListItem *renumItem = (TEncodeListItem *)m_EncoderListView->ItemAt(index);
					if (renumItem)
						renumItem->ID(index + 1);
				}
			}
			
			if (curSel == 0 && restart)
			{
				//	Start encoder
				StartCompressor();
			}
		}
		
		//	Refresh controls basewd on new state
		UpdateEncoderControls();
		
		Unlock();
	}
}

//------------------------------------------------------------------
//	UpdateEncoderControls
//------------------------------------------------------------------
//
//	Update encoder controls based on current list state
//  

void TProgressDialog::UpdateEncoderControls()
{
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{		
		//	Check comprerssor status
		if (Compressing())
		{
			m_EncoderStartButton->SetEnabled(false);
			m_EncoderStopButton->SetEnabled(true);
		}
		else
		{
			m_EncoderStartButton->SetEnabled(true);
			m_EncoderStopButton->SetEnabled(false);	
		}
		
		//	Items in list?
		if (m_EncoderListView->CountItems() > 0)
		{
			m_EncoderAddButton->SetEnabled(true);
			if (m_EncoderListView->CurrentSelection() >= 0)
				m_EncoderRemoveButton->SetEnabled(true);
			else
				m_EncoderRemoveButton->SetEnabled(false);
		}
		//	No items in list
		else
		{
			m_EncoderAddButton->SetEnabled(true);
			m_EncoderRemoveButton->SetEnabled(false);
		}
		
		Unlock();
	}
}

//---------------------------------------------------------------------
//	ShowFileOpenPanel
//---------------------------------------------------------------------
//
//	Show file panel
//

void TProgressDialog::ShowFileOpenPanel()
{ 		
	
	// If the panel has already been constructed, show the panel
	// Otherwise, create the panel	
	if (m_FileOpenPanel)
		m_FileOpenPanel->Show();
	else
	{
		// Create a RefFilter for a "audio/x-aiff" type
		TRefFilter* refFilter = new TRefFilter(kAudioAiffFilter);

		// Construct a file panel and set it to modal
	 	m_FileOpenPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, true, NULL, refFilter, true, true );
	 	if (m_FileOpenPanel)
	 	{
		 	//	Set target to this window
			m_FileOpenPanel->SetTarget(this);
			
		 	// Set it to application's home directory
		 	app_info appInfo;
		 	be_app->GetAppInfo(&appInfo); 	
		 	BEntry entry(&appInfo.ref);
		 	BDirectory parentDir;
		 	entry.GetParent(&parentDir);
		 	m_FileOpenPanel->SetPanelDirectory(&parentDir);

			// Center Panel
			CenterWindow(m_FileOpenPanel->Window());
						
			m_FileOpenPanel->Show();
		}
	}
}


#pragma mark -
#pragma mark === Accessor Functions ===

//------------------------------------------------------------------
//	Artist
//------------------------------------------------------------------
//
//	Update artist string
//  

void TProgressDialog::Artist(const char *str)
{
	//if (LockLooperWithTimeout(kLockTimeout))
	if (LockLooper())
	{
		//	Update artist
		m_Artist = str; 
		m_ArtistString->SetText(m_Artist.String());
		
		//	Update all list items
		for (int32 index = 0; index < m_TracksListView->CountItems(); index++)
		{
			TCDListItem *listItem = (TCDListItem *)m_TracksListView->ItemAt(index);
			if (listItem)
				strncpy(listItem->ID3Tag()->artist, m_ArtistString->Text(), TAGLEN_ARTIST);
		}
								
		UnlockLooper();
	}
}


//------------------------------------------------------------------
//	Album
//------------------------------------------------------------------
//
//	Update album string
//  

void TProgressDialog::Album(const char *str)
{
	//if (LockLooperWithTimeout(kLockTimeout))
	if (LockLooper())
	{
		m_Album = str; 
		m_AlbumString->SetText(m_Album.String());
		
		//	Update all list items
		for (int32 index = 0; index < m_TracksListView->CountItems(); index++)
		{
			TCDListItem *listItem = (TCDListItem *)m_TracksListView->ItemAt(index);
			if (listItem)
				strncpy(listItem->ID3Tag()->album, m_AlbumString->Text(), TAGLEN_ALBUM);
		}

		UnlockLooper();
	}
}


//------------------------------------------------------------------
//	Year
//------------------------------------------------------------------
//
//	Update year string
//  

void TProgressDialog::Year(const char *str)
{
	//if (LockLooperWithTimeout(kLockTimeout))
	if (LockLooper())
	{
		m_Year = str;
		
		//	Update all list items
		for (int32 index = 0; index < m_TracksListView->CountItems(); index++)
		{
			TCDListItem *listItem = (TCDListItem *)m_TracksListView->ItemAt(index);
			if (listItem)
				strncpy(listItem->ID3Tag()->year, m_Year.String(), TAGLEN_YEAR);
		}

		UnlockLooper();
	}
}


//------------------------------------------------------------------
//	Genre
//------------------------------------------------------------------
//
//	Update genre
//  

void TProgressDialog::Genre(int32 genre)
{
	//if (LockLooperWithTimeout(kLockTimeout))
	if (LockLooper())
	{
		m_Genre = genre;
		
		//	Update all list items
		for (int32 index = 0; index < m_TracksListView->CountItems(); index++)
		{
			TCDListItem *listItem = (TCDListItem *)m_TracksListView->ItemAt(index);
			if (listItem)
				listItem->ID3Tag()->genre = m_Genre;
		}

		UnlockLooper();
	}
}


#pragma mark -
#pragma mark === Extractor/Compressor Pane State ===

//------------------------------------------------------------------
//	ShowExtractor
//------------------------------------------------------------------
//
//	Display or hide extractor pane
//  

void TProgressDialog::ShowExtractor(int32 value)
{
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		//	Expand
		if (value == 1)
		{
			if (m_EncoderPanelOpen)
			{
				//	Move extractor to extractor position
				m_ExtractorPane->MoveTo(kExtractorPos);
	
				//	Move encoder to encoder position
				m_EncoderPane->MoveTo(kEncoderPos);
				
				//	Increase height of window to max
				ResizeTo(Bounds().Width(), kEncoderHeight);		
			}
			else
			{
				ResizeTo(Bounds().Width(), kExtractorHeight);
			}
			
			//	Update tracking val
			m_ExtractorPanelOpen = true;
		}
		//	Contract
		else
		{
			if (m_EncoderPanelOpen)
			{
				//	Move extractor to outer space
				m_ExtractorPane->MoveTo(-500, -500);
	
				//	Move encoder to extractor position
				m_EncoderPane->MoveTo(kExtractorPos);
				
				//	Decrease height of window
				ResizeTo(Bounds().Width(), Bounds().Height() - kExtractorPaneHeight);	
			}
			else
			{
				//	Resize to collapsed height
				ResizeTo(Bounds().Width(), kCollapsedHeight);
			}
			
			//	Update tracking val
			m_ExtractorPanelOpen = false;
		}
		Unlock();
	}
}

//------------------------------------------------------------------
//	ShowEncoder
//------------------------------------------------------------------
//
//	Dispaly or hide encoder pane
//  

void TProgressDialog::ShowEncoder(int32 value)
{
	//if (LockWithTimeout(kLockTimeout))
	if (Lock())
	{
		//	Expand
		if (value == 1)
		{
			//	Is extractor pane showing?
			if (m_ExtractorPanelOpen)
			{
				//	Just increase height of window
				ResizeTo(Bounds().Width(), kEncoderHeight);
			}
			else
			{
				//	Move extractor pane into outer space
				m_ExtractorPane->MoveTo(-500, -500);
				
				//	Move encoder to previous extractor position
				m_EncoderPane->MoveTo(kExtractorPos);
				
				//	Just increase height of window by encoder pane height
				ResizeTo(Bounds().Width(), Bounds().Height() + kEncoderPaneHeight);				
			}
			
			//	Update flag						
			m_EncoderPanelOpen	 = true;
		}
		//	Contract
		else
		{			
			//	Is extractor pane showing?
			if (m_ExtractorPanelOpen)
			{
				//	Just decrease height of window the size of the encoder pane
				ResizeTo(Bounds().Width(), Bounds().Height() - kEncoderPaneHeight);
			}
			else
			{
				//	Decrease height of window to minimized state
				ResizeTo(Bounds().Width(), kCollapsedHeight);				
	
				//	Move panes back to standard position
				m_ExtractorPane->MoveTo(kExtractorPos);
				m_EncoderPane->MoveTo(kEncoderPos);				
			}
			
			//	Update flag						
			m_EncoderPanelOpen	 = false;						
		}
		Unlock();
	}
}		



#pragma mark -
#pragma mark === Selection Routines ===

//------------------------------------------------------------------
//	SelectAllExtractorItems
//------------------------------------------------------------------
//
//  

void TProgressDialog::SelectAllExtractorItems()
{	
	//	Deselect all list items
	m_TracksListView->DeselectAll();
	
	//	Set extraction status to true
	for (int32 index = 0; index < m_TracksListView->CountItems(); index++)
	{
		//	Have it draw selected check
		TCDListItem *listItem = (TCDListItem *)m_TracksListView->ItemAt(index);
		if (listItem)
		{
			listItem->Selected(true);
			listItem->Pending(true);
			listItem->StatusString("Pending");
	
			
			//	Set extract flag to true
			track_info *albumData = (track_info *)m_TitleList->ItemAt(listItem->ID());
			if (albumData)
				albumData->extract = true;
		}
	}
	
	// Redraw list
	m_TracksListView->Invalidate();
}


//------------------------------------------------------------------
//	DeselectAllExtractorItems
//------------------------------------------------------------------
//
//  

void TProgressDialog::DeselectAllExtractorItems()
{	
	//	Deselect all list items
	m_TracksListView->DeselectAll();
	
	//	Set extraction status to false
	for (int32 index = 0; index < m_TracksListView->CountItems(); index++)
	{
		TCDListItem *listItem = (TCDListItem *)m_TracksListView->ItemAt(index);
		if (listItem)
		{
			listItem->Selected(false);
			listItem->Pending(false);
			listItem->StatusString("");
			
			//	Set extract flag to true
			track_info *albumData = (track_info *)m_TitleList->ItemAt(listItem->ID());
			if (albumData)
				albumData->extract = false;
		}
	}
	
	// Redraw list
	m_TracksListView->Invalidate();
}

