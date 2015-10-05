// Tables.h
// Header file for maintaining tables of the sort used by projections
// Created from scratch 12/30/00 by Gary R. Huber
// Copyright 2000 by 3D Nature. All rights reserved.

#include "stdafx.h"
#include "Tables.h"
#include "AppMem.h"
#include "Useful.h"
#include "Application.h"

extern WCSApp *GlobalApp;

/*===========================================================================*/
/*===========================================================================*/

TableField::TableField()
{

NameSize = TextSize = FieldSize = 0;
DataType = 0;	// not a valid data type
DataValid = 0;
FieldOffset = 0;
Name = NULL;
Data.Double = 0.0;

} // TableField::TableField

/*===========================================================================*/

TableField::~TableField()
{

if (Data.Text && TextSize > 0)	// if Data.Text is a pointer to a block of memory someone else owns, TextSize will be 0
	AppMem_Free(Data.Text, TextSize);
if (Name && NameSize > 0)	// if Name is a pointer to a block of memory someone else owns, NameSize will be 0
	AppMem_Free(Name, NameSize);

} // TableField::~TableField

/*===========================================================================*/
/*===========================================================================*/

GenericTable::GenericTable()
{

FieldValues = NULL;
LiveRecord = NULL;
NumFields = NumRecords = RecordSize = HeaderSize = 0;
LiveRecordNum = -1;
Instantiated = 0;

} // GenericTable::GenericTable

/*===========================================================================*/

GenericTable::~GenericTable()
{

FreeAll();

} // GenericTable::~GenericTable

/*===========================================================================*/

void GenericTable::FreeAll(void)
{

FreeLiveRecord();
FreeFieldValues();
NumRecords = HeaderSize = 0;
Instantiated = 0;
LiveRecordNum = -1;

} // GenericTable::FreeAll

/*===========================================================================*/

int GenericTable::AllocLiveRecord(void)
{

if (LiveRecord)
	AppMem_Free(LiveRecord, RecordSize);
LiveRecord = NULL;
if (RecordSize > 0)
	LiveRecord = (char *)AppMem_Alloc(RecordSize, 0);

return (LiveRecord ? 1: 0);

} // GenericTable::AllocLiveRecord

/*===========================================================================*/

int GenericTable::AllocFieldValues(void)
{

if (FieldValues)
	delete [] FieldValues;
FieldValues = NULL;
if (NumFields > 0)
	FieldValues = new TableField[NumFields];

return (FieldValues ? 1: 0);

} // GenericTable::AllocFieldValues

/*===========================================================================*/

void GenericTable::FreeLiveRecord(void)
{

if (LiveRecord)
	AppMem_Free(LiveRecord, RecordSize);
LiveRecord = NULL;
RecordSize = 0;

} // GenericTable::FreeLiveRecord

/*===========================================================================*/

void GenericTable::FreeFieldValues(void)
{

if (FieldValues)
	delete [] FieldValues;
FieldValues = NULL;
NumFields = 0;

} // GenericTable::FreeFieldValues

/*===========================================================================*/

long GenericTable::FindFieldByName(char *FieldName)
{
long Ct;

if (! Instantiated)
	Instantiate();

if (FieldValues)
	{
	for (Ct = 0; Ct < NumFields; Ct ++)
		{
		if (FieldValues[Ct].Name)
			{
			if (! stricmp(FieldValues[Ct].Name, FieldName))
				return (Ct);
			} // if
		} // for
	} // if

return (-1);

} // GenericTable::FindFieldByName

/*===========================================================================*/

long GenericTable::FindRecordByFieldLong(char *FieldName, long MatchValue)
{	// returns record number, looks for a match between a field and supplied value, -1 if not found
long FieldNumber, RecordNumber = 0;

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	{
	while (FetchLiveRecord(RecordNumber))
		{
		if (FieldValues[FieldNumber].DataValid && FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_LONG
			&& FieldValues[FieldNumber].Data.Long == MatchValue)
			return (RecordNumber);
		RecordNumber ++;
		} // while
	} // if

return (-1);

} // GenericTable::FindRecordByFieldLong

/*===========================================================================*/

