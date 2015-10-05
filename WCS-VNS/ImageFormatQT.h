// ImageFormatQT.h
// Built from bits of ImageFormat and Apple's CreateMovie and QTVideo

#ifndef _MAC
#include <ConditionalMacros.h>
#endif // !_MAC

#ifdef _WIN32
	//#include <Win32Headers.mch>
	//#define TARGET_OS_WIN32			1
	#ifndef _MAC
	#include <QTML.h>
	#endif // !_MAC
#else
	#ifndef _MAC
	#include <ConditionalMacros.h>
	#endif // !_MAC
#endif


#ifdef _MAC
#include <macname1.h>
#include "Components.h"
//#include "MacTypes.h"
//#include "MacMemory.h"
//#include "MacErrors.h"
#include "Fonts.h"
#include "QuickDraw.h"
#include "Resources.h"
#include "Gestalt.h"
#include "FixMath.h"
#include "Sound.h"
#include "string.h"
#include "Movies.h"
//#include "ImageCompression.h"
//#include "Script.h"
//#include "TextUtils.h"
//#include "Processes.h"
//#include <NumberFormatting.h>
#include <QuickTimeComponents.h>

#include <macname2.h>
#else // !_MAC
#include "MacTypes.h"
#include "MacMemory.h"
#include "MacErrors.h"
#include "Fonts.h"
#include "QuickDraw.h"
#include "Resources.h"
#include "Gestalt.h"
#include "FixMath.h"
#include "Sound.h"
#include "string.h"
#include "Movies.h"
#include "ImageCompression.h"
#include "Script.h"
#include "TextUtils.h"
#include "Processes.h"
#include <NumberFormatting.h>
#include <QuickTimeComponents.h>
#endif // !_MAC

