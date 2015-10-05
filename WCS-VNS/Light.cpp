/// Light.cpp
// For managing WCS Lights
// Built from scratch on 03/23/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "Conservatory.h"
#include "LightEditGUI.h"
#include "SunPosGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "GraphData.h"
#include "requester.h"
#include "Render.h"
#include "Raster.h"
#include "Database.h"
#include "Interactive.h"
#include "ViewGUI.h"
#include "Security.h"

/*===========================================================================*/

Light::Light()
: GeneralEffect(NULL), InclExcl(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_LIGHT;
SetDefaults();

} // Light::Light

/*===========================================================================*/

Light::Light(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), InclExcl(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_LIGHT;
SetDefaults();

} // Light::Light

/*===========================================================================*/

Light::Light(RasterAnimHost *RAHost, EffectsLib *Library, Light *Proto)
: GeneralEffect(RAHost), InclExcl(this)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_LIGHT;
Prev = Library->LastLight;
if (Library->LastLight)
	{
	Library->LastLight->Next = this;
	Library->LastLight = this;
	} // if
else
	{
	Library->Lights = Library->LastLight = this;
	} // else
Name[0] = NULL;
SetDefaults();
if (Proto)
	{
	Copy(this, Proto);
	Name[0] = NULL;
	strcpy(NameBase, Proto->Name);
	} // if
else
	{
	strcpy(NameBase, "Light");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // Light::Light

/*===========================================================================*/

Light::~Light()
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->SPG && GlobalApp->GUIWins->SPG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->SPG;
		GlobalApp->GUIWins->SPG = NULL;
		} // if
	if (GlobalApp->GUIWins->NPG && GlobalApp->GUIWins->NPG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->NPG;
		GlobalApp->GUIWins->NPG = NULL;
		} // if
	} // if

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

if (LightPos)
	delete LightPos;
if (LightAim)
	delete LightAim;
LightPos = LightAim = NULL;

KillShadows();

} // Light::~Light

/*===========================================================================*/

void Light::KillShadows(void)
{
ShadowMap3D *TempShadow;

while (Shadows)
	{
	TempShadow = Shadows->Next;
	delete Shadows;
	Shadows = TempShadow;
	} // while

} // Light::KillShadows

/*===========================================================================*/

void Light::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_LIGHT_NUMANIMPAR] = {0.0, 0.0, EffectsLib::CelestialPresetDistance[0], 
														0.0, 0.0, 0.0, 30.0, 20.0, EffectsLib::CelestialPresetRadius[0]};
double RangeDefaults[WCS_EFFECTS_LIGHT_NUMANIMPAR][3] = {
												90.0, -90.0, .0001,	// lat
												FLT_MAX, -FLT_MAX, .0001,	// lon
												FLT_MAX, -FLT_MAX, 1.0,		// elev
												FLT_MAX, -FLT_MAX, 1.0,		// heading
												FLT_MAX, -FLT_MAX, 1.0,		// pitch
												5.0, 0.0, .1,		// falloff exp
												180.0, 0.0, 1.0,		// spot cone angle
												90.0, 0.0, 1.0,		// spot cone soft edge angle
												FLT_MAX, 0.0, 1.0		// light radius for soft shadows
												};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
Color.SetDefaults(this, (char)Ct);
Color.SetValue3(1.0, 1.0, 1.0);
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for

LightType = WCS_EFFECTS_LIGHTTYPE_PARALLEL;
Distant = 1;
CastShadows = 1;
SoftShadows = AAShadows = 0;
ColorEvaluated = 0;
IllumAtmosphere = 1;
TargetObj = NULL;
LightPos = LightAim = NULL;	// allocated by renderer
LightPosUnit[0] = LightPosUnit[1] = LightPosUnit[2] = 0.0;
Shadows = NULL;
MaxConeCosAngle = MaxHotSpotCosAngle = ConeEdgeCosAngle = MaxIllumDist = 0.0;
MaxIllumDistSq = TangentDistance = CosTangentAngle = InvCosTangentAngleDifference = CosTangentAnglePlus5Pct = 0.0;
CompleteColor[0] = CompleteColor[1] = CompleteColor[2] = 0.0;
Floating = FlipFoliage = 0;
FallOffExp = 0.0;
FallSeason = 0;

AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);

} // Light::SetDefaults

/*===========================================================================*/

void Light::Copy(Light *CopyTo, Light *CopyFrom)
{
long Result = -1;
NotifyTag Changes[2];

CopyTo->TargetObj = NULL;
if (CopyFrom->TargetObj)
	{
	if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
		{
		CopyTo->TargetObj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->TargetObj);
		} // if no need to make another copy, its all in the family
	else
		{
		if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->TargetObj->EffectType, CopyFrom->TargetObj->Name))
			{
			Result = UserMessageCustom("Copy Light", "How do you wish to resolve Target Object name collisions?\n\nLink to existing Objects, replace existing Objects, or create new Objects?",
				"Link", "Create", "Overwrite", 1);
			} // if
		if (Result <= 0)
			{
			CopyTo->TargetObj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->TargetObj->EffectType, NULL, CopyFrom->TargetObj);
			} // if create new
		else if (Result == 1)
			{
			CopyTo->TargetObj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->TargetObj);
			} // if link to existing
		else if (CopyTo->TargetObj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->TargetObj->EffectType, CopyFrom->TargetObj->Name))
			{
			CopyTo->TargetObj->Copy(CopyTo->TargetObj, CopyFrom->TargetObj);
			Changes[0] = MAKE_ID(CopyTo->TargetObj->GetNotifyClass(), CopyTo->TargetObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->TargetObj);
			} // else if found and overwrite
		else
			{
			CopyTo->TargetObj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->TargetObj->EffectType, NULL, CopyFrom->TargetObj);
			} // else
		} // else better copy or overwrite it since its important to get just the right object
	} // if

