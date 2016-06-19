//---------------------------------------------------------------------
//
//	File:	AppUtils.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	01.26.98
//
//	Desc:	Misc utility routines
//
//	Copyright ©1998 mediapede Software
//
//---------------------------------------------------------------------

// Includes
#include <string.h>

#include <Application.h>
#include <InterfaceDefs.h>
#include <Directory.h>
#include <Path.h>
#include <Debug.h>
#include <TimeCode.h>
#include <MediaFile.h>
#include <MediaTrack.h>
#include <ScrollBar.h>
#include <Screen.h>
#include <Bitmap.h>
#include <NodeInfo.h>
#include <TextControl.h>
#include <SimpleGameSound.h>

#include "DebugBuild.h"

#include "AppUtils.h"


#pragma mark -
#pragma mark === Keyboard Routines ===


//---------------------------------------------------------------------
//	IsKeyDown
//---------------------------------------------------------------------
//
//	Check for down state of indicated key
//

bool IsKeyDown( char theKey)
{
	key_info keyInfo;
	
	get_key_info(&keyInfo);
	
	if ( keyInfo.key_states[theKey >> 3] & (1 << (7 - (theKey % 8))) )
		return true;
	else 
		return false;
}


//---------------------------------------------------------------------
//	IsShiftKeyDown
//---------------------------------------------------------------------
//
//	Check for down state of shift key
//

bool IsShiftKeyDown()
{
	
	key_info keyInfo;
	
	get_key_info(&keyInfo);
	
	if (keyInfo.modifiers & B_SHIFT_KEY)
		return true;
	else
		return false;
}


//---------------------------------------------------------------------
//	IsCommandKeyDown
//---------------------------------------------------------------------
//
//	Check for down state of command key
//

bool IsCommandKeyDown()
{
	key_info keyInfo;
	
	get_key_info(&keyInfo);
	
	if (keyInfo.modifiers & B_COMMAND_KEY)
		return true;
	else 
		return false;
}

//---------------------------------------------------------------------
//	IsOptionKeyDown
//---------------------------------------------------------------------
//
//	Check for down state of option key
//

bool IsOptionKeyDown()
{
	key_info keyInfo;
	
	get_key_info(&keyInfo);
	
	if (keyInfo.modifiers & B_OPTION_KEY)
		return true;
	else 
		return false;
}

//---------------------------------------------------------------------
//	IsControlKeyDown
//---------------------------------------------------------------------
//
//	Check for down state of control key
//

bool IsControlKeyDown()
{
	key_info keyInfo;
	
	get_key_info(&keyInfo);
	
	if (keyInfo.modifiers & B_CONTROL_KEY)
		return true;
	else 
		return false;
}



//---------------------------------------------------------------------
//	AdjustScrollBar
//---------------------------------------------------------------------
//
//	Check for down state of control key
//

void AdjustScrollBar( BScrollBar *scrollBar, float page, float pageStep, float total, float start)
{	
	//	Set range
	if (total <= page) 
	{
		scrollBar->SetRange(start, start);
	} 
	else 
	{
		scrollBar->SetRange(start, start + total - page);
	}
	
	//	Set steps and proportion
	scrollBar->SetSteps(pageStep, pageStep*4);	
	scrollBar->SetProportion(page/total);	
}

#pragma mark -
#pragma mark === Window Utilities ===

//---------------------------------------------------------------------
//	FindWindow
//---------------------------------------------------------------------
//
//	Return a list containing matching windows.  Caller must free
//	the list when done.
//


BList *FindWindow(const char *title)
{
	BWindow 	*theWindow = NULL;
	const char *tmpTitle;
	
	//	Create list
	BList *windowList = new BList();
	
	for ( int32 index = 0; index < be_app->CountWindows(); index++)
	{
		theWindow = be_app->WindowAt(index);
	     
		if (theWindow)
		{ 
			if (theWindow->Lock())
			{	     
				tmpTitle = theWindow->Title();
				
				//	Add to list if we have a match
				if( tmpTitle != NULL && strcmp( tmpTitle, title) == 0) 
					windowList->AddItem(theWindow);
					
				theWindow->Unlock();
			}
		}
	}
	 
	return windowList;
}


