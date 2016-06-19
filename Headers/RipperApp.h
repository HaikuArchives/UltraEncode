//-------------------------------------------------------------------- 
//      
//      RipperApp.h
//      
//-------------------------------------------------------------------- 

#ifndef __RIPPERAPP_H__ 
#define __RIPPERAPP_H__

#include <Application.h>

//	Class Definition
class RipperApp : public BApplication
{ 
	
	public: 
		RipperApp(); 
		~RipperApp(); 
		
		void	ReadyToRun();
		void    AboutRequested(); 
		void    MessageReceived(BMessage*);
		int32 	TryDir(const char *dir, char *name);
		
		//	Inlines
		inline char		*CDPath(){ return m_CDPath; }
		inline void		CDPath(const char *path){ strcpy(m_CDPath, path); }
		inline BEntry	*PrefsEntry(){ return &m_PrefsEntry; }
	
	private:
		void 	RegisterMp3Attributes();
		
		char 	m_CDPath[B_PATH_NAME_LENGTH];
		BEntry	m_PrefsEntry;	
};

#endif 
