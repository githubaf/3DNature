/* ProjectDispatch.cpp
** World Construction Set function for changing project values.
** By Gary R. Huber, 1996.
** Copyright 1996 by Questar Productions. All Rights Reserved.
*/

#include "stdafx.h"
#include "Types.h"
#include "Notify.h"
#include "Project.h"
#include "ProjectDispatch.h"

void Project::SetParam(int Notify, ...)
{
va_list VarA;
unsigned int Change = 0, ProjClass, SubClass, Item, Component;
ULONG NotifyProjChanges[WCS_MAXPROJ_NOTIFY_CHANGES + 1];

#ifdef BUILD_VIEWTOY
return;
#else // !BUILD_VIEWTOY

va_start(VarA, Notify);

ProjClass = va_arg(VarA, int);

while (ProjClass && Change < WCS_MAXPROJ_NOTIFY_CHANGES)
	{
	if (ProjClass > 255)
		{
		NotifyProjChanges[Change] = ProjClass;
		SubClass = (ProjClass & 0xff0000) >> 16;
		Item = (ProjClass & 0xff00) >> 8;
		Component = (ProjClass & 0xff);
		ProjClass >>= 24;
		} // if event passed as longword ID
	else
		{
		SubClass = va_arg(VarA, int);
		Item = va_arg(VarA, int);
		Component = va_arg(VarA, int);
		NotifyProjChanges[Change] = (ProjClass << 24) + (SubClass << 16) + (Item << 8) + Component;
		} // else event passed as decomposed list of class, subclass, item and component
	switch (ProjClass)
		{
		case WCS_PROJECTCLASS_PARAMETERS:
			{
			switch (SubClass)
				{
				case 0:
					{
					ParamData[Item] = (short)va_arg(VarA, int);
					break;
					} // short
				} /* switch SubClass */
			break;
			} /* Parameters */
		case WCS_PROJECTCLASS_PREFS:
			{
			switch (SubClass)
				{
				case WCS_SUBCLASS_PROJPREFS_UNITS:
					{
					switch (Item)
						{
						case WCS_PROJPREFS_HORDISPLAYUNITS:
							{
							Prefs.HorDisplayUnits = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_VERTDISPLAYUNITS:
							{
							Prefs.VertDisplayUnits = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_ANGLEDISPLAYUNITS:
							{
							Prefs.AngleDisplayUnits = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_POSLONHEMISPHERE:
							{
							Prefs.PosLonHemisphere = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_LATLONSIGNDISPLAY:
							{
							Prefs.LatLonSignDisplay = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_GEOPROJDISPLAY:
							{
							Prefs.DisplayGeoUnitsProjected = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_TIMEDISPLAYUNITS:
							{
							Prefs.TimeDisplayUnits = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_SIGNIFICANTDIGITS:
							{
							Prefs.SignificantDigits = (short)va_arg(VarA, int);
							break;
							} // short
						} // switch SubClass
					break;
					} // display units
				case WCS_SUBCLASS_PROJPREFS_CONFIG:
					{
					switch (Item)
						{
						case WCS_PROJPREFS_ENABLEDFILTER:
							{
							Prefs.EnabledFilter = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_ANIMFILTER:
							{
							Prefs.AnimatedFilter = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_TASKMODE:
							{
							Prefs.TaskMode = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_GUICONFIG:
							{
							Prefs.GUIConfiguration = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_SAGEXPANDED:
							{
							Prefs.SAGExpanded = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_SHOWDBBYLAYER:
							{
							Prefs.ShowDBbyLayer = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_RECORDMODE:
							{ // RecordMode is currently unused and may be retired eventually
							Prefs.RecordMode = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_INTERACTIVEMODE:
							{
							Prefs.InteractiveMode = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_KEYGROUPMODE:
							{
							Prefs.KeyGroupMode = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_SAGBOTTOMHTPCT:
							{
							Prefs.SAGBottomHtPct = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_INTERSTYLE:
							{
							Prefs.InteractiveStyle = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_GLOBALADVANCED:
							{
							Prefs.GlobalAdvancedEnabled = (short)va_arg(VarA, int);
							break;
							} // short
						case WCS_PROJPREFS_MAXSAGDBITEMS:
							{
							Prefs.MaxSAGDBEntries = (long)va_arg(VarA, int);
							break;
							} // Long ints
						case WCS_PROJPREFS_MAXSORTEDSAGDBITEMS:
							{
							Prefs.MaxSortedSAGDBEntries = (long)va_arg(VarA, int);
							break;
							} // Long ints
						case WCS_PROJPREFS_VECPOLYLIMITMEGS:
							{
							Prefs.VecPolyMemoryLimit = (long)va_arg(VarA, int);
							break;
							} // Long ints
						case WCS_PROJPREFS_DEMLIMITMEGS:
							{
							Prefs.DEMMemoryLimit = (long)va_arg(VarA, int);
							break;
							} // Long ints
						} // switch SubClass
					break;
					} // config
				} // switch subclass
			break;
			} // prefs
		} // switch

	ProjClass = va_arg(VarA, int);
	Change ++;

	} // while

va_end(VarA);

NotifyProjChanges[Change] = 0;

if (Notify)
	{
	GenerateNotify(NotifyProjChanges);
	} // if notify clients of changes

#endif // !BUILD_VIEWTOY
} // Project::SetParam

/*===========================================================================*/

void Project::GetParam(void *Value, ...)
{
va_list VarA;
unsigned int ProjClass, SubClass, Item, Component;
short *ShtPtr = (short *)Value;
long *LngPtr = (long *)Value;

va_start(VarA, Value);

ProjClass = va_arg(VarA, int);

if (ProjClass)
	{
	if (ProjClass > 255)
		{
		SubClass = (ProjClass & 0xff0000) >> 16;
		Item = (ProjClass & 0xff00) >> 8;
		Component = (ProjClass & 0xff);
		ProjClass >>= 24;
		} // if event passed as longword ID
	else
		{
		SubClass = va_arg(VarA, int);
		Item = va_arg(VarA, int);
		Component = va_arg(VarA, int);
		} // else event passed as decomposed list of class, subclass, item and component
	switch (ProjClass)
		{
		case WCS_PROJECTCLASS_PARAMETERS:
			{
			switch (SubClass)
				{
				case 0:
					{
					*ShtPtr = (short)ParamData[Item];
					break;
					} // short
				} // switch SubClass
			break;
			} // Parameters
		case WCS_PROJECTCLASS_PREFS:
			{
			switch (SubClass)
				{
				case WCS_SUBCLASS_PROJPREFS_UNITS:
					{
					switch (Item)
						{
						case WCS_PROJPREFS_HORDISPLAYUNITS:
							{
							*ShtPtr = (short)Prefs.HorDisplayUnits;
							break;
							} // short
						case WCS_PROJPREFS_VERTDISPLAYUNITS:
							{
							*ShtPtr = (short)Prefs.VertDisplayUnits;
							break;
							} // short
						case WCS_PROJPREFS_ANGLEDISPLAYUNITS:
							{
							*ShtPtr = (short)Prefs.AngleDisplayUnits;
							break;
							} // short
						case WCS_PROJPREFS_POSLONHEMISPHERE:
							{
							*ShtPtr = (short)Prefs.PosLonHemisphere;
							break;
							} // short
						case WCS_PROJPREFS_LATLONSIGNDISPLAY:
							{
							*ShtPtr = (short)Prefs.LatLonSignDisplay;
							break;
							} // short
						case WCS_PROJPREFS_TIMEDISPLAYUNITS:
							{
							*ShtPtr = (short)Prefs.TimeDisplayUnits;
							break;
							} // short
						case WCS_PROJPREFS_SIGNIFICANTDIGITS:
							{
							*ShtPtr = (short)Prefs.SignificantDigits;
							break;
							} // short
						} // switch SubClass
					break;
					} // display units
				case WCS_SUBCLASS_PROJPREFS_CONFIG:
					{
					switch (Item)
						{
						case WCS_PROJPREFS_ENABLEDFILTER:
							{
							*ShtPtr = (short)Prefs.EnabledFilter;
							break;
							} // short
						case WCS_PROJPREFS_ANIMFILTER:
							{
							*ShtPtr = (short)Prefs.AnimatedFilter;
							break;
							} // short
						case WCS_PROJPREFS_TASKMODE:
							{
							*ShtPtr = (short)Prefs.TaskMode;
							break;
							} // short
						case WCS_PROJPREFS_GUICONFIG:
							{
							*ShtPtr = (short)Prefs.GUIConfiguration;
							break;
							} // short
						case WCS_PROJPREFS_SAGEXPANDED:
							{
							*ShtPtr = (short)Prefs.SAGExpanded;
							break;
							} // short
						case WCS_PROJPREFS_SHOWDBBYLAYER:
							{
							*ShtPtr = (short)Prefs.ShowDBbyLayer;
							break;
							} // short
						case WCS_PROJPREFS_RECORDMODE:
							{ // RecordMode is currently unused and may be retired eventually
							*ShtPtr = (short)Prefs.RecordMode;
							break;
							} // short
						case WCS_PROJPREFS_INTERACTIVEMODE:
							{
							*ShtPtr = (short)Prefs.InteractiveMode;
							break;
							} // short
						case WCS_PROJPREFS_KEYGROUPMODE:
							{
							*ShtPtr = (short)Prefs.KeyGroupMode;
							break;
							} // short
						case WCS_PROJPREFS_SAGBOTTOMHTPCT:
							{
							*ShtPtr = (short)Prefs.SAGBottomHtPct;
							break;
							} // short
						case WCS_PROJPREFS_INTERSTYLE:
							{
							*ShtPtr = (short)Prefs.InteractiveStyle;
							break;
							} // short
						case WCS_PROJPREFS_GLOBALADVANCED:
							{
							*ShtPtr = (short)Prefs.GlobalAdvancedEnabled;
							break;
							} // short
						case WCS_PROJPREFS_MAXSAGDBITEMS:
							{
							*LngPtr = (long)Prefs.MaxSAGDBEntries;
							break;
							} // Long ints
						case WCS_PROJPREFS_MAXSORTEDSAGDBITEMS:
							{
							*LngPtr = (long)Prefs.MaxSortedSAGDBEntries;
							break;
							} // Long ints
						case WCS_PROJPREFS_VECPOLYLIMITMEGS:
							{
							*LngPtr = (long)Prefs.VecPolyMemoryLimit;
							break;
							} // Long ints
						case WCS_PROJPREFS_DEMLIMITMEGS:
							{
							*LngPtr = (long)Prefs.DEMMemoryLimit;
							break;
							} // Long ints
						} // switch Item
					break;
					} // config
				} // switch subclass
			break;
			} // prefs 
		} // switch

	} // if

va_end(VarA);

} // Project::GetParam

/*===========================================================================*/
