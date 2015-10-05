// ProjUpdateGUI.h
// Header file for ProjUpdateGUI window
// built from VersionGUI.h on 3/11/08 by CXH

#ifndef WCS_PROJUPDATEGUI_H
#define WCS_PROJUPDATEGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "WCSWidgets.h"
#include "AppHelp.h"
#include "Raster.h"
#include "Effectslib.h"

// Very important that we list the base classes in this order.
class ProjUpdateGUI : public WCSModule, public GUIFenetre
	{
	private:
		bool _WarnAboutV5Displacement, _WarnAboutV6Displacement, _WarnAboutV6FractalDepth,
			_WarnAboutDensByPolygon;
		HFONT TipTextFont;

	public:

		ProjUpdateGUI();
		~ProjUpdateGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		void ConfigureWidgets(void);
		bool SetTerrainMessagesVisible(bool WarnAboutV5Displacement, bool WarnAboutV6Displacement, bool WarnAboutV6FractalDepth);
		bool SetEcotypeMessagesVisible(bool WarnAboutDensByPolygon);
		void ShowIfNeeded(void);

	}; // class ProjUpdateGUI

#endif // WCS_PROJUPDATEGUI_H
