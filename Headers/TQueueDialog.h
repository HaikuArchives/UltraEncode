//-------------------------------------------------------------------
//
//	File:	TQueueDialog.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	06.10.99
//
//	Desc:	
//
//	Copyright ©1998 mediapede software
//
//---------------------------------------------------------------------

#ifndef __TQUEUEDIALOG_H__
#define __TQUEUEDIALOG_H__

//	Includes
#include <Window.h>
#include <ListView.h>

//	Forward Declarationsd
class TProgressDialog;
class TQueueListView;

//	Messages
#define START_ENCODE_MSG		'strt'
#define STOP_ENCODE_MSG			'stop'
#define ADD_ENCODE_MSG			'add '
#define REMOVE_ENCODE_MSG		'remo'
#define ENCODE_WINDOW_MSG		'ecls'
#define QUEUE_LIST_SELECT_MSG	'trSL'
#define QUEUE_LIST_INVOKE_MSG	'trIN'


// Class Definition
class TQueueDialog : public BWindow
{
	public:
		TQueueDialog(BMessage *message, TProgressDialog *parent);
		virtual ~TQueueDialog();
		
		bool	QuitRequested();
		
		void 	MessageReceived(BMessage *message);
		
		void 	RemoveQueueItem();
		
		//	Inlines
		TQueueListView 	*EncoderList(){ return m_TracksListView; }
		BStatusBar 		*StatusBar(){ return m_EncodeStatusBar; }
		BScrollView 	*ScrollView(){ return m_TracksListScrollView; }
		
	private:
		void 	Init();
		
		void 	UpdateControls();
		
		void 		AddItemToDataList(BEntry *theEntry, BNodeInfo &nodeInfo);
		bool 		AlreadyInList(const entry_ref& newRef);
				
		void 		HandleRefsMessage(BMessage *theMessage);
		status_t	EvaluateRef(entry_ref &ref);
		status_t 	HandleLink(entry_ref &theRef, struct stat &st);
		status_t 	HandleFile(entry_ref &theRef, struct stat &st);
		status_t 	HandleDirectory(entry_ref &ref, struct stat &st, BDirectory &dir);
		status_t 	HandleVolume(entry_ref &ref, struct stat &st, BDirectory &dir);
		
		void 		ShowFileOpenPanel();
		
		//	Member Variables
		TProgressDialog *m_Parent;
		
		BButton 	*m_StopButton;
		BButton 	*m_StartButton;
		BButton 	*m_AddButton;
		BButton 	*m_RemoveButton;
		BStatusBar 	*m_EncodeStatusBar;
		
		BScrollView 	*m_TracksListScrollView;
		TQueueListView 	*m_TracksListView;
		
		BFilePanel		*m_FileOpenPanel;
};

#endif
