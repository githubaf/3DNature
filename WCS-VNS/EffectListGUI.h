// EffectListGUI.h
// Header file for Effect List GUI
// Built from GradProfListGUI.h on 8/16/99 by Gary R. Huber
// Copyright 1999 by Questar Productions. All rights reserved.

#ifndef WCS_EFFECTLISTGUI_H
#define WCS_EFFECTLISTGUI_H

//class EffectsLib;
class GeneralEffect;
class RasterAnimHost;
class ThematicOwner;

#include "Application.h"
#include "Fenetre.h"

// Very important that we list the base classes in this order.
class EffectListGUI : public WCSModule, public GUIFenetre
	{
	public:

		int ConstructError;
		GeneralEffect *HostList;
		RasterAnimHost *ActiveHost;
		ThematicOwner *ThemeOwner;
		EffectsLib *HostLib;
		long EffectType, CamToVec, Themes, ThemeNumber;

		EffectListGUI(EffectsLib *LibSource, RasterAnimHost *ActiveSource, long EffectTypeSource, ThematicOwner *ThemeOwnerSource, long ThemeNumberSource);
		~EffectListGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void BuildList(void);
		void Apply(void);

	}; // class EffectListGUI

#endif // WCS_EFFECTLISTGUI_H
