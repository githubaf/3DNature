// DataOpsGPS.cpp
// Routines for handling GPS files
// Created 05/15/06 by Frank Weed II
// Copyright 2006 3D Nature, LLC

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Useful.h"
#include "Project.h"
#include "resource.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "GraphData.h"
#include "DBFilterEvent.h"
#include "Interactive.h"
#include "Conservatory.h"
#include "AppMem.h"
#include "PathAndFile.h"
#include "UsefulPathString.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

/*
 * Useful functions for GPX debugging:
 *	xmlTextReaderGetParserColumnNumber
 *	xmlTextReaderGetParserLineNumber
 *
 */

using namespace std;

static Joe *gpsJoe;
static VectorPoint *pLink;
static char gpsLine[7][1024];
static vector<string> vAttribNames;
//static vector<string>::iterator vANIter;
static TextColumnMarkerWidgetInstance TCMWI;
static vector<unsigned long> fixedCols;

/*===========================================================================*/

//FILE *fDump = NULL;



/*
 *To compile this file using gcc you can type
 *gcc `xml2-config --cflags --libs` -o xmlexample libxml2-example.c
 */

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
static void
print_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            //fprintf(fDump, "node type: Element, name: %s\n", cur_node->name);
        }

        print_element_names(cur_node->children);
    }
}


/**
 * Simple example to parse a file called "file.xml", 
 * walk down the DOM, and print the name of the 
 * xml elements nodes.
 */
int Hijack2(void)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

//fDump = fopen("C:/fells_loop.txt", "w");

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile("C:/fells_loop.gpx", NULL, 0);

//    if (doc == NULL) {
//        fprintf(fDump, "error: could not parse file\n");
//    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    print_element_names(root_element);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

//fclose(fDump);
    return 0;
}

/*===========================================================================*/

void ImportWizGUI::ConfigureGPSFieldsList1(void)
{
FILE *fGPS;
char *ColumnNames[] = {"Line", "Input"};
//LayerEntry *Entry, **LayerPtrs = NULL;
//LayerStub *CurStub;
//Joe *CurJoe;
//const char *LayerName, **AttribNames = NULL;
//long AttribTotal, JoeTotal, JoeTotalCount;
char Str[128];
//long column, subItemCt;
//double AttribValue;
HWND PanelID;
LV_COLUMN columnData;
LV_ITEM itemData;
BusyWin *BWAT = NULL;
//char *contents;
char *field;
long fields[64], i = 0, numFields;
//char delim[8];
char parsing[256];

fGPS = PROJ_fopen(Importing->LoadName, "r");

fgetline(&gpsLine[0][0], 1024, fGPS);
fgetline(&gpsLine[1][0], 1024, fGPS);
fgetline(&gpsLine[2][0], 1024, fGPS);
fgetline(&gpsLine[3][0], 1024, fGPS);
fgetline(&gpsLine[4][0], 1024, fGPS);
fgetline(&gpsLine[5][0], 1024, fGPS);
fgetline(&gpsLine[6][0], 1024, fGPS);

fclose(fGPS);

for (i = 0; i < 64; i++)
	fields[i] = 0;

i = 0;
strcpy(parsing, Importing->gpsParsing);
field = strtok(parsing, ",");
while (field)
	{
	fields[i++] = atol(field);
	field = strtok(NULL, ",");
	} // while

numFields = i;
columnData.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
columnData.cx = 20;

PanelID = SubPanels[0][29];

columnData.pszText = ColumnNames[0];
columnData.iSubItem = 0;
if ((columnData.cx = SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW1, LVM_GETSTRINGWIDTH, 0, (LPARAM)ColumnNames[0])) <= 0)
	columnData.cx = 20;
columnData.cx += 13;
SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW1, LVM_INSERTCOLUMN, 0, (LPARAM)&columnData);

columnData.pszText = ColumnNames[1];
columnData.iSubItem = 1;
if ((columnData.cx = SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW1, LVM_GETSTRINGWIDTH, 0, (LPARAM)ColumnNames[1])) <= 0)
	columnData.cx = 20;
columnData.cx += 308;
SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW1, LVM_INSERTCOLUMN, 1, (LPARAM)&columnData);

// this makes adding large numbers of items more efficient
SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW1, LVM_SETITEMCOUNT, 7, 0);

// clear the list in case we've been here before
SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW1, LVM_DELETEALLITEMS, 0, 0);
for (i = 0; i < 7; i++)
	{
	itemData.mask = LVIF_TEXT;
	itemData.iItem = i;
	itemData.iSubItem = 0;
	itemData.pszText = Str;
	sprintf(Str, "%d", i + 1);
	SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW1, LVM_INSERTITEM, 0, (LPARAM)&itemData);
	itemData.pszText = &gpsLine[i][0];
	itemData.iSubItem = 1;
	SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW1, LVM_SETITEMTEXT, i, (LPARAM)&itemData);
	} // for

} // ImportWizGUI::ConfigureGPSFieldsList1

/*===========================================================================*/

void ImportWizGUI::ConfigureGPSFieldsList2D(void)
{
FILE *fGPS;
//char *ColumnNames[] = {"Line", "Input"};
char *ColumnNames[] = {"Input"};
//LayerEntry *Entry, **LayerPtrs = NULL;
//LayerStub *CurStub;
//Joe *CurJoe;
//const char *LayerName, **AttribNames = NULL;
//long AttribTotal, JoeTotal, JoeTotalCount;
//long column;
//double AttribValue;
HWND PanelID;
LV_COLUMN columnData;
LV_ITEM itemData;
BusyWin *BWAT = NULL;
//char *contents;
char *field;
long fields[64], i, numFields;
char parsing[256];
//char input[1024];
//char Str[128];

fGPS = PROJ_fopen(Importing->LoadName, "r");

gpsLine[6][0] = gpsLine[5][0] = gpsLine[4][0] = gpsLine[3][0] = gpsLine[2][0] = gpsLine[1][0] = gpsLine[0][0] = 0;

i = 0;
while (i < Importing->gpsStart)
	{
	fgetline(&gpsLine[0][0], 1024, fGPS);
	i++;
	} // while
fgetline(&gpsLine[1][0], 1024, fGPS);
fgetline(&gpsLine[2][0], 1024, fGPS);
fgetline(&gpsLine[3][0], 1024, fGPS);
fgetline(&gpsLine[4][0], 1024, fGPS);
fgetline(&gpsLine[5][0], 1024, fGPS);
fgetline(&gpsLine[6][0], 1024, fGPS);

fclose(fGPS);

SetGPSDelims();

for (i = 0; i < 64; i++)
	fields[i] = 0;

i = 0;
strcpy(parsing, Importing->gpsParsing);
field = strtok(parsing, ",");
while (field)
	{
	fields[i++] = atol(field);
	field = strtok(NULL, ",");
	} // while

numFields = i;

PanelID = SubPanels[0][31];

// I don't know why we need these lines, as there's no display of the column on this widget, but it doesn't work without this code.
columnData.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
columnData.cx = 20;
columnData.pszText = ColumnNames[0];
columnData.iSubItem = 0;
if ((columnData.cx = SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW2D, LVM_GETSTRINGWIDTH, 0, (LPARAM)ColumnNames[0])) <= 0)
	columnData.cx = 20;
