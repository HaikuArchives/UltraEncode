//-------------------------------------------------------------------
//
//	File:	TQueueDialog.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	05.20.99
//
//	Desc:	Encoder Queue dialog
//
//	Copyright ©1999	Mediapede, Inc.
//
//---------------------------------------------------------------------

// Includes
#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <ScrollView.h>
#include <Path.h>
#include <NodeInfo.h>
#include <Directory.h>
#include <SoundFile.h>
#include <FilePanel.h>
#include <StatusBar.h>

#include <string.h>

#include "DebugBuild.h"
#include "AppMessages.h"
#include "AppUtils.h"
#include "RipperApp.h"

#include "TRefFilter.h"
#include "TProgressDialog.h"
#include "TQueueListView.h"
#include "TQueueListItem.h"

#include "TQueueDialog.h"

//	Constants

//-------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------
//
//

TQueueDialog::TQueueDialog(BMessage *theMessage, TProgressDialog *parent) : 
				BWindow(theMessage),
				m_Parent(parent),
				m_FileOpenPanel(NULL)
{	
	//	Default initialization
	Init();		
}


//-------------------------------------------------------------------
//	Destructor
//-------------------------------------------------------------------
//
//

TQueueDialog::~TQueueDialog()
{
}


//-------------------------------------------------------------------
//	QuitRequested
//-------------------------------------------------------------------
//
//

bool TQueueDialog::QuitRequested()
{
	//	Just hide
	Hide();
	return false;
}


#pragma mark -
#pragma mark === Initialization ===

//-------------------------------------------------------------------
//	Init
//-------------------------------------------------------------------
//
//

void TQueueDialog::Init() 
{
	//
	//	Locate all child views
	//
	
	//	Find buttons
	m_StopButton 	= (BButton *)FindView("StopButton");
	m_StartButton 	= (BButton *)FindView("StartButton");
	m_AddButton 	= (BButton *)FindView("AddButton");
	m_RemoveButton	= (BButton *)FindView("RemoveButton");
	
	//	Find and setup list view
	m_TracksListScrollView 	 = (BScrollView *)FindView("TracksListScrollView");
	BListView *tracksListView = (BListView *)FindView("TracksListView");
	BRect frame = tracksListView->Frame();
	frame.InsetBy(2, 2);
	m_TracksListView = new TQueueListView(this, frame);
	tracksListView->Parent()->AddChild(m_TracksListView);
	tracksListView->RemoveSelf();
	delete tracksListView;
	
	//	Set list view messages
	BMessage *selectMessage = new BMessage(QUEUE_LIST_SELECT_MSG);
	m_TracksListView->SetSelectionMessage(selectMessage);
	
	BMessage *invokeMessage = new BMessage(QUEUE_LIST_INVOKE_MSG);
	m_TracksListView->SetInvocationMessage(invokeMessage);
	
	//	Find status bar
	m_EncodeStatusBar = (BStatusBar *)FindView("EncodeStatusBar");
	m_EncodeStatusBar->Reset("Waiting...", "");
	
	UpdateControls();	
}


#pragma mark -
#pragma mark === Message Handling ===

//-------------------------------------------------------------------
//	MessageReceived
//-------------------------------------------------------------------
//
//

void TQueueDialog::MessageReceived(BMessage *message) 
{ 
	switch(message->what)
	{			
		//	We have received some file refs from a drag onto one of our sorters
		case B_SIMPLE_DATA:
			{
				if (message->WasDropped())
					HandleRefsMessage(message);
			}
			break;
			
		case B_REFS_RECEIVED:
			HandleRefsMessage(message);
			break;
			
		//	Start encoder
		case START_ENCODE_MSG:
			{
				m_Parent->StartCompressor();
				UpdateControls();
				m_TracksListView->Invalidate();
			}
			break;
					
		//	Stop encoder
		case STOP_ENCODE_MSG:
			{
				m_Parent->StopCompressor();
				UpdateControls();
				m_TracksListView->Invalidate();
			}
			break;
			
		case ADD_ENCODE_MSG:
			ShowFileOpenPanel();
			break;
			
		case REMOVE_ENCODE_MSG:
			RemoveQueueItem();
			break;
			
		case ENCODE_WINDOW_MSG:
			break;

		case QUEUE_LIST_SELECT_MSG:
			break;
			
		case QUEUE_LIST_INVOKE_MSG:
			break;
			
		default:
			break;
	}
}

#pragma mark -
#pragma mark === Control Handling ===

//------------------------------------------------------------------
//	UpdateControls
//------------------------------------------------------------------
//
//	Update controls based on current list state
//  

