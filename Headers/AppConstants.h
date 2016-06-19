//-------------------------------------------------------------------
//
//	File:	AppConstants.h
//
//	Author:	Gene Z. Ragan
//
//	Date:	01.26.98
//
//-------------------------------------------------------------------

#ifndef __APPCONSTANTS_H__
#define __APPCONSTANTS_H__

#include <Application.h>

// Colors
const rgb_color kBlack 			= {0, 	0, 	  0,  255};
const rgb_color kWhite 			= {255, 255, 255, 255};
const rgb_color kPaleGrey 		= {240, 240, 240, 255};
const rgb_color kLightGrey 		= {225, 225, 225, 255};
const rgb_color kSteelGrey 		= {200, 200, 200, 255};
const rgb_color kMediumGrey 	= {128, 128, 128, 255};
const rgb_color kGrey 			= {216, 216, 216, 255};
const rgb_color kDarkGrey 		= {64, 64, 64, 255};

const rgb_color kLightMetallicBlue 	= {143,	166, 240, 255};
const rgb_color kMedMetallicBlue 	= {75, 	96,  154, 255};
const rgb_color kDarkMetallicBlue 	= {78, 	89,  126, 255};

const rgb_color kSteelBlue 			= {205, 205, 255, 255};
const rgb_color kMediumSteelBlue 	= {153, 153, 255, 255};
const rgb_color kDarkSteelBlue 		= {100, 100, 255, 255};
const rgb_color kSIGrey 			= {171, 167, 166, 255};
const rgb_color kBlueGrey 			= {160, 160, 160, 255};
const rgb_color kHeaderGrey 		= {138, 134, 131, 255};

const rgb_color kAdobeBrown 		= {152, 102, 51,  255};
const rgb_color kGreyViolet 		= {100, 100, 109, 255};

const rgb_color kLightKhaki 		= {209, 209, 159, 255};
const rgb_color kKhaki 				= {144, 144, 105, 255};
const rgb_color kDarkKhaki 			= {91,  91,  41, 255};

const rgb_color kLightTaupe 		= {175, 194, 175, 255};
const rgb_color kTaupe 				= {100, 119, 100, 255};
const rgb_color kDarkTaupe 			= {28,  47,  28,  255};

const rgb_color kRed 				= {255,	0, 	 0,   255};
const rgb_color kGreen 				= {0, 	255, 0,   255};
const rgb_color kBlue 				= {0, 	0, 	 255, 255};

const rgb_color kBeHighlight 		= {255, 255, 255, 255};
const rgb_color kBeGrey 			= {216, 216, 216, 255};
const rgb_color kBeDarkGrey 		= {200, 200, 200, 255};
const rgb_color kBeLightShadow 		= {194, 194, 194, 255};  // was 184
const rgb_color kBeLightBevel 		= {184, 184, 184, 255};
const rgb_color kBeShadow 			= {152, 152, 152, 255};
const rgb_color kBeFocusBlue 		= {0, 	 0,  229, 255};
const rgb_color kBeTitleBarYellow 	= {255, 203, 0,	  255};

const rgb_color kTextHilite 	= {188, 183, 182, 255}; 
const rgb_color kTextShadow 	= {100,  94,  90, 255}; 


// Generic Dialog Messages
#define OK_MSG					'ok__'
#define CANCEL_MSG				'cncl'
#define APPLY_MSG				'aply'


#endif