columnData.cx += 321;
SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW2D, LVM_INSERTCOLUMN, 0, (LPARAM)&columnData);

// this makes adding large numbers of items more efficient
SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW2D, LVM_SETITEMCOUNT, 7, 0);

for (i = 0; i < 7; i++)
	{
	itemData.mask = LVIF_TEXT;
	itemData.iItem = i;
	itemData.iSubItem = 0;
	itemData.pszText = gpsLine[i];
	SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW2D, LVM_INSERTITEM, 0, (LPARAM)&itemData);
	} // for

} // ImportWizGUI::ConfigureGPSFieldsList2D

/*===========================================================================*/

void ImportWizGUI::ConfigureGPSFieldsList2F(void)
{
FILE *fGPS;
long i;
static bool beenHere = 0;
string newline;
static string displayStr;

if (! beenHere)
	{
	fGPS = PROJ_fopen(Importing->LoadName, "r");

	gpsLine[6][0] = gpsLine[5][0] = gpsLine[4][0] = gpsLine[3][0] = gpsLine[2][0] = gpsLine[1][0] = gpsLine[0][0] = 0;

	i = 0;
	while (i < Importing->gpsStart)
		{
		fgetline(&gpsLine[0][0], 1024, fGPS);
		i++;
		} // while
	fgetline(&gpsLine[1][0], 1024, fGPS);
	fgetline(&gpsLine[2][0], 1024, fGPS);
	fgetline(&gpsLine[3][0], 1024, fGPS);
	fgetline(&gpsLine[4][0], 1024, fGPS);
	fgetline(&gpsLine[5][0], 1024, fGPS);
	fgetline(&gpsLine[6][0], 1024, fGPS);

	fclose(fGPS);

	newline = "\n";
	displayStr = gpsLine[0];
	displayStr += newline;
	displayStr += gpsLine[1];
	displayStr += newline;
	displayStr += gpsLine[2];
	displayStr += newline;
	displayStr += gpsLine[3];
	displayStr += newline;
	displayStr += gpsLine[4];
	displayStr += newline;
	displayStr += gpsLine[5];
	displayStr += newline;
	displayStr += gpsLine[6];
	WidgetTCConfig(IDC_GPS_FIXEDFIELDS, &TCMWI);
	WidgetTCSetText(IDC_GPS_FIXEDFIELDS, displayStr.c_str());
	beenHere = 1;
	Importing->gpsFileType = 3;
	} // if

} // ImportWizGUI::ConfigureGPSFieldsList2F

/*===========================================================================*/

void ImportWizGUI::ConfigureGPSFieldsList3(void)
{
char *ColumnNames[] = {"< err >", "Lat", "Lon", "Elev", "Name", "Label", "Layer", "Attribute", "Ignore"};
FILE *fGPS;
//LayerEntry *Entry, **LayerPtrs = NULL;
//LayerStub *CurStub;
//Joe *CurJoe;
//const char *LayerName, **AttribNames = NULL;
//long AttribTotal, JoeTotal, JoeTotalCount;
long column;
//double AttribValue;
HWND PanelID;
LV_COLUMN columnData;
LV_ITEM itemData;
BusyWin *BWAT = NULL;
char *contents, *field;
unsigned long u;
long fields[64], i, j, numFields;
int colNum, lineNum;
char parsing[256];
char input[1024];
char stash;
char tq;

fGPS = PROJ_fopen(Importing->LoadName, "r");

i = 0;
while (i < Importing->gpsStart)
	{
	fgetline(&gpsLine[0][0], 1024, fGPS);
	i++;
	} // while
fgetline(&gpsLine[1][0], 1024, fGPS);
fgetline(&gpsLine[2][0], 1024, fGPS);
fgetline(&gpsLine[3][0], 1024, fGPS);
fgetline(&gpsLine[4][0], 1024, fGPS);
fgetline(&gpsLine[5][0], 1024, fGPS);
fgetline(&gpsLine[6][0], 1024, fGPS);

fclose(fGPS);

(void)WidgetCBAddEnd(IDC_GPS_FIELDIS, "Lat");
(void)WidgetCBAddEnd(IDC_GPS_FIELDIS, "Lon");
(void)WidgetCBAddEnd(IDC_GPS_FIELDIS, "Elev");
(void)WidgetCBAddEnd(IDC_GPS_FIELDIS, "Name");
(void)WidgetCBAddEnd(IDC_GPS_FIELDIS, "Label");
(void)WidgetCBAddEnd(IDC_GPS_FIELDIS, "Layer");
(void)WidgetCBAddEnd(IDC_GPS_FIELDIS, "Attrib");
(void)WidgetCBAddEnd(IDC_GPS_FIELDIS, "Ignore");
WidgetCBSetCurSel(IDC_GPS_FIELDIS, 7);

for (i = 0; i < 64; i++)
	fields[i] = 0;

if (Importing->gpsFieldType)	// fixed width
	{
	for (u = 0; u <= fixedCols.size(); u++)
		{
		strcat(Importing->gpsParsing, "8,");
		if ((u + 1) > fixedCols.size())
			Importing->gpsParsing[strlen(Importing->gpsParsing) - 1] = 0;	// delete last comma
		} // for
	} // if

if (Importing->gpsParsing[0] == 0)
	GenGPSParsing();

i = 0;
strcpy(parsing, Importing->gpsParsing);
field = strtok(parsing, ",");
while (field)
	{
	fields[i++] = atol(field);
	field = strtok(NULL, ",");
	} // while

numFields = i;

vAttribNames.clear();
for (j = 0; j < i; ++j)
	vAttribNames.push_back("");
//j = vAttribNames.size();
//for (vANIter = vAttribNames.begin(); vANIter != vAttribNames.end(); ++vANIter)
//	printf("%s", *vANIter);

columnData.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
columnData.cx = 20;

PanelID = SubPanels[0][32];

for (column = 0; column < numFields; column++)
	{
	columnData.pszText = ColumnNames[fields[column]];
	columnData.iSubItem = column;
	if ((columnData.cx = SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW3, LVM_GETSTRINGWIDTH, 0, (LPARAM)ColumnNames[fields[column]])) <= 0)
		columnData.cx = 20;
	columnData.cx += 30;
	SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW3, LVM_INSERTCOLUMN, column, (LPARAM)&columnData);
	} // for

