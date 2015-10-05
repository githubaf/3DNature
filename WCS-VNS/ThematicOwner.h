// ThematicOwner.h
// Built from scratch 4/1/01 by Gary R. Huber
// Copyright 2001 by 3D Nature LLC, All rights reserved.

#ifndef WCS_THEMATICOWNER_H
#define WCS_THEMATICOWNER_H

class EffectList;

class ThematicOwner
	{
	public:
		void Copy(ThematicOwner *CopyTo, ThematicOwner *CopyFrom);
		int BuildFileComponentsList(EffectList **Themes);
		virtual char *GetThemeName(long ThemeNum) = 0;
		virtual long GetNumThemes(void) = 0;
		virtual ThematicMap *GetTheme(long ThemeNum) = 0;
		virtual ThematicMap *GetEnabledTheme(long ThemeNum) = 0;
		virtual ThematicMap **GetThemeAddr(long ThemeNum) = 0;
		virtual int SetTheme(long ThemeNum, ThematicMap *NewTheme) = 0;
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme) = 0;
		long GetThemeNumberFromAddr(ThematicMap **Addr);
		int GetThemeUnique(long ThemeNum);

	}; // class ThematicOwner

#endif // WCS_THEMATICOWNER_H
