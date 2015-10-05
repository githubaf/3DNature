// QuantaAllocator.cpp
// Specialized memory-management functions for inter-related pools of fixed-size objects 
// that need highly-efficient allocation and freeing
// Created from scratch 8/4/06 by CXH

#include "stdafx.h"
#include "QuantaAllocator.h"
#include "UsefulMath.h"
// inhibit Window's min/max defines, since we're using the C++ standard template ones instead
#define NOMINMAX //lint -e750
#include "AppMem.h"

#ifndef NDEBUG
#define NDEBUG // turn off asserts even in debug builds
#endif // NDEBUG

// define this here only if you want extensive and expensive
// leak tracking of all allocated QA types. It's defined over there so it can be toggled at compile time
// without a recompile of EVERYTHING.
//#define WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING

#ifdef WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING
#include <set>
std::set<void *> QAAllocationTracker;

#include <typeinfo.h>
class VectorNodeLink;

void QAWalkAllocated(void)
{
std::set<void *>::iterator Walk;
std::set<void *>::size_type CurrentlyAllocated;
CurrentlyAllocated = QAAllocationTracker.size();

VectorNodeLink *Foo;
const char *foo_name;

for(Walk = QAAllocationTracker.begin(); Walk != QAAllocationTracker.end(); Walk++)
	{
	Foo = (VectorNodeLink *)*Walk; // pull original void * out of container iterator. Make Foo whatever class you want to view it as.
	const type_info& foo_id = typeid(Foo);
	foo_name = foo_id.name();
	} // for
} // QAWalkAllocated

#endif // WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING

/*===========================================================================*/

QuantaAllocatorPool::QuantaAllocatorPool(QuantaAllocator *NewManager, unsigned long NewPoolQuantaSize, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits, unsigned long NewPoolAlignSize)
{
unsigned long PoolPaddedBlockSize;
PoolQuantaSize = NewPoolQuantaSize;
PoolInitialUnits = NewPoolInitialUnits;
PoolGrowByUnits = NewPoolGrowByUnits;
PoolAlignSize = NewPoolAlignSize;
PoolPaddedBlockSize = NewPoolQuantaSize + WCS_QUANTAALLOCATOR_UNIT_BLOCKID_SIZE_BYTES;
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
PoolPaddedSemiAlignedBlockSize = ROUNDUP(PoolPaddedBlockSize, WCS_QUANTAALLOCATOR_UNIT_BLOCKID_SIZE_BYTES); // make sure BlockID is aligned to _its_ size
#else // !WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
PoolPaddedSemiAlignedBlockSize = PoolPaddedBlockSize;
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
PoolPaddedAlignedBlockSize = ROUNDUP(PoolPaddedSemiAlignedBlockSize, NewPoolAlignSize); // align that up to the real specified size, if need be.
AvailablePool = NULL;
Manager = NewManager;

#ifdef DEBUG
_PoolMaxMemoryInUseAtOnceBytes = _PoolMaxMemoryInUseAtOncePaddedBytes = _PoolCurrentMemoryInUseBytes = _PoolCurrentMemoryInUsePaddedBytes = _PoolCurrentItemsInUse = _PoolMaxItemsInUse = 0;
#endif // DEBUG
} // QuantaAllocatorPool::QuantaAllocatorPool

/*===========================================================================*/

// The destructor does not update any usage counts in the QuantaAllocator host -- do not call except from the QuantaAllocator::~QuantaAllocator destructor.
QuantaAllocatorPool::~QuantaAllocatorPool()
{
// iterate all QuantaBlocks and delete their RawBlocks
for(std::vector<QuantaBlock>::iterator BlockWalk = Blocks.begin(); BlockWalk != Blocks.end(); ++BlockWalk)
	{
	AppMem_Free(BlockWalk->RawBlock, BlockWalk->BlockSize);
	// no need to clear the vars in the QuantaBlock, it'll be obliterated in a moment anyway...
	} // for
} // QuantaAllocatorPool::~QuantaAllocatorPool

/*===========================================================================*/