CopyTo->Color.Copy(&CopyTo->Color, &CopyFrom->Color);
CopyTo->InclExcl.Copy(&CopyTo->InclExcl, &CopyFrom->InclExcl);
CopyTo->LightType = CopyFrom->LightType;
CopyTo->Distant = CopyFrom->Distant;
CopyTo->CastShadows = CopyFrom->CastShadows;
CopyTo->SoftShadows = CopyFrom->SoftShadows;
CopyTo->AAShadows = CopyFrom->AAShadows;
CopyTo->IllumAtmosphere = CopyFrom->IllumAtmosphere;
CopyTo->FlipFoliage = CopyFrom->FlipFoliage;
CopyTo->FallSeason = CopyFrom->FallSeason;
CopyTo->Floating = CopyFrom->Floating;
CopyPoint3d(CopyTo->LightPosUnit, CopyFrom->LightPosUnit);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // Light::Copy

/*===========================================================================*/

ULONG Light::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TargetName[256];

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
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (BrowseInfo)
							BrowseInfo->FreeAll();
						else
							BrowseInfo = new BrowseData();
						if (BrowseInfo)
							BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_LIGHTTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&LightType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_DISTANT:
						{
						BytesRead = ReadBlock(ffile, (char *)&Distant, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_CASTSHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&CastShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_SOFTSHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&SoftShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_AASHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&AAShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_ILLUMATMOSPHERE:
						{
						BytesRead = ReadBlock(ffile, (char *)&IllumAtmosphere, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&Floating, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_FLIPFOLIAGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&FlipFoliage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_FALLSEASON:
						{
						BytesRead = ReadBlock(ffile, (char *)&FallSeason, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_LAT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_LON:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_ELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_HEADING:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_PITCH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_FALLOFFEXP:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_SPOTCONE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_SPOTCONEEDGE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_LIGHTRADIUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_COLOR:
						{
						BytesRead = Color.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_TEXCOLOR:
						{
						if (TexRoot[WCS_EFFECTS_LIGHT_TEXTURE_COLOR] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_LIGHT_TEXTURE_COLOR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_LIGHT_INCLUDEEXCLUDE:
						{
						BytesRead = InclExcl.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LIGHT_TARGETNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TargetName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TargetName[0])
							{
							TargetObj = (Object3DEffect *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_OBJECT3D, TargetName);
							} // if
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

if (Floating)
	{
	// some early projects were built when we didn't implement separate variables
	// those projects need to be brought into line
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetFloating(1);
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetFloating(1);
	} // if

return (TotalRead);

} // Light::Load

/*===========================================================================*/

unsigned long int Light::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_LIGHT_NUMANIMPAR] = {WCS_EFFECTS_LIGHT_LAT,
																 WCS_EFFECTS_LIGHT_LON,
																 WCS_EFFECTS_LIGHT_ELEV,
																 WCS_EFFECTS_LIGHT_HEADING,
																 WCS_EFFECTS_LIGHT_PITCH,
																 WCS_EFFECTS_LIGHT_FALLOFFEXP,
																 WCS_EFFECTS_LIGHT_SPOTCONE,
																 WCS_EFFECTS_LIGHT_SPOTCONEEDGE,
																 WCS_EFFECTS_LIGHT_LIGHTRADIUS
																 };
unsigned long int TextureItemTag[WCS_EFFECTS_LIGHT_NUMTEXTURES] = {WCS_EFFECTS_LIGHT_TEXCOLOR};

if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
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
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LIGHT_LIGHTTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LightType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LIGHT_DISTANT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Distant)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LIGHT_CASTSHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CastShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LIGHT_SOFTSHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SoftShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LIGHT_AASHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AAShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LIGHT_ILLUMATMOSPHERE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&IllumAtmosphere)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LIGHT_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LIGHT_FLIPFOLIAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FlipFoliage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LIGHT_FALLSEASON, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FallSeason)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} /* if wrote size of block */
				else
					goto WriteError;
				} /* if anim param saved */
			else
				goto WriteError;
			} /* if size written */
		else
			goto WriteError;
		} /* if tag written */
	else
		goto WriteError;
	} // for

ItemTag = WCS_EFFECTS_LIGHT_COLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Color.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} /* if wrote size of block */
			else
				goto WriteError;
			} /* if Color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct])
		{
		ItemTag = TextureItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TexRoot[Ct]->Save(ffile))
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
					} // if texture saved
				else
					goto WriteError;
				} // if size written
			else
				goto WriteError;
			} // if tag written
		else
			goto WriteError;
		} // if
	} // for

ItemTag = WCS_EFFECTS_LIGHT_INCLUDEEXCLUDE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = InclExcl.Save(ffile))
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
			} // if texture saved
		else
			goto WriteError;
		} // if size written
	else
		goto WriteError;
	} // if tag written
else
	goto WriteError;

if (TargetObj)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LIGHT_TARGETNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(TargetObj->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)TargetObj->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Light::Save

/*===========================================================================*/

void Light::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->NPG)
	{
	delete GlobalApp->GUIWins->NPG;
	}
GlobalApp->GUIWins->NPG = new LightEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->NPG)
	{
	GlobalApp->GUIWins->NPG->Open(GlobalApp->MainProj);
	}

} // Light::Edit

/*===========================================================================*/

short Light::AnimateShadows(void)
{

if (GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_LAT)->GetNumNodes(0) > 1)
	return (1);
if (GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_LON)->GetNumNodes(0) > 1)
	return (1);
if (GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_ELEV)->GetNumNodes(0) > 1)
	return (1);

return (0);

} // Light::AnimateShadows

/*===========================================================================*/

char *LightCritterNames[WCS_EFFECTS_LIGHT_NUMANIMPAR] = {"Light Latitude (deg)", "Light Longitude (deg)", 
	"Light Elevation (m)", "Heading (deg)", "Pitch (deg)", "Falloff Exponent", "Spotlight Cone Angle (deg)",
	"Soft Edge Angle (deg)", "Light Radius (m)"};
char *LightTextureNames[WCS_EFFECTS_LIGHT_NUMTEXTURES] = {"Light Color"};

char *Light::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (LightCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (LightTextureNames[Ct]);
		} // if
	} // for
