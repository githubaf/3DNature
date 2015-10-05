// Joe.h
// Header file for Joe objects
// created from scratch on 4/4/95 by Chris "Xenon" Hanson
// Effects Attributes and Effect handling functions added 1997 by Gary R. Huber
// Copyright 1995 by Questar Productions. All rights reserved.

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_JOE_H
#define WCS_JOE_H

// define FASTCALL if Microsoft compiler & generating Win32 code
#ifndef FASTCALL
#if defined(_MSC_VER) && defined(_WIN32)
#define FASTCALL __fastcall
#else // _MSC_VER
#define FASTCALL
#endif // _MSC_VER
#endif // FASTCALL

class LayerStub;
class LayerEntry;
class LayerTable;
class Database;
class VectorPoint;
class EffectsLib;
class Joe;
class JoeAttribute;
class JoeList;
class EcosystemEffect;
class LakeEffect;
class RasterTerraffectorEffect;
class TerraffectorEffect;
class ShadowEffect;
class FoliageEffect;
class StreamEffect;
class Object3DEffect;
class MaterialEffect;
class BackgroundEffect;
class CelestialEffect;
class StarFieldEffect;
class GeneralEffect;
class RenderJobEffect;
class RenderOptsEffect;
class CameraEffect;
class PlanetOptsEffect;
class TerrainParamEffect;
class GroundEffect;
class SkyEffect;
class AtmosphereEffect;
class LightEffect;
class SnowEffect;
class EnvironmentEffect;
class CloudEffect;
class WaveEffect;
class CmapEffect;
class DEM;
class Project;
class Camera;
class ThematicMap;
class CoordSys;
class Fence;
class Label;
class VertexDEM;
class JoeAttribLayerData;
class PlanetOpt;
class SXQueryAction;
class NameList;
struct GeneralLinearEquation;
class VectorPart;
class VectorNode;
class TfxDetail;

// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a Joe

#include "RasterAnimHost.h"
#include "SXQueryItem.h"
#include "Types.h"

// Joe class definition flags
#define WCS_JOEFLAG_HASKIDS                             (1 << 0)
#define WCS_JOEFLAG_ISDEM                               (1 << 1)
#define WCS_JOEFLAG_EXTFILE                             (1 << 2)
#define WCS_JOEFLAG_OLDACTIVATED						(1 << 3)
#define WCS_JOEFLAG_ISCONTROL							(1 << 4)
#define WCS_JOEFLAG_ACTIVATED							(1 << 5)
#define WCS_JOEFLAG_DRAWENABLED							(1 << 6)
#define WCS_JOEFLAG_RENDERENABLED						(1 << 7)
#define WCS_JOEFLAG_TOPOLOGYVALIDATED					(1 << 8)
#define WCS_JOEFLAG_POINTSSHIFTED						(1 << 9)	// has never been saved to a file

// Attributes for describing the above classes
// DEM
#define WCS_JOEFLAG_DEM_AS_SURFACE              (1 << 22) // only valid on WCS_JOEFLAG_ISDEM
#define WCS_JOEFLAG_DEM_NO_SEALEVEL             (1 << 23) // Compensate for Death Valley, only applicable on WCS_JOEFLAG_ISDEM Joes
// VECTOR
#define WCS_JOEFLAG_VEC_ILLUMINATED             (1 << 21) // Only valid on Vectors (non-WCS_JOEFLAG_ISDEM)
#define WCS_JOEFLAG_VEC_AS_SEGMENTED			(1 << 23) // Only meaningful on Vectors (non-WCS_JOEFLAG_ISDEM)
#define WCS_JOEFLAG_VEC_SPFP_ELEV               (1 << 24) // Only valid on Vectors (non-WCS_JOEFLAG_ISDEM)

// Internal flags. Don't nobody use.
#define WCS_JOEFLAG_NEWRETRYCACHE               (1 << 29) // Internal program flag: Object is new, allow multiple tries to fetch cached data in DBEdit
#define WCS_JOEFLAG_NAMEREPLACED                (1 << 30) // Internal program flag: User has renamed object.
#define WCS_JOEFLAG_MODIFIED                    (1U << 31) // Internal program flag. What it says.

// undefine this if the Joe loader ever is changed to not load the old WCS label point
#define WCS_JOE_LABELPOINTEXISTS

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

struct GroupSpec
    {
    Joe *JChild, *GroupChild;
    // NumKids is all direct and indirect (grand, great-grand, etc...)
    // children of group and attribute type. NumGroups is simply
    // the number of immediate direct group children.
    unsigned long NumKids, NumGroups;
    };

struct AttribSpec
    {
    JoeAttribute *FirstAttrib;
    unsigned short LineStyle;
    unsigned short CanonicalTypeMajor, CanonicalTypeMinor;
	unsigned short SecondTypeMajor, SecondTypeMinor;
    unsigned char LineWidth, PointStyle, RedGun, GreenGun, BlueGun;
    };

