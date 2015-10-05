// MaterialStrata.cpp
// For managing WCS Material Strata
// Built from scratch on 06/23/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "Useful.h"
#include "Application.h"
#include "EffectsLib.h"
#include "EffectsIO.h"
#include "Conservatory.h"
#include "DragnDropListGUI.h"
#include "requester.h"
#include "Toolbar.h"
#include "MaterialStrataEditGUI.h"
#include "AppMem.h"
#include "Database.h"
#include "Raster.h"
#include "Security.h"
#include "Lists.h"

unsigned char *MaterialStrata::StrataNoiseMap;

MaterialStrata::MaterialStrata(void)
: RasterAnimHost(NULL)
{

SetDefaults();

} // MaterialStrata::MaterialStrata

/*===========================================================================*/

MaterialStrata::MaterialStrata(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{

SetDefaults();

} // MaterialStrata::MaterialStrata

/*===========================================================================*/

void MaterialStrata::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_MATERIALSTRATA_NUMANIMPAR] = {100.0, 0.0, 0.0, 1.0};
double RangeDefaults[WCS_EFFECTS_MATERIALSTRATA_NUMANIMPAR][3] = {FLT_MAX, -FLT_MAX, 1.0,	// deformation scale
														FLT_MAX, -FLT_MAX, 1.0,				// north dip
														FLT_MAX, -FLT_MAX, 1.0,				// west dip
														5.0, -5.0, .01};					// bump intensity
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
Enabled = 1;
ColorStrata = BumpLines = 0;
LinesEnabled = 1;
StrataColor[0].SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR);
StrataColor[1].SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR + 1);
StrataColor[2].SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR + 2);
StrataColor[3].SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR + 3);
PixelTexturesExist = 0;

AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_BUMPINTENSITY].SetMultiplier(100.0);

} // MaterialStrata::SetDefaults

/*===========================================================================*/

MaterialStrata::~MaterialStrata()
{
RootTexture *DelTex;
long Ct;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->MSG && GlobalApp->GUIWins->MSG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->MSG;
		GlobalApp->GUIWins->MSG = NULL;
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

} // MaterialStrata::~MaterialStrata

/*===========================================================================*/

void MaterialStrata::Copy(MaterialStrata *CopyTo, MaterialStrata *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	} // for
for (Ct = 0; Ct < 4; Ct ++)
	{
	StrataColor[Ct].Copy(&CopyTo->StrataColor[Ct], &CopyFrom->StrataColor[Ct]);
	} // for
/*
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (CopyTo->TexRoot[Ct])
		{
		delete CopyTo->TexRoot[Ct];		// removes links to images
		CopyTo->TexRoot[Ct] = NULL;
		} // if
	if (CopyFrom->TexRoot[Ct])
		{
		if (CopyTo->TexRoot[Ct] = new RootTexture(CopyTo, CopyFrom->TexRoot[Ct]->ApplyToEcosys, CopyFrom->TexRoot[Ct]->ApplyToColor, CopyFrom->TexRoot[Ct]->ApplyToDisplace))
			{
			CopyTo->TexRoot[Ct]->Copy(CopyTo->TexRoot[Ct], CopyFrom->TexRoot[Ct]);
			} // if
		} // if
	} // for
*/
CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->ColorStrata = CopyFrom->ColorStrata;
CopyTo->BumpLines = CopyFrom->BumpLines;
CopyTo->LinesEnabled = CopyFrom->LinesEnabled;
RootTextureParent::Copy(CopyTo, CopyFrom);
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // MaterialStrata::Copy

/*===========================================================================*/

