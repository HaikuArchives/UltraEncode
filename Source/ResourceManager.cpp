//---------------------------------------------------------------------
//
//	File:	ResourceManager.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	03.25.98
//
//	Desc:	Routines for resource management
//
//	Copyright ©1998 mediapede Software
//
//---------------------------------------------------------------------

// Includes
#include <Debug.h>
#include <Application.h>
#include <Resources.h>
#include <Bitmap.h>
#include <TranslationUtils.h>

#include <Directory.h>
#include <Path.h>
#include <SimpleGameSound.h>

#include <string.h>

#include "DebugBuild.h"

#include "ResourceManager.h"

#pragma mark -
#pragma mark === Resource Handling ===


//---------------------------------------------------------------------
//	GetCicnFromResource
//---------------------------------------------------------------------
//
//	Find and return cicn bitmap by string
//

BBitmap *GetCicnFromResource(const char *theResource)
{	
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);	
	if (file.InitCheck())
		return NULL;
	
	size_t 		size;
	cicn 		*icon;
	
	BResources res; 
	status_t err; 
	if ( (err = res.SetTo(&file)) != B_NO_ERROR ) 
		return NULL;
		
	icon = (cicn *)res.LoadResource('cicn', theResource, &size);
	if (!icon)			
		return NULL;
			
	// 	Swap bytes if needed.  We do this because the resources are currently
	// 	built on Macintosh BeOS
	if (B_HOST_IS_LENDIAN)
	{
		status_t retVal;
		retVal = swap_data(B_INT16_TYPE, &icon->width, sizeof(int16), B_SWAP_BENDIAN_TO_HOST);		
		retVal = swap_data(B_INT16_TYPE, &icon->height, sizeof(int16), B_SWAP_BENDIAN_TO_HOST);
	}
	
	// Get cicn bounding rect
	BRect bounds(0, 0, icon->width-1, icon->height-1);
		
	// Load bitmap
	BBitmap *bitmap = new BBitmap(bounds, B_COLOR_8_BIT);
	ASSERT(bitmap);
	bitmap->SetBits(&icon->data, size - sizeof(int16)*2, 0, B_COLOR_8_BIT);
	
	return (bitmap);	
}

//---------------------------------------------------------------------
//	GetIcon8FromResource
//---------------------------------------------------------------------
//
//	Find and return 8x8 icon bitmap by string
//

BBitmap *GetIcon8FromResource(const char *theResource)
{
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);	
	if (file.InitCheck())
		return NULL;
	
	size_t 		size;
	BBitmap 	*data;
	
	BResources res(&file);
	data = (BBitmap *)res.LoadResource('bits', theResource, &size);
	if (!data)			
		return NULL;
	
	// Load icon
	BRect bounds;
	bounds.Set(0, 0, 7, 7);
	BBitmap *bitmap = new BBitmap(bounds, B_COLOR_8_BIT);
	ASSERT(bitmap);
	bitmap->SetBits(data, size, 0, B_COLOR_8_BIT);
	
	return (bitmap);	
}

//---------------------------------------------------------------------
//	GetIcon16FromResource
//---------------------------------------------------------------------
//
//	Find and return 16x16 icon bitmap by string
//

BBitmap *GetIcon16FromResource(const char *theResource)
{	
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);
	if (file.InitCheck())
		return NULL;
	
	size_t 		size;
	BBitmap 	*data;
	
	BResources res(&file);
	data = (BBitmap *)res.LoadResource('bits', theResource, &size);
	if (!data)			
		return NULL;
	
	// Load icon
	BRect bounds;
	bounds.Set(0, 0, 15, 15);
	BBitmap *bitmap = new BBitmap(bounds, B_COLOR_8_BIT);
	ASSERT(bitmap);
	bitmap->SetBits(data, size, 0, B_COLOR_8_BIT);
	
	return (bitmap);	
}

//---------------------------------------------------------------------
//	GetIcon32FromResource
//---------------------------------------------------------------------
//
//	Find and return 32x32 icon by string
//

BBitmap *GetIcon32FromResource(const char *theResource)
{	
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);
	if (file.InitCheck())
		return NULL;
	
	size_t 		size;
	BBitmap 	*data;
	
	BResources res(&file);
	data = (BBitmap *)res.LoadResource('bits', theResource, &size);
	if (!data)			
		return NULL;
	
	// Load icon
	BRect bounds;
	bounds.Set(0,0,31,31);
	BBitmap *bitmap = new BBitmap(bounds, B_COLOR_8_BIT);
	ASSERT(bitmap);
	bitmap->SetBits(data, size, 0, B_COLOR_8_BIT);
	
	return (bitmap);	
}

//---------------------------------------------------------------------
//	GetMICNFromResource
//---------------------------------------------------------------------
//
//	Find and return 16x16 Be Small Icon 'MICN' by string
//

BBitmap *GetMICNFromResource(const char *theResource)
{
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);
	if (file.InitCheck())
		return NULL;
	
	size_t 		size;
	BBitmap 	*data;
	
	BResources res(&file);
	data = (BBitmap *)res.LoadResource('MICN', theResource, &size);
	if (!data)			
		return NULL;
	
	// Load icon
	BRect bounds;
	bounds.Set(0,0,15,15);
	BBitmap *bitmap = new BBitmap(bounds, B_COLOR_8_BIT);
	ASSERT(bitmap);
	bitmap->SetBits(data, size, 0, B_COLOR_8_BIT);
	
	return (bitmap);	
}


