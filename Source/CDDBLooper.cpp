//-------------------------------------------------------------------
//
//	File:	CDDBLooper.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.20.99
//
//	Desc:	A looper handling communications to the CDDB Server,
//			and loading of cached CDDB entries.
//
//	Copyright ©1999	Mediapede, Inc.
//
//---------------------------------------------------------------------

//	Includes
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include <Application.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Alert.h>

#include "DebugBuild.h"
#include "AppUtils.h"
#include "ResourceManager.h"

#include "TInexactMatchWindow.h"
#include "cddb.h"

#include "CDDBLooper.h"


//	Message
const char *quit_msg = "quit\n";
const char *status_msg = "Copyright Info "B_UTF8_COPYRIGHT"1999";

//	Constants
const char *kAppSettingsFolder = "UltraEncode Folder";


CDDBLooper::CDDBLooper(const char *name, long priority, const char *server, int32 server_port) : 
			BLooper(name, priority),
			m_InitCheck(false) 
{
	strcpy(cddbServer, server);
	port = server_port;
	Run();
}

CDDBLooper::~CDDBLooper() 
{
}

void CDDBLooper::Quit() 
{
	BLooper::Quit();
}

void CDDBLooper::MessageReceived(BMessage *msg)
{
	FUNCTION("CDDBLooper::MessageReceived() \n");	
	
	int8 numTracks;
	const char *category;
	const char *strdiscID;
	cddb_toc *cddbTOC;
	
	switch (msg->what)
	{
		case CDDB_LOOKUP:
			msg->FindInt8("numTracks",&numTracks);
			msg->FindPointer("cddb_toc", (void **)&cddbTOC);
			m_ReplyTo = msg->ReturnAddress();
			CDDBLookup(numTracks, cddbTOC);
			break;
			
		case CDDB_CACHE_LOOKUP:
			msg->FindInt8("numTracks",&numTracks);
			msg->FindPointer("cddb_toc", (void **)&cddbTOC);
			m_ReplyTo = msg->ReturnAddress();
			CDDBLookup(numTracks, cddbTOC);
			break;
			
		case CDDB_SITES:
			m_ReplyTo = msg->ReturnAddress();
			CDDBSites();
			break;
						
		case CDDB_READ:
			{
				msg->FindString("category", &category);
				msg->FindString("disc_id", &strdiscID);
				CDDBRead(category, strdiscID);
			}
			break;
			
		default:
			BLooper::MessageReceived(msg);
			break;
	}
}

// Initializes the socket and connects to an address.
status_t CDDBLooper::OpenCDDBConnection() 
{
	FUNCTION("CDDBLooper::OpenCDDBConnection() - ENTER -\n");
		
	//	Make sure we can have net connection
	PROGRESS("Server: %s\n", cddbServer);
	hostent *h;
	h = gethostbyname(cddbServer);
	if (!h) 
	{
		ERROR("CDDBLooper::OpenCDDBConnection() - Error resolving hostname -\n");
		return B_IO_ERROR;
	}
	
	uint32 hostaddr = *(uint32 *)h->h_addr_list[0];
	/*
	if (hostaddr==-1) 
	{
		ERROR("CDDBLooper::OpenCDDBConnection() - Error resolving hostname -\n");
		return B_IO_ERROR;
	}
	*/
	
	sockaddr_in address;
	::memset(&address,0,sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = hostaddr;
	
	sock_id = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sock_id < 0) 
	{
		ERROR("CDDBLooper::OpenCDDBConnection() - Error creating socket -\n");
		return B_IO_ERROR;
	}
	
	if (::connect(sock_id, (sockaddr *)&address, sizeof(address)) < 0) 
	{
		ERROR("CDDBLooper::OpenCDDBConnection() - Error connecting socket -\n");
		return B_IO_ERROR;
	}
	
	/*
	// Address to use
	BTSAddress *addr= new BTSAddress(AF_INET,888,cddbServer);
	if (addr==NULL) {
		PROGRESS("Could not construct address\n");
		return B_IO_ERROR;	
	}
		
	// Open up the socket
	sock = new BTSSocket(SOCK_STREAM,0);
//	result = sock->Open();
	// Check for errors
	if (sock->ID()<0) {
		PROGRESS("Couldn not open socket\n");
	}
	int32 *a;
	PROGRESS("size %d\n",sizeof(a));
	// Connect to server
	result = sock->ConnectToAddress(*addr);
	// Check for errors
	if (result<0) {
		PROGRESS("Couldn not Connect socket\n");
		sock->Close();
		return B_IO_ERROR;
	}
	*/
	
	PROGRESS("Leaving OpenCDDBConnection()\n");
	return B_OK;
}

