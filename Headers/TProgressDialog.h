//-------------------------------------------------------------------
//
//	File:	TProgressDialog.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	05.07.98
//
//	Desc:	
//
//	Copyright ©1998 mediapede software
//
//---------------------------------------------------------------------

#ifndef __TPROGRESSDIALOG_H__
#define __TPROGRESSDIALOG_H__

//	Includes
#include <scsi.h> 
#include <Window.h>
#include <MessageFilter.h>
#include <Volume.h>
#include <String.h>
#include <StringView.h>
#include <netdb.h>
#include <MediaFile.h>
#include <MediaTrack.h>

#include "CDDA.h"
#include "CDDBLooper.h"
#include "TSettingsDialog.h"
#include "TExtractListTitle.h"
#include "encodelib.h"
#include "id3tag.h"

//	Structs
typedef struct { 
        int32   flags; 
        char    title[256]; 
        int32   length;
        bool	extract;
} track_info; 

//	Messages
#define START_MSG				'strt'
#define CDDB_MSG				'cddb'
#define OPTIONS_MSG				'optn'
#define TRACKLIST_SELECT_MSG	'trSL'
#define TRACKLIST_INVOKE_MSG	'trIN'
#define SHOW_EXTRACTOR_MSG		'shEX'
#define SHOW_ENCODER_MSG		'shEN'
#define ENCODER_LIST_SELECT_MSG	'enSL'
#define ENCODER_LIST_INVOKE_MSG	'enIN'
#define START_ENCODE_MSG		'strE'
#define STOP_ENCODE_MSG			'stop'
#define ADD_ENCODE_MSG			'add '
#define REMOVE_ENCODE_MSG		'remo'
#define CD_INFO_MSG				'cdIN'
#define MOUNT_RIO_MSG			'mtRO'

const uint32 CDDB_INFO_MSG		= 'cdin';


//	Forward Declarations
class TSettingsDialog;
class TProgressListView;
class TQueueListView;
class KnobSwitch;
class TFrameBox;
class TEncodeListTitle;
class TInfoBox;
class TCDDBInfoWindow;

// Class Definition
class TProgressDialog : public BWindow
{
	public:
		TProgressDialog(BMessage *message);
		virtual ~TProgressDialog();
		
		void 	Quit() ;
		bool 	QuitRequested();
		void 	MessageReceived(BMessage *message);
	
		void 	Progress(float progress);
		
		void	StartCompressor();
		void	StopCompressor();
		
		void	Artist(const char *str);
		void	Album(const char *str);
		void 	Year(const char *str);
		void	Genre(int32 genre);
		
		void 	SelectAllExtractorItems();
		void 	DeselectAllExtractorItems();
		
		void 	UpdateCDDBServer();

		//	Inlines
		inline BList 				*ProcessList(){ return m_ProcessList; }
		inline BList 				*TitleList(){ return m_TitleList; }
		inline bool 				Extracting(){ return m_StartExtraction; }	
		inline TProgressListView	*TracksListView(){ return m_TracksListView; }
		inline layer				*LayerInfo(){ return &m_LayerInfo; }
		inline int32				*AcousticModel(){ return &m_AcousticModel; }
		inline void					AcousticModel(int32 model){ m_AcousticModel = model; }
		inline const char			*ExtractPathName(){ return m_ExtractPathName; }
		inline void					ExtractPathName(const char *path){ strcpy(m_ExtractPathName, path); }
		inline const char			*CDDBServerName(){ return m_CDDBServerName; }
		inline void					CDDBServerName(const char *path){ strcpy(m_CDDBServerName, path); }
		inline BString				*Artist(){ return &m_Artist; }		
		inline BString				*Album(){ return &m_Album; }
		inline BString				*Year(){ return &m_Year; }		
		inline int32				Genre(){return m_Genre; }		
		inline int32				Compressing(){ return m_CompressorThread; }
		inline BScrollBar			*ExtractScrollBar(){ return m_ExtractListScrollBar; }
		inline TQueueListView		*EncoderListView(){ return m_EncoderListView; }
		inline BScrollBar			*EncoderScrollBar(){ return m_EncoderListScrollBar; }

				
		//	Prefs
		int32			m_DeleteSource;
		int32			m_TrackDoneBeep;
		int32			m_CDDoneBeep;
		int32			m_EncodeDoneBeep;
		int32			m_AddArtist;
		int32			m_AddAlbum;
		int32			m_AddTrack;
			
	private:
		void 			Init();
		void 			InitControls();
		
		void 			CreateMouseFilter();
		void			CreateKeyboardFilter();
	
		static int32 	cd_watcher(void *arg);
		int32 			CDWatcher();
		static int32 	compressor(void *arg);
		int32 			Compressor();
		static int32 	extractor(void *arg);
		int32 			Extractor();

		bool 			StartExtraction();
		void 			StopExtraction();
		void 			BumpMSF(CDDA_MSF *a);
		bool 			MSFEqual(CDDA_MSF a, CDDA_MSF b);
		
