// QuantaAllocator.h
// Specialized memory-management functions for inter-related pools of fixed-size objects 
// that need highly-efficient allocation and freeing
// Created from scratch 8/4/06 by CXH

// Intended for use with objects like WCS/VNS's VectorNode class and friends.

//
// this is written using "long long" (64-bit) ints so that it can make the migration to
// 64-bit address space in the future if need be.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_QUANTAALLOCATOR_H
#define WCS_QUANTAALLOCATOR_H

#if _MSC_VER < 1400 // VC++8/2005
typedef unsigned long WCS_QUANTAALLOCATOR_BIGSIZE_TYPE;
#else // older compilers
typedef unsigned long long WCS_QUANTAALLOCATOR_BIGSIZE_TYPE;
#endif // old compilers

#include <vector>

// define this to allow the QA system to be able to free blocks back to the system when they become
// 100% unused, or recycle memory from unused blocks in one pool over to another pool that needs them.
// If this is _not_ defined, once allocated, memory stays in a particular pool until the destruction
// of the QA system. However, omitting WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING support speeds up
// allocation/freeing and lowers the memory overhead of each Item, so that's a significant savings.
//#define WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING

// define this (but not here, over in QuantaAllocator.cpp) only if you want extensive and expensive
// leak tracking of all allocated QA types. It's defined over there so it can be toggled at compile time
// without a recompile of EVERYTHING. It's listed here too so you notice it.
// WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING

// if WCS_QUANTAALLOCATOR_SUPPORT__ALLOCATION_TRACKING is defined, you can call this to iterate the
// currently-allocated blocks (and customize it to do something). It won't tell you the size or type
// of those blocks, just the pointer to them.
void QAWalkAllocated(void);




// placeholder forward declarations
class QuantaAllocator;

// The size (in bytes) of the BlockID data member that is (sometimes) hidden in front of each Item
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
#define WCS_QUANTAALLOCATOR_UNIT_BLOCKID_SIZE_BYTES		2
#else // !WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
#define WCS_QUANTAALLOCATOR_UNIT_BLOCKID_SIZE_BYTES		0
#endif // !WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING

#define WCS_QUANTAALLOCATOR_HEADROOM_PERCENT		0.05


// this is the header used to keep track of quanta units while they're sitting in the free list
// Each unit is usually bigger than this, this is just the defined portion of it
class QuantaUnitHeader
	{
	public:
		QuantaUnitHeader *Next;
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		unsigned short SourceBlockNum;
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
	}; // QuantaUnitHeader
	
// we're assuming here that one block will never be larger than 4Gb, so we use 32-bit sizes and counters
class QuantaBlock
	{
	public:
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		unsigned long UnitsUsedInBlock;
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		unsigned long BlockSize;
		void *RawBlock;
	
		QuantaBlock()
			{
			#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
			UnitsUsedInBlock = 0;
			#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
			BlockSize = 0; RawBlock = NULL;
			};
	}; // class QuantaBlock

class QuantaAllocatorErrorFunctor
	{
	// A functor is an object that allows us to pass customizable state data to a callback function, without
	// knowing in advance what exactly the data will be. All we need to know is which method to call [operator (), which
	// is virtual so it can be overridden in a derived class without us knowing]. Derive a class from this, add any data
	// members your callback will need access to, and implement your callback in operator().
	// Refer to http://en.wikipedia.org/wiki/Function_object for more details.

	// return TRUE to indicate that you were able to free some memory and to try again. FALSE means give up.
	// This callback needs to be able to do its thing WITHOUT disrupting the state of the allocation call
	// that is in progress, or, any other call on the stack. So be REALLY careful when you employ this.
	public: // so it can be accessed by QuantaAllocatorPool::AttemptAllocateNewBlock
		virtual bool operator() (QuantaAllocator *ManagingAllocator) = 0; // Pure virtual, must be overridden. ManagingAllocator provides access to any state you might want to look at
	}; // QuantaAllocatorErrorFunctor