// Closes down the socket connection to the CDDB server
status_t CDDBLooper::CloseCDDBConnection() 
{
	status_t err=B_OK;
	PROGRESS("CDDBLooper::CloseCDDBConnection()\n");	

	//	if (sock) {
	//		err =  sock->Close();
	//	}
	//	delete sock;

	SetStatusMessage("Connection closed.");

	return err;
}

// Give the hello handshake
void CDDBLooper::CDDBHello()
{
	char *response;
	
	PROGRESS("CDDBLooper::CDDBHello()\n");	

	// Get the user's login
	char username[256];
	if (!getusername(username,256))
		strcpy(username,"unknown");
	
	// Lookup the user's host name
	char hostname[MAXHOSTNAMELEN];
	hostent *h;
	if (gethostname(hostname, MAXHOSTNAMELEN))
	{
		h = gethostbyname(hostname);
		if (!h)
			strcpy(hostname,"unknown");
		else
		{
			strcpy(hostname, h->h_name);
			//delete h;
		}
	}
	else
		strcpy(hostname,"unknown");

	// Shake hands
	char helo[256];
	sprintf(helo, "cddb hello %s %s CDPlayer v1.0\n", username, hostname);
	SendString(helo);
	response = ReceiveString();
	SetStatusMessage(response);
	delete [] response;
}

// Send the quit message and closes teh connection.
void CDDBLooper::CDDBQuit()
{
	PROGRESS("CDDBLooper::CDDBQuit()\n");	
	char *response;
	SendString(quit_msg);
	response = ReceiveString();
	delete [] response;
	CloseCDDBConnection();
}


//-------------------------------------------------------------------
//	TryDir
//-------------------------------------------------------------------
//
//	Performs a simple query, and handles inexact matches.
//

void CDDBLooper::CDDBQuery(unsigned long discID, int32 numTracks, cddb_toc *cdtoc)
{
	PROGRESS("CDDBLooper::CDDBQuery()\n");	
	char *response;
	char query[256];
	char temp[32];
	char strCDDBid[9];
	
	//	Compute the total length of the CD.
	long cd_length = cdtoc[numTracks].min*60+cdtoc[numTracks].sec;

	//	Format the query
	sprintf(query,"cddb query %08x %d ",discID,numTracks);
	
	//	Add frame offsets
	for (int32 i=0; i < numTracks; i++)
	{
		sprintf(temp, "%d ",cdtoc[i].min*4500+cdtoc[i].sec*75+cdtoc[i].frame);
		strcat(query,temp);
	}
	
	//	Finish it off with the total Cd length.
	sprintf(temp,"%d\n",cd_length);
	strcat(query,temp);
	
	//	Send it off.
	SendString(query);
	response = ReceiveString();
	SetStatusMessage(response);
	
	//	PROGRESS("CDid is %08x\n",cddb_id);
	// Error retrieving cd info
	if (response[0] != '2') 
	{
		CDDBQuit();
		return;
	}
	sprintf(strCDDBid,"%08x",discID);
	
	char junk[256];
	char category[80];
	
	// Handling of 211 Inexact Match code
	if (sscanf(response,"211 %s",&junk))
	{
		PROGRESS("CDDBLooper::CDDBQuery() - Inexact Match -\n");
		
		BMessage *message = GetWindowFromResource("InexactMatchWindow");	
		TInexactMatchWindow *matchWindow = new TInexactMatchWindow(message);
		CenterWindow(matchWindow);
		matchWindow->Show();
		BMessage *matchMakerMsg;
		response = ReceiveString();
		while(strcmp(response,"."))
		{
			PROGRESS("CDDBLooper::CDDBQuery() - Response: %s\n", response);			
			matchMakerMsg = new BMessage(ADD_CD);
			matchMakerMsg->AddString("info",response);
			matchWindow->PostMessage(matchMakerMsg,NULL,this);
			delete [] response;
			delete matchMakerMsg;
			response = ReceiveString();
		}
	}
	else
	{
		// Grab the category from the response
		if (!sscanf(response,"200 %s %s",&category,&junk))
		{
			// Hmm, didn't find it.  Try under "misc"
			strcpy(category,"misc");
		}
		CDDBRead(category,strCDDBid);
	}
	
	delete [] response;
}


