// NVParsing.cpp
// Code to read and parse NVW scene files
// mostly broken off of main.cpp on 1/31/06 by CXH

#include <string.h>
#include <locale>
#include <direct.h>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

#include "IdentityDefines.h"
#include "NVScene.h"
#include "NVParsing.h"
#include "NVAnimObject.h"
#include "ScriptArg.h"
#include "BusyWin.h"
#include "Useful.h"
#include "DecompressZipScene.h"
#include "RequesterBasic.h"
#include "NVSigCheck.h"

extern NVScene MasterScene;


bool ParseBooleanPresenceIsTrue(char *InputString)
{
if(InputString) // if NULL
	{
	if(InputString[0])
		{
		if(!strcmp(InputString, "1"))
			{
			return(true);
			} // if
		else if(!strcmp(InputString, "0"))
			{
			return(false);
			} // else if
		else if(!stricmp(InputString, "yes"))
			{
			return(true);
			} // else if
		else if(!stricmp(InputString, "no"))
			{
			return(false);
			} // else if
		else if(!stricmp(InputString, "true"))
			{
			return(true);
			} // else if
		else if(!stricmp(InputString, "false"))
			{
			return(false);
			} // else if
		return(false); // some text, but it isn't recognisible as boolean
		} // if
	} // if
return(true); // assume true for mere presence of tag
} // ParseBooleanPresenceIsTrue


osg::Vec4 DecodeHTMLHexColor(char *InputText)
{ // turns something like #F7E7CE into 247,231,206 and thence into 0.968627,0.905882,0.807843
osg::Vec4 Color;
unsigned char RedGun = 0, GrnGun = 0, BluGun = 0;
float Red, Green, Blue;

RedGun |= HexToDecDigit(InputText[0]) << 4;
RedGun |= HexToDecDigit(InputText[1]);
Red = (float)RedGun / 255.0f;
GrnGun |= HexToDecDigit(InputText[2]) << 4;
GrnGun |= HexToDecDigit(InputText[3]);
Green = (float)GrnGun / 255.0f;
BluGun |= HexToDecDigit(InputText[4]) << 4;
BluGun |= HexToDecDigit(InputText[5]);
Blue = (float)BluGun / 255.0f;

Color.set(Red, Green, Blue, 1.0);


return(Color);
} // DecodeHTMLHexColor

int HexToDecDigit(unsigned char InputDigit)
{
int OutValue = 0;

if(isxdigit(InputDigit))
	{
	if(isdigit(InputDigit))
		{
		OutValue = InputDigit - '0';
		} // if
	else
		{
		InputDigit = tolower(InputDigit);
		OutValue = (InputDigit - 'a') + 10;
		} // else
	} // if

return(OutValue);
} // HexToDecDigit


osg::Vec4 DecodeColor(char *InputText)
{ // turns something like #F7E7CE into 247,231,206 and thence into 0.968627,0.905882,0.807843
osg::Vec4 Color;

if(InputText && InputText[0] && InputText[0] == '#' && strlen(InputText) == 7)
	{
	return(DecodeHTMLHexColor(&InputText[1]));
	} // if
// <<<>>> else handle other color notations

return(Color);
} // DecodeColor




// we can't have a subtag that conflicts with a supertag, since we're very
// liberal about allowing the omission of the closing tag, and interpreting
// a subsequent opening supertag as the de-facto closing tag for the previous
// supertag

char *TagRipTemplate = "VERSION SPLASH DEM INSTANCEFILE OBJFILE OBJINSTANCE LIGHT SKY OCEAN NAVIGATION CAMERA OVERLAY WATERMARK HAZE META LOD ACTION NOTICE SIGNATURE KEY";
enum
	{
	TAG_NONE = -1,
	TAG_VERSION = 0,
	TAG_SPLASH,
	TAG_DEM,
	TAG_INSTANCEFILE,
	TAG_OBJFILE,
	TAG_OBJINSTANCE,
	TAG_LIGHT,
	TAG_SKY,
	TAG_OCEAN,
	TAG_NAVIGATION,
	TAG_CAMERA,
	TAG_OVERLAY,
	TAG_WATERMARK,
	TAG_HAZE,
	TAG_META,
	TAG_LOD,
	TAG_ACTION,
	TAG_NOTICE,
	TAG_SIGNATURE,
	TAG_KEY, // actually a subtag
	TAG_MAX
	}; // TagIDs

// potentially animated items
// SIZE is a synonym for SCALEX for isometrically-sized items
char *ALLKEYSubTagRipTemplate = "TIME ELEVATION Z LATITUDE Y LONGITUDE X TARGETELEVATION TARGETZ TARGETLATITUDE TARGETY TARGETLONGITUDE TARGETX HFOV EDGE BANK SIZE SCALEX SCALEY SCALEZ HEADING PITCH QI QJ QK QW AXISX AXISY AXISZ AXISANGLE FILENAME SPECULARITY REFLECTIVITY REFLECTIONMAPFILE SIG REFMAPSIG START RANGE STARTINTENSITY ENDINTENSITY COLOR FALLOFFEXP";

enum
	{
	KEYSUBTAG_NONE = -1,
	KEYSUBTAG_TIME = 0, // don't move this one
	KEYSUBTAG_ELEVATION, // or this one
	KEYSUBTAG_Z,
	KEYSUBTAG_LATITUDE,
	KEYSUBTAG_Y,
	KEYSUBTAG_LONGITUDE,
	KEYSUBTAG_X,
	KEYSUBTAG_TARGETELEVATION,
	KEYSUBTAG_TARGETZ,
	KEYSUBTAG_TARGETLATITUDE,
	KEYSUBTAG_TARGETY,
	KEYSUBTAG_TARGETLONGITUDE,
	KEYSUBTAG_TARGETX,
	KEYSUBTAG_HFOV,
	KEYSUBTAG_EDGE, // used for spot lights, stored in BANK (below)
	KEYSUBTAG_BANK,
	KEYSUBTAG_SIZE,
	KEYSUBTAG_SCALEX,
	KEYSUBTAG_SCALEY,
	KEYSUBTAG_SCALEZ,
	KEYSUBTAG_HEADING,
	KEYSUBTAG_PITCH,
	KEYSUBTAG_QI,
	KEYSUBTAG_QJ,
	KEYSUBTAG_QK,
	KEYSUBTAG_QW,
	KEYSUBTAG_AXISX,
	KEYSUBTAG_AXISY,
	KEYSUBTAG_AXISZ,
	KEYSUBTAG_AXISANGLE,
	KEYSUBTAG_FILENAME,
	KEYSUBTAG_SPECULARITY,
	KEYSUBTAG_REFLECTIVITY,
	KEYSUBTAG_REFLECTIONMAPFILE,
	KEYSUBTAG_SIG,
	KEYSUBTAG_REFMAPSIG,
	KEYSUBTAG_START,
	KEYSUBTAG_RANGE,
	KEYSUBTAG_STARTINTENSITY,
	KEYSUBTAG_ENDINTENSITY,
	KEYSUBTAG_COLOR,
	KEYSUBTAG_FALLOFFEXP,
	KEYSUBTAG_MAX
	}; // TagIDs


