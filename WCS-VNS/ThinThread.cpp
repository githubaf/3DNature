// ThinThread.cpp
// A really simple (thin) threading management object
// built from scratch on 10/29/07 by CXH
//
// WARNING: For performance reasons, this has very little error-checking. Don't be dumb.

#include "stdafx.h"
#include "ThinThread.h"

/* =================================== ThinThread =================================== */

ThinThread::ThinThread()
{
_W32ThreadObject = NULL;
_W32ThreadID = NULL;
_Paused = false;
_RequestTerminateFunction = NULL;
_StoredData = NULL;
} // ThinThread::ThinThread

ThinThread::~ThinThread()
{
if(_W32ThreadObject)
	{	
	// ask for it to exit cleanly
	RequestVoluntaryTermination();
	// wait while it tries to die quietly
	WaitForCompletion(WCS_THINTHREAD_DEATHWATCH_TIME_MS);

	// is it done?
	if(!CheckCompleted())
		{
		// kill it
		ForciblyKill();
		} // if
	// clean it up
	CloseHandle(_W32ThreadObject);
	_W32ThreadObject = NULL;
	_W32ThreadID = NULL;
	} // if
} // ThinThread::~ThinThread

bool ThinThread::Start(void *Function, void *Data, void *RequestTerminate, bool Start_Paused)
{
_RequestTerminateFunction = (LPTHREAD_START_ROUTINE)RequestTerminate;
_StoredData = Data;
_W32ThreadObject = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Function, (LPVOID)Data, Start_Paused ? CREATE_SUSPENDED : NULL, &_W32ThreadID);
return(_W32ThreadObject ? true : false);
} // ThinThread::Start

bool ThinThread::Pause(bool New_PausedState)
{
if(New_PausedState) // are we currently _Paused?
	{ // Yes, unpause
	ResumeThread(_W32ThreadObject);
	} // if
else
	{ // no, Pause now
	SuspendThread(_W32ThreadObject);
	} // else
New_PausedState = !New_PausedState;
return(New_PausedState);
} // ThinThread::Pause

bool ThinThread::WaitForCompletion(int HowLongToWait_ms)
{
WaitForSingleObject(_W32ThreadObject, HowLongToWait_ms);

return(CheckCompleted());
} // ThinThread::WaitForCompletion

bool ThinThread::CheckCompleted(void)
{
DWORD Status;

// is it done?
if(GetExitCodeThread(_W32ThreadObject, &Status))
	{
	if(Status == STILL_ACTIVE) // nope, still running
		{
		return(false);
		} // if
	else
		{
		return(true);
		} // else
	} // if
return(false); // actually, unknown
} // ThinThread::CheckCompleted

bool ThinThread::RequestVoluntaryTermination(void)
{
if(_RequestTerminateFunction) _RequestTerminateFunction(_StoredData);
SwitchToThread(); // let it try to terminate

return(CheckCompleted()); // has it ended yet?
} // ThinThread::RequestVoluntaryTermination

void ThinThread::ForciblyKill(void)
{
TerminateThread(_W32ThreadObject, 0);
} // ThinThread::ForciblyKill

/* =================================== ThinMutex =================================== */


ThinMutex::ThinMutex()
{
_Mutex = CreateMutex(NULL, FALSE, NULL);
} // ThinMutex::ThinMutex

ThinMutex::~ThinMutex()
{
if(_Mutex) CloseHandle(_Mutex);
_Mutex = NULL;
} // ThinMutex::~ThinMutex

bool ThinMutex::Obtain(int HowLongToWait_ms)
{
return((WaitForSingleObject(_Mutex, HowLongToWait_ms) == WAIT_TIMEOUT) ? false : true);
} // ThinMutex::Obtain

void ThinMutex::Release(void)
{
ReleaseMutex(_Mutex);
} // ThinMutex::Release


/* =================================== ThinSemaphore =================================== */

ThinSemaphore::ThinSemaphore(int MaxCount, int StartCount)
{
_Semaphore = CreateSemaphore(NULL, StartCount, MaxCount, NULL);
} // ThinSemaphore::ThinSemaphore

ThinSemaphore::~ThinSemaphore()
{
if(_Semaphore) CloseHandle(_Semaphore);
_Semaphore = NULL;
} // ThinSemaphore::~ThinSemaphore

bool ThinSemaphore::Obtain(int HowLongToWait_ms)
{
return((WaitForSingleObject(_Semaphore, HowLongToWait_ms) == WAIT_TIMEOUT) ? false : true);
} // ThinSemaphore::Obtain

void ThinSemaphore::Release(void)
{
ReleaseSemaphore(_Semaphore, 1, NULL);
} // ThinSemaphore::Release

/* =================================== ThinSingleWorkRelay =================================== */

ThinSingleWorkRelay::ThinSingleWorkRelay()
: _QueueIsEmpty(1, 1), _QueueHasWorkAvailable(1, 0) // at start: empty=1, full=0
{
CurrentItem = NULL;
} // ThinSingleWorkRelay::ThinSingleWorkRelay

ThinSingleWorkRelay::~ThinSingleWorkRelay()
{ // don't think there's anything to do, actually
} // ThinSingleWorkRelay::~ThinSingleWorkRelay

