// GraphData.h
// Header file for GraphData for World Construction Set v4.
// Built from scratch on 4/13/98 by Gary R. Huber
// Copyright 1998 by Questar Productions. All rights reserved.

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_GRAPHDATA_H
#define WCS_GRAPHDATA_H

union KeyDataValues;
class KeyData;
class GraphData;
class AnimCritter;
class V4KeyTable;
class Param;
class Gradient;
class ImageLib;
class Texture;
class RasterAnimHost;
class EcosystemEffect;
class CrossSectionData;
class TwinMaterials;
class RenderOpt;
class BufferNode;
class TextureData;
class RenderData;
class Object3DEffect;
struct Ecosystem;
union KeyFrame;
class EffectList;

#include "Types.h"
#include "RasterAnimHost.h"

// from GeoRaster.h
enum
	{
	WCS_GRADIENT_SOLID,
	WCS_GRADIENT_SPLINE,
	WCS_GRADIENT_LINEAR,
	WCS_GRADIENT_PARABOLIC,
	WCS_GRADIENT_COMPLEXPARAB,
	WCS_GRADIENT_CUBIC,
	WCS_GRADIENT_EXP,
	WCS_GRADIENT_ROOT,
	WCS_GRADIENT_INVEXP,
	WCS_GRADIENT_INVROOT,
	WCS_GRADIENT_NATEXP,
	WCS_GRADIENT_NATLOG
	}; // gradient classes

#define Clamp(X)	(((X) > Grad->ClampHigh) ? Grad->ClampHigh: (((X) < Grad->ClampLow) ? Grad->ClampLow: (X)))

class GraphNode;
class GraphData;
class NotifyEx;

#define AnimClamp(X)	(((X) > MaxMin[0]) ? MaxMin[0]: (((X) < MaxMin[1]) ? MaxMin[1]: (X)))

// these will be sent as components in a Notify Event - they must not conflict with color component values 0, 1, 2
enum
	{
	WCS_NOTIFYCOMP_ANIM_VALUECHANGED = 100,
	WCS_NOTIFYCOMP_ANIM_POSITIONCHANGED,
	WCS_NOTIFYCOMP_ANIM_SELECTIONCHANGED,
	WCS_NOTIFYCOMP_ANIM_NODEADDED,
	WCS_NOTIFYCOMP_ANIM_NODEREMOVED,
	WCS_NOTIFYCOMP_ANIM_ABOUTTOCHANGE
	}; // notify components

// notification subclasses
enum
	{
	WCS_SUBCLASS_ANIMDOUBLETIME = 165,
	WCS_SUBCLASS_ANIMDOUBLEDISTANCE,
	WCS_SUBCLASS_ANIMDOUBLEENVELOPE,
	WCS_SUBCLASS_ANIMDOUBLEPROFILE,
	WCS_SUBCLASS_ANIMDOUBLECLOUDPROF,
	WCS_SUBCLASS_ANIMDOUBLESECTION,
	WCS_SUBCLASS_ANIMDOUBLEBOOLEAN,
	WCS_SUBCLASS_ANIMDOUBLECURVE,
	WCS_SUBCLASS_ANIMCOLORTIME,
	WCS_SUBCLASS_ANIMCOLORGRADIENT,
	WCS_SUBCLASS_ANIMMATERIALGRADIENT,
	WCS_SUBCLASS_COLORTEXTURE
	}; // subclasses contained in effects that are not effects per se

enum
	{
	WCS_GRADIENTCRITTER_BLENDSTYLE_SHARP,
	WCS_GRADIENTCRITTER_BLENDSTYLE_SOFT,
	WCS_GRADIENTCRITTER_BLENDSTYLE_QUARTER,
	WCS_GRADIENTCRITTER_BLENDSTYLE_HALF,
	WCS_GRADIENTCRITTER_BLENDSTYLE_FULL,
	WCS_GRADIENTCRITTER_BLENDSTYLE_FASTINCREASE,
	WCS_GRADIENTCRITTER_BLENDSTYLE_SLOWINCREASE,
	WCS_GRADIENTCRITTER_BLENDSTYLE_SCURVE
	}; // animgradient node blend styles

enum
	{
	WCS_KEYOPERATION_DELETE,
	WCS_KEYOPERATION_SCALE,
	WCS_KEYOPERATION_CUROBJ,
	WCS_KEYOPERATION_CUROBJGROUP,
	WCS_KEYOPERATION_ROOTOBJ,
	WCS_KEYOPERATION_OBJCLASS,
	WCS_KEYOPERATION_ALLOBJ,
	WCS_KEYOPERATION_ONEKEY,
	WCS_KEYOPERATION_KEYRANGE,
	WCS_KEYOPERATION_ALLKEYS
	}; // defines for key scale and delete operations

#define WCS_ANIM_MAX_NUMGRAPHS	3

// file stuff
#define	WCS_GRAPHNODE_DISTANCE				0x00010000
#define	WCS_GRAPHNODE_VALUE					0x00020000
#define	WCS_GRAPHNODE_TENSION				0x00030000
#define	WCS_GRAPHNODE_CONTINUITY			0x00040000
#define	WCS_GRAPHNODE_BIAS					0x00050000
#define	WCS_GRAPHNODE_LINEAR				0x00060000
#define	WCS_GRAPHNODE_CROSSSECTIONDATA		0x00070000

#define	WCS_GRAPHDATA_NODE			0x00010000
#define	WCS_GRAPHDATA_NUMNODES		0x00020000
#define	WCS_GRAPHDATA_NODENUM		0x00030000
#define	WCS_GRAPHDATA_FIRSTDIST		0x00040000
#define	WCS_GRAPHDATA_LASTDIST		0x00050000

#define	WCS_CROSSSECTION_PRIORITY		0x00010000
#define	WCS_CROSSSECTION_ECOMIXING		0x00020000
#define	WCS_CROSSSECTION_ROUGHNESS		0x00030000
#define	WCS_CROSSSECTION_ECONAME		0x00040000

