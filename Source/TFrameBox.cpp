//-------------------------------------------------------------------
//
//	File:	TFrameBox.cpp
//
//	Author:	Gene Z. Ragan
//
//	Date:	07.20.99
//
//	Desc:	Custom BBox
//
//	Copyright ©1999 mediapede Software
//
//---------------------------------------------------------------------


// Includes


#include <app/Application.h>
#include <support/Debug.h>

#include "DebugBuild.h"
#include "AppConstants.h"
#include "AppUtils.h"
#include "ResourceManager.h"

#include "TInfoDialog.h"

#include "TFrameBox.h"


//-------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------
//
//

TFrameBox::TFrameBox(BRect rect, const char *title) : 
		BBox(rect, title),
		m_BevelRect(rect)
{
}


//-------------------------------------------------------------------
//	Draw
//-------------------------------------------------------------------
//
//	Draw beveled frame
//

void TFrameBox::Draw(BRect updateRect)
{
	//	Call parent
	BBox::Draw(updateRect);
	
	//	Draw outer edge
	BPoint startPt, endPt;
	SetHighColor(kBeLightBevel);
	startPt.Set(m_BevelRect.left, m_BevelRect.bottom);
	endPt.Set(m_BevelRect.left, m_BevelRect.top);
	StrokeLine(startPt, endPt);	
	endPt.Set(m_BevelRect.right, m_BevelRect.top);
	StrokeLine(endPt);	
	SetHighColor(kWhite);
	startPt.Set(m_BevelRect.right, m_BevelRect.top+1);
	endPt.Set(m_BevelRect.right, m_BevelRect.bottom);
	StrokeLine(startPt, endPt);
	endPt.Set(m_BevelRect.left+1, m_BevelRect.bottom);
	StrokeLine(endPt);

	//	Draw inner edge
	BRect bevel = m_BevelRect;
	bevel.InsetBy(1, 1);
	SetHighColor(kBeShadow);
	StrokeRect(bevel);
}
