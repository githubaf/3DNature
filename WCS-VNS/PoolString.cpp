// PoolString.cpp
// Code for pool-allocated string object
// Written from scratch on 03/23/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"
#include "PoolString.h"
#include "Useful.h"

char *PoolString::GetStrMem(int MemSize)
{
return((char *)calloc(1, 1 + ROUNDUP(MemSize, POOLSTRING_ROUNDUP)));
} // GetStrMem

void PoolString::FreeStrMem(char *StringMem)
{
if(StringMem)
	{
	free(StringMem);
	} // if

} //PoolString::FreeStrMem


PoolString::PoolString(void)
{
this->NullChar = NULL;
StrData = &this->NullChar;
} // PoolString:PoolString(void)

PoolString::PoolString(unsigned int StrLen)
{

StrData = GetStrMem(StrLen);
if(StrData == NULL)
	{
	StrData = &NullChar;
	} // allocation unsuccessful

} // PoolString::PoolString(StrLen)

PoolString::PoolString(const char *Source)
{
StrData = GetStrMem((int)strlen(Source));

if(StrData)
	{
	strcpy(StrData, Source);
	} // if Valid
else
	{
	StrData = &NullChar;
	} // not Valid
} // PoolString::PoolString(Source)

unsigned char PoolString::Valid(void)
{
return((StrData != NULL) && (StrData != &NullChar));
} // PoolString::Valid

void PoolString::Clear(void)
{
if(Valid())
	{
	FreeStrMem(StrData);
	StrData = &NullChar;
	} // if StrData
} // PoolString::Clear

char *PoolString::Set(const char *Source)
{
if(Valid())
	{
	if(strlen(Source) < ROUNDUP(strlen(StrData), POOLSTRING_ROUNDUP))
		{
		strcpy(StrData, Source);
		return(StrData);
		} // if allocated is long enough for new data
	else
		{
		FreeStrMem(StrData);
		} // not enough room, free before re-allocating
	} // if

if(StrData = GetStrMem((int)strlen(Source)))
	{
	strcpy(StrData, Source);
	return(StrData);
	} // if allocated OK
else
	{
	StrData = &NullChar;
	return(NULL);
	} // failure
} // PoolString::Set


PoolString::~PoolString(void)
{
if(Valid())
	{
	FreeStrMem(StrData);
	//StrData = NULL;
	} // if Valid
} // PoolString::~PoolString()