if (Test == &Color)
	return ("Color");
return ("");

} // Light::GetCritterName

/*===========================================================================*/

char *Light::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as an Light Texture! Remove anyway?");

} // Light::OKRemoveRaster

/*===========================================================================*/

char *Light::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? LightTextureNames[TexNumber]: (char*)"");

} // Light::GetTextureName

/*===========================================================================*/

void Light::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = (Test == GetTexRootPtr(WCS_EFFECTS_LIGHT_TEXTURE_COLOR));
ApplyToDisplace = 0;

} // Light::GetTextureApplication

/*===========================================================================*/

RootTexture *Light::NewRootTexture(long TexNumber)
{
char ApplyToColor = (TexNumber == WCS_EFFECTS_LIGHT_TEXTURE_COLOR);
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // Light::NewRootTexture

/*===========================================================================*/

int Light::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
if (Color.SetToTime(Time))
	Found = 1;

return (Found);

} // Light::SetToTime

/*===========================================================================*/

long Light::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;

if (GeneralEffect::GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (Color.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // Light::GetKeyFrameRange

/*===========================================================================*/

int Light::SetTarget(Object3DEffect *NewTarget)
{
NotifyTag Changes[2];

TargetObj = NewTarget;
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // Light::SetTarget

/*===========================================================================*/

int Light::RemoveRAHost(RasterAnimHost *RemoveMe)
{
NotifyTag Changes[2];

if (TargetObj == (Object3DEffect *)RemoveMe)
	{
	TargetObj = NULL;

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // Light::RemoveRAHost

/*===========================================================================*/

int Light::ApproveInclExclClass(long MyClass)
{

if (MyClass == WCS_EFFECTSSUBCLASS_OBJECT3D ||
	MyClass == WCS_EFFECTSSUBCLASS_LAKE ||
	MyClass == WCS_EFFECTSSUBCLASS_ECOSYSTEM ||
	MyClass == WCS_EFFECTSSUBCLASS_CLOUD ||
	MyClass == WCS_EFFECTSSUBCLASS_CMAP ||
	MyClass == WCS_EFFECTSSUBCLASS_FOLIAGE ||
	MyClass == WCS_EFFECTSSUBCLASS_STREAM ||
	//MyClass == WCS_EFFECTSSUBCLASS_MATERIAL ||
	MyClass == WCS_EFFECTSSUBCLASS_CELESTIAL ||
	//MyClass == WCS_EFFECTSSUBCLASS_STARFIELD ||
	MyClass == WCS_EFFECTSSUBCLASS_GROUND ||
	//MyClass == WCS_EFFECTSSUBCLASS_SNOW ||
	//MyClass == WCS_EFFECTSSUBCLASS_SKY ||
	MyClass == WCS_EFFECTSSUBCLASS_ATMOSPHERE ||
	MyClass == WCS_EFFECTSSUBCLASS_FENCE ||
	MyClass == WCS_EFFECTSSUBCLASS_LABEL)
	return (1);

return (0);

} // Light::ApproveInclExclClass

/*===========================================================================*/

char Light::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME ||
	DropType == WCS_EFFECTSSUBCLASS_OBJECT3D ||
	ApproveInclExclClass(DropType))
	return (1);

return (0);

} // Light::GetRAHostDropOK

/*===========================================================================*/

int Light::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success, Result;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Light *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Light *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = Color.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s as a target or include/exclude item?", DropSource->Name, DropSource->Type, NameStr);
	if ((Result = UserMessageCustom(NameStr, QueryStr, "Target", "Cancel", "Include/Exclude", 1)) == 1)
		{
		Success = SetTarget((Object3DEffect *)DropSource->DropSource);
		} // if position
	else if (Result == 2)
		{
		Success = InclExcl.AddRAHost(DropSource->DropSource) ? 1: 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // else if alignment
	} // else if
else if (DropSource->TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && DropSource->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	Success = InclExcl.AddRAHost(DropSource->DropSource) ? 1: 0;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // else if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // Light::ProcessRAHostDragDrop

/*===========================================================================*/

int Light::GetRAHostAnimated(void)
{

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (Color.GetRAHostAnimated())
	return (1);

return (0);

} // Light::GetRAHostAnimated

/*===========================================================================*/

unsigned long Light::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	if (! RAParent)
		Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_LIGHT | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // Light::GetRAFlags

/*===========================================================================*/

int Light::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil, ThematicMap **&ThemeAffil)
{
long Ct;

AnimAffil = NULL;
TexAffil = NULL;
ThemeAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			return (1);
			} // if
		} // for
	if (ChildA == &Color)
		{
		AnimAffil = (AnimCritter *)ChildA;
		TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_LIGHT_TEXTURE_COLOR);
		return (1);
		} // if
	} // if