ULONG MaterialStrata::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TexRootNumber = -1;

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
					default:
						break;
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
					case WCS_EFFECTS_MATERIALSTRATA_DEFORMSCALE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_DEFORMSCALE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_NORTHDIP:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_NORTHDIP].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_WESTDIP:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_WESTDIP].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_BUMPINTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_BUMPINTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_COLOR1:
						{
						BytesRead = StrataColor[0].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_COLOR2:
						{
						BytesRead = StrataColor[1].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_COLOR3:
						{
						BytesRead = StrataColor[2].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_COLOR4:
						{
						BytesRead = StrataColor[3].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_COLORSTRATA:
						{
						BytesRead = ReadBlock(ffile, (char *)&ColorStrata, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_LINESENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&LinesEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_BUMPLINES:
						{
						BytesRead = ReadBlock(ffile, (char *)&BumpLines, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							{
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_TEXDEFORMATION:
						{
						if (TexRoot[WCS_EFFECTS_MATERIALSTRATA_TEXTURE_DEFORMATION] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIALSTRATA_TEXTURE_DEFORMATION]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIALSTRATA_TEXBUMPINTENSITY:
						{
						if (TexRoot[WCS_EFFECTS_MATERIALSTRATA_TEXTURE_BUMPINTENSITY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIALSTRATA_TEXTURE_BUMPINTENSITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
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

} // MaterialStrata::Load

/*===========================================================================*/

unsigned long int MaterialStrata::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_MATERIALSTRATA_NUMANIMPAR] = {WCS_EFFECTS_MATERIALSTRATA_DEFORMSCALE, 
																WCS_EFFECTS_MATERIALSTRATA_NORTHDIP,
																WCS_EFFECTS_MATERIALSTRATA_WESTDIP,
																WCS_EFFECTS_MATERIALSTRATA_BUMPINTENSITY};
unsigned long int TextureItemTag[WCS_EFFECTS_MATERIALSTRATA_NUMTEXTURES] = {WCS_EFFECTS_MATERIALSTRATA_TEXDEFORMATION,
																			WCS_EFFECTS_MATERIALSTRATA_TEXBUMPINTENSITY};
unsigned long int ColorItemTag[4] = {WCS_EFFECTS_MATERIALSTRATA_COLOR1, 
									WCS_EFFECTS_MATERIALSTRATA_COLOR2,
									WCS_EFFECTS_MATERIALSTRATA_COLOR3,
									WCS_EFFECTS_MATERIALSTRATA_COLOR4};

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MATERIALSTRATA_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MATERIALSTRATA_COLORSTRATA, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ColorStrata)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MATERIALSTRATA_LINESENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LinesEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MATERIALSTRATA_BUMPLINES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BumpLines)) == NULL)
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

for (Ct = 0; Ct < 4; Ct ++)
	{
	ItemTag = ColorItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = StrataColor[Ct].Save(ffile))
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
				} /* if strata color saved */
			else
				goto WriteError;
			} /* if size written */
		else
			goto WriteError;
		} /* if tag written */
	else
		goto WriteError;
	} // for

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
					} // if anim param saved
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

} // MaterialStrata::Save

/*===========================================================================*/

static char *MaterialStrataCritterNames[WCS_EFFECTS_MATERIALSTRATA_NUMANIMPAR] = {"Deformation (m)", "North Dip (m/deg)", "West Dip (m/deg)", "Bump Intensity (%)"};
static char *MaterialStrataTextureNames[WCS_EFFECTS_MATERIALSTRATA_NUMTEXTURES] = {"Deformation (%)", "Bump Intensity (%)"};

char *MaterialStrata::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (MaterialStrataCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (MaterialStrataTextureNames[Ct]);
		} // if
	} // for
if (Test == &StrataColor[0])
	return ("Strata Color 1");
if (Test == &StrataColor[1])
	return ("Strata Color 2");
if (Test == &StrataColor[2])
	return ("Strata Color 3");
if (Test == &StrataColor[3])
	return ("Strata Color 4");

return ("");

} // MaterialStrata::GetCritterName

/*===========================================================================*/

void MaterialStrata::EditRAHost(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->MSG)
	{
	delete GlobalApp->GUIWins->MSG;
	}
GlobalApp->GUIWins->MSG = new MaterialStrataEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->MSG)
	{
	GlobalApp->GUIWins->MSG->Open(GlobalApp->MainProj);
	}

} // MaterialStrata::EditRAHost

/*===========================================================================*/

char *MaterialStrata::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? MaterialStrataTextureNames[TexNumber]: (char*)"");

} // MaterialStrata::GetTextureName

/*===========================================================================*/

RootTexture *MaterialStrata::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // MaterialStrata::NewRootTexture

/*===========================================================================*/

char *MaterialStrata::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Strata Texture! Remove anyway?");

} // MaterialStrata::OKRemoveRaster

/*===========================================================================*/

int MaterialStrata::RemoveRAHost(RasterAnimHost *RemoveMe)
{
NotifyTag Changes[2];
int Removed = 0;
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (RemoveMe == GetTexRootPtr(Ct))
		{
		delete GetTexRootPtr(Ct);
		SetTexRootPtr(Ct, NULL);
		Removed = 1;
		} // if
	} // for

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // MaterialStrata::RemoveRAHost

