//-------------------------------------------------------------------
//
//	File:	TExtractListTitle.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	05.25.98
//
//-------------------------------------------------------------------

#ifndef __TEXTRACTLISTTITLE_H__
#define __TEXTRACTLISTTITLE_H__ 
 
//	Forarwd Declaration
class TProgressListView;

// Class Declarations
class TExtractListTitle: public BView
{
	public:
		TExtractListTitle(TProgressListView *parent, BRect bounds);
		~TExtractListTitle();
		
		void 	MessageReceived(BMessage* message);
		void 	Draw(BRect updateRect);
		void 	FrameResized(float width, float height);
		void 	MouseDown(BPoint where);
		
		inline void 	ScrollBy(float h, float v){}
		inline void 	ScrollTo(BPoint pt){}
		inline void 	ScrollBTo(float x, float y){}
		
	private:
					
		// Member Functions
		void 	Init();
		
			
		// Member variables
		TProgressListView 	*m_Parent;
};

#endif
