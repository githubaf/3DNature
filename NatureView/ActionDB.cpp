// ActionDB.cpp
// Code to support database lookups of the proper action to go with a queryaction ActionID
// Written from scratch by CXH
// Copyright 2005

#include <stdio.h>

#include "ActionDB.h"
#ifdef NVW_SUPPORT_QUERYACTION
// must be building under NatureView
#include "NVScene.h"
#ifdef NV_CHECK_SIGNATURES
#include "NVSigCheck.h"
#endif // NV_CHECK_SIGNATURES
#endif // NVW_SUPPORT_QUERYACTION
#include "UsefulIO.h"


// these methods will be implemented right here in NV builds, and elsewhere in WCS/VNS builds.
const std::string &GetActionRecordsFileAsSTLString(void);
const std::string &GetActionIndexFileAsSTLString(void);
const std::string &GetObjectIndexFileAsSTLString(void);
const std::string &GetObjectRecordsFileAsSTLString(void);


#ifdef NVW_SUPPORT_QUERYACTION
// must be building under NatureView
extern NVScene MasterScene;

const std::string &GetActionRecordsFileAsSTLString(void) {return(MasterScene.GetActionRecordsFile());};
const std::string &GetActionIndexFileAsSTLString(void) {return(MasterScene.GetActionIndexFile());};
const std::string &GetObjectIndexFileAsSTLString(void) {return(MasterScene.GetObjectIndexFile());};
const std::string &GetObjectRecordsFileAsSTLString(void) {return(MasterScene.GetObjectRecordsFile());};


#else // !NVW_SUPPORT_QUERYACTION
// must be building elsewhere, like in WCS/VNS

#endif // !NVW_SUPPORT_QUERYACTION

static bool ActionSigValidated = false;
static bool ObjectSigValidated = false;



// returns a count of number of NVActions
// list is owned by caller and needs no explicit management
// list contents (NVAction objects) also are owned by caller after return
// and must be explicitly freed
unsigned long int FetchActionsFromID(const signed long int ActionID, std::vector<NVAction *> &Results)
{
unsigned long int RecordsAdded = 0;
// we will not empty the Results list, because we don't do freeing of the contents
if(ActionID >= 0)
	{
#ifdef NVW_SUPPORT_QUERYACTION // only built in NV, not WCS/VNS
	if(MasterScene.CheckAllActionDataPresent())
#endif // NVW_SUPPORT_QUERYACTION
		{
		std::vector<unsigned long int> ActionRecordNumbers;
		if(FetchActionRecordNumbersFromID(ActionID, ActionRecordNumbers))
			{
			RecordsAdded = FetchActionRecordsFromRecordNumbers(ActionRecordNumbers, Results);
			// return below on success
			} // if
		} // if
	} // if

return(RecordsAdded);
} // FetchActionsFromID


unsigned long int FetchActionRecordsFromRecordNumbers(const std::vector<unsigned long int> &ActionRecordNumbers, std::vector<NVAction *> &Results)
{
unsigned long int RecordsAdded = 0;
FILE *Action;


#ifdef NV_CHECK_SIGNATURES
// Validate signature, but only on first access
if(!ActionSigValidated)
	{
	if(!MasterScene.GetActionRecordsSig().empty() && CheckDependentBinaryFileSignature(MasterScene.GetActionRecordsSig().c_str(), MasterScene.GetActionRecordsFile().c_str()))
		{
		ActionSigValidated = true;
		} // if
	} // if
	
if(!ActionSigValidated) return(RecordsAdded); // no sig, failure 
#endif // NV_CHECK_SIGNATURES


if(Action = fopen(GetActionRecordsFileAsSTLString().c_str(), "rb"))
	{
	// handle header
	char FileID[8];
	fread(FileID, 8, 1, Action);
	if(!stricmp(FileID, "NVWANQA"))
		{
		char Super, Major, Minor, Revision;
		Super    = GetL8U(Action);
		Major    = GetL8U(Action);
		Minor    = GetL8U(Action);
		Revision = GetL8U(Action);
		if(Super == 1 && Major == 0 && Minor == 0) // revision should be a transparent change so we don't validate it
			{
			unsigned long ByteOrder, HeaderSize;
			ByteOrder = GetL32U(Action);
			if(ByteOrder == 0xaabbccdd)
				{
				HeaderSize = GetL32U(Action);
				// skip over any additional header (none written at this time)
				fseek(Action, HeaderSize, SEEK_SET);
				
				// read all actions in ActionRecordNumbers
				for(std::vector<unsigned long int>::const_iterator RecordWalk = ActionRecordNumbers.begin(); RecordWalk != ActionRecordNumbers.end(); RecordWalk++)
					{
					unsigned long int RecordOffset;
					unsigned long int RecordNumber;
					RecordNumber = *RecordWalk;
					if(FetchRecordOffsetFromRecordNumber(RecordOffset, RecordNumber, GetActionIndexFileAsSTLString()))
						{
						signed char ActionType;
						unsigned short ActionLen;
						NVAction *ActionNode;
						fseek(Action, RecordOffset, SEEK_SET);
						// Read this action!
						ActionType = GetL8S(Action);
						ActionLen = GetL16U(Action);
						if(ActionNode = new NVAction(ActionType))
							{
							if(ActionNode->ReadActionString(Action, ActionLen) == ActionLen)
								{
								Results.push_back(ActionNode);
								RecordsAdded++;
								} // if
							} // if
						} // if
					} // for
				} // if
			} // if
		} // if
	fclose(Action);
	Action = NULL;
	} // if

return(RecordsAdded);
} // FetchActionRecordsFromRecordNumbers