#define WCS_JOEPROPERTIES_MASKBIT_LINESTYLE		(1 << 0)
#define WCS_JOEPROPERTIES_MASKBIT_LINEWEIGHT	(1 << 1)
#define WCS_JOEPROPERTIES_MASKBIT_RED			(1 << 2)
#define WCS_JOEPROPERTIES_MASKBIT_GREEN			(1 << 3)
#define WCS_JOEPROPERTIES_MASKBIT_BLUE			(1 << 4)
#define WCS_JOEPROPERTIES_MASKBIT_ENABLED		(1 << 5)
#define WCS_JOEPROPERTIES_MASKBIT_DRAWENABLED	(1 << 6)
#define WCS_JOEPROPERTIES_MASKBIT_RENDERENABLED	(1 << 7)
#define WCS_JOEPROPERTIES_MASKBIT_SELECTED		(1 << 8)
#define WCS_JOEPROPERTIES_MASKBIT_EFFECT		(1 << 9)

class JoeProperties
	{
	public:
		GeneralEffect *Effect;
		unsigned long Mask;
		long EffectType;
		unsigned char LineStyle, LineWeight, Red, Green, Blue, Enabled, DrawEnabled, RenderEnabled, Selected;

		JoeProperties();

	}; // class JoeProperties

class JoeSegmentData
	{
	public:
		Point3d BisectrixData;
		double MetersPerDegLon, PtX, PtY, PtLon;
		float CumulativeLength;
		VectorPoint *MyPoint;
		char BisectrixOK;

		JoeSegmentData()	{BisectrixOK = 0; MyPoint = NULL;
			CumulativeLength = 0.0f; MetersPerDegLon = 1.0; 
			BisectrixData[0] = BisectrixData[1] = BisectrixData[2] = PtX = PtY = PtLon = 0.0;
			};
	}; // class JoeSegmentData

class JoeRenderData
	{
	public:
		JoeSegmentData *SegData[2];
		Joe *SplinedJoe;
		double OrigLon, FirstPtLat, FirstPtLon, LastPtLat, LastPtLon;
		float MinEl;
		long NumSegs, ConnectEnds;

		JoeRenderData()	{NumSegs = 0; SegData[1] = SegData[0] = NULL; SplinedJoe = NULL; MinEl = FLT_MAX; OrigLon =  
			FirstPtLat = FirstPtLon = LastPtLat = LastPtLon = 0.0; ConnectEnds = 0;};
		void FreeSegmentData(void);
		void FreeSplinedJoe(void);
		JoeSegmentData **AddSegmentData(unsigned long NewNumSegs);
		Joe *AddJoeSpline(Database *DBHost, Joe *Original, double AngularDev);
	}; // class JoeRenderData

/***************************************************************************/

class Joe : public RasterAnimHost
{
// Joe is a Database object. I thought about DBObj
// and WCSObject and all sorts of variants like
// that, but they were all too vague. Joe is a
// name we will know and love. (Or hate. 5/1/95 -CXH)

friend class Database;
friend class Renderer;
//private:
public:
        double NWLat, NWLon, SELat, SELon; // We gone double!
        Joe *NextJoe, *PrevJoe, *Parent;
		// Note: Utility (below) is used to link up the heirarchy during
		// database loading. It _will_ be obliterated. It can be used for
		// very temporary data during other processes.
        unsigned long Flags, nNumPoints, Utility;
		#ifdef WCS_BUILD_VNS
		// this is here to support Scenarios, a very special and complex case which WCS doesn't need
		unsigned long UniqueLoadSaveID; // if valid, will be non-zero
		#endif // WCS_BUILD_VNS
        VectorPoint *nPoints;
        LayerStub *LayerList;
        char *FName;
		JoeRenderData *JRendData;
        unsigned char NameLen; // Label + Name
        union 
                {
                // used because Group objects don't have drawing attribs and
                // regular drawable objects don't have children
                // Use Red(), Green(), Blue() and SetColor() to access
                struct GroupSpec GroupInfo; // Part one of union
                struct AttribSpec AttribInfo; // Part two of union
                };

        //void SetName(char *);
        void UnlinkMe(void);
        Joe *LinkMeAfter(Joe *);
        Joe *LinkMeBefore(Joe *);
        void IncrementUpTree(void);
        void DecrementUpTree(void);
		void ZeroUpTree(void);
        LayerStub *MatchEntryToStub(LayerEntry *Me);
		int MatchEntryToChildStubs(LayerEntry *Me);
        inline void JoeInitClear(void);
        void *WriteToFile(FILE *SaveFile, void *FileBuf);
        void WriteFileCleanUp(void *Buffer);
        Joe* FASTCALL WalkUp(void); // used to be on Database object
        Joe *WalkUpGroup(void);
        Joe *Descend(void);
        Joe *BottomWalkUpGroup(void);
        void UpdateGroupBounds();
		void FreeSegmentData(void);
		void FreeRenderData(void);
		JoeSegmentData **AddSegmentData(unsigned long NewNumSegs);
		Joe *AddJoeSpline(Database *DBHost, double AngularDev);
		bool Spliney(Database *DBHost, double AngularDev);
		double AngleBetweenSegments(VectorPoint *PLinkFirst, VectorPoint *PLinkSecond);

//public:

        void * operator new(size_t MySize, char *MyName);
        void operator delete(void *);
// Vis4 didn't like it...
#if _MSC_VER >= 1200
		void operator delete(void *pMem, char *MyName);
#endif // _MSC_VER >= 1200
        Joe(size_t, char *);
        Joe(void); // only for use by Database object, I'm warning you!
		~Joe();
        const char *Name();
        const char *FileName() {return((const char *)FName);};
		const char *CanonName(void);
		const char *SecondaryName(void);
		Joe *RemoveMe(EffectsLib *Lib);
        Joe *RemoveChild(Joe *);
        Joe *AddChild(Joe *TheChild, Joe *AfterChild = NULL);
        unsigned long Children();
        // These are relatively harmless to use even on a group object,
        // since you'll just get ugly colors if you try to read the
        // group object child pointer as color components
        inline unsigned char Red(void) {return(AttribInfo.RedGun);}; //lint !e1402
        inline unsigned char Green(void) {return(AttribInfo.GreenGun);};
        inline unsigned char Blue(void) {return(AttribInfo.BlueGun);};
		inline unsigned char GetLineWidth(void) {return(AttribInfo.LineWidth);};
		inline unsigned short GetLineStyle(void) {return(AttribInfo.LineStyle);};
		inline unsigned char GetPointStyle(void) {return(AttribInfo.PointStyle);};
		inline unsigned long GetFlags(void) {return(Flags);};
		short GetClass(void);
		inline double GetNorth(void) {return(NWLat);};
		inline double GetSouth(void) {return(SELat);};
		inline double GetWest(void) {return(NWLon);};
		inline double GetEast(void) {return(SELon);};
		inline double GetNSCenter(void) {return((NWLat + SELat) * 0.5);};
		inline double GetEWCenter(void) {return((SELon + NWLon) * 0.5);};
		float GetElevRange(float &MaxEl, float &MinEl);
		float GetMinElev(void);
		void GetTrueBounds(double &NWLatitude, double &NWLongitude, double &SELatitude, double &SELongitude);
		int GetBoundsProjected(CoordSys *Projection, double &NNorthing, double &WEasting, double &SNorthing, double &EEasting, int GISConvention);
		void ExaggeratePointElevations(PlanetOpt *DefPlanetOpt);
		double GetVecLength(int ConsiderElev);
		int DefDegToProj(VertexDEM *Vert);
		const char *GetBestName(void);
		VectorPoint *CopyPoints(Joe *CopyTo, Joe *CopyFrom, short DoNotification, short DoBoundsCheck);
		void Copy(Joe *CopyTo, Joe *CopyFrom);
		void ConvertPointDatums(CoordSys *ConvertFrom, CoordSys *ConvertTo);
		bool ConvertToDefGeo(CoordSys *MyCoords);

		VectorPoint **PointsAddr(void)	{return(&nPoints);};
		VectorPoint *Points(void)	{return(nPoints);};
		VectorPoint *Points(VectorPoint *SetPoints)	{nPoints = SetPoints; return (nPoints);};
		unsigned long NumPoints(void)	{return(nNumPoints);};
		unsigned long NumPoints(unsigned long SetNumPoints)	{nNumPoints = SetNumPoints; return (nNumPoints);};
		unsigned long *NumPointsAddr(void)	{return(&nNumPoints);};

		// this method replaces GetNumPoints() and makes it possible to remove the 'label point' sometime
		unsigned long GetNumRealPoints(void);
		static unsigned long GetFirstRealPtNum(void);
		// this method replaces direct access to Points list and makes it possible to remove the 'label point' sometime
		VectorPoint *GetFirstRealPoint(void);
		VectorPoint *GetSecondRealPoint(void);

		VectorPoint *TransferPoints(VectorPart *CopyFrom, bool ReplicateOrigin);
		void RemoveDuplicatePoints(Database *DBHost);

