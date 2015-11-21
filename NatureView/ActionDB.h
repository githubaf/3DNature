// ActionDB.h
// Code to support database lookups of the proper action to go with a queryaction ActionID
// Written from scratch by CXH
// Copyright 2005

#ifndef NVW_ACTIONDB_H
#define NVW_ACTIONDB_H

#include <vector>
#include <stdio.h>
#include "PropertyMacros.h"



class NVAction
	{
	// Properties
		PROPERTY_RW(signed char, ActionType)
		PROPERTY_RW(std::string, ActionString)

	public:
		NVAction() : _ActionType(-1){};
		NVAction(signed char Type) : _ActionType(Type){};
		unsigned short int ReadActionString(FILE *Input, unsigned short InputLen);

		// Originally from WCS/VNS's SXQueryAction.h
		enum
			{
			WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYLABEL,
			WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYDATATABLE,
			WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGE,
			WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGE,
			WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILE,
			WCS_SXQUERYACTION_ACTIONTYPE_HIGHLIGHTOBJECTSET, // or Highlight Object, perhaps
			WCS_SXQUERYACTION_ACTIONTYPE_LOADNEWSCENE,
			WCS_SXQUERYACTION_ACTIONTYPE_RUNSHELLCOMMAND, // NVX only
			WCS_SXQUERYACTION_ACTIONTYPE_LAUNCHPLUGIN, // NVX only
			WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGEINTERNAL,
			WCS_SXQUERYACTION_ACTIONTYPE_PLAYSOUNDINTERNAL,
			WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGEINTERNAL,
			WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILEINTERNAL,
			WCS_SXQUERYACTION_ACTIONTYPE_VIEWTEXTFILE,
			WCS_SXQUERYACTION_NUMACTIONTYPES
			}; // ActionTypes

	}; // NVAction


unsigned long int FetchActionsFromID(const signed long int ActionID, std::vector<NVAction *> &Results);
unsigned long int FetchActionRecordNumbersFromID(const signed long int ActionID, std::vector<unsigned long int> &ActionRecordNumbers);
unsigned long int FetchActionRecordsFromRecordNumbers(const std::vector<unsigned long int> &ActionRecordNumbers, std::vector<NVAction *> &Results);
bool FetchRecordOffsetFromRecordNumber(unsigned long int &RecordOffset, signed long int ActionID, const std::string FilePathAndName);


#endif // NVW_ACTIONDB_H

