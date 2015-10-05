// ScriptAction.h
// Header file for ScriptAction.cpp
// Created from scratch on May 29 1997 by Chris 'Xenon' Hanson
// Copyright 1997

#ifndef WCS_SCRIPT_ACTION_H
#define WCS_SCRIPT_ACTION_H

#include "AppModule.h"

class DBFilterEvent;

class ScriptActor : public WCSModule
	{
	private:
		char ScriptActorMessageFmt[200];
		char ScriptActorArgBuf[256], ScriptActorNumericArgBuf[256];
		double NumericArgs[10];
		WCSApp *LocalApp;
		ScriptParser *ParseMod;
		DBFilterEvent *DBFilter, *CurFilter;

		long HandleProject(struct CmdContext *ArgScope);
		long HandleScript(struct CmdContext *ArgScope);
		long HandleRender(struct CmdContext *ArgScope);
		long HandleWindow(struct CmdContext *ArgScope);
		long HandleMapView(struct CmdContext *ArgScope);
		long HandleCamView(struct CmdContext *ArgScope);
		long HandleRendSet(struct CmdContext *ArgScope);
		long HandleParam(struct CmdContext *ArgScope);
		long HandleEffect(struct CmdContext *ArgScope);
		long HandleDataBase(struct CmdContext *ArgScope);
		long HandleImageLib(struct CmdContext *ArgScope);
		long HandleGISImport(struct CmdContext *ArgScope);
		long HandleQuit(struct CmdContext *ArgScope);

		Fenetre *FindWinFromArg(const char *WinArg);
		int GetNumericArgs(const char *NumericString);
		long IdentifyEffectType(int Token);
		long IdentifyEffectType(char *Test);
		int TestBoolean(char *TestStr);
		DBFilterEvent *AddFilter(void);
		void RemoveFilter(void);
		
	public:
		ScriptActor(ScriptParser *ScriptMod);
		~ScriptActor();
		long HandleEvent(void);
	}; // ScriptActor

#endif // WCS_SCRIPT_ACTION_H
