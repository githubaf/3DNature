// UnitTests.cpp
// some tests we can run at startup
// moved out of WCS.cpp on 10/29/07 by CXH

#include "stdafx.h"
#include "ThinThread.h"

//#define WCS_THINTHREAD_UNIT_TEST_B

#ifdef WCS_THINTHREAD_UNIT_TEST_A
#include "RequesterBasic.h"

bool KeepGoing = true;

int InternalCounter;

DWORD WINAPI ThinThreadFunction(void * Data)
{ // this is run within the alternate thread

while(KeepGoing)
	{
	InternalCounter++;
	} // while

return(0);
} // ThinThreadFunction

DWORD WINAPI ThinThreadRequestTerminate(void * Data)
{
// this is NOT a safe way to do this -- KeepGoing is not protected in any way.
// for this purpose, it's not a big deal, but don't do it this way. Normally,
// we'll use a Work Queue and send a Quit command of some sort via the Queue.
KeepGoing = false;
return(true);
} // ThinThreadRequestTerminate

#endif // WCS_THINTHREAD_UNIT_TEST_A



#ifdef WCS_THINTHREAD_UNIT_TEST_B
#include "RequesterBasic.h"
#include "Random.h"
#include "UsefulTime.h"

static int InternalCounter;

void SimulateDoWork(int HowLongWillItTake)
{
Sleep(HowLongWillItTake); // "do some work" that takes a while
InternalCounter += HowLongWillItTake; // record that we did that much "work" for test purposes
} // SimulateDoWork

DWORD WINAPI ThinThreadWorkRelayFunction(void * Data)
{ // this is run within the alternate thread
// normally we'd pass a bigger object that probably contained our WorkRelay,
// plus other useful stuff, but this is just a test/demo
ThinSingleWorkRelay *LocalWorkRelay = (ThinSingleWorkRelay *)Data;
void *WorkUnit;

while(1) // until we break
	{
	// wait for some work to arrive
	if(WorkUnit = LocalWorkRelay->WaitForAndRemoveItem())
		{ // simulate doing some work
		int HowLongWillItTake = (int)WorkUnit; // not dereferencing WorkUnit, just using it as a number
		SimulateDoWork(HowLongWillItTake); // "do some work" that takes a while
		} // if
	else
		{ // NULL work unit indicates exit
		break; // exit while, and exit thread
		} // else
	} // while

return(0);
} // ThinThreadWorkRelayFunction

DWORD WINAPI ThinThreadWorkRelayRequestTerminate(void * Data)
{
ThinSingleWorkRelay *LocalWorkRelay = (ThinSingleWorkRelay *)Data;
LocalWorkRelay->WaitAndAddItem(NULL); // Wait as long as necessary for Consumer thread to idle, then send NULL work unit to terminate
return(true);
} // ThinThreadWorkRelayRequestTerminate

#endif // WCS_THINTHREAD_UNIT_TEST_B



#ifdef WCS_TEST_QUANTA_ALLOCATOR
	// Test:
	// 24250 VectorNodes, 4363802 VectorNodeLinks, 290893 VectorPolygonListDoubles, and 290893 EffectJoeLists.
	#define WCS_TEST_VECTORNODE_QUANTITY 24250
	#define WCS_TEST_VECTORNODELINKS_QUANTITY 4363802
	#define WCS_TEST_VECTORPOLYGONLISTDOUBLES_QUANTITY 290893
	#define WCS_TEST_EFFECTJOELISTS_QUANTITY 290893
	
	VectorNode *VN[WCS_TEST_VECTORNODE_QUANTITY];
	VectorNodeLink *VNL[WCS_TEST_VECTORNODELINKS_QUANTITY];
	VectorPolygonListDouble *VPLD[WCS_TEST_VECTORPOLYGONLISTDOUBLES_QUANTITY];
	EffectJoeList *EJL[WCS_TEST_EFFECTJOELISTS_QUANTITY];
#endif // WCS_TEST_QUANTA_ALLOCATOR