bool ThinSingleWorkRelay::WaitAndAddItem(void *NewItem, int HowLongToWait_ms)
{
bool Success = false;
// wait for queue to become empty (not full)
if(_QueueIsEmpty.Obtain(HowLongToWait_ms)) // down(empty)
	{
	CurrentItem = NewItem; // putItemIntoBuffer(item)
	_QueueHasWorkAvailable.Release(); // up(full)
	Success = true;
	} // if
return(Success);
} // ThinSingleWorkRelay::WaitAndAddItem

void *ThinSingleWorkRelay::WaitForAndRemoveItem(int HowLongToWait_ms)
{
void *RetreivedItem = NULL;
// wait for queue to become full (not empty)
if(_QueueHasWorkAvailable.Obtain(HowLongToWait_ms)) // down(full)
	{
	RetreivedItem = CurrentItem; // item = removeItemFromBuffer()
	CurrentItem = NULL; // clear the queue
	_QueueIsEmpty.Release(); // up(empty)
	} // if
return(RetreivedItem);
} // ThinSingleWorkRelay::WaitForAndRemoveItem


bool ThinSingleWorkRelay::WaitForAllWorkCompletion(int HowLongToWait_ms)
{
bool Success = false;
// wait for queue to become empty (not full)
if(_QueueIsEmpty.Obtain(HowLongToWait_ms)) // down(empty)
	{
	Success = true;
	// we've changed the queue status by the above wait/obtain operation, so we need to restore it
	_QueueIsEmpty.Release();
	} // if
return(Success);
} // ThinSingleWorkRelay::WaitForAllWorkCompletion




/* =================================== ThinWorkQueue =================================== */



ThinWorkQueue::ThinWorkQueue(int MaxQueueDepth)
: _QueueIsEmpty(MaxQueueDepth, MaxQueueDepth), _QueueHasWorkAvailable(1, 0) // at start: empty=MaxQueueDepth, full=0
{
_InternalWorkQueue = NULL;

} // ThinWorkQueue::ThinWorkQueue

ThinWorkQueue::~ThinWorkQueue()
{

// <<<>>>

delete _InternalWorkQueue;
_InternalWorkQueue = NULL;
} // ThinWorkQueue::~ThinWorkQueue


bool ThinWorkQueue::AddItemToQueue(void *WorkItem, int HowLongToWait_ms)
{
bool Success = false;
if(_QueueIsEmpty.Obtain(HowLongToWait_ms)) // down(empty)
	{
	// <<<>>> // putItemIntoBuffer(item)
	_QueueHasWorkAvailable.Release(); // up(full)
	Success = true;
	} // if
return(Success);
} // ThinWorkQueue::AddItemToQueue

void *ThinWorkQueue::WaitForAndRemoveItem(int HowLongToWait_ms)
{
void *RetreivedItem = NULL;
if(_QueueHasWorkAvailable.Obtain(HowLongToWait_ms)) // down(full)
	{
	// <<<>>> // item = removeItemFromBuffer()
	_QueueIsEmpty.Release(); // up(empty)
	} // if
return(RetreivedItem);
} // ThinWorkQueue::WaitForAndRemoveItem


bool ThinWorkQueue::WaitForAllWorkCompletion(int HowLongToWait_ms)
{
bool Success = false;
// wait for queue to become empty (not full)
if(_QueueIsEmpty.Obtain(HowLongToWait_ms)) // down(empty)
	{
	Success = true;
	// we've changed the queue status by the above wait/obtain operation, so we need to restore it
	_QueueIsEmpty.Release();
	} // if
return(Success);
} // ThinWorkQueue::WaitForAllWorkCompletion

int ThinWorkQueue::CheckHowMuchWorkRemainsNow(void)
{
// <<<>>> Not Implemented yet
return(0);
} // ThinWorkQueue::CheckHowMuchWorkRemainsNow


/* =================================== ThinWorkQueueMulti =================================== */

ThinWorkQueueMulti::ThinWorkQueueMulti(int MaxQueueDepth)
: ThinWorkQueue(MaxQueueDepth)
{
} // ThinWorkQueueMulti::ThinWorkQueueMulti

bool ThinWorkQueueMulti::AddItemToQueue(void *WorkItem, int HowLongToWait_ms)
{
bool Success = false;
if(_QueueIsEmpty.Obtain(HowLongToWait_ms)) // down(empty)
	{
	_Mutex.Obtain(WCS_THINTHREAD_TIME_INFINTE); // can't really abort from here, AFAIK
	// <<<>>> // putItemIntoBuffer(item)
	_Mutex.Release();
	_QueueHasWorkAvailable.Release(); // up(full)
	Success = true;
	} // if
return(Success);
} // ThinWorkQueueMulti::AddItemToQueue

void *ThinWorkQueueMulti::WaitForAndRemoveItem(int HowLongToWait_ms)
{
void *RetreivedItem = NULL;
if(_QueueHasWorkAvailable.Obtain(HowLongToWait_ms)) // down(full)
	{
	_Mutex.Obtain(WCS_THINTHREAD_TIME_INFINTE); // can't really abort from here, AFAIK
	// <<<>>> // item = removeItemFromBuffer()
	_Mutex.Release();
	_QueueIsEmpty.Release(); // up(empty)
	} // if
return(RetreivedItem);
} // ThinWorkQueueMulti::WaitForAndRemoveItem