else if (ChildB)
	{
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetTexRootPtrAddr(Ct))
			{
			TexAffil = (RootTexture **)ChildB;
			switch (Ct)
				{
				case WCS_EFFECTS_LIGHT_TEXTURE_COLOR:
					{
					AnimAffil = &Color;
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // Light::GetAffiliates

/*===========================================================================*/

void Light::GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)
{

if (! FlagMe)
	{
	Prop->InterFlags = 0;
	return;
	} // if

Prop->InterFlags = (WCS_RAHOST_INTERBIT_CLICKTOPOS | WCS_RAHOST_INTERBIT_MOVEX | 
					WCS_RAHOST_INTERBIT_MOVEY | WCS_RAHOST_INTERBIT_MOVEZ | 
					WCS_RAHOST_INTERBIT_MOVEELEV |
					WCS_RAHOST_INTERBIT_SCALEY | WCS_RAHOST_INTERBIT_SCALEZ |
					WCS_RAHOST_INTERBIT_ROTATEX | WCS_RAHOST_INTERBIT_ROTATEY);

} // Light::GetInterFlags

/*===========================================================================*/

int Light::ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation)
{
char ElevText[256];
double NewVal, NewLat, NewLon;

if (! MoveMe)
	{
	return (0);
	} // if

// position group
if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOS)
	{
	if (Distant)
		{
		if (GlobalApp->GUIWins->CVG->CollideCoord(Data->ViewSource, NewLat, NewLon, Data->PixelX, Data->PixelY, 
			AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue))
			{
			AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetCurValue(NewLat);
			AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetCurValue(NewLon);
			} // if
		} // if
	else if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		{
		sprintf(ElevText, "%f", Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
		if (GetInputString("Enter new elevation for Light. Clear the field to leave at current elevation.", ":;*/?`#%", ElevText))
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
				AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
			if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
			if (ElevText[0])
				AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].SetCurValue(atof(ElevText));
			} // if
		} // else
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOSNOQUERY)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_MOVEXYZ ||
	Operation == WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV)
	{
	Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
	Data->Value[WCS_DIAGNOSTIC_LATITUDE] = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
	Data->Value[WCS_DIAGNOSTIC_LONGITUDE] = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
	Data->Value[WCS_DIAGNOSTIC_ELEVATION] = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;
	GlobalApp->GUIWins->CVG->ScaleMotion(Data);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SCALE)
	{
	// allow size factor
	NewVal = -Data->MoveY / 100.0;
	NewVal += 1.0;
	if (NewVal < .01)
		NewVal = .01;
	// let animcritter do clamping
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].SetCurValue(AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].CurValue * NewVal);
	NewVal = -Data->MoveZ / 100.0;
	NewVal += 1.0;
	if (NewVal < .01)
		NewVal = .01;
	// let animcritter do clamping
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].SetCurValue(AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue * NewVal);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETSIZE)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALX])
		{
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALY])
		{
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALZ])
		{
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_ROTATE)
	{
	// allow heading and pitch
	NewVal = Data->MoveX * .5;
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].SetCurValue(AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].CurValue + NewVal);
	NewVal = Data->MoveY * .5;
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].SetCurValue(AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].CurValue + NewVal);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETROTATION)
	{
	// allow heading and pitch
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALX])
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALY])
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALY]);
	} // if 

return (0);	// return 0 if nothing changed

} // Light::ScaleMoveRotate

/*===========================================================================*/

RasterAnimHost *Light::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	return (&Color);
if (Current == &Color)
	return (&InclExcl);
if (Current == &InclExcl)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for
if (Found && TargetObj)
	return (TargetObj);

return (NULL);

} // Light::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *Light::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_LAT))
	return (GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_LON));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_LON))
	return (GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_ELEV));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_ELEV))
	return (GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_LAT));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_HEADING))
	return (GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_PITCH));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_PITCH))
	return (GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_HEADING));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE))
	return (GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE))
	return (GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE));

return (NULL);

} // Light::GetNextGroupSibling

/*===========================================================================*/

void Light::SetFloating(char NewFloating)
{
Database *CurDB;
NotifyTag Changes[2];
RasterAnimHostProperties Prop;
DEMBounds CurBounds;

if (NewFloating)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
	GetRAHostProperties(&Prop);
	if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
		{
		if (! UserMessageOKCAN("Set Light Floating", "Keyframes exist for this Light. They will be removed\n if the Light is set to float. Remove key frames?"))
			{
			Floating = 0;
			return;
			} // if
		} // if
	Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
	Prop.FrameOperator = WCS_KEYOPERATION_ALLKEYS;
	Prop.KeyframeOperation = WCS_KEYOPERATION_DELETE;
	Prop.TypeNumber = WCS_EFFECTSSUBCLASS_LIGHT;
	Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;
	SetRAHostProperties(&Prop);
	if (CurDB = GlobalApp->AppDB)
		{
		if (! CurDB->FillDEMBounds(&CurBounds))
			{
			CurBounds.North = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y) + .5;
			CurBounds.South = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y) - .5;
			CurBounds.West = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X) + .5;
			CurBounds.East = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X) - .5;
			} // if
		if ((CurBounds.North + CurBounds.South) * 0.5 > 0.0)
			AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetValue(((CurBounds.North + CurBounds.South) * 0.5) - 30.0);
		else
			AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetValue(((CurBounds.North + CurBounds.South) * 0.5) + 30.0);
		AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetValueNotify(((CurBounds.East + CurBounds.West) * 0.5) - 30.0);
		} // if
	Floating = 1;

	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetFloating(1);
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetFloating(1);
	} // if enable floating
else
	{
	Floating = 0;
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetFloating(0);
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetFloating(0);
	} // else if unfloat

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // Light::SetFloating

/*===========================================================================*/

int Light::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
double ZAngle, MinDEMElevRadius, LightDistance;
float MaxDEMElev, MinDEMElev;

if (! LightPos)
	{
	if (! (LightPos = new VertexDEM()))
		return (0);
	} // if
if (! LightAim)
	{
	if (! (LightAim = new VertexDEM()))
		return (0);
	} // if

LightPos->Lat = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
LightPos->Lon = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
LightPos->Elev = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;

if (Distant)
	LightPos->Lon -= Rend->EarthRotation;

#ifdef WCS_BUILD_VNS
Rend->DefCoords->DegToCart(LightPos);
#else // WCS_BUILD_VNS
LightPos->DegToCart(Rend->PlanetRad);
#endif // WCS_BUILD_VNS