long GenericTable::FindRecordByFieldDbl(char *FieldName, double MatchValue)
{	// returns record number, looks for a match between a field and supplied value, -1 if not found
long FieldNumber, RecordNumber = 0;

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	{
	while (FetchLiveRecord(RecordNumber))
		{
		if (FieldValues[FieldNumber].DataValid && FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_DOUBLE
			&& FieldValues[FieldNumber].Data.Double == MatchValue)
			return (RecordNumber);
		RecordNumber ++;
		} // while
	} // if

return (-1);

} // GenericTable::FindRecordByFieldDbl

/*===========================================================================*/

long GenericTable::FindRecordByFieldStr(char *FieldName, char *MatchValue)
{	// returns record number, looks for a match between a field and supplied value, -1 if not found
long FieldNumber, RecordNumber = 0;

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	{
	while (FetchLiveRecord(RecordNumber))
		{
		if (FieldValues[FieldNumber].DataValid && FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_CHAR
			&& FieldValues[FieldNumber].Data.Text && (! stricmp(FieldValues[FieldNumber].Data.Text, MatchValue)))
			return (RecordNumber);
		RecordNumber ++;
		} // while
	} // if

return (-1);

} // GenericTable::FindRecordByFieldStr

/*===========================================================================*/

// same as FindRecordByFieldStr, but permits matching to truncated MatchValue
long GenericTable::FindRecordByFieldStrTrunc(char *FieldName, char *MatchValue)
{	// returns record number, looks for a match between a field and supplied value, -1 if not found
long FieldNumber, RecordNumber = 0;

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	{
	while (FetchLiveRecord(RecordNumber))
		{
		if (FieldValues[FieldNumber].DataValid && FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_CHAR
			&& FieldValues[FieldNumber].Data.Text && (! strnicmp(FieldValues[FieldNumber].Data.Text, MatchValue, strlen(MatchValue))))
			return (RecordNumber);
		RecordNumber ++;
		} // while
	} // if

return (-1);

} // GenericTable::FindRecordByFieldStrTrunc

/*===========================================================================*/

int GenericTable::FetchFieldValueLong(long RecordNumber, char *FieldName, long &Receiver)
{
long FieldNumber;

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	return (FetchFieldValueLong(RecordNumber, FieldNumber, Receiver));

return (0);

} // GenericTable::FetchFieldValueLong

/*===========================================================================*/

int GenericTable::FetchFieldValueDbl(long RecordNumber, char *FieldName, double &Receiver)
{
long FieldNumber;

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	return (FetchFieldValueDbl(RecordNumber, FieldNumber, Receiver));

return (0);

} // GenericTable::FetchFieldValueDbl

/*===========================================================================*/

int GenericTable::FetchFieldValueStr(long RecordNumber, char *FieldName, char *Receiver, long StrSize)
{
long FieldNumber;

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	return (FetchFieldValueStr(RecordNumber, FieldNumber, Receiver, StrSize));

return (0);

} // GenericTable::FetchFieldValueStr

/*===========================================================================*/

int GenericTable::FetchFieldValueLong(long RecordNumber, long FieldNumber, long &Receiver)
{

if (FetchLiveRecord(RecordNumber))
	{
	if (FieldNumber < NumFields && FieldNumber >= 0)
		{
		if (FieldValues[FieldNumber].DataValid)
			{
			if (FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_CHAR)
				{
				Receiver = atoi(FieldValues[FieldNumber].Data.Text);
				} // if
			else if (FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_LONG)
				{
				Receiver = FieldValues[FieldNumber].Data.Long;
				} // else if
			else if (FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_DOUBLE)
				{
				Receiver = (long)FieldValues[FieldNumber].Data.Double;
				} // else if
			return (1);
			} // if
		} // if
	} // if

return (0);

} // GenericTable::FetchFieldValueLong

/*===========================================================================*/

int GenericTable::FetchFieldValueDbl(long RecordNumber, long FieldNumber, double &Receiver)
{

if (FetchLiveRecord(RecordNumber))
	{
	if (FieldNumber < NumFields && FieldNumber >= 0)
		{
		if (FieldValues[FieldNumber].DataValid)
			{
			if (FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_CHAR)
				{
				Receiver = atof(FieldValues[FieldNumber].Data.Text);
				} // if
			else if (FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_LONG)
				{
				Receiver = (double)FieldValues[FieldNumber].Data.Long;
				} // else if
			else if (FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_DOUBLE)
				{
				Receiver = FieldValues[FieldNumber].Data.Double;
				} // else if
			return (1);
			} // if
		} // if
	} // if

return (0);

} // GenericTable::FetchFieldValueDbl