// this makes adding large numbers of items more efficient
SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW3, LVM_SETITEMCOUNT, 7, 0);

switch (Importing->gpsTextQualifier)
	{
	case IMWIZ_GPS_TEXTQUAL_DBLQUOTE:
		tq = '"';
		break;
	case IMWIZ_GPS_TEXTQUAL_APOSTROPHE:
		tq = '\'';
		break;
	case IMWIZ_GPS_TEXTQUAL_NONE:
	default:
		tq = 0;
	} // switch

if (Importing->gpsFieldType)	// fixed width
	{
	for (lineNum = 0; lineNum < 7; lineNum++)
		{
		strcpy(input, gpsLine[lineNum]);
		itemData.mask = LVIF_TEXT;
		itemData.iItem = lineNum;
		itemData.iSubItem = 0;
		itemData.pszText = &input[0];
		if (numFields > 1)
			{
			stash = input[fixedCols[0]];
			input[fixedCols[0]] = 0;
			} // if
		SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW3, LVM_INSERTITEM, 0, (LPARAM)&itemData);
		input[fixedCols[0]] = stash;
		for (colNum = 1; colNum < numFields; colNum++)
			{
			itemData.pszText = &input[fixedCols[colNum - 1]];
			itemData.iSubItem = colNum;
			if (colNum != fixedCols.size())
				{
				stash = input[fixedCols[colNum]];
				input[fixedCols[colNum]] = 0;
				SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW3, LVM_SETITEMTEXT, lineNum, (LPARAM)&itemData);
				input[fixedCols[colNum]] = stash;
				} // if
			else
				{
				SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW3, LVM_SETITEMTEXT, lineNum, (LPARAM)&itemData);
				}
			} // for
		} // for
	} // if
else
	{
	for (lineNum = 0; lineNum < 7; lineNum++)
		{
		strcpy(input, gpsLine[lineNum]);
		contents = strtok2(input, Importing->gpsDelims, tq);
		itemData.mask = LVIF_TEXT;
		itemData.iItem = lineNum;
		itemData.iSubItem = 0;
		itemData.pszText = contents;
		SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW3, LVM_INSERTITEM, 0, (LPARAM)&itemData);
		for (colNum = 1; colNum < numFields; colNum++)
			{
			contents = strtok2(NULL, Importing->gpsDelims, tq);
			itemData.pszText = contents;
			itemData.iSubItem = colNum;
			SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW3, LVM_SETITEMTEXT, lineNum, (LPARAM)&itemData);
			} // for
		} // for
	} // else

WidgetSetDisabled(IDC_GPS_ATTRIBNAME, 1);

} // ImportWizGUI::ConfigureGPSFieldsList3

/*===========================================================================*/

void ImportWizGUI::DoGPSAttribName(void)
{
char attribName[80];

if (WidgetGetModified(IDC_GPS_ATTRIBNAME))
	{
	(void)WidgetGetText(IDC_GPS_ATTRIBNAME, 79, attribName);
	vAttribNames[Importing->gpsColumn] = attribName;
	} // if

} // ImportWizGUI::DoGPSAttribName

/*===========================================================================*/

void ImportWizGUI::DoGPSFieldChange()
{
char *choices[] = {"Lat", "Lon", "Elev", "Name", "Label", "Layer", "Attrib", "Ignore"};
HWND PanelID;
LVCOLUMN columnData;
long witch;
char disability;

witch = WidgetCBGetCurSel(IDC_GPS_FIELDIS);
columnData.mask = LVCF_TEXT;
PanelID = SubPanels[0][32];
columnData.pszText = choices[witch];
SendDlgItemMessage(PanelID, IDC_GPS_FILEPREVIEW3, LVM_SETCOLUMN, (WPARAM)(int)Importing->gpsColumn, (LPARAM)&columnData);
Importing->gpsParsing[Importing->gpsColumn * 2] = (char)(witch + 49);

if (Importing->gpsColumn >= 0)
	{
	disability = (6 != witch);
	WidgetSetText(IDC_GPS_ATTRIBNAME, vAttribNames[Importing->gpsColumn].c_str());
	WidgetSetDisabled(IDC_GPS_ATTRIBNAME, disability);
	} // if

} // ImportWizGUI::DoGPSFieldChange()

/*===========================================================================*/

void ImportWizGUI::GenGPSParsing(void)
{
char *token;
char parseMe[1024];
char tq;

SetGPSDelims();

switch (Importing->gpsTextQualifier)
	{
	case IMWIZ_GPS_TEXTQUAL_DBLQUOTE:
		tq = '"';
		break;
	case IMWIZ_GPS_TEXTQUAL_APOSTROPHE:
		tq = '\'';
		break;
	case IMWIZ_GPS_TEXTQUAL_NONE:
	default:
		tq = 0;
	} // switch

strcpy(parseMe, gpsLine[0]);
token = strtok2(parseMe, Importing->gpsDelims, tq);
while (token)
	{
	strcat(Importing->gpsParsing, "8,");
	token = strtok2(NULL, Importing->gpsDelims, tq);
	if (!token)
		Importing->gpsParsing[strlen(Importing->gpsParsing) - 1] = 0;	// delete last comma
	} // while

} // ImportWizGUI::GenGPSParsing

/*===========================================================================*/