#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
bool QuantaAllocatorPool::AttemptExpungeUnusedBlocks(void)
{
bool Result = false;
unsigned long CurBlockNum = 0;


// Iterate the free-item list and remove Items that belong to Blocks with zero usage. (This is an odd process, given the singly-linked list)
for(QuantaUnitHeader *PoolScan = AvailablePool; PoolScan != NULL;)
	{
	// is the next block a candidate for removal?
	if(PoolScan->Next && Blocks[PoolScan->Next->SourceBlockNum].UnitsUsedInBlock == 0)
		{
		// remove the "Next" block and link to Next->Next (which could be NULL)
		PoolScan->Next = PoolScan->Next->Next;
		// we don't move on until we know that the current "Next" is a keeper
		} // if
	else
		{ // no we move on to the next
		PoolScan = PoolScan->Next;
		} // else
	} // if
// Handle the unit at the front of the list
if(AvailablePool && Blocks[AvailablePool->SourceBlockNum].UnitsUsedInBlock == 0)
	{
	AvailablePool = AvailablePool->Next;
	} // if


// iterate all QuantaBlocks and expunge any with UnitsUsedInBlock == 0
for(std::vector<QuantaBlock>::iterator BlockWalk = Blocks.begin(); BlockWalk != Blocks.end();)
	{
	if(BlockWalk->UnitsUsedInBlock == 0)
		{ // remove this block and move onto next block
		Result = true;
		// free the RawBlock
		AppMem_Free(BlockWalk->RawBlock, BlockWalk->BlockSize);
		// Update our Manager's counters
		Manager->DecreaseCombinedPoolCurrentSizeBy(BlockWalk->BlockSize);
		// remove this QuantaBlock from the container
		BlockWalk = Blocks.erase(BlockWalk); // deletes this entry, safely returns next entry
		} // if
	else
		{ // move onto next
		++BlockWalk;
 		} // else
 	CurBlockNum++;
	} // for

return(Result);
} // QuantaAllocatorPool::AttemptExpungeUnusedBlocks
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING

/*===========================================================================*/

// returns true if an alloc of the specified size would put us over the limit
bool QuantaAllocatorPool::CheckAtLimitOfCombinedPool(unsigned long PoolAllocSize)
{
return(Manager->GetCombinedPoolCeilingBytes() > 0 && Manager->GetCombinedPoolCurrentSizeBytes() + PoolAllocSize > Manager->GetCombinedPoolCeilingBytes());
} // QuantaAllocatorPool::CheckAtLimitOfCombinedPool

/*===========================================================================*/