/*===========================================================================*/

int GenericTable::FetchFieldValueStr(long RecordNumber, long FieldNumber, char *Receiver, long StrSize)
{

if (FetchLiveRecord(RecordNumber))
	{
	if (FieldNumber < NumFields && FieldNumber >= 0)
		{
		if (FieldValues[FieldNumber].DataValid)
			{
			if (FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_CHAR)
				{
				strncpy(Receiver, FieldValues[FieldNumber].Data.Text, StrSize);
				Receiver[StrSize - 1] = 0;
				} // if
			else if (FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_LONG)
				{
				sprintf(Receiver, "%01d", FieldValues[FieldNumber].Data.Long);
				} // else if
			else if (FieldValues[FieldNumber].DataType == WCS_TABLEFIELD_DATATYPE_DOUBLE)
				{
				sprintf(Receiver, "%f", FieldValues[FieldNumber].Data.Double);
				} // else if
			return (1);
			} // if
		} // if
	} // if

return (0);

} // GenericTable::FetchFieldValueStr

/*===========================================================================*/
/*===========================================================================*/

DBFTable::DBFTable()
{

ffile = NULL;

} // DBFTable::DBFTable

/*===========================================================================*/

DBFTable::~DBFTable()
{

CloseFile();

} // DBFTable::~DBFTable

/*===========================================================================*/

void DBFTable::CloseFile(void)
{

if (ffile)
	fclose(ffile);
ffile = NULL;

} // DBFTable::CloseFile

/*===========================================================================*/

int DBFTable::OpenFile(void)
{
char FullName[512];

if (ffile)
	return (1);

strmfp(FullName, (char *)TablePAF.GetPath(), (char *)TablePAF.GetName());
if (ffile = fopen(FullName, "rb"))
	return (1);

return (0);

} // DBFTable::OpenFile

/*===========================================================================*/

void DBFTable::SetPathAndFile(char *NewPath, char *NewFile)
{

TablePAF.SetPathAndName(NewPath, NewFile);

} // DBFTable::SetPathAndFile

/*===========================================================================*/

int DBFTable::Instantiate(void)
{
int Success = 0;

FreeAll();

if (InitFileName())
	{
	if (OpenFile())
		{
		// read header and create field value array
		if (ReadDBFHeader())
			{
			// allocate LiveRecord and read first record
			if (AllocLiveRecord())
				{
				Instantiated = 1;
				if (FetchLiveRecord(0))
					{
					Success = 1;
					} // if
				} // if
			} // if
		// close the file if it is still open
		CloseFile();
		} // if
	} // if

return (Success);

} // DBFTable::Instantiate

/*===========================================================================*/

int DBFTable::FetchLiveRecord(long NewRecordNumber)
{
int Success = 1;

if (! Instantiated)
	Instantiate();

if (LiveRecord)
	{
	if (LiveRecordNum != NewRecordNumber)
		{
		if (NewRecordNumber < NumRecords && NewRecordNumber >= 0)
			{
			if (OpenFile())
				{
				// seek to correct file position
				if (! fseek(ffile, HeaderSize + RecordSize * NewRecordNumber, SEEK_SET))
					{
					// read a record from file
					if (fread(LiveRecord, RecordSize, 1, ffile) == 1)
						{
						// transfer values to FieldValues array
						if (TransferLiveRecord())
							LiveRecordNum = NewRecordNumber;
						else
							Success = 0;
						} // if read successful
					else
						Success = 0;
					} // if seek succeded
				else
					Success = 0;
				CloseFile();
				} // if
			else
				Success = 0;
			} // if
		else
			Success = 0;
		} // if need new record
	} // if
else
	Success = 0;

return (Success);

} // DBFTable::FetchLiveRecord

/*===========================================================================*/

