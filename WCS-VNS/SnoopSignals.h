// SnoopSignals.h
// Defines for signal flags
// 10/13/99 FPW2
// Copyright 3D Nature 1999


#define ARC_VARIANT			(1<<1)

#define GET_ELEV_UNITS		(1<<13)		// vertical units need defining
#define GET_NSEW_UNITS		(1<<14)		// horizontal units need defining
#define DEAD_END			(1<<15)		// don't ever use this for anything else

#define FOUND_COLS			(1<<1)
#define FOUND_ROWS			(1<<2)
#define FOUND_XLL			(1<<3)
#define FOUND_YLL			(1<<4)
#define FOUND_CELLSIZE		(1<<5)
#define FOUND_BYTEORDER		(1<<6)
#define FOUND_BITS			(1<<7)
#define FOUND_TFWDATA		(1<<8)
#define FOUND_XDIM			(1<<9)
#define FOUND_YDIM			(1<<10)

#define FOUND_ULX			(1<<3)
#define FOUND_ULY			(1<<4)

#define FOUND_ALTW			(1<<3)

#define DXF_3DFACE			(1<<12)