bool QuantaAllocatorPool::AttemptAllocateNewBlock(bool TryHard)
{
unsigned long PoolAllocUnits, PoolAllocSize;
void *NewRawBlock;

if(Blocks.size() == 0)
	{
	PoolAllocUnits = PoolInitialUnits;
	} // if
else
	{
	PoolAllocUnits = PoolGrowByUnits;
	} // else

PoolAllocSize = PoolPaddedAlignedBlockSize * PoolAllocUnits;

// are we allowed to grab a new block, or will we hit the ceiling and need to make some space?
if(CheckAtLimitOfCombinedPool(PoolAllocSize))
	{
	if (TryHard)
		{
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		// try to expunge a single block that is wholly unused right now (if any)
		if(Manager->AttemptExpungeSomeUnusedBlocks(false)) // false = just one block, even if more could be expunged
			{ // we expunged something, see if it was enough
			if(CheckAtLimitOfCombinedPool(PoolAllocSize)) // was that insufficient? Do we need to expunge more?
				{ // need to expunge more if we can
				Manager->AttemptExpungeSomeUnusedBlocks(true); // true = expunge anything you can
				} // if
			else
				{ // no need to expunge more
				// ok, nothing needs to be done, we'll handle things further below
				} // else
			} // if
		else
			{ // couldn't expunge anything, need to call error handler (below)
			} // else
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		if(CheckAtLimitOfCombinedPool(PoolAllocSize)) // have we solved the problem, or do we need to try the error handler?
			{ // still not enough, try the error handler
			CallCeilingErrorHandlerAndFree(); // we don't care about the result, we'll just check the state of the pool afterward
			if(AvailablePool)
				{
				// we're ok, return success
				return(true);
				} // else
			else
				{ // need to invoke extreme measures, because the usual measures weren't adequate
				// we will fall through to AppMem_Alloc, below, without enforcing memory ceiling any more
				} // if
			} // if
		} // if
	else
		return (false);
	} // if

NewRawBlock = AppMem_Alloc(PoolAllocSize, NULL, NULL, true); // we pass Optional=true to avoid modal error requesters right here
if(!NewRawBlock) // alloc failed, let's try (possibly a scond time) to free some items instead
	{
	return(CallCeilingErrorHandlerAndFree()); // either this worked and returned item to the pool to avoid needing allocation, or we fail.
	} // if
else // Lots to do here to prep the new block for use as items
	{
	unsigned char *RawBlockSplitter = (unsigned char *)NewRawBlock; // cast over for by-byte indexing
	QuantaUnitHeader *UnitHeader;
	unsigned long UnitIndexer;
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
	unsigned long MyBlockNum;
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
	QuantaBlock MyBlock;
	
	// Record the block size in the combined pool size
	Manager->IncreaseCombinedPoolCurrentSizeBy(PoolAllocSize);
	
	// record the block info
	MyBlock.BlockSize = PoolAllocSize;
	MyBlock.RawBlock = NewRawBlock;
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
	MyBlock.UnitsUsedInBlock = 0;
	MyBlockNum = Blocks.size(); // what is the subscript for the next item we might push on the end?
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
	Blocks.push_back(MyBlock); // Push it on. <<<>>> this could throw an exception, couldn't it?
	
	// break up the block into items
	// Uncomment ALL the lines marked [forward mode] or [reverse mode] to select stacking order
	//for(unsigned long UnitStep = UnitIndexer = 0; UnitStep < PoolAllocUnits; UnitStep++) // [forward mode]
	for(unsigned long UnitStep = PoolAllocUnits - 1; ; UnitStep--) // [reverse mode]
		{
		UnitIndexer = UnitStep * PoolPaddedAlignedBlockSize; // Locate our block [reverse mode]
		UnitHeader = (QuantaUnitHeader *)&RawBlockSplitter[UnitIndexer]; // grab the next Unit
		//UnitIndexer += PoolPaddedAlignedBlockSize; // step ahead one padded Unit size [forward mode]
		UnitHeader->Next = AvailablePool;
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		UnitHeader->SourceBlockNum = (unsigned short)MyBlockNum;
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		AvailablePool = UnitHeader;
		if(UnitStep == 0) break; // [reverse mode]
		} // for
	return(true);
	} // if

return(false);

} // QuantaAllocatorPool::AttemptAllocateNewBlock

/*===========================================================================*/

// returns true if we freed something
bool QuantaAllocatorPool::CallCeilingErrorHandlerAndFree(void)
{

if(Manager->_CeilingErrorHandler != NULL) // is the error handler installed?
	{ // error handler is present, invoke it
	if(Manager->_CeilingErrorHandler->operator ()(Manager)) // call error handler, passing QuantaAllocator object for additional info, if desired
		{
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		Manager->AttemptExpungeSomeUnusedBlocks(true); // true = expunge anything you can
		if(CheckAtLimitOfCombinedPool(PoolAllocSize)) // Did that gain us any ground?
			{ // no, still constrained
			// might want to implement a second level of freeing 
			return(false); // nothing worked, bail out
			} // if
		// if we're now below the limit we fall through to the AppMem_Alloc 
		// block below where a new block is appropriated
#else // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		// return and try using the freed resources
		return(true);	
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		} // if
	else
		{
		// might want to implement a second level of freeing 
		return (false);
		} // else
	} // if
else	// no error handler
	return(false);

return(false); // just in case

} // QuantaAllocatorPool::CallCeilingErrorHandlerAndFree

/*===========================================================================*/

void *QuantaAllocatorPool::AttemptAllocateNewItem(bool TryHard)
{
void *Result = NULL;

if(TryHard && !AvailablePool)
	{
	AttemptAllocateNewBlock(TryHard);
	} // if
	
if(AvailablePool)
	{
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
	unsigned short *BlockNumStash;
	unsigned char *ItemRawMemIndexer;
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
	Result = (void *)AvailablePool;
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
	ItemRawMemIndexer = (unsigned char *)Result; // setup for arbitrary indexing to access specific offsets
	// record the SourceBlockNum after the allocated block (alignment issues?)
	// calculate where the SourceBlockNum will be stashed
	BlockNumStash = (unsigned short *)&ItemRawMemIndexer[PoolPaddedSemiAlignedBlockSize - WCS_QUANTAALLOCATOR_UNIT_BLOCKID_SIZE_BYTES];
	*BlockNumStash = AvailablePool->SourceBlockNum; // write the SourceBlockNum
	// record change in Block's UnitsInUse
	assert(Blocks.size() > AvailablePool->SourceBlockNum);
	Blocks[AvailablePool->SourceBlockNum].UnitsUsedInBlock++;
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
	AvailablePool = AvailablePool->Next; // move the next one up, or move a NULL into the Available slot
	// record metrics about bytes in use
	#ifdef DEBUG
	IncreasePoolCurrentMemoryInUseByOneUnit();
	#endif // DEBUG
	} // if

#ifdef WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING
if(Result)
	{
	QAAllocationTracker.insert(Result);
	} // if
#endif // WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING

return(Result);

} // QuantaAllocatorPool::AttemptAllocateNewItem

