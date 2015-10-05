// Log.cpp
// Error logging object
// Created from scratch on 9/13/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"
#include "Application.h"
#include "Project.h"
#include "Log.h"
#include "Toolbar.h"
#include "resource.h"

extern WCSApp *GlobalApp;

struct ErrorEntry StockErrors[] =
	{
	{WCS_LOG_ERR_MEM_FAIL, WCS_LOG_SEVERITY_ERR, " Out of memory! "},
	{WCS_LOG_ERR_OPEN_FAIL, WCS_LOG_SEVERITY_ERR, " Open file failed! "},
	{WCS_LOG_ERR_READ_FAIL, WCS_LOG_SEVERITY_ERR, " Read file failed! "},
	{WCS_LOG_ERR_WRITE_FAIL, WCS_LOG_SEVERITY_ERR, " Writing to file failed! "},
	{WCS_LOG_ERR_WRONG_TYPE, WCS_LOG_SEVERITY_ERR, " Wrong file type! "},
	{WCS_LOG_ERR_ILL_INST, WCS_LOG_SEVERITY_ERR, " Illegal instruction! "},
	{WCS_LOG_ERR_ILL_VAL, WCS_LOG_SEVERITY_ERR, " Illegal value! "},
	{WCS_LOG_ERR_NO_LOAD, WCS_LOG_SEVERITY_ERR, " File not loaded! "},
	{WCS_LOG_WNG_OPEN_FAIL, WCS_LOG_SEVERITY_WNG, " Open file failed. "},
	{WCS_LOG_WNG_READ_FAIL, WCS_LOG_SEVERITY_WNG, " Read file failed. "},
	{WCS_LOG_WNG_WRONG_TYPE, WCS_LOG_SEVERITY_WNG, " Wrong file type. "},
	{WCS_LOG_WNG_ILL_INSTR, WCS_LOG_SEVERITY_WNG, " Illegal instruction. "},
	{WCS_LOG_WNG_ILL_VAL, WCS_LOG_SEVERITY_WNG, " Illegal value. "},
	{WCS_LOG_MSG_NO_MOD, WCS_LOG_SEVERITY_MSG, " Module not implemented. "},
	{WCS_LOG_MSG_NO_GUI, WCS_LOG_SEVERITY_MSG, " GUI not implemented. "},
	{WCS_LOG_MSG_PAR_LOAD, WCS_LOG_SEVERITY_MSG, " Parameter file loaded. "},
	{WCS_LOG_MSG_PAR_SAVE, WCS_LOG_SEVERITY_MSG, " Parameter file saved. "},
	{WCS_LOG_MSG_DBS_LOAD, WCS_LOG_SEVERITY_MSG, " Database file loaded. "},
	{WCS_LOG_MSG_DBS_SAVE, WCS_LOG_SEVERITY_MSG, " Database file saved. "},
	{WCS_LOG_MSG_DEM_LOAD, WCS_LOG_SEVERITY_MSG, " DEM file loaded. "},
	{WCS_LOG_MSG_DEM_SAVE, WCS_LOG_SEVERITY_MSG, " DEM file saved. "},
	{WCS_LOG_MSG_VEC_LOAD, WCS_LOG_SEVERITY_MSG, " Vector file loaded. "},
	{WCS_LOG_MSG_VEC_SAVE, WCS_LOG_SEVERITY_MSG, " Vector file saved. "},
	{WCS_LOG_MSG_IMG_LOAD, WCS_LOG_SEVERITY_MSG, " Image file loaded. "},
	{WCS_LOG_MSG_IMG_SAVE, WCS_LOG_SEVERITY_MSG, " Image file saved. "},
	{WCS_LOG_MSG_CMP_LOAD, WCS_LOG_SEVERITY_MSG, " Color Map file loaded. "},
	{WCS_LOG_MSG_CMP_SAVE, WCS_LOG_SEVERITY_MSG, " Color Map file saved. "},
	{WCS_LOG_MSG_NO_LOAD, WCS_LOG_SEVERITY_MSG, " File not loaded. "},
	{WCS_LOG_ERR_SDTSDEM_XFER, WCS_LOG_SEVERITY_ERR, " Error opening SDTS DEM transfer file: "},
	{WCS_LOG_ERR_SDTSDEM_DDR, WCS_LOG_SEVERITY_ERR, " Error reading DDR. "},
	{WCS_LOG_ERR_SDTSDEM_RSUB, WCS_LOG_SEVERITY_ERR, " Error reading data record subfield. "},
	{WCS_LOG_ERR_SDTSDEM_CSUB, WCS_LOG_SEVERITY_ERR, " Error checking data record subfield. "},
	{WCS_LOG_ERR_SDTSDEM_MANGLED, WCS_LOG_SEVERITY_ERR, " Mangled elevation file. "},
#ifdef _WIN32
	{WCS_LOG_MSG_UTIL_TAB, WCS_LOG_SEVERITY_MSG, "     "},
#endif // _WIN32
	{WCS_LOG_MSG_MPG_MOD, WCS_LOG_SEVERITY_MSG, " Mapping Module. "},
	{WCS_LOG_ERR_DIR_FAIL, WCS_LOG_SEVERITY_ERR, " Directory not found! "},
	{WCS_LOG_ERR_WIN_FAIL, WCS_LOG_SEVERITY_ERR, " Open window failed! "},
	{WCS_LOG_MSG_NULL, 0, ""},
	{WCS_LOG_DTA_NULL, 0, ""},
	{WCS_LOG_ERR_WRONG_SIZE, WCS_LOG_SEVERITY_ERR, " Incorrect file size! "},
	{WCS_LOG_WNG_WIN_FAIL, WCS_LOG_SEVERITY_WNG, " Open window failed. "},
	{WCS_LOG_WNG_WRONG_SIZE, WCS_LOG_SEVERITY_WNG, " Incorrect file size! "},
	{WCS_LOG_WNG_WRONG_VER, WCS_LOG_SEVERITY_WNG, " Incorrect file version! "},
	{WCS_LOG_MSG_RELEL_SAVE, WCS_LOG_SEVERITY_MSG, " Relative Elevation File Saved. "},
	{WCS_LOG_WNG_NULL, 0, ""},
	{WCS_LOG_MSG_VCS_LOAD, WCS_LOG_SEVERITY_MSG, " Vector objects loaded. "},
	{WCS_LOG_MSG_PROJ_LOAD, WCS_LOG_SEVERITY_MSG, " Project file loaded. "},
	{WCS_LOG_MSG_PROJ_SAVE, WCS_LOG_SEVERITY_MSG, " Project file saved. "},
	{WCS_LOG_MSG_DIRLST_LOAD	, WCS_LOG_SEVERITY_MSG, " Directory List loaded. "},
	{WCS_LOG_ERR_WRONG_VER, WCS_LOG_SEVERITY_ERR, " Incorrect file version! "},
	{WCS_LOG_MSG_TIME_ELAPSE, WCS_LOG_SEVERITY_MSG, " Render time for frame "},
	{WCS_LOG_MSG_TOTAL_ELAPS, WCS_LOG_SEVERITY_MSG, " Render time for anim, "},
	{WCS_LOG_MSG_START_SCRIPT, WCS_LOG_SEVERITY_MSG, " Executing Script: "},
	{WCS_LOG_ERR_SCRIPT_FAIL, WCS_LOG_SEVERITY_ERR, " Script Failed: "},
	{WCS_LOG_MSG_COMPONENT_SAVE, WCS_LOG_SEVERITY_MSG, " Component file saved: "},
	{WCS_LOG_MSG_COMPONENT_LOAD, WCS_LOG_SEVERITY_MSG, " Component file loaded: "},
	{WCS_LOG_ERR_NULL, 0, ""}
	}; // StockErrors

