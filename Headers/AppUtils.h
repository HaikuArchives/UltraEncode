//---------------------------------------------------------------------
//
//	File:	AppUtils.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	01.26.98
//
//	Desc:	Misc utility routines
//
//	Copyright ©1998 mediapede software
//
//---------------------------------------------------------------------

#ifndef __APPUTILS_H__
#define __APPUTILS_H__

#include <Bitmap.h>

//	Enums
typedef enum
{
	kTopLeft,
	kTopRight,
	kTop,
	kBottomLeft,
	kBottomRight,
	kBottom
} db_loc;

//
// Function protoypes
//

// Keyboard Utilities
bool 	IsKeyDown( char theKey);
bool 	IsShiftKeyDown();
bool 	IsCommandKeyDown();
bool 	IsOptionKeyDown();
bool 	IsControlKeyDown();

void 	AdjustScrollBar( BScrollBar *scrollBar, float page, float pageStep, float total, float start);


// Window Utilities
BList *FindWindow(const char *title);
void 	CenterWindow(BWindow *theWindow);

// Unit Utilities
bool DiffRect( BRect *rectOne, BRect *rectTwo, BRect *dstRect);

// Bitmap routines
// void FillBitmap(BBitmap * map, uint32 color);
void FillBitmap(BBitmap *bitmap, rgb_color color);
BBitmap *MakeTransparentBitmap(BBitmap *srcBitmap, uchar transparency);
BBitmap *CloneBitmap(const BBitmap &input);
void SetWhiteTransparent8(BBitmap *bitmap);

// File Utilities
bool IsSupportedType(BNodeInfo &nodeInfo);
bool IsImage(BNodeInfo &nodeInfo);
bool IsAudio(BNodeInfo &nodeInfo);
bool IsAudioAiff(BNodeInfo &nodeInfo);
bool IsMIDI(BNodeInfo &nodeInfo);
bool IsVideo(BNodeInfo &nodeInfo);
bool IsText(BNodeInfo &nodeInfo);
bool IsCueSheet(BNodeInfo &nodeInfo);
bigtime_t MediaDuration(entry_ref *fileRef);

// Thread Utilities
bool ValidThread(thread_id theThread);


//	View Utilities
int32 IndexForColor(rgb_color color);
float FontHeight(BView* target, bool full);
void LimitToNumbers(BTextControl* tc);
db_loc LocateDeskbar(BRect* deskbarFrame, BRect* screenBounds);
void ScreenDimensions(float *w, float *h);

void PlayStatusSound(char *soundName);

bool ValidDoubleClick(BPoint where, BPoint zone);

#endif
