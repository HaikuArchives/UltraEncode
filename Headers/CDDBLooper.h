/*
	File:	CDDBLooper.h
	Author:	Chip Paul(cpaul@resgen.com)
	Function:
		A looper handling communications to the CDDB Server,
		and loading of cached CDDB entries.
	Incoming Messages:
		CDDB_LOOKUP
			"numTracks", int8,  number of tracks on disk
		CDDB_SITES
			none
	OutGoing Messages:
		CD_INFO
			"cd_info", struct cd_info,  CD track information
		STATUS_MSG
			"status_msg", string, information on current status
		SITE_INFO
			"site", string, internet address of server
			"port", int16, port to connect to
			"latitude", string, latitude of server location
			"longitude", string, longitude of server location
			"description", string, description of physical location
*/

#ifndef CDDBLOOPER_H
#define CDDBLOOPER_H

#include "cddb.h"

// Incoming messages
#define CDDB_LOOKUP 		'lkup'
#define CDDB_CACHE_LOOKUP 	'clup'
#define CDDB_SITES			'site'
#define CDDB_WRITE			'wrte'
#define CDDB_QUERY			'qury'
#define CDDB_READ			'read'
#define CDDB_SEARCH			'srch'

// Outgoing messages
#define CD_INFO				'info'
#define STATUS_MSG 			'smsg'
#define SITE_INFO			'stsi'

//	Messages
class CDDBLooper : public BLooper
{

	public:
		CDDBLooper(const char *name, long priority, const char *server, int32 port);
		~CDDBLooper();
		void	Quit();
		void	MessageReceived(BMessage *message);
		
		//	Inline Accessors		
		inline bool InitCheck(){ return m_InitCheck; };
		inline void	UpdateServer(char *server){ strcpy(cddbServer, server); }
	
	private:
		// The server to connect to
		char cddbServer[64];
		int32 sock_id;
		int32 port;
		
		// CD information structure
		cd_info 	fCDInfo;
		BMessenger 	m_ReplyTo;
		
		// Sends a string over the socket
		status_t	SendString(const char *str);
		// Receives a string over the socket
		char *		ReceiveString();
		// Opens the connection
		status_t	OpenCDDBConnection();
		// Closes the connection
		status_t	CloseCDDBConnection();
		// Sends the hello command
		void 	CDDBHello();
		bool 	LoadCachedData(uint32 discID);
		// Sends the quit command
		void 	CDDBQuit();
		// Performs a query command
		void CDDBQuery(unsigned long discID, int32 numTracks, cddb_toc *cdtoc);
		// Performs a read command
		void CDDBRead(const char *category, char *discid);
		// Performs a sites command
		void CDDBSites();
		// Performs a write command
		void CDDBWrite();
		// Performs a search command
		void CDDBSearch();
		// Peforms a proto command
		void CDDBProto();
		
		// Does Hello, Query, Read, Quit
		void 	CDDBLookup(int32 numTracks, cddb_toc *cddbTOC);
		void 	SetStatusMessage(const char *msgString);
		uint32 	CDDBDiscID(int32 tot_trks, cddb_toc *cdtoc);
		int32 	CDDBSum(int32 n);
		
		bool m_InitCheck;
	
};

#endif
