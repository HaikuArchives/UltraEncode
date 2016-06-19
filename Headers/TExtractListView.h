
//---------------------------------------------------------------------
//
//	File:	TExtractListView.h
//
//	Author:	Mike Ost
//
//	Date:	08.11.98
//
//	Desc:	Sub-class of TElementsView. Handles the tab labelled 'Media'
//
//	Copyright ©1998 mediapede Software
//
//---------------------------------------------------------------------

#ifndef __TEXTRACTLISTVIEW_H__
#define __TEXTRACTLISTVIEW_H__ 

// Forware decl
class TRadioBitmapButton;
class ColumnListView;

//	Includes
#include "ColumnListView.h"
#include "CLVListItem.h"
#include "TID3Dialog.h"

//	Forward Declarations
class TProgressDialog;

//	Types
typedef struct
{
	BBitmap *m_MiniIcon;
	BBitmap *m_LargeIcon;
} TSorterIcons;


// Class Declarations
class TExtractListView : public ColumnListView
{
	public:
		// Member functions
		TExtractListView(TProgressDialog *parent, BRect bounds, BScrollView **containerView);
		~TExtractListView();
		
		// Overrides
		virtual void MessageReceived(BMessage* message);
			// This override is needed to resize this tab view when
			// the window is shown. A bug in the BTabView class keeps
			// the resize information from getting to tabs unless they
			// are visible.
			
		void 	MouseDown(BPoint where);
		void 	KeyDown(const char *bytes, int32 numBytes);
		void 	FrameResized(float width, float height);
		void 	AttachedToWindow();
		
		bool	InitiateDrag(BPoint point, int32 index, bool wasSelected);
		
	private:				
		// Member functions
		void Init();
		
		// Member data
		TProgressDialog *m_Parent;
};		


//	Media item 
class TExtractListItem : public CLVListItem
{
	public:
		TExtractListItem(int32 id, const char* text0, const char* text1, 
						 const char* text2, const char* text3);
		~TExtractListItem();
		void DrawItemColumn(BView *owner, BRect item_column_rect, int32 column_index, bool complete);
		void Update(BView *owner, const BFont *font);
		static int MyCompare(const TExtractListItem *Item1, const TExtractListItem *Item2, int32 KeyColumn);
		

		//	Accessor Inlines
		inline int32		ID(){ return m_ID; }
		inline bool			Selected(){ return m_Selected; }
		inline void			Selected(bool val){ m_Selected = val; }
		inline bool			Locked(){ return m_Locked; }
		inline void			Locked(bool val){ m_Locked = val; }
		inline void			SetDialog(TID3Dialog *dialog){ m_Dialog = dialog; }
		inline TID3Dialog	*Dialog(){ return m_Dialog; }

		
	private:
		char		*m_Text[4];
		float 		m_TextOffset;
		int32		m_ID;
		bool		m_Selected;
		bool		m_Locked;		//	Set to true while extraction is in progress
		TID3Dialog 	*m_Dialog;

};


#endif
