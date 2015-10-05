// PoolString.h
// Pool-allocated string object
// Written from scratch on 3/23/95 by Chris "Xenon" Hanson
// Copyright 1995

class PoolString;

#ifndef WCS_POOLSTRING_H
#define WCS_POOLSTRING_H

#define POOLSTRING_ROUNDUP	16		// bytes

class PoolString
	{
	private:
		// int AllocSize;
		char *GetStrMem(int);
		void FreeStrMem(char *);
		//static char NullChar;
		char NullChar;

	public:
		char *StrData;
		PoolString(void);
		PoolString(unsigned int StrLen);
		PoolString(const char *Source);
		unsigned char Valid(void);
		void Clear(void);
		char *Set(const char *Source);
		~PoolString(void);
	}; // PoolString

#endif // WCS_POOLSTRING_H
