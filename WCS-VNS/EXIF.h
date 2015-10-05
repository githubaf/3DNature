// EXIF.h
// Header for EXIF tag support
// Created by Frank Weed II, 01/28/2009
// Copyright 2009 by 3D Nature, LLC. All rights reserved.

#ifndef WCS_EXIF
#define WCS_EXIF

#include "Render.h"

// a class to call "ExifTool" by Paul Harvey
class EXIFtool
	{
	public:
		EXIFtool(void);
		~EXIFtool();
		bool CreateEXIF(Renderer *RHost);
		bool SaveEXIF(const char *completeSavePath);
	private:
		char shellParams[4096];
	}; // class EXIFtool

#endif // WCS_EXIF
