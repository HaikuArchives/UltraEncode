//-------------------------------------------------------------------
//
//	File:	TAboutBox.h
//
//	Author:	Gene Z. Ragan
//
//	Data:	5.23.97
//
//-------------------------------------------------------------------

#ifndef __TABOUTBOX_H__
#define __TABOUTBOX_H__

class TAboutBox : public BWindow
{
	public:
		TAboutBox();
		~TAboutBox();
		
		virtual void		MessageReceived(BMessage* message);	
		//virtual bool		QuitRequested();
		//virtual void		Refresh();	
		
	private:
		void Init();		
};

#endif