//---------------------------------------------------------------------
//	CenterWindow
//---------------------------------------------------------------------
//
//	Center window on current screen
//

void CenterWindow(BWindow *theWindow)
{
	// Get screen settings
	BScreen theScreen(B_MAIN_SCREEN_ID);
	BRect screenBounds = theScreen.Frame();
	BRect windBounds = theWindow->Frame();
	
	theWindow->Lock();
	theWindow->Hide();
	theWindow->MoveTo( (screenBounds.Width() - windBounds.Width())/2, (screenBounds.Height() - windBounds.Height())/2);	
	theWindow->Show();
	theWindow->Unlock();	
}



#pragma mark -
#pragma mark === Unit Utilities ===

//---------------------------------------------------------------------
//	DiffRect
//---------------------------------------------------------------------
//
//	Calculate the difference between two rects
//

bool DiffRect( BRect *rectOne, BRect *rectTwo, BRect *dstRect)
{
	if (rectOne->Intersects( *rectTwo ) )
	{
		if (rectOne->Width() > rectTwo->Width() )
		{
			// Determine shared side of rect
			//
			
			// Left sides match
			if (rectOne->left == rectTwo->left)
			{
				dstRect->left 	= rectTwo->right;
				dstRect->top 	= rectTwo->top;
				dstRect->right 	= rectOne->right;
				dstRect->bottom	= rectTwo->bottom;					
			}
			// right sides match
			else if ( rectOne->right == rectTwo->right)
			{			
				dstRect->left 	= rectOne->left;
				dstRect->top 	= rectOne->top;
				dstRect->right 	= rectTwo->left;
				dstRect->bottom	= rectOne->bottom;								
			}
			// no shared side...
			else
				return false;
		}
		else
		{
			// Determine shared side of rect
			//
			
			// Left sides match
			if (rectOne->left == rectTwo->left)
			{
				dstRect->left 	= rectOne->right;
				dstRect->top 	= rectOne->top;
				dstRect->right 	= rectTwo->right;
				dstRect->bottom = rectOne->bottom;			
			}
			// right sides match
			else if ( rectOne->right == rectTwo->right)
			{			
				dstRect->left 	= rectTwo->left;
				dstRect->top 	= rectTwo->top;
				dstRect->right 	= rectOne->left;
				dstRect->bottom	= rectTwo->bottom;								
			}
			// no shared side...
			else
				return false;		
		}
		return true;
	}
	
	return false;		
}

#pragma mark -
#pragma mark === Bitmap Utilities ===
/*
//---------------------------------------------------------------------
//	FillBitmap
//---------------------------------------------------------------------
//
//	Fill bitmap with color
//

void FillBitmap(BBitmap * map, uint32 color) 
{
	ASSERT(map->ColorSpace() == B_RGB_32_BIT);
	ASSERT(sizeof(double) == 8);
	
	uchar *dst = (uchar *)map->Bits();
	uchar *end = dst+map->BitsLength();
	
	if (((ulong)dst)&4) 
	{
		*(uint32*)dst = color;
		dst += 4;
	}
	
	double d;
	uint32 a[2] = { color, color };
	
	d = *(double *)a;
	while (end-dst > 7) 
	{
		*(double *)dst = d;
		dst += 8;
	}
	
	if (end-dst > 3) 
		*(uint32*)dst = color;
}
*/

//---------------------------------------------------------------------
//	FillBitmap
//---------------------------------------------------------------------
//
//	Fill bitmap with color
//

void FillBitmap(BBitmap *bitmap, rgb_color color)
{	
	#if B_HOST_IS_LENDIAN
		#define ARGB_FORMAT B_RGBA32_LITTLE
		struct ARGBPixel { uint8 b,g,r,a; };
	#else
		#define ARGB_FORMAT B_RGBA32_BIG
		struct ARGBPixel { uint8 a,r,g,b; };
	#endif

	//	Set up properties
	int32 x 	= bitmap->Bounds().IntegerWidth()+1;
	int32 y 	= bitmap->Bounds().IntegerHeight()+1;	

	ARGBPixel pixel, *bits = (ARGBPixel *)bitmap->Bits();
	
	int32 offs = bitmap->BytesPerRow() / 4 - x;
	
	pixel.r = color.red;
	pixel.g = color.green;
	pixel.b = color.blue;
		
	//	Set bits
	for (int32 yy = 0; yy < y; yy++)
	{
		for (int32 xx = 0; xx < x; xx++)
		{			
			*bits++ = pixel;
		}
		
		bits += offs;
	}
}


