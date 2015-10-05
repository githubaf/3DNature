// Notify.h
// Notification-control object and support objects
// Created from the warped imagination of Chris 'Xenon' Hanson
// on 2/10/96
// Copyright 1996

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class NotifyClient;
class NotifyEx;
class NotifyEvent;

class WCSModule;

#ifndef WCS_NOTIFY_H
#define WCS_NOTIFY_H

#include "Types.h"

#define WCS_NOTIFY_MAX_CLIENTS 30
#define WCS_NOTIFY_DELAY_MAX_CHANGE 10

#define NOTIFYCLASSPART(a)			((unsigned char)((((unsigned long)a) & 0xff000000) >> 24))
#define NOTIFYSUBCLASSPART(a)		((unsigned char)((((unsigned long)a) & 0x00ff0000) >> 16))
#define NOTIFYITEMPART(a)			((unsigned char)((((unsigned long)a) & 0x0000ff00) >>  8))
#define NOTIFYCOMPONENTPART(a)		((unsigned char)((((unsigned long)a) & 0x000000ff)))

/* These are the defined notify classes - they are just here for reference
// from ParamDispatch.h
// these will be obsolete in v5
#define WCS_PARAMCLASS_ALL				~0
#define WCS_PARAMCLASS_MOTION			1
#define WCS_PARAMCLASS_COLOR			2
#define WCS_PARAMCLASS_ECO				3
#define WCS_PARAMCLASS_CLOUD			4
#define WCS_PARAMCLASS_WAVE				5
#define WCS_PARAMCLASS_SETTING			10
#define WCS_PARAMCLASS_FRAME			20

// Effect types hold range 0-99

// from ProjectDispatch.h
#define WCS_PROJECTCLASS_ALL			~0
#define WCS_PROJECTCLASS_PATHS			101
#define WCS_PROJECTCLASS_DATABASE		102
#define WCS_PROJECTCLASS_PARAMETERS		103
#define WCS_PROJECTCLASS_PLUGINS		104
#define WCS_PROJECTCLASS_WIZARD			105
#define WCS_PROJECTCLASS_GUICONFIG		106
#define WCS_PROJECTCLASS_PREFS			107
#define WCS_PROJECTCLASS_USERDATA		108
#define WCS_PROJECTCLASS_MODULE			109

// from EffectsLib.h
#define WCS_NOTIFYCLASS_EFFECTS			130

// from Raster.h
#define WCS_NOTIFYCLASS_IMAGES			131

// from Database.h
#define WCS_NOTIFYCLASS_DBASE			132

// from Interactive.h
#define WCS_INTERCLASS_CAMVIEW			220
#define WCS_INTERCLASS_MAPVIEW			221
#define WCS_INTERCLASS_VECTOR			223
#define WCS_INTERCLASS_MISC				224
#define WCS_INTERCLASS_TIME				226

#define WCS_NOTIFYCLASS_DIAGNOSTICDATA	240

#define WCS_NOTIFYCLASS_VIEWGUI			241

// space for non-overlapping subclass values

// from Toolbar.h
#define WCS_TOOLBARCLASS_MODULES		250
//#define WCS_TOOLBARCLASS_MENUACTIONS	251 // now retired (did it ever do much?)

// subclasses

// from Effectslib.h
WCS_EFFECTSSUBCLASS_GENERIC				0
...										...
WCS_EFFECTSSUBCLASS_RENDEROPT			40
WCS_SUBCLASS_GEOREGISTER				140
WCS_SUBCLASS_MATERIALSTRATA				141
WCS_SUBCLASS_WAVESOURCE					142
WCS_SUBCLASS_ATMOCOMPONENT				143
WCS_SUBCLASS_INCLUDEEXCLUDE				144
WCS_SUBCLASS_POSTPROCEVENT				145

// from Ecotype.h
WCS_SUBCLASS_ECOTYPE					160
WCS_SUBCLASS_FOLIAGEGRP					161
WCS_SUBCLASS_FOLIAGE					162

// from GraphData.h
WCS_SUBCLASS_ANIMDOUBLETIME				165
WCS_SUBCLASS_ANIMDOUBLEDISTANCE			166
WCS_SUBCLASS_ANIMDOUBLEENVELOPE,		167
WCS_SUBCLASS_ANIMDOUBLEPROFILE,			168
WCS_SUBCLASS_ANIMDOUBLECLOUDPROF		169
WCS_SUBCLASS_ANIMDOUBLESECTION,			170
WCS_SUBCLASS_ANIMCOLORTIME				171
WCS_SUBCLASS_ANIMCOLORGRADIENT			172
WCS_SUBCLASS_ANIMMATERIALGRADIENT		173
WCS_SUBCLASS_COLORTEXTURE				174

// from Texture.h
WCS_SUBCLASS_ROOTTEXTURE				180
WCS_SUBCLASS_TEXTURE					181

// from Raster.h
WCS_SUBCLASS_RASTER						190
WCS_IMAGESSUBCLASS_GENERIC				191

// from Database.h
WCS_SUBCLASS_JOE						200
WCS_SUBCLASS_VECTOR						201
WCS_SUBCLASS_DEM						202
WCS_SUBCLASS_CONTROLPT					203
WCS_NOTIFYDBASE_NEW						204
WCS_NOTIFYDBASE_PRELOAD					205
WCS_NOTIFYDBASE_LOAD					206
WCS_NOTIFYDBASE_ADDOBJ					207
WCS_NOTIFYDBASE_PRECHANGEOBJ			208
WCS_NOTIFYDBASE_CHANGINGOBJ				209
WCS_NOTIFYDBASE_CHANGEOBJ				210
WCS_NOTIFYDBASE_PRECHANGEACTIVE			211
WCS_NOTIFYDBASE_CHANGEACTIVE			212
WCS_NOTIFYDBASE_DELOBJ					213

// from Layers.h
WCS_SUBCLASS_LAYER						215
WCS_SUBCLASS_LAYERATTRIB				216

// from Project.h
WCS_PATHS_DIRLIST						220
WCS_PATHS_DEFAULTDIR					221
WCS_SUBCLASS_MODULE_OPEN				222
WCS_SUBCLASS_MODULE_CLOSE				223
WCS_SUBCLASS_PROJPREFS_UNITS			224
WCS_SUBCLASS_PROJPREFS_CONFIG			225

*/

