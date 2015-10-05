// ImageViewGUI.h
// Header file for Material Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_IMAGEVIEWGUI_H
#define WCS_IMAGEVIEWGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"

class ImageLib;
class Raster;


// Very important that we list the base classes in this order.
class ImageViewGUI : public WCSModule, public GUIFenetre
	{
	private:
		ImageLib *ImageHost;
		Raster *Active;
		DrawingFenetre *ViewWin;

		short WinWidth, WinHeight, WinTop, WinLeft, W, H,
			InsideWidth, InsideHeight, SampleRate, MadeNewRaster, AlreadyDrawn, Sampling;

	public:

		int ConstructError, Closing, LoadAsRendered;

		ImageViewGUI(ImageLib *ImageSource, Raster *ActiveSource, int LoadLastRendered, int LoadAsRenderedSource);
		~ImageViewGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		virtual long HandleLeftButtonDown(long X, long Y, char Alt, char Control, char Shift);
		virtual long HandleLeftButtonUp(long X, long Y, char Alt, char Control, char Shift);
		virtual long HandleMouseMove(long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		Raster *GetActive(void) {return (Active);};

	private:
		Raster *MakeNewRaster(int LoadLastRendered);
		DrawingFenetre *ConstructView(void);
		void PaintImage(void);
		void SampleImage(long X, long Y, unsigned char EventType);
		
	}; // class ImageViewGUI

#endif // WCS_IMAGEVIEWGUI_H
