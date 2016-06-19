//-------------------------------------------------------------------
//
//	File:	TProgressListView.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.14.99
//
//	Desc:	
//
//	Copyright ©1999 mediapede software
//
//---------------------------------------------------------------------

#ifndef __TPROGRESSLISTVIEW_H__
#define __TPROGRESSLISTVIEW_H__

//	Includes
#include <ListView.h>

//	Forward Declarations
class TProgressDialog;

//	Messages

// Class Definition
class TProgressListView : public BListView
{
	public:
		TProgressListView(TProgressDialog *parent, BRect frame);
		virtual ~TProgressListView();
		
		bool	QuitRequested();
		
		void 	MessageReceived(BMessage *message);
		void 	KeyDown(const char *bytes, int32 numBytes);
		void 	MouseDown(BPoint where);
		void 	MouseUp(BPoint where);
		void	FrameResized(float width, float height);
		void 	AttachedToWindow();
		void 	Draw(BRect updateRect);
		
		bool 		AddItem(BListItem *item);
		bool 		AddItem(BListItem *item, int32 index);
		bool 		RemoveItem(BListItem *item);
		BListItem 	*RemoveItem(int32 item);
		
		void 		UpdateScrollBar();
		
		//	Inlines
		inline int32	TitleOffset(){ return m_TitleOffset; }
		inline int32	TimeOffset(){ return m_TimeOffset; }
		inline int32	StatusOffset(){ return m_StatusOffset; }
		
		inline void	TitleOffset(int32 val){ m_TitleOffset = val; }
		inline void	TimeOffset(int32 val){ m_TimeOffset = val; }
		inline void	StatusOffset(int32 val){ m_StatusOffset = val; }
		
		inline TProgressDialog	*GetParent(){ return m_Parent; }
		
	private:
		//	Member Functions
		void	DeleteSelectedItems();
		void 	ShowID3Dialog();
		
		//	Member Variables
		TProgressDialog	*m_Parent;
		
		int32		m_TitleOffset;
		int32		m_TimeOffset;
		int32		m_StatusOffset;
		
		BPoint		m_LastClick;
};

#endif