int DBFTable::TransferLiveRecord(void)
{
long Ct;
char Temp[64];

if (LiveRecord && FieldValues)
	{
	for (Ct = 0; Ct < NumFields; Ct ++)
		{
		if (FieldValues[Ct].DataType == WCS_TABLEFIELD_DATATYPE_CHAR)
			{
			// this will always copy 1 less than the Text array size and the last byte is initialized to 0
			strncpy(FieldValues[Ct].Data.Text, &LiveRecord[FieldValues[Ct].FieldOffset], FieldValues[Ct].FieldSize);
			TrimTrailingSpaces(FieldValues[Ct].Data.Text);
			// dbf files fill empty fields with blanks, if the field is blank we consider it invalid data
			if (FieldValues[Ct].Data.Text[0])
				FieldValues[Ct].DataValid = 1;
			else
				FieldValues[Ct].DataValid = 0;
			} // if
		else if (FieldValues[Ct].DataType == WCS_TABLEFIELD_DATATYPE_LONG)
			{
			memcpy(Temp, &LiveRecord[FieldValues[Ct].FieldOffset], FieldValues[Ct].FieldSize);
			Temp[FieldValues[Ct].FieldSize] = 0;
			// dbf files fill empty fields with blanks, if the field is blank we consider it invalid data
			TrimTrailingSpaces(Temp);
			if (Temp[0])
				{
				FieldValues[Ct].DataValid = 1;
				FieldValues[Ct].Data.Long = atoi(Temp);
				} // if
			else
				FieldValues[Ct].DataValid = 0;
			} // else if
		else if (FieldValues[Ct].DataType == WCS_TABLEFIELD_DATATYPE_DOUBLE)
			{
			memcpy(Temp, &LiveRecord[FieldValues[Ct].FieldOffset], FieldValues[Ct].FieldSize);
			Temp[FieldValues[Ct].FieldSize] = 0;
			// dbf files fill empty fields with blanks, if the field is blank we consider it invalid data
			TrimTrailingSpaces(Temp);
			if (Temp[0])
				{
				FieldValues[Ct].DataValid = 1;
				FieldValues[Ct].Data.Double = atof(Temp);
				} // if
			else
				FieldValues[Ct].DataValid = 0;
			} // else if
		} // for
	return (1);
	} // if

return (0);

} // DBFTable::TransferLiveRecord

/*===========================================================================*/