// for consistent usage in GlobalApp->AppEx
// classes correspond with effects type class, image lib, effects lib & database, see listing above
// class will indicate the root of the RasterAnimHost chain whenever dealing with GlobalApp->AppEx
#define WCS_NOTIFYCLASS_ALL				0xff
// Effects								0
// Project								100
// EffectsLib							130
// ImageLib								131
// Database								132
// Misc Effect subclasses				140
// Foliage								160
// AnimCritters and their derivatives	165
// Textures								180
// Rasters								190
// Joes									200
#define WCS_NOTIFYCLASS_VIEWGUI			241
// Toolbar								250
#define WCS_NOTIFYCLASS_DELAYEDEDIT		252
#define WCS_NOTIFYCLASS_FREEZE			254

// subclasses correspond with object type which has actually changed
// class and subclass will be the same for an action to the root of the RasterAnimHost chain
// subclasses are defined in the header that defines the object class:
//  effectslib.h, graphdata.h, raster.h, database.h, Ecotype.h, texture.h
//  see listing above
// do not allow any overlap with pre-v5 class values except for param classes which are now obsolete
#define WCS_NOTIFYSUBCLASS_ALL			0xff
// Effects								0
// Misc Effect subclasses				140
// Foliage								160
// AnimCritters and their derivatives	165
// Textures								180
// Rasters								190
// Joes									200

#define WCS_NOTIFYSUBCLASS_FREEZE		250
#define WCS_NOTIFYSUBCLASS_THAW			251

#define WCS_NOTIFYSUBCLASS_NEWTERRAGEN	252

#define WCS_NOTIFYSUBCLASS_REVERSE		253 // used for DelayedEdit to indicate Edit Previous instead of Edit Next

// item is the object number, if it has one, within its RasterAnimHost parent, 0xff otherwise
#define WCS_NOTIFYITEM_ALL				0xff

// EffectsLib.h defines these items for use with effects and GlobalApp->AppEffects as the NotifyEx
//WCS_EFFECTSGENERIC_NAME				50
//WCS_EFFECTSGENERIC_ENABLED			51
//WCS_EFFECTSGENERIC_JOESADDED			52
//WCS_EFFECTSGENERIC_EFFECTADDED		53
//WCS_EFFECTSGENERIC_PROFILEADDED		54
//WCS_EFFECTSGENERIC_GROUPENABLED		58

// Database.h defines these items for use with joes and GlobalApp->AppDB as the NotifyEx
//WCS_NOTIFYDBASECHANGE_NAME			0
//WCS_NOTIFYDBASECHANGE_DRAWATTRIB		1
//WCS_NOTIFYDBASECHANGE_FLAGS			2
//WCS_NOTIFYDBASECHANGE_POINTS			3

// Raster.h defines these items for use with AppImages as the NotifyEx
//WCS_IMAGESGENERIC_NAME				1
//WCS_IMAGESGENERIC_ENABLED,			2
//WCS_IMAGESGENERIC_IMAGEADDED,			3
//WCS_IMAGESGENERIC_ATTRIBUTEADDED,		4
//WCS_IMAGESGENERIC_ACTIVECHANGED		5

// component is an action type, it should be a unique value that lets client
//  determine what general class action occurred without even knowing what type of 
//  object it happened to
#define WCS_NOTIFYCOMP_ALL				0xff

// rgb color values are often used as components 0, 1, 2 in pre v5 so avoid them here