//---------------------------------------------------------------------
//	GetICONFromResource
//---------------------------------------------------------------------
//
//	Find and return 32x32 Be Icon 'ICON' by string
//

BBitmap *GetICONFromResource(const char *theResource)
{
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);
	if (file.InitCheck())
		return NULL;
	
	size_t 		size;
	BBitmap 	*data;
	
	BResources res(&file);
	data = (BBitmap *)res.LoadResource('ICON', theResource, &size);
	if (!data)			
		return NULL;
	
	// Load icon
	BRect bounds;
	bounds.Set(0,0,31,31);
	BBitmap *bitmap = new BBitmap(bounds, B_COLOR_8_BIT);
	ASSERT(bitmap);
	bitmap->SetBits(data, size, 0, B_COLOR_8_BIT);
	
	return (bitmap);	
}


//---------------------------------------------------------------------
//	GetTICNFromResource
//---------------------------------------------------------------------
//
//	Find and return 'ticn' by string
//
//	A "ticn" is just an 8-bit targa file
//

BBitmap *GetTICNFromResource(const char *theResource)
{
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);
	if (file.InitCheck())
		return NULL;
	
	//	Load resource data
	size_t 		size;
		
	BResources res(&file);
	void *data = (void *)res.LoadResource('ticn', theResource, &size);
	if (!data)			
		return NULL;
	
	//	Copy data into BMemoryIO object
	BMemoryIO ioData(data, size);
	
	//	Use BTranslationUtils to load data
	BBitmap *bitmap = BTranslationUtils::GetBitmap(&ioData);
	ASSERT(bitmap);
	
	//	Convert to 8-bit
	if (bitmap->ColorSpace() != B_CMAP8)
	{
		//PROGRESS("GetTICNFromResource() - Converting to 8-bit -\n");
		
		BBitmap *convBits = new BBitmap( bitmap->Bounds(), B_CMAP8, true);	
		BView 	*convView = new BView(bitmap->Bounds(), "convView", B_FOLLOW_NONE, 0);
	
		if (convBits->Lock())
		{
			convBits->AddChild(convView);			
			convView->DrawBitmap(bitmap);
			convBits->RemoveChild(convView);
			delete convView;
			convBits->Unlock();
			
			delete bitmap;
			bitmap = convBits;
		}
	}
	
	return (bitmap);	
}


//---------------------------------------------------------------------
//	GetWindowFromResource
//---------------------------------------------------------------------
//
//	Load an archived window object from the application resource and
//	return the archived BMessage


BMessage *GetWindowFromResource(const char *theResource)
{
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);
	if (file.InitCheck())
	{
		ERROR("GetWindowFromResource() -InitCheck failure-\n");
		return NULL;
	}
	
	size_t 		size;
	BMessage 	*message = new BMessage();
	void 		*windowData = NULL;
	
	BResources res(&file);
	if( res.HasResource('ARCV', theResource ) )
	{
		windowData = (BMessage *)res.LoadResource('ARCV', theResource, &size);
		if (!windowData)
		{
			ERROR("GetWindowFromResource() -Bad windowData-\n");
			return NULL;
		}
	
		message->Unflatten((const char *)windowData);
		return (message);
	}
	else
	{
		// We have an error condition
		ERROR("GetWindowFromResource() -NULL Message-\n");
		return (NULL);	
	}
}


	
//---------------------------------------------------------------------
//	GetMenuFromResource
//---------------------------------------------------------------------
//
//	Load an archived menu object from the application resource and
//	return the archived BMessage


BMessage *GetMenuFromResource(const char *theResource)
{
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);
	if (file.InitCheck())
		return NULL;
	
	size_t 		size;
	BMessage 	*message = new BMessage();
	void 		*windowData = NULL;
	
	BResources res(&file);
	
	if( res.HasResource('ARCV', theResource ) )
	{
		windowData = (BMessage *)res.LoadResource('ARCV', theResource, &size);
		if (!windowData)			
			return NULL;
	
		message->Unflatten((const char *)windowData);
			
		return (message);
	}
	else
	{
		// We have an error condition
		return (NULL);	
	}
}

/*
//---------------------------------------------------------------------
//	GetCursorFromResource
//---------------------------------------------------------------------
//
//	Load named cursor from resource
//

bool GetCursorFromResource(const char *theResource, Cursor *theCursor)
{
	// Get application info
	app_info info;
	
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, O_RDONLY);
	if (file.InitCheck())
		return false;
	
	size_t 	size;
	Cursor *data;
	
	BResources res(&file);
	data = (Cursor *)res.LoadResource('crsr', theResource, &size);
	if (!data)			
		return false;

	memcpy(theCursor, data, sizeof(Cursor));
		
	return true;
}


//---------------------------------------------------------------------
//	GetAppIcons
//---------------------------------------------------------------------
//
//	Return pointer to preloaded app icons
//

TMuseumIcons *GetAppIcons()
{
	return static_cast<MuseumApp *>(be_app)->m_MuseumIcons;
}
*/

