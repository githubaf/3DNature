// NVQueryAction.h
// Code to handle actions and queries
// Created from scratch on 01/21/05 (Rhys' one-month birthday!) by CXH
// Copyright 2005

#ifndef NVW_NVQUERYACTION_H
#define NVW_NVQUERYACTION_H

#include "ActionDB.h"

unsigned long int PerformQueryAction(float X, float Y, bool NonIntrusive);

// de-allocates the NVAction records in the passed container
unsigned long int PerformActionsFromActionRecords(std::vector<NVAction *> ActionRecords, float X, float Y, bool NonIntrusive, osg::Geometry *HighlightGeom, osg::Node *HighlightNode);
void HighlightObject(osg::Geometry *HighlightGeom, osg::Node *HighlightNode);
void ClearAllHighlights();

#endif // NVW_NVQUERYACTION_H
