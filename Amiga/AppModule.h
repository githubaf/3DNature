// AppModule.h
// Root class definition for input-capable sections of the program
// Created from scratch on 24 May 1995 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_APPMODULE_H
#define WCS_APPMODULE_H

class AppEvent;
class WCSApp;
class WCSModule;

class WCSModule
	{
	public:
		virtual long HandleEvent(AppEvent *Activity, WCSApp *AppScope) = 0;
	}; // WCSModule

#endif // WCS_APPMODULE_H