int DBFTable::ReadDBFHeader(void)
{
unsigned char Version;
char FieldData[32];
long FieldsRead, CurOffset;
int Success = 1, FileOpened = 0;
unsigned short HeaderSize16, RecordSize16;

FieldData[0] = 0;
FreeFieldValues();
NumFields = 0;

if (ffile || (FileOpened = OpenFile()))
	{
	if (((fread((char *)&Version, 1, 1, ffile)) == 1) && Version == 3)
		{
		if (! fseek(ffile, 4, SEEK_SET))
			{
			if (((fread((char *)&NumRecords, 4, 1, ffile)) == 1) && NumRecords)
				{
				#ifdef BYTEORDER_BIGENDIAN
				SimpleEndianFlip32S(NumRecords, &NumRecords);
				#endif // BYTEORDER_BIGENDIAN
				if (((fread((char *)&HeaderSize16, 2, 1, ffile)) == 1) && HeaderSize16)
					{
					#ifdef BYTEORDER_BIGENDIAN
					SimpleEndianFlip16U(HeaderSize16, &HeaderSize16);
					#endif // BYTEORDER_BIGENDIAN
					HeaderSize = HeaderSize16;
					if (((fread((char *)&RecordSize16, 2, 1, ffile)) == 1) && RecordSize16)
						{
						#ifdef BYTEORDER_BIGENDIAN
						SimpleEndianFlip16U(RecordSize16, &RecordSize16);
						#endif // BYTEORDER_BIGENDIAN
						RecordSize = RecordSize16;
						if (! fseek(ffile, 32, SEEK_SET))
							{
							while (FieldData[0] != 0x0d)
								{
								if (((fread((char *)FieldData, 32, 1, ffile)) == 1))
									{
									if (FieldData[0] != 0x0d)
										NumFields ++;
									} // if
								else
									{
									Success = 0;
									break;
									} // else
								} // while
							} // if
						else
							Success = 0;
						if (Success && AllocFieldValues())
							{
							if (! fseek(ffile, 32, SEEK_SET))
								{
								CurOffset = 1;
								for (FieldsRead = 0; FieldsRead < NumFields; FieldsRead ++)
									{
									if (((fread((char *)FieldData, 32, 1, ffile)) == 1))
										{
										// transfer data to FieldValues[FieldsRead]
										FieldValues[FieldsRead].FieldOffset = CurOffset;
										FieldValues[FieldsRead].FieldSize = (unsigned char)FieldData[16];
										CurOffset += FieldValues[FieldsRead].FieldSize;
										FieldValues[FieldsRead].NameSize = (short)(strlen(FieldData) + 1);
										if (FieldValues[FieldsRead].Name = (char *)AppMem_Alloc(FieldValues[FieldsRead].NameSize, APPMEM_CLEAR))
											strncpy(FieldValues[FieldsRead].Name, FieldData, FieldValues[FieldsRead].NameSize - 1);
										else
											{
											Success = 0;
											break;
											} // else
										if (FieldData[11] == 'C')
											{
											FieldValues[FieldsRead].TextSize = FieldValues[FieldsRead].FieldSize + 1;
											if (FieldValues[FieldsRead].Data.Text = (char *)AppMem_Alloc(FieldValues[FieldsRead].TextSize, APPMEM_CLEAR))
												{
												FieldValues[FieldsRead].DataType = WCS_TABLEFIELD_DATATYPE_CHAR;
												} // if
											} // if
										else if (FieldData[11] == 'N')
											{
											if (FieldData[17])	// indicates decimal places
												FieldValues[FieldsRead].DataType = WCS_TABLEFIELD_DATATYPE_DOUBLE;
											else
												FieldValues[FieldsRead].DataType = WCS_TABLEFIELD_DATATYPE_LONG;
											} // else if
										else if (FieldData[11] == 'F')
											{
											FieldValues[FieldsRead].DataType = WCS_TABLEFIELD_DATATYPE_DOUBLE;
											} // else if
										} // if
									else
										{
										Success = 0;
										break;
										} // else
									} // for
								} // if
							else
								Success = 0;
							} // if
						else
							Success = 0;
						} // if
					else
						Success = 0;
					} // if
				else
					Success = 0;
				} // if
			else
				Success = 0;
			} // if
		else
			Success = 0;
		} // if
	else
		Success = 0;
	if (FileOpened)
		CloseFile();
	} // if
else
	Success = 0;

return (Success);

} // DBFTable::ReadDB3LayerTable

/*===========================================================================*/
/*===========================================================================*/

ProjectionSystemTable::ProjectionSystemTable()
{

} // ProjectionSystemTable::ProjectionSystemTable

/*===========================================================================*/

int ProjectionSystemTable::InitFileName(void)
{
char PathPart[256];

if (GlobalApp)
	{
	strmfp(PathPart, GlobalApp->GetProgDir(), "Tools");
	SetPathAndFile(PathPart, "ProjectionSystems.dbf");
	return (1);
	} // if

return (0);

} // ProjectionSystemTable::InitFileName

/*===========================================================================*/
/*===========================================================================*/

ProjectionZoneTable::ProjectionZoneTable()
{

TableName[0] = 0;

} // ProjectionZoneTable::ProjectionZoneTable

/*===========================================================================*/

void ProjectionZoneTable::SetName(char *NewName)
{

strncpy(TableName, NewName, 64);
TableName[63] = 0;

} // ProjectionZoneTable::SetName

/*===========================================================================*/

int ProjectionZoneTable::InitFileName(void)
{
char PathPart[256];

if (GlobalApp)
	{
	strmfp(PathPart, GlobalApp->GetProgDir(), "Tools");
	SetPathAndFile(PathPart, TableName);
	return (1);
	} // if

return (0);

} // ProjectionZoneTable::InitFileName

/*===========================================================================*/
/*===========================================================================*/

ProjectionMethodTable::ProjectionMethodTable()
{

} // ProjectionMethodTable::ProjectionMethodTable

/*===========================================================================*/