// AnimCritter
#define	WCS_ANIMCRITTER_GRAPHDATA	0x00010000
#define	WCS_ANIMCRITTER_GRAPHNUM	0x00020000

// AnimDouble, AnimColor
#define	WCS_ANIM_ANIMCRITTER		0x00010000

// AnimDouble
#define	WCS_ANIMDOUBLE_VALUE		0x00020000

// AnimColor, AnimColorTime
#define	WCS_ANIMCOLOR_VALUE1		0x00020000
#define	WCS_ANIMCOLOR_VALUE2		0x00030000
#define	WCS_ANIMCOLOR_VALUE3		0x00040000

// AnimColorTime
#define	WCS_ANIMCOLORTIME_INTENSITY	0x00110000

// AnimColorGradient
#define	WCS_ANIMCOLORGRADIENT_GRADIENTCRITTER	0x00110000

// AnimMaterialGradient
#define	WCS_ANIMMATERIALGRADIENT_GRADIENTCRITTER	0x00120000

// GradientCritter
#define	WCS_GRADIENTCRITTER_POSITION		0x00210000
#define	WCS_GRADIENTCRITTER_BLENDSTYLE		0x00220000
#define	WCS_GRADIENTCRITTER_COLORTEXTURE	0x00310000
#define	WCS_GRADIENTCRITTER_MATERIAL		0x00320000

// ColorTextureThing
#define	WCS_COLORTEXTURE_COLOR			0x00410000
#define	WCS_COLORTEXTURE_TEXTURETYPE	0x00420000
#define	WCS_COLORTEXTURE_TEXTURE		0x00430000


// flags  - these don't get saved to file
#define WCS_GRAPHNODE_FLAGS_SELECTED	(1 << 0)
#define WCS_GRAPHNODE_FLAGS_MODIFIED	(1 << 1)


class GraphNode
	{
	public:
		double Value, Distance, TCB[3];
		GraphNode *Next, *Prev;
		CrossSectionData *Data;
		char Linear, Flags;

		GraphNode();
		~GraphNode();
		void SetDefaults(void);
		CrossSectionData *AddData(RasterAnimHost *Parent);
		void Copy(GraphNode *CopyTo, GraphNode *CopyFrom, RasterAnimHost *Parent);
		void SetValue(double NewVal) {Value = NewVal;};
		double GetValue(void) {return (Value);};
		double GetDistance(void) {return (Distance);};
		char GetLinear(void) {return (Linear);};
		double GetTCB(char Channel) {return (TCB[Channel]);};
		char GetSelectedState(void) {return (Flags & (char)WCS_GRAPHNODE_FLAGS_SELECTED);};
		char GetModifiedState(void) {return (Flags & (char)WCS_GRAPHNODE_FLAGS_MODIFIED);};
		void SetDistance(double NewDist) {Distance = NewDist;};
		void SetTension(double NewTension) {TCB[0] = NewTension;};
		void SetContinuity(double NewContinuity) {TCB[1] = NewContinuity;};
		void SetBias(double NewBias) {TCB[2] = NewBias;};
		void SetTCB(char Channel, double NewValue) {TCB[Channel] = NewValue;};
		void SetLinear(char NewLinear) {Linear = NewLinear;};
		void SetFlag(char FlagToSet) {Flags |= FlagToSet;};
		void ClearFlag(char FlagToClear) {Flags &= ~FlagToClear;};
		void ToggleFlag(char FlagToToggle) {Flags ^= FlagToToggle;};
		ULONG Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, RasterAnimHost *Parent);
		unsigned long Save(FILE *ffile);
		
	}; // class GraphNode

class GraphData
	{
	public:
		double FirstDist, LastDist;
		GraphNode *Nodes;
		long NumNodes;

		GraphData();
		~GraphData();
		void SetDefaults(void);
		void Copy(GraphData *CopyTo, GraphData *CopyFrom, RasterAnimHost *Parent);
		double GetValue(double Dist);
		double GetValueBoolean(double Dist);
		double GetFirstDist(void) {return (FirstDist);};
		double GetLastDist(void) {return (LastDist);};
		int GetMinMaxVal(double &FindMin, double &FindMax);
		int GetMinMaxDist(double &FindMin, double &FindMax);
		int GetNextDist(double &NewDist, short Direction, double CurrentDist);
		int AdjustDistRange(double OldFirstDist, double OldLastDist, double NewFirstDist, double NewLastDist);
		int DeleteDistRange(double DeleteFirstDist, double DeleteLastDist);
		int ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop);
		int GetNextAnimNode(RasterAnimHostProperties *Prop);
		GraphNode *GetFirstNode(void) {return (Nodes);};
		GraphNode *GetFirstSelectedNode(void);
		GraphNode *GetNextNode(GraphNode *Current) {return (Current ? Current->Next: NULL);};
		GraphNode *AddNode(double Dist, double DistRange, double NewVal);
		GraphNode *AddNodeEnd(GraphNode *PrevNode, double Dist, double DistRange, double NewVal);
		GraphNode *RemoveNode(double Dist, double DistRange);
		GraphNode *UpdateNode(double Dist, double DistRange, double NewVal);
		GraphNode *ShiftNode(double Dist, double DistRange, double NewDist);
		GraphNode *FindNode(double Dist, double DistRange, GraphNode *&PrevNode);
		GraphNode *FindNode(double Dist, double DistRange, double Val, double ValRange);
		GraphNode *FindNode(double Dist, double Value);
		GraphNode *FindSplineNodes(double Dist, GraphNode *&PrevPrevNode, GraphNode *&PrevNode, GraphNode *&NextNode, GraphNode *&NextNextNode);
		GraphNode *InsertNode(double Dist, GraphNode *&PrevNode);
		GraphNode *DeleteNode(GraphNode *TempNode, GraphNode *&PrevNode);
		GraphNode *NewNodes(long NewNumNodes);
		void SetNodeSelectedAll(void);
		void ClearNodeSelectedAll(void);
		void ToggleNodeSelectedAll(void);
		void SetNodeModifiedAll(void);
		void ClearNodeModifiedAll(void);
		void SetNodeLinearAll(char NewValue);
		void ReleaseNodes(void);
		void SortNodesByDistance(void);
		GraphNode *SwapNodes(GraphNode *First, GraphNode *Last);
		long GetNumNodes(void) {return (NumNodes);};
		long CountSelectedNodes(void);
		GraphNode *FindNearestNode(double Dist);
		ULONG Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, AnimCritter *Parent);
		unsigned long Save(FILE *ffile);

	}; // class GraphData


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
// Base class
enum
	{
	WCS_ANIMCRITTER_TYPE_DOUBLETIME,
	WCS_ANIMCRITTER_TYPE_DOUBLEDIST,
	WCS_ANIMCRITTER_TYPE_DOUBLECURVE,
	WCS_ANIMCRITTER_TYPE_COLORTIME,
	WCS_ANIMCRITTER_TYPE_COLORDIST
	}; // AnimCritter derived types

