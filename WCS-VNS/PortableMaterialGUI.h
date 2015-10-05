// PortableMaterialGUI.h
// Header file for material controls of numerous component editors
// Created from PortableFoliageGUI.h on 4/17/08 CXH
// Copyright 2008 Questar Productions. All rights reserved.

#ifndef WCS_PORTABLEMATERIALGUI_H
#define WCS_PORTABLEMATERIALGUI_H

class EffectsLib;
class GUIFenetre;
class GradientCritter;
class AnimMaterialGradient;
class GeneralEffect;

class PortableMaterialGUINotifyFunctor
	{
	// A functor is an object that allows us to pass customizable state data to a callback function, without
	// knowing in advance what exactly the data will be. All we need to know is which methods to call which
	// are virtual so they can be overridden in a derived class without us knowing]. Derive a class from this, add any data
	// members your callback will need access to, and implement your callbacks.
	// Refer to http://en.wikipedia.org/wiki/Function_object for more details.
	
	public:
		virtual void HandleConfigureMaterial(void) = 0; // Pure virtual, must be overridden.
		virtual void HandleNewActiveGrad(GradientCritter *NewNode) = 0; // Pure virtual, must be overridden.
	}; // PortableMaterialGUINotifyFunctor

class PortableMaterialGUI
	{
	private:
		int Ordinal;
		EffectsLib *EffectsHost;
		GUIFenetre *FenetreHost;
		GeneralEffect *Active;
		GradientCritter *ActiveGrad;
		AnimGradient *Grad;
		int AnimParDriverSubscript, TexSubscript;
		int ConstructError;
		int Panel, SecondPanel;
		double DummyValue0, DummyValue100;
		PortableMaterialGUINotifyFunctor *NotifyFunctor;

		void BlendStyle(void);
		void AddMatNode(void);
		void RemoveMatNode(void);
		void MatName(short WidID);

	public:
		// you need to supply both Panel and SecondPanel because we use two windows (nested) to make the dropshadow work
		// If you have multiple (two supported now) PortableMaterialGUI PopDrops in one editor (Lake), setup one using
		// Ordinal=0, and the next as Ordinal=1 so identical control IDs won't overlap.
		PortableMaterialGUI(int InitOrdinal, GUIFenetre *FenetreSource, EffectsLib *EffectsSource, 
			GeneralEffect *ActiveSource, AnimGradient *GradSource, int AnimParDriverSubscriptSource = 0, int TexSubscriptSource = 0);
		~PortableMaterialGUI();
		void SetNotifyFunctor(PortableMaterialGUINotifyFunctor *NewNotifyFunctor) {NotifyFunctor = NewNotifyFunctor;};
		NativeGUIWin Construct(int Panel, int SecondPanel);
		
		// (re)config
		void Setup(int InitOrdinal, GUIFenetre *FenetreSource, EffectsLib *EffectsSource, 
			GeneralEffect *ActiveSource, AnimGradient *GradSource, int AnimParDriverSubscriptSource = 0, int TexSubscriptSource = 0);

		// this is all public so it can be forwarded from the hosting window's Fenetre class
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);

		void ConfigureWidgets(void);
		void ConfigureMaterial(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		
		int GetPanel(void) const {return(Panel);};
		int GetSecondPanel(void) const {return(SecondPanel);};
		bool QueryIsDisplayed(void);

	}; // class PortableMaterialGUI

#endif // WCS_PORTABLEMATERIALGUI_H
