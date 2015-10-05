// TrigTable.h
// Trig table objects
// Built from V1 Globemap.c on 11/29/95 by GRH and CXH.
// Copyright 1995

#ifndef WCS_TRIGTABLE_H
#define WCS_TRIGTABLE_H

#define TRIGTABLE_ENTRIES 361
#define CURVETABLE_ENTRIES 255
#define POW2TABLE_ENTRIES 65

class TrigTable
	{
	protected:
		float Table[TRIGTABLE_ENTRIES];
	}; // TrigTable

// This class does not appear to be used currently
class CurveTable
	{
	private:
		float Table[CURVETABLE_ENTRIES];
	public:
		CurveTable();
		double Lookup(unsigned char Value);
	}; //

class ACosineTable : private TrigTable
	{
	public:
		ACosineTable();

		double Lookup(double Value);
		double LookupLerped(double Value);

	}; // ACosineTable

class ASineTable : private TrigTable
	{
	public:
		ASineTable();

		double Lookup(double Value);
		double LookupLerped(double Value);

	}; // ASineTable

class MathAccellerators
	{
	public:
		CurveTable CurveTab; 
		ACosineTable ACosTab;
		ASineTable ASinTab;

		MathAccellerators();
	}; // class MathAccellerators

#endif // WCS_TRIGTABLE_H
