// Log.h
// Log log log log LOG log log...
// Created from scratch on 9/13/95 by Chris "Xenon" Hanson
// Copyright 1995

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_LOG_H
#define WCS_LOG_H

#include "Application.h"
#include "AppModule.h"
#include "Fenetre.h"

// Standard Messages
// Make sure these are in the same order as their severity level
// and error text in the Log.cpp file.
enum {
	WCS_LOG_ERR_MEM_FAIL,
	WCS_LOG_ERR_OPEN_FAIL,
	WCS_LOG_ERR_READ_FAIL,
	WCS_LOG_ERR_WRITE_FAIL,
	WCS_LOG_ERR_WRONG_TYPE,
	WCS_LOG_ERR_ILL_INST,
	WCS_LOG_ERR_ILL_VAL,
	WCS_LOG_ERR_NO_LOAD,
	WCS_LOG_WNG_OPEN_FAIL,
	WCS_LOG_WNG_READ_FAIL,
	WCS_LOG_WNG_WRONG_TYPE,
	WCS_LOG_WNG_ILL_INSTR,
	WCS_LOG_WNG_ILL_VAL,
	WCS_LOG_MSG_NO_MOD,
	WCS_LOG_MSG_NO_GUI,
	WCS_LOG_MSG_PAR_LOAD,
	WCS_LOG_MSG_PAR_SAVE,
	WCS_LOG_MSG_DBS_LOAD,
	WCS_LOG_MSG_DBS_SAVE,
	WCS_LOG_MSG_DEM_LOAD,
	WCS_LOG_MSG_DEM_SAVE,
	WCS_LOG_MSG_VEC_LOAD,
	WCS_LOG_MSG_VEC_SAVE,
	WCS_LOG_MSG_IMG_LOAD,
	WCS_LOG_MSG_IMG_SAVE,
	WCS_LOG_MSG_CMP_LOAD,
	WCS_LOG_MSG_CMP_SAVE,
	WCS_LOG_MSG_NO_LOAD,
	WCS_LOG_ERR_SDTSDEM_XFER,
	WCS_LOG_ERR_SDTSDEM_DDR,
	WCS_LOG_ERR_SDTSDEM_RSUB,
	WCS_LOG_ERR_SDTSDEM_CSUB,
	WCS_LOG_ERR_SDTSDEM_MANGLED,
	WCS_LOG_MSG_UTIL_TAB,
	WCS_LOG_MSG_MPG_MOD,
	WCS_LOG_ERR_DIR_FAIL,
	WCS_LOG_ERR_WIN_FAIL,
	WCS_LOG_MSG_NULL,
	WCS_LOG_DTA_NULL,
	WCS_LOG_ERR_WRONG_SIZE,
	WCS_LOG_WNG_WIN_FAIL,
	WCS_LOG_WNG_WRONG_SIZE,
	WCS_LOG_WNG_WRONG_VER,
	WCS_LOG_MSG_RELEL_SAVE,
	WCS_LOG_WNG_NULL,
	WCS_LOG_MSG_VCS_LOAD,
	WCS_LOG_MSG_PROJ_LOAD,
	WCS_LOG_MSG_PROJ_SAVE,
	WCS_LOG_MSG_DIRLST_LOAD	,
	WCS_LOG_ERR_WRONG_VER,
	WCS_LOG_MSG_TIME_ELAPSE	,
	WCS_LOG_MSG_TOTAL_ELAPS	,
	WCS_LOG_MSG_START_SCRIPT,
	WCS_LOG_ERR_SCRIPT_FAIL,
	WCS_LOG_MSG_COMPONENT_SAVE,
	WCS_LOG_MSG_COMPONENT_LOAD,
	WCS_LOG_ERR_NULL
	}; // Log Errors

enum {
	WCS_LOG_SEVERITY_NONE,
	WCS_LOG_SEVERITY_DTA = 50,
	WCS_LOG_SEVERITY_MSG = 100,
	WCS_LOG_SEVERITY_WNG = 128,
	WCS_LOG_SEVERITY_ERR = 200
	}; // Log Severity

#define WCS_LOG_LINE_WIDTH 200

struct ErrorEntry
	{
	unsigned int Number;
	unsigned char Severity;
	char *Text;
	}; // ErrorEntry

class MessageLog : public WCSModule, public GUIFenetre
	{
	private:
		char LogFileName[255], LogFileEnable;
		char TempBuffer[WCS_LOG_LINE_WIDTH];
		char *LineArray;
		unsigned char OpenSeverity;
		unsigned short LogSize, LoggedLines, LogTop;

		void OpenLogWin(void);

	public:
		void ClearListObj(void);
		void AddToListObj(const char *Text);
		void RemoveOldest(void);
		NativeGUIWin Construct(void);

		MessageLog(unsigned short Entries = 100);
		~MessageLog();
		NativeGUIWin Open(Project *Moi);
		void OpenLogFile(char *LogName);
		void DumpToLogFile(void);
		void CloseLogFile(void);
		const char *FetchLogName(void);
		void ForceLogOpen(void);
		char IsLogOpen(void) {return((char)!IsThisYou(NULL));};
		void PostError(unsigned char Magnitude, const char *ErrorMsg);
		void PostStockError(unsigned int StockNum, const char *ErrorMsg);
		void ClearLog(void);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleReSized(int ReSizeType, long NewWidth, long NewHeight);
	}; // MessageLog

#endif // WCS_LOG_H