// this maps casual relations between keywords and canonical channels
// oddball ones have CANON_NONE in them for special handling
int KeySubTagIndex[KEYSUBTAG_MAX] =
	{
	/* KEYSUBTAG_TIME */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_ELEVATION */ NVAnimObject::CANONZ,
	/* KEYSUBTAG_Z */ NVAnimObject::CANONZ,
	/* KEYSUBTAG_LATITUDE */ NVAnimObject::CANONY,
	/* KEYSUBTAG_Y */ NVAnimObject::CANONY,
	/* KEYSUBTAG_LONGITUDE */ NVAnimObject::CANONX,
	/* KEYSUBTAG_X */ NVAnimObject::CANONX,
	/* KEYSUBTAG_TARGETELEVATION */ NVAnimObject::CANONAUXZ,
	/* KEYSUBTAG_TARGETZ */ NVAnimObject::CANONAUXZ,
	/* KEYSUBTAG_TARGETLATITUDE */ NVAnimObject::CANONAUXY,
	/* KEYSUBTAG_TARGETY */ NVAnimObject::CANONAUXY,
	/* KEYSUBTAG_TARGETLONGITUDE */ NVAnimObject::CANONAUXX,
	/* KEYSUBTAG_TARGETX */ NVAnimObject::CANONAUXX,
	/* KEYSUBTAG_HFOV */ NVAnimObject::CANONHANGLE,
	/* KEYSUBTAG_EDGE */ NVAnimObject::CANONB,
	/* KEYSUBTAG_BANK */ NVAnimObject::CANONB,
	/* KEYSUBTAG_SIZE */ NVAnimObject::CANONAUXX,
	/* KEYSUBTAG_SCALEX */ NVAnimObject::CANONAUXX,
	/* KEYSUBTAG_SCALEY */ NVAnimObject::CANONAUXY,
	/* KEYSUBTAG_SCALEZ */ NVAnimObject::CANONAUXZ,
	/* KEYSUBTAG_HEADING */ NVAnimObject::CANONH,
	/* KEYSUBTAG_PITCH */ NVAnimObject::CANONP,
	/* KEYSUBTAG_QI */ NVAnimObject::CANONQI,
	/* KEYSUBTAG_QJ */ NVAnimObject::CANONQJ,
	/* KEYSUBTAG_QK */ NVAnimObject::CANONQK,
	/* KEYSUBTAG_QW */ NVAnimObject::CANONQW,
	/* KEYSUBTAG_AXISX */ NVAnimObject::CANONAXISX,
	/* KEYSUBTAG_AXISY */ NVAnimObject::CANONAXISY,
	/* KEYSUBTAG_AXISZ */ NVAnimObject::CANONAXISZ,
	/* KEYSUBTAG_AXISANGLE */ NVAnimObject::CANONB,
	/* KEYSUBTAG_FILENAME */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_SPECULARITY */ NVAnimObject::CANONINTENSITYA,
	/* KEYSUBTAG_REFLECTIVITY */ NVAnimObject::CANONINTENSITYB,
	/* KEYSUBTAG_REFLECTIONMAPFILE */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_SIG */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_REFMAPSIG */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_START */ NVAnimObject::CANONDISTANCEA,
	/* KEYSUBTAG_RANGE */ NVAnimObject::CANONDISTANCEB,
	/* KEYSUBTAG_STARTINTENSITY */ NVAnimObject::CANONINTENSITYA,
	/* KEYSUBTAG_ENDINTENSITY */ NVAnimObject::CANONINTENSITYB,
	/* KEYSUBTAG_COLOR */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_FALLOFFEXP */ NVAnimObject::CANONINTENSITYB
	};

// this maps casual relations between keywords and canonical strings
// non-string entries have CANON_NONE in them
int KeySubTagStringIndex[KEYSUBTAG_MAX] =
	{
	/* KEYSUBTAG_TIME */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_ELEVATION */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_Z */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_LATITUDE */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_Y */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_LONGITUDE */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_X */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_TARGETELEVATION */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_TARGETZ */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_TARGETLATITUDE */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_TARGETY */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_TARGETLONGITUDE */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_TARGETX */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_HFOV */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_EDGE */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_BANK */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_SIZE */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_SCALEX */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_SCALEY */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_SCALEZ */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_HEADING */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_PITCH */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_QI */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_QJ */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_QK */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_QW */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_AXISX */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_AXISY */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_AXISZ */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_AXISANGLE */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_FILENAME */ NVAnimObject::STRING_FILEA,
	/* KEYSUBTAG_SPECULARITY */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_REFLECTIVITY */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_REFLECTIONMAPFILE */ NVAnimObject::STRING_FILEB,
	/* KEYSUBTAG_SIG */ NVAnimObject::STRING_SIGA,
	/* KEYSUBTAG_REFMAPSIG */ NVAnimObject::STRING_SIGB,
	/* KEYSUBTAG_START */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_RANGE */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_STARTINTENSITY */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_ENDINTENSITY */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_COLOR */ NVAnimObject::CANON_NONE,
	/* KEYSUBTAG_FALLOFFEXP */ NVAnimObject::CANON_NONE
	};


