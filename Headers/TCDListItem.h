//-------------------------------------------------------------------
//
//	File:	TCDListItem.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.10.99
//
//	Desc:	
//
//	Copyright ©1998 mediapede software
//
//---------------------------------------------------------------------

#ifndef __TCDLISTITEM_H__
#define __TCDLISTITEM_H__

//	Includes
#include <ListItem.h>
#include "TID3Dialog.h"

//	Forward Declarations
class TProgressListView;

// Class Definition
class TCDListItem : public BStringItem
{
	public:
		//	Member Functions
		TCDListItem(TProgressListView *parent, int32 id, const char *text, uint32 outlineLevel = 0, bool expanded = true);
		~TCDListItem();
		
		void 	DrawItem(BView *owner, BRect frame, bool complete = false);
		void	UpdateInfo();
		
		//	Accessor Inlines
		inline int32		ID(){ return m_ID; }
		inline bool			Selected(){ return m_Selected; }
		inline void			Selected(bool val){ m_Selected = val; }
		inline bool			Locked(){ return m_Locked; }
		inline void			Locked(bool val){ m_Locked = val; }
		inline bool			Pending(){ return m_Pending; }
		inline void			Pending(bool val){ m_Pending = val; }
		inline void			SetDialog(TID3Dialog *dialog){ m_Dialog = dialog; }
		inline TID3Dialog	*Dialog(){ return m_Dialog; }
		
		inline	void		IDString(char *str){ strcpy(m_IDString, str); }
		inline	const char	*IDString(){ return m_IDString; }
		inline	void		TitleString(char *str){ strcpy(m_TitleString, str); }
		inline	const char	*TitleString(){ return m_TitleString; }
		inline	void		TimeString(char *str){ strcpy(m_TimeString, str); }
		inline	const char	*TimeString(){ return m_TimeString; }
		inline	void		StatusString(char *str){ strcpy(m_StatusString, str); }
		inline	const char	*StatusString(){ return m_StatusString; }
		inline	ID3_tag		*ID3Tag(){ return &m_ID3Tag; }
		inline	void		EditMode(bool mode){m_EditMode = mode; }
		inline	bool		EditMode(){return m_EditMode; }
		
	private:
	
		// Member Variables
		TProgressListView 	*m_Parent;
		int32				m_ID;
		bool				m_Selected;
		bool				m_Locked;		//	Set to true while extraction is in progress
		bool				m_Pending;		
		TID3Dialog 			*m_Dialog;
		
		ID3_tag				m_ID3Tag;
		
		char		m_IDString[10];
		char		m_TitleString[B_FILE_NAME_LENGTH];
		char		m_TimeString[16];
		char		m_StatusString[32];
		bool		m_EditMode;
};

#endif

