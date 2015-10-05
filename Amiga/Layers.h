// Layers.h
// Layer object for WCS Database
// Written from scratch on 03/20/95 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_LAYERS_H
#define WCS_LAYERS_H

class Joe;

class LayerEntry;
class LayerStub;
class LayerTable;
class PoolString;

#include <stddef.h>
#include <stdio.h>
#include "PoolString.h"

#define WCS_LAYER_COLORBY		1 << 0
class LayerEntry 
	{
	friend class LayerTable;
	private:
		LayerEntry *PrevLayer, *NextLayer;
		LayerEntry *NextHash;
		PoolString LayerName;
		signed long int LayerPri;
		unsigned long int NumObjects, LayerNum;
		unsigned long int Flags;

	public:
		unsigned char Red, Green, Blue, Pen; // If color by layer flag set

		inline LayerEntry(void) {Flags = NumObjects = (unsigned long int)NULL; LayerPri = 0;};
		inline const char *GetName(void) {return(LayerName.StrData);};
		inline const char *SetName(const char *NewName) {return(LayerName.Set(NewName));};
		inline const signed int GetPri(void) {return(LayerPri);};
		inline const signed int SetPri(const signed int NewPri) {return(LayerPri = NewPri);};
		inline unsigned int GetLayerNum(void) {return(LayerNum);};
		inline int TestFlags(unsigned long int FlagTest) {return(FlagTest & Flags);};
		inline void SetFlags(unsigned long int FlagSet) {Flags |= FlagSet;};
		inline void ClearFlags(unsigned long int FlagClear) {Flags &= ~FlagClear;};
	}; // LayerEntry


class LayerStub
	{ // A real nexus of object pointers -- binds objects to layers
	friend class Joe;
	private:
		Joe *ObjectReferredTo;
		LayerStub *NextStubSameObject, *NextObjectStubSameLayer,
			*PrevObjectStubSameLayer;
		LayerEntry *ThisObjectsLayer;

	public:	
		inline LayerStub() {NextStubSameObject = NextObjectStubSameLayer = PrevObjectStubSameLayer = NULL; ThisObjectsLayer = NULL;};
		~LayerStub(); // Unlink pointers from all lists
		inline LayerEntry *MyLayer(void) {return(ThisObjectsLayer);};
		inline LayerStub *NextObjectInLayer(void) {return(NextObjectStubSameLayer);};
		inline LayerStub *NextLayerInObject(void) {return(NextStubSameObject);};
		inline Joe *MyObject(void) {return(ObjectReferredTo);};
	}; // LayerStub


class LayerTable
	{
	private:
		unsigned long int NumLayers, MaxLayer;
		LayerEntry *LayerList;
		LayerEntry *LayerHash[255];
		LayerEntry *NewLayer(char *Name, unsigned int Pri);
		int RemoveLayer(LayerEntry *Me); // 1 = success, 0 = failure
		unsigned char MakeHash(char *Name);
		int RemoveLayerFromHash(LayerEntry *Me, unsigned char Hash);

	public:
		LayerTable();
		~LayerTable() {DestroyAll();};
		void InitClear(void);
		void DestroyAll(void);  // Not to be called trivially
		inline LayerEntry *FirstEntry() {return(LayerList);};
		inline LayerEntry *NextEntry(LayerEntry *Current);
		inline LayerEntry *PrevEntry(LayerEntry *Current);
		LayerEntry *MatchLayer(char *MatchName);
		LayerEntry *MatchMakeLayer(char *MatchName, unsigned int Pri = 0);
		LayerEntry *PonderLayers(Joe *NewObj);
		LayerEntry *RenameLayer(LayerEntry *Me, char *NewName);
		LayerStub *AddObjectToLayer(Joe *Me, char *DestLayerName);
		unsigned long int HowManyLayers(void) {return(NumLayers);};
		unsigned long int WriteLayerHeader(FILE *SaveFile);
				
	}; // LayerTable

#endif //WCS_LAYERS_H
