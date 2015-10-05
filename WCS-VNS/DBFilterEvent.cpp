// DBFilterEvent.cpp
// For managing WCS DB Filter Events
// Built from scratch on 3/1/00 by Gary R. Huber
// Copyright 2000 Questar Productions

#include "stdafx.h"
#include "AppMem.h"
#include "DBFilterEvent.h"
#include "Database.h"
#include "Joe.h"
#include "Layers.h"
#include "Useful.h"
#include "Application.h"

extern WCSApp *GlobalApp;

DBFilterEvent::DBFilterEvent()
: GeoBnds(NULL), GeoPt(NULL)
{

EventType = WCS_DBFILTER_EVENTTYPE_ADD;

Enabled = PassControlPt = PassEnabled = PassLine = PassPoint = PassVector = PassDEM = PassDisabled = 1;
LayerEquals = LayerSimilar = LayerNumeric = LayerNot = NameEquals = NameSimilar = NameNumeric = NameNot = 
	LabelEquals = LabelSimilar = LabelNumeric = LabelNot = AttributeTest = AttributeNot = 
	GeoBndsInside = GeoBndsOutside = GeoBndsCompletely = GeoPtContained = GeoPtUncontained = 0;
Layer = Name = Label = Attribute = AttributeValue = NULL;
Next = NULL;

} // DBFilterEvent::DBFilterEvent

/*===========================================================================*/

DBFilterEvent::~DBFilterEvent()
{

FreeAll();

} // DBFilterEvent::~DBFilterEvent

/*===========================================================================*/

void DBFilterEvent::FreeAll(void)
{

FreeLayer();
FreeName();
FreeLabel();
FreeAttribute();
FreeAttributeValue();

} // DBFilterEvent::FreeAll

/*===========================================================================*/

void DBFilterEvent::SetScriptDefaults(void)
{

EventType = WCS_DBFILTER_EVENTTYPE_ADD;

Enabled = PassControlPt = PassVector = PassDEM = PassDisabled = PassEnabled = PassLine = PassPoint = 1;

LayerEquals = LayerSimilar = LayerNumeric = LayerNot = NameEquals = NameSimilar = NameNumeric = NameNot = 
	LabelEquals = LabelSimilar = LabelNumeric = LabelNot = AttributeTest = AttributeNot = 
	GeoBndsInside = GeoBndsOutside = GeoBndsCompletely = GeoPtContained = GeoPtUncontained = 0;
FreeLayer();
FreeName();
FreeLabel();
FreeAttribute();
FreeAttributeValue();

} // DBFilterEvent::SetScriptDefaults

/*===========================================================================*/

void DBFilterEvent::Copy(DBFilterEvent *CopyTo, DBFilterEvent *CopyFrom)
{

CopyTo->EventType = CopyFrom->EventType;
CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->PassControlPt = CopyFrom->PassControlPt;
CopyTo->PassEnabled = CopyFrom->PassEnabled;
CopyTo->PassLine = CopyFrom->PassLine;
CopyTo->PassPoint = CopyFrom->PassPoint;
CopyTo->PassVector = CopyFrom->PassVector;
CopyTo->PassDEM = CopyFrom->PassDEM;
CopyTo->PassDisabled = CopyFrom->PassDisabled;
CopyTo->LayerEquals = CopyFrom->LayerEquals;
CopyTo->LayerSimilar = CopyFrom->LayerSimilar;
CopyTo->LayerNumeric = CopyFrom->LayerNumeric;
CopyTo->LayerNot = CopyFrom->LayerNot;
CopyTo->NameEquals = CopyFrom->NameEquals;
CopyTo->NameSimilar = CopyFrom->NameSimilar;
CopyTo->NameNumeric = CopyFrom->NameNumeric;
CopyTo->NameNot = CopyFrom->NameNot;
CopyTo->LabelEquals = CopyFrom->LabelEquals;
CopyTo->LabelSimilar = CopyFrom->LabelSimilar;
CopyTo->LabelNumeric = CopyFrom->LabelNumeric;
CopyTo->LabelNot = CopyFrom->LabelNot;
CopyTo->AttributeTest = CopyFrom->AttributeTest;
CopyTo->AttributeNot = CopyFrom->AttributeNot;
CopyTo->GeoBndsInside = CopyFrom->GeoBndsInside;
CopyTo->GeoBndsOutside = CopyFrom->GeoBndsOutside;
CopyTo->GeoBndsCompletely = CopyFrom->GeoBndsCompletely;
CopyTo->GeoPtContained = CopyFrom->GeoPtContained;
CopyTo->GeoPtUncontained = CopyFrom->GeoPtUncontained;
CopyTo->GeoBnds.Copy(&CopyTo->GeoBnds, &CopyFrom->GeoBnds);
CopyTo->GeoPt.Copy(&CopyTo->GeoPt, &CopyFrom->GeoPt);

CopyTo->FreeLayer();
if (CopyFrom->Layer)
	CopyTo->NewLayer(CopyFrom->Layer);
CopyTo->FreeName();
if (CopyFrom->Name)
	CopyTo->NewName(CopyFrom->Name);
CopyTo->FreeLabel();
if (CopyFrom->Label)
	CopyTo->NewLabel(CopyFrom->Label);
CopyTo->FreeAttribute();
if (CopyFrom->Attribute)
	CopyTo->NewAttribute(CopyFrom->Attribute);
CopyTo->FreeAttributeValue();
if (CopyFrom->AttributeValue)
	CopyTo->NewAttributeValue(CopyFrom->AttributeValue);

} // DBFilterEvent::Copy

