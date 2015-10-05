// SXQueryItem.cpp
// Source file for SceneExporter Click-to-Query SXQueryItem.
// Built from scratch on 12/30/04 by Gary R. Huber
// Copyright 2004 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "SXQueryItem.h"
#include "SXQueryAction.h"
#include "UsefulPathString.h"
#include "UsefulIO.h"
#include "Project.h"
#include "ExportControlGUI.h"

// define this if you want ascii output for action query files
// also found in SXQueryAction.cpp
//#define WCS_SXACTIONQUERY_ASCIIFILE

long SXQueryItem::ObjectFileID;
long SXQueryItem::ActionFileID;
FILE *SXQueryItem::ObjectFile;
FILE *SXQueryItem::ObjectFileRecords;
const char *SXQueryItem::OutputFilePath;

SXQueryItem::SXQueryItem()
{

ClickQueryEnabled = false;
RecordNum = -1;
RecordFreq = WCS_SXQUERYITEM_FREQ_UNKNOWN;
NumActions = 0;
ActionList = NULL;
VectorList = NULL;

} // SXQueryItem::SXQueryItem

/*===========================================================================*/

SXQueryItem::~SXQueryItem()
{

DestroyVectorList();
DestroyActionList();

} // SXQueryItem::~SXQueryItem

/*===========================================================================*/

void SXQueryItem::DestroyVectorList(void)
{
SXQueryVectorList *CurList;

while (VectorList)
	{
	CurList = VectorList->Next;
	delete VectorList;
	VectorList = CurList;
	} // while

} // SXQueryItem::DestroyVectorList

/*===========================================================================*/

void SXQueryItem::DestroyActionList(void)
{
SXQueryActionList *CurList;

while (ActionList)
	{
	CurList = ActionList->Next;
	delete ActionList;
	ActionList = CurList;
	} // while

} // SXQueryItem::DestroyActionList

/*===========================================================================*/

int SXQueryItem::SXQuerySetupForExport(SXQueryAction *AddAction, const char *OutputFilePathSource)
{
SXQueryActionList **CurListPtr = &ActionList;

// init static counters
ObjectFileID = ActionFileID = 0;
CloseObjectFile();
OutputFilePath = OutputFilePathSource;

// enable instant query-enabled checking
SetClickQueryEnabled(true);
RecordNum = -1;
RecordFreq = WCS_SXQUERYITEM_FREQ_UNKNOWN;
NumActions = 0;

// add SXQueryActionList items for each query
while (*CurListPtr)
	{
	CurListPtr = &(*CurListPtr)->Next;
	} // if

if (! (*CurListPtr = new SXQueryActionList(AddAction)))
	return (0);

return (1);

} // SXQueryItem::SXQuerySetupForExport

/*===========================================================================*/

void SXQueryItem::SXQueryCleanupFromExport(void)
{

SetClickQueryEnabled(false);
DestroyVectorList();
DestroyActionList();
CloseObjectFile();
OutputFilePath = NULL;

} // SXQueryItem::SXQueryCleanupFromExport

/*===========================================================================*/

SXQueryVectorList *SXQueryItem::AddVectorList(Joe *AddVector)
{
SXQueryVectorList **CurListPtr = &VectorList;

while (*CurListPtr)
	{
	CurListPtr = &(*CurListPtr)->Next;
	} // if

return (*CurListPtr = new SXQueryVectorList(AddVector));

} // SXQueryItem::AddVectorList

/*===========================================================================*/

SXQueryVectorList *SXQueryItem::SearchVectorList(Joe *SearchVector)
{
SXQueryVectorList *CurList = VectorList;

while (CurList)
	{
	if (CurList->QueryVector == SearchVector)
		return CurList;
	CurList = CurList->Next;
	} // if

return (NULL);

} // SXQueryItem::SearchVectorList

/*===========================================================================*/