		void SetNewNames(const char *NewName, const char *NewFName);
        // These would be very painful to use on a group object since
        // you would obliterate the Child pointer. Therefore they check
        // the WCS_JOEFLAG_HASKIDS flag before setting colors.
		void SetRGB(unsigned char Red, unsigned char Green, unsigned char Blue);
        void SetRed(unsigned char Red);
        void SetGreen(unsigned char Green);
        void SetBlue(unsigned char Blue);
		void SetLineWidth(unsigned char NewWidth);
		void SetLineStyle(unsigned short NewLineStyle);
		void SetPointStyle(unsigned char NewPointStyle);
		void SetCanonType(unsigned short Major, unsigned short Minor);
		void SetSecondaryType(unsigned short Major, unsigned short Minor);
        // These are here for convenience, readability and future transparency
        void SetFlags(unsigned long FlagSet);
		unsigned long CountSetNumPoints(void);
        void ClearFlags(unsigned long FlagClear);
        inline unsigned long TestFlags(unsigned long FlagTest)
         {return(FlagTest & Flags);};
        inline unsigned long TestDrawFlags(void)
         {return((WCS_JOEFLAG_DRAWENABLED & Flags) && (WCS_JOEFLAG_ACTIVATED & Flags));};
        inline unsigned long TestRenderFlags(void)
         {return((WCS_JOEFLAG_RENDERENABLED & Flags) && (WCS_JOEFLAG_ACTIVATED & Flags));};
        JoeAttribute *MatchAttribute(unsigned char MajorAttrib,
         unsigned char MinorAttrib);
		JoeAttribute *MatchAttributeRange(unsigned char MajorAttrib,
	         unsigned char MinorAttribLow, unsigned char MinorAttribHigh, JoeAttribute *Current);
        void AddAttribute(JoeAttribute *Me);
        void AddAttributeNotify(JoeAttribute *Me);
        short AddEffect(GeneralEffect *Effect, int ApplyToAll);
		short AddCoordSysFastDangerous(CoordSys *Effect, int ApplyToAll);
        JoeAttribute *RemoveAttribute(unsigned char MajorAttrib, unsigned char MinorAttrib);
		JoeAttribute *RemoveEffectAttribute(unsigned char MajorAttrib, unsigned char MinorAttrib, GeneralEffect *MatchEffect);
		JoeAttribute *RemoveSpecificAttribute(JoeAttribute *RemoveMe);
		JoeAttribute *RemoveSpecificAttributeNoNotify(JoeAttribute *RemoveMe);
		JoeAttribute *MatchEffectAttribute(JoeAttribute *Attrib, GeneralEffect *MatchEffect);
        JoeAttribute *RemoveFirstAttribute(void);
        JoeAttribute *GetFirstAttribute(void);
		short GetEffectPriority(unsigned char AttributeClass);
		short GetEffectEvalOrder(unsigned char AttributeClass);
        // Note: The following function is actually found on the LayerTable
        // Object now
        // LayerStub *AddObjectToLayer(char *DestLayerName);
        LayerStub *AddObjectToLayer(LayerEntry *MyEntry);
        LayerStub *AddObjectToLayer(LayerStub *MasterStub);
        LayerStub *FirstLayer(void) {return(LayerList);};
        LayerStub *NextLayer(LayerStub *PrevLayer);
        LayerStub *MatchLayer(LayerEntry *MatchMe);
		LayerEntry *GetMultiPartLayer(void);
        Joe *RemoveObjectFromLayer(int FullCleanup, LayerEntry *Me = NULL,
         LayerStub *Him = NULL); // returns this if successful


		//   Functions to wrap Layer functionality to implement generic
		//   numeric and text attribute fields
		//   Note, a single object can (appear to) have a Layer,
		//   a Text Attribute and a Numeric Attribute all of the
		//   same name. This is because Layers that represent a Text or
		//   Numeric attribute have a special collision-preventing type
		//   code prefixed onto their name. (See the top of Layers.h,
		//   WCS_LAYER_ATTRIB_MARKER_SYMBOL_).

		// These next three return NULL for non-existance, or a
		// helpful LayerStub pointer that can speed up a Get
		// operation immediately following
		LayerStub *CheckTextAttributeExistance(char *Name);
		LayerStub *CheckIEEEAttributeExistance(char *Name);
		LayerStub *CheckAttributeExistance(LayerEntry *CheckEntry);
		// You really shouldn't use this one unless you have good reason
		char *MakeAttributeModifiedName(char *Name, char SymbolAdd);

		// Get methods. The version that takes the LayerStub is
		// faster if you already have the LayerStub.
		char *GetTextAttributeValue(char *Name);
		double GetIEEEAttributeValue(char *Name);
		char *GetTextAttributeValue(LayerStub *GetFrom);
		double GetIEEEAttributeValue(LayerStub *GetFrom);
		int GetAttribValueList(JoeAttribLayerData *AttrList);

		// These return NULL for failure or a handy LayerStub
		// pointer for success. You can use the LayerStub
		// to quickly recheck/verify the value you just set.
		LayerStub *AddTextAttribute(char *AttribName, const char *NewText);
		LayerStub *AddIEEEAttribute(char *AttribName, double NewIEEE);
		LayerStub *AddTextAttribute(LayerEntry *AttribEntry, const char *NewText);
		LayerStub *AddIEEEAttribute(LayerEntry *AttribEntry, double NewIEEE);

