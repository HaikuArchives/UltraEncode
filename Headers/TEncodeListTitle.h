//-------------------------------------------------------------------
//
//	File:	TEncodeListTitle.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.21.99
//
//-------------------------------------------------------------------

#ifndef __TENCODELISTTITLE_H__
#define __TENCODELISTTITLE_H__ 
 
//	Forarwd Declaration
class TQueueListView;

// Class Declarations
class TEncodeListTitle: public BView
{
	public:
		TEncodeListTitle(TQueueListView *parent, BRect bounds);
		~TEncodeListTitle();
		
		void 	MessageReceived(BMessage* message);
		void 	Draw(BRect updateRect);
		void 	FrameResized(float width, float height);
		

		inline void 	ScrollBy(float h, float v){}
		inline void 	ScrollTo(BPoint pt){}
		inline void 	ScrollBTo(float x, float y){}
		
	private:
					
		// Member Functions
		void 	Init();
		
			
		// Member variables
		TQueueListView 	*m_Parent;
};

#endif
