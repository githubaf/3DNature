// EDSSControl.cpp
// Code module for Ecosystem Decision Support System control class
// Created from scratch 10/15/01 by Gary R. Huber
// Copyright 2001 3D Nature. All rights reserved.

#include "stdafx.h"
#include "EDSSControl.h"
#include "Script.h"
#include "Toolbar.h"
#include "Project.h"
#include "Requester.h"

EDSSControl::EDSSControl(ArgRipper *Argh, int NumArgsParsed)
{
int Count;
char *Result, TotalTemplateName[512], ShapeFilePath[512];
NewProject *NewProj;
double FrameRate;
double EffectDefault[9] = {0.0, 0.0, 20.0, 0.0, 0.0, 10.0, 45.0, 3.0, 6.0};
double RangeDefaults[8][3] = {					90.0, -90.0, .0001,	// cam lat
												FLT_MAX, -FLT_MAX, .0001,	// cam lon
												FLT_MAX, -FLT_MAX, 1.0,	// cam elev
												90.0, -90.0, .0001,	// target lat
												FLT_MAX, -FLT_MAX, .0001,	// target lon
												FLT_MAX, -FLT_MAX, 1.0,	// target elev
												179.9, 0.0001, 1.0,	// fov
												FLT_MAX, 0.0, .01	// times
												};

InitError = 0;
NewProjectName[0] = 0;
TemplateName[0] = 0;
ShapeFileName[0] = 0;
EndKeyEnabled = MidKeyEnabled = 0;

for(Count = 0; Count < NumArgsParsed;)
	{
	if(Result = Argh->GetArg(7, Count)) // NEWPROJECT
		{
		strcpy(NewProjectName, Result);
		} // if
	else Count++;
	} // for
for(Count = 0; Count < NumArgsParsed;)
	{
	if(Result = Argh->GetArg(8, Count)) // TEMPLATE
		{
		strcpy(TemplateName, Result);
		} // if
	else Count++;
	} // for
for(Count = 0; Count < NumArgsParsed;)
	{
	if(Result = Argh->GetArg(9, Count)) // SHAPEFILE
		{
		strcpy(TotalTemplateName, Result);
		BreakFileName(TotalTemplateName, ShapeFilePath, 512, ShapeFileName, 512);
		StripExtension(ShapeFileName);
		AddExtension(ShapeFileName, "shp");
		} // if
	else Count++;
	} // for

if (! (NewProjectName[0] && TemplateName[0] && ShapeFileName[0]))
	{
	InitError = 1;
	return;
	} // if

// load template project
strmfp(TotalTemplateName, "WCSProjects:Templates", TemplateName);
AddExtension(TotalTemplateName, "proj");

FirstCamLatADT.SetDefaults(NULL, 0, EffectDefault[0]); // lat
FirstCamLatADT.SetRangeDefaults(RangeDefaults[0]);
MidCamLatADT.SetDefaults(NULL, 0, EffectDefault[0]);
MidCamLatADT.SetRangeDefaults(RangeDefaults[0]);
EndCamLatADT.SetDefaults(NULL, 0, EffectDefault[0]);
EndCamLatADT.SetRangeDefaults(RangeDefaults[0]);
FirstCamLonADT.SetDefaults(NULL, 0, EffectDefault[1]); // lon
FirstCamLonADT.SetRangeDefaults(RangeDefaults[1]);
MidCamLonADT.SetDefaults(NULL, 0, EffectDefault[1]);
MidCamLonADT.SetRangeDefaults(RangeDefaults[1]);
EndCamLonADT.SetDefaults(NULL, 0, EffectDefault[1]);
EndCamLonADT.SetRangeDefaults(RangeDefaults[1]);
FirstCamElevADT.SetDefaults(NULL, 0, EffectDefault[2]); // elev
FirstCamElevADT.SetRangeDefaults(RangeDefaults[2]);
MidCamElevADT.SetDefaults(NULL, 0, EffectDefault[2]);
MidCamElevADT.SetRangeDefaults(RangeDefaults[2]);
EndCamElevADT.SetDefaults(NULL, 0, EffectDefault[2]);
EndCamElevADT.SetRangeDefaults(RangeDefaults[2]);

FirstTargLatADT.SetDefaults(NULL, 0, EffectDefault[3]); // lat
FirstTargLatADT.SetRangeDefaults(RangeDefaults[3]);
MidTargLatADT.SetDefaults(NULL, 0, EffectDefault[3]);
MidTargLatADT.SetRangeDefaults(RangeDefaults[3]);
EndTargLatADT.SetDefaults(NULL, 0, EffectDefault[3]);
EndTargLatADT.SetRangeDefaults(RangeDefaults[3]);
FirstTargLonADT.SetDefaults(NULL, 0, EffectDefault[4]); // lon
FirstTargLonADT.SetRangeDefaults(RangeDefaults[4]);
MidTargLonADT.SetDefaults(NULL, 0, EffectDefault[4]);
MidTargLonADT.SetRangeDefaults(RangeDefaults[4]);
EndTargLonADT.SetDefaults(NULL, 0, EffectDefault[4]);
EndTargLonADT.SetRangeDefaults(RangeDefaults[4]);
FirstTargElevADT.SetDefaults(NULL, 0, EffectDefault[5]); // elev
FirstTargElevADT.SetRangeDefaults(RangeDefaults[5]);
MidTargElevADT.SetDefaults(NULL, 0, EffectDefault[5]);
MidTargElevADT.SetRangeDefaults(RangeDefaults[5]);
EndTargElevADT.SetDefaults(NULL, 0, EffectDefault[5]);
EndTargElevADT.SetRangeDefaults(RangeDefaults[5]);

FirstFOV.SetDefaults(NULL, 0, EffectDefault[6]); // fov
FirstFOV.SetRangeDefaults(RangeDefaults[6]);
MidFOV.SetDefaults(NULL, 0, EffectDefault[6]);
MidFOV.SetRangeDefaults(RangeDefaults[6]);
EndFOV.SetDefaults(NULL, 0, EffectDefault[6]);
EndFOV.SetRangeDefaults(RangeDefaults[6]);

if (GlobalApp && GlobalApp->MainProj && GlobalApp->MainProj->Interactive)
	{
	if ((FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate()) <= 0.0)
		FrameRate = 30.0;
	} // if
else
	FrameRate = 30.0;

RangeDefaults[7][2] = 1.0 / FrameRate;
MidTime.SetDefaults(NULL, 0, EffectDefault[7]);
MidTime.SetRangeDefaults(RangeDefaults[7]);
EndTime.SetDefaults(NULL, 0, EffectDefault[8]);
EndTime.SetRangeDefaults(RangeDefaults[7]);

FirstCamLatADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
MidCamLatADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
EndCamLatADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
FirstCamLonADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
MidCamLonADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
EndCamLonADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
FirstCamElevADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
MidCamElevADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
EndCamElevADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
FirstTargLatADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
MidTargLatADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
EndTargLatADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
FirstTargLonADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
MidTargLonADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
EndTargLonADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
FirstTargElevADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
MidTargElevADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
EndTargElevADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
FirstFOV.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
MidFOV.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
EndFOV.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);