// Flags
#define	WCS_ANIMCRITTER_FLAG_NONODES			(1 << 0)
#define	WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY		(1 << 1)
#define	WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE	(1 << 2)
#define	WCS_ANIMCRITTER_FLAG_FLOATING			(1 << 3)


class AnimCritter : public RasterAnimHost
	{
	public:
		double MaxMin[2], Increment, Multiplier;
		GraphData *Graph[WCS_ANIM_MAX_NUMGRAPHS];
		unsigned char NotifyItem, MetricType, FlagBits;

		AnimCritter();
		~AnimCritter();
		void SetDefaults(void);
		void SetDefaults(RasterAnimHost *RAHost, unsigned char Item);
		void SetRangeDefaults(double *Defaults);
		void GetRangeDefaults(double *Defaults);
		void CopyRangeDefaults(AnimCritter *CopyFrom);
		void SetMetricType(unsigned char NewType)	{MetricType = NewType;};
		void SetFlags(unsigned char FlagSet) {FlagBits |= FlagSet;};
		void ClearFlags(unsigned char Flags) {FlagBits &= ~Flags;};
		unsigned char TestFlags(unsigned char Flags) {return(FlagBits & Flags);};
		void SetNoNodes(unsigned char NewNoNodes)	{if(NewNoNodes) SetFlags(WCS_ANIMCRITTER_FLAG_NONODES); else ClearFlags(WCS_ANIMCRITTER_FLAG_NONODES);};
		unsigned char GetMetricType(void)	{return (MetricType);};
		void SetMultiplier(double NewMult)	{Multiplier = NewMult;};
		double GetMultiplier(void)	{return (Multiplier);};
		unsigned char GetNotifyItem(void) {return (NotifyItem);};
		GraphData *NewGraph(char GraphNum);
		void ReleaseGraph(char GraphNum);
		void ReleaseNodes(void)	{ReleaseGraph(0); ReleaseGraph(1); ReleaseGraph(2);};
		void OpenTimeline(void);
		int GetNumNodes(char GraphNum) {return (Graph[GraphNum] ? Graph[GraphNum]->GetNumNodes(): 0);};
		GraphNode *GetFirstNode(char GraphNum) {return (Graph[GraphNum] ? Graph[GraphNum]->GetFirstNode(): NULL);};
		GraphNode *GetFirstSelectedNode(char GraphNum);
		GraphNode *GetFirstSelectedNodeNoSet(char GraphNum);
		GraphNode *GetNextSelectedNode(GraphNode *CurNode);
		GraphNode *GetNextNode(char GraphNum, GraphNode *Current) {return (Graph[GraphNum] ? Graph[GraphNum]->GetNextNode(Current): NULL);};
		void AboutToChange(void);
		void ValueChanged(void);
		void PositionChanged(void);
		void SelectionChanged(void);
		void ObjectChanged(void);
		void NodeAdded(void);
		void NodeRemoved(void);
		void SetIncrement(double NewIncrement)	{Increment = NewIncrement;};
		double GetIncrement(void) {return (Increment);};
		double GetMaxVal(void) {return (MaxMin[0]);};
		double GetMinVal(void) {return (MaxMin[1]);};
		void SortNodesByDistance(char GraphNum) {if (Graph[GraphNum]) Graph[GraphNum]->SortNodesByDistance();};
		long GetNumSelectedNodes(void);
		GraphNode *FindNearestSiblingNode(double Dist, GraphNode *CompareNode);
		GraphNode *RemoveNode(double Dist, double DistRange);
		GraphNode *RemoteAddNode(double Dist);
		GraphNode *RemoteRemoveSelectedNode(void);
		virtual GraphNode *RemoteAlterSelectedNodeValue(double NewValue, double OldValue);
		GraphNode *RemoteAlterSelectedNodePosition(double NewDist, double OldDist);
		GraphNode *RemoteAlterSelectedNodeLinear(char NewValue);
		GraphNode *RemoteAlterSelectedNodeTCB(char Channel, double NewValue);
		GraphNode *RemoteAlterSelectedNodePriority(short NewValue);
		GraphNode *RemoteAlterSelectedNodeRoughness(double NewValue);
		GraphNode *RemoteAlterSelectedNodeEcoMixing(double NewValue);
		GraphNode *RemoteAlterSelectedNodeEcosystem(EcosystemEffect *NewEco);
		GraphNode *SetNodeSelected(GraphNode *Select, char Notify);
		GraphNode *ClearNodeSelected(GraphNode *Clear, char Notify);
		GraphNode *ToggleNodeSelected(GraphNode *Toggle, char Notify);
		GraphNode *SetNodeModified(GraphNode *Select, char Notify);
		GraphNode *ClearNodeModified(GraphNode *Clear);
		void SetNodeSelectedAll(void);
		void ClearNodeSelectedAll(void);
		void ToggleNodeSelectedAll(void);
		void SetNodeModifiedAll(void);
		void ClearNodeModifiedAll(void);
		void SetNodeLinearAll(char NewValue);
		GraphNode *FindNode(double Dist, double DistRange, double Val, double ValRange);
		GraphData *GetGraphPtr(char GraphNum) {return (Graph[GraphNum]);};
		const char *GetMetricSpecifier(void);
		void ScaleValues(double ScaleFactor);
		GraphNode *ValidateNode(GraphNode *CheckNode);
		ULONG Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);

		virtual void Copy(AnimCritter *CopyTo, AnimCritter *CopyFrom);
		virtual char GetType(void) = 0;
		virtual int TimeDude(void) = 0;
		virtual int NumChannels(void) = 0;
		virtual void SetCurValue(char Component, double NewValue) = 0;
		virtual int SetToTime(double Time) {return (0);};
		virtual int SetToDistance(double Dist) {return (0);};
		virtual void EnforceRange(void);
		virtual int GetNumGraphs(void) {return (0);};
		virtual double GetValue(char Channel, double Dist) {return (0.0);};
		virtual double GetCurValue(char Component) = 0;
		//virtual int GetNextDist(double &NewDist, short Direction, double CurrentDist) = 0;
		virtual int GetMinMaxDist(double &FindMin, double &FindMax) = 0;
		virtual int GetMinMaxVal(double &FindMin, double &FindMax) = 0;
		virtual double GetMinDistSeparation(void)	{return (.0099999);};	// making this a nice round number like .01 
			// doesn't work because in binary addition and subtraction are not exactly reversible. Use the value below when
			// offsetting a node to the next available position but use the number above when testing a position for
			// legality.
		virtual double GetMinDistIncrement(void)	{return (.01);};
		//virtual int DeleteDistRange(double FirstDist, double LastDist) = 0;
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0) = 0;
		virtual int GetRAHostAnimated(void);
		virtual int NodeDataAvailable(void)		{return (0);};
		virtual GraphNode *AddNode(void) = 0;

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) = 0;
		virtual char *GetRAHostTypeString(void) = 0;
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop) = 0;
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop) = 0;
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter) {return(NULL);};
		virtual void EditRAHost(void)							{OpenTimeline();};
		virtual char *GetCritterName(RasterAnimHost *TestMe)	{return ("");};
		virtual int ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop);
		virtual int GetNextAnimNode(RasterAnimHostProperties *Prop);
		virtual void SetFloating(char NewFloating)	{if(NewFloating) SetFlags(WCS_ANIMCRITTER_FLAG_FLOATING); else ClearFlags(WCS_ANIMCRITTER_FLAG_FLOATING);};
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)	
			{if (RAParent) RAParent->GetInterFlags(Prop, FlagMe); else Prop->InterFlags = 0; return;};

	}; // class AnimCritter