void ImportWizGUI::GPSParseLineD(char *string, long passNum)
{
char *contents, *field;
long fields[64], i = 0;
char parsing[256];
char tq;

for (i = 0; i < 64; i++)
	fields[i] = 0;

i = 0;
strcpy(parsing, Importing->gpsParsing);
field = strtok(parsing, ",");
while (field)
	{
	fields[i++] = atol(field);
	field = strtok(NULL, ",");
	} // while

switch (Importing->gpsTextQualifier)
	{
	case IMWIZ_GPS_TEXTQUAL_DBLQUOTE:
		tq = '"';
		break;
	case IMWIZ_GPS_TEXTQUAL_APOSTROPHE:
		tq = '\'';
		break;
	case IMWIZ_GPS_TEXTQUAL_NONE:
	default:
		tq = 0;
	} // switch

i = 0;
contents = strtok2(string, Importing->gpsDelims, tq);

if (passNum == 1)
	{
	// set up the Joe before adding to the database with these
	do
		{
		switch (fields[i])
			{
			default:
			case 8: // ignore
				break;
			case 1:	// Lat
				pLink->Latitude = ReadGPSLatField(contents);
				break;
			case 2: // Lon
				pLink->Longitude = ReadGPSLonField(contents);
				break;
			case 3: // Elev
				pLink->Elevation = (float)ReadGPSElevField(contents);
				break;
			case 4: // Name
				gpsJoe->SetNewNames(NULL, (const char *)contents);
				break;
			case 5: // Label
				gpsJoe->SetNewNames((const char *)contents, NULL);
				break;
			} // switch
		i++;
		contents = strtok2(NULL, Importing->gpsDelims, tq);
		} while (contents && fields[i]);
	} // if
else
	{
	// the Joe has to be in the database first for these
	do
		{
		switch (fields[i])
			{
			default:
			case 8: // ignore
				break;
			case 6: // Layer
				ReadGPSLayerField(contents);
				break;
			case 7: // Attribute
				ReadGPSAttribField(gpsJoe, contents, i);
				break;
			} // switch
		i++;
		contents = strtok2(NULL, Importing->gpsDelims, tq);
		} while (fields[i]);
	} // else

} // ImportWizGUI::GPSParseLineD

/*===========================================================================*/

void ImportWizGUI::GPSParseLineF(char* gpsLine, long passNum)
{
//const char *command;
char* field = gpsLine;
unsigned long i = 0;
long fieldCol = 0;
size_t len, pos = 0;
string str, sub;
char parsing[256];
char fieldType;

fixedCols.push_back((unsigned long)strlen(gpsLine));
len = fixedCols[0];

str = gpsLine;

strcpy(parsing, Importing->gpsParsing);
field = strtok(parsing, ",");
if (passNum == 1)
	{
	// set up the Joe before adding to the database with these
	do
		{
		sub = str.substr(pos, len);
		fieldType = *field;
		switch (fieldType)
			{
			default:
			case '8': // ignore
				break;
			case '1':	// Lat
				pLink->Latitude = ReadGPSLatField(sub.c_str());
				break;
			case '2': // Lon
				pLink->Longitude = ReadGPSLonField(sub.c_str());
				break;
			case '3': // Elev
				pLink->Elevation = (float)ReadGPSElevField(sub.c_str());
				break;
			case '4': // Name
				gpsJoe->SetNewNames(NULL, sub.c_str());
				break;
			case '5': // Label
				gpsJoe->SetNewNames(sub.c_str(), NULL);
				break;
			} // switch
		field = strtok(NULL, ",");
		if (field)
			{
			pos = fixedCols[i++];
			len = fixedCols[i] - pos;
			} // if
		} while (field);
	} // if
else
	{
	// the Joe has to be in the database first for these
	do
		{
		sub = str.substr(pos, len);
		fieldType = *field;
		switch (fieldType)
			{
			default:
			case '8': // ignore
				break;
			case '6': // Layer
				ReadGPSLayerField((char *)sub.c_str());
				break;
			case '7': // Attribute
				ReadGPSAttribField(gpsJoe, sub.c_str(), fieldCol);
				break;
			} // switch
		fieldCol++;
		field = strtok(NULL, ",");
		if (field)
			{
			pos = fixedCols[i++];
			len = fixedCols[i] - pos;
			} // if
		} while (field);
	} // else

fixedCols.pop_back();

} // ImportWizGUI::GPSParseLineF

/*===========================================================================*/

long ImportWizGUI::HandleTextColumnMarkerChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long ChangeMode)
{

WidgetTCGetData(IDC_GPS_FIXEDFIELDS, &fixedCols);
sort(fixedCols.begin(), fixedCols.end());

return(0);

} // ImportWizGUI::HandleTextColumnMarkerChange

/*===========================================================================*/

short ImportWizGUI::LoadASCII_GPS(char *filename, char TestOnly)
{
FILE *fGPS;
unsigned short row = 0;
short result = 0;
char gpsLine[1024];

if (TestOnly)
	{
	Importing->LoadAs = LAS_CP;
	if ((fGPS = PROJ_fopen(filename, "r")) == NULL)
		{
		return (2);
		} // if file open error
	fgetline(gpsLine, 1024, fGPS);
	row++;
	if (! strncmp(gpsLine, "OziExplorer", 11))
		{
		Importing->gpsFileType = 1;
		Importing->gpsHasDatum = 1;
		Importing->gpsDatum = 2;
		Importing->gpsStart = 5;
		Importing->gpsFieldType = IMWIZ_GPS_DELIMITED;
		Importing->gpsDComma = 1;
		strcpy(Importing->gpsParsing, "8,4,1,2,8,8,8,8,8,8,8,8,8,8,3");
		fgetline(gpsLine, 1024, fGPS);
		row++;
		if (! strcmp(gpsLine, "WGS 84"))
			{
			Importing->CoordSysWarn = 0;
			Importing->IWCoordSys.SetSystemByCode(12);	// Geographic WGS 84
			} // if
		} // if
	else if (! strcmp(gpsLine, "Version 2"))
		{
		Importing->gpsFileType = 2;
		fgetline(gpsLine, 1024, fGPS);
		row++;
		fgetline(gpsLine, 1024, fGPS);
		row++;
		if (! strncmp(gpsLine, "D ", 2))
			{
			long datumOK = 0;

			if (! strcmp(&gpsLine[2], "WGS-84"))
				{
				datumOK = 1;
				Importing->IWCoordSys.SetSystemByCode(12);	// Geographic WGS 84
				} // if
			if (datumOK)
				{
				Importing->gpsHasDatum = 1;
				Importing->gpsDatum = 3;
				Importing->CoordSysWarn = 0;
				} // if
			} // if
		fgetline(gpsLine, 1024, fGPS);
		row++;
		if (! strncmp(gpsLine, "M ", 2))
			{
			if (! strcmp(&gpsLine[2], "DDD"))
				{
				Importing->gpsNumFormat = IMWIZ_GPS_COORDS_DDD;
				} // if
			else if (! strcmp(&gpsLine[2], "DMS"))
				{
				Importing->gpsNumFormat = IMWIZ_GPS_COORDS_DMS;
				} // else if
			} // if
		do
			{
			fgetline(gpsLine, 1024, fGPS);
			row++;
			} while ((gpsLine[0] != 'T') && (! feof(fGPS)));
		Importing->gpsStart = row;
		} // if
	} // if
else
	{
	switch (Importing->gpsFileType)
		{
		default:
			result = ReadGPSFileType0(filename);
			break;
		case 1:
			result = ReadGPSFileType1(filename);
			break;
		case 2:
			result = ReadGPSFileType2(filename);
			break;
		case 3:
			result = ReadGPSFileType3(filename);
			break;
		} // switch
	} // else

return result;

} // ImportWizGUI::LoadASCII_GPS

