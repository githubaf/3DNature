// Joe.h
// Header file for Joe objects
// created from scratch on 4/4/95 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_JOE_H
#define WCS_JOE_H

class LayerStub;
class LayerEntry;
class LayerTable;
class Database;
class VectorPoint;

class Joe;
class JoeAttribute;

#include <stddef.h>
#include <stdio.h>

// Cruft for reference to old object type.

// EXTERN struct database {
// char         Enabled;
// char         Name[11];
// char         Layer1[3];
// char         Layer2[4];
// USHORT LineWidth;
// USHORT Color;
// char         Pattern[2];
// char         Label[16];
// USHORT Points;
/* char         Mark[2]; */
// USHORT Red, Grn, Blu;
/* char Special[4]; */
/* USHORT MaxFract; */
// UBYTE        Flags;
///* Flag bits:
//      1       Modified since last save
//      2       Enabled for mapping and rendering       
//      3       Enabled when file loaded        
//      4
//      5
//      6
//      7
//      8
//*/
// ULONG        VecArraySize;
// double       *Lat,
//      *Lon;
// short        *Elev;  
// /* SE, SE, SW, NW, NE */
//} *DBase;


//                ***************************************
//
//                        SOMEWHAT DIRE WARNING:
//
//                ***************************************
//
//  Do not create static or auto instances of class Joe. Due to the
// peculiar way in which the name field is dynamically allocated, bad
// things will happen to a non-dynamic Joe object when you try to set
// the name. Just don't do it. Static/auto pointers to Joe objects
// are of course, fine.

class Joe
{
// Joe is a Database object. I thought about DBObj
// and WCSObject and all sorts of variants like
// that, but they were all too vague. Joe is a
// name we will know and love. (Or hate. 5/1/95 -CXH)

friend class Database;
//private:
public:
        Joe *NextJoe, *PrevJoe, *Parent;
        unsigned long int Flags, NumPoints, Utility;
        VectorPoint *Points;
        LayerStub *LayerList;
        char *FName;
        unsigned char NameLen; // Label + Name
        union 
                {
                // used because Group objects don't have drawing attribs and
                // regular drawable objects don't have children
                // Use Red(), Green(), Blue(), PenNum(), and SetColor() to access
                struct GroupSpec
                        {
                        Joe *Child, *GroupChild;
                        // NumKids is all direct and indirect (grand, great-grand, etc...)
                        // children of group and attribute type. NumGroups is simply
                        // the number of immediate direct group children.
                        unsigned long int NumKids, NumGroups;
                        } GroupInfo; // Part two of union
                struct AttribSpec
                        {
                        unsigned short int LineStyle;
                        unsigned char LineWidth, PointStyle, RedGun, GreenGun,
                         BlueGun, DrawPen;
                        JoeAttribute *FirstAttrib;
                        unsigned short int CanonicalTypeMajor, CanonicalTypeMinor;
                        } AttribInfo; // Part one of union
                };

        void SetName(char *);
        void UnlinkMe(void);
        Joe *LinkMeAfter(Joe *);
        Joe *LinkMeBefore(Joe *);
        inline void IncrementUpTree(void);
        inline void DecrementUpTree(void);
        LayerStub *MatchEntryToStub(LayerEntry *Me);
        inline void JoeInitClear(void);
        void *WriteToFile(FILE *SaveFile, void *FileBuf);
        void WriteFileCleanUp(void *Buffer);
        Joe *WalkUp(void); // used to be on Database object
        Joe *WalkUpGroup(void);
        Joe *Descend(void);
        Joe *BottomWalkUpGroup(void);
        void UpdateGroupBounds();



//public:
        float NWLat, NWLon, SELat, SELon; // We really do mean single-precision