long SXQueryItem::GetRecordNumber(GeneralEffect *MyEffect, Joe *MyVector, NameList **FileNamesCreated)
{

if (RecordFreq == WCS_SXQUERYITEM_FREQ_UNKNOWN)
	{
	DetermineRecordFrequency();
	} // if

switch (RecordFreq)
	{
	case WCS_SXQUERYITEM_FREQ_PERACTION:
	case WCS_SXQUERYITEM_FREQ_PERCOMPONENT:
		{
		// for each action see if a new action record needs to be generated
		// then write an object record incorporating all the actions
		SXQueryActionList *CurAction = ActionList;
		if (RecordNum < 0)
			{
			while (CurAction)
				{
				if (CurAction->QueryAction)
					{
					if (CurAction->RecordNum < 0 && CurAction->RecordFreq != WCS_SXQUERYITEM_FREQ_NORECORD)
						CurAction->RecordNum = CurAction->QueryAction->GetRecordNumber(MyEffect, MyVector, ActionFileID, FileNamesCreated);
					if (CurAction->RecordNum < 0)
						NumActions --;
					} // if
				CurAction = CurAction->Next;
				} // while
			if (NumActions > 0)
				{
				// write Object file record
				if ((ObjectFile && ObjectFileRecords) || OpenObjectFile(FileNamesCreated))
					{
					unsigned long FilePos;
					//long RecordNumFlip;

					// log the start of the object record in the index file
					FilePos = ftell(ObjectFileRecords);
					#ifdef WCS_SXACTIONQUERY_ASCIIFILE
					fprintf(ObjectFile, "%d\n", FilePos);
					#else // WCS_SXACTIONQUERY_ASCIIFILE
					//#ifdef WCS_BYTEORDER_BIGENDIAN
					//SimpleEndianFlip32U(FilePos, &FilePos);
					//#endif // WCS_BYTEORDER_BIGENDIAN
					//fwrite((char *)&FilePos, sizeof (unsigned long), 1, ObjectFile);
					PutL32U(FilePos, ObjectFile);
					#endif // WCS_SXACTIONQUERY_ASCIIFILE
					// write number of Actions in records file
					#ifdef WCS_SXACTIONQUERY_ASCIIFILE
					fprintf(ObjectFileRecords, "%d, ", NumActions);
					#else // WCS_SXACTIONQUERY_ASCIIFILE
					if ((fwrite((char *)&NumActions, sizeof (unsigned char), 1, ObjectFileRecords)) != 1)
						return (-1);
					#endif // WCS_SXACTIONQUERY_ASCIIFILE
					// for each action that has a >= 0 record number write the record number
					CurAction = ActionList;
					while (CurAction)
						{
						if (CurAction->QueryAction)
							{
							if (CurAction->RecordNum >= 0)
								{
								#ifdef WCS_SXACTIONQUERY_ASCIIFILE
								fprintf(ObjectFileRecords, "%d ", CurAction->RecordNum);
								#else // WCS_SXACTIONQUERY_ASCIIFILE
								//RecordNumFlip = CurAction->RecordNum;
								//#ifdef WCS_BYTEORDER_BIGENDIAN
								//SimpleEndianFlip32S(CurAction->RecordNum, &RecordNumFlip);
								//#endif // WCS_BYTEORDER_BIGENDIAN
								//if ((fwrite((char *)&RecordNumFlip, sizeof (unsigned long), 1, ObjectFileRecords)) != 1)
								if(PutL32U(CurAction->RecordNum, ObjectFileRecords))
									return (-1);
								#endif // WCS_SXACTIONQUERY_ASCIIFILE
								} // if
							} // if
						CurAction = CurAction->Next;
						} // while
					#ifdef WCS_SXACTIONQUERY_ASCIIFILE
					fprintf(ObjectFileRecords, "\n");
					#endif // WCS_SXACTIONQUERY_ASCIIFILE
					} // if
				RecordNum = ObjectFileID;
				ObjectFileID ++;
				} // if
			else
				RecordFreq = WCS_SXQUERYITEM_FREQ_NORECORD;
			} // if
		break;
		} // 
	case WCS_SXQUERYITEM_FREQ_PERVECTOR:
	case WCS_SXQUERYITEM_FREQ_PERVECTORPERCOMPONENT:
		{
		SXQueryVectorList *FoundVector;

		// check and see if this vector has been done already
		if (FoundVector = SearchVectorList(MyVector))
			return (FoundVector->RecordNum);
		// not found so add vector
		if (FoundVector = AddVectorList(MyVector))
			{
			SXQueryActionList *CurAction = ActionList;
			char TempActions = NumActions;

			while (CurAction)
				{
				if (CurAction->QueryAction && CurAction->RecordFreq != WCS_SXQUERYITEM_FREQ_NORECORD)
					{
					if (CurAction->RecordNum < 0 || CurAction->RecordFreq == WCS_SXQUERYITEM_FREQ_PERVECTOR || CurAction->RecordFreq == WCS_SXQUERYITEM_FREQ_PERVECTORPERCOMPONENT)
						CurAction->RecordNum = CurAction->QueryAction->GetRecordNumber(MyEffect, MyVector, ActionFileID, FileNamesCreated);
					if (CurAction->RecordNum < 0)
						TempActions --;
					} // if
				CurAction = CurAction->Next;
				} // while

			if (TempActions > 0)
				{
				// write Object file record
				if ((ObjectFile && ObjectFileRecords) || OpenObjectFile(FileNamesCreated))
					{
					unsigned long FilePos;
					//long RecordNumFlip;

					// log the start of the object record in the index file
					FilePos = ftell(ObjectFileRecords);
					#ifdef WCS_SXACTIONQUERY_ASCIIFILE
					fprintf(ObjectFile, "%d\n", FilePos);
					#else // WCS_SXACTIONQUERY_ASCIIFILE
					//#ifdef WCS_BYTEORDER_BIGENDIAN
					//SimpleEndianFlip32U(FilePos, &FilePos);
					//#endif // WCS_BYTEORDER_BIGENDIAN
					//fwrite((char *)&FilePos, sizeof (unsigned long), 1, ObjectFile);
					PutL32U(FilePos, ObjectFile);
					#endif // WCS_SXACTIONQUERY_ASCIIFILE
					// write number of Actions in records file
					#ifdef WCS_SXACTIONQUERY_ASCIIFILE
					fprintf(ObjectFileRecords, "%d, ", TempActions);
					#else // WCS_SXACTIONQUERY_ASCIIFILE
					if ((fwrite((char *)&TempActions, sizeof (unsigned char), 1, ObjectFileRecords)) != 1)
						return (-1);
					#endif // WCS_SXACTIONQUERY_ASCIIFILE

					// for each action that has a >= 0 record number write the record number
					CurAction = ActionList;
					while (CurAction)
						{
						if (CurAction->QueryAction)
							{
							if (CurAction->RecordNum >= 0)
								{
								#ifdef WCS_SXACTIONQUERY_ASCIIFILE
								fprintf(ObjectFileRecords, "%d ", CurAction->RecordNum);
								#else // WCS_SXACTIONQUERY_ASCIIFILE
								//RecordNumFlip = CurAction->RecordNum;
								//#ifdef WCS_BYTEORDER_BIGENDIAN
								//SimpleEndianFlip32S(CurAction->RecordNum, &RecordNumFlip);
								//#endif // WCS_BYTEORDER_BIGENDIAN
								//if ((fwrite((char *)&RecordNumFlip, sizeof (unsigned long), 1, ObjectFileRecords)) != 1)
								if(PutL32U(CurAction->RecordNum, ObjectFileRecords))
									return (-1);
								#endif // WCS_SXACTIONQUERY_ASCIIFILE
								} // if
							} // if
						CurAction = CurAction->Next;
						} // while
					#ifdef WCS_SXACTIONQUERY_ASCIIFILE
					fprintf(ObjectFileRecords, "\n");
					#endif // WCS_SXACTIONQUERY_ASCIIFILE
					} // if
				FoundVector->RecordNum = ObjectFileID;
				ObjectFileID ++;
				return (FoundVector->RecordNum);
				} // if
			} // if
		// failure
		return (-1);
		} // 
	case WCS_SXQUERYITEM_FREQ_PERINSTANCE:
		{
		// no questions asked, create a new record, 
		// use existing action records when not necessary to generate new ones
		break;
		} // 
	default:	// includes WCS_SXQUERYITEM_FREQ_NORECORD
		{
		// no record is to be written for this object
		return (-1);
		} // 
	} // switch

return (RecordNum);

} // SXQueryItem::GetRecordNumber