class QuantaAllocatorPool
	{
	friend class QuantaAllocator;
	private:
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		// only called from QuantaAllocator
		bool AttemptExpungeUnusedBlocks(void); // returns true if it succeeded in expunging any. May expunge more than one.
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		// only called by our own code
		bool CheckAtLimitOfCombinedPool(unsigned long PoolAllocSize); // returns true if an alloc of the specified size would put us over the limit
		// only called from QuantaAllocatorPool::AttemptAllocateNewBlock
		bool CallCeilingErrorHandlerAndFree(void);

	public:
		// don't ever modify these after construction of this object
		// these are read-only, but I don't want to incur the possible overhead of enforcing this
		// by some property/get method
		// I don't know of any reason to make these 64-bit at this time
		unsigned long PoolQuantaSize, PoolInitialUnits, PoolGrowByUnits, PoolAlignSize, PoolPaddedAlignedBlockSize, PoolPaddedSemiAlignedBlockSize;
		QuantaAllocator *Manager;
		QuantaUnitHeader *AvailablePool;
		std::vector<QuantaBlock> Blocks; // not pointers to blocks, but actual blocks

#ifdef DEBUG
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE _PoolMaxMemoryInUseAtOnceBytes, _PoolMaxMemoryInUseAtOncePaddedBytes, _PoolCurrentMemoryInUseBytes, _PoolCurrentMemoryInUsePaddedBytes, _PoolCurrentItemsInUse, _PoolMaxItemsInUse;
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) const {return(_PoolCurrentItemsInUse);};
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) const {return(_PoolMaxItemsInUse);};
#else  // !DEBUG
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) const {return(0);};
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) const {return(0);};
#endif // !DEBUG

		QuantaAllocatorPool(QuantaAllocator *NewManager, unsigned long NewPoolQuantaSize, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits, unsigned long NewPoolAlignSize);
		~QuantaAllocatorPool();
		
		bool AttemptAllocateNewBlock(bool TryHard); // TryHard: Will ask managing host to expunge, and host app to cleanup before retrying or failing
		void *AttemptAllocateNewItem(bool TryHard); // TryHard: Will ask managing host to expunge, and host app to cleanup before retrying or failing
		void FreeItem(void *Item); // does not try to expunge anything

#ifdef DEBUG
		void IncreasePoolCurrentMemoryInUseByOneUnit(void);
		void DecreasePoolCurrentMemoryInUseByOneUnit(void);
#endif // DEBUG

	}; // QuantaAllocatorPool

class QuantaAllocator
	{
	friend class QuantaAllocatorPool;
	private:
#ifdef DEBUG
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE _MaxMemoryInUseAtOnceBytes, _MaxMemoryInUseAtOncePaddedBytes, _CurrentMemoryInUseBytes, _CurrentMemoryInUsePaddedBytes;
#endif // DEBUG
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE _CombinedPoolActualCeilingBytes, _CombinedPoolCeilingBytes, _CombinedPoolCurrentSizeBytes;
		QuantaAllocatorErrorFunctor *_CeilingErrorHandler;
		std::vector<QuantaAllocatorPool *> _Pools; // pointers to QuantaAllocatorPools, because we want to be able to share these pointer with other code

		// only called from QuantaAllocatorPool
		void IncreaseCombinedPoolCurrentSizeBy(const unsigned long SizeModifier) {_CombinedPoolCurrentSizeBytes += SizeModifier;};
#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		void DecreaseCombinedPoolCurrentSizeBy(const unsigned long SizeModifier) {_CombinedPoolCurrentSizeBytes -= SizeModifier;};
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		
	public:
		QuantaAllocator(unsigned long InitialPoolSlots = 12);
		~QuantaAllocator();

		// SetCombinedPoolCeilingBytes(0) to disable pool size limit checking (defaults to 0)
		void SetCombinedPoolCeilingBytes(const WCS_QUANTAALLOCATOR_BIGSIZE_TYPE NewCombinedPoolCeilingBytes) {_CombinedPoolActualCeilingBytes = NewCombinedPoolCeilingBytes; _CombinedPoolCeilingBytes = (WCS_QUANTAALLOCATOR_BIGSIZE_TYPE)((double)_CombinedPoolActualCeilingBytes * (1.0 - (double)WCS_QUANTAALLOCATOR_HEADROOM_PERCENT));};
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetCombinedPoolCeilingBytes(void) const {return(_CombinedPoolCeilingBytes);};
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetCombinedPoolCurrentSizeBytes(void) const {return(_CombinedPoolCurrentSizeBytes);};
		void InstallCeilingErrorHandlerFunctor(QuantaAllocatorErrorFunctor *NewErrorHandler) {_CeilingErrorHandler = NewErrorHandler;};
		// this keeps track of the pool in the internal _Pools container
		QuantaAllocatorPool *CreatePool(unsigned long NewPoolQuantaSize, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits, unsigned long NewPoolAlignSize);
		// There is no DeletePool() -- all you can do is delete the whole QuantaAllocator

#ifdef WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING
		bool AttemptExpungeSomeUnusedBlocks(bool ExpungeAll); // returns true if it succeeded in expunging any blocks on any pools
#endif // WCS_QUANTAALLOCATOR_SUPPORT_INSITU_FREEING

#ifdef DEBUG
		void IncreaseCurrentMemoryInUseBy(const unsigned long SizeModifier, const unsigned long PaddedSizeModifier);
		void DecreaseCurrentMemoryInUseBy(const unsigned long SizeModifier, const unsigned long PaddedSizeModifier);
#endif // DEBUG
#ifdef DEBUG
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetCurrentMemoryInUseBytes(void) const {return(_CurrentMemoryInUseBytes);};
#else  // !DEBUG
		WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetCurrentMemoryInUseBytes(void) const {return(0);};
#endif // !DEBUG

	}; // QuantaAllocator

#endif // WCS_QUANTAALLOCATOR_H