/******************************************************************************/
/******************************************************************************/
// 1st Derivatives

// these enums are used by FloatInts
enum
	{
	WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS = 0,
	WCS_ANIMDOUBLE_METRIC_HEIGHT,
	WCS_ANIMDOUBLE_METRIC_DISTANCE,
	WCS_ANIMDOUBLE_METRIC_ANGLE,
	WCS_ANIMDOUBLE_METRIC_LATITUDE,
	WCS_ANIMDOUBLE_METRIC_LONGITUDE,
	WCS_ANIMDOUBLE_METRIC_TIME,
	WCS_ANIMDOUBLE_METRIC_VELOCITY
	}; // anim double data types

class AnimDouble : public AnimCritter
	{
	public:
		double CurValue;

		AnimDouble();
		void SetDefaults(RasterAnimHost *RAHost, char Item, double Default);
		void SetValue(double NewValue) {CurValue = AnimClamp(NewValue);};
		double GetCurValue(void) {return (CurValue);};
		void SetCurValue(double NewValue);
		virtual GraphNode *AddNode(void)	{return (NULL);};
		GraphNode *AddNode(double Dist, double DistRange = 0.0);
		GraphNode *AddNode(double Dist, double NewValue,  double DistRange);
		GraphNode *AddNodeEnd(GraphNode *PrevNode, double Dist, double NewValue,  double DistRange);
		//GraphNode *RemoveNode(double Dist, double DistRange = 0.0);	// fall through to AnimCritter
		GraphNode *UpdateNode(double Dist, double DistRange = 0.0);
		void ScaleValues(double ScaleFactor);
		ULONG Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);

		// inherited from AnimCritter
		virtual void Copy(AnimCritter *CopyTo, AnimCritter *CopyFrom);
		virtual char GetType(void) = 0;
		virtual int TimeDude(void) = 0;
		virtual int NumChannels(void) {return (1);};
		virtual void SetCurValue(char Component, double NewValue) {SetCurValue(NewValue);};
		virtual int SetToTime(double Time) {return (0);};
		virtual int SetToDistance(double Dist) {return (0);};
		virtual void EnforceRange(void);
		virtual int GetNumGraphs(void) {return (1);};
		virtual double GetValue(char Channel, double Dist);
		virtual double GetCurValue(char Component) {return (CurValue);};
		virtual int GetNextDist(double &NewDist, short Direction, double CurrentDist);
		virtual int GetMinMaxDist(double &FindMin, double &FindMax);
		virtual int GetMinMaxVal(double &FindMin, double &FindMax);
		//virtual int DeleteDistRange(double FirstDist, double LastDist);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0) = 0;
		virtual char GetRAHostDropOK(long DropType) = 0;
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource) = 0;
		virtual long GetRAHostTypeNumber(void) = 0;
		virtual int NodeDataAvailable(void)		{return (0);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) = 0;
		virtual char *GetRAHostTypeString(void) = 0;
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop) {return (0);};
		virtual int GetNextAnimNode(RasterAnimHostProperties *Prop) {return (0);};
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)	
			{if (RAParent) RAParent->GetInterFlags(Prop, FlagMe); else Prop->InterFlags = 0; return;};

	}; // class AnimDouble

/******************************************************************************/