/*===========================================================================*/

long ImportWizGUI::ParseField(long fieldNum)
{
char *field;
long fields[64], i = 0, rVal = 1;
char parsing[256];

if ((fieldNum >= 0) && (fieldNum < 64))
	{
	strcpy(parsing, Importing->gpsParsing);
	field = strtok(parsing, ",");
	while (field)
		{
		fields[i++] = atol(field);
		field = strtok(NULL, ",");
		} // while
	rVal = fields[fieldNum];
	} // if

return (rVal - 1);

} // ImportWizGUI::ParseField

/*===========================================================================*/

void ImportWizGUI::ReadGPSAttribField(Joe *gpsJoe, const char *string, long index)
{

#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
/***
if (numericAttribute)
	AverageGuy->AddIEEEAttribute(LayerDude, atof(TargetName));
else
	AverageGuy->AddTextAttribute(LayerDude, TargetName);
***/
gpsJoe->AddTextAttribute((char *)vAttribNames[index].c_str(), (char *)string);

#endif // WCS_SUPPORT_GENERIC_ATTRIBS

} // ImportWizGUI::ReadGPSAttribField

/*===========================================================================*/

double ImportWizGUI::ReadGPSElevField(const char *elevString)
{

return (atof(elevString));

} // ImportWizGUI::ReadGPSElevField

/*===========================================================================*/

void ImportWizGUI::ReadGPSLabelField(const char *string)
{
} // ImportWizGUI::ReadGPSLabelField

/*===========================================================================*/

//lint -save -e522
double ImportWizGUI::ReadGPSLatField(const char *latString)
{
double sign = 1.0, value = 0.0;
const char *first = latString;

while (isspace(*first))
	*first++;

if (*first == 'N')
	*first++;

if (*first == 'S')
	{
	*first++;
	sign = -1.0;
	} // if

if (Importing->gpsNumFormat == IMWIZ_GPS_COORDS_DDD)
	value = atof(first);
else if (Importing->gpsNumFormat == IMWIZ_GPS_COORDS_DMS)
	{
	double dPart, mPart, sPart;

	dPart = atof(first);
	*first++;
	*first++;
	*first++;
	mPart = atof(first);
	*first++;
	*first++;
	*first++;
	sPart = atof(first);
	value = dPart + mPart/60.0 + sPart/3600.0;
	} // else if

return (value * sign);

} // ImportWizGUI::ReadGPSLatField
//lint -restore

/*===========================================================================*/

void ImportWizGUI::ReadGPSLayerField(char* string)
{

DBHost->DBLayers.AddObjectToLayer(gpsJoe, string);

} // ImportWizGUI::ReadGPSLayerField

/*===========================================================================*/

//lint -save -e522
// Geographic Longitudes are W positive in WCS/VNS
double ImportWizGUI::ReadGPSLonField(const char* lonString)
{
double sign = -1.0, value = 0.0;
const char *first = lonString;

while (isspace(*first))
	*first++;

if (*first == 'E')
	*first++;

if (*first == 'W')
	{
	*first++;
	sign = 1.0;
	} // if

if (Importing->gpsNumFormat == IMWIZ_GPS_COORDS_DDD)
	value = atof(first);
else if (Importing->gpsNumFormat == IMWIZ_GPS_COORDS_DMS)
	{
	double dPart, mPart, sPart;

	dPart = atof(first);
	*first++;
	*first++;
	*first++;
	*first++;
	mPart = atof(first);
	*first++;
	*first++;
	*first++;
	sPart = atof(first);
	value = dPart + mPart/60.0 + sPart/3600.0;
	} // else if

return (value * sign);

} // ImportWizGUI::ReadGPSLonField
//lint -restore

/*===========================================================================*/

void ImportWizGUI::ReadGPSNameField(const char *string)
{
} // ImportWizGUI::ReadGPSNameField

/*===========================================================================*/

short ImportWizGUI::ReadGPSFileType0(char *filename)
{
FILE *fGPS;
Joe *afterGuy = NULL, *loadRoot;
PlanetOpt *DefPlanetOpt;
unsigned long joeNum = 0, row = 0;
char dbName[64], gpsLine[1024], gpsLine2[1024];

if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
	{
	if ((DefPlanetOpt->Coords) && (DefPlanetOpt->Coords->Equals(&Importing->IWCoordSys)))
		{
		Importing->IWCoordSys.Copy(&Importing->IWCoordSys, DefPlanetOpt->Coords);
		} // if
	} // if

loadRoot = new (Importing->NameBase) Joe;
DBHost->AddJoe(loadRoot, WCS_DATABASE_STATIC, ProjHost);

if (loadRoot && (fGPS = PROJ_fopen(filename, "r")))
	{
	gpsLine2[0] = 0;
	for (unsigned short i = 1; i <= Importing->gpsStart; i++)
		fgetline(gpsLine2, 1024, fGPS);
	while (! feof(fGPS))
		{
		sprintf(dbName, "%s%d", Importing->NameBase, joeNum++);
		gpsJoe = new (dbName) Joe;
		if (gpsJoe && gpsJoe->Points(DBHost->MasterPoint.Allocate(2)))
			{
			gpsJoe->TemplateItem = GlobalApp->TemplateLoadInProgress;
			gpsJoe->NumPoints(2);
			gpsJoe->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_ISCONTROL);
			gpsJoe->SetLineStyle(3);
			gpsJoe->SetLineWidth((unsigned char)1);
			gpsJoe->SetRGB(222, 222, 0);
			pLink = gpsJoe->GetFirstRealPoint();
			strcpy(gpsLine, gpsLine2);
			GPSParseLineD(gpsLine, 1);	// pass 1
#ifdef WCS_BUILD_VNS
			if (NewCoordSys)
				gpsJoe->AddEffect(NewCoordSys, -1);
			gpsJoe->RecheckBounds();
#endif // WCS_BUILD_VNS
			loadRoot->AddChild(gpsJoe, afterGuy);
			afterGuy = gpsJoe;
			strcpy(gpsLine, gpsLine2);
			GPSParseLineD(gpsLine, 2);	// pass 2
			} // if
		fgetline(gpsLine2, 1024, fGPS);
		}; // while
	} // if