//---------------------------------------------------------------------
//	CloneBitmap
//---------------------------------------------------------------------
//
//	Clone bitmap
//

BBitmap *CloneBitmap(const BBitmap &input)
{
	BBitmap *clone = new BBitmap(input.Bounds(), input.ColorSpace());
	ASSERT(input.BytesPerRow() == clone->BytesPerRow());
	ASSERT(input.BitsLength() <= clone->BitsLength());
	memcpy(clone->Bits(), input.Bits(), input.BitsLength());
	return clone;
}


//---------------------------------------------------------------------
//	MakeTransparentBitmap
//---------------------------------------------------------------------
//
//	Clone bitmap and apply transparency
//

BBitmap *MakeTransparentBitmap(BBitmap *srcBitmap, uchar transparency)
{
	BRect tranRect 		= srcBitmap->Bounds();
	BView *dragView 	= new BView(tranRect, "dragView", B_FOLLOW_NONE, 0);
	BBitmap *dragBitmap = new BBitmap(tranRect, B_RGBA32, true);
	ASSERT(dragBitmap);
	if (dragBitmap->Lock())
	{
		dragBitmap->AddChild(dragView);			
		dragView->SetHighColor(0, 0, 0, 0);
		dragView->FillRect(dragView->Bounds());
		dragView->SetDrawingMode(B_OP_ALPHA);
		dragView->SetHighColor(0, 0, 0, transparency);
		dragView->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE);
		dragView->DrawBitmap(srcBitmap);
		dragBitmap->RemoveChild(dragView);
		delete dragView;
		dragBitmap->Unlock();
	}
	
	return dragBitmap;
}


//---------------------------------------------------------------------
//	SetWhiteTransparent8
//---------------------------------------------------------------------
//
//	Set white value in 8-bit bitmap to transparent.  63 is the index
//	of white in the 8-bit screen color map
//

void SetWhiteTransparent8(BBitmap *bitmap)
{
	//	Make sure we are using an 8-bit bitmap
	ASSERT(bitmap->ColorSpace() == B_CMAP8);
	
	int32 bitsLength = bitmap->BitsLength();
	for ( int32 index = 0; index < bitsLength; index++)
	{
		uchar color = ((uchar *)bitmap->Bits())[index];
		if (color == 63)	
		{	
			((uchar *)bitmap->Bits())[index] = B_TRANSPARENT_8_BIT;
		}
	}
}

#pragma mark -
#pragma mark === File Utilities ===


//---------------------------------------------------------------------
//	IsSupportedType
//---------------------------------------------------------------------
//
//	Return true if this is a file type that we can handle
//

bool IsSupportedType(BNodeInfo &nodeInfo)
{
	//	We support the following supertypes:
	//
	//	-- Image, Audio, Video
	//
			
	//if ( IsImage(nodeInfo) )
	//	return true;
	
	if ( IsAudioAiff(nodeInfo))
	{
		return true;
	}
		
	//if (IsVideo(nodeInfo))
	//	return true;
	
	// We don't support it	
	return false;
}


//---------------------------------------------------------------------
//	IsImage
//---------------------------------------------------------------------
//
//	Return true if this is an "image" file. 
//

bool IsImage(BNodeInfo &nodeInfo)
{
	char theType[B_MIME_TYPE_LENGTH];

	if (nodeInfo.GetType(theType) == B_NO_ERROR)
	{
		BMimeType mimeType(theType);
		if (mimeType.InitCheck() != B_NO_ERROR)
			return false;
		
		BMimeType superType;
		if ( mimeType.GetSupertype(&superType) == B_NO_ERROR)
		{		
			const char *superTypeStr= superType.Type();

			if ( strcmp(superTypeStr, "image") == 0)
				return true;
		}
		return false;		
	}
	
	return false;
}


//---------------------------------------------------------------------
//	IsAudio
//---------------------------------------------------------------------
//
//	Return true if this is an "audio" file. 
//