/*===========================================================================*/

MessageLog::MessageLog(unsigned short Entries)
: GUIFenetre('STLG', this, "Status Log")
{
LogFileName[0] = NULL;
OpenSeverity = WCS_LOG_SEVERITY_ERR; // auto-open on serious errors
LogSize = Entries;
LoggedLines = LogTop = 0;
LogFileEnable = 0;

// We error-check this pointer as we use it, no need to fail if it
// is NULL
LineArray = new char [Entries * WCS_LOG_LINE_WIDTH];
SetWinManFlags(WCS_FENETRE_WINMAN_NOPOPUP | WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOWINLIST | WCS_FENETRE_WINMAN_SMTITLE);
} // MessageLog::MessageLog

/*===========================================================================*/

NativeGUIWin MessageLog::Open(Project *Moi)
{
NativeGUIWin Success;
RECT LOGWIN;
int LeftEdge, BotEdge, TopEdge, Width, Height;

if (Success = GUIFenetre::Open(Moi))
	{
	GetWindowRect(NativeWin, &LOGWIN);
	Height   = LOGWIN.bottom - LOGWIN.top;
	LeftEdge = GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_LOGSTATUS_LEFTEDGE) - 3;
	BotEdge  = GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN) + GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
	TopEdge  = BotEdge - Height;
	Width    = LOGWIN.right - LOGWIN.left;
	Width    = GlobalApp->WinSys->InquireRootWidth() - LeftEdge;
	MoveAndSizeFen(LeftEdge, TopEdge, Width, Height);
	HandleReSized(SIZE_RESTORED, Width, Height);
	} // if

