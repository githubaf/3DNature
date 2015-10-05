// DBFilterEvent.h
// For managing WCS DB Filter Events
// Built from scratch on 3/1/00 by Gary R. Huber
// Copyright 2000 Questar Productions

#include "stdafx.h"

#ifndef WCS_DBFILTEREVENT_H
#define WCS_DBFILTEREVENT_H

#include "GeoRegister.h"

class Joe;

#define WCS_DBFILTER_EVENTTYPE_ADD	1
#define WCS_DBFILTER_EVENTTYPE_SUB	0

enum
	{
	WCS_DBFILTER_ATTRIBUTE_EXISTS = 1,
	WCS_DBFILTER_ATTRIBUTE_EQUALS,
	WCS_DBFILTER_ATTRIBUTE_GREATER,
	WCS_DBFILTER_ATTRIBUTE_GREATEREQUALS,
	WCS_DBFILTER_ATTRIBUTE_LESS,
	WCS_DBFILTER_ATTRIBUTE_LESSEQUALS,
	WCS_DBFILTER_ATTRIBUTE_SIMILAR
	}; // attribute comparison methods, these are stored in files so don't change order

enum
	{
	WCS_DBFILTER_GEOBOUNDS_INSIDE = 1,
	WCS_DBFILTER_GEOBOUNDS_OUTSIDE
	}; // geographic bounds comparisons

class DBFilterEvent
	{
	public:
		char EventType, Enabled, PassControlPt, PassVector, PassDEM, PassEnabled, PassDisabled, PassLine, PassPoint,
			LayerEquals, LayerSimilar, LayerNumeric, LayerNot, NameEquals, NameSimilar, NameNumeric, NameNot, 
			LabelEquals, LabelSimilar, LabelNumeric, LabelNot, AttributeTest, AttributeNot,
			GeoBndsInside, GeoBndsOutside, GeoBndsCompletely, GeoPtContained, GeoPtUncontained, StashPassDisabled, StashPassEnabled;
		char *Layer, *Name, *Label, *Attribute, *AttributeValue;
		DBFilterEvent *Next;
		GeoRegister GeoBnds, GeoPt;

		DBFilterEvent();
		~DBFilterEvent();
		void SetScriptDefaults(void);
		void Copy(DBFilterEvent *CopyTo, DBFilterEvent *CopyFrom);
		void FreeAll(void);
		void FreeLayer(void);
		void FreeName(void);
		void FreeLabel(void);
		void FreeAttribute(void);
		void FreeAttributeValue(void);
		char *NewLayer(char *SetLayer);
		char *NewName(char *SetName);
		char *NewLabel(char *SetLabel);
		char *NewAttribute(char *SetAttribute);
		char *NewAttributeValue(char *SetAttributeValue);
		void SetBounds(double LatRange[2], double LonRange[2]);
		void SetGeoPoint(double LatPt, double LonPt);
		int PassJoe(Joe *Inspect);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);

	}; // class DBFilterEvent

// IO codes
#define WCS_DBFILTER_EVENTTYPE			0x00210000
#define WCS_DBFILTER_ENABLED			0x00220000
#define WCS_DBFILTER_PASSCONTROL		0x00230000
#define WCS_DBFILTER_PASSVECTOR			0x00240000
#define WCS_DBFILTER_PASSDEM			0x00250000
#define WCS_DBFILTER_PASSENABLED		0x00260000
#define WCS_DBFILTER_PASSDISABLED		0x00270000
#define WCS_DBFILTER_PASSLINE			0x00280000
#define WCS_DBFILTER_PASSPOINT			0x00290000
#define WCS_DBFILTER_LAYEREQUALS		0x002a0000
#define WCS_DBFILTER_LAYERSIMILAR		0x002b0000
#define WCS_DBFILTER_LAYERNUMERIC		0x002c0000
#define WCS_DBFILTER_LAYERNOT			0x002d0000
#define WCS_DBFILTER_NAMEEQUALS			0x002e0000
#define WCS_DBFILTER_NAMESIMILAR		0x002f0000
#define WCS_DBFILTER_NAMENUMERIC		0x00310000
#define WCS_DBFILTER_NAMENOT			0x00320000
#define WCS_DBFILTER_LABELEQUALS		0x00330000
#define WCS_DBFILTER_LABELSIMILAR		0x00340000
#define WCS_DBFILTER_LABELNUMERIC		0x00350000
#define WCS_DBFILTER_LABELNOT			0x00360000
#define WCS_DBFILTER_LAYER				0x00370000
#define WCS_DBFILTER_NAME				0x00380000
#define WCS_DBFILTER_LABEL				0x00390000
#define WCS_DBFILTER_ATTRIBUTE			0x003a0000
#define WCS_DBFILTER_ATTRIBUTEVALUE		0x003b0000
#define WCS_DBFILTER_ATTRIBUTETEST		0x003c0000
#define WCS_DBFILTER_ATTRIBUTENOT		0x003d0000
#define WCS_DBFILTER_GEOBNDSINSIDE		0x003e0000
#define WCS_DBFILTER_GEOBNDSOUTSIDE		0x003f0000
#define WCS_DBFILTER_GEOBNDSCOMPLETELY	0x00410000
#define WCS_DBFILTER_GEOBNDS			0x00420000
#define WCS_DBFILTER_GEOPTCONTAINED		0x00430000
#define WCS_DBFILTER_GEOPTUNCONTAINED	0x00440000
#define WCS_DBFILTER_GEOPT				0x00450000

#define WCS_PARAM_DONE				0xffff0000

#endif // WCS_DBFILTEREVENT_H