/*===========================================================================*/

void QuantaAllocatorPool::FreeItem(void *Item)
{
QuantaUnitHeader *ItemAsQuantaUnitHeader;
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
unsigned short RecoveredSourceBlockNum;
unsigned short *BlockNumStash; // for recovering the SourceBlockNum ID
unsigned char *ItemRawMemIndexer; // for recovering the SourceBlockNum ID
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING

assert(Item != NULL);

#ifdef WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING
QAAllocationTracker.erase(Item);
#endif // WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING

#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
ItemRawMemIndexer = (unsigned char *)Item; // setup for arbitrary indexing to access specific offsets
// find the SourceBlockNum after the allocated block (alignment issues?)
// calculate where the SourceBlockNum will be read from
BlockNumStash = (unsigned short *)&ItemRawMemIndexer[PoolPaddedSemiAlignedBlockSize - WCS_QUANTAALLOCATOR_UNIT_BLOCKID_SIZE_BYTES];
RecoveredSourceBlockNum = *BlockNumStash; // Recover the SourceBlockNum ID
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING

ItemAsQuantaUnitHeader = (QuantaUnitHeader *)Item;
ItemAsQuantaUnitHeader->Next = AvailablePool; // link up to existing free list
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
ItemAsQuantaUnitHeader->SourceBlockNum = RecoveredSourceBlockNum; // replace the recovered SourceBlockNum
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
AvailablePool = ItemAsQuantaUnitHeader; // put ourselves at the top of the free list

#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
assert(Blocks.size() > RecoveredSourceBlockNum);
assert(Blocks[RecoveredSourceBlockNum].UnitsUsedInBlock > 0);
Blocks[RecoveredSourceBlockNum].UnitsUsedInBlock--;
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
// record metrics about bytes in use
#ifdef DEBUG
DecreasePoolCurrentMemoryInUseByOneUnit();
#endif // DEBUG

} // QuantaAllocatorPool::FreeItem

/*===========================================================================*/

#ifdef DEBUG
void QuantaAllocatorPool::IncreasePoolCurrentMemoryInUseByOneUnit(void)
{
_PoolCurrentItemsInUse++;
if(_PoolCurrentItemsInUse > _PoolMaxItemsInUse) _PoolMaxItemsInUse = _PoolCurrentItemsInUse;
_PoolCurrentMemoryInUseBytes += PoolQuantaSize;
_PoolCurrentMemoryInUsePaddedBytes += PoolPaddedAlignedBlockSize;
if(_PoolCurrentMemoryInUseBytes > _PoolMaxMemoryInUseAtOnceBytes) _PoolMaxMemoryInUseAtOnceBytes = _PoolCurrentMemoryInUseBytes;
if(_PoolCurrentMemoryInUsePaddedBytes > _PoolMaxMemoryInUseAtOnceBytes) _PoolMaxMemoryInUseAtOnceBytes = _PoolCurrentMemoryInUsePaddedBytes;
Manager->IncreaseCurrentMemoryInUseBy(PoolQuantaSize, PoolPaddedAlignedBlockSize);
} // QuantaAllocatorPool::IncreasePoolCurrentMemoryInUseByOneUnit

/*===========================================================================*/

void QuantaAllocatorPool::DecreasePoolCurrentMemoryInUseByOneUnit(void)
{
_PoolCurrentItemsInUse--;
_PoolCurrentMemoryInUseBytes -= PoolQuantaSize;
_PoolCurrentMemoryInUsePaddedBytes -= PoolPaddedAlignedBlockSize;
Manager->DecreaseCurrentMemoryInUseBy(PoolQuantaSize, PoolPaddedAlignedBlockSize);
//  max tracking not needed when size decreasing
}; // QuantaAllocatorPool::DecreasePoolCurrentMemoryInUseByOneUnit
#endif // DEBUG

