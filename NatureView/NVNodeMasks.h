// NVNodeMasks.h
// Node Masks for Scene Graph

#ifndef NVW_NODEMASKS_H
#define NVW_NODEMASKS_H

enum
	{ // NodeMasks
	NVW_NODEMASK_TANGIBLE = (1 << 0),
	NVW_NODEMASK_INTANGIBLE = (1 << 1), // stuff you can see but can't "hit"
	NVW_NODEMASK_FOLIAGE = (1 << 2),
	NVW_NODEMASK_TERRAIN = (1 << 3),
	NVW_NODEMASK_STRUCT = (1 << 4), // aka 3D Objects
	NVW_NODEMASK_LABEL = (1 << 5),
	NVW_NODEMASK_VECTOR = (1 << 6),
	NVW_NODEMASK_OCEAN = (1 << 7),
	NVW_NODEMASK_SKY = (1 << 8),
	NVW_NODEMASK_EVERYTHING = 0xffffffff
	}; // enum

#endif // NVW_NODEMASKS_H