void RunUnitTests(void)
{


	
#ifdef ATAN_TEST
float ATanParam = 0.0;
float MaxATanError = 0.0, MaxATanErrorArg = 0.0;


double TestStartTime, TestEndTime, TestElapsedTime;

TestStartTime = GetSystemTimeFP();
for(unsigned long int WasteTime = 0; WasteTime < 200; WasteTime++)
for(int ATanLoop = 0; ATanLoop < 3000000; ATanLoop++)
	{
	double Res; //, ResAppx, Delta;
	Res = fastatanf(ATanParam);
	if(Res > MaxATanError)
		{
		MaxATanError = (float)Res;
		} // if
/*	ResAppx = fastatanf(ATanParam);
	Delta = fabs(Res - ResAppx);
	if(Delta > MaxATanError)
		{
		MaxATanError = (float)Delta;
		MaxATanErrorArg = ATanParam;
		} // if
*/
	ATanParam += .01f;
	} // for

TestEndTime = GetSystemTimeFP();
TestElapsedTime = TestEndTime - TestStartTime;

/*
if(MaxATanError > 0.0)
	{
	char ATanMsg[255];
	sprintf(ATanMsg, "Maximum error %f at %f.\natan:%f fast:%f", MaxATanError, MaxATanErrorArg, atan(MaxATanErrorArg), fastatanf(MaxATanErrorArg));
	UserMessageOK("Diagnostic atan", ATanMsg);
	} // if
*/

char ATanMsg[255];
sprintf(ATanMsg, "Elapsed time: %f [%f]", TestElapsedTime, MaxATanError);
UserMessageOK("Diagnostic atan", ATanMsg);
#endif // ATAN_TEST





#ifdef WCS_TEST_QUANTA_ALLOCATOR
	{
	QuantaAllocator QA;
	unsigned long int VNloop;

	class MyQuantaAllocatorErrorFunctor : public QuantaAllocatorErrorFunctor
		{
		public:
			bool operator() (QuantaAllocator *ManagingAllocator) {UserMessageOK("QuantaAllocator", "Here, we would try to free some resources.", REQUESTER_ICON_STOP); return(true);};
		}; // MyQuantaAllocatorErrorFunctor

	double TestStartTime, TestEndTime, TestElapsedTime;
	
	TestStartTime = GetSystemTimeFP();

	MyQuantaAllocatorErrorFunctor MyErrorHandler;
	MaterialList::PrepareAllocation(&QA, 100000, 50000); // prepare the class
	VectorNodeRenderData::PrepareAllocation(&QA, 100000, 50000); // prepare the class
	VectorNodeList::PrepareAllocation(&QA, 10000, 5000); // prepare the class
	VectorNodeLink::PrepareAllocation(&QA, 1000000, 250000); // prepare the class
	VectorNode::PrepareAllocation(&QA, 100000, 50000); // prepare the class
	EffectJoeList::PrepareAllocation(&QA, 10000, 5000); // prepare the class
	VectorPolygon::PrepareAllocation(&QA, 10000, 5000); // prepare the class
	VectorPolygonList::PrepareAllocation(&QA, 1000, 500); // prepare the class
	VectorPolygonListDouble::PrepareAllocation(&QA, 10000, 5000); // prepare the class
	PolygonBoundingBox::PrepareAllocation(&QA, 10000, 5000); // prepare the class

	QA.InstallCeilingErrorHandlerFunctor(&MyErrorHandler);
	QA.SetCombinedPoolCeilingBytes(0); // 0=disabled

	for(VNloop = 0; VNloop < WCS_TEST_VECTORNODE_QUANTITY; VNloop++)
		{
		VN[VNloop] = new VectorNode;
		} // for

	for(VNloop = 0; VNloop < WCS_TEST_VECTORNODELINKS_QUANTITY; VNloop++)
		{
		VNL[VNloop] = new VectorNodeLink;
		} // for

	for(VNloop = 0; VNloop < WCS_TEST_VECTORPOLYGONLISTDOUBLES_QUANTITY; VNloop++)
		{
		VPLD[VNloop] = new VectorPolygonListDouble;
		} // for

	for(VNloop = 0; VNloop < WCS_TEST_EFFECTJOELISTS_QUANTITY; VNloop++)
		{
		// Can't test this, no simple constructor
		//EJL[VNloop] = new EffectJoeList;
		} // for
	
	// Now free

	for(VNloop = 0; VNloop < WCS_TEST_VECTORNODE_QUANTITY; VNloop++)
		{
		delete VN[VNloop];
		VN[VNloop] = NULL;
		} // for

	for(VNloop = 0; VNloop < WCS_TEST_VECTORNODELINKS_QUANTITY; VNloop++)
		{
		delete VNL[VNloop];
		VNL[VNloop] = NULL;
		} // for

	for(VNloop = 0; VNloop < WCS_TEST_VECTORPOLYGONLISTDOUBLES_QUANTITY; VNloop++)
		{
		delete VPLD[VNloop];
		} // for

	for(VNloop = 0; VNloop < WCS_TEST_EFFECTJOELISTS_QUANTITY; VNloop++)
		{
		// Can't test this
		delete EJL[VNloop];
		} // for
	
	
	VectorNode::CleanupAllocation(); // clean up
	VectorNodeLink::CleanupAllocation(); // clean up
	VectorPolygonListDouble::CleanupAllocation(); // clean up
	EffectJoeList::CleanupAllocation(); // clean up
	
	TestEndTime = GetSystemTimeFP();
	TestElapsedTime = TestEndTime - TestStartTime;
	char Message[300];
	sprintf(Message, "Elapsed time: %f seconds.\n", TestElapsedTime);
	UserMessageOK("Time", Message);
	} // temp scope
#endif // WCS_TEST_QUANTA_ALLOCATOR


#ifdef WCS_THINTHREAD_UNIT_TEST_A
ThinThread EnoughRope;
InternalCounter = 0;

EnoughRope.Start(ThinThreadFunction, &EnoughRope, ThinThreadRequestTerminate, false);
Sleep(1000); // 1 second
EnoughRope.RequestVoluntaryTermination();
if(EnoughRope.CheckCompleted())
	{
	UserMessageOK("ThinThread Debug", "Terminated Voluntarily within time allotted.");
	} // if
else
	{
	UserMessageOK("ThinThread Debug", "Attempting delayed termination.");
	EnoughRope.WaitForCompletion(1000);
	if(EnoughRope.CheckCompleted())
		{
		UserMessageOK("ThinThread Debug", "Terminated voluntarily after delay.");
		} // if
	else
		{
		EnoughRope.ForciblyKill();
		UserMessageOK("ThinThread Debug", "Terminated forcibly.");
		} // else
	} // else

char ThinThreadDebugStr[100];

sprintf(ThinThreadDebugStr, "InternalCounter: %d loops run by Thin Thread.", InternalCounter);
UserMessageOK("ThinThread Progress", ThinThreadDebugStr);


#endif // WCS_THINTHREAD_UNIT_TEST_A


#ifdef WCS_THINTHREAD_UNIT_TEST_B
ThinThread EnoughRope;
ThinSingleWorkRelay WorkHandoff;
InternalCounter = 0;
double StartTime, EndTime, ElapseTime;

EnoughRope.Start(ThinThreadWorkRelayFunction, &WorkHandoff, ThinThreadWorkRelayRequestTerminate, false); // false = do not start thread as paused

// simulate generating and dispatching some number of random batches of work to consumer-worker thread
unsigned long int Random;
void *RandomAsWorkUnitPointer;

xseed48(0x12345678, 0x90abcdef); // uniform seeding

StartTime = GetSystemTimeFP();
for(int i=0; i<250; i++) // 250 batches
	{
	Random = (unsigned int)(xrand48()* 100.0); // max of 1/10th of milliseconds
	RandomAsWorkUnitPointer = (void *)(Random + 1); // can't be 0
	WorkHandoff.WaitAndAddItem(RandomAsWorkUnitPointer); // wait as long as necessary for worker-consumer to be available, then submit work unit and return
	//SimulateDoWork(Random + 1); // for comparison, this is how we do it without threading
	} // for

WorkHandoff.WaitForAllWorkCompletion(); // wait indefinitely for worker-consumer to finish
EndTime = GetSystemTimeFP();
ElapseTime = EndTime - StartTime;

EnoughRope.RequestVoluntaryTermination(); // send worker-consumer a NULL work unit to signal it to terminate

if(EnoughRope.CheckCompleted())
	{
	UserMessageOK("ThinThreadWorkRelay Debug", "Terminated Voluntarily within time allotted.");
	} // if
else
	{
	//UserMessageOK("ThinThreadWorkRelay Debug", "Attempting delayed termination.");
	EnoughRope.WaitForCompletion(1000);
	if(EnoughRope.CheckCompleted())
		{
		UserMessageOK("ThinThreadWorkRelay Debug", "Terminated voluntarily after delay.");
		} // if
	else
		{
		EnoughRope.ForciblyKill();
		UserMessageOK("ThinThreadWorkRelay Debug", "Terminated forcibly.");
		} // else
	} // else


static char ThinThreadDebugStr[100];

sprintf(ThinThreadDebugStr, "InternalCounter: %d Work Units simulated by ThinThreadWorkRelay\nin %f seonds.", InternalCounter, ElapseTime);
UserMessageOK("ThinThreadWorkRelay Progress", ThinThreadDebugStr);


#endif // WCS_THINTHREAD_UNIT_TEST_B


} // RunUnitTests
