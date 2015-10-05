// Layers.h
// Layer object for WCS Database
// Written from scratch on 03/20/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_LAYERS_H
#define WCS_LAYERS_H

class Joe;

class LayerEntry;
class LayerStub;
class LayerTable;
class PoolString;

#include "PoolString.h"
#include "RasterAnimHost.h"

//#define 	(1 << 0) // dfeprecated, was _COLORBY
#define WCS_LAYER_ISATTRIBUTE	(1 << 1) // has a data value associated with it
#define WCS_LAYER_TEXTATTRIBUTE	(1 << 2) // otherwise double is assumed
#define WCS_LAYER_LINKATTRIBUTE	(1 << 3) // used to identify linked vector parts imported from Shape files
#define WCS_SUBCLASS_LAYER			215
#define WCS_SUBCLASS_LAYERATTRIB	216

// This symbol in the first character of a Layer name indicates it is used to
// bear attribute/field data
#define WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL		'\x01'
#define WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT		'\x02'

struct LayerTextVal
	{
	unsigned long TextBufLen;
	char *TextBuf;
	}; // LayerTextVal

union LayerAttribute
	{
	double IEEEVal;
	struct LayerTextVal TextVal;
	}; // LayerAttribute

class LayerEntry : public RasterAnimHost
	{
	friend class LayerTable;
	friend class LayerStub;
	friend class Joe;
	friend class ExportFormatWCSVNS;
	private:
		LayerEntry *PrevLayer, *NextLayer;
		LayerEntry *NextHash;
		LayerStub *ChainStart;
		PoolString LayerName;
		signed long LayerPri;
		unsigned long NumObjects, LayerNum;
		unsigned long Flags;

	public:
		// These members were never implemented, and are now vestigial.
		//unsigned char Red, Green, Blue, Pen; // If color by layer flag set

		inline LayerEntry(void) {PrevLayer = NextLayer = NextHash = NULL; ChainStart = NULL; Flags = NumObjects = LayerNum = (unsigned long int)NULL; LayerPri = 0;};
		inline const char *GetName(void) {return(LayerName.StrData);};
		inline const char *GetCleanName(void) {if(TestFlags(WCS_LAYER_ISATTRIBUTE) && LayerName.StrData) {return(&LayerName.StrData[1]);} else return(LayerName.StrData);};
		inline const char *SetName(const char *NewName) {return(LayerName.Set(NewName));};
		inline const signed int GetPri(void) {return(LayerPri);};
		inline const signed int SetPri(const signed int NewPri) {return(LayerPri = NewPri);};
		inline unsigned int GetLayerNum(void) {return(LayerNum);};
		inline LayerStub *FirstStub(void) {return(ChainStart);};
		inline unsigned long TestFlags(unsigned long FlagTest) {return(FlagTest & Flags);};
		inline void SetFlags(unsigned long FlagSet) {Flags |= FlagSet;};
		inline void ClearFlags(unsigned long FlagClear) {Flags &= ~FlagClear;};
		void SetJoeFlags(unsigned long FlagSet, bool PerformClear);
		long GetRAHostTypeNumber(void)		{return (WCS_SUBCLASS_LAYER);};
		// <<<>>> use this version rather than the above once S@G and others understand new WCS_SUBCLASS_ATTRIB type
		//long GetRAHostTypeNumber(void)		{return (Flags & WCS_LAYER_ISATTRIBUTE ? WCS_SUBCLASS_ATTRIB : WCS_SUBCLASS_LAYER);};
		unsigned long GetRAFlags(unsigned long Mask);
		char GetRAHostDropOK(long DropType);
		long GetKeyFrameRange(double &FirstKey, double &LastKey)	{FirstKey = LastKey = 0.0; return (0);};
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);

		// inherited from RasterAnimHost
		virtual char *GetRAHostName(void)						{return ((char *)GetName());};
		virtual char *GetRAHostTypeString(void)					{return ("(Layer)");};
		virtual unsigned char GetNotifyClass(void)				{return (RAParent ? RAParent->GetNotifyClass(): (unsigned char)GetRAHostTypeNumber());};
		virtual unsigned char GetNotifySubclass(void)			{return ((unsigned char)GetRAHostTypeNumber());};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe)			{return (1);};
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);	// return 1 means object found and deleted

		// S@G context popup menus
		virtual int AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags);
	}; // LayerEntry


class LayerStub
	{ // A real nexus of object pointers -- binds objects to layers
	friend class Joe;
	private:
		Joe *ObjectReferredTo;
		LayerStub *NextStubSameObject, *NextObjectStubSameLayer,
			*PrevObjectStubSameLayer;
		LayerEntry *ThisObjectsLayer;
		LayerAttribute LAttrib;

	public:	
		inline LayerStub() {ObjectReferredTo = NULL; NextStubSameObject = NextObjectStubSameLayer = PrevObjectStubSameLayer = NULL; ThisObjectsLayer = NULL; LAttrib.TextVal.TextBufLen = 0; LAttrib.TextVal.TextBuf = NULL;};
		~LayerStub(); // Unlink pointers from all lists
		inline LayerEntry *MyLayer(void) {return(ThisObjectsLayer);};
		inline LayerStub *NextObjectInLayer(void) {return(NextObjectStubSameLayer);};
		inline LayerStub *PrevObjectInLayer(void) {return(PrevObjectStubSameLayer);};
		inline LayerStub *NextLayerInObject(void) {return(NextStubSameObject);};
		inline Joe *MyObject(void) {return(ObjectReferredTo);};
		void SetIEEEAttribVal(double NewIEEEVal);
		double GetIEEEAttribVal(void);
		void SetTextAttribVal(const char *NewTextVal);
		char *GetTextAttribVal(void);

	}; // LayerStub


