// CoordSys.h
// Some handy adjuct info for working with CoordSys::LoadFromArcPrj
// and related functions
// Stuff that didn't seem to belong in EffectsLib.h, because most code
// using CoordSys won't care.

// don't include CoordSys.h without including EffectsLib.h first.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_COORDSYS_H
#define WCS_COORDSYS_H

enum
	{
	WCS_COORDSYS_SLOT_UNIDENTIFIED,
	WCS_COORDSYS_SLOT_PROJECTION,
	WCS_COORDSYS_SLOT_DATUM,
	WCS_COORDSYS_SLOT_UNITS,
	WCS_COORDSYS_SLOT_ZUNITS,
	WCS_COORDSYS_SLOT_SPHEROID,
	WCS_COORDSYS_SLOT_XSHIFT,
	WCS_COORDSYS_SLOT_YSHIFT,
	WCS_COORDSYS_SLOT_ZONE,
	WCS_COORDSYS_SLOT_FIPSZONE,
	WCS_COORDSYS_SLOT_QUADRANT,
	WCS_COORDSYS_SLOT_FLIP,
	WCS_COORDSYS_SLOT_PARAMETERS, // (dummy, really)
	WCS_COORDSYS_SLOT_MAX
	}; // Slots for LoadInfo

extern char CSLoadNumericMap[];
extern char *CSLoadNameMap[];

// This should be enough for all projection methods I know of...
// We need a few extras (3 for now) for unmapped Type and Radius fields
#define WCS_COORDSYS_LOADINFO_MAXPARAMS (WCS_EFFECTS_PROJMETHOD_NUMANIMPAR + 3)
#define WCS_COORDSYS_LOADINFO_MAXENTRIES	30
#define WCS_COORDSYS_LOADINFO_MAXFIELDNAMELEN	50
#define WCS_COORDSYS_LOADINFO_MAXFIELDVALLEN	200

void AssignValueViaMetricType(AnimDoubleTime *DestADT, double NewVal);

class CSLoadEntry
	{
	public:
		int IdentSlot;
		char OrigFieldName[WCS_COORDSYS_LOADINFO_MAXFIELDNAMELEN], OrigFieldVal[WCS_COORDSYS_LOADINFO_MAXFIELDVALLEN];
		double DVal;
		int IntVal;

		CSLoadEntry() {OrigFieldName[0] = OrigFieldVal[0] = NULL; DVal = 0.0; IntVal = 0;};
	}; // CSLoadEntry


class CSLoadInfo
	{
	public:
		CSLoadEntry Entries[WCS_COORDSYS_LOADINFO_MAXENTRIES];
		double ParamBlock[WCS_COORDSYS_LOADINFO_MAXPARAMS];
		int NumEntries, NumParam;

		CSLoadInfo() {NumParam = NumEntries = 0;};

		int MatchEntry(char *InputLine, int IgnoreArcComments = 0);
		int ReadParam(char *InputLine, int IgnoreArcComments = 0);
		char *FindEntry(int Ident);
		char *FindEntry(char *FindStr);

	}; // CSLoadInfo

#endif // WCS_COORDSYS_H
