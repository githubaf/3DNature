// Useful.h
// Just generally useful code I thought I'd put in one file.
// Written from memory on 3/23/95 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_USEFUL_H
#define WCS_USEFUL_H

#include <stddef.h>
#include <exec/types.h>

#ifndef ROUNDUP
        // Do NOT make b a zero, that would be dumb.
        #define ROUNDUP(a,b)    (b * (a + (b - 1)) / b)

#endif // ROUNDUP


char *StripExtension(char *Source);

#endif // WCS_USEFUL_H