if (afterGuy)
	{
	DBHost->SetActiveObj(afterGuy);
	DBHost->BoundUpTree(afterGuy);
	} // if

if (fGPS)
	{
	fclose(fGPS);
	fGPS = NULL;
	} // if

return 1;

} // ImportWizGUI::ReadGPSFileType0

/*===========================================================================*/

short ImportWizGUI::ReadGPSFileType1(char *filename)
{
FILE *fGPS;
Joe *afterGuy = NULL, *loadRoot;
PlanetOpt *DefPlanetOpt;
unsigned long joeNum = 0, row = 0;
char dbName[64], gpsLine[1024];

/***

if (LayerA[0])
	{
	DBHost->DBLayers.AddObjectToLayer(AverageGuy, LayerA);
	} // if

***/

if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
	{
	if ((DefPlanetOpt->Coords) && (DefPlanetOpt->Coords->Equals(&Importing->IWCoordSys)))
		{
		Importing->IWCoordSys.Copy(&Importing->IWCoordSys, DefPlanetOpt->Coords);
		} // if
	} // if

loadRoot = new (Importing->NameBase) Joe;
DBHost->AddJoe(loadRoot, WCS_DATABASE_STATIC, ProjHost);

if (loadRoot && (fGPS = PROJ_fopen(filename, "r")))
	{
	fgetline(gpsLine, 1024, fGPS);
	fgetline(gpsLine, 1024, fGPS);
	fgetline(gpsLine, 1024, fGPS);
	fgetline(gpsLine, 1024, fGPS);
	fgetline(gpsLine, 1024, fGPS);
	row = 5;
	while (! feof(fGPS))
		{
		sprintf(dbName, "%s%d", Importing->NameBase, joeNum++);
		gpsJoe = new (dbName) Joe;
		if (gpsJoe && gpsJoe->Points(DBHost->MasterPoint.Allocate(2)))
			{
			gpsJoe->TemplateItem = GlobalApp->TemplateLoadInProgress;
			gpsJoe->NumPoints(2);
			gpsJoe->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_ISCONTROL);
			gpsJoe->SetLineStyle(3);
			gpsJoe->SetLineWidth((unsigned char)1);
			gpsJoe->SetRGB(222, 222, 0);
			pLink = gpsJoe->GetFirstRealPoint();
			GPSParseLineD(gpsLine, 1);	// pass 1
#ifdef WCS_BUILD_VNS
			if (NewCoordSys)
				gpsJoe->AddEffect(NewCoordSys, -1);
			gpsJoe->RecheckBounds();
#endif // WCS_BUILD_VNS
			loadRoot->AddChild(gpsJoe, afterGuy);
			afterGuy = gpsJoe;
			GPSParseLineD(gpsLine, 2);	// pass 2
			} // if
		fgetline(gpsLine, 1024, fGPS);
		}; // while
	} // if

if (afterGuy)
	{
	DBHost->SetActiveObj(afterGuy);
	DBHost->BoundUpTree(afterGuy);
	} // if

if (fGPS)
	{
	fclose(fGPS);
	fGPS = NULL;
	} // if

return 1;

} // ImportWizGUI::ReadGPSFileType1

/*===========================================================================*/

short ImportWizGUI::ReadGPSFileType2(char *filename)
{
FILE *fGPS;
unsigned long joeNum = 0, row = 0;
//char dbName[64];
char gpsLine[1024];

if (fGPS = PROJ_fopen(filename, "r"))
	{
	while (! feof(fGPS))
		{
		row++;
		fgetline(gpsLine, 1024, fGPS);
		if (row == Importing->gpsDatum)
			{
			if ((gpsLine[0] == 'D') && (gpsLine[1] == ' '))
				{
				if (! strcmp(&gpsLine[2], "WGS-84"))
					{
					Importing->CoordSysWarn = 0;
					Importing->IWCoordSys.SetSystemByCode(352);	// WGS84
					} // if
				// If other datums are to be supported, use 'g7datums.txt' file in g7tow source.
				// The datum name is in the first CSV field.  Subtract the 'da' field from the WGS84 ellipsoid radius
				// to get the radius.  Take the 'df' field, divide by 10000, invert it, and subtract from the inverse
				// of the WGS84 flattening to get the inverse of the new flattening.  Use the dx, dy, and dz params,
				// but account for sign change that we use.
				} // if
			} // if
		else if (row >= Importing->gpsStart)
			{
			if ((gpsLine[0] == 'T') && (gpsLine[1] == ' '))
				{
				GPSParseLineF(gpsLine, 1);
				} // if
			} // else if
		} // while
	fclose(fGPS);
	} // if

return 1;

} // ImportWizGUI::ReadGPSFileType2

/*===========================================================================*/

short ImportWizGUI::ReadGPSFileType3(char *filename)
{
FILE *fGPS;
Joe *afterGuy = NULL, *loadRoot;
unsigned long joeNum = 0, row = 0;
char dbName[64], gpsLine[1024];

loadRoot = new (Importing->NameBase) Joe;
DBHost->AddJoe(loadRoot, WCS_DATABASE_STATIC, ProjHost);

if (loadRoot && (fGPS = PROJ_fopen(filename, "r")))
	{
	while (! feof(fGPS))
		{
		row++;
		fgetline(gpsLine, 1024, fGPS);
		if ((row >= Importing->gpsStart) && gpsLine[0])
			{
			sprintf(dbName, "%s%d", Importing->NameBase, joeNum++);
			gpsJoe = new (dbName) Joe;
			if (gpsJoe && gpsJoe->Points(DBHost->MasterPoint.Allocate(2)))
				{
				gpsJoe->TemplateItem = GlobalApp->TemplateLoadInProgress;
				gpsJoe->NumPoints(2);
				gpsJoe->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_ISCONTROL);
				gpsJoe->SetLineStyle(3);
				gpsJoe->SetLineWidth((unsigned char)1);
				gpsJoe->SetRGB(222, 222, 0);
				pLink = gpsJoe->GetFirstRealPoint();
				GPSParseLineF(gpsLine, 1);	// pass 1
#ifdef WCS_BUILD_VNS
				if (NewCoordSys)
					gpsJoe->AddEffect(NewCoordSys, -1);
				gpsJoe->RecheckBounds();
#endif // WCS_BUILD_VNS
				loadRoot->AddChild(gpsJoe, afterGuy);
				afterGuy = gpsJoe;
				GPSParseLineF(gpsLine, 2);	// pass 2
				} // if
			} // if
		} // while
	fclose(fGPS);
	} // if

if (afterGuy)
	{
	DBHost->SetActiveObj(afterGuy);
	DBHost->BoundUpTree(afterGuy);
	} // if