// generic component values
#define WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED		10
#define WCS_NOTIFYCOMP_OBJECT_REMOVEPENDING		11
#define WCS_NOTIFYCOMP_OBJECT_ABOUTTOCHANGE		12
#define WCS_NOTIFYCOMP_OBJECT_CHANGED			13
#define WCS_NOTIFYCOMP_OBJECT_VALUECHANGED		14
#define WCS_NOTIFYCOMP_OBJECT_NAMECHANGED		15
#define WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED	16

// attributes are things that are refferred to by reference and may exist
//  independent of the containing object, such as an Ecosystem referred to by an Environment
#define WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED	20
#define WCS_NOTIFYCOMP_ATTRIBUTE_REMOVEPENDING	21
#define WCS_NOTIFYCOMP_ATTRIBUTE_ABOUTTOCHANGE	22
#define WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED		23
#define WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED	24

// GraphData.h defines these components
//WCS_NOTIFYCOMP_ANIM_VALUECHANGED				100
//WCS_NOTIFYCOMP_ANIM_POSITIONCHANGED			101
//WCS_NOTIFYCOMP_ANIM_SELECTIONCHANGED			102
//WCS_NOTIFYCOMP_ANIM_NODEADDED					103
//WCS_NOTIFYCOMP_ANIM_NODEREMOVED				104
//WCS_NOTIFYCOMP_ANIM_ABOUTTOCHANGE				105

// From Raster.h
//WCS_IMAGESGENERIC_NAME						1
//WCS_IMAGESGENERIC_ENABLED						2
//WCS_IMAGESGENERIC_IMAGEADDED					3
//WCS_IMAGESGENERIC_ATTRIBUTEADDED				4
//WCS_IMAGESGENERIC_ACTIVECHANGED				5

// Database.h defines these components in overlapping ranges 
//  dependent on item for correct interpretation
//WCS_NOTIFYDBASECHANGE_DRAWATTRIB_RGB			0
//WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PEN			1
//WCS_NOTIFYDBASECHANGE_DRAWATTRIB_WIDTH		2
//WCS_NOTIFYDBASECHANGE_DRAWATTRIB_LSTYLE		3
//WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PSTYLE		4
//WCS_NOTIFYDBASECHANGE_INTEREDIT_ERASE			5
//WCS_NOTIFYDBASECHANGE_INTEREDIT_DRAW			6

//WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM		0
//WCS_NOTIFYDBASECHANGE_POINTS_CONFORM			1

//WCS_NOTIFYDBASECHANGE_FLAGS_SELSTATECLEAR		1





// Not tip for dealing with NotifyTag arrays:
// Don't put lots of tags into them. Use the all-bits-on wildcard
// tag for the least-significant class. Also, put the most-likely
// events first, so that the code can match and bail quicker.


// A Notify Client entry
class NotifyClient
	{
	friend class NotifyEx;
	private:
		// Interested points to an array of longwords, each specifying
		// the types of events the Client is interested in. The array
		// can be any length, and must be terminated with a NULL longword.
		NotifyTag *Interested;
		// ClientModule is the WCSModule that will receive the Notify events
		WCSModule *ClientModule;
	public:
		NotifyClient() {Interested = NULL; ClientModule = NULL;};
	}; // NotifyClient

// The notify Exchange
class NotifyEx
	{
	private:
		NotifyClient NotifyList[WCS_NOTIFY_MAX_CLIENTS];
		NotifyTag DelayedList[WCS_NOTIFY_DELAY_MAX_CHANGE];
		void *DelayedData;

	public:
		NotifyEx();

		// returns 0 if it can't register
		int RegisterClient(WCSModule *Client, NotifyTag *InterestedList);

		void RemoveClient(WCSModule *Client);

		// returns the number of modules that it notified. Handy.
		int GenerateNotify(NotifyTag *ChangedList, void *NotifyData = NULL);

		// Cannot know the number of modules notified, just returns true if
		// successful. FALSE(0) is a serious error!
		// GenerateDelayedNotify is also limited in the size of the
		// ChangedList it can handle, since it must make a temporary copy.
		int GenerateDelayedNotify(NotifyTag *ChangedList, void *NotifyData = NULL);

		// ReGenerateDelayedNotify gets called from the Application object
		// to finalize the actual invocation of the notify.
		void ReGenerateDelayedNotify(void) {(void)GenerateNotify(DelayedList, DelayedData); DelayedList[0] = NULL;};

		NotifyTag MatchNotifyClass(NotifyTag *Match, NotifyTag *Events, int Single);
		NotifyTag MatchNotifyComponent(NotifyTag *Match, NotifyTag *Events, int Single);
	}; // NotifyEx

class NotifyEvent
	{
	public:
		NotifyEx *NotifyOrigin;
		NotifyTag *ChangeList;
		void *NotifyData;

	}; // NotifyEvent

class SetCritter 
	{
	public:
		virtual void SetParam(int Notify, ...) = 0;
		virtual void GetParam(void *Value, ...) = 0;
	};

#endif // WCS_NOTIFY_H