/*===========================================================================*/

void DBFilterEvent::FreeLayer(void)
{

if (Layer)
	AppMem_Free(Layer, strlen(Layer) + 1);
Layer = NULL;

} // DBFilterEvent::FreeLayer

/*===========================================================================*/

void DBFilterEvent::FreeName(void)
{

if (Name)
	AppMem_Free(Name, strlen(Name) + 1);
Name = NULL;

} // DBFilterEvent::FreeName

/*===========================================================================*/

void DBFilterEvent::FreeLabel(void)
{

if (Label)
	AppMem_Free(Label, strlen(Label) + 1);
Label = NULL;

} // DBFilterEvent::FreeLabel

/*===========================================================================*/

void DBFilterEvent::FreeAttribute(void)
{

if (Attribute)
	AppMem_Free(Attribute, strlen(Attribute) + 1);
Attribute = NULL;

} // DBFilterEvent::FreeAttribute

/*===========================================================================*/

void DBFilterEvent::FreeAttributeValue(void)
{

if (AttributeValue)
	AppMem_Free(AttributeValue, strlen(AttributeValue) + 1);
AttributeValue = NULL;

} // DBFilterEvent::FreeAttributeValue

/*===========================================================================*/

char *DBFilterEvent::NewLayer(char *SetLayer)
{

FreeLayer();
if (SetLayer[0])
	{
	if (Layer = (char *)AppMem_Alloc(strlen(SetLayer) + 1, 0))
		{
		strcpy(Layer, SetLayer);
		} // if
	} // if

return (Layer);

} // DBFilterEvent::NewLayer

/*===========================================================================*/

char *DBFilterEvent::NewName(char *SetName)
{

FreeName();
if (SetName[0])
	{
	if (Name = (char *)AppMem_Alloc(strlen(SetName) + 1, 0))
		{
		strcpy(Name, SetName);
		} // if
	} // if

return (Name);

} // DBFilterEvent::NewName

/*===========================================================================*/

char *DBFilterEvent::NewLabel(char *SetLabel)
{

FreeLabel();
if (SetLabel[0])
	{
	if (Label = (char *)AppMem_Alloc(strlen(SetLabel) + 1, 0))
		{
		strcpy(Label, SetLabel);
		} // if
	} // if

return (Label);

} // DBFilterEvent::NewLabel

/*===========================================================================*/

char *DBFilterEvent::NewAttribute(char *SetAttribute)
{

FreeAttribute();
if (SetAttribute[0])
	{
	if (Attribute = (char *)AppMem_Alloc(strlen(SetAttribute) + 1, 0))
		{
		strcpy(Attribute, SetAttribute);
		} // if
	} // if

return (Attribute);

} // DBFilterEvent::NewAttribute

/*===========================================================================*/

char *DBFilterEvent::NewAttributeValue(char *SetAttributeValue)
{

FreeAttributeValue();
if (SetAttributeValue[0])
	{
	if (AttributeValue = (char *)AppMem_Alloc(strlen(SetAttributeValue) + 1, 0))
		{
		strcpy(AttributeValue, SetAttributeValue);
		} // if
	} // if

return (AttributeValue);

} // DBFilterEvent::NewAttributeValue

/*===========================================================================*/