return (Success);

} // MessageLog::Open

/*===========================================================================*/

long MessageLog::HandleReSized(int ReSizeType, long NewWidth, long NewHeight)
{
RECT TempRect;
POINT TransUL;
NativeControl ListControl;
//int LBorder, TopBorder;

if (ListControl = GetWidgetFromID(ID_LOG_TEXT))
	{
	GetWindowRect(ListControl, &TempRect);

	TransUL.x = TempRect.left;
	TransUL.y = TempRect.top;
	ScreenToClient(NativeWin, &TransUL);
	GetClientRect(NativeWin, &TempRect);

	SetWindowPos(ListControl, NULL, 0, 0, TempRect.right - (TransUL.x + 3), TempRect.bottom - (TransUL.y + 3), SWP_NOMOVE | SWP_NOZORDER);
	} // if

return(0);
} // MessageLog::HandleReSized

/*===========================================================================*/

MessageLog::~MessageLog()
{

if (GlobalApp->MCP)
	{
	GlobalApp->MCP->WidgetSetCheck(IDI_STATUSLOG, 0);
	} // if

Close();
Destruct();

if (LineArray)
	{
	delete [] LineArray;
	LineArray = NULL;
	} // if

} // MessageLog::~MessageLog

/*===========================================================================*/

void MessageLog::OpenLogWin(void)
{
int AddLoop, LoopStop;

if (!NativeWin)
	{
	Open(GlobalApp->MainProj);
	if (GlobalApp->MCP)
		{
		GlobalApp->MCP->WidgetSetCheck(IDI_STATUSLOG, 1);
		} // if
	// add items to List object
	if (LoggedLines)
		{
		AddLoop = LogTop - LoggedLines;
		if (AddLoop < 0)
			{
			AddLoop += LogSize;
			LoopStop = LogSize - 1;
			for(; AddLoop < LoopStop; AddLoop++)
				{
				AddToListObj(&LineArray[AddLoop * WCS_LOG_LINE_WIDTH]);
				} // for
			AddLoop = 0;
			} // if
		LoopStop = LogTop;
		for(; AddLoop < LoopStop; AddLoop++)
			{
			AddToListObj(&LineArray[AddLoop * WCS_LOG_LINE_WIDTH]);
			} // for
		} // if
	} // if
} // MessageLog::OpenLogWin

/*===========================================================================*/

void MessageLog::OpenLogFile(char *LogName)
{
char *TimeStr /*, *FileBase */;
time_t Now;

strncpy(LogFileName, LogName, 250);
LogFileName[254] = NULL;
LogFileEnable = 1;

/*
for(FileBase = LogName; *FileBase != NULL; FileBase++);

while(!(FileBase < LogName))
	{
	if ((*FileBase == '\\') || (*FileBase == '/'))
		{
		break;
		} // if
	FileBase--;
	} // while
*/
time(&Now);
TimeStr = ctime(&Now);
TimeStr[24] = NULL;
sprintf(TempBuffer, "Log File \"%s\" opened on %s.", LogName, TimeStr);
PostError(0, TempBuffer);

} // MessageLog::OpenLogFile