		int CalcBisectrices(double MetersPerDegLat, int ConnectEnds);
		int CalcBisectrix(JoeSegmentData *VecPt, JoeSegmentData *VecPt2, JoeSegmentData *VecPt3, struct GeneralLinearEquation *Eq, float &LengthThisSegment);
		int CalcOrthogonal(JoeSegmentData *VecPt1, JoeSegmentData *VecPt2, int LastPt, struct GeneralLinearEquation *Eq, float &LengthThisSegment);
		CoordSys *GetCoordSys(void);
		int AddPointSegData(double MetersPerDegLat);
		void HeadingAndPitch(double Lat, double Lon, double MetersPerDegLat, double &Heading, double &Pitch);
		// VNS 3 version
		double MinDistToPoint(VectorNode *CurNode, TfxDetail *SegmentDetail, double MetersPerDegLat, double Exageration, double ElevDatum, 
			int SplineElev, int ConnectEnds, double &Elev, float *DistXYZ, double *Slope);
		// V6 version
		double MinDistToPoint(double Lat, double Lon, double ElevPt, double MetersPerDegLat, double Exageration, double ElevDatum,
			int SplineLatLon, int SplineElev, int ConnectEnds, int SkipFirstPoint, int &OffEnd, double &Elev, float *DistXYZ, double *Slope = NULL);
		// V5 and before
		double MinDistToPoint(double Lat, double Lon, double MetersPerDegLat, double *Elev, double *Slope = NULL);
		double MinDistToPoint(double Lat, double Lon, double Elev, double MetersPerDegLat, int ConnectToOrigin, 
			double Datum, double Exag, double &DistX, double &DistY, double &DistZ);
		double SplineLatLonElevation(VectorPoint **Point, double *Distance, Point2d LatLonPt, double *Slope);
		double SplineElevation(VectorPoint **Point, double *Distance, double *Slope);
		double SplineElevation(VectorPoint **Point, double *Distance, double *Slope, double Lat, double Lon, double latscale, double lonscale);
		int LastTrulyContained(VectorPoint **Point, double *Distance, double Lat, double Lon, double latscale, double lonscale);
		short SimpleContained(double Lat, double Lon);
		int IsJoeContainedInMyGeoBounds(Joe *AmIContained);
		int CompareGeoBounds(double NorthBnds, double SouthBnds, double WestBnds, double EastBnds);
		double AverageElevation(void);
		void StretchNorth(double NewNorth);
		void StretchSouth(double NewSouth);
		void StretchWest(double NewWest);
		void StretchEast(double NewEast);
		void StretchLowElev(float MaxEl, float MinEl, float OldMinEl);
		void StretchHighElev(float MaxEl, float MinEl, float OldMaxEl);
		void StretchWidth(double RefLon, double NewWidth, double OldWidth);
		void StretchHeight(double RefLat, double NewHeight, double OldHeight);
		void RecheckBounds(void);
		double ComputeAreaDegrees(void);
		void Edit(void);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void);
		long GetKeyFrameRange(double &FirstKey, double &LastKey)	{FirstKey = LastKey = 0.0; return (0);};
		GeneralEffect *GetAttributeEffect(JoeAttribute *Attr);
		void CopyCameraPath(Camera *PathSource);
		unsigned long ConformToTopo(void);
		bool MatchEndPoint(Joe *MatcheMe);
		JoeRenderData *SetFirstLastCoords(double FirstLat, double FirstLon, double LastLat, double LastLon); 

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifyClass(void) {return (RAParent ? RAParent->GetNotifyClass(): (unsigned char)GetRAHostTypeNumber());};
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)GetRAHostTypeNumber());};
		virtual int GetRAEnabled(void)
			{return (RAParent ? (TestFlags(WCS_JOEFLAG_ACTIVATED) && RAParent->GetRAEnabled()): TestFlags(WCS_JOEFLAG_ACTIVATED) ? 1: 0);};
		virtual void EditRAHost(void)							{RAParent ? RAParent->EditRAHost(): Edit();};
		virtual char *GetRAHostName(void)						{return (RAParent ? RAParent->GetRAHostName(): (char *)GetBestName());};
		virtual char *GetRAHostTypeString(void);
		virtual char *GetCritterName(RasterAnimHost *TestMe)			{return ("");};
		virtual long GetNextKeyFrame(double &NewDist, short Direction, double CurrentDist)	{NewDist = 0.0; return(0);};
		virtual long DeleteKeyFrame(double FirstDist, double LastDist)	{return (0);};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe)			{return (1);};
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);	// return 1 means object found and deleted

		// Only useful if we are WCS_JOEFLAG_ISDEM and have a JoeDEM attribute
		unsigned long AttemptLoadDEM(short LoadRelEl, Project *OpenFrom); // Loads Pristine
		unsigned long AttemptLoadDownSampled(long GridSize, Project *OpenFrom); // Loads ViewPref
		int AttemptLoadBetterDownSampled(int MaxPolys, Project *OpenFrom); // Loads ViewPref
		signed long HowManyPolys(void); // tells you how many polygons this DEM would have

		// for use in scripts
		static unsigned char GetLineStyleFromString(char *Str);
		int SetProperties(JoeProperties *JoeProp);

		// for SX Query Action Item
		int SXQuerySetupForExport(SXQueryAction *AddAction, const char *OutputFilePathSource);
		void SXQueryCleanupFromExport(void);
		bool IsSXClickQueryEnabled(void);
		long GetSXQueryRecordNumber(NameList **FileNamesCreated);

		// S@G context popup menus
		virtual int AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags);
		virtual int HandlePopMenuSelection(void *Action);

}; // Joe -- because my sister said so.