void DBFilterEvent::SetBounds(double LatRange[2], double LonRange[2])
{
NotifyTag Changes[2];

// can't use the bounds if they are equal
if (LatRange[0] != LatRange[1] && LonRange[0] != LonRange[1])
	{
	if (LatRange[1] < LatRange[0])
		swmem(&LatRange[0], &LatRange[1], sizeof (double));
	if (LonRange[1] < LonRange[0])
		swmem(&LonRange[0], &LonRange[1], sizeof (double));
	// if bounds appear to wrap more than halfway around earth then probably 
	// want to take the smaller arc
	if (fabs(LonRange[1] - LonRange[0]) > 180.0)
		{
		LonRange[1] -= 360.0;
		} // if

	if (LonRange[1] < LonRange[0])
		swmem(&LonRange[0], &LonRange[1], sizeof (double));
	GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(LatRange[1]);
	GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(LatRange[0]);
	GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(LonRange[1]);
	GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(LonRange[0]);
	Changes[0] = MAKE_ID(GeoBnds.GetNotifyClass(), GeoBnds.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GeoBnds.GetRAHostRoot());
	} // if

} // DBFilterEvent::SetBounds

/*===========================================================================*/

void DBFilterEvent::SetGeoPoint(double LatPt, double LonPt)
{
NotifyTag Changes[2];

GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(LatPt);
GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(LonPt);
Changes[0] = MAKE_ID(GeoBnds.GetNotifyClass(), GeoBnds.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GeoBnds.GetRAHostRoot());

} // DBFilterEvent::SetGeoPoint