        void * operator new(size_t MySize, char *MyName);
        void operator delete(void *);
        Joe(size_t, char *);
        Joe(void); // only for use by Database object, I'm warning you!
        const char *Name();
        const char *FileName() {return((const char *)FName);};
        Joe *RemoveChild(Joe *);
        Joe *AddChild(Joe *TheChild, Joe *AfterChild = NULL);
        unsigned long int Children();
        // These are relatively harmless to use even on a group object,
        // since you'll just get ugly colors if you try to read the
        // group object child pointer as color components
        inline unsigned char Red(void) {return(AttribInfo.RedGun);};
        inline unsigned char Green(void) {return(AttribInfo.GreenGun);};
        inline unsigned char Blue(void) {return(AttribInfo.BlueGun);};
        inline unsigned char PenNum(void) {return(AttribInfo.DrawPen);};
        // These would be very painful to use on a group object since
        // you would obliterate the Child pointer. Therefore they check
        // the WCS_JOEFLAG_HASKIDS flag before setting colors.
        void SetRed(unsigned char Red);
        void SetGreen(unsigned char Green);
        void SetBlue(unsigned char Blue);
        void SetPenNum(unsigned char PenNum);
        // These are here for convenience, readability and future transparency
        inline void SetFlags(unsigned long int FlagSet) {Flags |= FlagSet;};
        inline void ClearFlags(unsigned long int FlagClear) {Flags &= ~FlagClear;};
        inline unsigned long int TestFlags(unsigned long int FlagTest)
         {return(FlagTest & Flags);};
        JoeAttribute *MatchAttribute(unsigned char MajorAttrib,
         unsigned char MinorAttrib);
        void AddAttribute(JoeAttribute *Me);
        JoeAttribute *RemoveAttribute(unsigned char MajorAttrib,
         unsigned char MinorAttrib);
        JoeAttribute *RemoveFirstAttribute(void);
        // Note: The following function is actually found on the LayerTable
        // Object now
        // LayerStub *AddObjectToLayer(char *DestLayerName);
        LayerStub *AddObjectToLayer(LayerEntry *MyEntry);
        LayerStub *AddObjectToLayer(LayerStub *MasterStub);
        LayerStub *FirstLayer(void) {return(LayerList);};
        LayerStub *NextLayer(LayerStub *PrevLayer);
        Joe *RemoveObjectFromLayer(LayerEntry *Me = NULL,
         LayerStub *Him = NULL); // returns this if successful



}; // Joe -- because my sister said so.

// definitions for flags
#define WCS_JOEFLAG_HASKIDS                             1 << 0
#define WCS_JOEFLAG_ISDEM                                       1 << 1
#define WCS_JOEFLAG_EXTFILE                             1 << 2

#define WCS_JOEFLAG_ILLUMINATED                 1 << 21
#define WCS_JOEFLAG_DEM_AS_SURFACE              1 << 22
#define WCS_JOEFLAG_VEC_AS_SEGMENTED    1 << 23

#define WCS_JOEFLAG_MODIFIED                    1 << 31

// Vector Objects with no name get a false name entry from their
// CanonicalType field, or "Unnamed" if CanonicalType is empty.
// DEM Objects with no name get a false name of "DEM: Lat,Lon"
// where Lat and Lon are replaced with the Latitude and Longitude
// from the SELat and SELon fields.

class JoeAttribute
{
// Used for any entended attributes on Joe objects
// In practice, you'd derive a class from this and add the kinds
// of member data you'd like. You don't mess with the ields below
// though, they're for internal use and linking.
friend class Joe;

private:
        JoeAttribute *NextAttrib;
        size_t AttribClassSize;
        unsigned char MajorAttribType, MinorAttribType;


}; // JoeAttribute


// Defines for loading and saving Joe objects
#define WCS_JOEFILE_FILENAME    1
#define WCS_JOEFILE_LAYERSYM8   2
#define WCS_JOEFILE_LAYERSYM16  3
#define WCS_JOEFILE_LAYERSYM32  4
#define WCS_JOEFILE_LAYERNAME   5
#define WCS_JOEFILE_EXTATTRIB   6
#define WCS_JOEFILE_BOUND               7
#define WCS_JOEFILE_POINTS              10
#define WCS_JOEFILE_LINEINFO    11

#define WCS_JOEFILE_DEMINFO             12


#endif // WCS_JOE_H
