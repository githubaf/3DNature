// CommonComponentEditor.h
// Common code for Component Editors
// Created from existing code by CXH 11/16/07

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_COMMONCOMPONENTEDITOR_H
#define WCS_COMMONCOMPONENTEDITOR_H

#include "Fenetre.h"

class EffectsLib;
class GeneralEffect;
class Database;
class Fenetre;

class CommonComponentEditor
	{
	private:
		GeneralEffect **StoredActiveComponent;
		Fenetre *StoredFenetre;
	public:
		// we're storing a pointer to the pointer in the Editor, so it's always live and fresh
		CommonComponentEditor(GeneralEffect **ActiveComponent, Fenetre *MyWin) {StoredActiveComponent = ActiveComponent; StoredFenetre = MyWin;};
		// if derived from CommonComponentEditor, a GUIFenetre EditGUI can implement this this like:
		// bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout) {return(RespondToInquireWindowCapabilities(AskAbout));};
		// I don't know of an easier way to do it.
		bool RespondToInquireWindowCapabilities(FenetreWindowCapabilities AskAbout);
		long HandleCommonEvent(int EventCode, EffectsLib *Lib, GeneralEffect *Active, Database *DBHost = NULL); // return(1) if handled. DBHost may be NULL for some classes
		virtual bool RespondToCanShowAdvancedFeatures(void) {return(false);};
	}; // CommonComponentEditor

#endif // WCS_COMMONCOMPONENTEDITOR_H