/*===========================================================================*/

void MessageLog::DumpToLogFile(void)
{
FILE *LogOut;
int LoopStop, RollSize, AddLoop;

if (!LogFileEnable)
	return;

if (LogOut = PROJ_fopen(LogFileName, "a"))
	{
	if (LoggedLines)
		{
		LoopStop = LogTop + LoggedLines;
		RollSize = 0;
		if ((LogTop + LoggedLines) > LogSize)
			{
			LoopStop = LogSize;
			RollSize = (LogTop + LoggedLines) - LogSize;
			} // if
		for(AddLoop = LogTop; AddLoop < LoopStop; AddLoop++)
			{
			fputs(&LineArray[AddLoop * WCS_LOG_LINE_WIDTH], LogOut);
			} // for
		if (RollSize)
			{
			for(AddLoop = 0; AddLoop < RollSize; AddLoop++)
				{
				fputs(&LineArray[AddLoop * WCS_LOG_LINE_WIDTH], LogOut);
				} // for
			} // if
		} // if
	fclose(LogOut);
	LogOut = NULL;
	} // if

} // MessageLog::DumpToLogFile

/*===========================================================================*/

void MessageLog::CloseLogFile(void)
{

LogFileEnable = 0;
PostError(0, "Log File Closed.");

} // MessageLog::CloseLogFile

/*===========================================================================*/

const char *MessageLog::FetchLogName(void)
{

return(LogFileName);

} // MessageLog::FetchLogName

/*===========================================================================*/

void MessageLog::ForceLogOpen(void)
{

if (LoggedLines)
	{
	OpenLogWin();
	} // if
else
	{
	PostError(255, "Log Window Opened.");
	} // else

} // MessageLog::ForceLogOpen

/*===========================================================================*/

void MessageLog::PostError(unsigned char Magnitude, const char *ErrorMsg)
{
FILE *LogOut;
char *ErrorToList;
int FilterScan;

ErrorToList = (char *)ErrorMsg;

// Add to RootWin
if (GlobalApp->MCP)
	{
	GlobalApp->MCP->SetCurrentStatusText((char *)ErrorMsg);
	} // if

// Do the real stuff
if ((LogTop + 1) == LogSize)
	{
	LogTop = 1;
	strncpy(&LineArray[0], ErrorMsg, WCS_LOG_LINE_WIDTH);
	ErrorToList = &LineArray[0];
	LineArray[WCS_LOG_LINE_WIDTH - 1] = NULL;
	} // if
else
	{
	strncpy(&LineArray[LogTop * WCS_LOG_LINE_WIDTH], ErrorMsg, WCS_LOG_LINE_WIDTH);
	ErrorToList = &LineArray[LogTop * WCS_LOG_LINE_WIDTH];
	LineArray[((WCS_LOG_LINE_WIDTH * (LogTop + 1)) - 1)] = NULL;
	LogTop++;
	if (LoggedLines < LogSize)
		{
		LoggedLines++;
		} // if
	} // else

// Note: Go ahead and add the entry to the List object.
// If the log window is open but hidden, or even if it isn't opened,
// AddToListObj will cope.

if (LoggedLines == LogSize)
	{
	RemoveOldest();
	} // if

// Try to filter out bad characters.
for(FilterScan = 0; ErrorToList[FilterScan]; FilterScan++)
	{
	if (!isprint(ErrorToList[FilterScan]))
		{
		ErrorToList[FilterScan] = ' ';
		} // if
	} // for

if (!NativeWin)
	{
	if (Magnitude >= OpenSeverity)
		{
		OpenLogWin();
		} // if
	} // if
else
	{
	AddToListObj(ErrorToList);
	} // else

// Now log it to the logfile if needed.
if (LogFileEnable && LogFileName[0])
	{
	if (LogOut = PROJ_fopen(LogFileName, "a"))
		{
		fputs(ErrorMsg, LogOut);
		fputc('\n', LogOut);
		fclose(LogOut);
		} // if
	} // if

} // MessageLog::PostError