class AnimColor : public AnimCritter
	{
	public:
		double CurValue[3];

		AnimColor();
		void SetDefaults(RasterAnimHost *RAHost, char Item, double Default);
		void SetValue(char Component, double NewValue) {CurValue[Component] = AnimClamp(NewValue);};
		void SetValue3(double NewValue1, double NewValue2, double NewValue3) {CurValue[0] = AnimClamp(NewValue1); CurValue[1] = AnimClamp(NewValue2); CurValue[2] = AnimClamp(NewValue3);};
		int GetIsAnimated(void) {return (Graph[0] || Graph[1] || Graph[2]);};
		void RGBtoHSV(double HSV[3]);
		void HSVtoRGB(double HSV[3]);
		virtual GraphNode *AddNode(void)	{return (NULL);};
		GraphNode *AddNode(double Dist, double DistRange = 0.0);
		GraphNode *AddNode(double Dist, double NewValue[3], double DistRange);
		//GraphNode *RemoveNode(double Dist, double DistRange = 0.0);	// fall through to AnimCritter
		GraphNode *UpdateNode(double Dist, double DistRange = 0.0);
		void ScaleValues(double ScaleFactor);
		ULONG Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);

		// inherited from AnimCritter
		virtual void Copy(AnimCritter *CopyTo, AnimCritter *CopyFrom);
		virtual char GetType(void) = 0;
		virtual int TimeDude(void) {return (1);};
		virtual int NumChannels(void) {return (3);};
		virtual int SetToTime(double Time) {return (0);};
		virtual void SetCurValue(char Component, double NewValue);
		virtual int GetNumGraphs(void) {return (3);};
		virtual double GetValue(char Channel, double Dist);
		virtual double GetCurValue(char Component) {return (CurValue[Component]);};
		//virtual int GetNextDist(double &NewDist, short Direction, double CurrentDist);
		virtual int GetMinMaxDist(double &FindMin, double &FindMax);
		virtual int GetMinMaxVal(double &FindMin, double &FindMax);
		//virtual int DeleteDistRange(double FirstDist, double LastDist);
		virtual int GetRAHostAnimated(void) {return (AnimCritter::GetRAHostAnimated());};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0) = 0;
		virtual char GetRAHostDropOK(long DropType) = 0;
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource) = 0;
		virtual long GetRAHostTypeNumber(void) = 0;

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) = 0;
		virtual char *GetRAHostTypeString(void) = 0;
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop) = 0;
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop) = 0;
		virtual void EditRAHost(void)							{if (RAParent) RAParent->EditRAHost();};
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter) {return(NULL);};
		virtual char *GetCritterName(RasterAnimHost *TestMe)	{return ("");};
		virtual int ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop) {return (0);};
		virtual int GetNextAnimNode(RasterAnimHostProperties *Prop) {return (0);};

	}; // class AnimColor

/******************************************************************************/
/******************************************************************************/
// 2nd Derivatives

class AnimDoubleTime : public AnimDouble
	{
	public:

		~AnimDoubleTime();
		virtual GraphNode *AddNode(void);
		GraphNode *AddNode(double Dist, double DistRange = 0.0) {return(AnimDouble::AddNode(Dist, DistRange));};
		GraphNode *AddNode(double Dist, double NewValue, double DistRange)
			{return(AnimDouble::AddNode(Dist, NewValue, DistRange));};
		GraphNode *RemoveNode(void);
		GraphNode *UpdateNode(void);
		void SetValueNotify(double NewValue)	{CurValue = AnimClamp(NewValue); ValueChanged();};

		// inherited from AnimDouble & AnimCritter
		virtual char GetType(void) {return (WCS_ANIMCRITTER_TYPE_DOUBLETIME);};
		virtual int TimeDude(void) {return (1);};
		virtual int SetToTime(double Time);	
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType)		{return (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME);};
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetRAHostTypeNumber(void)		{return (WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMDOUBLETIME);};
		virtual char *GetRAHostTypeString(void)		{return ("(Anim Time)");};
		virtual int ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop);
		virtual int GetNextAnimNode(RasterAnimHostProperties *Prop);
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)	
			{if (RAParent) RAParent->GetInterFlags(Prop, FlagMe); else Prop->InterFlags = 0; return;};

		// in ParamConvert.cpp
		void TransferParamMotion(short MotionNumber, double ScaleFactor);

	}; // class AnimDoubleTime

/******************************************************************************/

class AnimDoubleDistance : public AnimDouble
	{
	public:

		~AnimDoubleDistance();
		// inherited from AnimDouble & AnimCritter
		virtual char GetType(void) {return (WCS_ANIMCRITTER_TYPE_DOUBLEDIST);};
		virtual int TimeDude(void) {return (0);};
		virtual int SetToDistance(double Dist);
		virtual int SetToTime(double Time) {return (0);};	
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType)		{return (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEDISTANCE);};
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetRAHostTypeNumber(void)		{return (WCS_RAHOST_OBJTYPE_ANIMDOUBLEDISTANCE);};
		virtual int NodeDataAvailable(void)		{return (0);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMDOUBLEDISTANCE);};
		virtual char *GetRAHostTypeString(void)		{return ("(Distance Envelope)");};

	}; // class AnimDoubleDistance

/******************************************************************************/

class AnimDoubleCurve : public AnimDouble
	{
	public:
		char XLabel[24], YLabel[24];
		unsigned char HorMetric;

		AnimDoubleCurve();
		~AnimDoubleCurve();
		void SetDefaults(RasterAnimHost *RAHost, unsigned char Item);

		// inherited from AnimDouble
		virtual char GetType(void) {return (WCS_ANIMCRITTER_TYPE_DOUBLECURVE);};
		unsigned char GetHorizontalMetric(void)	{return (HorMetric);};
		void SetHorizontalMetric(unsigned char NewMetric)	{HorMetric = NewMetric;};
		virtual int TimeDude(void) {return (0);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType)		{return (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLECURVE);};
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetRAHostTypeNumber(void)		{return (WCS_RAHOST_OBJTYPE_ANIMDOUBLECURVE);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMDOUBLECURVE);};
		virtual char *GetRAHostTypeString(void)		{return ("(Relationship Curve)");};

	}; // class AnimDoubleCurve

/******************************************************************************/