int ParseKeySubTag(NVAnimObject *CurAnimSubObject, char *Result)
{
int Success = 0;
int NumTagArgsParsed;

if(CurAnimSubObject) // no point in parsing if we don't know where it'll go
	{
	ArgRipper KEYTag(ALLKEYSubTagRipTemplate);

	if(NumTagArgsParsed = KEYTag.Rip(Result))
		{
		MiniKeyFrame *KF;
		if(KF = new MiniKeyFrame)
			{
			int Where = 0;
			// this logic design only handles first occurrance of a subtag

			// handle TIME
			if(Result = KEYTag.GetArg(KEYSUBTAG_TIME, Where))
				{
				KF->SetTimeValue(atof(Result));
				} // if

			// next try the easy value mapping of our table
			for(int TableID = KEYSUBTAG_ELEVATION; TableID < KEYSUBTAG_MAX; TableID++)
				{
				Where = 0;
				if(Result = KEYTag.GetArg(TableID, Where))
					{
					if(KeySubTagIndex[TableID] != NVAnimObject::CANON_NONE)
						{
						KF->SetChannelValue(KeySubTagIndex[TableID], atof(Result));
						} // if
					// try string mapping while we're here
					else if(KeySubTagStringIndex[TableID] != NVAnimObject::CANON_NONE)
						{
						KF->SetString(KeySubTagStringIndex[TableID], Result);
						} // else
					} // if
				} // for

			// now handle the oddball entities

			// COLOR
			Where = 0;
			if(Result = KEYTag.GetArg(KEYSUBTAG_COLOR, Where))
				{
				osg::Vec4 Color = DecodeColor(Result);
				KF->SetChannelValue(NVAnimObject::CANONCOLORR, Color.x()); //R
				KF->SetChannelValue(NVAnimObject::CANONCOLORG, Color.y()); //G
				KF->SetChannelValue(NVAnimObject::CANONCOLORB, Color.z()); //B
				} // if
			CurAnimSubObject->AddKeyFrame(KF);
			} // if
		} // if
	} // if

return(Success);
} // ParseKeySubTag






