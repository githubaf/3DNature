// ThinThread.h
// A really simple (thin) threading management object
// built from scratch on 10/29/07 by CXH
//
// WARNING: For performance reasons, this has very little error-checking. Don't be dumb.

#include "stdafx.h"

#ifndef WCS_THINTHREAD_H
#define WCS_THINTHREAD_H

#define WCS_THINTHREAD_DEATHWATCH_TIME_MS	1000		// time to wait, in milliseconds
#define WCS_THINTHREAD_TIME_INFINTE	INFINITE

class ThinThread
	{
	private:
		HANDLE _W32ThreadObject;
		DWORD _W32ThreadID;
		bool _Paused;
		LPTHREAD_START_ROUTINE _RequestTerminateFunction; // same prototype as our start function
		void *_StoredData;
	
	public:
		ThinThread();
		~ThinThread();
		// Function and RequestTerminate must be function pointers to functions of the form of:
		// ThreadProc http://msdn2.microsoft.com/en-us/library/ms686736.aspx
		// DWORD WINAPI Function(void * Data) or DWORD WINAPI RequestTerminate(void * Data)
		bool Start(void *Function, void *Data, void *RequestTerminate = NULL, bool StartPaused = false);
		bool Pause(bool NewPausedState);
		bool GetPaused(void) const {return(_Paused);};
		bool WaitForCompletion(int HowLongToWait_ms); // returns true if successfully completed
		bool RequestVoluntaryTermination(void);
		void ForciblyKill(void);
		bool CheckCompleted(void); // returns true if completed
	}; // ThinThread

class ThinMutex
	{
	private:
		HANDLE _Mutex;
	public:
		ThinMutex();
		~ThinMutex();
		bool Obtain(int HowLongToWait_ms); // returns true if obtained
		void Release(void);
	}; // ThinMutex

class ThinSemaphore
	{
	private:
		HANDLE _Semaphore;	
	public:
		ThinSemaphore(int MaxCount, int StartCount);
		~ThinSemaphore();
		bool Obtain(int HowLongToWait_ms); // returns true if obtained
		void Release(void);
	}; // ThinSemaphore

// for Work Relay/Queue (implementation of solution to Producer/Consumer problem)
// see http://en.wikipedia.org/wiki/Producer-consumer_problem

// this class only works with one producer, one consumer, and one work item
class ThinSingleWorkRelay
	{
	private:
		ThinSemaphore _QueueIsEmpty, _QueueHasWorkAvailable;
		void *CurrentItem;
	public:
		ThinSingleWorkRelay();
		~ThinSingleWorkRelay();
		bool WaitAndAddItem(void *NewItem, int HowLongToWait_ms = WCS_THINTHREAD_TIME_INFINTE);
		void *WaitForAndRemoveItem(int HowLongToWait_ms = WCS_THINTHREAD_TIME_INFINTE);
		bool WaitForAllWorkCompletion(int HowLongToWait_ms = WCS_THINTHREAD_TIME_INFINTE);
	}; // ThinSingleWorkRelay

// this is multiple work items, but still only one producer and one consumer
class ThinWorkQueue
	{
	protected:
		(void *)_InternalWorkQueue; // will be an array of void pointers to blind work units [avoiding STL and containers for simplicity]
		ThinSemaphore _QueueIsEmpty, _QueueHasWorkAvailable;
	public:
		ThinWorkQueue(int MaxQueueDepth);
		~ThinWorkQueue();
		bool AddItemToQueue(void *WorkItem, int HowLongToWait_ms = WCS_THINTHREAD_TIME_INFINTE);
		void *WaitForAndRemoveItem(int HowLongToWait_ms = WCS_THINTHREAD_TIME_INFINTE);
		bool WaitForAllWorkCompletion(int HowLongToWait_ms = WCS_THINTHREAD_TIME_INFINTE);
		int CheckHowMuchWorkRemainsNow(void); // this value could be out of date by the time you use it. Not Implemented yet
	}; // ThinWorkQueue

// tis adds mutex protection allowing multiple producers and/or consumers
class ThinWorkQueueMulti : public ThinWorkQueue
	{
	private:
		ThinMutex _Mutex;
	public:
		ThinWorkQueueMulti(int MaxQueueDepth);
		~ThinWorkQueueMulti();
		bool AddItemToQueue(void *WorkItem, int HowLongToWait_ms = WCS_THINTHREAD_TIME_INFINTE);
		void *WaitForAndRemoveItem(int HowLongToWait_ms = WCS_THINTHREAD_TIME_INFINTE);
	}; // ThinWorkQueueMulti

#endif // WCS_THINTHREAD_H