void CDDBLooper::CDDBRead(const char *category, char *discid)
{
	FUNCTION("CDDBLooper::CDDBRead()\n");
	
	char *response;
	char query[256];

	//	Read the CDDB Info
	sprintf(query,"cddb read %s %s\n",category,discid);
	SendString(query);
	response = ReceiveString();
	SetStatusMessage(response);
	if (!strstr(response,"210"))
	{
		CDDBQuit();
		PROGRESS("Not in database, connection closed.\n");
		return;
	}
	
	//	Get the file ready for writing...
	char cache_path[256];
	BFile *cdFile;
	BDirectory *cacheDir;
	BDirectory *cdDir;
	//char *line;
	
	//	Compute the file name the disc would be in...
	find_directory(B_USER_SETTINGS_DIRECTORY, 0, true, cache_path, 256);
	cacheDir = new BDirectory(cache_path);
	cdDir 	 = new BDirectory();	
	if (cacheDir->InitCheck()!=B_OK) 
		ERROR("Could not set directory %s\n",cache_path);
		
	cacheDir->CreateDirectory(kAppSettingsFolder, cdDir);
	strcat(cache_path,"/");
	strcat(cache_path, kAppSettingsFolder);
	cdDir->SetTo(cache_path);
	if (cdDir->InitCheck()!=B_OK) 
		ERROR("Could not set directory %s/CDPlayer\n",cache_path);
	
	// Create the file object.
	cdFile = new BFile(cdDir,discid,B_WRITE_ONLY|B_CREATE_FILE);
	
	// Error checking.
	if (cdFile)
	{
		if (!cdFile->IsWritable())  
		{
			cdFile->Unset();
			cdFile = NULL;
			PROGRESS("Error writing disc data.\n");
		}
	}
	SetStatusMessage("Writing CD info to cache.");
	
	// Get rid of all that garbage.
	while (!strstr(response,"DTITLE")) 
	{
		delete [] response;
		response = NULL;
		response=ReceiveString();
		if (cdFile) cdFile->Write(response,strlen(response));
		if (cdFile) cdFile->Write("\n",1);
	}
	
	//	Found the title.
	if (strstr(response,"DTITLE")) 
	{
		char *parse_str;
		char *title;
		char *artist;
		
		//	Try to parse the title/author via a slash or dash
		parse_str = strchr(response,'=')+1;
		
		artist = strtok(parse_str,"/-");
		title = strtok(NULL,"/-");
		if (title) title++;
		else title = artist;
				
		// and store into the global structure
		strcpy(fCDInfo.disc_title,title);
		strcpy(fCDInfo.disc_artist,artist);
	}
	
	delete [] response;
	response = NULL;
	
	//	Read in the track names, and replace all the old untitled menu
	//	items with the new, titled ones.
	int32 i=0;
	char *tempTrackName;
	char trackname[256];
	response = ReceiveString();
	
	if (cdFile) 
		cdFile->Write(response,strlen(response));
	if (cdFile) 
		cdFile->Write("\n",1);		
	strcpy(trackname,"");
	
	do {
		int32 trackNum;
		if (!sscanf(response,"TTITLE%d",&trackNum)) break;
		tempTrackName=strchr(response,'=')+1;
		
		if (trackNum==i) 
		{
			//strcat(trackname," ");
			strcat(trackname,tempTrackName);
		} 
		else
		 
		{
			// Store track name in global cd_info structure.
			strcpy(fCDInfo.track[i].track_name,trackname);
			strcpy(trackname,tempTrackName);
			i++;
		}
		delete [] response;
		response = NULL;
		response = ReceiveString();
		if (cdFile) cdFile->Write(response,strlen(response));
		if (cdFile) cdFile->Write("\n",1);
	} while (!strstr(response,"EXTD"));
	
	delete [] response;
	response = NULL;
	
	// Add the last one.
	strcpy(fCDInfo.track[i].track_name,trackname);
	
	// Read the rest of the data, mainly just to put in the cached copy
	response = ReceiveString();
	while (response[0]!='.') 
	{
		if (cdFile) cdFile->Write(response,strlen(response));
		if (cdFile) cdFile->Write("\n",1);
		delete [] response;
		response = NULL;
		response = ReceiveString();
	}
	
	PROGRESS("PREPARING CD_INFO MESSAGE\n");
	BMessage msg(CD_INFO);
	PROGRESS("ADDING DATA TO CD_INFO MESSAGE\n");
	msg.AddData("cd_info", B_RAW_TYPE, &fCDInfo, sizeof(cd_info));
	PROGRESS("SENDING CD_INFO MESSAGE fCDInfo = %08x, size = %08x\n", &fCDInfo, sizeof(cd_info));
	m_ReplyTo.SendMessage(&msg);
	CDDBQuit();
	
	delete [] response;
}

