// Useful.h
// Just generally useful code I thought I'd put in one file.
// Written from memory on 3/23/95 by Chris "Xenon" Hanson
// Copyright 1995

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_USEFUL_H
#define WCS_USEFUL_H

// All this stuff has now been spread through a bunch of individual files
// Useful.h picks them all up, for convenience in existing code.
// New code can use Useful.h or the individual headers as preferrable
#include "UsefulArray.h"
#include "UsefulClip.h"
#include "UsefulPathString.h"
#include "UsefulTime.h"
#include "UsefulMath.h"
#include "UsefulGeo.h"
#include "UsefulIO.h"
#include "UsefulEndian.h"
#include "UsefulUnit.h"
#include "UsefulSwap.h"
#include "UsefulColor.h"
#include "UsefulCPU.h"

//#include <stddef.h>
//#include <stdio.h>
//#include <limits.h>

#undef RGB // stupid Microsoft makes macros with hazardously-common names

// define FASTCALL if Microsoft compiler & generating Win32 code
#ifndef FASTCALL
#if defined(_MSC_VER) && defined(_WIN32)
#define FASTCALL __fastcall
#else // _MSC_VER
#define FASTCALL
#endif // _MSC_VER
#endif // FASTCALL

// Useful debugging output macro, disappears in release build
#ifdef DEBUG
#ifdef _WIN32
#define DEBUGOUT(a) OutputDebugString(a)
#endif // _WIN32
#else // !DEBUG
// NULL expression
#define DEBUGOUT(a) 
#endif // !DEBUG

#endif // WCS_USEFUL_H

// Overview of where everyone ended up:

/*

UsefulArray
---------------------------------------
long *Zip_New(long Width, long Height);
void Zip_Del(long *Me, long Width, long Height);
double ArrayPointExtract(float *Data, double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, long Cols, long Rows);
double ArrayPointExtract(short *Data, double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, long Cols, long Rows);


UsefulClip
---------------------------------------
class VectorClipper
		short LowX, HighX, LowY, HighY;
		VectorClipper() {LowX = LowY = 0; HighX = HighY = SHRT_MAX;};
		void SetClipBounds(DrawingFenetre *DF);
		void MinimizeClipBounds(int X1, int Y1, int X2, int Y2);
		int ClipSeg(double &XS, double &YS, double &XE, double &YE);


UsefulColor
---------------------------------------
struct AICMYKcolor ToCMYK(const struct AIRGBcolor &rgb);
void HSVtoRGB(double *HSV, double *RGB);
void RGBtoHSV(double *HSV, double *RGB);
void ScaleHSV(double *RGB, double RotateHue, double AddSaturation, double AddValue);


UsefulCPU
---------------------------------------
unsigned long SSE2Supported(void);


UsefulEndian
---------------------------------------
       void BlindSimpleEndianFlip64 (        void *Source64, void *Dest64);
       void BlindSimpleEndianFlip32F(        void *Source32, void *Dest32);
//       void SimpleEndianFlip64 (            double Source64, double *Dest64);
//       void SimpleEndianFlip32F(             float Source32, float  *Dest32);
       void SimpleEndianFlip64 (             void *Source64, double *Dest64);	// it isn't safe to load a double/float before
       void SimpleEndianFlip32F(             void *Source32, float  *Dest32);	// it's been normalized
       void SimpleEndianFlip32U( unsigned long Source32, unsigned long *Dest32);
       void SimpleEndianFlip32S(   signed long Source32, signed long *Dest32);
inline void SimpleEndianFlip16U(unsigned short Source16, unsigned short *Dest16) {(*Dest16) = (unsigned short)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );};
inline void SimpleEndianFlip16S(  signed short Source16, signed short *Dest16) {(*Dest16) = (  signed short)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );};

BSWAP_W(w)
BSWAP_L(L)


UsefulGeo
---------------------------------------
void DecimalToDMS(double GeoDecimal, double &Degrees, double &Minutes, double &Seconds);
double DMSToDecimal(double Degrees, double Minutes, double Seconds);
double DecimalToPackedDMS(double GeoDecimal);
double DegMinSecToDegrees(char *str);
inline double LatScale(double SphereRad) {return(Pi * (SphereRad + SphereRad) * (1.0 / 360.0));};
double LonScale(double SphereRad, double Latitude);
double FindDistance(double Lat1, double Lon1, double Lat2, double Lon2, double PlanetRad);
void MakeTempCart(double Lat, double Lon, double SphereRad,	double *X, double *Y, double *Z);
double SolveDistCart(double XJ, double YJ, double ZJ, double XK, double YK, double ZK);
double SolveArcAng(double CartDist, double SphereRad);
double ConvertMetersToDeg(double MeterVal, double Latitude, double GlobeRad);
inline double CalcExag(double Elev, double Datum, double Exag)
inline double UnCalcExag(double Elev, double Datum, double Exag)


UsefulIO
---------------------------------------
double			Get64(char byteorder, FILE *fp);
float			Get32F(char byteorder, FILE *fp);
unsigned long	Get32U(char byteorder, FILE *fp);
long			Get32S(char byteorder, FILE *fp);
unsigned short	Get16U(char byteorder, FILE *fp);
short			Get16S(char byteorder, FILE *fp);
double			GetB64(FILE *fp);
float			GetB32F(FILE *fp);
unsigned long	GetB32U(FILE *fp);
long			GetB32S(FILE *fp);
unsigned short	GetB16U(FILE *fp);
short			GetB16S(FILE *fp);
double			GetL64(FILE *fp);
float			GetL32F(FILE *fp);
unsigned long	GetL32U(FILE *fp);
long			GetL32S(FILE *fp);
unsigned short	GetL16U(FILE *fp);
short			GetL16S(FILE *fp);
int	Put64(char byteorder, double *dval, FILE *fp);
int	Put32F(char byteorder, float *fval, FILE *fp);
int Put32U(char byteorder, unsigned long ulval, FILE *fp);
int	Put32S(char byteorder, long lval, FILE *fp);
int	Put16U(char byteorder, unsigned short usval, FILE *fp);
int	Put16S(char byteorder, short sval, FILE *fp);
int PutB64(double *dval, FILE *fp);
int	PutB32F(float *fval, FILE *fp);
int	PutB32U(unsigned long ulval, FILE *fp);
int	PutB32S(long lval,  FILE *fp);
int	PutB16U(unsigned short usval, FILE *fp);
int	PutB16S(short sval, FILE *fp);
int	PutL64(double *dval, FILE *fp);
int	PutL32F(float *fval, FILE *fp);
int	PutL32U(unsigned long ulval, FILE *fp);
int	PutL32S(long lval, FILE *fp);
int	PutL16U(unsigned short usval, FILE *fp);
int	PutL16S(short sval, FILE *fp);
unsigned long ReadBlock(FILE *ffile, char *Block, unsigned long Flags, short ByteFlip);
unsigned long WriteBlock(FILE *ffile, char *Block, unsigned long Flags);
unsigned long PrepWriteBlock(FILE *ffile, unsigned long ItemTag, unsigned long SizeSize,
		unsigned long SizeType, unsigned long FieldSize, unsigned long FieldType, char *FieldAddr);
unsigned long ReadLongBlock(FILE *ffile, char *Block, unsigned long SizeBlock);
unsigned long WriteLongBlock(FILE *ffile, char *Block, unsigned long SizeBlock);
unsigned long PrepWriteLongBlock(FILE *ffile, unsigned long ItemTag, unsigned long SizeSize,
		unsigned long SizeType, unsigned long FieldSize, unsigned long FieldType, char *FieldAddr);


UsefulMath
---------------------------------------
ROUNDUP(a,b)
double inline PERLIN_clamp(double x, double a, double b) {return (x <= a ? a: x >= b ? b: x);};
PERLIN_lerp(t, a, b)
PERLIN_s_curve(t)
lerp(t, a, b)
inline double flerp(double t, double a, double b) {return( a + t * (b - a) );}
MAX3(a, b, c)
MIN3(a, b, c)
MID3(a, b, c)
inline double WCS_floor(double x)	{return((double)((int)(x) - (int)((x) < 0.0 && (x) != (int)(x))  ));};
inline double WCS_ceil(double x)	{return((double)((int)(x) + (int)((x) > 0.0)  ));};
inline double WCS_round(double x)	{return(WCS_floor(x + .5));};
inline double WCS_max(double x, double y)	{return((x) > (y) ? (x): (y));};
inline double WCS_min(double x, double y)	{return((x) < (y) ? (x): (y));};
double FresnelReflectionCoef(double CosAngle, double RefrRatio);
double Round(double InVal, short Precision);
void StandardNormalDistributionInit(void);
double StandardNormalDistribution(double K);


UsefulPathString
---------------------------------------
unsigned long MAKE_ID(long a, long b, long c, long d);
unsigned long MakeIDFromString(const char *IDStr);
char *StripExtension(char *Source);
char *AddExtension(char *Source, char *AddExt);
char *EscapeBackslashes(char *InputPath);
char *UnifyPathGlyphs(char *Path);
char *ForceAllPathGlyphs(char *Path, char Glyph);
char *strupr(char *Source);
char *strlwr(char *Source);
char *strncpyupr(char *Dest, const char *Source, size_t Len);
char *strncpylwr(char *Dest, const char *Source, size_t Len);
char *strdoublenullcopy(char *Dest, const char *Source);
char *fgetline( char *string, int n, FILE *stream, char TerminateOnNull = 0, char ReturnLineIfEOF = 0);
void BreakFileName(char *FullName, char *PathPart, int PathSize, char *FilePart, int FileSize);
char *ScanToNextWord(char *String);
char *FindFileExtension(char *String);
char *InsertNameNum(char *DestName, const char *NumStr, int Append = 1);
char *InsertNameNumDigits(char *DestName, unsigned int Number, int Digits);
char MatchChar(const char CheckChar, const char *MatchList);
char *SkipPastNextSpace(const char *InStr);
char *SkipSpaces(const char *InStr);
char *TrimTrailingSpaces(char *String);
char *TrimTrailingDigits(char *String);
void TrimZeros(char *String);
void TrimDecimalZeros(char *String);
int CountIntDigits(const char *String);
int CountDecDigits(const char *String);
char *strovly(char *Dest, char *Src);
char *MakeCompletePath(char *Dest, char *Base, char *ObjName, char *ObjExt);
char *MakeNonPadPath(char *Dest, char *Base, char *ObjName, char *ObjExt);
int atoisub(const char *string, unsigned short first, unsigned short last, unsigned char style);
int ismark(int c);
int EscapeURI(char *Inbuf, char *Outbuf, int OutbufSize);


UsefulSwap
---------------------------------------
void swmem(void *a, void *b, unsigned n);
ARNIE_SWAP(a,b)
inline void Swap64(double &A, double &B) {register double C; C=A;A=B;B=C;};
inline void Swap32S(signed int &A, signed int &B) {register signed int C; C=A;A=B;B=C;};
inline void Swap32U(unsigned int &A, unsigned int &B) {register unsigned int C; C=A;A=B;B=C;};
inline void Swap32F(float &A, float &B) {register float C; C=A;A=B;B=C;};
inline void SwapV(void **A, void **B) {register void *C; C=*A;*A=*B;*B=C;};
inline void Swap16S(signed short &A, signed short &B) {register signed short C; C=A;A=B;B=C;};
inline void Swap16U(unsigned short &A, unsigned short &B) {register unsigned short C; C=A;A=B;B=C;};
void SimpleDataFlip(void *a, unsigned long n);


UsefulTime
---------------------------------------
double GetSystemTimeFP(void);
double GetProcessTimeFP(void);
void GetTime(time_t &SetMe);
const char *GetTimeString(time_t &MyTime);
void StartHiResTimer(void);
double StopHiResTimerSecs(void);
unsigned int StopHiResTimer(void);


UsefulUnit
---------------------------------------
double ConvertToMeters(double OrigVal, int FromUnit);
double ConvertFromMeters(double MeterVal, int ToUnit);
int GetUpconvertUnit(int Unit);
int GetDownconvertUnit(int Unit);
char *GetUnitName(int Unit);
char *GetUnitSuffix(int Unit);
int MatchUnitSuffix(char *Suffix);
int GetNormalizedUnit(int Unit, double Meters);


*/