// used for either Index file (both same format) so filename is provided externally
// rather than the more convenient technique of fetching it ourself internally
bool FetchRecordOffsetFromRecordNumber(unsigned long int &RecordOffset, signed long int ActionID, const std::string FilePathAndName)
{
bool Success = false;

FILE *Index;

if(ActionID < 0 || ActionID >= 0x3fffffff) // with 4-byte records, this is as far as we can 32-bit fseek
	{
	return(false);
	} // if

if(Index = fopen(FilePathAndName.c_str(), "rb"))
	{
	// handle header
	char FileID[8];
	fread(FileID, 8, 1, Index);
	if(!stricmp(FileID, "NVWONQX") || !stricmp(FileID, "NVWANQX")) // either index -- both same format
		{
		char Super, Major, Minor, Revision;
		Super    = GetL8U(Index);
		Major    = GetL8U(Index);
		Minor    = GetL8U(Index);
		Revision = GetL8U(Index);
		if(Super == 1 && Major == 0 && Minor == 0) // revision should be a transparent change so we don't validate it
			{
			unsigned long ByteOrder, HeaderSize;
			ByteOrder = GetL32U(Index);
			if(ByteOrder == 0xaabbccdd)
				{
				HeaderSize = GetL32U(Index);
				// we're using constant 4 instead of sizeof because the file format is fixed, regardless of the platform/architecture
				fseek(Index, HeaderSize + (ActionID * 4), SEEK_SET);
				RecordOffset = GetL32U(Index);
				Success = true;
				} // if
			} // if
		} // if
	fclose(Index);
	Index = NULL;
	} // if

return(Success);
} // FetchRecordOffsetFromRecordNumber


// returns a count of Action Record Numbers placed into the vector pass as an arg
// list is owned by caller and needs no explicit management
unsigned long int FetchActionRecordNumbersFromID(const signed long int ActionID, std::vector<unsigned long int> &ActionRecordNumbers)
{
unsigned long int NumRecordsAdded = 0;
unsigned long int RecordOffset;

ActionRecordNumbers.clear(); // clear the list, JIC

#ifdef NV_CHECK_SIGNATURES
// Validate signature, but only on first access
if(!ObjectSigValidated)
	{
	if(!MasterScene.GetObjectRecordsSig().empty() && CheckDependentBinaryFileSignature(MasterScene.GetObjectRecordsSig().c_str(), MasterScene.GetObjectRecordsFile().c_str()))
		{
		ObjectSigValidated = true;
		} // if
	} // if
	
if(!ObjectSigValidated) return(NumRecordsAdded); // no sig, failure 
#endif // NV_CHECK_SIGNATURES


if(FetchRecordOffsetFromRecordNumber(RecordOffset, ActionID, GetObjectIndexFileAsSTLString()))
	{
	FILE *ObjectRecord;
	if(ObjectRecord = fopen(GetObjectRecordsFileAsSTLString().c_str(), "rb"))
		{
		char FileID[8];
		// handle header 0xaabbccdd
		fread(FileID, 8, 1, ObjectRecord);
		if(!stricmp(FileID, "NVWONQA"))
			{
			char Super, Major, Minor, Revision;
			Super    = GetL8U(ObjectRecord);
			Major    = GetL8U(ObjectRecord);
			Minor    = GetL8U(ObjectRecord);
			Revision = GetL8U(ObjectRecord);
			if(Super == 1 && Major == 0 && Minor == 0) // revision should be a transparent change so we don't validate it
				{
				unsigned long ByteOrder, HeaderSize;
				ByteOrder = GetL32U(ObjectRecord);
				if(ByteOrder == 0xaabbccdd)
					{
					HeaderSize = GetL32U(ObjectRecord);
					// skip any additional header (none written currently)
					fseek(ObjectRecord, HeaderSize, SEEK_SET);
					
					// lets read!
					unsigned long int NumRecords, CurRecord;
					fseek(ObjectRecord, RecordOffset, SEEK_SET);
					NumRecords = GetL8U(ObjectRecord);
					for(unsigned long int RecordLoop = 0; RecordLoop < NumRecords; RecordLoop++)
						{
						CurRecord = GetL32U(ObjectRecord);
						ActionRecordNumbers.push_back(CurRecord);
						NumRecordsAdded++;
						} // if
					} // if
				} // if
			} // if
		fclose(ObjectRecord);
		ObjectRecord = NULL;
		} // if
	} // if

return(NumRecordsAdded);
} // FetchActionRecordNumbersFromID


unsigned short int NVAction::ReadActionString(FILE *Input, unsigned short InputLen)
{ // does some evil manipulation of the std::string internal buffer, like by writing to it directly!
unsigned short int ReadLen = 0;
const char *Safe;
char *Evil;
std::string TempEvil;

TempEvil.resize(InputLen + 1); // resize it
Safe = TempEvil.c_str();
Evil = (char *)Safe; // making it writable is evil!

if(fread(Evil, InputLen, 1, Input))
	{
	Evil[InputLen] = NULL;
	_ActionString = TempEvil;
	ReadLen = InputLen;
	} // if

return(ReadLen);
} // NVAction::ReadActionString