void CDDBLooper::CDDBSites() 
{
	PROGRESS("CDDBLooper::CDDBSites()\n");	
	char *response;
	status_t err;
	
	// Talk to the CDDB server
	err = OpenCDDBConnection();
	if (err != B_OK) 
	{
		//(new BAlert("CDPlayer", "Network Error: Could not open socket.","Sorry"))->Go();
		ERROR("CDDBLooper::CDDBSites() - Could not open socket -\n");
		CloseCDDBConnection();
		m_InitCheck = false;
		return;
	}
	
	// Let 'em know we're connecting
	SetStatusMessage("Connecting to CDDB Server.");
	response = ReceiveString();
	SetStatusMessage(response);
	delete [] response;
	response = NULL;
	
	// Handshake with the server
	CDDBHello();

	char query[256];

	// Perform sites query
	sprintf(query,"sites\n");
	SendString(query);
	response = ReceiveString();
	SetStatusMessage(response);
	if (!strstr(response,"210")) 
	{
		CDDBQuit();
		ERROR("Server %s does not support sites.\n", cddbServer);
		return;
	}
	delete [] response;
	response = NULL;
	
	// Read in and parse up the sites
	char *site;
	char *str_port;
	int16 port;
	char *latitude;
	char *longitude;
	char *description;
	
	response = ReceiveString();
	BMessage *msg = new BMessage(SITE_INFO);
	while(strcmp(response,".")) 
	{
		site = strtok(response," ");
		str_port = strtok(NULL," ");
		port = atoi(str_port);
		latitude = strtok(NULL," ");
		longitude = strtok(NULL," ");
		description = strtok(NULL," ");
		
		msg->AddString("site",site);
		msg->AddString("latitude",latitude);
		msg->AddString("longitude",longitude);
		msg->AddString("description",description);
		msg->AddInt16("port",port);
		delete [] response;
		response = ReceiveString();
	}
	delete [] response;
	PROGRESS("SENDING SITES MESSAGE\n");
	m_ReplyTo.SendMessage(msg);
	delete msg;
	
	// Close off the connection
	CDDBQuit();
}

void CDDBLooper::CDDBWrite() {
}

void CDDBLooper::CDDBSearch() {
}

void CDDBLooper::CDDBProto() {
}

void CDDBLooper::SetStatusMessage(const char *msgString) 
{
	FUNCTION("CDDBLooper::SetStatusMessage()\n");
	
	PROGRESS(msgString);
	BMessage msg(STATUS_MSG);
	msg.AddString("status_msg", msgString);
	m_ReplyTo.SendMessage(&msg);
}

