// Notify.cpp
// The Notify object, and other supporting cast
// created from scratch by CXH on 2/10/96
// Copyright 1996

#include "stdafx.h"
#include "Application.h"
#include "Notify.h"

extern WCSApp *GlobalApp;

NotifyEx::NotifyEx()
{
int Clear;

for(Clear = 0; Clear < WCS_NOTIFY_DELAY_MAX_CHANGE; Clear++)
	{
	DelayedList[Clear] = NULL;
	} // for

for(Clear = 0; Clear < WCS_NOTIFY_MAX_CLIENTS; Clear++)
	{
	NotifyList[Clear].ClientModule = NULL;
	NotifyList[Clear].Interested = NULL;
	} // for

DelayedData = NULL;

} // NotifyEx::NotifyEx

/*===========================================================================*/

int NotifyEx::RegisterClient(WCSModule *Client, NotifyTag *InterestedList)
{
int FindBlank;

if(Client && InterestedList)
	{
	for(FindBlank = 0; FindBlank < WCS_NOTIFY_MAX_CLIENTS; FindBlank++)
		{
		if(NotifyList[FindBlank].ClientModule == NULL)
			{
			NotifyList[FindBlank].ClientModule = Client;
			NotifyList[FindBlank].Interested   = InterestedList;
			return(1);
			} // if
		} // for
	} // if
return(0);

} // NotifyEx::RegisterClient

/*===========================================================================*/

void NotifyEx::RemoveClient(WCSModule *Client)
{
int FindBlank;

if(Client)
	{
	for(FindBlank = 0; FindBlank < WCS_NOTIFY_MAX_CLIENTS; FindBlank++)
		{
		if(NotifyList[FindBlank].ClientModule == Client)
			{
			NotifyList[FindBlank].ClientModule = NULL;
			NotifyList[FindBlank].Interested   = NULL;
			return;
			} // if
		} // for
	} // if
} // NotifyEx::RemoveClient

/*===========================================================================*/

NotifyEvent ChangeEvent;
static AppEvent AEvent;

/*===========================================================================*/

int NotifyEx::GenerateNotify(NotifyTag *ChangedList, void *NotifyData)
{
int FindClient, Notified = 0;

if(ChangedList)
	{
	for(FindClient = 0; FindClient < WCS_NOTIFY_MAX_CLIENTS; FindClient++)
		{
		if(NotifyList[FindClient].ClientModule)
			{
			if(MatchNotifyClass(NotifyList[FindClient].Interested, ChangedList, 0))
				{
				// Do the notify
				AEvent.Type = WCS_APP_EVENTTYPE_NOTIFYCHANGE;
				AEvent.ChangeNotify = &ChangeEvent;
				
				ChangeEvent.NotifyOrigin = this;
				ChangeEvent.ChangeList = ChangedList;
				ChangeEvent.NotifyData = NotifyData;

				NotifyList[FindClient].ClientModule->SetCommonVars(&AEvent, GlobalApp, NULL, NULL);
				NotifyList[FindClient].ClientModule->HandleNotifyEvent();
				Notified++;
				} // if
			} // if
		} // for
	} // if

return(Notified);
} // NotifyEx::GenerateNotify

/*===========================================================================*/

int NotifyEx::GenerateDelayedNotify(NotifyTag *ChangedList, void *NotifyData)
{
int TotalChanged;

if(ChangedList && (DelayedList[0] == NULL))
	{
	// Count number of entries in ChangedList, see if it'll fit.
	for(TotalChanged = 0; ChangedList[TotalChanged]; TotalChanged++);	//lint !e722
	
	if((TotalChanged > 0) && (TotalChanged <= WCS_NOTIFY_DELAY_MAX_CHANGE))
		{
		for(TotalChanged = 0; ChangedList[TotalChanged]; TotalChanged++)
			{
			DelayedList[TotalChanged] = ChangedList[TotalChanged];
			} // for
		// NULL-terminate list
		DelayedList[TotalChanged] = NULL;
		DelayedData = NotifyData;
		if(GlobalApp->SetDelayNotify(this))
			{
			return(TRUE);
			} // if
		} // if
	} // if
return(NULL);

} // NotifyEx::GenerateDelayedNotify

/*===========================================================================*/