// Vector Objects with no name get a false name entry from their
// CanonicalType field, or "Unnamed" if CanonicalType is empty.
// DEM Objects with no name get a false name of "DEM: Lat,Lon"
// where Lat and Lon are replaced with the Latitude and Longitude
// from the SELat and SELon fields.

// Don't use this directly, use the CanonName and SecondName
// functions to get the names. This is for the DLG loader to decrypt
// additional names that won't fit.
const char *DecodeCanonName(unsigned short Major, unsigned short Minor);

class JoeAttribLayerData
	{
	public:
		char *AttribName, *AttrValueTxt;
		double AttrValueDbl;
		JoeAttribLayerData *Next;

		JoeAttribLayerData()	{AttribName = NULL; AttrValueTxt = NULL; AttrValueDbl = 0.0; Next = NULL;};
	}; // class JoeAttribLayerData


class JoeAttribute
{
// Used for any extended attributes on Joe objects
// In practice, you'd derive a class from this and add the kinds
// of member data you'd like. You don't normally mess with the fields
// below though, they're for internal use and linking.
friend class Joe;
friend class Database;
friend class Renderer;
friend class EffectsLib;

protected:
		// You _MUST_ initialize these in your derived-class constructor.
        JoeAttribute *NextAttrib;
        size_t AttribClassSize, FileSize; // FileSize includes header tags
        unsigned char MajorAttribType, MinorAttribType;
public:
		virtual ~JoeAttribute() {NextAttrib = NULL;};
		virtual size_t GetFileSize(void) {return(FileSize);};
		virtual unsigned long WriteToFile(FILE *Out) = 0;
		JoeAttribute *GetNextAttribute(void) {return (NextAttrib);};
		unsigned char MajorAttrib(void) {return (MajorAttribType);};
		unsigned char MinorAttrib(void) {return (MinorAttribType);};
		short GetEffectPriority(void);
		short GetEffectEvalOrder(void);
		virtual GeneralEffect *GetGeneralEffect(void) const {return(NULL);};
}; // JoeAttribute

// Values for MajorAttribType
enum
	{
	WCS_JOE_ATTRIB_INTERNAL,
	WCS_JOE_ATTRIB_EXTENDED
	}; // MajorType

// Values for WCS_JOE_ATTRIB_INTERNAL MinorAttrib
enum
	{
	WCS_JOE_ATTRIB_INTERNAL_DEM = 0,
	WCS_JOE_ATTRIB_INTERNAL_COMBO,			// there is no attribute of this number, its just a place holder to keep effects in sync
	WCS_JOE_ATTRIB_INTERNAL_LAKE,
	WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM,
	WCS_JOE_ATTRIB_INTERNAL_FOG,
	WCS_JOE_ATTRIB_INTERNAL_WAVE,
	WCS_JOE_ATTRIB_INTERNAL_CLOUD,
	WCS_JOE_ATTRIB_INTERNAL_CLOUDSHADOW,
	WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT,
	WCS_JOE_ATTRIB_INTERNAL_TINT,
	WCS_JOE_ATTRIB_INTERNAL_CMAP,
	WCS_JOE_ATTRIB_INTERNAL_ILLUMINATION, // retired
	WCS_JOE_ATTRIB_INTERNAL_LANDWAVE,
	WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR,

	WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR,
	WCS_JOE_ATTRIB_INTERNAL_PROFILE,		// retired
	WCS_JOE_ATTRIB_INTERNAL_GRADIENTPROFILE, // retired
	WCS_JOE_ATTRIB_INTERNAL_PATHFOLLOW,
	WCS_JOE_ATTRIB_INTERNAL_PATHCONFORM,
	WCS_JOE_ATTRIB_INTERNAL_MORPH,

	WCS_JOE_ATTRIB_INTERNAL_ECOTYPE,
	WCS_JOE_ATTRIB_INTERNAL_FOLIAGEGRP,
	WCS_JOE_ATTRIB_INTERNAL_FOLIAGE,
	WCS_JOE_ATTRIB_INTERNAL_OBJECT3D,
	WCS_JOE_ATTRIB_INTERNAL_OBJECTVEC,

	WCS_JOE_ATTRIB_INTERNAL_SHADOW,
	WCS_JOE_ATTRIB_INTERNAL_STREAM,
	WCS_JOE_ATTRIB_INTERNAL_MATERIAL,
	WCS_JOE_ATTRIB_INTERNAL_BACKGROUND,
	WCS_JOE_ATTRIB_INTERNAL_CELESTIAL,
	WCS_JOE_ATTRIB_INTERNAL_STARFIELD,
	WCS_JOE_ATTRIB_INTERNAL_PLANETOPT,
	WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM,
	WCS_JOE_ATTRIB_INTERNAL_GROUND,
	WCS_JOE_ATTRIB_INTERNAL_SNOW,
	WCS_JOE_ATTRIB_INTERNAL_SKY,
	WCS_JOE_ATTRIB_INTERNAL_ATMOSPHERE,
	WCS_JOE_ATTRIB_INTERNAL_LIGHT,
	WCS_JOE_ATTRIB_INTERNAL_CAMERA,
	WCS_JOE_ATTRIB_INTERNAL_RENDERJOB,
	WCS_JOE_ATTRIB_INTERNAL_RENDEROPT,
	WCS_JOE_ATTRIB_INTERNAL_GRIDDER,
	WCS_JOE_ATTRIB_INTERNAL_GENERATOR,
	WCS_JOE_ATTRIB_INTERNAL_SEARCHQUERY,
	WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP,
	WCS_JOE_ATTRIB_INTERNAL_COORDSYS,
	WCS_JOE_ATTRIB_INTERNAL_FENCE,
	WCS_JOE_ATTRIB_INTERNAL_POSTPROC,
	WCS_JOE_ATTRIB_INTERNAL_SCENARIO,
	WCS_JOE_ATTRIB_INTERNAL_DEMMERGER,
	WCS_JOE_ATTRIB_INTERNAL_EXPORTER,
	WCS_JOE_ATTRIB_INTERNAL_LABEL,
// <<<>>> ADD_NEW_EFFECTS  Important - add a value here even if the new effect type cannot be attached to a Joe!
// Add it right above this comment - this list MUST be in the same order as the one in EffectsLib.h!
// The FBI, CIA, GRH, Wall Street, Santa Claus and numerous paramilitary groups are all watching!
// Failure to keep this list in sync with the enum list in EffectsLib.h will undoubtedly cause planets and stars to
// collide which would be cool to watch except that you won't live long enough to enjoy it. 

	// Because this attribute never gets saved, it doesn't matter what
	// the value actually is, only that it is consistant during one run
	// of the program, that it doesn't get in the way of other attributes,
	// and that it be out of the range of saveable effects.
	WCS_JOE_ATTRIB_INTERNAL_VIEWTEMP,
	// this one is used to identify temporary terrain generated by Terrain Generator
	WCS_JOE_ATTRIB_INTERNAL_RAHOST,
	// this one exists only during SX export if the vector is chosen as an action item for NatureView
	WCS_JOE_ATTRIB_INTERNAL_SXQUERYITEM
	}; // MinorType INTERNAL