void CDDBLooper::CDDBLookup(int32 numTracks, cddb_toc *cddbTOC)
{
	//const void * data;
	//ssize_t size;
	//uint32 type = '    ';
	bool fCDDB;
	//char strCDDBid[9];
	PROGRESS("Inside CDDBLookup\n");
	
	// Calculate disc id
	uint32 cddb_id = CDDBDiscID(numTracks, cddbTOC);
	fCDInfo.numtracks = numTracks;
	
	// Try to load it out of cache
	if (LoadCachedData(cddb_id)) 
		return;
	
	// Try to load the use CDDB option out of libprefs, default to false (no lookup)
	//if (gSettings.GetData("use_cddb", data, size, type) || (type != B_BOOL_TYPE)) 
	//{
		fCDDB = true;
	//} 
	//else 
	//{
	//	memcpy(&fCDDB, data, size);
	//}
	PROGRESS("After GetData(use_cddb)\n");
	
	// Don't bother, they don't want networking anyway
	if (!fCDDB)
	{
		SetStatusMessage(status_msg);
		//	return;
	}
	
	char *response;
	status_t err;
	
	// Talk to the CDDB server
	err = OpenCDDBConnection();
	if (err != B_OK)
	{
		//(new BAlert("CDPlayer","Network Error: Could not open socket.","Sorry"))->Go();
		ERROR("CDDBLooper::CDDBLookup() - Could not open socket -\n");
		CloseCDDBConnection();
		return;
	}
	
	// Let 'em know we're connecting
	SetStatusMessage("Connecting to CDDB Server.");
	response = ReceiveString();
	SetStatusMessage(response);
	delete [] response;
	
	// Handshake with the server
	CDDBHello();
	CDDBQuery(cddb_id, numTracks, cddbTOC);
	
}

//	Loads a CDDB entry out of the cache
bool CDDBLooper::LoadCachedData(uint32 discid) 
{
	FUNCTION("CDDBLooper::LoadCachedData() \n");	
	
	char cache_path[256];
	char disc_name[9];
	BFile *cdFile;
	BDirectory *cacheDir;
	BDirectory *cdDir;
	char *buffer;
	off_t size;
	char *line;
	
	SetStatusMessage("Checking cache...");
	
	//	Compute the file name the disc would be in...
	find_directory(B_USER_SETTINGS_DIRECTORY, 0, true, cache_path, 256);
	cacheDir = new BDirectory(cache_path);
	cdDir 	 = new BDirectory();
	
	if (cacheDir->InitCheck()!= B_OK) 
		ERROR("Could not set directory %s\n",cache_path);
		
	cacheDir->CreateDirectory(kAppSettingsFolder, cdDir);
	strcat(cache_path,"/");
	strcat(cache_path, kAppSettingsFolder);
	cdDir->SetTo(cache_path);
	if (cdDir->InitCheck()!= B_OK) 
		ERROR("Could not set directory %s/CDPlayer\n",cache_path);
	
	//	Create the file object.
	sprintf(disc_name,"%08x",discid);
	cdFile = new BFile(cdDir,disc_name,B_READ_ONLY);
	
	//	Error checking.
	if (!cdFile) 
		return false;
		
	if (!cdFile->IsReadable()) 
		return false;
		
	SetStatusMessage("Reading CD info from cache.");
	
	//	Read the buffer
	cdFile->GetSize(&size);
	buffer = new char[size+1];
	char *delete_me = buffer;
	cdFile->Read(buffer, size);
	buffer[size+1] = '\0';
	
	//	Grab out the stuff...
	line = strtok(buffer,"\n");
	while (line) 
	{
		buffer += strlen(line)+1;
		
		//	Found the title.
		if (strstr(line,"DTITLE")) 
		{
			char *parse_str;
			char *title;
			char *artist;
			
			// Try to parse the title/author via a slash or dash
			parse_str = strchr(line,'=')+1;
			
			artist = strtok(parse_str,"/-");
			title = strtok(NULL,"/-");
			if (title) 
				title++;
			else 
				title = artist;
				
			//	and store into the info structure
			strcpy(fCDInfo.disc_title,title);
			strcpy(fCDInfo.disc_artist,artist);
			break;
		}
		line = strtok(buffer,"\n");
	}
	//status_t err;
	
	// Read in the track names, and replace all the old untitled items with the new ones
	int32 i=0;
	char *tempTrackName;
	char trackname[256];
	
	line = strtok(buffer,"\n");
	buffer += strlen(line)+1;
	strcpy(trackname,"");
	do {
		int32 trackNum;
		if (!sscanf(line,"TTITLE%d",&trackNum)) 
			break;
			
		tempTrackName=strchr(line,'=')+1;
		if (trackNum == i) 
		{
			//	This annoyed the heck out of Steve...
			//strcat(trackname," ");
			strcat(trackname,"");
			strcat(trackname,tempTrackName);
		} 
		else 
		{
			// Store track name in global cd_info structure.
			strcpy(fCDInfo.track[i].track_name,trackname);
			strcpy(trackname,tempTrackName);
			i++;
		}
		
		line = strtok(buffer,"\n");
		buffer += strlen(line)+1;
	} while (!strstr(line,"EXTD"));
	
	// Add the last one.
	// Store track name in global cd_info structure.
	strcpy(fCDInfo.track[i].track_name,trackname);
	
	// Send the info out to the caller
	BMessage msg(CD_INFO);
	msg.AddData("cd_info",B_RAW_TYPE,&fCDInfo,sizeof(cd_info));
	PROGRESS("SENDING CD_INFO MESSAGE\n");
	m_ReplyTo.SendMessage(&msg);
	
	SetStatusMessage("Found CD info in cache.");
		
	// Clean up
	delete cacheDir;
	delete cdDir;
	delete cdFile;
	delete [] delete_me;
	return true;
}