if (! Distant)
	{
	if (LightType != WCS_EFFECTS_LIGHTTYPE_OMNI)
		{
		if (TargetObj)
			{
			// translate object origin's lat/lon/elev + xyz offset into global cartesian coordinates
			// the same as would be done to render the object
			TargetObj->FindCurrentCartesian(Rend, LightAim, 1);	// 1 = center on object origin
			LightAim->GetPosVector(LightPos);
			LightAim->RotateY(-LightPos->Lon);
			LightAim->RotateX(90.0 - LightPos->Lat);
			// apply heading and pitch rotations
			// find angle that vector makes around y with +z axis
			ZAngle = LightAim->FindAngleYfromZ();
			// rotate around y by -z angle
			LightAim->RotateY(-ZAngle);
			// apply pitch rotation
			LightAim->RotateX(AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].CurValue);
			// rotate around y axis
			LightAim->RotateY(ZAngle + AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].CurValue);
			// rotate back to camera position
			LightAim->RotateX(LightPos->Lat - 90.0);
			LightAim->RotateY(LightPos->Lon);
			} // if
		else
			{
			// set defuault light orientation
			LightAim->XYZ[0] = LightAim->XYZ[1] = 0.0;
			LightAim->XYZ[2] = 1.0;
			// apply heading and pitch rotations
			LightAim->RotateX(AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].CurValue);
			// rotate around y axis
			LightAim->RotateY(AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].CurValue);
			// rotate to light position
			LightAim->RotateX(LightPos->Lat - 90.0);
			LightAim->RotateY(LightPos->Lon);
			} // else
		// reverse vector orientation so it points at the light
		NegateVector(LightAim->XYZ);
		} // if
	} // if aimed light
else
	{
	// copy the light position into the aim vector so vector is aimed from center of earth at light
	LightAim->CopyXYZ(LightPos);
	} // else

// Normalize the aim vector for the sake of good form (and in case its needed)
LightAim->UnitVector();

// Create a unitized vector version of LightPos->XYZ for more optimal use with VectorAngle calls
CopyPoint3d(LightPosUnit, LightPos->XYZ);
if(UnitVectorMagnitude(LightPosUnit) == 0.0) // Light position at center of Earth?
	{
	// this illogical and non-productive case can make optimization of general cases later on
	// very difficult due to additional error-checking, so we're preventing it in order to
	// accelerate the more useful general cases.
	char Message[256];
	sprintf(Message, "Light %s: Lights cannot be located at the exact center of the planet. Aborting.", GetName());
	UserMessageOK("Renderer Light Init", Message);
	return(0); // sowwy charwie.
	} // if

if (AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue > 180.0)
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue = 180.0;
if (AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue + 2 * AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].CurValue > 180.0)
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].CurValue = (180.0 - AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue) * 0.5;

MaxConeCosAngle = cos((AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].CurValue + AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue * 0.5) * PiOver180);
MaxHotSpotCosAngle = cos((AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue * 0.5) * PiOver180);
ConeEdgeCosAngle = MaxHotSpotCosAngle - MaxConeCosAngle;	// cosine of hot spot angle will be larger than total cone cosine
if (ConeEdgeCosAngle > 0.0)
	InvConeEdgeCosAngle = 1.0 / ConeEdgeCosAngle;
else
	InvConeEdgeCosAngle = 1.0;	// it will never be used but set it anyway
// max distance is computed for a minimum visible light amount of .1%
// guard against possibility of float overflow
if (AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].CurValue > 0.1)
	{
	MaxIllumDist = SafePow(1000.0 * Color.Intensity.CurValue, 1.0 / AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].CurValue);
	MaxIllumDistSq = MaxIllumDist * MaxIllumDist;
	} // if
else
	MaxIllumDistSq = MaxIllumDist = FLT_MAX;

CompleteColor[0] = Color.GetCompleteValue(0);
CompleteColor[1] = Color.GetCompleteValue(1);
CompleteColor[2] = Color.GetCompleteValue(2);
ColorEvaluated = 1;

Rend->DBase->GetDEMElevRange(MaxDEMElev, MinDEMElev);
MinDEMElevRadius = Rend->PlanetRad + MinDEMElev;
LightDistance = Rend->PlanetRad + LightPos->Elev;
TangentDistance = sqrt(LightDistance * LightDistance - MinDEMElevRadius * MinDEMElevRadius);
CosTangentAngle = TangentDistance / LightDistance;
InvCosTangentAngleDifference = .05 * (1.0 - CosTangentAngle);
CosTangentAnglePlus5Pct = CosTangentAngle + InvCosTangentAngleDifference;
InvCosTangentAngleDifference = InvCosTangentAngleDifference > 0.0 ? 1.0 / InvCosTangentAngleDifference: 0.0;
//InvCosTangentAngle = 1.0 / CosTangentAngle;

if ((FallOffExp = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].CurValue) > 0.0 && FallOffExp < .1)
	FallOffExp = .1;	// smaller non-zero values can cause float overflow

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // Light::InitFrameToRender

/*===========================================================================*/

void Light::EvalColor(PixelData *Pix)
{
double Value[3], TexOpacity;

CompleteColor[0] = Color.GetCompleteValue(0);
CompleteColor[1] = Color.GetCompleteValue(1);
CompleteColor[2] = Color.GetCompleteValue(2);
ColorEvaluated = 1;

if (TexRoot[WCS_EFFECTS_LIGHT_TEXTURE_COLOR] && TexRoot[WCS_EFFECTS_LIGHT_TEXTURE_COLOR]->Enabled)
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_LIGHT_TEXTURE_COLOR]->Eval(Value, Pix->TexData)) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			TexOpacity = 1.0 - TexOpacity;
			CompleteColor[0] = CompleteColor[0] * TexOpacity + Value[0];
			CompleteColor[1] = CompleteColor[1] * TexOpacity + Value[1];
			CompleteColor[2] = CompleteColor[2] * TexOpacity + Value[2];
			} // if
		else
			{
			CompleteColor[0] = Value[0];
			CompleteColor[1] = Value[1];
			CompleteColor[2] = Value[2];
			} // else
		} // if
	} // if

} // Light::EvalColor

/*===========================================================================*/

// aray used for offsetting antialiased shadow map points
double LightShadowAAOffset[WCS_LIGHT_MAXSHADOWAASAMPLES][2] = {0.0, 0.0, -.44, -.1, .07, -.47, .48, .06, -.08, .45,
																-.15, -.21, .26, -.13, .29, .22, -.24, .16};

// This f() returns the amount of light passed to the pixel (0-1).
// It will never exceed 1. it is not dependent on light intensity
// but does take into account all objects including terrain that cast shadows; soft shadows,
// and light radius; and shadow map offset.