// this isn't a regular Effect attribute but does get saved to Database files so needs an unchanging value
#define WCS_JOE_ATTRIB_INTERNAL_ALIGN3DOBJ	200

class JoeDEM: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class Renderer;
	friend class JoeList;
	public:
		char MaxFract;
		short MaxEl, MinEl;
		short NWAlt, NEAlt, SEAlt, SWAlt;
		float SumElDif, SumElDifSq;
		double ElScale, ElDatum;
		DEM *Pristine, *ViewPref;

		// these are cached copies from the DEM object, for faster access without loading the .ELEV
		unsigned long pLonEntries, pLatEntries;
		float pElMaxEl, pElMinEl, pNullValue;
		double pLatStep, pLonStep;
		double DEMGridNS, DEMGridWE; // calculated by DEM::GetDEMCellSizeMeters
		
		bool QueryCachedELEVDataValid(void) const {return(pLonEntries != 0);};
		bool UpdateCachedELEVData(Joe *MyJoe, Project *OpenFrom);

		// Initialize JoeAttribute base members in constructor...
		JoeDEM();
		~JoeDEM();

                // because SAS doesn`t like explicitly calling the
                // constructor, we`ll provide a member fn to do that
                // initialization.  Eventually, we should assign to 'this'
                // to do our own allocation inside the default
                // constructor, instead of doing separate allocation and
                // initialization. - SRK 13-Feb-98

                void InitClear(void);
                                 
		virtual unsigned long WriteToFile(FILE *Out);
		double FetchBestElScale(void);

	}; // JoeDEM

// JoeViewTemp now found in ViewGUI.h

class JoeEcosystem: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		EcosystemEffect *Eco;

		// Initialize JoeAttribute base members in constructor...
		JoeEcosystem();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Eco);};

	}; // JoeEcosystem

class JoeLake: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		LakeEffect *Lake;

		// Initialize JoeAttribute base members in constructor...
		JoeLake();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Lake);};

	}; // JoeLake

class JoeRasterTA: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		RasterTerraffectorEffect *Terra;

		// Initialize JoeAttribute base members in constructor...
		JoeRasterTA();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Terra);};

	}; // JoeRasterTA

class JoeTerraffector: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		TerraffectorEffect *Terra;

		// Initialize JoeAttribute base members in constructor...
		JoeTerraffector();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Terra);};

	}; // JoeIllumination

class JoeShadow: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		ShadowEffect *Shadow;

		// Initialize JoeAttribute base members in constructor...
		JoeShadow();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Shadow);};

	}; // JoeShadow

class JoeFoliage: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		FoliageEffect *Foliage;

		// Initialize JoeAttribute base members in constructor...
		JoeFoliage();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Foliage);};

	}; // JoeFoliage

class JoeStream: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		StreamEffect *Stream;

		// Initialize JoeAttribute base members in constructor...
		JoeStream();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Stream);};

	}; // JoeStream