// Sends a string over the socket
status_t CDDBLooper::SendString(const char *str) 
{
	PROGRESS("CDDBLooper::SendString()\n");	
//	if (!sock)
//		return B_ERROR;
//	sock->Send(str,strlen(str));
	::send(sock_id, str, strlen(str), 0);
	PROGRESS(">%s\n",str);
	return B_OK;
}

// Reads a string from the socket
char *CDDBLooper::ReceiveString() 
{
	int32 buffer_size = 1024;
	char *return_str;
	char *buffer= new char[buffer_size];
	int32 recd_bytes=-1;
	int32 num_bytes = 0;
	while (recd_bytes != 0 && num_bytes<buffer_size-1 && buffer[num_bytes-1]!='\n') {
		recd_bytes = recv(sock_id,buffer+num_bytes,1,0);
		num_bytes += recd_bytes;
	}
	buffer[num_bytes-2] = '\0';
	return_str = new char[num_bytes];
	strcpy(return_str,buffer);
	PROGRESS("'%s'\n",return_str);
	
	if (strlen(return_str) > 64) 
		ERROR("CDDBLooper::ReceiveString - STRING TOO LONG!\n");
		
	delete[] buffer;
	return return_str;
}


//	Computes the disc id based on the # of tracks and the toc.
uint32 CDDBLooper::CDDBDiscID(int32 tot_trks, cddb_toc *cdtoc)
{
	PROGRESS("CDDBLooper::CDDBDiscID()\n");	
	int32 i;
	int32 n=0;
	int32 t=0;
	
	// For backward compatibility this algorithm must not change
	for (i = 0; i < tot_trks; i++)
	{
		n += CDDBSum((cdtoc[i].min * 60) + cdtoc[i].sec);
		t+=	((cdtoc[i+1].min * 60)+ cdtoc[i+1].sec) -
			((cdtoc[i].min*60)+cdtoc[i].sec);
	}
	return (((n%0xff) << 24) + (t << 8) + tot_trks);
}

//	A helper function for computing the disc id.
int32 CDDBLooper::CDDBSum(int32 n)
{
	char buf[12];
	char *p;
	unsigned long ret = 0;
	
	//	For backward compatibility this algorithm must not change
	sprintf(buf, "%lu", n);
	for (p=buf; *p != '\0'; p++)
		ret += (*p-'0');
	return ret;
}
