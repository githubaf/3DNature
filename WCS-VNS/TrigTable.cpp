// TrigTable.cpp
// Lookup tables for crude trig functions
// Built from Globemap.c
// on 11/29/95 by GRH
// Copyright 1995

#include "stdafx.h"
#include "TrigTable.h"
#include "UsefulMath.h"

CurveTable::CurveTable()
{
double X, Y;
int i, Idx;

Idx = 0;
for (i = 0; i < 128; i++, Idx++)
	{
	X = 1.0 - ((double)i / 127.0);
	Y = sqrt(1.0 - (X * X));
	Table[Idx] = (float)Y;
	} // for

for (i = 0; i < 128; i++, Idx++)
	{
	X = (double)i / 127.0;
	Y = sqrt(1.0 - (X * X));
	Table[Idx] = (float)Y;
	} // for

} // CurveTable::CurveTable

/*===========================================================================*/

double CurveTable::Lookup(unsigned char Value)
{

return((double)Table[Value]);

} // CurveTable::Lookup()

/*===========================================================================*/

ACosineTable::ACosineTable()
{
double Val, Interval;
long Entry;

Interval = 2.0 / (double)(TRIGTABLE_ENTRIES - 1);
for (Entry=0, Val=-1.0; Entry<TRIGTABLE_ENTRIES; Entry++, Val+=Interval)
	{
	Table[Entry] = (float)acos(Val);
	} // for

} // ACosineTable::ACosineTable()

/*===========================================================================*/

double ACosineTable::Lookup(double Value)
{
long Entry;

if ((Value > 1.0) || (Value < -1.0))
	return (0.0);

Value += 1.0;
Value *= 0.5;
Value *= (double)(TRIGTABLE_ENTRIES - 1);
Entry = quickftol(Value);

return ((double)Table[Entry]);

} // ACosineTable::Lookup()

/*===========================================================================*/

double ACosineTable::LookupLerped(double Value)
{
double ValFrac;
long Entry;

if ((Value > 1.0) || (Value < -1.0))
	return (0.0);

Value += 1.0;
Value *= 0.5;
Value *= (double)(TRIGTABLE_ENTRIES - 1);
Entry = quickftol(Value);
ValFrac = Value - (double)Entry;
if(ValFrac > 0 && Entry != (TRIGTABLE_ENTRIES - 1))
	{
	return(((double)Table[Entry] * (1.0 - ValFrac)) + ((double)Table[Entry + 1] * ValFrac)); // basic lerp from Entry to Entry+1
	} // if 

return ((double)Table[Entry]);

} // ACosineTable::LookupLerped()

/*===========================================================================*/

ASineTable::ASineTable()
{
double Val, Interval;
long Entry;

Interval = 2.0 / (double)(TRIGTABLE_ENTRIES - 1);
for (Entry=0, Val=-1.0; Entry<TRIGTABLE_ENTRIES; Entry++, Val+=Interval)
	{
	Table[Entry] = (float)asin(Val);
	} // for

} // ASineTable::ASineTable()

/*===========================================================================*/

double ASineTable::Lookup(double Value)
{
long Entry;

if ((Value > 1.0) || (Value < -1.0))
	return (0.0);

Value += 1.0;
Value *= 0.5;
Value *= (double)(TRIGTABLE_ENTRIES - 1);
Entry = quickftol(Value);

return ((double)Table[Entry]);

} // ASineTable::Lookup()

/*===========================================================================*/

double ASineTable::LookupLerped(double Value)
{
double ValFrac;
long Entry;

if ((Value > 1.0) || (Value < -1.0))
	return (0.0);

Value += 1.0;
Value *= 0.5;
Value *= (double)(TRIGTABLE_ENTRIES - 1);
Entry = quickftol(Value);
ValFrac = Value - (double)Entry;
if (ValFrac > 0 && Entry != (TRIGTABLE_ENTRIES - 1))
	{
	return(((double)Table[Entry] * (1.0 - ValFrac)) + ((double)Table[Entry + 1] * ValFrac)); // basic lerp from Entry to Entry+1
	} // if 

return ((double)Table[Entry]);

} // ASineTable::LookupLerped()

/*===========================================================================*/

MathAccellerators::MathAccellerators()
{

} // MathAccellerators::MathAccellerators