bool IsAudio(BNodeInfo &nodeInfo)
{
	char theType[B_MIME_TYPE_LENGTH];

	if (nodeInfo.GetType(theType) == B_NO_ERROR)
	{
		BMimeType mimeType(theType);
		if (mimeType.InitCheck() != B_NO_ERROR)
			return false;
		
		BMimeType superType;
		if ( mimeType.GetSupertype(&superType) == B_NO_ERROR)
		{		
			const char *superTypeStr= superType.Type();

			if ( strcmp(superTypeStr, "audio") == 0)
			{
				//	Make sure it is not a MIDI file
				if ( strcmp(mimeType.Type(), "audio/x-midi") != 0)
					return true;			
			}
			return false;
		}
		return false;		
	}
	
	return false;
}

//---------------------------------------------------------------------
//	IsAudioAiff
//---------------------------------------------------------------------
//
//	Return true if this is an "audio/x-aiff" file. 
//

bool IsAudioAiff(BNodeInfo &nodeInfo)
{
	char theType[B_MIME_TYPE_LENGTH];

	if (nodeInfo.GetType(theType) == B_NO_ERROR)
	{
		BMimeType mimeType(theType);
		if (mimeType.InitCheck() != B_NO_ERROR)
			return false;
		
		BMimeType superType;
		if ( mimeType.GetSupertype(&superType) == B_NO_ERROR)
		{		
			const char *superTypeStr= superType.Type();

			if ( strcmp(superTypeStr, "audio") == 0)
			{
				//	Make sure it is an AIFF file
				if ( strcmp( mimeType.Type(), "audio/x-aiff") == 0)
					return true;	
			}
			return false;
		}
		return false;		
	}
	
	return false;
}


//---------------------------------------------------------------------
//	IsMIDI
//---------------------------------------------------------------------
//
//	Return true if this is a "midi" file. 
//

bool IsMIDI(BNodeInfo &nodeInfo)
{
	char theType[B_MIME_TYPE_LENGTH];

	if (nodeInfo.GetType(theType) == B_NO_ERROR)
	{
		BMimeType mimeType(theType);
		if (mimeType.InitCheck() != B_NO_ERROR)
			return false;
		
		BMimeType superType;
		if ( mimeType.GetSupertype(&superType) == B_NO_ERROR)
		{		
			const char *superTypeStr= superType.Type();

			if ( strcmp(superTypeStr, "audio") == 0)
			{
				//	Make sure it is not a MIDI file
				if ( strcmp( mimeType.Type(), "audio/midi") == 0)
					return true;			
			}
			return false;
		}
		return false;		
	}
	
	return false;
}

//---------------------------------------------------------------------
//	IsVideo
//---------------------------------------------------------------------
//
//	Return true if this is an "video" file. 
//

bool IsVideo(BNodeInfo &nodeInfo)
{
	char theType[B_MIME_TYPE_LENGTH];

	if (nodeInfo.GetType(theType) == B_NO_ERROR)
	{
		BMimeType mimeType(theType);
		if (mimeType.InitCheck() != B_NO_ERROR)
			return false;
		
		BMimeType superType;
		if ( mimeType.GetSupertype(&superType) == B_NO_ERROR)
		{		
			const char *superTypeStr= superType.Type();

			if ( strcmp(superTypeStr, "video") == 0)
				return true;
		}
		return false;		
	}
	
	return false;
}


//---------------------------------------------------------------------
//	IsText
//---------------------------------------------------------------------
//
//	Return true if this is a "text" file. 
//

bool IsText(BNodeInfo &nodeInfo)
{
	char theType[B_MIME_TYPE_LENGTH];

	if (nodeInfo.GetType(theType) == B_NO_ERROR)
	{
		BMimeType mimeType(theType);
		if (mimeType.InitCheck() != B_NO_ERROR)
			return false;
		
		BMimeType superType;
		if ( mimeType.GetSupertype(&superType) == B_NO_ERROR)
		{		
			const char *superTypeStr= superType.Type();

			if ( strcmp(superTypeStr,  "text") == 0)
				return true;
		}
		return false;		
	}
	
	return false;
}


//---------------------------------------------------------------------
//	IsCueSheet
//---------------------------------------------------------------------
//
//	Return true if this is an "application/x-mediapede-cuesheet" file. 
//