int ProjectionMethodTable::InitFileName(void)
{
char PathPart[256];

if (GlobalApp)
	{
	strmfp(PathPart, GlobalApp->GetProgDir(), "Tools");
	SetPathAndFile(PathPart, "ProjectionMethods.dbf");
	return (1);
	} // if

return (0);

} // ProjectionMethodTable::InitFileName

/*===========================================================================*/
/*===========================================================================*/

ProjectionParameterTable::ProjectionParameterTable()
{

} // ProjectionParameterTable::ProjectionParameterTable

/*===========================================================================*/

int ProjectionParameterTable::InitFileName(void)
{
char PathPart[256];

if (GlobalApp)
	{
	strmfp(PathPart, GlobalApp->GetProgDir(), "Tools");
	SetPathAndFile(PathPart, "ProjectionParams.dbf");
	return (1);
	} // if

return (0);

} // ProjectionParameterTable::InitFileName

/*===========================================================================*/
/*===========================================================================*/

DatumTable::DatumTable()
{

} // DatumTable::DatumTable

/*===========================================================================*/

int DatumTable::InitFileName(void)
{
char PathPart[256];

if (GlobalApp)
	{
	strmfp(PathPart, GlobalApp->GetProgDir(), "Tools");
	SetPathAndFile(PathPart, "Datums.dbf");
	return (1);
	} // if

return (0);

} // DatumTable::InitFileName

/*===========================================================================*/
/*===========================================================================*/

EllipsoidTable::EllipsoidTable()
{

} // EllipsoidTable::EllipsoidTable

/*===========================================================================*/

int EllipsoidTable::InitFileName(void)
{
char PathPart[256];

if (GlobalApp)
	{
	strmfp(PathPart, GlobalApp->GetProgDir(), "Tools");
	SetPathAndFile(PathPart, "Ellipsoids.dbf");
	return (1);
	} // if

return (0);

} // EllipsoidTable::InitFileName

/*===========================================================================*/
/*===========================================================================*/

ExportTable::ExportTable()
{

} // ExportTable::ExportTable

/*===========================================================================*/

int ExportTable::InitFileName(void)
{
char PathPart[256];

if (GlobalApp)
	{
	strmfp(PathPart, GlobalApp->GetProgDir(), "Tools");
	SetPathAndFile(PathPart, "ExportTargets.dbf");
	return (1);
	} // if

return (0);

} // ExportTable::InitFileName

/*===========================================================================*/

// tests to see if a field value starts with letter y, Y (yes) or a non-zero digit.
char ExportTable::IsFieldValueTrue(long RecordNumber, char *FieldName)
{
long FieldNumber;
char Receiver[128];

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	{
	if (FetchFieldValueStr(RecordNumber, FieldNumber, Receiver, 128))
		{
		if (Receiver[0] && (Receiver[0] == 'y' || Receiver[0] == 'Y' || atoi(Receiver) != 0)) 
			return (1);
		} // if
	} // if field exists

return (0);

} // ExportTable::IsFieldValueTrue

/*===========================================================================*/

// tests for case insensitive complete match between OptionName and any alphanumeric substrings in field value
char ExportTable::IsOptionSupported(long RecordNumber, char *FieldName, char *OptionName)
{
long FieldNumber, StrLen, NextSeg, Ct;
char Receiver[128];

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	{
	if (FetchFieldValueStr(RecordNumber, FieldNumber, Receiver, 128))
		{
		StrLen = (long)strlen(Receiver);
		Ct = StrLen - 1;
		if ((Receiver[Ct] < 48 && Receiver[Ct] != ' ' && Receiver[Ct] != '(' && Receiver[Ct] != ')' && Receiver[Ct] != '.') || Receiver[Ct] > 122 || (Receiver[Ct] > 90 && Receiver[Ct] < 97)
			|| (Receiver[Ct] > 57 && Receiver[Ct] < 65))
			{
			Receiver[Ct] = 0;
			StrLen --;
			} // if non-alpha final character
		NextSeg = 0;
		if (StrLen > 0)
			{
			for (Ct = 0; Ct <= StrLen; Ct ++)
				{
				if ((Receiver[Ct] < 48 && Receiver[Ct] != ' ' && Receiver[Ct] != '(' && Receiver[Ct] != ')' && Receiver[Ct] != '.') || Receiver[Ct] > 122 || (Receiver[Ct] > 90 && Receiver[Ct] < 97)
					|| (Receiver[Ct] > 57 && Receiver[Ct] < 65))
					{
					Receiver[Ct] = 0;
					if (! stricmp(&Receiver[NextSeg], OptionName))
						return (1);
					NextSeg = Ct + 1;
					} // if
				} // for
			} // if
		} // if
	} // if field exists

return (0);

} // ExportTable::IsOptionSupported

