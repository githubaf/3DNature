// EDSSControl.h
// Header file for Ecosystem Decision Support System control class
// Created from scratch 10/15/01 by Gary R. Huber
// Copyright 2001 3D Nature. All rights reserved.

#ifndef WCS_EDSSCONTROL_H
#define WCS_EDSSCONTROL_H

#include "GraphData.h"

class ArgRipper;

class EDSSControl
	{
	public:
		char InitError, NewProjectName[512], TemplateName[512], ShapeFileName[512], EndKeyEnabled, MidKeyEnabled;
		AnimDoubleTime FirstCamLatADT, MidCamLatADT, EndCamLatADT, FirstCamLonADT, MidCamLonADT, EndCamLonADT, FirstCamElevADT, MidCamElevADT, EndCamElevADT, 
			FirstTargLatADT, MidTargLatADT, EndTargLatADT, FirstTargLonADT, MidTargLonADT, EndTargLonADT, FirstTargElevADT, MidTargElevADT, EndTargElevADT, 
			FirstFOV, MidFOV, EndFOV, MidTime, EndTime;

		EDSSControl(ArgRipper *Argh, int NumArgsParsed);
		~EDSSControl();
		void InstantiateGUI(void);

	}; // class EDSSControl


#endif // WCS_EDSSCONTROL_H
