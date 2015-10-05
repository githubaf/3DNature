// Tables.h
// Header file for maintaining tables of the sort used by projections
// Created from scratch 12/30/00 by Gary R. Huber
// Copyright 2000 by 3D Nature. All rights reserved.

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_TABLES_H
#define WCS_TABLES_H

#include "PathAndFile.h"

union FieldValue
	{
	double Double;
	char *Text;	// this will point to the start of a string in LiveRecord
	long Long;
	}; // union FieldValue

enum
	{
	WCS_TABLEFIELD_DATATYPE_CHAR = 1,
	WCS_TABLEFIELD_DATATYPE_LONG,
	WCS_TABLEFIELD_DATATYPE_DOUBLE
	}; // data types

class TableField
	{
	public:
		short NameSize, TextSize, FieldSize;
		char DataType, DataValid;
		long FieldOffset;
		char *Name;
		union FieldValue Data;

		TableField();
		~TableField();
	}; // class TableField

class GenericTable
	{
	public:
		TableField *FieldValues;
		long NumFields, NumRecords, HeaderSize, RecordSize, LiveRecordNum;
		char *LiveRecord, Instantiated;

		GenericTable();
		~GenericTable();
		void FreeAll(void);
		virtual int Instantiate() = 0;
		virtual int FetchLiveRecord(long NewRecordNumber) = 0;
		int AllocLiveRecord(void);
		int AllocFieldValues(void);
		void FreeLiveRecord();
		void FreeFieldValues();
		long FindFieldByName(char *FieldName);	// returns field number, -1 if not found
		long FindRecordByFieldLong(char *FieldName, long MatchValue);	// returns record number, looks for a match between a field and supplied value, -1 if not found
		long FindRecordByFieldDbl(char *FieldName, double MatchValue);	// returns record number, looks for a match between a field and supplied value, -1 if not found
		long FindRecordByFieldStr(char *FieldName, char *MatchValue);	// returns record number, looks for a match between a field and supplied value, -1 if not found
		long FindRecordByFieldStrTrunc(char *FieldName, char *MatchValue);	// returns record number, looks for a match between a field and supplied value, -1 if not found
		int FetchFieldValueLong(long RecordNumber, long FieldNumber, long &Receiver);	// converts field value to long if necessary
		int FetchFieldValueDbl(long RecordNumber, long FieldNumber, double &Receiver);	// converts field value to double if necessary
		int FetchFieldValueStr(long RecordNumber, long FieldNumber, char *Receiver, long StrSize);	// converts field value to string if necessary
		int FetchFieldValueLong(long RecordNumber, char *FieldName, long &Receiver);	// converts field value to long if necessary
		int FetchFieldValueDbl(long RecordNumber, char *FieldName, double &Receiver);	// converts field value to double if necessary
		int FetchFieldValueStr(long RecordNumber, char *FieldName, char *Receiver, long StrSize);	// converts field value to string if necessary
		char FetchFieldDataType(long FieldNumber);						// returns type of data field or -1 if not found

	}; // class GenericTable

class DBFTable: public GenericTable
	{
	public:
		FILE *ffile;
		PathAndFile TablePAF;

		DBFTable();
		~DBFTable();
		void SetPathAndFile(char *NewPath, char *NewFile);
		virtual int InitFileName(void) = 0;
		int OpenFile();
		void CloseFile();
		int Instantiate();
		int FetchLiveRecord(long NewRecordNumber);
		int TransferLiveRecord(void);
		int ReadDBFHeader(void);

	}; // class DBFTable

class ProjectionSystemTable: public DBFTable
	{
	public:

		ProjectionSystemTable();
		int InitFileName();

	}; // class ProjectionSystemTable

class ProjectionZoneTable: public DBFTable
	{
	public:
		char TableName[64];

		ProjectionZoneTable();
		void SetName(char *NewName);
		char *GetName(void)	{return (TableName);};
		int InitFileName();

	}; // class ProjectionZoneTable

class ProjectionMethodTable: public DBFTable
	{
	public:

		ProjectionMethodTable();
		int InitFileName();

	}; // class ProjectionMethodTable

class ProjectionParameterTable: public DBFTable
	{
	public:

		ProjectionParameterTable();
		int InitFileName();

	}; // class ProjectionParameterTable

class DatumTable: public DBFTable
	{
	public:

		DatumTable();
		int InitFileName();

	}; // class DatumTable

class EllipsoidTable: public DBFTable
	{
	public:

		EllipsoidTable();
		int InitFileName();

	}; // class EllipsoidTable

class ExportTable: public DBFTable
	{
	public:

		ExportTable();
		int InitFileName();
		char IsFieldValueTrue(long RecordNumber, char *FieldName);
		char IsOptionSupported(long RecordNumber, char *FieldName, char *OptionName);
		int FetchDefaultOption(long RecordNumber, char *FieldName, char *OptionName);
		int FetchNextOption(long RecordNumber, char *FieldName, char *OptionName);

	}; // class ExportTable

#endif // WCS_TABLES_H