class LayerTable
	{
	private:
		unsigned long NumLayers, MaxLayer;
		LayerEntry *LayerList;
		LayerEntry *LayerHash[256];
		LayerEntry *NewLayer(char *Name, unsigned int Pri);
		unsigned char MakeHash(char *Name);
		int RemoveLayerFromHash(LayerEntry *Me, unsigned char Hash);

	public:
		LayerTable();
		~LayerTable() {DestroyAll();LayerList=NULL;};
		void InitClear(void);
		void DestroyAll(void);  // Not to be called trivially
		inline LayerEntry *FirstEntry() {return(LayerList);};
		inline LayerEntry *NextEntry(LayerEntry *Current);
		inline LayerEntry *PrevEntry(LayerEntry *Current);
		LayerEntry *MatchLayer(char *MatchName);
		LayerEntry *MatchMakeLayer(char *MatchName, unsigned int Pri = 0);
		LayerEntry *RenameLayer(LayerEntry *Me, char *NewName);
		LayerStub *AddObjectToLayer(Joe *Me, char *DestLayerName);
		unsigned long HowManyLayers(void) {return(NumLayers);};
		unsigned long WriteLayerHeader(FILE *SaveFile);
		int RemoveLayer(LayerEntry *Me); // 1 = success, 0 = failure
		void RemoveUnusedLayers(void);
				
	}; // LayerTable

LayerEntry *LayerTable::NextEntry(LayerEntry *Current)
{
if(Current)
        {
        return(Current->NextLayer);
        } // if
return(NULL);
} // LayerTable::NextEntry

LayerEntry *LayerTable::PrevEntry(LayerEntry *Current)
{
if(Current)
        {
        return(Current->PrevLayer);
        } // if
return(NULL);
} // LayerTable::PrevEntry


/*
A diagram of how layers are linked to Joes and each other through LayerStubs
Every Joe has a parent except for StaticRoot. The parent and StaticRoot may be one and the same
which elliminates the Group column from this table.

Stub identifiers (e.g. SR1, G2, B3) are not part of the actual program structure or variable names, 
they are simply used in this illustration for the purpose of identifying individual LayerStubs.
Similarly LayerEntries are not numbered nor Joes given letters in the program itself, 
only in this illustration.

Abbreviations:
NSSO = NextStubSameObject
NOSSL = NextObjectStubSameLayer
POSSL = PrevObjectStubSameLayer
STUB = LayerStub
LAYER = LayerEntry


                                                                             <Parent
                                                         .========<============<=========.
                                                         |                    Child>     |
									    <Parent         \|/ <Parent			            \|/
                 JOE:   StaticRoot <------------------ Group <=======> JOE A           JOE B
                            |                            |   Child>     |                |
                            |                            |              |                |
                            |                            |              |                |
                            |\/LayerList                 |\/LayerList   |\/LayerList     |\/LayerList		
LAYERENTRY:                 |                            |              |                |				
            ChainStart>    \|/ NextObjectStubSameLayer> \|/    NOSSL>  \|/     NOSSL>   \|/				
LAYER 1  <============> STUB SR1 <==================> STUB G1 <=====> STUB A1 <=====> STUB B1
       <ThisObjectsLayer    | <PrevObjectStubSameLayer   |    <POSSL    |     <POSSL     |				
                            |                            |              |                |				
                            |\/NextStubSameObject        |\/NSSO        |\/NSSO          |\/NSSO			
                            |                            |   			|                |				
            ChainStart>    \|/ NextObjectStubSameLayer> \|/    NOSSL>  \|/     NOSSL>   \|/				
LAYER 2  <============> STUB SR2 <==================> STUB G2 <=====> STUB A2 <=====> STUB B2
        <ThisObjectsLayer   | <PrevObjectStubSameLayer   |    <POSSL    |     <POSSL     |				
                            |                            |              |                |				
                            |\/NextStubSameObject        |\/NSSO        |\/NSSO          |\/NSSO			
                            |                            |   			|                |				
            ChainStart>    \|/ NextObjectStubSameLayer> \|/    NOSSL>  \|/     NOSSL>   \|/				
LAYER 3  <============> STUB SR3 <==================> STUB G3 <=====> STUB A3 <=====> STUB B3
        <ThisObjectsLayer     <PrevObjectStubSameLayer        <POSSL          <POSSL		


*/

#endif //WCS_LAYERS_H