double Light::EvaluateShadows(PixelData *Pix)
{
double CurLightPassed, CurLightLost, LightPassed = 1.0, ScrnCoords[3], AACoords[2], NearZ, 
	RealDistToLight, DistToLight, DistToPoint, Radius, Inv255 = 1.0 / 255.0;
ShadowMap3D *CurShadow = Shadows;
long PixZip, AACt, AASamples;

while (CurShadow && LightPassed > 0.0)
	{
	if (CurShadow->MyType & Pix->ShadowFlags)
		{
		// if not just self-shadowed or if casting and receiving object are the same
//		if (Pix->ReceiveShadows == 1 || CurShadow->MyEffect == Pix->Object)
			{
			// project the pixel using CurShadow's projection matrix
			if (CurShadow->ProjectPoint(Pix->XYZ, ScrnCoords))
				{
				// special treatment for AA and Soft shadows unless it is a cloud which is softened by barycentric sampling
				if ((AAShadows || SoftShadows) && CurShadow->MyType != WCS_SHADOWTYPE_CLOUDSM)
					{
					AASamples = 0;
					ScrnCoords[2] -= Pix->ShadowOffset;
					CurLightLost = 0.0;
					NearZ = FLT_MAX;
					for (AACt = 0; AACt < WCS_LIGHT_MAXSHADOWAASAMPLES; AACt ++)
						{
						AACoords[0] = ScrnCoords[0] + LightShadowAAOffset[AACt][0];
						AACoords[1] = ScrnCoords[1] + LightShadowAAOffset[AACt][1];
						if (AACoords[0] >= 0 && AACoords[0] < CurShadow->Rast->Cols
							&& AACoords[1] >= 0 && AACoords[1] < CurShadow->Rast->Rows)
							{
							PixZip = quickftol(AACoords[1]) * CurShadow->Rast->Cols + quickftol(AACoords[0]);
							if (CurShadow->AABuf[PixZip])
								{
								if (ScrnCoords[2] > CurShadow->ZBuf[PixZip])
									{
									// terrain shadows behave badly, creating windrows of shadow where shadow pixels are
									// obscured due to insufficient resolution in shadow map. Compensate by looking at the
									// next higher pixel in the map.
									if (CurShadow->MyType == WCS_SHADOWTYPE_TERRAIN && PixZip >= CurShadow->Rast->Cols)
										{
										PixZip -= CurShadow->Rast->Cols;
										if (ScrnCoords[2] > CurShadow->ZBuf[PixZip])
											{
											CurLightLost += (CurShadow->AABuf[PixZip] * Inv255);
											if (CurShadow->ZBuf[PixZip] < NearZ)
												NearZ = CurShadow->ZBuf[PixZip];
											} // if
										} // if
									else
										{
										CurLightLost += (CurShadow->AABuf[PixZip] * Inv255);
										if (CurShadow->ZBuf[PixZip] < NearZ)
											NearZ = CurShadow->ZBuf[PixZip];
										} // if
									} // if
								} // if
							AASamples ++;
							} // if
						} // for
					if (SoftShadows && CurLightLost > 0.0)
						{
						// randomize in-plane projection position based on light radius and distance from
						// light to object to pixel
						// calculate shadow width in shadow map units
						DistToLight = NearZ + CurShadow->ZOffset;
						DistToPoint = ScrnCoords[2] + Pix->ShadowOffset - NearZ;
						RealDistToLight = DistToLight + CurShadow->DistanceOffset;
						Radius = (CurShadow->HorScale * AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].CurValue * DistToPoint / RealDistToLight) / DistToLight;
						// we've already super-sampled anything with radius less than .5 pixels
						if (Radius > .5)
							{
							Radius *= 2.0;	// offset array is based on radius of .5 pixels so need to bump it up to 1.0
							// Use the antialiased value already computed for radius < .5
							// This will give us some antialiasing even if radius is small.
							// Treat the composite as a new sample at center of zone and skip sampling the centerpoint again
							CurLightLost /= AASamples;
							AASamples = 1;
							for (AACt = 1; AACt < WCS_LIGHT_MAXSHADOWAASAMPLES; AACt ++)
								{
								AACoords[0] = ScrnCoords[0] + LightShadowAAOffset[AACt][0] * Radius;
								AACoords[1] = ScrnCoords[1] + LightShadowAAOffset[AACt][1] * Radius;
								if (AACoords[0] >= 0 && AACoords[0] < CurShadow->Rast->Cols
									&& AACoords[1] >= 0 && AACoords[1] < CurShadow->Rast->Rows)
									{
									PixZip = quickftol(AACoords[1]) * CurShadow->Rast->Cols + quickftol(AACoords[0]);
									if (CurShadow->AABuf[PixZip])
										{
										if (ScrnCoords[2] > CurShadow->ZBuf[PixZip])
											{
											// terrain shadows behave badly, creating windrows of shadow where shadow pixels are
											// obscured due to insufficient resolution in shadow map. Compensate by looking at the
											// next higher pixel in the map.
											if (CurShadow->MyType == WCS_SHADOWTYPE_TERRAIN && PixZip >= CurShadow->Rast->Cols)
												{
												PixZip -= CurShadow->Rast->Cols;
												if (ScrnCoords[2] > CurShadow->ZBuf[PixZip])
													{
													CurLightLost += (CurShadow->AABuf[PixZip] * Inv255);
													} // if
												} // if
											else
												CurLightLost += (CurShadow->AABuf[PixZip] * Inv255);
											} // if
										} // if
									AASamples ++;
									} // if
								} // for
							} // if
						} // if
					if (AASamples > 0)
						LightPassed *= (1.0 - (CurLightLost / AASamples));
					} // if
				else
					{
					PixZip = quickftol(ScrnCoords[1]) * CurShadow->Rast->Cols + quickftol(ScrnCoords[0]);
					if (CurShadow->AABuf[PixZip])
						{
						// <<<>>>gh randomize z offset maybe
						ScrnCoords[2] -= Pix->ShadowOffset;
						if (ScrnCoords[2] > CurShadow->ZBuf[PixZip])
							{
							// terrain shadows behave badly, creating windows of shadow where shadow pixels are
							// obscured due to insufficient resolution in shadow map. Compensate by looking at the
							// next higher pixel in the map.
							if (CurShadow->MyType == WCS_SHADOWTYPE_TERRAIN && PixZip >= CurShadow->Rast->Cols)
								{
								PixZip -= CurShadow->Rast->Cols;
								if (ScrnCoords[2] > CurShadow->ZBuf[PixZip])
									CurLightPassed = (1.0 - CurShadow->AABuf[PixZip] * Inv255);
								else
									CurLightPassed = 1.0;
								} // if
							else if (CurShadow->MyType == WCS_SHADOWTYPE_CLOUDSM)
								{
								if (ScrnCoords[2] > CurShadow->ZBuf[PixZip])
									{
									// barycentric sample pixel
									CurLightPassed = (1.0 - CurShadow->SampleWeighted(PixZip, ScrnCoords[0], ScrnCoords[1]) * Inv255);
									} // if
								else
									CurLightPassed = 1.0;
								} // if
							else
								CurLightPassed = (1.0 - CurShadow->AABuf[PixZip] * Inv255);
							LightPassed *= CurLightPassed;
							} // if
						} // if
					} // else not AA shadows
				} // if
			// if self-shadowed we're done
//			if (Pix->ReceiveShadows == 2)
//				break;
			} // if
		} // if shadow type matches
	CurShadow = CurShadow->Next;
	} // if