unsigned long int ReadScene(const char *OrigSceneName)
{
FILE *SceneFile;
unsigned long int Entities = 0, PBLen;
unsigned long int TotalFileLen = 0;
char LineBuffer[1000], PathPart[513], FilePart[101], *ParseBuffer;
int CurrentSuperTagID = TAG_NONE;
bool ClosingTag;
float NVWFileVersion;
bool InComment = false, SkipLine, SignatureRead = false;
#ifdef NV_CHECK_SIGNATURES
bool SigInvalid = false;
#endif // NV_CHECK_SIGNATURES
NVAnimObject *CurAnimSubObject = NULL;
const char *SceneName;
char SNBuf[1000];

BusyWin *ProgressDisplay;

SceneName = OrigSceneName;

// check for ZIP archive
if(SceneFile = fopen(OrigSceneName, "rb"))
	{
	unsigned char PKHeader[4];
	PKHeader[0] = 0;

	// change to location of input scene file
	strcpy(LineBuffer, SceneName); // BreakFileName alters input string
	BreakFileName(LineBuffer, PathPart, 512, FilePart, 100);
	if(strlen(PathPart))
		{
		osgDB::Registry::instance()->setDataFilePathList(PathPart);
		chdir(PathPart); // so we pick up relative file paths properly
		} // if

	fread(&PKHeader[0], 4, 1, SceneFile);
	fclose(SceneFile);
	SceneFile = NULL;
	if(PKHeader[0] == 0x50 && PKHeader[1] == 0x4B && PKHeader[2] == 0x03 && PKHeader[3] == 0x04)
		{ // it's a zipfile, decompress it before moving on.
		char OrigPart[1000], PathPart[500], FilePart[200];
		strcpy(OrigPart, OrigSceneName);
		BreakFileName(OrigPart, PathPart, 500, FilePart, 200);
		ProgressDisplay = new BusyWin("Decompressing", FilePart, 0, 0);
		if(SceneName = DecompressZipScene(OrigSceneName))
			{ // scene file should now be local to current dir
			std::string JustPart;
			JustPart = osgDB::getFilePath(SceneName);
			if(!JustPart.empty())
				{
				chdir(JustPart.c_str());
				getcwd(LineBuffer, 999);
				osgDB::Registry::instance()->setDataFilePathList(LineBuffer);
				JustPart = osgDB::getSimpleFileName(SceneName);
				strcpy(SNBuf, JustPart.c_str());
				SceneName = &SNBuf[0];
				} // if
			} // if
		if(ProgressDisplay) delete ProgressDisplay;
		ProgressDisplay = NULL;
		} // if
	} // if

// detect stupid input or failure of unzip, above
if(!SceneName) return(0);

if(SceneFile = fopen(SceneName, "r"))
	{
	unsigned long int CurPos;
	std::string ScenePathPart;
	ScenePathPart = osgDB::getFilePath(SceneName);
	MasterScene.SetSceneFileWorkingPath(ScenePathPart);
	fseek(SceneFile, 0, SEEK_END);
	TotalFileLen = ftell(SceneFile);
	fseek(SceneFile, 0, SEEK_SET);
	if(TotalFileLen > 0) // prevent divide-by-zero later...
		{
		ProgressDisplay = new BusyWin(NVW_NATUREVIEW_NAMETEXT, "Loading", 0, 1);
		} // if
 	while(fgetline(LineBuffer, 999, SceneFile, 0, 0) && !SignatureRead) // stop reading file once signature is read
		{
		if(ProgressDisplay)
			{
			float CurFrac;
			CurPos = ftell(SceneFile);
			CurFrac = (double)CurPos / (double)TotalFileLen;
			ProgressDisplay->UpdateAndCheckAbort(CurFrac);
			} // if
		ClosingTag = false;
		SkipLine = false;
		// rewrite tabs to spaces for consistent handling further on
		for(int TabToSpace = 0; LineBuffer[TabToSpace]; TabToSpace++)
			{
			if(LineBuffer[TabToSpace] == '\t') LineBuffer[TabToSpace] = ' ';
			} // for
		if(LineBuffer[strlen(LineBuffer) - 1] == '\n' || LineBuffer[strlen(LineBuffer) - 1] == '\r') LineBuffer[strlen(LineBuffer) - 1] = 0;
		if(LineBuffer[strlen(LineBuffer) - 1] == '\n' || LineBuffer[strlen(LineBuffer) - 1] == '\r') LineBuffer[strlen(LineBuffer) - 1] = 0;
		TrimTrailingSpaces(LineBuffer);
		if(ParseBuffer = SkipSpaces(LineBuffer))
			{
			PBLen = strlen(ParseBuffer);
			// try to handle XML-style comments
			if(PBLen > 2) // comment tags are either <!-- or -->
				{
				if(InComment)
					{ // look for ending -->
					// we've already establed that len => 2 above
					if(!strncmp(&ParseBuffer[PBLen - 3], "-->", 3)) // check for endcomment at end of line
						{
						SkipLine = true;
						InComment = 0; // no longer in comment
						} // if
					} // if
				else
					{ // look for beginning <!--
					if(PBLen > 3)
						{
						if(!strncmp(ParseBuffer, "<!--", 4))
							{
							// look for special file-ending SIGNATURE comment
							if(!strncmp(ParseBuffer, "<!-- SIGNATURE", 14))
								{ // strip off special junk, reformat for parsing below, and set SignatureRead true so we stop reading
								ParseBuffer[3] = '<'; // reformat as as <SIGNATURE... -->
								ParseBuffer = &ParseBuffer[3];
								PBLen -= 3; // account for front truncated characters
								SignatureRead = true;
								} // if
							else
								{
								// is it a single-line comment (set SkipLine but don't set InComment)
								if(PBLen > 6 && !strncmp(&ParseBuffer[PBLen - 3], "-->", 3)) // at least 7 chars, enough for "<!---->"
									{ // single line comment, do not set InComment
									SkipLine = true;
									} // if
								else
									{ // comment does not end on this line, set InComment for multi-line handling
									SkipLine = true;
									InComment = 1;
									} // else
								} // else
							} // if
						} // if
					} // else
				} // if

			if(InComment) SkipLine = true; // keep skipping lines until we hit a closing comment line

			if(!SkipLine && PBLen > 2 && ParseBuffer[0] == '<' && ParseBuffer[PBLen - 1] == '>') // more or less a tag
				{
				ParseBuffer[strlen(ParseBuffer) - 1] = 0; // munch last > char
				ParseBuffer = &ParseBuffer[1]; // skip over first < char

				// Identify the Tag
				char ButcherableArg[1024], *AfterFirstWord;
				int NumArgsParsed, NumTagArgsParsed, Where = 0;
				char *Result;

				strncpy(ButcherableArg, ParseBuffer, 1000); ButcherableArg[1000] = NULL;

				// truncate at first whitespace
				if(AfterFirstWord = ScanToNextWord(ButcherableArg))
					{
					if(AfterFirstWord != ButcherableArg)
						{
						AfterFirstWord[0] = 0;
						} // if
					} // if

				if(AfterFirstWord[0] == '/')
					{
					ClosingTag = true;
					// move past /
					AfterFirstWord = &AfterFirstWord[1];
					} // if

				ArgRipper JackTheTag(TagRipTemplate);
				NumArgsParsed = JackTheTag.Rip(ButcherableArg);
				if(NumArgsParsed > 0)
					{
					bool AutoCloseAtEnd = false;
					if(ClosingTag) // technically, we should only close tag if it matches our current open supertag, but we're not that strict
						{
						CurrentSuperTagID = TAG_NONE;
						} // if
					else
						{
						for(int TagID = 0; TagID < TAG_MAX; TagID++)
							{
							Where = 0;
							if(Result = JackTheTag.GetArg(TagID, Where)) // only do the first bit, pass rest along for further parsing
								{
								Entities++;

								// determine if closing tag is at end of line and set autoclose
								if(JackTheTag.GetArgText(0)) // get text of first word
									{
									char EndTagTest[100];
									sprintf(EndTagTest, "></%s", JackTheTag.GetArgText(0)); // it will already be missing closing >
									if(strlen(ParseBuffer) >= strlen(EndTagTest))
										{
										char *CloseTagTest = &ParseBuffer[strlen(ParseBuffer) - strlen(EndTagTest)];
										if(!stricmp(EndTagTest, CloseTagTest))
											{
											CloseTagTest[0] = NULL; // chop off the closing tag now that we've noted it
											AutoCloseAtEnd = true;
											} // if
										} // if
									} // if

								// FILENAME arg is common to several tags
								char *FILENAMEArgRipTemplate = "FILENAME SIG";
								ArgRipper FNTag(FILENAMEArgRipTemplate);

								// prep for parsing the args for each tag
								strncpy(ButcherableArg, ParseBuffer, 1000); ButcherableArg[1000] = NULL;
								AfterFirstWord = ScanToNextWord(ButcherableArg);

								CurrentSuperTagID = TagID; // note supertag for subtag processing
								switch(TagID)
									{
									case TAG_SPLASH:
										{
										int Count;
										char *SPLASHArgRipTemplate = "TIME";
										ArgRipper SPLASHTag(SPLASHArgRipTemplate);
										if(NumTagArgsParsed = SPLASHTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = SPLASHTag.GetArg(0, Count)) // 0=TIME
													{
													MasterScene.SetSplashTime(atof(Result));
													} // if
												else Count++;
												} // for
											} // if
										break;
										} // SPLASH
									case TAG_VERSION:
										{
										int Count;
										char *VERSArgRipTemplate = "FILEVERSION PREFERREDVIEWER";
										ArgRipper VERSTag(VERSArgRipTemplate);
										if(NumTagArgsParsed = VERSTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = VERSTag.GetArg(0, Count)) // 0=FILEVERSION
													{
													NVWFileVersion = atof(Result);
													if(NVWFileVersion > NVW_FILE_VERSION)
														{
														char FileVersionError[512];
														sprintf(FileVersionError, "File version %.2f is newer than this viewer recognizes.\nPlease update " NVW_NATUREVIEW_NAMETEXT " to the most current version.\n\nhttp://www.3DNature.com/nv/", NVWFileVersion);
														UserMessageOK(NVW_NATUREVIEW_NAMETEXT, FileVersionError);
														fclose(SceneFile); SceneFile = NULL;
														return(0); // error
														} // if
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = VERSTag.GetArg(1, Count)) // 1=PREFERREDVIEWER
													{
													char ViewerTypeWarning[512], UpResult[512];
													strcpy(ViewerTypeWarning, NVW_NATUREVIEW_NAMETEXT);
													strcpy(UpResult, Result);
													strupr(UpResult); // stricmp is giving me problems, so we'll do it the hard way
													strupr(ViewerTypeWarning);
													if(strcmp(UpResult, ViewerTypeWarning))
														{
														sprintf(ViewerTypeWarning, "This file is intended for use with the %s viewer.\nNot all functionality may be available in this viewer.", Result);
														UserMessageOK(NVW_NATUREVIEW_NAMETEXT, ViewerTypeWarning);
														} // if
													} // if
												else Count++;
												} // for
											} // if
										break;
										} // TAG_VERSION
									case TAG_DEM:
										{
										int Count;
										char *DEMArgRipTemplate = "FILENAME DRAPEIMAGEFILENAME DEMSIG DRAPESIG FOLDRAPEIMAGEFILENAME FOLDRAPESIG BDRAPEIMAGEFILENAME BDRAPESIG MAXELEV MINELEV";
										ArgRipper DEMTag(DEMArgRipTemplate);
										if(NumTagArgsParsed = DEMTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = DEMTag.GetArg(0, Count)) // 0=FILENAME
													{
													MasterScene.SetDEMName(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = DEMTag.GetArg(1, Count)) // 1=DRAPEIMAGEFILENAME
													{
													MasterScene.SetDrapeImageName(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = DEMTag.GetArg(2, Count)) // 2=DEMSIG
													{
													MasterScene.SetDEMSig(Result); // DEMSIG
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = DEMTag.GetArg(3, Count)) // 3=DRAPESIG
													{
													MasterScene.SetDrapeImageSigA(Result); // DRAPESIG
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = DEMTag.GetArg(4, Count)) // 4=FOLDRAPEIMAGEFILENAME
													{
													MasterScene.SetFolDrapeImageName(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = DEMTag.GetArg(5, Count)) // 5=FOLDRAPESIG
													{
													MasterScene.SetFolDrapeImageSig(Result); // FOLDRAPESIG
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = DEMTag.GetArg(6, Count)) // 6=BDRAPEIMAGEFILENAME
													{
													MasterScene.SetDrapeImageNameB(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = DEMTag.GetArg(7, Count)) // 7=BDRAPESIG
													{
													MasterScene.SetDrapeImageSigB(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = DEMTag.GetArg(8, Count)) // 8=MAXELEV
													{
													MasterScene.SetTerrainMaxElev(atof(Result));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = DEMTag.GetArg(9, Count)) // 9=MINELEV
													{
													MasterScene.SetTerrainMinElev(atof(Result));
													} // if
												else Count++;
												} // for
											} // if
										break;
										} // TAG_DEM
									case TAG_INSTANCEFILE:
										{
										int Count;
										char *INSTArgRipTemplate = "FILENAME FOLIAGETYPE SIG";
										ArgRipper INSTTag(INSTArgRipTemplate);
										if(NumTagArgsParsed = INSTTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = INSTTag.GetArg(0, Count)) // 0=FILENAME
													{
													MasterScene.SetFolFileName(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = INSTTag.GetArg(1, Count)) // 1=FOLIAGETYPE
													{
													strupr(Result);
													if(!strcmp(Result, "BILLBOARDS"))
														{
														MasterScene.SetFolType(0);
														} // if
													else if(!strcmp(Result, "CROSSBOARDS"))
														{
														MasterScene.SetFolType(1);
														} // if
													else if(!strcmp(Result, "SPRITES"))
														{
														MasterScene.SetFolType(2);
														} // if
													else
														{
														MasterScene.SetFolType(0);
														} // if
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = INSTTag.GetArg(2, Count)) // 2=SIG
													{
													MasterScene.SetFolFileSig(Result); // SIG
													} // if
												else Count++;
												} // for
											//MasterScene.SetFolType(2); // debug
											} // if
										break;
										} // TAG_INSTANCEFILE
									case TAG_OBJFILE:
										{
										 // <<<>>>SIG
										if(NumTagArgsParsed = FNTag.Rip(Result))
											{
											Where = 0;
											if(Result = FNTag.GetArg(0, Where)) // 0=FILENAME
												{
												MasterScene.SetObjFileName(Result);
												} // if
											} // if
										break;
										} // TAG_OBJFILE
									case TAG_OBJINSTANCE:
										{
										int Count;
										// FILENAME arg plus ACTIONID
										char *OBJINSTArgRipTemplate = "FILENAME SIG ACTIONID";
										ArgRipper OITag(OBJINSTArgRipTemplate);
										bool ReadName = false;
										if(CurAnimSubObject = new NVAnimObject)
											{
											CurAnimSubObject->Type = NVAnimObject::OBJINSTANCE;
											} //

										if(NumTagArgsParsed = OITag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = OITag.GetArg(0, Count)) // 0=FILENAME
													{
													if(CurAnimSubObject)
														{
														CurAnimSubObject->SetName(Result);
														} // if
													ReadName = true;
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = OITag.GetArg(1, Count)) // 1=SIG
													{
													if(CurAnimSubObject)
														{
														CurAnimSubObject->SetOverallSig(Result);
														} // if
													ReadName = true;
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = OITag.GetArg(2, Count)) // 2=ACTIONID
													{
													if(CurAnimSubObject)
														{
														CurAnimSubObject->SetGenericIntValue(NVAnimObject::ACTIONID, atoi(Result));
														} // if
													ReadName = true;
													} // if
												else Count++;
												} // for
											} // if

										if(ReadName)
											{
											if(CurAnimSubObject)
												{
												MasterScene.Objects.push_back(CurAnimSubObject);
												} // if
											} // if
										else
											{
											if(CurAnimSubObject)
												{
												delete CurAnimSubObject;
												CurAnimSubObject = NULL;
												} // if
											} // else
										break;
										} // TAG_OBJINSTANCE
									case TAG_LIGHT: // TYPE[PARALLEL, SPOT, OMNI, AMBIENT SKY, AMBIENT GROUND], NAME, subkeys
										{
										int Count;
										char *LIGHTArgRipTemplate = "TYPE NAME"; // we currently ignore NAME for lights, maybe someday we'll find a use for it
										ArgRipper LIGHTTag(LIGHTArgRipTemplate);

										if(CurAnimSubObject = new NVAnimObject)
											{
											CurAnimSubObject->Type = NVAnimObject::LIGHT;
											CurAnimSubObject->SetEnabled(true);
											} //

										if(NumTagArgsParsed = LIGHTTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LIGHTTag.GetArg(0, Count)) // 0=TYPE
													{
													if(CurAnimSubObject)
														{
														if(!stricmp(Result, "PARALLEL"))
															{
															CurAnimSubObject->SetGenericIntValue(NVAnimObject::LIGHTTYPE, NVAnimObject::LIGHTPARALLEL);
															} // if
														if(!stricmp(Result, "SPOT"))
															{
															CurAnimSubObject->SetGenericIntValue(NVAnimObject::LIGHTTYPE, NVAnimObject::LIGHTSPOT);
															} // if
														if(!stricmp(Result, "OMNI"))
															{
															CurAnimSubObject->SetGenericIntValue(NVAnimObject::LIGHTTYPE, NVAnimObject::LIGHTOMNI);
															} // if
														if(!stricmp(Result, "AMBIENT SKY"))
															{
															CurAnimSubObject->SetGenericIntValue(NVAnimObject::LIGHTTYPE, NVAnimObject::LIGHTAMBIENTSKY);
															} // if
														if(!stricmp(Result, "AMBIENT GROUND"))
															{
															CurAnimSubObject->SetGenericIntValue(NVAnimObject::LIGHTTYPE, NVAnimObject::LIGHTAMBIENTGROUND);
															} // if
														} // if
													} // if
												else Count++;
												} // for
											} // if

										if(CurAnimSubObject)
											{
											MasterScene.Lights.push_back(CurAnimSubObject);
											} // if
										break;
										} // TAG_LIGHT
									case TAG_SKY: // TYPE[SPHERE, CUBE, etc], <<<>>>
										{
										int Count;
										char *SKYArgRipTemplate = "TYPE";
										ArgRipper SKYTag(SKYArgRipTemplate);

										if(CurAnimSubObject = new NVAnimObject)
											{
											CurAnimSubObject->Type = NVAnimObject::SKY;
											CurAnimSubObject->SetEnabled(true);
											} //

										if(NumTagArgsParsed = SKYTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = SKYTag.GetArg(0, Count)) // 0=TYPE
													{
													if(CurAnimSubObject)
														{
														CurAnimSubObject->SetGenericIntValue(NVAnimObject::SKYTYPE, NVAnimObject::SKYDOME);
														} // if
													} // if
												else Count++;
												} // for
											} // if

										if(CurAnimSubObject)
											{
											MasterScene.Skies.push_back(CurAnimSubObject);
											} // if
										break;
										} // TAG_SKY
									case TAG_OVERLAY: // LOGOFILENAME, TEXT, LOGOSIG, NUMLINES, SHOWMAP
										{
										int Count, Params = 0;
										char *OVLYArgRipTemplate = "LOGOFILENAME TEXT LOGOSIG NUMLINES SHOWMAP LOGOALIGN";
										ArgRipper OVLYTag(OVLYArgRipTemplate);
										if(NumTagArgsParsed = OVLYTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = OVLYTag.GetArg(0, Count)) // 0=LOGOFILENAME
													{
													MasterScene.SetLogoImageName(Result);
													Params++;
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = OVLYTag.GetArg(1, Count)) // 1=TEXT
													{
													MasterScene.Overlay.SetOverlayTextTemplate(Result);
													Params++;
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = OVLYTag.GetArg(2, Count)) // 2=LOGOSIG
													{
													MasterScene.SetLogoImageSig(Result);
													Params++;
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = OVLYTag.GetArg(3, Count)) // 3=NUMLINES
													{
													MasterScene.Overlay.SetNumLines(max(atoi(Result), 1));
													Params++;
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = OVLYTag.GetArg(4, Count)) // 4=SHOWMAP
													{
													MasterScene.Overlay.SetMapState(ParseBooleanPresenceIsTrue(Result));
													Params++;
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = OVLYTag.GetArg(5, Count)) // 5=LOGOALIGN
													{
													strupr(Result);
													if(!strcmp(Result, "LEFT"))
														{
														MasterScene.SetLogoAlign(NVScene::Left);
														} // if
													Params++;
													} // if
												else Count++;
												} // for
											} // if
										if(Params)
											{
											MasterScene.Overlay.SetOverlayState(1);
											} // if
										else
											{
											MasterScene.Overlay.SetOverlayState(0);
											} // else
										break;
										} // TAG_OVERLAY
									case TAG_WATERMARK: // TEXT
										{
										char *WMArgRipTemplate = "TEXT";
										ArgRipper WMTag(WMArgRipTemplate);
										if(NumTagArgsParsed = WMTag.Rip(Result))
											{
											Where = 0;
											if(Result = WMTag.GetArg(0, Where)) // 0=TEXT
												{
												std::string ProcessedText;
												//MasterScene.Watermark.SetWatermarkText(Result);
												ProcessedText = NVOverlay::GetProcessedText(Result);
												MasterScene.Watermark.SetWatermarkText(ProcessedText.c_str());
												MasterScene.Watermark.SetWatermarkState(true);
												} // if
											} // if
										MasterScene.Watermark.SetWatermarkState(1);
										break;
										} // TAG_WATERMARK
									case TAG_HAZE: // TYPE[LINEAR]
										{
										int Count;
										char *HAZEArgRipTemplate = "TYPE";

										ArgRipper HAZETag(HAZEArgRipTemplate);

										if(CurAnimSubObject = new NVAnimObject)
											{
											CurAnimSubObject->Type = NVAnimObject::HAZE;
											CurAnimSubObject->SetEnabled(true);
											} //

										if(NumTagArgsParsed = HAZETag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = HAZETag.GetArg(0, Count)) // 0=TYPE
													{
													if(CurAnimSubObject)
														{
														CurAnimSubObject->SetGenericIntValue(NVAnimObject::HAZETYPE, NVAnimObject::HAZELINEAR);
														} // if
													} // if
												else Count++;
												} // for
											} // if

										if(CurAnimSubObject)
											{
											MasterScene.Hazes.push_back(CurAnimSubObject);
											} // if
										break;
										} // TAG_HAZE
									case TAG_META: // NAME, COPYRIGHT, AUTHOR, USER1-5
										{
										int Count;
										char *METAArgRipTemplate = "NAME COPYRIGHT AUTHOR USERONE USERTWO USERTHREE USERFOUR USERFIVE EMAIL";
										ArgRipper METATag(METAArgRipTemplate);
										if(NumTagArgsParsed = METATag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = METATag.GetArg(0, Count)) // 0=NAME
													{
													MasterScene.SetMetaName(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = METATag.GetArg(1, Count)) // 1=COPYRIGHT
													{
													MasterScene.SetMetaCopyright(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = METATag.GetArg(2, Count)) // 2=AUTHOR
													{
													MasterScene.SetMetaAuthor(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = METATag.GetArg(3, Count)) // 3=USERONE
													{
													MasterScene.SetMetaUser1(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = METATag.GetArg(4, Count)) // 4=USERTWO
													{
													MasterScene.SetMetaUser2(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = METATag.GetArg(5, Count)) // 5=USERTHREE
													{
													MasterScene.SetMetaUser3(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = METATag.GetArg(6, Count)) // 6=USERFOUR
													{
													MasterScene.SetMetaUser4(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = METATag.GetArg(7, Count)) // 7=USERFIVE
													{
													MasterScene.SetMetaUser5(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = METATag.GetArg(8, Count)) // 8=EMAIL
													{
													MasterScene.SetMetaEMail(Result);
													} // if
												else Count++;
												} // for
											} // if
										break;
										} // TAG_META
									case TAG_LOD: // CLIPNEAR, CLIPFAR, MINFEATURESIZEPIXELS, TERRAINCLIP, FOLIAGECLIP, OBJECTCLIP, OBJECTBOX, COMPRESSTERRAINTEX, COMPRESSFOLIAGETEX, FOLIAGEQUALITY, MAXFOLIAGESTEMS, OPTIMIZEMOVE, LABELCLIP
										{
										int Count;
										char *LODArgRipTemplate = "CLIPNEAR CLIPFAR MINFEATURESIZEPIXELS TERRAINCLIP FOLIAGECLIP OBJECTCLIP OBJECTBOX COMPRESSTERRAINTEX COMPRESSFOLIAGETEX FOLIAGEQUALITY MAXFOLIAGESTEMS OPTIMIZEMOVE LABELCLIP MIPMAP";
										ArgRipper LODTag(LODArgRipTemplate);

										if(NumTagArgsParsed = LODTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(0, Count)) // 0=CLIPNEAR
													{
													MasterScene.SceneLOD.SetNearClip(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(1, Count)) // 1=CLIPFAR
													{
													MasterScene.SceneLOD.SetFarClip(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(2, Count)) // 2=MINFEATURESIZEPIXELS
													{
													MasterScene.SceneLOD.SetMinFeatureSizePixels(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(3, Count)) // 3=TERRAINCLIP
													{
													MasterScene.SceneLOD.SetTerrainClip(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(4, Count)) // 4=FOLIAGECLIP
													{
													MasterScene.SceneLOD.SetFolClip(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(5, Count)) // 5=OBJECTCLIP
													{
													MasterScene.SceneLOD.SetObjClip(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(6, Count)) // 6=OBJECTBOX
													{
													MasterScene.SceneLOD.SetObjBox(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(7, Count)) // 7=COMPRESSTERRAINTEX 
													{
													MasterScene.SceneLOD.SetCompressTerrainTex(ParseBooleanPresenceIsTrue(Result));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(8, Count)) // 8=COMPRESSFOLIAGETEX
													{
													MasterScene.SceneLOD.SetCompressFolTex(ParseBooleanPresenceIsTrue(Result));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(9, Count)) // 9=FOLIAGEQUALITY
													{
													if(!stricmp(Result, "FALSE") || !stricmp(Result, "LOW"))
														{
														MasterScene.SceneLOD.SetFoliageQuality(false);
														} // if
													else // HIGH, TRUE, or anything else, or just the presence of the tag
														{
														MasterScene.SceneLOD.SetFoliageQuality(true);
														} // else
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(10, Count)) // 10=MAXFOLIAGESTEMS
													{
													MasterScene.SceneLOD.SetMaxFoliageStems(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(11, Count)) // 11=OPTIMIZEMOVE
													{
													MasterScene.SceneLOD.SetOptimizeMove(ParseBooleanPresenceIsTrue(Result));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(12, Count)) // 12=LABELCLIP
													{
													MasterScene.SceneLOD.SetLabelClip(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = LODTag.GetArg(13, Count)) // 13=MIPMAP
													{
													if(!stricmp(Result, "LINEAR"))
														{
														MasterScene.SceneLOD.SetMipmapMode(osg::Texture2D::LINEAR);
														} // if
													else if(!stricmp(Result, "LINEAR_MIPMAP_NEAREST"))
														{
														MasterScene.SceneLOD.SetMipmapMode(osg::Texture2D::LINEAR_MIPMAP_NEAREST);
														} // else if
													else if(!stricmp(Result, "NEAREST_MIPMAP_LINEAR"))
														{
														MasterScene.SceneLOD.SetMipmapMode(osg::Texture2D::NEAREST_MIPMAP_LINEAR);
														} // else if
													else if(!stricmp(Result, "NEAREST_MIPMAP_NEAREST"))
														{
														MasterScene.SceneLOD.SetMipmapMode(osg::Texture2D::NEAREST_MIPMAP_NEAREST);
														} // else if
													else if(!stricmp(Result, "LINEAR_MIPMAP_LINEAR"))
														{ // this is the default
														MasterScene.SceneLOD.SetMipmapMode(osg::Texture2D::LINEAR_MIPMAP_LINEAR);
														} // else if
													// now we default to NEAREST
													else // if(!stricmp(Result, "NEAREST"))
														{
														MasterScene.SceneLOD.SetMipmapMode(osg::Texture2D::LINEAR_MIPMAP_LINEAR);
														} // else if
													} // if
												else Count++;
												} // for
											} // if
										break;
										} // TAG_LOD
									case TAG_ACTION: // OBJECTRECORDSFILE, OBJECTINDEXFILE, ACTIONRECORDSFILE, ACTIONINDEXFILE, OBJECTRECORDSSIG, ACTIONRECORDSSIG
										{ // action database
										int Count;
										char *ACTIONArgRipTemplate = "OBJECTRECORDSFILE OBJECTINDEXFILE ACTIONRECORDSFILE ACTIONINDEXFILE OBJECTRECORDSSIG ACTIONRECORDSSIG";
										ArgRipper ACTIONTag(ACTIONArgRipTemplate);

										if(NumTagArgsParsed = ACTIONTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = ACTIONTag.GetArg(0, Count)) // 0=OBJECTRECORDSFILE
													{
													MasterScene.SetObjectRecordsFile(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = ACTIONTag.GetArg(1, Count)) // 1=OBJECTINDEXFILE
													{
													MasterScene.SetObjectIndexFile(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = ACTIONTag.GetArg(2, Count)) // 2=ACTIONRECORDSFILE
													{
													MasterScene.SetActionRecordsFile(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = ACTIONTag.GetArg(3, Count)) // 3=ACTIONINDEXFILE
													{
													MasterScene.SetActionIndexFile(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = ACTIONTag.GetArg(4, Count)) // 4=OBJECTRECORDSSIG
													{
													MasterScene.SetObjectRecordsSig(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = ACTIONTag.GetArg(5, Count)) // 5=ACTIONRECORDSSIG
													{
													MasterScene.SetActionRecordsSig(Result);
													} // if
												else Count++;
												} // for
											} // if
										break;
										} // TAG_ACTION
									case TAG_OCEAN:
										{
										// no attribs to read at open-tag level
										if(CurAnimSubObject = new NVAnimObject)
											{
											CurAnimSubObject->Type = NVAnimObject::OCEAN;
											CurAnimSubObject->SetEnabled(true);
											} //

										if(CurAnimSubObject)
											{
											MasterScene.Oceans.push_back(CurAnimSubObject);
											} // if
										break;
										} // TAG_OCEAN
									case TAG_NAVIGATION: // SPEED, FOLLOWTERRAINHEIGHT, TYPE/STYLE[PANO], FOLLOWTERRAINMAXHEIGHT, FRICTION, MAXSPEED, INERTIA, ACCELERATION, CONSTRAIN
										{
										int Count;
										char *NAVArgRipTemplate = "SPEED FOLLOWTERRAINHEIGHT STYLE TYPE FOLLOWTERRAINMAXHEIGHT FRICTION MAXSPEED INERTIA ACCELERATION CONSTRAIN"; // TYPE == STYLE
										ArgRipper NAVTag(NAVArgRipTemplate);

										if(NumTagArgsParsed = NAVTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NAVTag.GetArg(0, Count)) // 0=SPEED
													{
													MasterScene.SetSpeed(atof(Result) * .5);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NAVTag.GetArg(1, Count)) // 1=FOLLOWTERRAINHEIGHT
													{
													MasterScene.SetFollowTerrainHeight(max(0.0, atof(Result)));
													MasterScene.SetFollowTerrainEnabled(true);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NAVTag.GetArg(2, Count)) // 2=STYLE
													{ // same as TYPE, below
													//MasterScene.SetNavStyle(); // <<<>>>
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NAVTag.GetArg(3, Count)) // 3=TYPE
													{ // same as STYLE, above
													//MasterScene.SetNavStyle(); // <<<>>>
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NAVTag.GetArg(4, Count)) // 4=FOLLOWTERRAINMAXHEIGHT
													{
													MasterScene.SetFollowTerrainHeightMax(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NAVTag.GetArg(5, Count)) // 5=FRICTION
													{
													MasterScene.SetFriction(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NAVTag.GetArg(6, Count)) // 6=MAXSPEED
													{
													MasterScene.SetMaxSpeed(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NAVTag.GetArg(7, Count)) // 7=INERTIA
													{
													MasterScene.SetInertia(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NAVTag.GetArg(8, Count)) // 8=ACCELERATION
													{
													MasterScene.SetAcceleration(max(0.0, atof(Result)));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NAVTag.GetArg(9, Count)) // 9=CONSTRAIN
													{
													MasterScene.SetConstrain(ParseBooleanPresenceIsTrue(Result));
													} // if
												else Count++;
												} // for
											} // if
										break;
										} // TAG_NAVIGATION
									case TAG_CAMERA:
										{
										int Count;
										bool ReadName = false;
										char *CAMArgRipTemplate = "NAME";
										ArgRipper CAMTag(CAMArgRipTemplate);

										if(CurAnimSubObject = new NVAnimObject)
											{
											CurAnimSubObject->Type = NVAnimObject::CAMERA;
											} //

										if(NumTagArgsParsed = CAMTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = CAMTag.GetArg(0, Count)) // 0=NAME
													{
													if(CurAnimSubObject)
														{
														CurAnimSubObject->SetName(Result);
														} // if
													ReadName = true;
													} // if
												else Count++;
												} // for
											} // if

										if(ReadName)
											{
											if(CurAnimSubObject)
												{
												MasterScene.Cameras.push_back(CurAnimSubObject);
												} // if
											} // if
										else
											{
											if(CurAnimSubObject)
												{
												delete CurAnimSubObject;
												CurAnimSubObject = NULL;
												} // if
											} // else
										break;
										} // TAG_CAMERA
									case TAG_KEY:
										{
										ParseKeySubTag(CurAnimSubObject, Result);
										break;
										} // KEY
									case TAG_NOTICE:
										{
										int Count;
										char *NOTArgRipTemplate = "TIME TEXT FILENAME MODAL SIGNATURE";
										ArgRipper NOTTag(NOTArgRipTemplate);

										if(NumTagArgsParsed = NOTTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NOTTag.GetArg(0, Count)) // 0=TIME
													{
													MasterScene.SetNoticeTime(atof(Result));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NOTTag.GetArg(1, Count)) // 1=TEXT
													{
													MasterScene.SetNoticeText(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NOTTag.GetArg(2, Count)) // 2=FILENAME
													{
													MasterScene.SetNoticeFile(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NOTTag.GetArg(3, Count)) // 3=MODAL
													{
													MasterScene.SetNoticeModal(ParseBooleanPresenceIsTrue(Result));
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = NOTTag.GetArg(4, Count)) // 4=SIGNATURE
													{
													MasterScene.SetNoticeSig(Result);
													} // if
												else Count++;
												} // for
											} // if
										break;
										} // NOTICE
									case TAG_SIGNATURE:
										{
										int Count;
										char *SIGArgRipTemplate = "VALUE TYPE";

										// Set SignatureRead again, to stop reading here
										SignatureRead = 1;

										// may need to lop off trailing " --" here for proper parsing
										if(strlen(Result) > 3)
											{
											if(!strcmp(&Result[strlen(Result) - 3], " --"))
												{
												Result[strlen(Result) - 3] = NULL;
												} // if
											} // if
										ArgRipper SIGTag(SIGArgRipTemplate);

										if(NumTagArgsParsed = SIGTag.Rip(Result))
											{
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = SIGTag.GetArg(0, Count)) // 0=VALUE
													{
													MasterScene.SetSig(Result);
													} // if
												else Count++;
												} // for
											for(Count = 0; Count < NumTagArgsParsed;)
												{
												if(Result = SIGTag.GetArg(1, Count)) // 1=TYPE
													{
													//MasterScene. (atof(Result));
													} // if
												else Count++;
												} // for
											} // if
										break;
										} // TAG_SIGNATURE
									default:
										{ // unknown tag
										CurrentSuperTagID = TAG_NONE; // clear supertag
										} // default
									} // TagID
								break;
								} // if
							} // for
						if(AutoCloseAtEnd)
							{ // we had our closing tag at the end of our line, so close it out here
							CurrentSuperTagID = TAG_NONE;
							} // if
						} // else
					} // if

				} // if
			} // if
		} // while
	fclose(SceneFile); SceneFile = NULL;
	if(ProgressDisplay)
		{
		delete ProgressDisplay;
		ProgressDisplay = NULL;
		} // if
	} // if
else
	{
	UserMessageOK(NVW_NATUREVIEW_NAMETEXT, "NVW Scene File could not be opened. Exiting.", REQUESTER_ICON_EXCLAMATION);
	return(0);
	} // else

#ifdef NV_CHECK_SIGNATURES

SigInvalid = CheckNVWFileSignature(SceneName);

if(SigInvalid)
	{
	UserMessageOK(NVW_NATUREVIEW_NAMETEXT, "NVW Scene File does not have valid signatures. Exiting.", REQUESTER_ICON_EXCLAMATION);
	Entities = 0; // failure
	} // else
#endif // NV_CHECK_SIGNATURES

return(Entities);
} // ReadScene

