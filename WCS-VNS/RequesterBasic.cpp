// RequesterBasic.cpp
// Very basic GUI/requester code
// built from Requester.h on 060403 by Chris "Xenon" Hanson

#include "stdafx.h"
#include "RequesterBasic.h"

// Remember, link with user32.lib when using as a library

#ifndef BUILD_LIB // building as library
class GUIContext;
class WCSApp;
extern GUIContext *ReqLocalGUI;
extern WCSApp *GlobalApp;

#include "GUI.h"
#include "Application.h"
#include "Toolbar.h"
#endif // BUILD_LIB

// used even in library form
HWND PrevUserMessageWin;

void UserMessageOK(const char *Topic, const char *Message, unsigned long Type)
{
#ifdef _WIN32
HWND ParentWin = NULL;
PrevUserMessageWin = NULL;
#endif // _WIN32
#ifndef BUILD_LIB // building as library
if (ReqLocalGUI)
	{
	ReqLocalGUI->GoModal();
	#ifdef _WIN32
	ParentWin = ReqLocalGUI->RootWin;
	#endif // _WIN32
	} // if
#endif // BUILD_LIB
#ifdef _WIN32
PrevUserMessageWin = GetActiveWindow();
MessageBox(ParentWin, Message, Topic, MB_OK | MB_TASKMODAL | Type);
if (PrevUserMessageWin) SetActiveWindow(PrevUserMessageWin);
PrevUserMessageWin = NULL;
#endif // _WIN32
#ifndef BUILD_LIB // building as library
if (ReqLocalGUI) ReqLocalGUI->EndModal();
if (GlobalApp && GlobalApp->MCP)
	{
	GlobalApp->MCP->SetCurrentStatusText(Message);
	} // if
#endif // BUILD_LIB

} // UserMessageOK

/*===========================================================================*/

unsigned char UserMessageNOMEM(char *Topic, unsigned long ReqSize, unsigned long AvailSize)
{
#ifdef _WIN32
HWND ParentWin = NULL;
#endif // _WIN32
int ReplyStatus;
char StuffBuff[110];

// if you change this message, check the buffer size above...
#ifdef _WIN32
sprintf(StuffBuff, "Could not allocate\n%lu bytes of memory.", ReqSize);
#else // !_WIN32
sprintf(StuffBuff, "Could not allocate\n%lu bytes of memory.\nLargest block available\nis %lu bytes.",
 ReqSize, AvailSize);
#endif // !_WIN32

#ifndef BUILD_LIB // building as library
if (ReqLocalGUI)
	{
	ReqLocalGUI->GoModal();
	#ifdef _WIN32
	ParentWin = ReqLocalGUI->RootWin;
	#endif // _WIN32
	} // if
#endif // BUILD_LIB

#ifdef _WIN32
//ReplyStatus = MessageBox(ReqLocalGUI->RootWin, StuffBuff, Topic, MB_RETRYCANCEL | MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONEXCLAMATION);
PrevUserMessageWin = GetActiveWindow();
ReplyStatus = MessageBox(ParentWin, StuffBuff, Topic, MB_RETRYCANCEL | MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONEXCLAMATION);
if (PrevUserMessageWin) SetActiveWindow(PrevUserMessageWin);
PrevUserMessageWin = NULL;
#endif // _WIN32

#ifndef BUILD_LIB // building as library
if (ReqLocalGUI) ReqLocalGUI->EndModal();
#endif // BUILD_LIB

#ifdef _WIN32
if (ReplyStatus == IDRETRY)  return(1);
else /* (ReplyStatus == IDCANCEL) */ return(0);
#endif // _WIN32

} // UserMessageNOMEM

/*===========================================================================*/

unsigned char UserMessageOKCAN(char *Topic, char *Message, unsigned char DefButton, unsigned long Type)
{
#ifdef _WIN32
HWND ParentWin = NULL;
#endif // _WIN32
int ReplyStatus;

#ifndef BUILD_LIB // building as library
if (ReqLocalGUI)
	{
	ReqLocalGUI->GoModal();
	#ifdef _WIN32
	ParentWin = ReqLocalGUI->RootWin;
	#endif // _WIN32
	} // if
#endif // BUILD_LIB

#ifdef _WIN32
if (DefButton == 1) Type |= MB_DEFBUTTON2;
if (DefButton == 2) Type |= MB_DEFBUTTON3;

//ReplyStatus = MessageBox(ParentWin, Message, Topic, MB_OKCANCEL | MB_TASKMODAL | MB_SETFOREGROUND |  Type);
PrevUserMessageWin = GetActiveWindow();

ReplyStatus = MessageBox(ParentWin, Message, Topic, MB_OKCANCEL | MB_TASKMODAL | MB_SETFOREGROUND |  Type);

if (PrevUserMessageWin) SetActiveWindow(PrevUserMessageWin);
PrevUserMessageWin = NULL;
#endif // _WIN32
#ifndef BUILD_LIB // building as library
if (ReqLocalGUI) ReqLocalGUI->EndModal();
#endif // BUILD_LIB

#ifdef _WIN32
if (ReplyStatus == IDOK)     return(1);
else /* (ReplyStatus == IDCANCEL) */ return(0);
#endif // _WIN32

} // UserMessageOKCAN