/*===========================================================================*/
// returns 1 for add, -1 for subtract, and 0 for no opinion
int DBFilterEvent::PassJoe(Joe *Inspect)
{
int IsControl, IsEnabled, IsLineStyle, IsDEM, Len1, Len2, MatchFound, BoundsCompare, Approve = 1;
double JoeVal, AttrVal;
char *JoeStr, *StartCmp;
LayerStub *Stub;

if (Inspect && Enabled)
	{
	IsDEM = Inspect->TestFlags(WCS_JOEFLAG_ISDEM) ? 1: 0;
	IsControl = Inspect->TestFlags(WCS_JOEFLAG_ISCONTROL) ? 1: 0;
	if ((PassDEM && IsDEM) || (PassControlPt && IsControl) || (PassVector && ! IsControl && ! IsDEM))
		{
		IsEnabled = Inspect->TestFlags(WCS_JOEFLAG_ACTIVATED) ? 1: 0;
		if ((PassEnabled && IsEnabled) || (PassDisabled && ! IsEnabled))
			{
			IsLineStyle = Inspect->GetLineStyle() >= 4;
			if (IsDEM || (PassLine && IsLineStyle) || (PassPoint && ! IsLineStyle))
				{
				if ((Layer && (LayerEquals || LayerSimilar)) || LayerNumeric)
					{
					MatchFound = 0;
					if (Stub = Inspect->LayerList)
						{
						while (Stub && ! MatchFound)
							{
							if (JoeStr = (char *)Stub->MyLayer()->GetName())
								{
								if (LayerEquals)
									{
									if (! strcmp(Layer, JoeStr))
										{
										MatchFound = 1;
										break;
										} // if
									} // if
								else if (LayerSimilar)
									{
									Len1 = (int)strlen(Layer);
									Len2 = (int)strlen(JoeStr);
									if (Len2 >= Len1)
										{
										for (StartCmp = &JoeStr[Len2 - Len1]; StartCmp >= JoeStr; StartCmp --)
											{
											if (! strnicmp(Layer, StartCmp, Len1))
												{
												MatchFound = 1;
												break;
												} // if
											} // for
										} // if
									} // else if
								else // if (LayerNumeric)
									{
									Len2 = (int)strlen(JoeStr);
									for (StartCmp = &JoeStr[Len2 - 1]; StartCmp >= JoeStr; StartCmp --)
										{
										if (isdigit(*StartCmp))
											{
											MatchFound = 1;
											break;
											} // if
										} // for
									} // else if
								} // if
							Stub = Stub->NextLayerInObject();
							} // while
						} // if
					if ((MatchFound && LayerNot) || (! MatchFound && ! LayerNot))
						Approve = 0;
					} // if
				if ((Name && (NameEquals || NameSimilar)) || NameNumeric)
					{
					MatchFound = 0;
					if (JoeStr = (char *)Inspect->FileName())
						{
						if (NameEquals)
							{
							if (! strcmp(Name, JoeStr))
								MatchFound = 1;
							} // if
						else if (NameSimilar)
							{
							Len1 = (int)strlen(Name);
							Len2 = (int)strlen(JoeStr);
							if (Len2 >= Len1)
								{
								for (StartCmp = &JoeStr[Len2 - Len1]; StartCmp >= JoeStr; StartCmp --)
									{
									if (! strnicmp(Name, StartCmp, Len1))
										{
										MatchFound = 1;
										break;
										} // if
									} // for
								} // if
							} // else if
						else // if (NameNumeric)
							{
							Len2 = (int)strlen(JoeStr);
							for (StartCmp = &JoeStr[Len2 - 1]; StartCmp >= JoeStr; StartCmp --)
								{
								if (isdigit(*StartCmp))
									{
									MatchFound = 1;
									break;
									} // if
								} // for
							} // else if
						} // if
					if ((MatchFound && NameNot) || (! MatchFound && ! NameNot))
						Approve = 0;
					} // if
				if ((Label && (LabelEquals || LabelSimilar)) || LabelNumeric)
					{
					MatchFound = 0;
					if (JoeStr = (char *)Inspect->Name())
						{
						if (LabelEquals)
							{
							if (! strcmp(Label, JoeStr))
								MatchFound = 1;
							} // if
						else if (LabelSimilar)
							{
							Len1 = (int)strlen(Label);
							Len2 = (int)strlen(JoeStr);
							if (Len2 >= Len1)
								{
								for (StartCmp = &JoeStr[Len2 - Len1]; StartCmp >= JoeStr; StartCmp --)
									{
									if (! strnicmp(Label, StartCmp, Len1))
										{
										MatchFound = 1;
										break;
										} // if
									} // for
								} // if
							} // else if
						else // if (LabelNumeric)
							{
							Len2 = (int)strlen(JoeStr);
							for (StartCmp = &JoeStr[Len2 - 1]; StartCmp >= JoeStr; StartCmp --)
								{
								if (isdigit(*StartCmp))
									{
									MatchFound = 1;
									break;
									} // if
								} // for
							} // else if
						} // if
					if ((MatchFound && LabelNot) || (! MatchFound && ! LabelNot))
						Approve = 0;
					} // if
				if (Attribute && AttributeTest)
					{
					MatchFound = 0;
					if (Stub = Inspect->CheckTextAttributeExistance(Attribute))
						{
						if (AttributeTest == WCS_DBFILTER_ATTRIBUTE_EXISTS)
							MatchFound = 1;
						else if (AttributeValue && (JoeStr = (char *)Inspect->GetTextAttributeValue(Stub)))
							{
							switch (AttributeTest)
								{
								case WCS_DBFILTER_ATTRIBUTE_EQUALS:
									{
									if (! strcmp(JoeStr, AttributeValue))
										{
										MatchFound = 1;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_EQUALS
								case WCS_DBFILTER_ATTRIBUTE_GREATER:
									{
									if (strcmp(JoeStr, AttributeValue) > 0)
										{
										MatchFound = 1;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_GREATER
								case WCS_DBFILTER_ATTRIBUTE_GREATEREQUALS:
									{
									if (strcmp(JoeStr, AttributeValue) >= 0)
										{
										MatchFound = 1;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_GREATEREQUALS
								case WCS_DBFILTER_ATTRIBUTE_LESS:
									{
									if (strcmp(JoeStr, AttributeValue) < 0)
										{
										MatchFound = 1;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_LESS
								case WCS_DBFILTER_ATTRIBUTE_LESSEQUALS:
									{
									if (strcmp(JoeStr, AttributeValue) <= 0)
										{
										MatchFound = 1;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_LESSEQUALS
								case WCS_DBFILTER_ATTRIBUTE_SIMILAR:
									{
									Len1 = (int)strlen(AttributeValue);
									Len2 = (int)strlen(JoeStr);
									if (Len2 >= Len1)
										{
										for (StartCmp = &JoeStr[Len2 - Len1]; StartCmp >= JoeStr; StartCmp --)
											{
											if (! strnicmp(AttributeValue, StartCmp, Len1))
												{
												MatchFound = 1;
												break;
												} // if
											} // for
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_SIMILAR
								} // switch
							} // if
						} // if
					else if (Stub = Inspect->CheckIEEEAttributeExistance(Attribute))
						{
						if (AttributeTest == WCS_DBFILTER_ATTRIBUTE_EXISTS)
							MatchFound = 1;
						else if (AttributeValue)
							{
							JoeVal = (double)Inspect->GetIEEEAttributeValue(Stub);
							AttrVal = atof(AttributeValue);
							switch (AttributeTest)
								{
								case WCS_DBFILTER_ATTRIBUTE_EQUALS:
									{
									if (JoeVal == AttrVal)
										{
										MatchFound = 1;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_EQUALS
								case WCS_DBFILTER_ATTRIBUTE_GREATER:
									{
									if (JoeVal > AttrVal)
										{
										MatchFound = 1;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_GREATER
								case WCS_DBFILTER_ATTRIBUTE_GREATEREQUALS:
									{
									if (JoeVal >= AttrVal)
										{
										MatchFound = 1;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_GREATEREQUALS
								case WCS_DBFILTER_ATTRIBUTE_LESS:
									{
									if (JoeVal < AttrVal)
										{
										MatchFound = 1;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_LESS
								case WCS_DBFILTER_ATTRIBUTE_LESSEQUALS:
									{
									if (JoeVal <= AttrVal)
										{
										MatchFound = 1;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_LESSEQUALS
								case WCS_DBFILTER_ATTRIBUTE_SIMILAR:
									{
									if (fabs(AttrVal - JoeVal) < .01)
										{
										MatchFound = 1;
										break;
										} // if
									break;
									} // WCS_DBFILTER_ATTRIBUTE_SIMILAR
								} // switch
							} // else if
						} // else if
					if ((MatchFound && AttributeNot) || (! MatchFound && ! AttributeNot))
						Approve = 0;
					} // if
				if (GeoBndsInside || GeoBndsOutside)
					{
					MatchFound = 0;

					BoundsCompare = Inspect->CompareGeoBounds(GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue, GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue, GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue, GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);
					if (BoundsCompare == 0)
						MatchFound = ! GeoBndsCompletely;
					else if (BoundsCompare == 1)
						MatchFound = GeoBndsInside ? 1: 0;
					else
						MatchFound = GeoBndsOutside ? 1: 0;

					if (! MatchFound)
						Approve = 0;
					} // if
				if (GeoPtContained || GeoPtUncontained)
					{
					MatchFound = 0;

					BoundsCompare = Inspect->CompareGeoBounds(GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue, GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue, GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue, GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue);
					if (BoundsCompare == -1 && GeoPtUncontained)
						{
						MatchFound = 1;
						} // if no more testing required
					else if (BoundsCompare == 0)
						{
						BoundsCompare = Inspect->SimpleContained(GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue, GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue);
						if ((BoundsCompare && GeoPtContained) || (! BoundsCompare && GeoPtUncontained))
							MatchFound = 1;
						} // if more testing required

					if (! MatchFound)
						Approve = 0;
					} // if
				} // if correct draw style
			else
				Approve = 0;
			} // if correct enabled status
		else
			Approve = 0;
		} // if correct class
	else
		Approve = 0;
	if (Approve)
		return (EventType == WCS_DBFILTER_EVENTTYPE_ADD ? 1: -1);
	} // if enabled

return (0);

} // DBFilterEvent::PassJoe

/*===========================================================================*/

ULONG DBFilterEvent::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char ReadBuf[512];


while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_DBFILTER_LAYER:
						{
						BytesRead = ReadBlock(ffile, (char *)ReadBuf, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						NewLayer(ReadBuf);
						break;
						}
					case WCS_DBFILTER_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)ReadBuf, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						NewName(ReadBuf);
						break;
						}
					case WCS_DBFILTER_LABEL:
						{
						BytesRead = ReadBlock(ffile, (char *)ReadBuf, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						NewLabel(ReadBuf);
						break;
						}
					case WCS_DBFILTER_ATTRIBUTE:
						{
						BytesRead = ReadBlock(ffile, (char *)ReadBuf, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						NewAttribute(ReadBuf);
						break;
						}
					case WCS_DBFILTER_ATTRIBUTEVALUE:
						{
						BytesRead = ReadBlock(ffile, (char *)ReadBuf, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						NewAttributeValue(ReadBuf);
						break;
						}
					case WCS_DBFILTER_EVENTTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&EventType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_PASSCONTROL:
						{
						BytesRead = ReadBlock(ffile, (char *)&PassControlPt, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_PASSVECTOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&PassVector, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_PASSDEM:
						{
						BytesRead = ReadBlock(ffile, (char *)&PassDEM, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_PASSENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&PassEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_PASSDISABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&PassDisabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_PASSLINE:
						{
						BytesRead = ReadBlock(ffile, (char *)&PassLine, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_PASSPOINT:
						{
						BytesRead = ReadBlock(ffile, (char *)&PassPoint, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_LAYEREQUALS:
						{
						BytesRead = ReadBlock(ffile, (char *)&LayerEquals, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_LAYERSIMILAR:
						{
						BytesRead = ReadBlock(ffile, (char *)&LayerSimilar, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_LAYERNUMERIC:
						{
						BytesRead = ReadBlock(ffile, (char *)&LayerNumeric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_LAYERNOT:
						{
						BytesRead = ReadBlock(ffile, (char *)&LayerNot, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_NAMEEQUALS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NameEquals, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_NAMESIMILAR:
						{
						BytesRead = ReadBlock(ffile, (char *)&NameSimilar, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_NAMENUMERIC:
						{
						BytesRead = ReadBlock(ffile, (char *)&NameNumeric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_NAMENOT:
						{
						BytesRead = ReadBlock(ffile, (char *)&NameNot, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_LABELEQUALS:
						{
						BytesRead = ReadBlock(ffile, (char *)&LabelEquals, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_LABELSIMILAR:
						{
						BytesRead = ReadBlock(ffile, (char *)&LabelSimilar, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_LABELNUMERIC:
						{
						BytesRead = ReadBlock(ffile, (char *)&LabelNumeric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_LABELNOT:
						{
						BytesRead = ReadBlock(ffile, (char *)&LabelNot, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_ATTRIBUTETEST:
						{
						BytesRead = ReadBlock(ffile, (char *)&AttributeTest, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_ATTRIBUTENOT:
						{
						BytesRead = ReadBlock(ffile, (char *)&AttributeNot, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_GEOBNDSINSIDE:
						{
						BytesRead = ReadBlock(ffile, (char *)&GeoBndsInside, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_GEOBNDSOUTSIDE:
						{
						BytesRead = ReadBlock(ffile, (char *)&GeoBndsOutside, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_GEOBNDSCOMPLETELY:
						{
						BytesRead = ReadBlock(ffile, (char *)&GeoBndsCompletely, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_GEOPTCONTAINED:
						{
						BytesRead = ReadBlock(ffile, (char *)&GeoPtContained, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_GEOPTUNCONTAINED:
						{
						BytesRead = ReadBlock(ffile, (char *)&GeoPtUncontained, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_GEOBNDS:
						{
						BytesRead = GeoBnds.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_DBFILTER_GEOPT:
						{
						BytesRead = GeoPt.Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // DBFilterEvent::Load

/*===========================================================================*/

unsigned long int DBFilterEvent::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_EVENTTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&EventType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_PASSCONTROL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PassControlPt)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_PASSVECTOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PassVector)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_PASSDEM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PassDEM)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_PASSENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PassEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_PASSDISABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PassDisabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_PASSLINE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PassLine)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_PASSPOINT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PassPoint)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_LAYEREQUALS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LayerEquals)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_LAYERSIMILAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LayerSimilar)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_LAYERNUMERIC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LayerNumeric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_LAYERNOT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LayerNot)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_NAMEEQUALS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&NameEquals)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_NAMESIMILAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&NameSimilar)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_NAMENUMERIC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&NameNumeric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_NAMENOT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&NameNot)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_LABELEQUALS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LabelEquals)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_LABELSIMILAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LabelSimilar)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_LABELNUMERIC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LabelNumeric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_LABELNOT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LabelNot)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_ATTRIBUTETEST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AttributeTest)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_ATTRIBUTENOT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AttributeNot)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_GEOBNDSINSIDE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GeoBndsInside)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_GEOBNDSOUTSIDE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GeoBndsOutside)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_GEOBNDSCOMPLETELY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GeoBndsCompletely)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_GEOPTCONTAINED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GeoPtContained)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_GEOPTUNCONTAINED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GeoPtUncontained)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_DBFILTER_GEOBNDS + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = GeoBnds.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if registration saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_DBFILTER_GEOPT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = GeoPt.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if registration saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

if (Layer)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_LAYER, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Layer) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Layer)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (Name)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_NAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (Label)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_LABEL, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Label) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Label)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (Attribute)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_ATTRIBUTE, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Attribute) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Attribute)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (AttributeValue)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_DBFILTER_ATTRIBUTEVALUE, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(AttributeValue) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)AttributeValue)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // DBFilterEvent::Save

/*===========================================================================*/