class AnimDoubleBoolean : public AnimDoubleTime
	{
	public:
		AnimDoubleBoolean();

		// inherited from AnimCritter
		virtual GraphNode *RemoteAlterSelectedNodeValue(double NewValue, double OldValue);

		// inherited from AnimDouble
		virtual double GetValue(char Channel, double Dist);
		virtual void SetCurValue(char Component, double NewValue);

		// inherited from AnimDoubleTime
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType)		{return (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEBOOLEAN);};
		virtual long GetRAHostTypeNumber(void)		{return (WCS_RAHOST_OBJTYPE_ANIMDOUBLEBOOLEAN);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMDOUBLEBOOLEAN);};
		virtual char *GetRAHostTypeString(void)		{return ("(Anim Switch)");};

	}; // class AnimDoubleBoolean

/******************************************************************************/

class AnimDoubleEnvelope : public AnimDoubleDistance
	{
	public:
		AnimDoubleEnvelope();
		void SetDefaults(RasterAnimHost *RAHost, unsigned char Item);

		// inherited from AnimDoubleDistance
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType)		{return (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEENVELOPE);};
		virtual long GetRAHostTypeNumber(void)		{return (WCS_RAHOST_OBJTYPE_ANIMDOUBLEENVELOPE);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMDOUBLEENVELOPE);};
		virtual char *GetRAHostTypeString(void)		{return ("(Amplitude Envelope)");};

	}; // class AnimDoubleEnvelope

/******************************************************************************/

class AnimDoubleProfile : public AnimDoubleDistance
	{
	public:
		AnimDoubleProfile();
		void SetDefaults(RasterAnimHost *RAHost, unsigned char Item);

		// inherited from AnimDoubleDistance
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType)		{return (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE);};
		virtual long GetRAHostTypeNumber(void)		{return (WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMDOUBLEPROFILE);};
		virtual char *GetRAHostTypeString(void)		{return ("(Edge Feathering Profile)");};

	}; // class AnimDoubleProfile

/******************************************************************************/

class AnimDoubleVerticalProfile : public AnimDoubleDistance
	{
	public:
		AnimDoubleVerticalProfile();
		void SetDefaults(RasterAnimHost *RAHost, unsigned char Item);

		// inherited from AnimDoubleDistance
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType)		{return (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLECLOUDPROF);};
		virtual long GetRAHostTypeNumber(void)		{return (WCS_RAHOST_OBJTYPE_ANIMDOUBLECLOUDPROF);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMDOUBLECLOUDPROF);};
		virtual char *GetRAHostTypeString(void)		{return ("(Vertical Profile)");};

	}; // class AnimDoubleVerticalProfile

/******************************************************************************/

class AnimDoubleCrossSection : public AnimDoubleDistance
	{
	public:
		AnimDoubleCrossSection();
		void SetDefaults(RasterAnimHost *RAHost, unsigned char Item);
		virtual int NodeDataAvailable(void)		{return (1);};
		void FetchSegmentData(short &Priority, double &Roughness, double &EcoMixing, 
			double &DistFromPrevNode, double &SegmentWidth, double Dist);
		void FetchSegmentData(short &Priority, double &Roughness, double &EcoMixing, EcosystemEffect *&Eco, 
			double &DistFromPrevNode, double &SegmentWidth, short SegNumber, double Dist);
		void FetchSegmentEco(EcosystemEffect *&Eco, double Dist);
		void FetchSegmentEcoPriority(EcosystemEffect *&Eco, short &Priority, double Dist);
		long InitImageIDs(long &ImageID);
		virtual int SetToTime(double Time);	
		int BuildFileComponentsList(EffectList **Ecosystems, EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords);
		int FindnRemoveEcosystems(EcosystemEffect *RemoveMe);
		void *GetNextSegmentWidth(void *PlaceHolder, double &SegWidth);

		// inherited from AnimDoubleDistance
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType)		{return (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION);};
		virtual long GetRAHostTypeNumber(void)		{return (WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMDOUBLESECTION);};
		virtual char *GetRAHostTypeString(void)		{return ("(Cross Section Profile)");};
		virtual int ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop);
		virtual int GetNextAnimNode(RasterAnimHostProperties *Prop);

	}; // class AnimDoubleCrossSection

/******************************************************************************/

class AnimColorTime : public AnimColor
	{
	public:
		AnimDoubleTime Intensity;

		AnimColorTime();
		~AnimColorTime();
		void SetDefaults(RasterAnimHost *RAHost, char Item);
		void Copy(AnimColorTime *CopyTo, AnimColorTime *CopyFrom);
		void OpenEditor(void);
		double GetCompleteValue(char Channel) {return (CurValue[Channel] * Intensity.CurValue);};
		double GetClampedCompleteValue(char Channel) {return (CurValue[Channel] * Intensity.CurValue < 1.0 ? CurValue[Channel] * Intensity.CurValue: 1.0);};
		double GetIntensity(void) {return (Intensity.GetCurValue());};
		GraphNode *AddNode(void);
		GraphNode *AddNode(double Dist, double DistRange = 0.0) {return(AnimColor::AddNode(Dist, DistRange));};
		GraphNode *AddNode(double Dist, double NewValue[3], double DistRange) {return(AnimColor::AddNode(Dist, NewValue, DistRange));};
		GraphNode *RemoveNode(void);
		GraphNode *UpdateNode(void);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		int GetAffiliates(RasterAnimHost *ChildA, AnimCritter *&AnimAffil);
		ULONG Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);

		// inherited from AnimColor & AnimCritter
		virtual char GetType(void) {return (WCS_ANIMCRITTER_TYPE_COLORTIME);};
		virtual int TimeDude(void) {return (1);};
		virtual int NumChannels(void) {return (3);};
		virtual int SetToTime(double Time);
		virtual int GetRAHostAnimated(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetRAHostTypeNumber(void)		{return (WCS_RAHOST_OBJTYPE_ANIMCOLORTIME);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMCOLORTIME);};
		virtual char *GetRAHostTypeString(void)		{return ("(Anim Color)");};
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual void EditRAHost(void)				{OpenEditor();};
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetCritterName(RasterAnimHost *TestMe);
		virtual int ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop);
		virtual int GetNextAnimNode(RasterAnimHostProperties *Prop);
		virtual int GetPopClassFlags(RasterAnimHostProperties *Prop);
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, 
			RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);

		// in ParamConvert.cpp
		void TransferParamColor(short PaletteNumber);

	}; // class AnimColorTime