		status_t		MakeFile(BEntry *item, BFile *file, const char *filename, const char *directoryname);
		BMediaFile 		*CreateOutputMediaFile(BFile *file);
		
		void 			Enable();
		void 			Disable();
		float 			GetTrackComplete();
		void 			SetupTrackStatus(track_info *trackData, float secs);
		void 			SetupEncoderStatus(BEntry *entry);
		
		void 			SetCDInfo(cd_info *newInfo);
		bool 			CheckCDDB();
		
		void			AddToEncoderQueue(BEntry *entry, ID3_tag *tag, char *time);
		void 			RemoveFromEncoderQueue(BEntry *entry);
		void 			UpdateEncoderControls();
		void 			ShowSettingsDialog();
		
		bool 			SavePrefs();
		bool 			LoadPrefs();
		
		//	Refs
		void 			AddItemToEncoderList(BEntry *theEntry, BNodeInfo &nodeInfo);
		bool 			AlreadyInEncoderList(const entry_ref& newRef);
		void 			RemoveEncoderQueueItem();
		BMediaTrack 	*GetAudioTrack(BMediaFile *file, media_format &format);

		void 			HandleRefsMessage(BMessage *theMessage);
		status_t		EvaluateRef(entry_ref &ref);
		status_t 		HandleLink(entry_ref &theRef, struct stat &st);
		status_t 		HandleFile(entry_ref &theRef, struct stat &st);
		status_t 		HandleDirectory(entry_ref &ref, struct stat &st, BDirectory &dir);
		status_t 		HandleVolume(entry_ref &ref, struct stat &st, BDirectory &dir);
		void 			ShowFileOpenPanel();
		
		void 			ShowExtractor(int32 value);
		void 			ShowEncoder(int32 value);

	
		// Member Variables
		BMenuBar	*m_MenuBar;		
		BStringView *m_AlbumString;
		BStringView *m_ArtistString;
		BStringView *m_TracksString;
		BStringView *m_CDDBString;
		
		TInfoBox 	*m_InfoBox;
		TFrameBox	*m_ExtractorPane;
		TFrameBox	*m_EncoderPane;
		BStatusBar	*m_ExtractTrackStatusBar;
		BStatusBar 	*m_ExtractCDStatusBar;
		BStatusBar 	*m_EncodeStatusBar;
		
		BButton 	*m_CancelButton;
		BButton 	*m_StartButton;
		BButton 	*m_CDDBButton;
		BButton 	*m_EncoderStopButton;
		BButton 	*m_EncoderStartButton;
		BButton 	*m_EncoderAddButton;
		BButton 	*m_EncoderRemoveButton;
		
		KnobSwitch	*m_ExtractorKnob;
		KnobSwitch	*m_EncoderKnob;
		
		BScrollBar			*m_ExtractListScrollBar;
		BScrollBar			*m_EncoderListScrollBar;
		TProgressListView 	*m_TracksListView;
		TExtractListTitle	*m_TracksListTitle;
		TEncodeListTitle	*m_EncodeListTitle;
		TQueueListView		*m_EncoderListView;
		
		BMenuField	*m_InfoMenu;
				
		CDDBLooper	*m_CDDBLooper;
		cddb_toc	m_CDDBTOC[100];
		
		thread_id	m_WatcherThread;
		thread_id	m_CompressorThread;
		thread_id	m_ExtractorThread;
		bool		m_Compress;
		
		int32		m_CDID;
		scsi_toc	m_SCSITOC;
		int32    	m_State;
		int32		m_CurrentTrack;
		int32		m_MinTrack;
		int32		m_MaxTrack; 
		int32		m_TotalTracks;
		bool		m_Changed;
		sem_id		m_CDSem;
				
		CDDA_TOC 		*m_TOC;			//	Table Of Contents
		scsi_read_cd 	*m_ReadCommand;	//	The READ_CD ioctl() command
		CDDA_BLOCK 		*m_FrameBuffer;	//	Buffer for one CDDA frame
		
		BList			*m_ProcessList;
		BList			*m_TitleList;
		bool			m_CanUseCDDB;
		bool			m_StartExtraction;
				
		BMessageFilter	*m_MouseFilter;
		BMessageFilter	*m_KeyboardFilter;
		
		TSettingsDialog *m_SettingsDialog;
		TCDDBInfoWindow *m_CDDBInfoWindow;
		
		layer			m_LayerInfo;
		int32			m_AcousticModel;
		
		char 			m_ExtractPathName[B_PATH_NAME_LENGTH];
		char 			m_CDDBServerName[MAXHOSTNAMELEN];
		BFilePanel		*m_FileOpenPanel;
		
		bool			m_ExtractorPanelOpen;
		bool			m_EncoderPanelOpen;
		BPoint			m_Position;
				
		//	Strings
		BString			m_Artist;
		BString			m_Album;
		BString			m_Year;
		int32			m_Genre;		
};

//	Function Prototypes
filter_result mouse_filter(BMessage *message, BHandler **target, BMessageFilter *messageFilter);
filter_result keyboard_filter(BMessage *message, BHandler **target, BMessageFilter *messageFilter);

#endif