void TQueueDialog::UpdateControls()
{
	//	Check comprerssor status
	if (m_Parent->Compressing())
	{
		m_StartButton->SetEnabled(false);
		m_StopButton->SetEnabled(true);
	}
	else
	{
		m_StartButton->SetEnabled(true);
		m_StopButton->SetEnabled(false);	
	}
	
	//	Items in list?
	if (m_TracksListView->CountItems() > 0)
	{
		m_AddButton->SetEnabled(true);
		m_RemoveButton->SetEnabled(true);
	}
	//	No items in list
	else
	{
		m_AddButton->SetEnabled(true);
		m_RemoveButton->SetEnabled(false);
	}
}


#pragma mark -
#pragma mark === List Functions ===

//------------------------------------------------------------------
//	AddItemToDataList
//------------------------------------------------------------------
//
//	Add relevant items to sorter list
//
//	Icon  Name  Date  Path  Type  Duration  Size  AudioInfo  VideoInfo
//  

void TQueueDialog::AddItemToDataList(BEntry *theEntry, BNodeInfo &nodeInfo)
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
	tag.songname[0] = '\0';
	tag.artist[0] 	= '\0';
	tag.album[0] 	= '\0';
	tag.year[0] 	= '\0';
	tag.comment[0] 	= '\0';
	tag.genre 		= 0;
	
	//	Add the item to the list
	TQueueListItem *item = new TQueueListItem(m_TracksListView, m_Parent, &tag, fileName);
	m_TracksListView->AddItem(item);
	
	//	Add item to master encode list
	m_Parent->ProcessList()->AddItem(theEntry);
	
	//	Update controls based on list contents
	UpdateControls();
}

//------------------------------------------------------------------
//	AlreadyInList
//------------------------------------------------------------------
//
//	See if an entry_ref is already in our browser list

bool TQueueDialog::AlreadyInList(const entry_ref& newRef)
{
	//for (int i = 0; i < m_MediaTabView->CountItems(); i++) {
	//	MediaItem* item = dynamic_cast<MediaItem*>(m_MediaTabView->ItemAt(i));
	//	if (newRef == *item->EntryRef())
	//		return true;
	//}
	
	return false;
}


//---------------------------------------------------------------------
//	RemoveQueueItem
//---------------------------------------------------------------------
//
//	Remove currently selected item from list
//

void TQueueDialog::RemoveQueueItem()
{
	if (Lock())
	{
		//	Get current selection
		int32 curSel = m_TracksListView->CurrentSelection();
		if (curSel >= 0)
		{
			//	Be nice and stop encoder
			m_Parent->StopCompressor();
			
			//	Remove selected item from list
			TQueueListItem *item = (TQueueListItem *)m_TracksListView->RemoveItem(curSel);
			if (item)
			{
				//	Free item
				delete item;
				
				//	Remove item from process list
				if (m_Parent->ProcessList())
				{
					BEntry *entry = (BEntry *)m_Parent->ProcessList()->RemoveItem(curSel);
					if (entry)
						delete entry;
				}
			}
			
			//	Start encoder
			m_Parent->StartCompressor();
		}
		
		Unlock();
	}
}


#pragma mark -
#pragma mark === Ref Handling ===

//---------------------------------------------------------------------
//	HandleRefsMessage
//---------------------------------------------------------------------
//
//	Handle files dragged into window
//

void TQueueDialog::HandleRefsMessage(BMessage *theMessage)
{
	uint32 		theType; 
	int32 		theCount; 
    entry_ref 	theRef;	

	theMessage->GetInfo("refs", &theType, &theCount); 
       
	if ( theType != B_REF_TYPE )
	{
		ERROR("TQueueDialog::HandleRefsMessage() - Not B_REF_TYPE -\n");
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
				ERROR("TQueueDialog::HandleRefsMessage() - Bad ref -\n");
		} 
	}
}	

//---------------------------------------------------------------------
//	EvaluateRef
//---------------------------------------------------------------------
//
//	Check ref and see if it is a type we can handle
//