/*===========================================================================*/

QuantaAllocator::QuantaAllocator(unsigned long InitialPoolSlots)
{
_CombinedPoolCeilingBytes = _CombinedPoolCurrentSizeBytes = 0;
_CeilingErrorHandler = NULL;

#ifdef DEBUG
_MaxMemoryInUseAtOnceBytes = _MaxMemoryInUseAtOncePaddedBytes = _CurrentMemoryInUseBytes = _CurrentMemoryInUsePaddedBytes = 0;
#endif // DEBUG

_Pools.reserve(InitialPoolSlots); // pre-configure for a dozen pools so we don't resize at runtime

} // QuantaAllocator::QuantaAllocator

/*===========================================================================*/

QuantaAllocatorPool *QuantaAllocator::CreatePool(unsigned long NewPoolQuantaSize, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits, unsigned long NewPoolAlignSize)
{
QuantaAllocatorPool *FreshPool = NULL;

try
	{
	if(FreshPool = new QuantaAllocatorPool(this, NewPoolQuantaSize, NewPoolInitialUnits, NewPoolGrowByUnits, NewPoolAlignSize))
		{
		_Pools.push_back(FreshPool);
		return(FreshPool);
		} // if
	} // try
catch(...) // handle STL push_back failure, or anything else
	{
	delete FreshPool;
	} // catch
return(NULL);

} // QuantaAllocator::CreatePool

/*===========================================================================*/

#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
bool QuantaAllocator::AttemptExpungeSomeUnusedBlocks(bool ExpungeAll) // returns true if it succeeded in expunging any blocks on any pools
{
bool Result = false;
for(std::vector<QuantaAllocatorPool *>::iterator PoolWalk = _Pools.begin(); PoolWalk != _Pools.end(); ++PoolWalk)
	{
	if((*PoolWalk)->AttemptExpungeUnusedBlocks())
		{
		Result = true;
		if(!ExpungeAll) break; // we got one, let's bail here..
		} // if
	} // for
return(Result);
} // QuantaAllocator::AttemptExpungeSomeUnusedBlocks
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING

/*===========================================================================*/

#ifdef DEBUG
void QuantaAllocator::IncreaseCurrentMemoryInUseBy(const unsigned long SizeModifier, const unsigned long PaddedSizeModifier)
{
_CurrentMemoryInUseBytes += SizeModifier;
_CurrentMemoryInUsePaddedBytes += PaddedSizeModifier;
if(_CurrentMemoryInUseBytes > _MaxMemoryInUseAtOnceBytes) _MaxMemoryInUseAtOnceBytes = _CurrentMemoryInUseBytes;
if(_CurrentMemoryInUsePaddedBytes > _MaxMemoryInUseAtOncePaddedBytes) _MaxMemoryInUseAtOncePaddedBytes = _CurrentMemoryInUsePaddedBytes;
} // QuantaAllocator::IncreaseCurrentMemoryInUseBy

/*===========================================================================*/

void QuantaAllocator::DecreaseCurrentMemoryInUseBy(const unsigned long SizeModifier, const unsigned long PaddedSizeModifier)
{
_CurrentMemoryInUseBytes -= SizeModifier; _CurrentMemoryInUsePaddedBytes -= PaddedSizeModifier;
// max size tracking not needed when size is decreasing
} // QuantaAllocator::DecreaseCurrentMemoryInUseBy
#endif // DEBUG

/*===========================================================================*/

QuantaAllocator::~QuantaAllocator()
{
// iterate all _Pools and delete them
for(std::vector<QuantaAllocatorPool *>::iterator PoolWalk = _Pools.begin(); PoolWalk != _Pools.end(); ++PoolWalk)
	{
	if(*PoolWalk)
		{
		delete (*PoolWalk); // (*PoolWalk) is still a pointer
		// we don't clear the entry because the whole container will be gone momentarily
		} // if
	} // for
#ifdef WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING
QAAllocationTracker.clear();
#endif // WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING
} // QuantaAllocator::~QuantaAllocator