/***********************************************************************/

class GradientCritter
	{
	public:
		AnimDoubleTime Position;
		char BlendStyle;
		void *GradientThing;
		GradientCritter *Next, *Prev;

		GradientCritter(RasterAnimHost *RAHost);
		~GradientCritter();
		void Copy(GradientCritter *CopyTo, GradientCritter *CopyFrom);
		void SetThing(void *NewThing) {GradientThing = NewThing;};
		void *GetThing(void) {return (GradientThing);};
		double GetDistance(void) {return (Position.CurValue);};
		void SetDistance(double NewDist) {Position.SetValue(NewDist);};
		int SetToTime(double Time);
		int IsAnimated(void);
		int GetRAHostAnimated(void);
		ULONG AnimColorGradient_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, RasterAnimHost *RAHost, unsigned char ApplyToEcosys);
		unsigned long AnimColorGradient_Save(FILE *ffile);
		ULONG AnimMaterialGradient_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, RasterAnimHost *RAHost, char MaterialType);
		unsigned long AnimMaterialGradient_Save(FILE *ffile);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil);

	}; // GradientCritter

/***********************************************************************/

class AnimGradient : public RasterAnimHost
	{
	public:
		GradientCritter *Grad, *ActiveNode;
		unsigned char ApplyToEcosys;

		AnimGradient(RasterAnimHost *RAHost, unsigned char EcosystemSource);
		~AnimGradient();
		int IsAnimated(void)	{return (0);};
		int RemoveNode(long RemoveNum);
		int RemoveNodeSetup(GradientCritter *RemoveMe);
		int AddNodeSetup(double NewDist, GradientCritter *&AddBefore, GradientCritter *&AddAfter);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		long CountNodes(void);

		// Chris: the commented functions can be used to configure and operate a gradient widget

		// returns the passed node if valid, otherwise NULL
		GradientCritter *ValidateNode(GradientCritter *CheckNode);
		// use this to determine if a node exists at a position or within +/-DistRange of the position
		GradientCritter *FindNode(double MatchDist, double DistRange = 0.0);
		// call this to find a node's current position (it can be animated), returns 0 if node invalid
		double GetNodePosition(GradientCritter *CurNode)	{return (ValidateNode(CurNode) ? CurNode->Position.CurValue: 0.0);};
		// sets the nodes position but not if it moves the node past one of the adjacent nodes
		int SetNodeDistance(GradientCritter *SetNode, double NewDistance);
		// pass this NULL to get the first node. returns NULL if no more nodes
		GradientCritter *GetNextNode(GradientCritter *Current)	{return (Current ? Current->Next: Grad);};
		// returns the current active node if valid otherwise sets ActiveNode to Grad and returns it
		GradientCritter *GetActiveNode(void)	{return (ActiveNode ? (ValidateNode(ActiveNode) ? ActiveNode: (ActiveNode = Grad)): (ActiveNode = Grad));};
		// checks passed value for validity and sets ActiveNode if OK, returns the active node in either case
		GradientCritter *SetActiveNode(GradientCritter *NewActive);
		// call this to add a node, returns existing node if a node already exists at precisely that position
		GradientCritter *AddNodeNotify(double NewDist, int Interpolate);
		// call this to ensure that a node is at a valid position (neither in front of the previous node or behind the next one).
		double CertifyNodePosition(GradientCritter *CheckNode);

		// virtual methods for AnimGradient
		// this version should only be called if no notification is desired
		virtual GradientCritter *AddNode(double NewDist, int Interpolate = 0) = 0;
		// call this to remove a known node, fails if node not found
		virtual int RemoveNode(GradientCritter *RemoveMe) = 0;
		// call this to evaluate color bar at a position fom 0 to 1
		virtual int GetBasicColor(unsigned char &Red, unsigned char &Green, unsigned char &Blue, double Pos) = 0;
		// call this to determine best color to represent a node
		virtual int GetNodeColor(unsigned char &Red, unsigned char &Green, unsigned char &Blue, GradientCritter *CurNode) = 0;
		// call this to determine if a symbol should be drawn on a node to indicate some kind of special data or condition exists for the node
		virtual int GetSpecialDataExists(GradientCritter *CheckNode) {return (0);};
		// for scaling and deleting animated nodes
		// for editing color at a node
		virtual void EditNodeColor(GradientCritter *EdNode) = 0;

		virtual unsigned long GetRAFlags(unsigned long Mask = ~0) = 0;
		virtual int GetRAHostAnimated(void)	{return (0);};
		virtual bool AnimateMaterials(void)	{return (false);};
		virtual int SetToTime(double Time)	{return (0);};
		virtual char GetRAHostDropOK(long DropType) = 0;
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource) = 0;
		virtual long GetRAHostTypeNumber(void) = 0;

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) = 0;
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop) = 0;
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop) = 0;
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter) = 0;
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual char *GetRAHostTypeString(void) = 0;
		virtual char *GetCritterName(RasterAnimHost *TestMe);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual char *OKRemoveRaster(void)	{return(RAParent ? RAParent->OKRemoveRaster(): NULL);};
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);

	}; // AnimGradient

/***********************************************************************/