status_t TQueueDialog::EvaluateRef(entry_ref &ref) 
{
	struct stat st; 
	BEntry 		entry; 
	
	// Can we create a BEntry?
	if (entry.SetTo(&ref, false) != B_OK)
	{
		ERROR("TQueueDialog::HandleRefsMessage() - BEntry SetTo() failure -\n");
		return B_ERROR; 
	}
		
	// Can we get a BStatable?
	if (entry.GetStat(&st) != B_OK) 
	{
		ERROR("TQueueDialog::HandleRefsMessage() - BEntry GetStat() failure -\n");
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

status_t TQueueDialog::HandleLink(entry_ref &theRef, struct stat &st) 
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

status_t TQueueDialog::HandleFile(entry_ref &theRef, struct stat &st) 
{ 
	//FUNCTION("TQueueDialog::HandleFile() - ENTER -\n");
	
	BFile theFile; 
	if ( theFile.SetTo(&theRef, B_READ_WRITE) == B_OK )
	{									
		// Create BEntry
		BEntry *theEntry = new BEntry(&theRef, true);
		
		// Create node
		BNodeInfo nodeInfo(&theFile);
		
		if (nodeInfo.InitCheck() == B_NO_ERROR)
		{
			if ( IsSupportedType(nodeInfo) && !AlreadyInList(theRef) )
			{									
				//	Replace with MEdiaFile checks...
				
				/*
				//	One more check.  For now we only handle 16-bit, 44.1k
				//	Stereo AIFF files
				BSoundFile *soundFile = new BSoundFile(&theRef, B_READ_ONLY);
				if (soundFile->InitCheck() != B_OK)
				{
					ERROR("TQueueDialog::HandleFile() - BSoundFile Error: %s -\n", strerror(soundFile->InitCheck()));					
					return B_ERROR;
				}
					
				//	Make sure we are AIFF
				if (soundFile->FileFormat() != B_AIFF_FILE)
				{
					ERROR("TQueueDialog::HandleFile() - File not AIFF. -\n");
					return B_ERROR;
				}
				
				//	Make sure we are stereo
				if (soundFile->CountChannels() != 2)
				{
					ERROR("TQueueDialog::HandleFile() - File not stereo. Has %ld channels. -\n", soundFile->CountChannels());
					return B_ERROR;
				}

				//	Check bit depth
				if (soundFile->SamplingRate() != 44100)
				{
					ERROR("TQueueDialog::HandleFile() - Invalid sampling rate: %ld -\n", soundFile->SamplingRate());
					return B_ERROR;
				}

				//	Check sampling rate
				if (soundFile->SampleSize() != 2)
				{
					ERROR("TQueueDialog::HandleFile() - File not 16bit -\n");
					return B_ERROR;
				}
				
				//	Must be good!
				delete soundFile;
				*/
				
				//	Add item to list
				AddItemToDataList(theEntry, nodeInfo);
									
				return B_NO_ERROR;
			}
			
			ERROR("TQueueDialog::HandleFile() - Not supported type -\n");
			return B_ERROR;				
		}
	}				

	ERROR("TQueueDialog::HandleFile() - BFile SetTo failure -\n");
	return B_ERROR;
}

//---------------------------------------------------------------------
//	HandleDirectory
//---------------------------------------------------------------------
//	iterate through the directory and pass the resulting
//	refs and attempt to add the resulting file
//
status_t TQueueDialog::HandleDirectory(entry_ref &ref, struct stat &st, BDirectory &dir) 
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

status_t TQueueDialog::HandleVolume(entry_ref &ref, struct stat &st, BDirectory &dir) 
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
#pragma mark === Dialog Routines ===

//---------------------------------------------------------------------
//	ShowFileOpenPanel
//---------------------------------------------------------------------
//
//	Show file panel
//

void TQueueDialog::ShowFileOpenPanel()
{ 		
	
	// If the panel has already been constructed, show the panel
	// Otherwise, create the panel	
	if (m_FileOpenPanel)
		m_FileOpenPanel->Show();
	else
	{
		// Create a RefFilter for a "audio/x-aiff" type
		TRefFilter *refFilter = new TRefFilter(kAudioAiffFilter);

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
		 		
			/*
			//	Get pointer to "Cancel" button		
			if (m_FileOpenPanel->Window()->Lock())
			{
				BView *backView = m_FileOpenPanel->Window()->ChildAt(0);
				BButton *cancelButton = (BButton *)m_FileOpenPanel->Window()->FindView("cancel button");
				
				//	Create our "New..." button
				BRect bounds = cancelButton->Frame();
				bounds.OffsetBy(-(bounds.Width()+12), 0);
				BButton *newButton = new BButton(bounds, "New...", "New...", new BMessage(FILE_NEW_MSG), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
				newButton->SetTarget(be_app);
				backView->AddChild(newButton);
				newButton->Show();
				
				m_FileOpenPanel->Window()->Unlock();
			}
			*/
			
			// Center Panel
			CenterWindow(m_FileOpenPanel->Window());
						
			m_FileOpenPanel->Show();
		}
	}
}