class JoeObject3D: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		Object3DEffect *Object;

		// Initialize JoeAttribute base members in constructor...
		JoeObject3D();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Object);};

	}; // JoeObject3D

class JoeTerrainParam: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		TerrainParamEffect *TerrainPar;

		// Initialize JoeAttribute base members in constructor...
		JoeTerrainParam();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)TerrainPar);};

	}; // JoeTerrainParam

class JoeGround: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		GroundEffect *Ground;

		// Initialize JoeAttribute base members in constructor...
		JoeGround();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Ground);};

	}; // JoeGround

class JoeSnow: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		SnowEffect *SnowBusinessLikeShowBusiness;

		// Initialize JoeAttribute base members in constructor...
		JoeSnow();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)SnowBusinessLikeShowBusiness);};

	}; // JoeSnow

class JoeEnvironment: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		EnvironmentEffect *Env;

		// Initialize JoeAttribute base members in constructor...
		JoeEnvironment();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Env);};

	}; // JoeEnvironment

class JoeCloud: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		CloudEffect *Cloud;

		// Initialize JoeAttribute base members in constructor...
		JoeCloud();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Cloud);};

	}; // JoeCloud

class JoeWave: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		WaveEffect *Wave;

		// Initialize JoeAttribute base members in constructor...
		JoeWave();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Wave);};

	}; // JoeWave

class JoeCmap: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		CmapEffect *Cmap;

		// Initialize JoeAttribute base members in constructor...
		JoeCmap();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Cmap);};

	}; // JoeCmap

class JoeThematicMap: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		ThematicMap *Theme;

		// Initialize JoeAttribute base members in constructor...
		JoeThematicMap();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Theme);};

	}; // JoeThematicMap

class JoeCoordSys: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		CoordSys *Coord;
		JoeList *CoordsJoeList;

		// Initialize JoeAttribute base members in constructor...
		JoeCoordSys();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Coord);};

	}; // JoeCoordSys

class JoeFence: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		Fence *Fnce;

		// Initialize JoeAttribute base members in constructor...
		JoeFence();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Fnce);};

	}; // JoeFence

class JoeLabel: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		Label *Labl;

		// Initialize JoeAttribute base members in constructor...
		JoeLabel();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Labl);};

	}; // JoeLabel

class JoeAlign3DObj: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	public:

		Object3DEffect *Obj;

		// Initialize JoeAttribute base members in constructor...
		JoeAlign3DObj();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);
		GeneralEffect *GetGeneralEffect(void) const {return((GeneralEffect *)Obj);};

	}; // JoeAlign3DObj

// This does not get written to the database file
// It is used to identify the effect that is responsible for generating a DEM
class JoeRAHost: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		RasterAnimHost *RAHost;

		// Initialize JoeAttribute base members in constructor...
		JoeRAHost();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);

	}; // JoeRAHost

// This does not get written to the database file
// It exists only during SX export if the vector is chosen as an action item for NatureView
class JoeSXQueryItem: public JoeAttribute, public SXQueryItem
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	public:

		// Initialize JoeAttribute base members in constructor...
		JoeSXQueryItem();
		virtual unsigned long WriteToFile(FILE *Out);
		void InitClear(void);

	}; // JoeSXQueryItem

// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a Joe you need to create a new JoeAttribute

// Defines for loading and saving Joe objects
// 0 is undefined
#define WCS_JOEFILE_FILENAME    1
#define WCS_JOEFILE_LAYERSYM8   2
#define WCS_JOEFILE_LAYERSYM16  3
#define WCS_JOEFILE_LAYERSYM32  4
#define WCS_JOEFILE_LAYERNAME   5
#define WCS_JOEFILE_EXTATTRIB   6
#define WCS_JOEFILE_BOUND       7
#define WCS_JOEFILE_POINTS      10
#define WCS_JOEFILE_LINEINFO    11

#define WCS_JOEFILE_DEMINFO             12
#define WCS_JOEFILE_ATTRIBINFO          13
// VAR8-derived extended tags
#define WCS_JOEFILE_DEMINFOEXTEND       14
#define WCS_JOEFILE_IEEELAYERATTRIB     15
#define WCS_JOEFILE_UNIQUEID            16 // to support VNS2 scenarios
#define WCS_JOEFILE_DEMINFOEXTENDV3     17

// VAR16-derived extended tags
#define WCS_JOEFILE_TEXTLAYERATTRIB     96
// VAR32-derived extended tags
#define WCS_JOEFILE_EFFECTINFO          176


#define	WCS_JOEFILE_EFFECTINFO_END		0
#define	WCS_JOEFILE_EFFECTINFO_NAME		1


// Tags WCS_JOEFILE_VAR8 have an 8-bit length counter
#define WCS_JOEFILE_VAR8				14

// Tags WCS_JOEFILE_VAR16 have a 16-bit length counter
#define WCS_JOEFILE_VAR16				96				

// Tags WCS_JOEFILE_VAR32 have a 32-bit length counter
#define WCS_JOEFILE_VAR32				176

#define WCS_JOEFILE_MAX					255

#endif // WCS_JOE_H