bool IsCueSheet(BNodeInfo &nodeInfo)
{
	char theType[B_MIME_TYPE_LENGTH];

	if (nodeInfo.GetType(theType) == B_NO_ERROR)
	{
		BMimeType mimeType(theType);
		if (mimeType.InitCheck() != B_NO_ERROR)
			return false;
		
		BMimeType superType;
		if ( mimeType.GetSupertype(&superType) == B_NO_ERROR)
		{		
			const char *superTypeStr= superType.Type();

			if ( strcmp(superTypeStr, "application") == 0)
			{
				//	Make sure it is not a MIDI file
				if ( strcmp( mimeType.Type(), "application/x-mediapede-cuesheet") == 0)
					return true;			
			}
			return false;
		}
		return false;		
	}
	
	return false;
}



//---------------------------------------------------------------------
//	MediaDuration
//---------------------------------------------------------------------
//
//	Return durstion of file in microseconfs
//

bigtime_t MediaDuration(entry_ref *fileRef)
{
	//	Create entry and make sure file exists
	BEntry entry(fileRef, true);
	if (entry.InitCheck() != B_OK || !entry.Exists())
	{
		ERROR("MediaDuration() - Unable to create BEntry or entry_ref does not exist -\n");
		return -1;
	}

	// Resolve possible symlink...
	entry.GetRef(fileRef);
	
	// Create BFile from ref...
	BFile *file = new BFile(fileRef, B_READ_ONLY);
	if (file->InitCheck() != B_OK)
	{
		ERROR("MediaDurartion - Error creating BFile -\n");
		return -1;	
	}
		
	//	Instantiate a BMediaFile object, and make sure there was no error.
	BMediaFile *mediaFile = new BMediaFile(fileRef);
	status_t err = mediaFile->InitCheck();
	if (err) 
	{
		ERROR("MediaDuration() - Cannot contruct BMediaFile object: %s\n", strerror(err));
		return -1;
	}
		
	//	Make sure they are valid
	BMediaTrack *track	 = NULL;
	bigtime_t	duration = -1;
	
	for (int32 i = 0; i < mediaFile->CountTracks(); i++) 
	{
		//	Get track
		track = mediaFile->TrackAt(i);
		if (!track) 
		{
			ERROR("MediaDuration() - Cannot contruct BMediaTrack object -\n");
			return -1;
		}
				
		//	Get track duration
		bigtime_t tmpDuration = track->Duration();
		mediaFile->ReleaseTrack(track);
		
		//	Compare with previous track duration
		if (tmpDuration > duration)
			duration = tmpDuration;
		
	}
					
	//	Return the value
	return duration;
}

#pragma mark -
#pragma mark === Thread Utilities ===


//---------------------------------------------------------------------
//	ValidThread
//---------------------------------------------------------------------
//
//	Return true if this is a valid thread_id
//

bool ValidThread( thread_id theThread)
{
	if( theThread != B_NO_MORE_THREADS && theThread != B_NO_MEMORY)
		return true;
	else
		return false;		
}

#pragma mark -
#pragma mark === View Utilities ===

//---------------------------------------------------------------------
//	IndexForColor
//---------------------------------------------------------------------
//
//

int32 IndexForColor(rgb_color color)
{
	int32 c = (color.red << 24) + (color.green << 16) + (color.blue << 8);
	return c;
}


//---------------------------------------------------------------------
//	FontHeight
//---------------------------------------------------------------------
//
//	Returns the font height for the given view
//

float FontHeight(BView* target, bool full)
{
	font_height finfo;		
	target->GetFontHeight(&finfo);
	float h = finfo.ascent + finfo.descent;

	if (full)
		h += finfo.leading;
	
	return h;
}


//---------------------------------------------------------------------
//	LimitToNumbers
//---------------------------------------------------------------------
//
//

void LimitToNumbers(BTextControl* tc)
{
	// Disallow all characters except numbers
	BTextView *theView = tc->TextView();
	
	if (theView)
	{
		// All characters up to "1"
		int32 index;
		
		for (index = 0;	index < 48; index++)
		{
			theView->DisallowChar(index);
		}
		
		// Skip over numbers
		for (index = 58; index < 2000; index++)
		{
			theView->DisallowChar(index);
		}
	}
}	