/*===========================================================================*/

// tests for case insensitive complete match between OptionName and any alphanumeric substrings in field value
int ExportTable::FetchDefaultOption(long RecordNumber, char *FieldName, char *OptionName)
{
long FieldNumber, StrLen, Ct;
char Receiver[128];

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	{
	if (FetchFieldValueStr(RecordNumber, FieldNumber, Receiver, 128))
		{
		StrLen = (long)strlen(Receiver);
		Ct = StrLen - 1;
		if ((Receiver[Ct] < 48 && Receiver[Ct] != ' ' && Receiver[Ct] != '(' && Receiver[Ct] != ')' && Receiver[Ct] != '.')
			|| Receiver[Ct] > 122 || (Receiver[Ct] > 90 && Receiver[Ct] < 97)
			|| (Receiver[Ct] > 57 && Receiver[Ct] < 65))
			{
			Receiver[Ct] = 0;
			StrLen --;
			} // if non-alpha final character
		if (StrLen > 0)
			{
			for (Ct = 0; Ct <= StrLen; Ct ++)
				{
				if ((Receiver[Ct] < 48 && Receiver[Ct] != ' ' && Receiver[Ct] != '(' && Receiver[Ct] != ')' && Receiver[Ct] != '.') || Receiver[Ct] > 122 || (Receiver[Ct] > 90 && Receiver[Ct] < 97)
					|| (Receiver[Ct] > 57 && Receiver[Ct] < 65))
					{
					Receiver[Ct] = 0;
					strcpy(OptionName, Receiver);
					return (1);
					} // if
				} // for
			} // if
		} // if
	} // if field exists

return (0);

} // ExportTable::FetchDefaultOption

/*===========================================================================*/

// tests for case insensitive complete match between OptionName and any alphanumeric substrings in field value
int ExportTable::FetchNextOption(long RecordNumber, char *FieldName, char *OptionName)
{
long FieldNumber, StrLen, NextSeg, Ct, Found = 0;
char Receiver[128];

if ((FieldNumber = FindFieldByName(FieldName)) >= 0)
	{
	if (FetchFieldValueStr(RecordNumber, FieldNumber, Receiver, 128))
		{
		StrLen = (long)strlen(Receiver);
		Ct = StrLen - 1;
		if ((Receiver[Ct] < 48 && Receiver[Ct] != ' ' && Receiver[Ct] != '(' && Receiver[Ct] != ')' && Receiver[Ct] != '.') || Receiver[Ct] > 122 || (Receiver[Ct] > 90 && Receiver[Ct] < 97)
			|| (Receiver[Ct] > 57 && Receiver[Ct] < 65))
			{
			Receiver[Ct] = 0;
			StrLen --;
			} // if non-alpha final character
		NextSeg = 0;
		if (StrLen > 0)
			{
			for (Ct = 0; Ct <= StrLen; Ct ++)
				{
				if ((Receiver[Ct] < 48 && Receiver[Ct] != ' ' && Receiver[Ct] != '(' && Receiver[Ct] != ')' && Receiver[Ct] != '.') || Receiver[Ct] > 122 || (Receiver[Ct] > 90 && Receiver[Ct] < 97)
					|| (Receiver[Ct] > 57 && Receiver[Ct] < 65))
					{
					Receiver[Ct] = 0;
					if (Found || ! OptionName[0])
						{
						strcpy(OptionName, &Receiver[NextSeg]);
						return (1);
						} // if
					if (! stricmp(&Receiver[NextSeg], OptionName))
						Found = 1;
					NextSeg = Ct + 1;
					} // if
				} // for
			} // if
		} // if
	} // if field exists

return (0);

} // ExportTable::FetchNextOption

/*===========================================================================*/