return (LightPassed);

} // Light::EvaluateShadows

/*===========================================================================*/

ShadowMap3D *Light::MatchShadow(GeneralEffect *TestObj, Joe *TestJoe, VectorPoint *TestVtx, char TestType)
{
ShadowMap3D *CurShadow = Shadows;

while (CurShadow)
	{
	if (CurShadow->MyEffect == TestObj && CurShadow->MyJoe == TestJoe && CurShadow->MyVertex == TestVtx
		&& CurShadow->MyType == TestType)
		return (CurShadow);
	CurShadow = CurShadow->Next;
	} // while

return (NULL);

} // Light::MatchShadow

/*===========================================================================*/

void Light::AddShadowMap(ShadowMap3D *AddMe)
{
ShadowMap3D **CurShadowPtr = &Shadows;

while (*CurShadowPtr)
	{
	CurShadowPtr = &(*CurShadowPtr)->Next;
	} // while

*CurShadowPtr = AddMe;

} // Light::AddShadowMap

/*===========================================================================*/

void Light::RemoveShadowMap(ShadowMap3D *RemoveMe)
{
ShadowMap3D *PrevShadow = NULL, *CurShadow = Shadows;

while (CurShadow)
	{
	if (CurShadow == RemoveMe)
		{
		if (PrevShadow)
			{
			PrevShadow->Next = CurShadow->Next;
			} // if
		else
			{
			Shadows = CurShadow->Next;
			} // else
		delete RemoveMe;
		return;
		} // if
	PrevShadow = CurShadow;
	CurShadow = CurShadow->Next;
	} // while

} // Light::RemoveShadowMap

/*===========================================================================*/

int Light::AttemptLoadShadowMapFile(ShadowMap3D *Map, long MapWidth, GeneralEffect *MapObj, Joe *MapVec, 
	VectorPoint *MapVtx, long MapVtxNum, char MapType)
{
char TempStr[32], TempFileName[256];
ShadowMap3D *TempMap;

TempFileName[0] = 0;

strcpy(TempFileName, Name);	// this is the light name, most useful if there are multiple lights
if (MapObj)
	strcat(TempFileName, MapObj->Name);	// this is the effect name, most useful if there are multiple effects
if (MapVec)	// vector name
	strcat(TempFileName, MapVec->Name() ? MapVec->Name(): MapVec->FileName() ? MapVec->FileName(): "NoVecName");
if (MapVtx)
	{
	sprintf(TempStr, "%1d", MapVtxNum);
	strcat(TempFileName, TempStr);	// different number for each vertex
	} // if
switch (MapType)
	{
	case WCS_SHADOWTYPE_TERRAIN:
		{
		strcat(TempFileName, "T");
		break;
		} // 
	case WCS_SHADOWTYPE_FOLIAGE:
		{
		strcat(TempFileName, "F");
		break;
		} // 
	case WCS_SHADOWTYPE_CLOUDSM:
		{
		strcat(TempFileName, "C");
		break;
		} // 
	case WCS_SHADOWTYPE_3DOBJECT:
		{
		strcat(TempFileName, "O");
		break;
		} // 
	} // switch

if (TempFileName[0])
	{
	if (TempMap = new ShadowMap3D())
		{
		if (TempMap->Rast = new Raster())
			{
			if (TempMap->Rast->LoadShadow3D(TempFileName, TempMap))
				{
				if ((TempMap->Rast->Cols == MapWidth) && 
					(fabs(TempMap->VP.XYZ[0] - Map->VP.XYZ[0]) < .01) && // one cm accuracy is close enough. It was found that values 
					(fabs(TempMap->VP.XYZ[1] - Map->VP.XYZ[1]) < .01) && // loaded from the file sometimes didn't match in 
					(fabs(TempMap->VP.XYZ[2] - Map->VP.XYZ[2]) < .01))	// the insignificant digits beyond what the debugger displayed
					{
					TempMap->MyEffect = MapObj;
					TempMap->MyJoe = MapVec;
					TempMap->MyVertex = MapVtx;
					// since the values in this variable changed late in the V6 beta cycle, 
					// we need to force the value to the one desired
					TempMap->MyType = MapType;
					AddShadowMap(TempMap);
					return (1);
					} // if
				} // if
			} // if
		delete TempMap;
		} // if
	} // if

return (0);

} // Light::AttemptLoadShadowMapFile

/*===========================================================================*/

