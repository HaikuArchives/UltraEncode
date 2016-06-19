//---------------------------------------------------------------------
//
//	File:	TSettingsDialog.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.10.99
//
//
//	Copyright ©1999 mediapede Software
//
//---------------------------------------------------------------------

#ifndef __TSETTINGSDIALOG_H__
#define __TSETTINGSDIALOG_H__

//	Includes
#include <Window.h>
#include "TProgressDialog.h"

//	Forward Declarations
class TProgressDialog;

//	Class Definition
class TSettingsDialog : public BWindow
{
	public:
		TSettingsDialog(TProgressDialog *parent, BMessage *message);
		~TSettingsDialog();
		
		void	MessageReceived(BMessage* message);
		
		bool 	QuitRequested();
		
	private:
	
	protected:
		//	Member Functions
		bool 	Init();
		void 	InitExtractFromMenu(BMenuField *menuField);
		int32	FindCDDirectory(const char *directory, char *name);
		int32	SetupCDMenu(const char *directory, BMenu *menu);
		
		void 	ShowPathDialog();
		void 	SetExtractPath(BMessage *message);
		
		//	Member Variables
		TProgressDialog *m_Parent;
		
		BButton		*m_ExtractToButton;
		BStringView	*m_ExtractToString;
		
		BMenuField	*m_ModeMenu;
		BMenuField	*m_BitRateMenu;
		BMenuField	*m_DeemphasisMenu;
		BMenuField	*m_FlagsMenu;		
		BMenuField	*m_ExtractFromMenu;
		BCheckBox	*m_AutoCheck;
		BCheckBox	*m_DeleteSourceCheck;
		BCheckBox	*m_TrackBeepCheck;
		BCheckBox	*m_CDBeepCheck;
		BCheckBox	*m_EncodeBeepCheck;

		BCheckBox	*m_AddArtistCheck;
		BCheckBox	*m_AddAlbumCheck;
		BCheckBox	*m_AddTrackCheck;
		
		BFilePanel	*m_FileOpenPanel;
		
		int32 		m_CDCount;
};

//	Control messages
#define AUTOSTART_MSG 		'auSt'
#define TRACK_BEEP_MSG 		'aTEX'
#define CD_BEEP_MSG 		'aCDE'
#define ENCODE_BEEP_MSG 	'aTEB'
#define EXTRACT_TO_MSG 		'exTB'
#define MP3_EXTENSION_MSG 	'mp3M'

#define ADD_ARTIST_MSG 		'aART'
#define ADD_ALBUM_MSG 		'aALB'
#define ADD_TRACK_MSG 		'aTRN'

//
//	Menu Messages
//

//	Stereo
#define STEREO_MSG			's_St'
#define JOINT_STEREO_MSG	's_Js'
#define MONO_MSG			's_Mo'
#define FORCE_MSG			's_Fo'

//	De-emphasis
#define DE_NONE_MSG			'd_No'
#define DE_50_MSG			'd_50'
#define DE_CCIT_MSG			'd_Cc'

//	Flags
#define PRIVATE_MSG			'f_Pv'
#define PROTECTED_MSG		'f_Pr'
#define COPYRIGHT_MSG		'f_Co'
#define ORIGINAL_MSG		'f_Or'

//	Bit Rate
#define BR_32_MSG			'b_32'
#define BR_48_MSG			'b_48'
#define BR_54_MSG			'b_54'
#define BR_56_MSG			'b_56'
#define BR_64_MSG			'b_64'
#define BR_80_MSG			'b_80'
#define BR_96_MSG			'b_96'
#define BR_112_MSG			'b112'
#define BR_128_MSG			'b128'
#define BR_160_MSG			'b160'
#define BR_192_MSG			'b192'
#define BR_224_MSG			'b224'
#define BR_256_MSG			'b256'
#define BR_320_MSG			'b320'
#define BR_384_MSG			'b384'

//	Extract From
#define EXTRACT_FROM_DISK_MSG	'exDF'


#endif

