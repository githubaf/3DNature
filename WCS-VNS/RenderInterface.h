// RenderInterface.h
// Header file for RenderInterface
// Created from scratch on 4/23/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#ifndef WCS_RENDERINTERFACE_H
#define WCS_RENDERINTERFACE_H

#include "Fenetre.h"

class Renderer;

class RenderInterface
	{
	public:
		virtual void GUIGoModal(void) {return;};
		virtual void GUIEndModal(void) {return;};
		virtual void SetPreviewCheck(int State) {return;};
		virtual NativeGUIWin GetGUINativeWin(void)	{return (NULL);};
		virtual Renderer *GetRenderer(void)	{return (NULL);};
		virtual int IsRunning(void)	{return (0);};
		virtual int GetPreview(void)	{return (0);};
		virtual int GetPause(void)	{return (0);};
		virtual void SetRenderer(Renderer *NewRend)	{return;};
		virtual void SetPreview(char PreviewOn)	{return;};
		virtual void SetPause(char State)	{return;};
		virtual void SetRunning(char RunState)	{return;};

		virtual void UpdateStamp(void)	{return;};
		virtual void UpdateLastFrame(void)	{return;};
		virtual void ClearAll(void)	{return;};

		virtual void AnimInit(unsigned long Steps, char *Text)	{return;};
		virtual void AnimUpdate(unsigned long NewSteps, char *Text = NULL)	{return;};
		virtual void SetAnimText(char *Text)	{return;};
		virtual void AnimClear(void)	{return;};

		virtual void FrameTextInit(char *Text)	{return;};
		virtual void FrameTextClear(void)	{return;};
		virtual void SetFrameText(char *Text)	{return;};
		virtual void SetFrameNum(char *FrameNum)	{return;};
		virtual void GetFrameSetup(unsigned long &StashCurSteps, unsigned long &StashMaxSteps, time_t &StashStartSecs, char *StashText)	{return;};
		virtual void RestoreFrameSetup(unsigned long StashCurSteps, unsigned long StashMaxSteps, time_t StashStartSecs, char *StashText)	{return;};
		virtual void StashFrame(unsigned long FrameTime)	{return;};

		virtual void FrameGaugeInit(unsigned long Steps)	{return;};
		virtual void FrameGaugeUpdate(unsigned long NewSteps)	{return;};
		virtual void FrameGaugeClear(void)	{return;};

		virtual void ProcInit(unsigned long Steps, char *Text)	{return;};
		virtual void ProcUpdate(unsigned long NewSteps, char *Text = NULL)	{return;};
		virtual void ProcClear(void)	{return;};
		virtual void SetProcText(char *Text)	{return;};
		virtual void GetProcSetup(unsigned long &StashCurSteps, unsigned long &StashMaxSteps, char *StashText)	{return;};
		virtual void RestoreProcSetup(unsigned long StashCurSteps, unsigned long StashMaxSteps, char *StashText)	{return;};

		virtual void SetImageText(char *Text)	{return;};
		virtual void SetResText(char *Text)	{return;};
		virtual void SetFractText(char *Text)	{return;};

	}; // class RenderInterface

#endif // WCS_RENDERINTERFACE_H