/*===========================================================================*/

void SXQueryItem::DetermineRecordFrequency(void)
{
SXQueryActionList *CurAction;

RecordFreq = WCS_SXQUERYITEM_FREQ_PERACTION;
NumActions = 0;

// look at each action for its record requirements
for (CurAction = ActionList; CurAction; CurAction = CurAction->Next)
	{
	if (CurAction->QueryAction)
		{
		// test to see if more frequent record generation is required
		if ((CurAction->RecordFreq = CurAction->QueryAction->GetRecordFrequency()) > RecordFreq)
			RecordFreq = CurAction->RecordFreq;
		if (CurAction->RecordFreq != WCS_SXQUERYITEM_FREQ_NORECORD)
			NumActions ++;
		} // if
	} // for

} // SXQueryItem::DetermineRecordFrequency

/*===========================================================================*/

void SXQueryItem::CloseObjectFile(void)
{

if (ObjectFile)
	fclose(ObjectFile);
ObjectFile = NULL;
if (ObjectFileRecords)
	fclose(ObjectFileRecords);
ObjectFileRecords = NULL;

} // SXQueryItem::CloseObjectFile

/*===========================================================================*/

int SXQueryItem::OpenObjectFile(NameList **FileNamesCreated)
{
unsigned long ByteOrder = 0xaabbccdd, HeaderSize;
char FullOutPath[512], FileID[8], IdxFileID[8];
char Super = 1, Major = 0, Minor = 0, Revision = 0;

CloseObjectFile();

strcpy(FileID, "NVWONQA");
strcpy(IdxFileID, "NVWONQX");

if (OutputFilePath)
	{
	#ifdef WCS_SXACTIONQUERY_ASCIIFILE
	strmfp(FullOutPath, OutputFilePath, "SXObjectIndex.txt");
	ObjectFile = PROJ_fopen(FullOutPath, "w");
	strmfp(FullOutPath, OutputFilePath, "SXObjectRecords.txt");
	ObjectFileRecords = PROJ_fopen(FullOutPath, "w");
	#else // ! WCS_SXACTIONQUERY_ASCIIFILE
	strmfp(FullOutPath, OutputFilePath, "SXObjectIndex.nqx");
	ObjectFile = PROJ_fopen(FullOutPath, "wb");
	strmfp(FullOutPath, OutputFilePath, "SXObjectRecords.nqa");
	ObjectFileRecords = PROJ_fopen(FullOutPath, "wb");
	// add items to file name list
	AddNewNameList(FileNamesCreated, "SXObjectIndex.nqx", WCS_EXPORTCONTROL_FILETYPE_QUERYOBJECTIDX);
	AddNewNameList(FileNamesCreated, "SXObjectRecords.nqa", WCS_EXPORTCONTROL_FILETYPE_QUERYOBJECT);
	// write header
	if (ObjectFile && ObjectFileRecords)
		{
		fwrite((char *)IdxFileID, strlen(IdxFileID) + 1, 1, ObjectFile);
		fwrite((char *)FileID, strlen(FileID) + 1, 1, ObjectFileRecords);
		fwrite((char *)&Super, sizeof (char), 1, ObjectFile);
		fwrite((char *)&Super, sizeof (char), 1, ObjectFileRecords);
		fwrite((char *)&Major, sizeof (char), 1, ObjectFile);
		fwrite((char *)&Major, sizeof (char), 1, ObjectFileRecords);
		fwrite((char *)&Minor, sizeof (char), 1, ObjectFile);
		fwrite((char *)&Minor, sizeof (char), 1, ObjectFileRecords);
		fwrite((char *)&Revision, sizeof (char), 1, ObjectFile);
		fwrite((char *)&Revision, sizeof (char), 1, ObjectFileRecords);
		fwrite((char *)&ByteOrder, sizeof (unsigned long), 1, ObjectFile);
		fwrite((char *)&ByteOrder, sizeof (unsigned long), 1, ObjectFileRecords);
		HeaderSize = ftell(ObjectFile) + sizeof (unsigned long);
		
		//#ifdef WCS_BYTEORDER_BIGENDIAN
		//SimpleEndianFlip32U(HeaderSize, &HeaderSize);
		//#endif // WCS_BYTEORDER_BIGENDIAN
		//if ((fwrite((char *)&HeaderSize, sizeof (unsigned long), 1, ObjectFile)) != 1)
		if(PutL32U(HeaderSize, ObjectFile))
			return (0);

		HeaderSize = ftell(ObjectFileRecords) + sizeof (unsigned long);
		//#ifdef WCS_BYTEORDER_BIGENDIAN
		//SimpleEndianFlip32U(HeaderSize, &HeaderSize);
		//#endif // WCS_BYTEORDER_BIGENDIAN
		//if ((fwrite((char *)&HeaderSize, sizeof (unsigned long), 1, ObjectFileRecords)) != 1)
		if(PutL32U(HeaderSize, ObjectFileRecords))
			return (0);

		#endif // ! WCS_SXACTIONQUERY_ASCIIFILE
		return (1);
		} // if
	} // if

return (0);

} // SXQueryItem::OpenObjectFile