/*===========================================================================*/

void MessageLog::PostStockError(unsigned int StockNum, const char *ErrorMsg)
{

if (StockNum <= WCS_LOG_ERR_NULL)
	{
	TempBuffer[0] = NULL;
	if (StockErrors[StockNum].Severity >= WCS_LOG_SEVERITY_ERR)
		{
		strcpy(TempBuffer, "ERR:"); 
		} // if
	else if (StockErrors[StockNum].Severity >= WCS_LOG_SEVERITY_WNG)
		{
		strcpy(TempBuffer, "WNG:"); 
		} // if
	else if (StockErrors[StockNum].Severity >= WCS_LOG_SEVERITY_MSG)
		{
		strcpy(TempBuffer, "MSG:"); 
		} // if
	else if (StockErrors[StockNum].Severity >= WCS_LOG_SEVERITY_DTA)
		{
		strcpy(TempBuffer, "DTA:"); 
		} // if
	strcat(TempBuffer, StockErrors[StockNum].Text);
	strncat(TempBuffer, ErrorMsg, WCS_LOG_LINE_WIDTH - strlen(TempBuffer) - 5);
	TempBuffer[WCS_LOG_LINE_WIDTH - 1] = NULL;
	PostError(StockErrors[StockNum].Severity, TempBuffer);
	} // if
else
	{
	PostError(WCS_LOG_SEVERITY_ERR, "Unknown Error.");
	} // else

} // MessageLog::PostStockError

/*===========================================================================*/

void MessageLog::ClearLog(void)
{
unsigned int WipeLoop;

for(WipeLoop = 0; WipeLoop < LogSize; WipeLoop++)
	{
	LineArray[WCS_LOG_LINE_WIDTH * WipeLoop] = NULL;
	} // for
LogTop = LoggedLines = 0;

// Clear the List object as well
ClearListObj();

} // MessageLog::ClearLog

/*===========================================================================*/

long MessageLog::HandleCloseWin(NativeGUIWin NW)
{

if (GlobalApp->MCP)
	{
	GlobalApp->MCP->WidgetSetCheck(IDI_STATUSLOG, 0);
	} // if
OpenSeverity = WCS_LOG_SEVERITY_ERR;
Close();
return(0);

} // MessageLog::HandleCloseWin

/*===========================================================================*/

long MessageLog::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case ID_LOG_CLEAR:
		{
		ClearLog();
		return(1);
		} // CLEAR
	} // ButtonID
return(0);

} // MessageLog::HandleButtonClick

/*===========================================================================*/

void MessageLog::ClearListObj(void)
{

if (NativeWin)
	{
	WidgetLBClear(ID_LOG_TEXT);
	} // if

} // MessageLog::ClearListObj

/*===========================================================================*/

void MessageLog::AddToListObj(const char *Text)
{
long NumItems;

if (NativeWin)
	{
	WidgetLBInsert(ID_LOG_TEXT, -1, Text);
	NumItems = WidgetLBGetCount(ID_LOG_TEXT);
	WidgetLBSetCurSel(ID_LOG_TEXT, NumItems - 1);
	} // if

} // MessageLog::AddToListObj

/*===========================================================================*/

void MessageLog::RemoveOldest(void)
{

if (NativeWin)
	{
	WidgetLBDelete(ID_LOG_TEXT, 0);
	} // if

} // MessageLog::RemoveOldest

/*===========================================================================*/

NativeGUIWin MessageLog::Construct(void)
{

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_STATUSLOG, LocalWinSys()->RootWin);

	if (NativeWin)
		{
		ConfigureTB(NativeWin, ID_LOG_CLEAR, IDI_CLEARLOG, NULL);
		WidgetLBSetHorizExt(ID_LOG_TEXT, 1024);
		} // if

	} // if
 
return(NativeWin);

} // MessageLog::Construct