MidTime.SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME); 
EndTime.SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
MidTime.SetNoNodes(1);
EndTime.SetNoNodes(1);

// set the auto-import file to the correct shape file
GlobalApp->MainProj->SetAuxAutoImportFile(ShapeFilePath, ShapeFileName);
if (GlobalApp->MainProj->Load(NULL, TotalTemplateName, GlobalApp->AppDB, GlobalApp->AppEffects, GlobalApp->AppImages, NULL, 0xffffffff))
	{
	// create new project
	if (NewProj = new NewProject(GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->AppEffects, GlobalApp->AppImages))
		{
		NewProj->ProjName.SetPath("WCSProjects:Projects");
		NewProj->ProjName.SetName(NewProjectName);
		NewProj->Clone = 1;
		NewProj->CloneType = 0;
		NewProj->PlaceInSubDir = 1;
		if (! NewProj->Create())
			{
			UserMessageOK("EDSS", "Error creating new project.");
			InitError = 1;
			} // else
		delete NewProj;
		} // if
	else
		{
		UserMessageOK("EDSS", "Error creating new project.");
		InitError = 1;
		} // else
	} // if
else
	{
	UserMessageOK("EDSS", "Error loading template project.");
	InitError = 1;
	} // else
GlobalApp->MainProj->SetAuxAutoImportFile("", "");

} // EDSSControl::EDSSControl


EDSSControl::~EDSSControl()
{


} // EDSSControl::~EDSSControl


void EDSSControl::InstantiateGUI(void)
{

GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
	WCS_TOOLBAR_ITEM_DSS, 0);

} // EDSSControl::InstantiateGUI