/*===========================================================================*/

NameList *SXQueryItem::AddNewNameList(NameList **Names, char *NewName, long FileType)
{
NameList **ListPtr;

if (Names)
	{
	ListPtr = Names;
	while (*ListPtr)
		{
		if (! stricmp((*ListPtr)->Name, NewName) && FileType == (*ListPtr)->ItemClass)
			return (*ListPtr);
		ListPtr = &(*ListPtr)->Next;
		} // if
	return (*ListPtr = new NameList(NewName, FileType));
	} // if

return (NULL);

} // SXQueryItem::AddNewNameList

/*===========================================================================*/
/*===========================================================================*/

SXQueryActionList::SXQueryActionList(SXQueryAction *ActionSource)
{

RecordNum = -1;
RecordFreq = WCS_SXQUERYITEM_FREQ_UNKNOWN;
QueryAction = ActionSource;
Next = NULL;

} // SXQueryActionList::SXQueryActionList
		
/*===========================================================================*/
/*===========================================================================*/

SXQueryVectorList::SXQueryVectorList(Joe *VectorSource)
{

RecordNum = -1;
RecordFreq = WCS_SXQUERYITEM_FREQ_UNKNOWN;
QueryVector = VectorSource;
Next = NULL;

} // SXQueryActionList::SXQueryActionList
		
/*===========================================================================*/
/*===========================================================================*/

SXQueryEffectList::SXQueryEffectList(GeneralEffect *EffectSource)
{

RecordNum = -1;
RecordFreq = WCS_SXQUERYITEM_FREQ_UNKNOWN;
QueryEffect = EffectSource;
Next = NULL;

} // SXQueryEffectList::SXQueryEffectList
		
/*===========================================================================*/
/*===========================================================================*/

SXQueryVectorEffectList::SXQueryVectorEffectList(GeneralEffect *EffectSource, Joe *VectorSource)
{

RecordNum = -1;
RecordFreq = WCS_SXQUERYITEM_FREQ_UNKNOWN;
QueryEffect = EffectSource;
QueryVector = VectorSource;
Next = NULL;

} // SXQueryVectorEffectList::SXQueryVectorEffectList
