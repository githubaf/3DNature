// RenderControlGUI.h
// Header file for RenderControlGUI
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_RENDERCONTROLGUI_H
#define WCS_RENDERCONTROLGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "RenderInterface.h"

class EffectsLib;
class ImageLib;
class Database;
class Project;
class RenderJob;
class Renderer;

// Very important that we list the base classes in this order.
class RenderControlGUI : public RenderInterface, public WCSModule, public GUIFenetre
	{
	public:
		EffectsLib *EffectsHost;
		ImageLib *ImageHost;
		Database *DBHost;
		Project *ProjectHost;
		RenderJob *ActiveJob;

		Renderer *Rend;
		char AText[200], FText[200], PText[200], PauseText[200],
			IText[200], RText[200], FRText[200], ProgCap[200], PrevCap[200],
			FrameTitle[200],
			Preview, Pause, Run, FrdWarned, RegenFDMs;
		unsigned long PMaxSteps, PCurSteps;
		unsigned long FMaxSteps, FCurSteps;
		time_t FStartSeconds, AStartSeconds;
		unsigned long AMaxSteps, ACurSteps;

		char ECPWDate[40], ECToday[40], ECProjDate[40];
		char NetNotifyDone;

		int ConstructError, Rendering;

		RenderControlGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Database *DBSource, Project *ProjectSource);
		~RenderControlGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);

		void ConfigureJobWidgets(void);
		void BuildJobList(void);
		void BuildJobListEntry(char *ListName, RenderJob *Me);
		void DisableWidgets(void);
		void EnableJob(void);
		void DisableJob(void);
		void ChangeJobPriority(short Direction);
		void EditJob(void);
		void SetActiveJob(void);
		void PriorityLevel(void);
		void RenderGo(void);
		void RenderStop(void);
		char GetRegenFDMs(void);
		void SetRegenFDMs(char Regen);

		void SyncTexts(void);
		void UpdatePreviewState(void);
		void DoUpdate(unsigned long Step, unsigned long MaxSteps,
			time_t StartSeconds, unsigned long &CurSteps, int Rem, int Elap, int Comp, int GaugeID);
		int CheckAbort(void);
		void ECRepaintFuelGauge(unsigned long CurSteps, unsigned long MaxSteps, int GaugeID);
		Renderer *GetRenderHandle(void)	{return (Rend);};

		virtual void GUIGoModal(void) {GoModal();};
		virtual void GUIEndModal(void) {EndModal();};
		virtual void SetPreviewCheck(int State);
		virtual NativeGUIWin GetGUINativeWin(void)	{return (NativeWin);};
		virtual Renderer *GetRenderer(void)	{return (Rend);};
		virtual int GetPreview(void)	{return (Preview);};
		virtual int GetPause(void)	{return (Pause);};
		virtual int IsRunning(void) {return(Run);};
		virtual void SetRenderer(Renderer *NewRend)	{Rend = NewRend;};
		virtual void SetPreview(char PreviewOn);
		virtual void SetPause(char State)	{Pause = State;};
		virtual void SetRunning(char RunState)	{Run = RunState;};

		virtual void UpdateStamp(void);
		virtual void UpdateLastFrame(void);
		virtual void ClearAll(void);

		virtual void AnimInit(unsigned long Steps, char *Text);
		virtual void AnimUpdate(unsigned long NewSteps, char *Text = NULL);
		virtual void AnimClear(void);
		virtual void SetAnimText(char *Text);

		virtual void FrameTextInit(char *Text);
		virtual void FrameTextClear(void);
		virtual void SetFrameText(char *Text);
		virtual void SetFrameNum(char *FrameNum);
		virtual void GetFrameSetup(unsigned long &StashCurSteps, unsigned long &StashMaxSteps, time_t &StashStartSecs, char *StashText);
		virtual void RestoreFrameSetup(unsigned long StashCurSteps, unsigned long StashMaxSteps, time_t StashStartSecs, char *StashText);
		virtual void StashFrame(unsigned long FrameTime);

		virtual void FrameGaugeInit(unsigned long Steps);
		virtual void FrameGaugeUpdate(unsigned long NewSteps);
		virtual void FrameGaugeClear(void);

		virtual void ProcInit(unsigned long Steps, char *Text);
		virtual void ProcUpdate(unsigned long NewSteps, char *Text = NULL);
		virtual void ProcClear(void);
		virtual void SetProcText(char *Text);
		virtual void GetProcSetup(unsigned long &StashCurSteps, unsigned long &StashMaxSteps, char *StashText);
		virtual void RestoreProcSetup(unsigned long StashCurSteps, unsigned long StashMaxSteps, char *StashText);

		virtual void SetImageText(char *Text);
		virtual void SetResText(char *Text);
		virtual void SetFractText(char *Text);

	}; // class RenderControlGUI

#endif // WCS_RENDERCONTROLGUI_H