int Light::SaveShadowMapFile(ShadowMap3D *Map, GeneralEffect *MapObj, Joe *MapVec, 
	VectorPoint *MapVtx, long MapVtxNum, char MapType)
{
char TempStr[32], TempFileName[256];

TempFileName[0] = 0;

strcpy(TempFileName, Name);	// this is the light name, most useful if there are multiple lights
if (MapObj)
	strcat(TempFileName, MapObj->Name);	// this is the effect name, most useful if there are multiple effects
if (MapVec)	// vector name
	strcat(TempFileName, MapVec->Name() ? MapVec->Name(): MapVec->FileName() ? MapVec->FileName(): "NoVecName");
if (MapVtx)
	{
	sprintf(TempStr, "%1d", MapVtxNum);
	strcat(TempFileName, TempStr);	// different number for each vertex
	} // if
switch (MapType)
	{
	case WCS_SHADOWTYPE_TERRAIN:
		{
		strcat(TempFileName, "T");
		break;
		} // 
	case WCS_SHADOWTYPE_FOLIAGE:
		{
		strcat(TempFileName, "F");
		break;
		} // 
	case WCS_SHADOWTYPE_CLOUDSM:
		{
		strcat(TempFileName, "C");
		break;
		} // 
	case WCS_SHADOWTYPE_3DOBJECT:
		{
		strcat(TempFileName, "O");
		break;
		} // 
	} // switch

if (TempFileName[0])
	{
	return (Map->Rast->SaveShadow3D(TempFileName, Map));
	} // if

return (0);

} // Light::SaveShadowMapFile

/*===========================================================================*/

void Light::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double ShiftWE, ShiftNS, TempVal;
GraphNode *CurNode;

ShiftWE = ((CurBounds->West + CurBounds->East) - (OldBounds->West - OldBounds->East)) * 0.5;
ShiftNS = ((CurBounds->North + CurBounds->South) - (OldBounds->North + OldBounds->South)) * 0.5;

AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetValue(
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue + ShiftNS);
if (CurNode = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetFirstNode(0))
	{
	TempVal = CurNode->GetValue() + ShiftNS;
	if (TempVal > 90.0)
		TempVal = 90.0;
	if (TempVal < -90.0)
		TempVal = -90.0;
	CurNode->SetValue(TempVal);
	while (CurNode = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetNextNode(0, CurNode))
		{
		TempVal = CurNode->GetValue() + ShiftNS;
		if (TempVal > 90.0)
			TempVal = 90.0;
		if (TempVal < -90.0)
			TempVal = -90.0;
		CurNode->SetValue(TempVal);
		} // while
	} // if
AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetValue(
	AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue + ShiftWE);
if (CurNode = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetFirstNode(0))
	{
	CurNode->SetValue(CurNode->GetValue() + ShiftWE);
	while (CurNode = AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetNextNode(0, CurNode))
		{
		CurNode->SetValue(CurNode->GetValue() + ShiftWE);
		} // while
	} // if

} // Light::ScaleToDEMBounds

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int Light::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
Light *CurrentLight = NULL;
DEMBounds OldBounds, CurBounds;

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	if (LoadToImages = new ImageLib())
		{
		// set some global pointers so that things know what libraries to link to
		GlobalApp->LoadToEffectsLib = LoadToEffects;
		GlobalApp->LoadToImageLib = LoadToImages;

		while (BytesRead && Success)
			{
			// read block descriptor tag from file 
			if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
				WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
				{
				TotalRead += BytesRead;
				ReadBuf[8] = 0;
				// read block size from file 
				if (BytesRead = ReadBlock(ffile, (char *)&Size,
					WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
					{
					TotalRead += BytesRead;
					BytesRead = 0;
					if (! strnicmp(ReadBuf, "DEMBnds", 8))
						{
						if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
							OldBoundsLoaded = 1;
						} // if material
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if Images
					else if (! strnicmp(ReadBuf, "Light", 8))
						{
						if (CurrentLight = new Light(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentLight->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if light
					else if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
					TotalRead += BytesRead;
					if (BytesRead != Size)
						{
						Success = 0;
						break;
						} // if error
					} // if size block read 
				else
					break;
				} // if tag block read 
			else
				break;
			} // while 
		} // if image lib
	else
		Success = 0;
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentLight)
	{
	if (EffectsLib::LoadQueries && OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load Light", "Do you wish the loaded Light's position\n to be scaled to current DEM bounds?"))
			{
			CurrentLight->ScaleToDEMBounds(&OldBounds, &CurBounds);
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentLight);
	strcpy(ReadBuf, Name);
	SetUniqueName(GlobalApp->AppEffects, ReadBuf);
	} // if

if (LoadToEffects)
	delete LoadToEffects;
if (LoadToImages)
	delete LoadToImages;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->CopyFromImageLib = GlobalApp->AppImages;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToImageLib = GlobalApp->AppImages;

return (Success);

} // Light::LoadObject

/*===========================================================================*/

int Light::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
DEMBounds CurBounds;

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	strcpy(StrBuf, "DEMBnds");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurBounds.Save(ffile))
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
				} // if dem bounds saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if dem bounds

// Images
GlobalApp->AppImages->ClearRasterIDs();
if (InitImageIDs(ImageID))
	{
	strcpy(StrBuf, "Images");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = GlobalApp->AppImages->Save(ffile, NULL, TRUE))
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
				} // if Images saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if images

// Light
strcpy(StrBuf, "Light");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Save(ffile))
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
			} // if Light saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

return (TotalWritten);

WriteError:

return (0);

} // Light::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::Light_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
Light *Current;

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
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new Light(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (Light *)FindDuplicateByName(Current->EffectType, Current))
								{
								RemoveRAHost(Current, 1);
								Current = NULL;
								} // if
							}
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

} // EffectsLib::Light_Load()

/*===========================================================================*/

ULONG EffectsLib::Light_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
Light *Current;

Current = Lights;
while (Current)
	{
	if (! Current->TemplateItem)
		{
		ItemTag = WCS_EFFECTSBASE_DATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Current->Save(ffile))
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
					} // if Light saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (Light *)Current->Next;
	} // while

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

} // EffectsLib::Light_Save()

/*===========================================================================*/