/*===========================================================================*/

int MaterialStrata::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < 4; Ct ++)
	{
	if (StrataColor[Ct].SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for

return (Found);

} // MaterialStrata::SetToTime

/*===========================================================================*/

long MaterialStrata::InitImageIDs(long &ImageID)
{
long NumImages = 0;
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		NumImages += GetTexRootPtr(Ct)->InitImageIDs(ImageID);
		} // if
	} // for

return (NumImages);

} // MaterialStrata::InitImageIDs

/*===========================================================================*/

int MaterialStrata::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
for (Ct = 0; Ct < 4; Ct ++)
	{
	if (StrataColor[Ct].GetRAHostAnimated())
		return (1);
	} // for

return (0);

} // MaterialStrata::GetRAHostAnimated

/*===========================================================================*/

long MaterialStrata::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Ct, Found = 0;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < 4; Ct ++)
	{
	if (StrataColor[Ct].GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for

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

} // MaterialStrata::GetKeyFrameRange

/*===========================================================================*/

char MaterialStrata::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
if (GetNumTextures() > 0 && (DropType == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropType == WCS_RAHOST_OBJTYPE_TEXTURE))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	return (1);

return (0);

} // MaterialStrata::GetRAHostDropOK

/*===========================================================================*/

int MaterialStrata::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
RasterAnimHost *TargetList[30];
int Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
char QueryStr[256], NameStr[128], WinNum;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (MaterialStrata *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (MaterialStrata *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = -1;
	for (Ct = 0; Ct < 4; Ct ++)
		{
		TargetList[Ct] = &StrataColor[Ct];
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		TargetList[Ct] = GetTexRootPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, NULL, NULL);
		if(GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // MaterialStrata::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long MaterialStrata::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_MATERIALSTRATA | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // MaterialStrata::GetRAFlags

/*===========================================================================*/

void MaterialStrata::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = "";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = EffectsLib::DefaultPaths[WCS_EFFECTSSUBCLASS_GROUND];
	Prop->Ext = "mst";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // MaterialStrata::GetRAHostProperties

/*===========================================================================*/

int MaterialStrata::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		Enabled = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return(LoadObject(Prop->fFile, 0, Prop->ByteFlip));
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return(SaveObject(Prop->fFile));
	} // if

return (Success);

} // MaterialStrata::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *MaterialStrata::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	Found = 1;
for (Ct = 0; Ct < 4; Ct ++)
	{
	if (Found)
		return (&StrataColor[Ct]);
	if (Current == &StrataColor[Ct])
		Found = 1;
	} // for
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

return (NULL);

} // MaterialStrata::GetRAHostChild

/*===========================================================================*/

int MaterialStrata::GetDeletable(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for

return (0);

} // MaterialStrata::GetDeletable

/*===========================================================================*/

int MaterialStrata::BuildFileComponentsList(EffectList **Coords)
{
#ifdef WCS_BUILD_VNS
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		if (! GetTexRootPtr(Ct)->BuildFileComponentsList(Coords))
			return (0);
		} // if
	} // for
#endif // WCS_BUILD_VNS

return (1);

} // MaterialStrata::BuildFileComponentsList

/*===========================================================================*/

int MaterialStrata::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil)
{
long Ct;

AnimAffil = NULL;
TexAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			switch (Ct)
				{
				case WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_BUMPINTENSITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIALSTRATA_TEXTURE_BUMPINTENSITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_DEFORMSCALE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIALSTRATA_TEXTURE_DEFORMATION);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	for (Ct = 0; Ct < 4; Ct ++)
		{
		if (ChildA == &StrataColor[Ct])
			{
			AnimAffil = (AnimCritter *)ChildA;
			return (1);
			} // if
		} // for
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
				case WCS_EFFECTS_MATERIALSTRATA_TEXTURE_BUMPINTENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_BUMPINTENSITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIALSTRATA_TEXTURE_DEFORMATION:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_DEFORMSCALE);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // MaterialStrata::GetAffiliates

/*===========================================================================*/

int MaterialStrata::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, TexAffil, NULL));
	} // if

return (0);

} // MaterialStrata::GetPopClassFlags

/*===========================================================================*/