return 1;

} // ImportWizGUI::ReadGPSFileType3

/*===========================================================================*/

void ImportWizGUI::SetGPSDelims(void)
{

Importing->gpsDelims[0] = 0;
if (Importing->gpsDTab)
	strcat(Importing->gpsDelims, "\t");
if (Importing->gpsDSemicolon)
	strcat(Importing->gpsDelims, ";");
if (Importing->gpsDComma)
	strcat(Importing->gpsDelims, ",");
if (Importing->gpsDSpace)
	strcat(Importing->gpsDelims, " ");
if (Importing->gpsDOther)
	strcat(Importing->gpsDelims, &Importing->gpsDOtherChar[0]);
if (Importing->gpsTextQualifier)
	{
	switch (Importing->gpsTextQualifier)
		{
		case IMWIZ_GPS_TEXTQUAL_DBLQUOTE:
			strcat(Importing->gpsDelims, "\"");
			break;
		case IMWIZ_GPS_TEXTQUAL_APOSTROPHE:
			strcat(Importing->gpsDelims, "'");
			break;
		case IMWIZ_GPS_TEXTQUAL_NONE:
		default:
			break;
		} // switch
	strcat(Importing->gpsDelims, &Importing->gpsTextQualifier);
	} // if

} // ImportWizGUI::SetGPSDelims

/*===========================================================================*/

void ImportWizGUI::GPX_Node(xmlTextReaderPtr reader)
{
const xmlChar *attStr = NULL, *name, *value;
int attCount;

name = xmlTextReaderConstName(reader);
if (name == NULL)
	name = BAD_CAST "--";

value = xmlTextReaderConstValue(reader);

attCount = xmlTextReaderAttributeCount(reader);

#ifdef FDUMP
fprintf(fDump, "%d %d %s %d %d %d(%d)",
    xmlTextReaderDepth(reader),
    xmlTextReaderNodeType(reader),
    name,
    xmlTextReaderIsEmptyElement(reader),
    xmlTextReaderHasValue(reader),
	xmlTextReaderHasAttributes(reader),
	attCount);

if (value == NULL)
	fprintf(fDump, "\n");
else
	{
    if (xmlStrlen(value) > 40)
        fprintf(fDump, " %.40s...\n", value);
    else
    fprintf(fDump, " %s\n", value);
	} // else
#endif // FDUMP

//	if (xmlTextReaderHasAttributes(reader) && (stricmp((const char *)name, "gpx") == 0))
//		{
//		xmlChar *latStr = NULL, *lonStr = NULL;
//
//		attStr = xmlTextReaderGetAttribute(reader, (xmlChar *)"version");
//		attStr = xmlTextReaderGetAttribute(reader, (xmlChar *)"creator");
//		} // if

if (xmlTextReaderHasAttributes(reader) && (stricmp((const char *)name, "wpt") == 0))
	{
	static Joe *loadRoot = NULL;
	xmlChar *latStr = NULL, *lonStr = NULL;

	if (loadRoot == NULL)
		{
		loadRoot = new (Importing->NameBase) Joe;
		DBHost->AddJoe(loadRoot, WCS_DATABASE_STATIC, ProjHost);
		} // if

	latStr = xmlTextReaderGetAttribute(reader, (xmlChar *)"lat");
	lonStr = xmlTextReaderGetAttribute(reader, (xmlChar *)"lon");
	if (loadRoot && latStr && lonStr)
		GPX_Waypoint(loadRoot, reader, latStr, lonStr);
	// ele = elevation
	// time = UTC ISO 8601
	// geoidheight
	// name
	// desc = description
	// cmt = comment
	// sym = symbol
	// type = classification
	} // if

if (stricmp((const char *)name, "rte") == 0)
	{
	GPX_Route(reader);
	} // if

} // ImportWizGUI::GPX_Node

/*===========================================================================*/

void ImportWizGUI::GPX_Route(xmlTextReaderPtr reader)
{
Joe *loadRoot = NULL;
xmlChar *latStr = NULL, *lonStr = NULL;
int status;
bool getName = true;

status = xmlTextReaderRead(reader);
while (1 == status)
	{
	const xmlChar *name, *value;
	int type;

    name = xmlTextReaderConstName(reader);
    if (name == NULL)
		name = BAD_CAST "--";

	type = xmlTextReaderNodeType(reader);

	if (getName && (XML_ELEMENT_NODE == type) && (strcmp((const char *)name, "name") == 0))
		{
		getName = false;
		if (1 == xmlTextReaderRead(reader))
			{
			type = xmlTextReaderNodeType(reader);
			value = xmlTextReaderConstValue(reader);

			if (XML_TEXT_NODE == type)
				{
				loadRoot = new ((char *)value) Joe;	// named route
				DBHost->AddJoe(loadRoot, WCS_DATABASE_STATIC, ProjHost);
				} // if
			} // if
		} // if
	else if ((XML_ELEMENT_NODE == type) && (strcmp((const char *)name, "rtept") == 0))
		{
		if (loadRoot == NULL)	// if route is unnamed
			{
			loadRoot = new (Importing->NameBase) Joe;
			DBHost->AddJoe(loadRoot, WCS_DATABASE_STATIC, ProjHost);
			} // if

		latStr = xmlTextReaderGetAttribute(reader, (xmlChar *)"lat");
		lonStr = xmlTextReaderGetAttribute(reader, (xmlChar *)"lon");
		if (loadRoot && latStr && lonStr)
			GPX_Waypoint(loadRoot, reader, latStr, lonStr);
		} // else if

	status = xmlTextReaderRead(reader);

	}; // while

} // ImportWizGUI::GPX_Route

/*===========================================================================*/

