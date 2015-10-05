// RequesterBasic.h
// Very basic GUI/requester code
// built from Requester.h on 060403 by Chris "Xenon" Hanson

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// These tell the UserMessage requesters what cute icon to put in
// the Box.
#ifdef _WIN32
enum
	{
	REQUESTER_ICON_EXCLAMATION = MB_ICONEXCLAMATION,
	REQUESTER_ICON_INFORMATION = MB_ICONINFORMATION,
	REQUESTER_ICON_QUESTION = MB_ICONQUESTION,
	REQUESTER_ICON_STOP = MB_ICONSTOP
	}; // IconTypes
#endif // _WIN32

// More complex ones in Requester.h/Requester.cpp, only the basic system-supplied ones here
void          UserMessageOK(const char *Topic, const char *Message,
							unsigned long Type = REQUESTER_ICON_INFORMATION);
unsigned char UserMessageNOMEM(char *Topic, unsigned long ReqSize, unsigned long AvailSize);
unsigned char UserMessageOKCAN(char *Topic, char *Message, unsigned char DefButton = 0,
							   unsigned long Type = REQUESTER_ICON_INFORMATION);
unsigned char UserMessageRETRYCAN(char *Topic, char *Message, unsigned char DefButton = 0,
								  unsigned long Type = REQUESTER_ICON_INFORMATION);
unsigned char UserMessageYN(char *Topic, char *Message, unsigned char DefButton = 0,
							unsigned long Type = REQUESTER_ICON_INFORMATION);
unsigned char UserMessageYNCAN(char *Topic, char *Message, unsigned char DefButton = 0,
							   unsigned long Type = REQUESTER_ICON_INFORMATION);