//---------------------------------------------------------------------
//	LocateDeskbar
//---------------------------------------------------------------------
//
//	Return struct containing location of deskbar
//

db_loc LocateDeskbar(BRect *deskbarFrame, BRect *screenBounds)
{
	// Get screen dimensions
	BScreen theScreen(B_MAIN_SCREEN_ID);
	*screenBounds = theScreen.Frame();	

	get_deskbar_frame(deskbarFrame);
	
	if (deskbarFrame->right < screenBounds->Width()
		&& deskbarFrame->left == 0
		&& deskbarFrame->top == 0) {
		return kTopLeft;
	} else if (deskbarFrame->left > 0
		&& deskbarFrame->right == screenBounds->Width()
		&& deskbarFrame->top == 0) {
		return kTopRight;
	} else if (deskbarFrame->top == 0
		&& deskbarFrame->left == 0
		&& deskbarFrame->right == screenBounds->Width()) {
		return kTop;
	} else if (deskbarFrame->bottom == screenBounds->Height()
		&& deskbarFrame->left == 0
		&& deskbarFrame->right < screenBounds->Width()){
		return kBottomLeft;
	} else if (deskbarFrame->bottom == screenBounds->Height()
		&& deskbarFrame->right == screenBounds->Width()
		&& deskbarFrame->left > 0) {
		return kBottomRight;
	} else if (deskbarFrame->bottom == screenBounds->Height()) {
		return kBottom;
	}

	ASSERT(false);
	return kTopRight;
}


//---------------------------------------------------------------------
//	ScreenDimensions
//---------------------------------------------------------------------
//
//

void ScreenDimensions(float *w, float *h)
{
	BRect screenFrame = (BScreen(B_MAIN_SCREEN_ID).Frame());
	*w = screenFrame.Width();
	*h = screenFrame.Height();
}


#pragma mark -
#pragma mark === Sound Utilities ===

//---------------------------------------------------------------------
//	PlayStatusSound
//---------------------------------------------------------------------
//
//


void PlayStatusSound(char *soundName)
{
	status_t err;
	
	//	Get sounds directory
	app_info appInfo;
	be_app->GetAppInfo(&appInfo); 	
	BEntry entry(&appInfo.ref);
	if (entry.InitCheck() != B_OK)
		return;
		
	BDirectory soundsDir;
	err = entry.GetParent(&soundsDir);
	if (err != B_OK)
		return;
		
	err = soundsDir.SetTo(&soundsDir, "Sounds");
	if (err != B_OK)
		return;
	
	//	Create GameSound
	BPath path(&soundsDir, NULL, false);
	if (path.InitCheck() != B_OK)
		return;
	
	char filePath[B_PATH_NAME_LENGTH];
	sprintf(filePath, "%s/%s", path.Path(), soundName);	
	err = path.SetTo(filePath);
	if (err != B_OK)
		return;

	err = entry.SetTo(path.Path());
	if (err != B_OK)
		return;

	
	entry_ref ref;
	err = entry.GetRef(&ref);
	if (err != B_OK)
		return;
	
	BSimpleGameSound sound(&ref);
	if (sound.InitCheck() == B_OK)
	{
		sound.StartPlaying();
			
		//	Wait for quit
		while(sound.IsPlaying())
		{
			snooze(10 * 1000);
		}
	}
}


#pragma mark -
#pragma mark === Mouse Uitlities ===

//---------------------------------------------------------------------
//	ValidDoubleClick
//---------------------------------------------------------------------
//
//	Make sure click occured in valid zone
//

bool ValidDoubleClick(BPoint where, BPoint zone)
{	
	bool retVal = false;
	
	//	X
	if (where.x > zone.x)
	{
		if (where.x < zone.x + 5)
		retVal = true;
	}
	else if (where.x <= zone.x)
	{
		if (where.x > zone.x - 5)
		retVal = true;
	}
	
	//	Y
	if (retVal)
	{
		if (where.y > zone.y)
		{
			if (where.y < zone.y + 5)
				return true;
			else
				return false;
		}
		else if (where.y <= zone.y)
		{
			if (where.y > zone.y - 5)
				return true;
			else
				return false;
		}
	}
	
	return retVal;
}