void ImportWizGUI::GPX_Waypoint(Joe *loadRoot, xmlTextReaderPtr reader, xmlChar *latStr, xmlChar *lonStr)
{
Joe *afterGuy = NULL;
//xmlNodePtr node;
static unsigned long count = 0;
int status;
//const xmlChar *name, *value;//, *str;
//xmlNodePtr curnode, node;
//xmlElementType type;
//int ret;
bool getCom, getDesc, getElev, getName, getSym, getTime, getType;
char dbName[64];

getCom = getDesc = getElev = getName = getSym = getTime = getType = false;

/////
//node = xmlTextReaderExpand(reader);	// create a subtree for processing this waypoint
/////

sprintf(dbName, "%s%d", Importing->NameBase, count++);
gpsJoe = new (dbName) Joe;
if (gpsJoe && gpsJoe->Points(DBHost->MasterPoint.Allocate(2)))
	{
	gpsJoe->TemplateItem = GlobalApp->TemplateLoadInProgress;
	gpsJoe->NumPoints(2);
	gpsJoe->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_ISCONTROL);
	gpsJoe->SetLineStyle(3);
	gpsJoe->SetLineWidth((unsigned char)3);
	gpsJoe->SetRGB((unsigned char)127, (unsigned char)255, (unsigned char)63);
	pLink = gpsJoe->GetFirstRealPoint();
	pLink->Latitude = atof((const char *)latStr);
	pLink->Longitude = -(atof((const char *)lonStr));
#ifdef WCS_BUILD_VNS
	if (NewCoordSys)
		gpsJoe->AddEffect(NewCoordSys, -1);
	gpsJoe->RecheckBounds();
#endif // WCS_BUILD_VNS
	loadRoot->AddChild(gpsJoe, afterGuy);
	afterGuy = gpsJoe;
	} // if gpsJoe

status = xmlTextReaderRead(reader);
while (1 == status)
	{
	const xmlChar *attStr = NULL, *name, *value;
	int attCount, depth, empty, hasAttributes, hasValue, type;

	name = xmlTextReaderConstName(reader);
	if (name == NULL)
		name = BAD_CAST "--";

	attCount = xmlTextReaderAttributeCount(reader);
	depth = xmlTextReaderDepth(reader);
	empty = xmlTextReaderIsEmptyElement(reader);
	hasValue = xmlTextReaderHasValue(reader);
	hasAttributes = xmlTextReaderHasAttributes(reader);
	type = xmlTextReaderNodeType(reader);
	value = xmlTextReaderConstValue(reader);

	switch (type)
		{
		case XML_ELEMENT_NODE:			// 1
			if (strcmp((const char *)name, "cmt") == 0)
				{
				getCom = true;	// next read will have the comment in value
				} // if
			else if (strcmp((const char *)name, "desc") == 0)
				{
				getDesc = true;	// next read will have the description in value
				} // else if
			else if (strcmp((const char *)name, "ele") == 0)
				{
				getElev = true;	// next read will have the elevation in value
				} // else if
			else if (strcmp((const char *)name, "name") == 0)
				{
				getName = true; // next read will have the name in value
				} // else if
			else if (strcmp((const char *)name, "sym") == 0)
				{
				getSym = true;	// next read will have the symbol in value
				} // else if
			else if (strcmp((const char *)name, "time") == 0)
				{
				getTime = true;	// next read will have the time in value
				} // else if
			else if (strcmp((const char *)name, "type") == 0)
				{
				getType = true;	// next read will have the type in value
				} // else if
			break;
		case XML_TEXT_NODE:				// 3
			// F2_NOTE: We need to make sure these strings aren't too long
			if (getCom)
				{
				gpsJoe->AddTextAttribute("Comment", (char *)value);
				getSym = false;
				} // if
			if (getElev)
				{
				pLink->Elevation = (float)atof((const char *)value);
				getElev = false;
				} // if
			else if (getName)
				{
				gpsJoe->SetNewNames(NULL, (const char *)value);
				getName = false;
				} // else if
			else if (getSym)
				{
				gpsJoe->AddTextAttribute("Symbol", (char *)value);
				getSym = false;
				} // else if
			else if (getTime)
				{
				gpsJoe->AddTextAttribute("Time", (char *)value);
				getTime = false;
				} // else if
			break;
		case XML_CDATA_SECTION_NODE:	// 4
			if (getDesc)
				{
				gpsJoe->AddTextAttribute("Description", (char *)value);
				getDesc = false;
				} // if
			else if (getType)
				{
				gpsJoe->AddTextAttribute("Type", (char *)value);
				getType = false;
				} // else if
			break;
		default:
			break;
		} // switch

#ifdef FDUMP
	fprintf(fDump, "%d %d %s %d %d %d(%d)",
		depth, type, name, empty, hasValue, hasAttributes, attCount);

	if (value == NULL)
		fprintf(fDump, "\n");
	else
		{
		if (xmlStrlen(value) > 40)
			fprintf(fDump, " %.40s...\n", value);
		else
			fprintf(fDump, " %s\n", value);
		} // else
#endif // FDUMP

	if (!((15 == type) && ((strcmp((const char *)name, "wpt") == 0) || (strcmp((const char *)name, "rtept") == 0))))
		{
		status = xmlTextReaderRead(reader);
		} // if
	else
		status = 0;	// we're done processing this waypoints data
	}; // while

/***
node = xmlTextReaderCurrentNode(reader);

for (curnode = node; curnode; curnode = curnode->next)
	{
	type = curnode->type;
	name = curnode->name;
	}

ret = xmlTextReaderRead(reader);
do
	{
	name = xmlTextReaderConstName(reader);
	value = xmlTextReaderConstValue(reader);
	//str = xmlTextReaderConstString(reader);
	} while (ret);
***/

} // ImportWizGUI::GPX_Waypoint

/*===========================================================================*/

short ImportWizGUI::LoadGPX(char *filename, char TestOnly)
{
int status;
short result = 99;
xmlTextReaderPtr reader;
char fullname[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

//fDump = fopen("C:/fells_loop.txt", "w");	// F2NOTE: change fDump refs to generic debug only mode code

if (TestOnly)
	{
	Importing->LoadAs = LAS_VECT;
	} // if

/*
 * This initialize the library and checks potential ABI mismatches
 * between the version it was compiled for and the actual shared
 * library used.
 */
LIBXML_TEST_VERSION

strcpy(fullname, GlobalApp->MainProj->MungPath(filename));
reader = xmlReaderForFile(fullname, NULL, 0);
if (reader != NULL)
	{
	status = xmlTextReaderRead(reader);
	while (status == 1)
		{
		GPX_Node(reader);
		status = xmlTextReaderRead(reader);
		} // while
	xmlFreeTextReader(reader);
	if (status != 0)
		{
		fprintf(stderr, "%s : failed to parse\n", filename);	// F2NOTE: fix
		} // if
	else
		result = 0;	// success
	} // if
else
	{
	fprintf(stderr, "Unable to open %s\n", filename);	// F2NOTE: fix
	} // else

//fclose(fDump);

/*
 * Cleanup function for the XML library.
 */
xmlCleanupParser();

return(result);

} // ImportWizGUI::LoadGPX

/*===========================================================================*/

void ImportWizGUI::UpdateGPSAttribName()
{

WidgetSetText(IDC_GPS_ATTRIBNAME, vAttribNames[Importing->gpsColumn].c_str());

} // ImportWizGUI::UpdateGPSAttribName