NotifyTag NotifyEx::MatchNotifyClass(NotifyTag *Match, NotifyTag *Events, int Single)
{
int Search, Seek;
unsigned char ClassCheck, Verify, SubClassCheck, ItemCheck, ComponentCheck;

if(Match && Events)
	{
	for(Search = 0; Match[Search]; Search++)
		{
		if((Search > 0) && (Single))
			{
			return(0);
			} // if
		for(Seek = 0; Events[Seek]; Seek++)
			{
			ClassCheck  = (unsigned char)((Match[Search] & 0xff000000) >> 24);
			SubClassCheck  = (unsigned char)((Match[Search] & 0x00ff0000) >> 16);
			ItemCheck  = (unsigned char)((Match[Search] & 0x0000ff00) >> 8);
			ComponentCheck  = (unsigned char)(Match[Search] & 0x000000ff);
			if((ClassCheck == 0xff) && (SubClassCheck == 0xff) && (ItemCheck == 0xff) && (ComponentCheck == 0xff))
				{ // ...
				return (Events[Seek]);
				} // if
			Verify = (unsigned char)((Events[Seek]  & 0xff000000) >> 24);
			if((ClassCheck == 0xff) || (ClassCheck == Verify) || (Verify == 0xff)) // is Class the same?
				{
				if((SubClassCheck == 0xff) && (ItemCheck == 0xff) && (ComponentCheck == 0xff))
					{ // ...
					return (Events[Seek]); // is subclass wildcard?
					} // if
				Verify = (unsigned char)((Events[Seek]  & 0x00ff0000) >> 16);
				if((SubClassCheck == 0xff) || (SubClassCheck == Verify) || (Verify == 0xff)) // is subclass the same?
					{
					if((ItemCheck == 0xff) && (ComponentCheck == 0xff))
						{ // ...
						return (Events[Seek]); // is item wildcard?
						} // if
					Verify = (unsigned char)((Events[Seek]  & 0x0000ff00) >> 8);
					if((ItemCheck == 0xff) || (ItemCheck == Verify) || (Verify == 0xff))
						{
						// Optimize: Bail out here to not check component?
						//return(Events[Seek]);
						if(ComponentCheck == 0xff)
							{ // ...
							return (Events[Seek]); // is component wildcard?
							} // if
						Verify = (unsigned char)(Events[Seek]  & 0x000000ff);
						if((ComponentCheck == 0xff) || (ComponentCheck == Verify) || (Verify == 0xff))
							{
							return(Events[Seek]);
							} // if
						} // if
					} // if
				} // if
			} // if
		} // for
	} // if

return(0);
} // NotifyEx::MatchNotifyClass

/*===========================================================================*/

NotifyTag NotifyEx::MatchNotifyComponent(NotifyTag *Match, NotifyTag *Events, int Single)
{
int Search, Seek;
unsigned char Verify, ComponentCheck;

if(Match && Events)
	{
	for(Search = 0; Match[Search]; Search++)
		{
		if((Search > 0) && (Single))
			{
			return(0);
			} // if
		for(Seek = 0; Events[Seek]; Seek++)
			{
			ComponentCheck  = (unsigned char)(Match[Search] & 0x000000ff);
			Verify = (unsigned char)(Events[Seek]  & 0x000000ff);
			if((ComponentCheck == 0xff) || (ComponentCheck == Verify))
				{
				return(Events[Seek]);
				} // if
			} // if
		} // for
	} // if

return(0);

} // NotifyEx::MatchNotifyComponent

/*===========================================================================*/

#ifdef OLD_NOTIFY
NotifyTag NotifyEx::MatchNotifyClass(NotifyTag *Match, NotifyTag *Events, int Single)
{
int Search, Seek;
unsigned char Check, Verify;

if(Match && Events)
	{
	for(Search = 0; Match[Search]; Search++)
		{
		if((Search > 0) && (Single))
			{
			return(0);
			} // if
		for(Seek = 0; Events[Seek]; Seek++)
			{
			Check  = (unsigned char)((Match[Search] & 0xff000000) >> 24);
			// Note: Setting high byte to 0xff gives you ALL events.
			// Mostly for debugging or something.
			if(Check == 0xff) return (Events[Seek]);
			Verify = (unsigned char)((Events[Seek]  & 0xff000000) >> 24);
			if(Check == Verify) // is Class the same?
				{
				Check  = (unsigned char)((Match[Search] & 0x00ff0000) >> 16);
				if(Check == 0xff) return (Events[Seek]); // is subclass wildcard?
				Verify = (unsigned char)((Events[Seek]  & 0x00ff0000) >> 16);
				if(Check == Verify) // is subclass the same?
					{
					Check  = (unsigned char)((Match[Search] & 0x0000ff00) >> 8);
					if(Check == 0xff) return (Events[Seek]); // is item wildcard?
					Verify = (unsigned char)((Events[Seek]  & 0x0000ff00) >> 8);
					if(Check == Verify)
						{
						// Optimize: Bail out here to not check component?
						return(Events[Seek]);
						//Check  = (unsigned char)(Match[Search] & 0x000000ff);
						//if(Check == 0xff) return 1; // is component wildcard?
						//Verify = (unsigned char)(Events[Seek]  & 0x000000ff);
						//if(Check == Verify)
						//	{
						//	return(1);
						//	} // if
						} // if
					} // if
				} // if
			} // if
		} // for
	} // if

return(0);
} // NotifyEx::MatchNotifyClass
#endif // OLD_NOTIFY