/*===========================================================================*/

unsigned char UserMessageRETRYCAN(char *Topic, char *Message, unsigned char DefButton, unsigned long Type)
{
#ifdef _WIN32
HWND ParentWin = NULL;
#endif // _WIN32
int ReplyStatus;

#ifndef BUILD_LIB // building as library
if (ReqLocalGUI)
	{
	ReqLocalGUI->GoModal();
	#ifdef _WIN32
	ParentWin = ReqLocalGUI->RootWin;
	#endif // _WIN32
	} // if
#endif // BUILD_LIB
#ifdef _WIN32
if (DefButton == 1) Type |= MB_DEFBUTTON2;
if (DefButton == 2) Type |= MB_DEFBUTTON3;

//ReplyStatus = MessageBox(ParentWin, Message, Topic, MB_RETRYCANCEL | MB_TASKMODAL | MB_SETFOREGROUND |  Type);
PrevUserMessageWin = GetActiveWindow();

ReplyStatus = MessageBox(ParentWin, Message, Topic, MB_RETRYCANCEL | MB_TASKMODAL | MB_SETFOREGROUND |  Type);

if (PrevUserMessageWin) SetActiveWindow(PrevUserMessageWin);
PrevUserMessageWin = NULL;
#endif // _WIN32
#ifndef BUILD_LIB // building as library
if (ReqLocalGUI) ReqLocalGUI->EndModal();
#endif // BUILD_LIB 

#ifdef _WIN32
if (ReplyStatus == IDRETRY)  return(1);
else /* (ReplyStatus == IDCANCEL) */ return(0);
#endif // _WIN32

} // UserMessageRETRYCAN

/*===========================================================================*/

unsigned char UserMessageYN(char *Topic, char *Message, unsigned char DefButton, unsigned long Type)
{
#ifdef _WIN32
HWND ParentWin = NULL;
#endif // _WIN32
int ReplyStatus;

#ifndef BUILD_LIB // building as library
if (ReqLocalGUI)
	{
	ReqLocalGUI->GoModal();
	#ifdef _WIN32
	ParentWin = ReqLocalGUI->RootWin;
	#endif // _WIN32
	} // if
#endif // BUILD_LIB
#ifdef _WIN32
if (DefButton == 1) Type |= MB_DEFBUTTON2;
if (DefButton == 2) Type |= MB_DEFBUTTON3;

//ReplyStatus = MessageBox(ParentWin, Message, Topic, MB_YESNO | MB_TASKMODAL | MB_SETFOREGROUND |  Type);
PrevUserMessageWin = GetActiveWindow();

ReplyStatus = MessageBox(ParentWin, Message, Topic, MB_YESNO | MB_TASKMODAL | MB_SETFOREGROUND |  Type);

if (PrevUserMessageWin) SetActiveWindow(PrevUserMessageWin);
PrevUserMessageWin = NULL;
#endif // _WIN32
#ifndef BUILD_LIB // building as library
if (ReqLocalGUI) ReqLocalGUI->EndModal();
#endif // BUILD_LIB 

#ifdef _WIN32
if (ReplyStatus == IDYES) return(1);
else /* (ReplyStatus == IDNO) */ return(0);
#endif // _WIN32

} // UserMessageYN

/*===========================================================================*/

unsigned char UserMessageYNCAN(char *Topic, char *Message, unsigned char DefButton, unsigned long Type)
{
#ifdef _WIN32
HWND ParentWin = NULL;
#endif // _WIN32
int ReplyStatus;

#ifndef BUILD_LIB // building as library
if (ReqLocalGUI)
	{
	ReqLocalGUI->GoModal();
	#ifdef _WIN32
	ParentWin = ReqLocalGUI->RootWin;
	#endif // _WIN32
	} // if
#endif // BUILD_LIB
#ifdef _WIN32
if (DefButton == 1) Type |= MB_DEFBUTTON2;
if (DefButton == 2) Type |= MB_DEFBUTTON3;

//ReplyStatus = MessageBox(ParentWin, Message, Topic, MB_YESNOCANCEL | MB_TASKMODAL | MB_SETFOREGROUND |  Type);
PrevUserMessageWin = GetActiveWindow();

ReplyStatus = MessageBox(ParentWin, Message, Topic, MB_YESNOCANCEL | MB_TASKMODAL | MB_SETFOREGROUND |  Type);

if (PrevUserMessageWin) SetActiveWindow(PrevUserMessageWin);
PrevUserMessageWin = NULL;
#endif // _WIN32
#ifndef BUILD_LIB // building as library
if (ReqLocalGUI) ReqLocalGUI->EndModal();
#endif // BUILD_LIB

#ifdef _WIN32
if (ReplyStatus == IDYES) return(2);
if (ReplyStatus == IDNO)  return(1);
else /* (ReplyStatus == IDCANCEL) */ return(0);
#endif // _WIN32

} // UserMessageYNCAN