int MaterialStrata::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long int MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, NULL));
	} // if

return(0);

} // MaterialStrata::AddSRAHBasePopMenus

/*===========================================================================*/

int MaterialStrata::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, TexAffil, NULL, this, NULL));
	} // if

return(0);

} // MaterialStrata::HandleSRAHPopMenuSelection

/*===========================================================================*/

int MaterialStrata::InitToRender(void)
{
char Ct;

PixelTexturesExist = 0;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->Enabled)
		{
		if (! TexRoot[Ct]->InitAAChain())
			{
			return (0);
			} // if
		PixelTexturesExist = 1;
		} // if
	} // for

if (! StrataNoiseMap)
	{
	if (! InitNoise())
		return (0);
	} // if

return (1);

} // MaterialStrata::InitToRender

/*===========================================================================*/

long MaterialStrata::MakeNoise(long MaxNoise, double Lat, double Lon)
{
double Noise, LonOff, LatOff, LonInvOff, LatInvOff, wt[4], val[4];
long Noisy, Col, Row, Colp1, Rowp1;

Lat -= ((int)Lat);
Lon -= ((int)Lon);
Lat *= 256.0;
Lon *= 256.0;
Col = quickftol(Lon);
Row = quickftol(Lat);

Colp1 = Col < 255 ? Col + 1: 0;
Rowp1 = Row < 255 ? Row + 1: 0;
LatOff = Lat - Row;
LonOff = Lon - Col;
LatInvOff = 1.0 - LatOff;
LonInvOff = 1.0 - LonOff;

wt[0] = LatInvOff * LonInvOff;
val[0] = StrataNoiseMap[Row * 256 + Col];
wt[1] = LatOff * LonInvOff;
val[1] = StrataNoiseMap[Row * 256 + Colp1];
wt[2] = LatOff * LonOff;
val[2] = StrataNoiseMap[Rowp1 * 256 + Colp1];
wt[3] = LonOff * LatInvOff;
val[3] = StrataNoiseMap[Rowp1 * 256 + Col];
Noise = (wt[0] * val[0] + wt[1] * val[1] + wt[2] * val[2] + wt[3] * val[3]);

Noisy = quickftol((Noise * MaxNoise) / 255.0);

return (Noisy);
 
} // MaterialStrata::MakeNoise() 

/*===========================================================================*/

unsigned char *MaterialStrata::InitNoise(void)
{
long Ct;

if (StrataNoiseMap = (unsigned char *)AppMem_Alloc(65536, 0))
	{
	Rand.Seed64(1111, 9999);
	for (Ct = 0; Ct < 65536; Ct ++)
		StrataNoiseMap[Ct] = (unsigned char)(127.5 + Rand.GenGauss() * 127.49999);
	} // if noise map created 

return (StrataNoiseMap);

} // MaterialStrata::InitNoise

/*===========================================================================*/

void MaterialStrata::FreeNoiseMap(void)
{

if (StrataNoiseMap)
	AppMem_Free(StrataNoiseMap, 65536);
StrataNoiseMap = NULL;

} // MaterialStrata::FreeNoiseMap

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int MaterialStrata::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
DEMBounds OldBounds, CurBounds;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
MaterialStrata *CurrentStrata = NULL;
CoordSys *CurrentCoordSys = NULL;
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];

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
						} // if DEMBnds
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if Images
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoordSys->Load(ffile, Size, ByteFlip);
							}
						} // if CoordSys
					else if (! strnicmp(ReadBuf, "Strata", 8))
						{
						if (CurrentStrata = new MaterialStrata(NULL))
							{
							if ((BytesRead = CurrentStrata->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Strata
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

if (Success == 1 && CurrentStrata)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentStrata);
	delete CurrentStrata;
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

} // MaterialStrata::LoadObject

/*===========================================================================*/

int MaterialStrata::SaveObject(FILE *ffile)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Coords = NULL;
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

if (BuildFileComponentsList(&Coords))
	{
	#ifdef WCS_BUILD_VNS
	CurEffect = Coords;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "CoordSys");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((CoordSys *)CurEffect->Me)->Save(ffile))
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
						} // if CoordSys saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while
	#endif // WCS_BUILD_VNS

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	} // if

// MaterialStrata
strcpy(StrBuf, "Strata");
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
			} // if MaterialStrata saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // MaterialStrata::SaveObject
