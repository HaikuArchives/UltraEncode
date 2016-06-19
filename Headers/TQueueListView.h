//-------------------------------------------------------------------
//
//	File:	TQueueListView.h
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

#ifndef __TQUEUELISTVIEW_H__
#define __TQUEUELISTVIEW_H__

//	Includes
#include <ListView.h>

//	Forward Declarations
class TProgressDialog;

//	Messages

// Class Definition
class TQueueListView : public BListView
{
	public:
		TQueueListView(TProgressDialog *parent, BRect frame);
		virtual ~TQueueListView();
		
		bool	QuitRequested();
		
		void 	MessageReceived(BMessage *message);
		void 	KeyDown(const char *bytes, int32 numBytes);
		void 	FrameResized(float width, float height);
		void 	AttachedToWindow();
		
		bool 		AddItem(BListItem *item);
		bool 		AddItem(BListItem *item, int32 index);
		bool 		RemoveItem(BListItem *item);
		BListItem 	*RemoveItem(int32 item);

		
		//	Inlines
		inline int32	TitleOffset(){ return m_TitleOffset; }
		inline int32	TimeOffset(){ return m_TimeOffset; }
		inline int32	StatusOffset(){ return m_StatusOffset; }
		
		inline void	TitleOffset(int32 val){ m_TitleOffset = val; }
		inline void	TimeOffset(int32 val){ m_TimeOffset = val; }
		inline void	StatusOffset(int32 val){ m_StatusOffset = val; }

		
	private:
		//	Member Functions
		void	DeleteSelectedItems();
		void 	UpdateScrollBar();
		
		//	Member Variables
		TProgressDialog *m_Parent;
		
		int32		m_TitleOffset;
		int32		m_TimeOffset;
		int32		m_StatusOffset;
};

#endif