class ColorTextureThing : public RasterAnimHost
	{
	public:
		unsigned char ApplyToEcosys;
		AnimColorTime Color;
		Texture *Tex;

		ColorTextureThing(RasterAnimHost *RAHost, unsigned char EcosystemSource);
		~ColorTextureThing();
		void Copy(ColorTextureThing *CopyTo, ColorTextureThing *CopyFrom);
		Texture *NewTexture(Texture *Proto);
		void SetTexture(Texture *NewTex);
		void SetTexturePrev(Texture *SetPrev);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		ULONG Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		long InitImageIDs(long &ImageID);
		int InitToRender(void);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);

		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual bool AnimateMaterials(void);
		virtual int SetToTime(double Time);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetRAHostTypeNumber(void) {return (WCS_RAHOST_OBJTYPE_COLORTEXTURE);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_COLORTEXTURE);};
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual char *GetRAHostTypeString(void)		{return ("(Color/Texture Combo)");};
		virtual char *GetCritterName(RasterAnimHost *TestMe);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual char *OKRemoveRaster(void)	{return(RAParent ? RAParent->OKRemoveRaster(): NULL);};
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);

	}; // ColorTextureThing

/***********************************************************************/

class AnimColorGradient : public AnimGradient
	{
	public:

		AnimColorGradient(RasterAnimHost *RAHost, unsigned char EcosystemSource);
		~AnimColorGradient();
		void Copy(AnimColorGradient *CopyTo, AnimColorGradient *CopyFrom);
		double GetValue(char Channel);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		ULONG Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		void SetTexturePrev(Texture *SetPrev);
		long InitImageIDs(long &ImageID);
		int InitToRender(void);
		// use this to evaluate the colors and textures from the gradient for rendering
		int Analyze(double Output[3], double Pos, TextureData *Data, int EvalChildren);

		// inherited from AnimGradient
		virtual GradientCritter *AddNode(double NewDist, int Interpolate = 0);
		virtual int RemoveNode(GradientCritter *RemoveMe);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual bool AnimateMaterials(void);
		virtual int SetToTime(double Time);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetRAHostTypeNumber(void) {return (WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT);};
		virtual int GetBasicColor(unsigned char &Red, unsigned char &Green, unsigned char &Blue, double Pos);
		virtual int GetBasicColor(double &Red, double &Green, double &Blue, double Pos);
		virtual int GetNodeColor(unsigned char &Red, unsigned char &Green, unsigned char &Blue, GradientCritter *CurNode);
		// special data for an AnimColorGradient is the Texture member
		virtual int GetSpecialDataExists(GradientCritter *CheckNode) 
			{return (CheckNode && CheckNode->GetThing() && ((ColorTextureThing *)CheckNode->GetThing())->Tex);};
		virtual void EditNodeColor(GradientCritter *EdNode);

		// inherited from AnimGradient & RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMCOLORGRADIENT);};
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetRAHostTypeString(void)		{return ("(Color Gradient)");};

	}; // AnimColorGradient

/***********************************************************************/

class AnimMaterialGradient : public AnimGradient
	{
	public:
		char MaterialType;

		AnimMaterialGradient(RasterAnimHost *RAHost, unsigned char EcosystemSource, char NewMatType);
		~AnimMaterialGradient();
		void Copy(AnimMaterialGradient *CopyTo, AnimMaterialGradient *CopyFrom);
		int AnimateEcoShadows(void);
		int Animate3DShadows(void);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		ULONG Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		long InitImageIDs(long &ImageID);
		int GetRenderMaterial(TwinMaterials *MatTwin, double Pos);
		int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords);
		int IsThereOpticallyTransparentMaterial(void);
		int IsThereTransparentMaterial(void);
		int AreThereEnabledTextures(void);
		int AreTexturesTileable(double &TileWidth, double &TileHeight, double &TileCenterX, double &TileCenterY);
		void GetWaterDepthAndWaveRange(double &MaximumMod, double &MinimumMod);

		// inherited from AnimGradient
		virtual GradientCritter *AddNode(double NewDist, int Interpolate = 0);
		virtual int RemoveNode(GradientCritter *RemoveMe);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual bool AnimateMaterials(void);
		virtual int SetToTime(double Time);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetRAHostTypeNumber(void) {return (WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT);};
		virtual int GetBasicColor(unsigned char &Red, unsigned char &Green, unsigned char &Blue, double Pos);
		virtual int GetNodeColor(unsigned char &Red, unsigned char &Green, unsigned char &Blue, GradientCritter *CurNode);
		virtual void EditNodeColor(GradientCritter *EdNode);

		// inherited from AnimGradient & RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_SUBCLASS_ANIMMATERIALGRADIENT);};
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetRAHostTypeString(void)		{return ("(Material Gradient)");};
		virtual char *GetCritterName(RasterAnimHost *Test);

		// in ParamConvert.cpp
		void TransferParamEcoData(struct Ecosystem *ParamEco, short EcoNumber);
		void TransferParamBeachData(void);
		void TransferParamStreamData(AnimDoubleTime *OldAnimPar, short OldColors[3], short OldUseWaves);
		void TransferParamSurfacesData(void);

	}; // AnimMaterialGradient

/***********************************************************************/
// used by terraffectors as a member of GraphNode

class CrossSectionData
	{
	public:
		EcosystemEffect *Eco;
		AnimDoubleTime EcoMixing, Roughness;
		short Priority;

		CrossSectionData(RasterAnimHost *Parent);
		void Copy(CrossSectionData *CopyTo, CrossSectionData *CopyFrom);
		short GetPriority(void)	{return (Priority);};
		double GetRoughness(void)	{return (Roughness.CurValue);};
		double GetEcoMixing(void)	{return (EcoMixing.CurValue);};
		EcosystemEffect *GetEcosystem(void)	{return (Eco);};
		void SetEco(EcosystemEffect *NewEco);
		void SetPriority(short NewValue)	{Priority = NewValue;};
		void SetRoughness(double NewValue)	{Roughness.SetValue(NewValue);};
		void SetEcoMixing(double NewValue)	{EcoMixing.SetValue(NewValue);};
		ULONG Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		int SetToTime(double Time);	
		int ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop);
		virtual int GetNextAnimNode(RasterAnimHostProperties *Prop);
		long InitImageIDs(long &ImageID);
		int BuildFileComponentsList(EffectList **Ecosystems, EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords);

	}; // class CrossSectionData

/******************************************************************************/
/******************************************************************************/
/********************************************************************/

#endif // WCS_GRAPHDATA_H
