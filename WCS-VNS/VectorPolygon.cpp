// VectorPolygon.cpp
// Source file for VectorPolygon class
// Created by Gary R. Huber 1/24/06
// Copyright 2006 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "VectorPolygon.h"
#include "VectorNode.h"
#include "VectorIntersecter.h"
#include "EffectEval.h"
#include "Joe.h"
#include "Points.h"
#include "Lists.h"
#include "Render.h"	// for VertexDEM
#include "EffectsLib.h"	// for CoordSys
#include "Database.h"	// for point deallocation
#include "Requester.h"	// for point deallocation

using namespace std;
//#define DEBUG_POLYGONS_TO_VECTOR

#ifdef DEBUG_POLYGONS_TO_VECTOR
extern bool PrintToVector;
#endif // DEBUG_POLYGONS_TO_VECTOR

#define WCS_VECTORPOLYGON_NODECOORD_TOLERANCE	(1.0E-11)

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//extern unsigned __int64 DevCounter[50];
//#endif // FRANK or GARY

#define MAKENOCLONES

QuantaAllocatorPool *VectorPart::MyPool;
QuantaAllocatorPool *VectorPolygon::MyPool;
QuantaAllocatorPool *VectorPolygonList::MyPool;
QuantaAllocatorPool *VectorPolygonListDouble::MyPoolD1;
QuantaAllocatorPool *PolygonBoundingBox::MyPool;
QuantaAllocatorPool *PolygonEdgeList::MyPool;

VNAccelerator VNA;
#ifdef _DEBUG
unsigned long NumberOfPolygons;
#endif // _DEBUG

//extern VectorNode *DebugNode;

VectorPart::~VectorPart()
{
RemoveNodes();
} // VectorPart::~VectorPart

/*===========================================================================*/

void VectorPart::RemoveNodes(void)
{

for (VectorNode *CurNode = MyNodes; NumNodes > 0; CurNode = MyNodes)
	{
	MyNodes = MyNodes->NextNode;
	delete CurNode;
	--NumNodes;
	} // for

} // VectorPart::RemoveNodes

/*===========================================================================*/

VectorNode *VectorPart::FirstNode(void)
{

return (MyNodes);

} // VectorPart::FirstNode

/*===========================================================================*/

VectorNode *VectorPart::SecondNode(void)
{

return (MyNodes ? MyNodes->NextNode: NULL);

} // VectorPart::SecondNode

/*===========================================================================*/

VectorNode *VectorPart::ThirdNode(void)
{

return (MyNodes && MyNodes->NextNode ? MyNodes->NextNode->NextNode: NULL);

} // VectorPart::ThirdNode

/*===========================================================================*/

unsigned long VectorPart::CountNodes(void)
{

NumNodes = MyNodes ? 1: 0;

if (MyNodes)
	{
	for (VectorNode *CurNode = MyNodes->NextNode; CurNode && CurNode != MyNodes; CurNode = CurNode->NextNode)
		{
		++NumNodes;
		} // for
	} // if

return (NumNodes);

} // VectorPart::CountNodes

/*===========================================================================*/

bool VectorPart::CrossLink(VectorPart *LinkMe, VectorPolygon *ThisPolygon, VectorPolygon *TheOtherPolygon)
{
unsigned long NodeCt;
VectorNode *NodeOfTheFirstPart, *NodeOfTheSecondPart;
bool Success = true;

for (NodeCt = 0, NodeOfTheFirstPart = FirstNode(), NodeOfTheSecondPart = LinkMe->FirstNode(); 
	NodeCt < NumNodes; ++NodeCt, NodeOfTheFirstPart = NodeOfTheFirstPart->NextNode, NodeOfTheSecondPart = NodeOfTheSecondPart->NextNode)
	{
	if (! NodeOfTheFirstPart->AddCrossLinks(NodeOfTheSecondPart, ThisPolygon, TheOtherPolygon))
		{
		if (NodeOfTheFirstPart->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT))
			NodeOfTheSecondPart->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT);
		Success = false;
		break;
		} // if
	} // for

return (Success);

} // VectorPart::CrossLink

/*===========================================================================*/
/*===========================================================================*/

// Constructor creates VectorNodes for the vector vertices, elliminating any duplicates
// For a DEM it creates a four node polygon outlining the DEM using the NWLat, NWLon, etc. 
// members of Joe so it may not be precise but should err on the generous side
// For an area-enclosing vector NumPtsIgnore is false, it should be true for linear vectors
VectorPolygon::VectorPolygon(Database *DBHost, GeneralEffect *NewEffect, Joe *NewVector, bool NumPtsIgnore, bool &InsufficientNodes)
{
bool Success = true;
VectorPoint *PLink, *LastPoint = NULL;
VectorNode **NodePtr = NULL, **PrevNodePtr = NULL;
VectorPart *CurPart;

#ifdef _DEBUG
++NumberOfPolygons;
#endif // _DEBUG

MyEffects = NULL;
TotalNumNodes = 0;
BBox = NULL;
CloneOfThis = NULL;
Normals[3] = Normals[2] = Normals[1] = Normals[0] = 0.0f;
PolyNumber = 0;
ImAnOriginalPolygon = 0;
CurPart = GetFirstPart();

SetSimpleEffect(NewEffect, NewVector);

// eliminate duplicate points
// make last point link back to first point making an endless loop of NextNode pointiers
// NumNodes being 0 and MyNodes NULL will be indication that unsufficient nodes were created

if (NumPtsIgnore || NewVector->GetNumRealPoints() > 2)
	{
	NodePtr = &CurPart->MyNodes;
	for (PLink = NewVector->GetFirstRealPoint(); PLink; PLink = PLink->Next)
		{
		// alter the vector so it doesn't cause a problem ever again
		if (LastPoint && LastPoint->SamePointLatLon(PLink))
			{
			LastPoint->Next = PLink->Next;
			PLink->Next = NULL;
			DBHost->MasterPoint.DeAllocate(PLink);
			PLink = LastPoint;
			NewVector->NumPoints(NewVector->NumPoints() - 1);
			continue;
			} // if
		if (*NodePtr = new VectorNode(PLink))
			{
			PrevNodePtr = NodePtr;
			NodePtr = &(*NodePtr)->NextNode;
			++CurPart->NumNodes;
			LastPoint = PLink;
			} // if
		else
			{
			Success = false;
			break;
			} // else
		} // while
	// create link back to origin
	if (Success)
		{
		// test to see if last point is the origin already
		if (PrevNodePtr)
			{
			if (CurPart->NumNodes > 1 && LastPoint->SamePoint(NewVector->GetFirstRealPoint()))
				{
				// eliminate last point
				delete (*PrevNodePtr);
				--CurPart->NumNodes;
				// link loop back to origin
				*PrevNodePtr = CurPart->FirstNode();
				} // if
			else
				{
				// link loop back to origin
				*NodePtr = CurPart->FirstNode();
				} // else
			} // if
		} // if
	// test to see if we ended up with more than two points after all is said and done
	// if not, eliminate the nodes altogether, this is not a valid area-enclosing polygon
	if ((! NumPtsIgnore && CurPart->NumNodes < 3) || (NumPtsIgnore && CurPart->NumNodes < 1))
		InsufficientNodes = true;
	if (! Success || InsufficientNodes || ! MyEffects)
		{
		RemoveNodes();
		} // if
	} // if
else if (NewVector->TestFlags(WCS_JOEFLAG_ISDEM))
	{
	NodePtr = &CurPart->MyNodes;
	if (*NodePtr = new VectorNode())
		{
		(*NodePtr)->Lat = NewVector->NWLat;
		(*NodePtr)->Lon = NewVector->NWLon;
		NodePtr = &(*NodePtr)->NextNode;
		++CurPart->NumNodes;
		if (*NodePtr = new VectorNode())
			{
			(*NodePtr)->Lat = NewVector->NWLat;
			(*NodePtr)->Lon = NewVector->SELon;
			NodePtr = &(*NodePtr)->NextNode;
			++CurPart->NumNodes;
			if (*NodePtr = new VectorNode())
				{
				(*NodePtr)->Lat = NewVector->SELat;
				(*NodePtr)->Lon = NewVector->SELon;
				NodePtr = &(*NodePtr)->NextNode;
				++CurPart->NumNodes;
				if (*NodePtr = new VectorNode())
					{
					(*NodePtr)->Lat = NewVector->SELat;
					(*NodePtr)->Lon = NewVector->NWLon;
					NodePtr = &(*NodePtr)->NextNode;
					++CurPart->NumNodes;
					// link loop back to origin
					*NodePtr = CurPart->FirstNode();
					} // if
				else
					Success = false;
				} // if
			else
				Success = false;
			} // if
		else
			Success = false;
		} // if
	else
		Success = false;
	// test to see if we ended up with more than two points after all is said and done
	// if not, eliminate the nodes altogether, this is not a valid area-enclosing polygon
	if (! Success || CurPart->NumNodes < 4 || ! MyEffects)
		{
		RemoveNodes();
		} // if
	} // else if

// NumNodes being 0 and MyNodes NULL will be indication that insufficient nodes were created
TotalNumNodes = CurPart->NumNodes;

} // VectorPolygon::VectorPolygon

/*===========================================================================*/

// Creates a three node polygon using the supplied nodes as node templates
// Gets the effects and vector list from the VectorPolygon template
VectorPolygon::VectorPolygon(VectorNode *Pt1, VectorNode *Pt2, VectorNode *Pt3, VectorPolygon *Template)
{
bool Success = true;
VectorNode **NodePtr = NULL;
VectorPart *CurPart;

#ifdef _DEBUG
++NumberOfPolygons;
#endif // _DEBUG

MyEffects = NULL;
TotalNumNodes = 0;
BBox = NULL;
CloneOfThis = NULL;
Normals[3] = Normals[2] = Normals[1] = Normals[0] = 0.0f;
PolyNumber = 0;
ImAnOriginalPolygon = 0;
CurPart = GetFirstPart();

NodePtr = &CurPart->MyNodes;
if (*NodePtr = new VectorNode(Pt1))
	{
	NodePtr = &(*NodePtr)->NextNode;
	++CurPart->NumNodes;
	if (*NodePtr = new VectorNode(Pt2))
		{
		NodePtr = &(*NodePtr)->NextNode;
		++CurPart->NumNodes;
		if (*NodePtr = new VectorNode(Pt3))
			{
			NodePtr = &(*NodePtr)->NextNode;
			++CurPart->NumNodes;
			// link back to origin
			*NodePtr = CurPart->FirstNode();
			} // if
		else
			{
			Success = false;
			} // else
		} // if
	else
		{
		Success = false;
		} // else
	} // if
else
	{
	Success = false;
	} // else

if (Success && Template->MyEffects)
	{
	Success = SupplementEffects(Template);
	} // if

// test to see if we ended up with more than two points after all is said and done
// if not, eliminate the nodes altogether, this is not a valid area-enclosing polygon
if (! Success || CurPart->NumNodes < 3)
	{
	RemoveNodes();
	} // if
TotalNumNodes = CurPart->NumNodes;

} // VectorPolygon::VectorPolygon

/*===========================================================================*/

// Creates a three node polygon using the supplied nodes as node templates
// Adds crosslinks from the new nodes to the supplied nodes and any of the supplied nodes' linked lists
// Gets the effects and vector list from the VectorPolygon template.
// AddLinkBack determines if a crosslink is made between the new nodes and the cloned nodes or just
// a one-way link. If two-way, then Template is assumed to be the owner of the cloned nodes.
VectorPolygon::VectorPolygon(VectorNode *Pt1, VectorNode *Pt2, VectorNode *Pt3, VectorPolygon *Template, bool AddLinkBack)
{
bool Success = true;
VectorNode **NodePtr = NULL;
VectorNodeLink *CurLink;
VectorPart *CurPart;

#ifdef _DEBUG
++NumberOfPolygons;
#endif // _DEBUG

MyEffects = NULL;
TotalNumNodes = 0;
BBox = NULL;
CloneOfThis = NULL;
Normals[3] = Normals[2] = Normals[1] = Normals[0] = 0.0f;
PolyNumber = 0;
ImAnOriginalPolygon = 0;
CurPart = GetFirstPart();

NodePtr = &CurPart->MyNodes;
if (*NodePtr = new VectorNode(Pt1))
	{
	for (CurLink = Pt1->LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList)
		{
		if (! (*NodePtr)->AddCrossLinks(CurLink->MyNode, this, CurLink->LinkedPolygon))
			{
			Success = false;
			break;
			} // if
		} // for
	if (AddLinkBack)
		{
		if (! (*NodePtr)->AddCrossLinks(Pt1, this, Template))
			Success = false;
		} // if
	else
		{
		if (! Pt1->AddLink(*NodePtr, this, 0))
			Success = false;
		} // else
	NodePtr = &(*NodePtr)->NextNode;
	++CurPart->NumNodes;
	if (*NodePtr = new VectorNode(Pt2))
		{
		for (CurLink = Pt2->LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList)
			{
			if (! (*NodePtr)->AddCrossLinks(CurLink->MyNode, this, CurLink->LinkedPolygon))
				{
				Success = false;
				break;
				} // if
			} // for
		if (AddLinkBack)
			{
			if (! (*NodePtr)->AddCrossLinks(Pt2, this, Template))
				Success = false;
			} // if
		else
			{
			if (! Pt2->AddLink(*NodePtr, this, 0))
				Success = false;
			} // else
		NodePtr = &(*NodePtr)->NextNode;
		++CurPart->NumNodes;
		if (*NodePtr = new VectorNode(Pt3))
			{
			for (CurLink = Pt3->LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList)
				{
				if (! (*NodePtr)->AddCrossLinks(CurLink->MyNode, this, CurLink->LinkedPolygon))
					{
					Success = false;
					break;
					} // if
				} // for
			if (AddLinkBack)
				{
				if (! (*NodePtr)->AddCrossLinks(Pt3, this, Template))
					Success = false;
				} // if
			else
				{
				if (! Pt3->AddLink(*NodePtr, this, 0))
					Success = false;
				} // else
			NodePtr = &(*NodePtr)->NextNode;
			++CurPart->NumNodes;
			// link back to origin
			*NodePtr = CurPart->FirstNode();
			} // if
		else
			{
			Success = false;
			} // else
		} // if
	else
		{
		Success = false;
		} // else
	} // if
else
	{
	Success = false;
	} // else

if (Success && Template->MyEffects)
	{
	Success = SupplementEffects(Template);
	} // if

// test to see if we ended up with more than two points after all is said and done
// if not, eliminate the nodes altogether, this is not a valid area-enclosing polygon
if (! Success || CurPart->NumNodes < 3)
	{
	RemoveNodes();
	} // if
TotalNumNodes = CurPart->NumNodes;

} // VectorPolygon::VectorPolygon

/*===========================================================================*/

// Creates a polygon using the supplied nodes directly and linking them together
// Gets the effects and vector list from the VectorPolygon template
VectorPolygon::VectorPolygon(VectorNode **Pts, unsigned long NumPts, VectorPolygon *Template)
{
unsigned long LastPt, PtCt;
VectorPart *CurPart;
bool Success = true;

#ifdef _DEBUG
++NumberOfPolygons;
#endif // _DEBUG

MyEffects = NULL;
TotalNumNodes = 0;
BBox = NULL;
CloneOfThis = NULL;
Normals[3] = Normals[2] = Normals[1] = Normals[0] = 0.0f;
PolyNumber = 0;
ImAnOriginalPolygon = 0;
CurPart = GetFirstPart();

if (NumPts > 0)
	{
	LastPt = NumPts - 1;
	CurPart->MyNodes = Pts[0];
	for (PtCt = 0; PtCt < LastPt; ++PtCt)
		{
		Pts[PtCt]->NextNode = Pts[PtCt + 1];
		} // for
	Pts[LastPt]->NextNode = CurPart->FirstNode();
	CurPart->NumNodes = NumPts;
	} // if

if (Template && Template->MyEffects)
	{
	Success = SupplementEffects(Template);
	} // if

// test to see if we ended up with more than two points after all is said and done
// if not, eliminate the nodes altogether, this is not a valid area-enclosing polygon
if (! Success || CurPart->NumNodes < 3)
	{
	RemoveNodes();
	} // if
TotalNumNodes = CurPart->NumNodes;

} // VectorPolygon::VectorPolygon

/*===========================================================================*/

// Creates an exact replicate of the supplied VectorPolygon but does not clone the node links
// The clone pointer variable in the original is set to point to the clone
VectorPolygon::VectorPolygon(VectorPolygon * const CloneMe)
{
bool Success = true;
VectorNode **NodePtr = NULL, *CurNode;
VectorPart *SourcePart, *DestPart;
unsigned long NodeCt;

#ifdef _DEBUG
++NumberOfPolygons;
#endif // _DEBUG

MyEffects = NULL; 
TotalNumNodes = 0;
BBox = NULL;
CloneOfThis = NULL;
CloneMe->SetClonePointer(this);
PolyNumber = 0;
ImAnOriginalPolygon = 0;
Normals[3] = Normals[2] = Normals[1] = Normals[0] = 0.0f;

if (CloneMe->TotalNumNodes > 0 && CloneMe->PolyFirstNode())
	{
	for (SourcePart = CloneMe->GetFirstPart(), DestPart = GetFirstPart(); SourcePart && DestPart; SourcePart = SourcePart->NextPart)
		{ 
		NodePtr = &DestPart->MyNodes;
		for (CurNode = SourcePart->MyNodes, NodeCt = 0; NodeCt < SourcePart->NumNodes; ++NodeCt, CurNode = CurNode->NextNode)
			{
			if (*NodePtr = new VectorNode(CurNode))
				{
				++DestPart->NumNodes;
				NodePtr = &(*NodePtr)->NextNode;
				} // if
			else
				{
				Success = false;
				break;
				} // else
			} // for
		// link last node to origin
		if (Success)
			{
			*NodePtr = DestPart->FirstNode();
			TotalNumNodes += DestPart->NumNodes;
			// if there is another part, replicate it
			if (SourcePart->NextPart)
				{
				if (DestPart->NextPart = new VectorPart())
					{
					DestPart = DestPart->NextPart;
					} // if
				else
					{
					Success = false;
					break;
					}
				} // if
			} // if
		} // for
	} // if
if (Success && CloneMe->MyEffects)
	{
	Success = SupplementEffects(CloneMe);
	} // if

// if there was a failure remove all the nodes
if (! Success)
	{
	RemoveNodes();
	} // if

} // VectorPolygon::VectorPolygon

/*===========================================================================*/

// Creates a clone of the polygon including node links unless a separate node list is supplied
// in which case the node list is used as is and assumes that the last node is already linked to the first
// If the nodes are cloned, the clone pointer variable in the original is set to point to the clone
VectorPolygon::VectorPolygon(VectorPolygon * const CloneMe, VectorNode *NewNodes, bool AddLinkBack)
{
VectorPart *CurPart;
bool Success = true;

#ifdef _DEBUG
++NumberOfPolygons;
#endif // _DEBUG

MyEffects = NULL;
TotalNumNodes = 0;
BBox = NULL;
CloneOfThis = NULL;
Normals[3] = Normals[2] = Normals[1] = Normals[0] = 0.0f;
PolyNumber = 0;
ImAnOriginalPolygon = 0;
CurPart = GetFirstPart();

if (NewNodes)
	{
	CurPart->MyNodes = NewNodes;
	TotalNumNodes = CurPart->CountNodes();
	} // else
else
	{
	if (CopyNodes(CloneMe, AddLinkBack))
		CloneMe->SetClonePointer(this);
	else
		Success = false;
	} // else

if (Success && CloneMe->MyEffects)
	{
	Success = SupplementEffects(CloneMe);
	} // if

// if there was a failure remove all the nodes
if (! Success)
	{
	RemoveNodes();
	} // if

} // VectorPolygon::VectorPolygon

/*===========================================================================*/

// Creates a clone of the polygon part including node links
VectorPolygon::VectorPolygon(VectorPart * const ClonePart, bool AddLinkBack, VectorPolygon *PartOwner)
{
VectorPart *CurPart;
bool Success = true;

#ifdef _DEBUG
++NumberOfPolygons;
#endif // _DEBUG

MyEffects = NULL;
TotalNumNodes = 0;
BBox = NULL;
CloneOfThis = NULL;
Normals[3] = Normals[2] = Normals[1] = Normals[0] = 0.0f;
PolyNumber = 0;
ImAnOriginalPolygon = 0;
CurPart = GetFirstPart();

if (! CopyNodes(ClonePart, AddLinkBack, PartOwner))
	Success = false;

// if there was a failure remove all the nodes
if (! Success)
	{
	RemoveNodes();
	} // if

} // VectorPolygon::VectorPolygon

/*===========================================================================*/

VectorPolygon::~VectorPolygon()
{

RemoveNodes();
if (BBox)
	delete BBox;
RemoveEffects();

} // VectorPolygon::~VectorPolygon

/*===========================================================================*/

void VectorPolygon::RemoveNodes(void)
{
VectorPart *CurPart;

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	CurPart->RemoveNodes();
	} // for
TotalNumNodes = 0;
while (CurPart = GetFirstPart()->NextPart)
	{
	GetFirstPart()->NextPart = CurPart->NextPart;
	delete CurPart;
	} // for

} // VectorPolygon::RemoveNodes

/*===========================================================================*/

VectorNode *VectorPolygon::CopyNodes(VectorPolygon *SourceToCopy, bool AddLinkBack)
{
unsigned long NodeCt;
VectorNode *CurNode, *CopiedNode, **NodePtr;
VectorNodeLink *CopyFromLink, **CopyToLink;
VectorPart *SourcePart, *DestPart;
bool Success = true;

TotalNumNodes = 0;

if (SourceToCopy->PolyFirstNode())
	{
	for (SourcePart = SourceToCopy->GetFirstPart(), DestPart = GetFirstPart(); SourcePart && DestPart && Success; SourcePart = SourcePart->NextPart)
		{ 
		NodePtr = &DestPart->MyNodes;
		for (CurNode = SourcePart->FirstNode(), NodeCt = 0; NodeCt < SourcePart->NumNodes && Success; ++NodeCt, CurNode = CurNode->NextNode)
			{
			if (*NodePtr = new VectorNode(CurNode))
				{
				++DestPart->NumNodes;
				CopiedNode = *NodePtr;
				NodePtr = &(*NodePtr)->NextNode;
				// copy links
				CopyToLink = &CopiedNode->LinkedNodes;
				for (CopyFromLink = CurNode->LinkedNodes; CopyFromLink; CopyFromLink = (VectorNodeLink *)CopyFromLink->NextNodeList)
					{
					if (AddLinkBack)
						{
						if (! (Success = CopiedNode->AddCrossLinks(CopyFromLink->MyNode, this, CopyFromLink->LinkedPolygon)))
							break;
						} // if
					else
						{
						if (! (*CopyToLink = new VectorNodeLink(CopyFromLink->MyNode, CopyFromLink->LinkedPolygon, 0)))
						//if (! (*CopyToLink = new VectorNodeLink(CopyFromLink->MyNode, CopyFromLink->LinkedPolygon, (1 << 7))))
							{
							Success = false;
							break;
							} // else
						CopyToLink = (VectorNodeLink **)&(*CopyToLink)->NextNodeList;
						} // else
					} // for
				} // if
			else
				{
				Success = false;
				break;
				} // else
			} // for
		TotalNumNodes += DestPart->NumNodes;
		// link last node to origin
		if (Success)
			{
			*NodePtr = DestPart->FirstNode();
			// if there is another part, replicate it
			if (SourcePart->NextPart)
				{
				if (DestPart->NextPart = new VectorPart())
					{
					DestPart = DestPart->NextPart;
					} // if
				else
					{
					Success = false;
					break;
					}
				} // if
			} // if
		} // for
	} // if

return (Success ? GetFirstPart()->MyNodes: NULL);

} // VectorPolygon::CopyNodes

/*===========================================================================*/

VectorNode *VectorPolygon::CopyNodes(VectorPart *PartToCopy, bool CopyLinks, VectorPolygon *PartOwner)
{
unsigned long NodeCt;
VectorNode *CurNode, *CopiedNode, **NodePtr;
VectorNodeLink *CopyFromLink, **CopyToLink;
VectorPart *DestPart;
bool Success = true;

TotalNumNodes = 0;

if (PartToCopy->FirstNode())
	{
	DestPart = GetFirstPart();
	NodePtr = &DestPart->MyNodes;
	for (CurNode = PartToCopy->FirstNode(), NodeCt = 0; NodeCt < PartToCopy->NumNodes && Success; ++NodeCt, CurNode = CurNode->NextNode)
		{
		if (*NodePtr = new VectorNode(CurNode))
			{
			++DestPart->NumNodes;
			CopiedNode = *NodePtr;
			NodePtr = &(*NodePtr)->NextNode;
			// copy links
			CopyToLink = &CopiedNode->LinkedNodes;
			for (CopyFromLink = CurNode->LinkedNodes; CopyFromLink; CopyFromLink = (VectorNodeLink *)CopyFromLink->NextNodeList)
				{
				if (CopyLinks)
					{
					if (! (Success = CopiedNode->AddCrossLinks(CopyFromLink->MyNode, this, CopyFromLink->LinkedPolygon)))
						break;
					} // if
				else
					{
					if (! (*CopyToLink = new VectorNodeLink(CopyFromLink->MyNode, CopyFromLink->LinkedPolygon, 0)))
					//if (! (*CopyToLink = new VectorNodeLink(CopyFromLink->MyNode, CopyFromLink->LinkedPolygon, (1 << 7))))
						{
						Success = false;
						break;
						} // else
					CopyToLink = (VectorNodeLink **)&(*CopyToLink)->NextNodeList;
					} // else
				} // for
			} // if
		else
			{
			Success = false;
			break;
			} // else
		} // for
	TotalNumNodes = DestPart->NumNodes;
	// link last node to origin
	if (Success)
		{
		*NodePtr = DestPart->FirstNode();
		} // if
	} // if

return (Success ? GetFirstPart()->MyNodes: NULL);

} // VectorPolygon::CopyNodes

/*===========================================================================*/

VectorPart *VectorPolygon::ClonePart(VectorPart *CloneMe)
{
unsigned long NodeCt;
VectorPart *NewPart = NULL, *CurPart;
VectorNode *CurNode, **NodePtr;
bool Success = true;

for (CurPart = GetFirstPart(); CurPart->NextPart; CurPart = CurPart->NextPart)
	{
	} // for

if (CurPart->NextPart = new VectorPart())
	{
	NewPart = CurPart->NextPart;
	NodePtr = &NewPart->MyNodes;
	for (CurNode = CloneMe->FirstNode(), NodeCt = 0; NodeCt < CloneMe->NumNodes && Success; ++NodeCt, CurNode = CurNode->NextNode)
		{
		if (*NodePtr = new VectorNode(CurNode))
			{
			++NewPart->NumNodes;
			NodePtr = &(*NodePtr)->NextNode;
			} // if
		else
			{
			Success = false;
			break;
			} // else
		} // for
	TotalNumNodes += NewPart->NumNodes;
	// link last node to origin
	if (Success)
		*NodePtr = NewPart->FirstNode();
	else
		NewPart = NULL;
	} // if

return (NewPart);
	
} // VectorPolygon::ClonePart

/*===========================================================================*/

unsigned long VectorPolygon::CountNodes(void)
{
VectorPart *CurPart;
unsigned long NodeCt = 0;

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	NodeCt += CurPart->CountNodes();
	} // for

return (NodeCt);

} // VectorPolygon::CountNodes

/*===========================================================================*/

unsigned long VectorPolygon::CountParts(void)
{
VectorPart *CurPart;
unsigned long PartCt = 0;

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	++PartCt;
	} // for

return (PartCt);

} // VectorPolygon::CountParts

/*===========================================================================*/

VectorNode *VectorPolygon::PolyFirstNode(void)
{

return (GetFirstPart()->FirstNode());

} // VectorPolygon::PolyFirstNode

/*===========================================================================*/

VectorNode *VectorPolygon::PolySecondNode(void)
{

return (GetFirstPart()->SecondNode());

} // VectorPolygon::PolySecondNode

/*===========================================================================*/

VectorNode *VectorPolygon::PolyThirdNode(void)
{

return (GetFirstPart()->ThirdNode());

} // VectorPolygon::PolyThirdNode

/*===========================================================================*/

VectorNode *VectorPolygon::PolyNextNode(VectorNode *CurNode)
{
VectorNode *NextNode;
VectorPart *CurPart;

if (CurNode)
	{
	NextNode = CurNode->NextNode;
	for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
		{
		if (NextNode == CurPart->MyNodes)
			{
			// reached last node in current part
			if (CurPart->NextPart)
				{
				return (CurPart->NextPart->FirstNode());
				} // if
			else
				{
				return (NULL);
				} // else
			} // if
		} // for
	return (NextNode);
	} // if
else
	return (PolyFirstNode());

} // VectorPolygon::PolyNextNode

/*===========================================================================*/

Joe *VectorPolygon::GetFirstVector(void)
{

return (MyEffects ? MyEffects->MyJoe: NULL);

} // VectorPolygon::GetFirstVector

/*===========================================================================*/

VectorNode *VectorPolygon::InsertNode(VectorPart *CurPart, VectorNode *InsertAfter, VectorNode *CloneNode)
{
VectorNode *NewNode;

if (InsertAfter->SamePointLatLon(CloneNode))
	return (InsertAfter);
if (InsertAfter->NextNode->SamePointLatLon(CloneNode))
	return (InsertAfter->NextNode);
if (NewNode = new VectorNode(CloneNode))
	{
	NewNode->NextNode = InsertAfter->NextNode;
	InsertAfter->NextNode->PrevNode = NewNode;
	InsertAfter->NextNode = NewNode;
	NewNode->PrevNode = InsertAfter;
	++CurPart->NumNodes;
	++TotalNumNodes;
	} // if

return (NewNode);

} // VectorPolygon::InsertNode

/*===========================================================================*/

VectorNode *VectorPolygon::InsertNode(VectorPart *CurPart, VectorNode *InsertAfter, double NewLat, double NewLon, double NewElev)
{
VectorNode *NewNode;

if (NewNode = new VectorNode())
	{
	NewNode->Lat = NewLat;
	NewNode->Lon = NewLon;
	NewNode->Elev = NewElev;

	if (InsertAfter->SamePointLatLon(NewNode))
		{
		delete NewNode;
		return (InsertAfter);
		} // if
	if (InsertAfter->NextNode->SamePointLatLon(NewNode))
		{
		delete NewNode;
		return (InsertAfter->NextNode);
		} // if
	
	NewNode->NextNode = InsertAfter->NextNode;
	InsertAfter->NextNode->PrevNode = NewNode;
	InsertAfter->NextNode = NewNode;
	NewNode->PrevNode = InsertAfter;
	++CurPart->NumNodes;
	++TotalNumNodes;
	} // if

return (NewNode);

} // VectorPolygon::InsertNode

/******************************************************************************/

bool VectorPolygon::InsertNodeAddLinks(VectorNode *InsertAfter, VectorNode *CloneNode, VectorPolygon *CloneNodeOwner)
{
unsigned long NodeCt;
VectorPart *CurPart;
VectorNode *CurNode, *LinkedNode;
VectorNodeLink *ListOfLinkedNodes = NULL, **NodeListPtr, *AbeLink, *MissingLink;
bool FoundPart, Success = true;

CurPart = GetFirstPart();
if (CurPart->NextPart)
	{
	// more than one part so must find the part that InsertAfter belongs to
	FoundPart = false;
	for (; CurPart; CurPart = CurPart->NextPart)
		{
		for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; ++ NodeCt, CurNode = CurNode->NextNode)
			{
			if (CurNode == InsertAfter)
				{
				FoundPart = true;
				break;
				} // if
			} // for
		if (FoundPart)
			break;
		} // for
	if (! FoundPart)
		return (NULL);
	} // if

// now we know which part to insert into
if (CurNode = InsertNode(CurPart, InsertAfter, CloneNode))
	{
	NodeListPtr = &ListOfLinkedNodes;
	// build a list of all the nodes CloneNode is linked to
	// there will be at least one member of the list: CloneNode
	CloneNode->MakeLinkedNodeList(ListOfLinkedNodes, NodeListPtr, CloneNodeOwner);
	if (ListOfLinkedNodes)
		{
		for (AbeLink = (VectorNodeLink *)ListOfLinkedNodes; AbeLink; AbeLink = MissingLink)
			{
			LinkedNode = AbeLink->MyNode;
			if (! (Success = CurNode->AddCrossLinks(LinkedNode, this, AbeLink->LinkedPolygon)))
				break;
			MissingLink = (VectorNodeLink *)AbeLink->NextNodeList;
			delete AbeLink;
			} // for
		} // if
	else
		Success = false;
	} // if
else
	Success = false;

return (Success);
	
} // VectorPolygon::InsertNodeAddLinks

/******************************************************************************/

void VectorPolygon::LinkBackwards(void)
{
unsigned long NodeCt;
VectorNode *CurNode, *LastNode;
VectorPart *CurPart;

// F2_NOTE: vvv CPI = 7
for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	LastNode = CurPart->FirstNode();
	// F2_NOTE: vvv CPI ~= 2.9
	for (NodeCt = 0, CurNode = CurPart->SecondNode(); CurNode && NodeCt < CurPart->NumNodes; ++NodeCt, CurNode = CurNode->NextNode)
		{
		// F2_NOTE: vvv CPI ~= 9.2
		CurNode->PrevNode = LastNode;
		// F2_NOTE: vvv CPI ~= 4.3
		LastNode = CurNode;
		} // for
	} // for

} // VectorPolygon::LinkBackwards

/*===========================================================================*/

VectorPart *VectorPolygon::AddPart(VectorPart *AddMe)
{
VectorPart *CurPart;

for (CurPart = GetFirstPart(); CurPart && CurPart->NextPart; CurPart = CurPart->NextPart)
	{
	} // for
if (CurPart)
	{
	CurPart->NextPart = AddMe;
	TotalNumNodes += AddMe->NumNodes;
	} // if

return (AddMe);

} // VectorPolygon::AddPart

/*===========================================================================*/

bool VectorPolygon::CrossLink(VectorPolygon *LinkMe)
{
VectorPart *PartyOfTheFirstPart, *PartyOfTheSecondPart;
bool Success = true;

if (TotalNumNodes == LinkMe->TotalNumNodes)
	{
	for (PartyOfTheFirstPart = GetFirstPart(), PartyOfTheSecondPart = LinkMe->GetFirstPart(); 
		PartyOfTheFirstPart && PartyOfTheSecondPart && Success; PartyOfTheFirstPart = PartyOfTheFirstPart->NextPart, PartyOfTheSecondPart = PartyOfTheSecondPart->NextPart)
		{
		if (PartyOfTheFirstPart->NumNodes == PartyOfTheSecondPart->NumNodes)
			{
			if (! PartyOfTheFirstPart->CrossLink(PartyOfTheSecondPart, this, LinkMe))
				{
				Success = false;
				break;
				} // if
			} // if
		} // for
	} // if
	
return (Success);

} // VectorPolygon::CrossLink

/*===========================================================================*/

EffectJoeList *VectorPolygon::SetSimpleEffect(GeneralEffect *NewEffect, Joe *NewJoe)
{
EffectJoeList **GEPtr;

if (NewEffect && NewJoe)
	{
	for (GEPtr = &MyEffects; *GEPtr; GEPtr = &(*GEPtr)->Next)
		{
		if ((*GEPtr)->GetEffect() == NewEffect && (*GEPtr)->GetVector() == NewJoe)
			return (*GEPtr);
		} // for

	if (*GEPtr = new EffectJoeList(NewEffect, NewJoe))
		{
		return (*GEPtr);
		} // if
	} // if

// memory error or NULL effect source
return (NULL);

} // VectorPolygon::SetSimpleEffect

/*===========================================================================*/

EffectJoeList *VectorPolygon::SetEffect(EffectJoeList *EffectSource)
{
EffectJoeList **GEPtr;
bool SameEffectAndVector;

if (EffectSource)
	{
	for (GEPtr = &MyEffects; *GEPtr; GEPtr = &(*GEPtr)->Next)
		{
		SameEffectAndVector = false;
		if ((*GEPtr)->Identical(EffectSource, SameEffectAndVector))
			return (*GEPtr);
		if (SameEffectAndVector && EffectSource->GetTfxDetail())
			{
			if ((*GEPtr)->SupplementTfxDetails(EffectSource))
				return (*GEPtr);
			return (NULL);
			} // if
		} // for

	if (*GEPtr = new EffectJoeList(EffectSource))
		{
		return (*GEPtr);
		} // if
	} // if

return (NULL);

} // VectorPolygon::SetEffect

/*===========================================================================*/

bool VectorPolygon::TestForEffect(GeneralEffect *TestEffect)
{
EffectJoeList *CurEffect;

for (CurEffect = MyEffects; CurEffect; CurEffect = CurEffect->Next)
	{
	if (CurEffect->MyEffect == TestEffect)
		return (true);
	} // for

return (false);

} // VectorPolygon::TestForEffect

/*===========================================================================*/

bool VectorPolygon::SupplementEffects(VectorPolygon *SourcePoly)
{
EffectJoeList *CurEffect;

for (CurEffect = SourcePoly->MyEffects; CurEffect; CurEffect = CurEffect->Next)
	{
	if (! SetEffect(CurEffect))
		return (false);
	} // for

return (true);

} // VectorPolygon::SupplementEffects

/*===========================================================================*/

void VectorPolygon::RemoveEffects(void)
{

for (EffectJoeList *CurEffect = MyEffects; MyEffects; CurEffect = MyEffects)
	{
	MyEffects = MyEffects->Next;
	delete CurEffect;
	} // for

} // VectorPolygon::RemoveEffects

/*===========================================================================*/

void VectorPolygon::RemoveEffect(GeneralEffect *RemoveMe)
{
EffectJoeList *PrevEffect = NULL;

for (EffectJoeList *CurEffect = MyEffects; CurEffect; CurEffect = CurEffect ? CurEffect->Next: MyEffects)
	{
	if (CurEffect->MyEffect == RemoveMe)
		{
		if (PrevEffect)
			PrevEffect->Next = CurEffect->Next;
		else
			MyEffects = CurEffect->Next;
		delete CurEffect;
		CurEffect = PrevEffect;
		} // if
	PrevEffect = CurEffect;
	} // for

} // VectorPolygon::RemoveEffect

/*===========================================================================*/

void VectorPolygon::RemoveTerrainEffectLinks(GeneralEffect *TerrainEffect)
{
unsigned long NodeCt, NodesToTest;
VectorNode *CurNode, *PrevNode;
VectorPart *CurPart;
VectorNodeLink *LinkedNode, *PrevLink;
bool TerrainLinksExist;

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	PrevNode = NULL;
	NodesToTest = CurPart->NumNodes;
	for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < NodesToTest; ++NodeCt, CurNode = CurNode ? CurNode->NextNode: CurPart->FirstNode())
		{
		// F2_NOTE: vvv CPI ~= 2.7
		if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_ORIGINALVECTORNODE))
			{
			// remove node and any links to other polygon nodes
			if (PrevNode)
				{
				PrevNode->NextNode = CurNode->NextNode;
				PrevNode->NextNode->PrevNode = PrevNode;
				} // if
			else
				{
				CurPart->MyNodes = CurNode->NextNode;
				if (CurNode->PrevNode)
					CurNode->PrevNode->NextNode = CurPart->MyNodes;
				CurPart->MyNodes->PrevNode = CurNode->PrevNode;
				} // else
			delete CurNode;
			--CurPart->NumNodes;
			--TotalNumNodes;
			CurNode = PrevNode;
			} // if
		else
			{
			PrevLink = NULL;
			TerrainLinksExist = false;
			// if the node is linked to a terrain polygon, dissolve the link
			// if the node is linked ONLY to a terrain polygon delete the node
			// F2_NOTE: vvv CPI ~= 1.7
			for (LinkedNode = CurNode->LinkedNodes; LinkedNode; LinkedNode = LinkedNode ? (VectorNodeLink *)LinkedNode->NextNodeList: CurNode->LinkedNodes)
				{
				// F2_NOTE: vvv CPI ~= 2.8
				if (! LinkedNode->LinkedPolygon->ImAnOriginalPolygon)	// && LinkedNode->LinkedPolygon->TestForEffect(TerrainEffect))
					{
					TerrainLinksExist = true;
					// delete the terrain link and its cross link from the terrain polygon
					if (PrevLink)
						PrevLink->NextNodeList = LinkedNode->NextNodeList;
					else
						CurNode->LinkedNodes = (VectorNodeLink *)LinkedNode->NextNodeList;
					LinkedNode->MyNode->RemoveLink(CurNode);
					delete LinkedNode;
					LinkedNode = PrevLink;
					} // if
				PrevLink = LinkedNode;
				} // for
			// if the node was linked to terrain but now has no links at all, clear the shared node flag
			if (TerrainLinksExist && ! CurNode->LinkedNodes)
				CurNode->FlagClear(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT);
			} // else
		PrevNode = CurNode;
		} // for
	} // for

} // VectorPolygon::RemoveTerrainEffectLinks

/*===========================================================================*/

TfxDetail *VectorPolygon::SetTfxDetail(GeneralEffect *MatchEffect, Joe *MatchVector, 
	unsigned long SetIndex, short SetSegment, unsigned short SetFlags)
{
EffectJoeList *EJL;

if (EJL = SetSimpleEffect(MatchEffect, MatchVector))
	{
	return (EJL->SetTfxDetail(SetIndex, SetSegment, SetFlags));
	} // if

return (NULL);

} // VectorPolygon::SetTfxDetail

/*===========================================================================*/

PolygonBoundingBox *VectorPolygon::SetBoundingBox(void)
{

if (BBox)
	BBox->UpdateBounds(this);
else
	BBox = new PolygonBoundingBox(this);

return (BBox);

} // VectorPolygon::SetBoundingBox

/*===========================================================================*/

double VectorPolygon::PolygonArea(void)
{
double LastY, LastX, OriginY, Area = 0.0;
VectorNode *CurNode;
VectorPart *CurPart;

// accumulate each part starting at its origin
// opposite ordered parts will subtract area
for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	CurNode = CurPart->FirstNode();
	OriginY = CurNode->Lat;
	LastY = CurNode->Lat;
	LastX = CurNode->Lon;
	for (CurNode = CurPart->SecondNode(); CurNode; CurNode = CurNode->NextNode)
		{
		Area += ((CurNode->Lat - LastY) * (CurNode->Lon - LastX) * .5 
			+ (LastY - OriginY) * (CurNode->Lon - LastX));
		// check if loop finished by coming back to origin
		if (CurNode == CurPart->MyNodes)
			break;
		LastY = CurNode->Lat;
		LastX = CurNode->Lon;
		} // for
	} // for
// because polygons are in our bass-ackwards notion of longitude
if (Area != 0.0)
	Area = -Area;

return (Area);

} // VectorPolygon::PolygonArea

/*===========================================================================*/

double VectorPolygon::PolygonAreaOutsidePart(void)
{
double LastY, LastX, OriginY, Area = 0.0;
VectorNode *CurNode;
VectorPart *CurPart;

CurPart = GetFirstPart();
CurNode = CurPart->FirstNode();
OriginY = CurNode->Lat;
LastY = CurNode->Lat;
LastX = CurNode->Lon;
for (CurNode = CurPart->SecondNode(); CurNode; CurNode = CurNode->NextNode)
	{
	Area += ((CurNode->Lat - LastY) * (CurNode->Lon - LastX) * .5 
		+ (LastY - OriginY) * (CurNode->Lon - LastX));
	// check if loop finished by coming back to origin
	if (CurNode == CurPart->MyNodes)
		break;
	LastY = CurNode->Lat;
	LastX = CurNode->Lon;
	} // for

// because polygons are in our bass-ackwards notion of longitude
if (Area != 0.0)
	Area = -Area;

return (Area);

} // VectorPolygon::PolygonAreaOutsidePart

/*===========================================================================*/

double VectorPolygon::FindNodeListArea(VectorNodeList *NodeList)
{
double LastY, LastX, OriginY, Area = 0.0;
VectorNode *CurNode;
VectorNodeList *CurList;

CurNode = NodeList->MyNode;
OriginY = CurNode->Lat;
LastY = CurNode->Lat;
LastX = CurNode->Lon;
CurList = NodeList->NextNodeList;
for (CurList = NodeList->NextNodeList; CurList; CurList = CurList->NextNodeList)
	{
	CurNode = CurList->MyNode;
	Area += ((CurNode->Lat - LastY) * (CurNode->Lon - LastX) * .5 
		+ (LastY - OriginY) * (CurNode->Lon - LastX));
	// check if loop finished by coming back to origin
	LastY = CurNode->Lat;
	LastX = CurNode->Lon;
	} // for
// connect back to origin if the list isn't self-closing
if (CurNode != NodeList->MyNode)
	{
	CurNode = NodeList->MyNode;
	Area += ((CurNode->Lat - LastY) * (CurNode->Lon - LastX) * .5 
		+ (LastY - OriginY) * (CurNode->Lon - LastX));
	} // if
// because polygons are in our bass-ackwards notion of longitude
if (Area != 0.0)
	Area = -Area;

return (Area);

} // VectorPolygon::FindNodeListArea

/*===========================================================================*/

bool VectorPolygon::ReverseDirection(void)
{
VectorNode **NodePtrArray, *CurNode;
VectorPart *CurPart;
unsigned long NodeCtForward, NodeCtReverse;
bool Success = true;

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	if (CurPart->NumNodes > 0 && (NodePtrArray = new VectorNode *[CurPart->NumNodes]))
		{
		for (NodeCtForward = 0, CurNode = CurPart->FirstNode(); NodeCtForward < CurPart->NumNodes; ++NodeCtForward, CurNode = CurNode->NextNode)
			{
			NodePtrArray[NodeCtForward] = CurNode;
			} // for
		// keep same first node and link in reverse order, this should bring us back to the origin again which is node 0
		for (NodeCtForward = 0, NodeCtReverse = CurPart->NumNodes - 1, CurNode = CurPart->FirstNode(); NodeCtForward < CurPart->NumNodes; ++NodeCtForward, --NodeCtReverse, CurNode = CurNode->NextNode)
			{
			CurNode->NextNode = NodePtrArray[NodeCtReverse];
			} // for
		delete [] NodePtrArray;
		} // if
	else
		{
		Success = false;
		break;
		} // else
	} // for
	
return (Success);

} // VectorPolygon::ReverseDirection

/*===========================================================================*/

void VectorPolygon::ClearIntersectionFlags(void)
{
VectorNode *CurNode;
VectorPart *CurPart;
unsigned long NodeCt;

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; ++NodeCt, CurNode = CurNode->NextNode)
		{
		#ifdef MAKENOCLONES
		// F2_NOTE: vvv CPI ~= 3.35
		CurNode->FlagClear(WCS_VECTORNODE_FLAG_DIRECTIONFLAGS);
		#else // MAKENOCLONES
		CurNode->FlagClear(WCS_VECTORNODE_FLAG_INTERSECTFLAGS);
		#endif // MAKENOCLONES
		} // for
	} // for
	
} // VectorPolygon::ClearIntersectionFlags

/*===========================================================================*/

bool VectorPolygon::ConvertToDefGeo(void)
{
bool Success = true;
#ifdef WCS_BUILD_VNS
unsigned long NodeCt;
VectorNode *CurNode;
VectorPart *CurPart;
CoordSys *MyCoords;
VertexDEM VDEM;

if (MyCoords = GetFirstVector()->GetCoordSys())
	{
	for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
		{
		for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; ++NodeCt, CurNode = CurNode->NextNode)
			{
			// translate projected coords to global cartesian and then to default degrees
			VDEM.xyz[0] = CurNode->Lon;
			VDEM.xyz[1] = CurNode->Lat;
			VDEM.xyz[2] = CurNode->Elev;
			if (MyCoords->ProjToDefDeg(&VDEM))
				{
				CurNode->Lon = VDEM.Lon;
				CurNode->Lat = VDEM.Lat;
				CurNode->Elev = VDEM.Elev;
				} // if
			else
				{
				Success = false;
				break;
				} // else
			} // for
		} // for
	} // if
#endif // WCS_BUILD_VNS

return (Success);

} // VectorPolygon::ConvertToDefGeo

/*===========================================================================*/

void VectorPolygon::RemoveVestigialSegments(void)
{
VectorNode *CurNode, *LastNode;
VectorPart *CurPart, *LastPart = NULL;
unsigned long NodeCt;
bool VestigialRemoved;

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	VestigialRemoved = true;
	while (VestigialRemoved)
		{
		// since only one vestigial segment can be removed per loop, the loop needs to repeat when
		// a vestigial segment is removed in case there is a string of them
		VestigialRemoved = false;
		for (NodeCt = 0, LastNode = CurPart->FirstNode(), CurNode = CurPart->SecondNode(); NodeCt <= CurPart->NumNodes && CurPart->NumNodes > 2; ++NodeCt, CurNode = CurNode->NextNode)
			{
			if (LastNode->SamePointLatLon(CurNode))
				{
				if (CurNode->FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE))
					LastNode->FlagSet(WCS_VECTORNODE_FLAG_VECTORNODE);
				if (CurNode->FlagCheck(WCS_VECTORNODE_FLAG_ORIGINALVECTORNODE))
					LastNode->FlagSet(WCS_VECTORNODE_FLAG_ORIGINALVECTORNODE);
				LastNode->NextNode = CurNode->NextNode;
				if (CurNode == CurPart->MyNodes)
					CurPart->MyNodes = LastNode;
				delete CurNode;
				CurNode = LastNode;
				--CurPart->NumNodes;
				--TotalNumNodes;
				--NodeCt;
				} // if
			else if (LastNode->SamePointLatLon(CurNode->NextNode))
				{
				if (CurNode->FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE))
					LastNode->FlagSet(WCS_VECTORNODE_FLAG_VECTORNODE);
				if (CurNode->FlagCheck(WCS_VECTORNODE_FLAG_ORIGINALVECTORNODE))
					LastNode->FlagSet(WCS_VECTORNODE_FLAG_ORIGINALVECTORNODE);
				VestigialRemoved = true;
				LastNode->NextNode = CurNode->NextNode;
				if (CurNode == CurPart->MyNodes)
					CurPart->MyNodes = LastNode;
				delete CurNode;
				CurNode = LastNode;
				--CurPart->NumNodes;
				--TotalNumNodes;
				--NodeCt;
				// do not break from the for loop because the next run will remove the duplicate vertex that now exists
				// if the duplicate is not removed then the next run through both loops will remove the duplicate but
				// not detect the vestigial segment if there are two vestigials in a row in the original polygon
				} // if
			LastNode = CurNode;
			} // for
		} // while
	if (CurPart->NumNodes < 3)
		{
		if (LastPart)
			{
			LastPart->NextPart = CurPart->NextPart;
			TotalNumNodes -= CurPart->NumNodes;
			delete CurPart;
			CurPart = LastPart;
			} // if
		else 
			{
			//it must be the first part
			// this will trigger the destruction of the whole polygon by the calling function
			TotalNumNodes = 1;
			break;
			} // else
		} // if
	LastPart = CurPart;
	} // for
	
} // VectorPolygon::RemoveVestigialSegments

/*===========================================================================*/

void VectorPolygon::RemoveNode(VectorNode *RemoveMe)
{
VectorNode *CurNode, *LastNode;
VectorPart *CurPart;
unsigned long NodeCt;
bool Done = false;

for (CurPart = GetFirstPart(); CurPart && ! Done; CurPart = CurPart->NextPart)
	{
	for (NodeCt = 0, LastNode = CurPart->FirstNode(), CurNode = CurPart->SecondNode(); NodeCt <= CurPart->NumNodes && CurPart->NumNodes > 2; ++NodeCt, CurNode = CurNode->NextNode)
		{
		if (CurNode == RemoveMe)
			{
			LastNode->NextNode = CurNode->NextNode;
			if (CurNode == CurPart->MyNodes)
				CurPart->MyNodes = LastNode;
			LastNode->NextNode->PrevNode = LastNode;
			delete CurNode;
			CurNode = LastNode;
			--CurPart->NumNodes;
			--TotalNumNodes;
			--NodeCt;
			Done = true;
			break;
			} // if
		LastNode = CurNode;
		} // for
	} // for
	
} // VectorPolygon::RemoveNode

/*===========================================================================*/

// requires that the polygon be in geographic coords and the direction be generally clockwise. 
// Counter CW segments will be considered anomalous.
bool VectorPolygon::ResolveSelfIntersections(bool &LinksAdded)
{
unsigned long CurSegCt, TestSegCt, CurNumNodes, NodesAddedOnCurSeg;
VectorNode *CurNode, *TestNode, *StashNextNode, *AdvanceNode;
VectorPart *CurPart, *TestPart;
bool Success = true, ContactFound;
VectorIntersecter VI;

ClearIntersectionFlags();
LinksAdded = false;

// look for intersecting segments or nodes that are the same
for (CurPart = GetFirstPart(); CurPart && Success; CurPart = CurPart->NextPart)
	{
	for (CurSegCt = 0, CurNode = CurPart->FirstNode(); CurSegCt < CurPart->NumNodes && Success; ++CurSegCt, CurNode = CurNode->NextNode)
		{
		for (TestPart = CurPart; TestPart; TestPart = TestPart->NextPart)
			{
			if (TestPart == CurPart)
				{
				TestSegCt = CurSegCt + 1;
				TestNode = CurNode->NextNode;
				} // if
			else
				{
				TestSegCt = 0;
				TestNode = TestPart->FirstNode();
				} // else
			for (; TestSegCt < TestPart->NumNodes && Success; ++TestSegCt, TestNode = TestNode->NextNode)
				{
				CurNumNodes = CurPart->NumNodes;
				StashNextNode = CurNode->NextNode;
				if (CurNode->SamePointLatLon(TestNode))
					{
					if (CurNode->AddCrossLinks(TestNode, this, this))
						{
						CurNode->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT);
						TestNode->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT);
						LinksAdded = true;
						} // if
					else
						Success = false;
					} // if
				else if (Success = VI.IntersectSegmentsLinkNodes(this, this, CurPart, TestPart, CurNode, TestNode, ContactFound = false))
					{
					// keep TestNodeCt in sync with the number of nodes behind it in the polygon
					// Nodes may have been added on either of the segments
					if (CurNumNodes != CurPart->NumNodes && TestPart == CurPart)
						{
						NodesAddedOnCurSeg = 0;
						for (AdvanceNode = CurNode->NextNode; AdvanceNode != StashNextNode; AdvanceNode = AdvanceNode->NextNode)
							{
							++NodesAddedOnCurSeg;
							} // for
						TestSegCt += NodesAddedOnCurSeg;
						} // if
					if (ContactFound)
						LinksAdded = true;
					} // else if
				} // for
			} // for
		} // for
	} // for
	
return (Success);

} // VectorPolygon::ResolveSelfIntersections

/*===========================================================================*/

bool VectorPolygon::CleanupPolygonRelationships(VectorPolygon *VP2)
{
unsigned long Node1Ct, Node2Ct, LinkCt1, LinkCt2;
VectorNode *Node1, *Node2, *Node1Prev, *Node2Prev;
VectorPart *VP1Part, *VP2Part;
bool ModificationsMade = false;
VectorNodeLink *CurLink;

for (VP1Part = GetFirstPart(); VP1Part; VP1Part = VP1Part->NextPart)
	{
	Node1Prev = VP1Part->FirstNode();
	for (Node1Ct = 0, Node1 = Node1Prev->NextNode; Node1Ct < VP1Part->NumNodes; ++Node1Ct, Node1 = Node1->NextNode)
		{
		for (VP2Part = VP2->GetFirstPart(); VP2Part; VP2Part = VP2Part->NextPart)
			{
			Node2Prev = VP2Part->FirstNode();
			for (Node2Ct = 0, Node2 = Node2Prev->NextNode; Node2Ct < VP2Part->NumNodes; ++Node2Ct, Node2 = Node2->NextNode)
				{
				if (Node1->SimilarPointLatLon(Node2, WCS_VECTORPOLYGON_NODECOORD_CLEANUPTOLERANCE))
					{
					if (! Node1->SamePointLatLon(Node2))
						{
						// if it would cause duplication - don't do it!
						if (! (Node1->NextNode->SamePointLatLon(Node2) || Node1Prev->SamePointLatLon(Node2)
							|| Node2->NextNode->SamePointLatLon(Node1) || Node2Prev->SamePointLatLon(Node1)))
							{
							// move the one with least links
							for (LinkCt1 = 0, CurLink = Node1->LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList, ++LinkCt1);
							for (LinkCt2 = 0, CurLink = Node2->LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList, ++LinkCt2);
							if (LinkCt2 <= LinkCt1)
								{
								Node2->Lat = Node1->Lat;
								Node2->Lon = Node1->Lon;
								for (CurLink = Node2->LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList)
									{
									CurLink->MyNode->Lat = Node2->Lat;
									CurLink->MyNode->Lon = Node2->Lon;
									} // for
								} // if
							else
								{
								Node1->Lat = Node2->Lat;
								Node1->Lon = Node2->Lon;
								for (CurLink = Node1->LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList)
									{
									CurLink->MyNode->Lat = Node1->Lat;
									CurLink->MyNode->Lon = Node1->Lon;
									} // for
								} // if
							ModificationsMade = true;
							} // if
						} // if
					} // if
				} // for
			} // for
		} // for
	} // for

// Set a flag on the vectors so they can be reported to user
//if (ModificationsMade)
//	{
//	EffectJoeList *CurEffect;
//	for (CurEffect = VP2->MyEffects; CurEffect; CurEffect = CurEffect->Next)
//		{
//		CurEffect->MyJoe->SetFlags(WCS_JOEFLAG_POINTSSHIFTED);
//		} // for
//	for (CurEffect = VP1->MyEffects; CurEffect; CurEffect = CurEffect->Next)
//		{
//		CurEffect->MyJoe->SetFlags(WCS_JOEFLAG_POINTSSHIFTED);
//		} // for
//	} // if
	
return (ModificationsMade);

} // VectorPolygon::CleanupPolygonRelationships

/*===========================================================================*/

void VectorPolygon::RemoveRightSideLists(void)
{
VectorNode *CurNode;
VectorPart *CurPart;
unsigned long NodeCt;

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; ++NodeCt, CurNode = CurNode->NextNode)
		{
		// F2_NOTE: vvv CPI ~= 5.4
		if (CurNode->RightSideList)
			{
			// F2_NOTE: vvv CPI ~= 3.7
			CurNode->RemoveRightSideList();
			} // if
		} // for
	} // for
	
} // VectorPolygon::RemoveRightSideLists

/*===========================================================================*/

bool VectorPolygon::MergeAdjoiningNodes(bool &NodeReplicationExists)
{
double NewLat, NewLon, NewElev, InvLinkCt;
VectorNode *CurNode, *LinkedNode;
VectorNodeLink *CurLink, *PrevLink;
VectorNodeLink *ListOfLinkedNodes, **NodeListPtr, *AbeLink, *MissingLink;
VectorPart *CurPart;
unsigned long NodeCt, LinkCt;
bool Success = true, UseAverage;

for (CurPart = GetFirstPart(); CurPart && Success; CurPart = CurPart->NextPart)
	{
	for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes && Success; ++NodeCt, CurNode = CurNode->NextNode)
		{
		if (CurNode->LinkedNodes)
			{
			if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT))
				{
				ListOfLinkedNodes = NULL;
				NodeListPtr = &ListOfLinkedNodes;
				CurNode->MakeLinkedNodeList(ListOfLinkedNodes, NodeListPtr, this);
				if (ListOfLinkedNodes && ListOfLinkedNodes->NextNodeList)
					{
					ListOfLinkedNodes->FlagSet(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT);
					NewLat = CurNode->Lat;
					NewLon = CurNode->Lon;
					NewElev = CurNode->Elev;
					LinkCt = 1;
					// replacement starts here 7/25/07
					UseAverage = true;
					for (AbeLink = (VectorNodeLink *)ListOfLinkedNodes->NextNodeList; AbeLink; AbeLink = (VectorNodeLink *)AbeLink->NextNodeList)
						{
						LinkedNode = AbeLink->MyNode;
						AbeLink->FlagSet(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT);
						// if the vertex isn't already shared then we can consider moving it
						if (! LinkedNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT))
							{
							if (UseAverage)
								{
								NewLat += LinkedNode->Lat;
								NewLon += LinkedNode->Lon;
								NewElev += LinkedNode->Elev;
								} // if
							++LinkCt;
							} // if
						else
							{
							if (UseAverage)
								{
								UseAverage = false;
								AbeLink->FlagSet(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT);
								NewLat = LinkedNode->Lat;
								NewLon = LinkedNode->Lon;
								NewElev = LinkedNode->Elev;
								++LinkCt;
								} // if
							else
								{
								if (CurNode->SamePointLatLon(LinkedNode))
									{
									AbeLink->FlagSet(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT);
									++LinkCt;
									} // if
								} // else
							} // else if
						} // for
					if (LinkCt > 1)
						{
						if (UseAverage)
							{
							InvLinkCt = 1.0 / LinkCt;
							NewLat *= InvLinkCt;
							NewLon *= InvLinkCt;
							NewElev *= InvLinkCt;
							} // if
					/* Dinosaurian age code replaced 7/25/07
					UseAverage = false;
					for (AbeLink = (VectorNodeLink *)ListOfLinkedNodes->NextNodeList; AbeLink; AbeLink = (VectorNodeLink *)AbeLink->NextNodeList)
						{
						LinkedNode = AbeLink->MyNode;
						// if the vertex isn't already shared then we can consider moving it
						if (! LinkedNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT))
							{
							// Don't we already know it is within tolerance since it is linked?
							if (CurNode->SimilarPointLatLon(LinkedNode, WCS_VECTORPOLYGON_NODECOORD_MERGETOLERANCE))
								{
								if (! UseAverage && ! CurNode->SamePointLatLon(LinkedNode))
									UseAverage = true;
								AbeLink->FlagSet(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT);
								NewLat += LinkedNode->Lat;
								NewLon += LinkedNode->Lon;
								NewElev += LinkedNode->Elev;
								++LinkCt;
								} // if
							} // if
						else if (! UseAverage && CurNode->SamePointLatLon(LinkedNode))
							{
							AbeLink->FlagSet(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT);
							NewLat += LinkedNode->Lat;
							NewLon += LinkedNode->Lon;
							NewElev += LinkedNode->Elev;
							++LinkCt;
							} // else if
						} // for
					if (LinkCt > 1)
						{
						if (UseAverage)
							{
							InvLinkCt = 1.0 / LinkCt;
							NewLat *= InvLinkCt;
							NewLon *= InvLinkCt;
							NewElev *= InvLinkCt;
							} // if
						else
							{
							NewLat = CurNode->Lat;
							NewLon = CurNode->Lon;
							NewElev = CurNode->Elev;
							} // else
						*/
						// check each pair of nodes and if both are being merged be sure they are linked
						// otherwise if only one is being merged remove the link to the other
						for (AbeLink = (VectorNodeLink *)ListOfLinkedNodes; AbeLink && Success; AbeLink = (VectorNodeLink *)AbeLink->NextNodeList)
							{
							LinkedNode = AbeLink->MyNode;
							PrevLink = AbeLink;
							for (VectorNodeLink *BoboLink = (VectorNodeLink *)AbeLink->NextNodeList; BoboLink; BoboLink = (VectorNodeLink *)BoboLink->NextNodeList)
								{
								if (AbeLink->FlagCheck(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT))
									{
									if (BoboLink->FlagCheck(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT))
										{
										/* this block removed 6/13/08 GRH - redundant nodes aren't necessarily bad and should be dealt with
										// by removing vestigial segments and self intersections.
										// If it proved efficacious, the polygon could be flagged as needing some more treatment
										if (AbeLink->LinkedPolygon == BoboLink->LinkedPolygon)
											{
											// the linked node will end up being the same as the current node -
											// it needs to be eliminated and all its links copied to the current node
											AbeLink->LinkedPolygon->RemoveNode(BoboLink->MyNode);
											PrevLink->NextNodeList = BoboLink->NextNodeList;
											delete BoboLink;
											BoboLink = PrevLink;
											} // if
										else if (! (Success = LinkedNode->AddCrossLinks(BoboLink->MyNode, AbeLink->LinkedPolygon, BoboLink->LinkedPolygon)))
										*/
										// these line replaces the one removed above here.
										if (AbeLink->LinkedPolygon == BoboLink->LinkedPolygon)
											NodeReplicationExists = true;
										if (! (Success = LinkedNode->AddCrossLinks(BoboLink->MyNode, AbeLink->LinkedPolygon, BoboLink->LinkedPolygon)))
											break;
										} // if
									else
										{
										LinkedNode->RemoveLink(BoboLink->MyNode);
										BoboLink->MyNode->RemoveLink(LinkedNode);
										} // else
									} // if
								else if (BoboLink->FlagCheck(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT))
									{
									LinkedNode->RemoveLink(BoboLink->MyNode);
									BoboLink->MyNode->RemoveLink(LinkedNode);
									} // else
								PrevLink = BoboLink;
								} // for
							if (AbeLink->FlagCheck(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT))
								{
								LinkedNode->Lat = NewLat;
								LinkedNode->Lon = NewLon;
								LinkedNode->Elev = NewElev;
								LinkedNode->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT);
								AbeLink->FlagClear(WCS_VECTORNODELINK_FLAG_MERGEIMMINENT);
								} // if
							} // for
						} // if
					} // if
				for (AbeLink = (VectorNodeLink *)ListOfLinkedNodes; AbeLink; AbeLink = MissingLink)
					{
					MissingLink = (VectorNodeLink *)AbeLink->NextNodeList;
					delete AbeLink;
					} // for
				} // if
			else
				{
				// has shared vertices but has already been moved so eliminate any links that are not the same position
				for (PrevLink = NULL, CurLink = CurNode->LinkedNodes; CurLink; CurLink = CurLink ? (VectorNodeLink *)CurLink->NextNodeList: CurNode->LinkedNodes)
					{
					LinkedNode = CurLink->MyNode;
					if (CurNode->SimilarPointLatLon(LinkedNode, WCS_VECTORPOLYGON_NODECOORD_MERGETOLERANCE) && ! LinkedNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT))
						{
						LinkedNode->Lat = CurNode->Lat;
						LinkedNode->Lon = CurNode->Lon;
						LinkedNode->Elev = CurNode->Elev;
						LinkedNode->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT);
						} // if
					else if (CurNode->SamePointLatLon(LinkedNode))
						{
						LinkedNode->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT);
						} // else if
					else
						{
						// not same point so remove link status
						LinkedNode->RemoveLink(CurNode);
						if (PrevLink)
							{
							PrevLink->NextNodeList = CurLink->NextNodeList;
							delete CurLink;
							CurLink = PrevLink;
							} // if
						else
							{
							CurNode->LinkedNodes = (VectorNodeLink *)CurLink->NextNodeList;
							delete CurLink;
							CurLink = NULL;
							} // else
						} // else
					PrevLink = CurLink;
					} // for
				} // else
			} // if
		} // for
	} // for
	
return (Success);

} // VectorPolygon::MergeAdjoiningNodes

/*===========================================================================*/

VectorPolygonListDouble *VectorPolygon::Triangulate(bool &Success)
{
double MinimumDistanceSquared, BestNodeDistanceSquared, TestNodeDistanceSquared,
	BestAngle, TestAngle, SideAngle, IntersectionLat, IntersectionLon, DotResult;
unsigned long NodeCt, NumEdgesToTest, NumEdgesTested, MaxTestsBeforeBailout, ContiguousNumNodes, BestNodeHits;
VectorPolygonListDouble *NewVPList = NULL, **VPListPtr;
VectorNode *CurNode, *Pt1, *Pt2, *Pt3, *DelPt, *FoundBestNode, *TestNode, *LastNode, **ContiguousNodeList = NULL;
VectorPart *CurPart, *CurTestPart, *FoundBestPart, *OuterPart;
PolygonEdgeList *EdgeList = NULL, **EdgeListPtr, *BestEdge, *CurEdge, *FoundBestLeadingEdge, 
	*FoundBestTrailingEdge, *EdgeToTest;
bool UseRight, SetLeadingEdge, PartDone;
TriangleBoundingBoxVector CP1, RightSide, LeftSide, TestSide, TBxFrom, TBxTo, AngleFinder;
VectorIntersecter VI;
TriangleBoundingBox TriBounds;

#ifdef DEBUG_POLYGONS_TO_VECTOR
long EdgeCt;
char DbgStr[256];
#endif // DEBUG_POLYGONS_TO_VECTOR

EdgeListPtr = &EdgeList;
VPListPtr = &NewVPList;

// break a polygon down into a set of triangles that do not overlap and cover only the area
// covered by the original polygon. Try to optimize the triangles so they are not slivers along the
// original polygon edges.

// first order of business: determine the depth of the problem.
// If there are only four vertices, find the two closest together and link them as the middle edge
// If there is more than one part, find the closest points between the inner parts and each other and the outer
// polygon and link all the parts and outside together with a pair of edges (coincident).
// Build a set of edge entities that connect all the edges with nodes in an edge-node topology that
// can be followed from one point through all the ponts in the polygon including any inside parts.
// Analyze the closest node to each edge that forms a positive triangle whose edges are not intersected by any other
// edge.

if (TotalNumNodes > 3)
	{
	if (TotalNumNodes == 4)
		{
		if (*VPListPtr = new VectorPolygonListDouble())
			{
			CurPart = GetFirstPart();
			CurNode = CurPart->FirstNode();
			// find the closest pair of opposite nodes
			// test to see if they form a positive triangle if joined.
			// if not, use the other set of opposing nodes to join
			RightSide.SetXY(CurNode->NextNode->NextNode, CurNode);
			LeftSide.SetXY(CurNode->NextNode->NextNode->NextNode, CurNode->NextNode);
			if (RightSide.LengthXYSquared() < LeftSide.LengthXYSquared())
				{
				LeftSide.SetXY(CurNode->NextNode, CurNode);
				UseRight = (CP1.CrossProductXY(&LeftSide, &RightSide) > 0.0);
				}
			else
				{
				RightSide.SetXY(CurNode, CurNode->NextNode);
				UseRight = (! (CP1.CrossProductXY(&LeftSide, &RightSide) > 0.0));
				} // else
			if (UseRight)
				{
				Pt1 = CurNode;
				Pt2 = CurNode->NextNode->NextNode;
				DelPt = Pt3 = CurNode->NextNode->NextNode->NextNode;
				// dissociate fourth node and reduce totals
				Pt2->NextNode = CurNode;
				--TotalNumNodes;
				CurPart->NumNodes = TotalNumNodes;
				} // if
			else
				{
				Pt1 = CurNode->NextNode;
				DelPt = Pt2 = CurNode->NextNode->NextNode;
				Pt3 = CurNode->NextNode->NextNode->NextNode;
				// dissociate second node and reduce totals
				Pt1->NextNode = Pt3;
				--TotalNumNodes;
				CurPart->NumNodes = TotalNumNodes;
				} // else
			// form a new polygon out of the three nodes using this polygon as a prototype
			if (! (*VPListPtr)->MakeVectorPolygon(Pt1, Pt2, Pt3, this, true))
				Success = false;
			delete DelPt;
			goto FinishIt;
			} // if
		Success = false;
		goto FinishIt;
		} // else if exactly 4 nodes in only one part the solution can be quite simple
	// Add edges for each polygon segment
	for (CurPart = OuterPart = GetFirstPart(); CurPart && Success; CurPart = CurPart->NextPart)
		{
		#ifdef DEBUG_POLYGONS_TO_VECTOR
		if (PrintToVector)
			{
			strcpy(DbgStr, "Part\n"); 
			OutputDebugString(DbgStr);
			} // if
		#endif // DEBUG_POLYGONS_TO_VECTOR
		for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; ++NodeCt, CurNode = CurNode->NextNode)
			{
			#ifdef DEBUG_POLYGONS_TO_VECTOR
			if (PrintToVector)
				{
				sprintf(DbgStr, " Node %d %p %f %f\n", NodeCt, CurNode, CurNode->Lat, CurNode->Lon); 
				OutputDebugString(DbgStr);
				} // if
			#endif // DEBUG_POLYGONS_TO_VECTOR
			if (*EdgeListPtr = new PolygonEdgeList(CurNode, CurNode->NextNode, CurPart))
				{
				CurNode->AddForwardEdge(*EdgeListPtr);
				EdgeListPtr = &(*EdgeListPtr)->NextEdgeList;
				} // if
			else
				{
				Success = false;
				break;
				} // else
			} // for
		} // for
	#ifdef DEBUG_POLYGONS_TO_VECTOR
	if (PrintToVector)
		{
		strcpy(DbgStr, "End Nodes\n\n"); 
		OutputDebugString(DbgStr);
		} // if
	#endif // DEBUG_POLYGONS_TO_VECTOR
		
	if ((CurPart = GetFirstPart()) && CurPart->NextPart)
		{
		ContiguousNodeList = new VectorNode *[TotalNumNodes];
		} // if
	// The Tested flag will be used later in determining which parts to search for the best node.
	for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
		{
		// initialize Tested flag
		CurPart->Tested = false;
		} // for

	// walk the edge list and solve the triangle for each edge,
	// Add new edges at the end of the list as necessary
	#ifdef DEBUG_POLYGONS_TO_VECTOR
	EdgeCt = -1;
	#endif // DEBUG_POLYGONS_TO_VECTOR
	for (CurEdge = EdgeList, CurPart = GetFirstPart(), PartDone = false; CurEdge && Success; CurEdge = CurEdge->NextEdgeList)
		{
		#ifdef DEBUG_POLYGONS_TO_VECTOR
		++EdgeCt;
		#endif // DEBUG_POLYGONS_TO_VECTOR
		// Test if we have come to the last edge to connect in this part
		if (CurEdge->Done)
			continue;
		CurPart = CurEdge->LeadingPart;
		CurPart->Irrelevant = true;
		// Assign shorthand for the two end nodes of the current edge
		Pt1 = CurEdge->TrailingNode;
		Pt2 = CurEdge->LeadingNode;
		// We will look for the closest node to the end nodes of the current edge that is greater than
		// MinimumDistanceSquared. MinimumDistanceSquared will be increased if the closest node proves to be
		// illegal due to interference from other polygon edges.
		MinimumDistanceSquared = 0.0;
		// FoundBestNode keeps track of what is thought ot be the best node to connect to the current segment end nodes.
		// Looping will continue as long as FoundBestNode is NULL or an error causes bailout.
		FoundBestNode = NULL;
		MaxTestsBeforeBailout = TotalNumNodes * 2;

		while (Success && ! FoundBestNode)
			{
			// Leading and trailing edges refer to the segments connected in front of and behind the best node.
			FoundBestLeadingEdge = FoundBestTrailingEdge = NULL;
			FoundBestPart = NULL;
			// A boolean to tell us to set FoundBestLeadingEdge on the next node loop. 
			SetLeadingEdge = false;
			// This is the value that node distances ill be compared against to see which is closest.
			BestNodeDistanceSquared = FLT_MAX;
			BestNodeHits = 0;
			TBxFrom.SetXY(Pt2, Pt1);
			// set first two nodes in contiguous list
			if (ContiguousNodeList)
				{
				ContiguousNodeList[0] = Pt1;
				ContiguousNodeList[1] = Pt2;
				ContiguousNumNodes = 2;
				} // if
			for (CurTestPart = CurPart->NextPart; CurTestPart; CurTestPart = CurTestPart->NextPart)
				{
				// initialize Irrelevant flag
				CurTestPart->Irrelevant = false;
				} // for
				
			// Test all the edges that are joined to the current edge.
			// This may cross to other parts and if they do, those parts should be marked as tested.
			// EdgeToTest should always be true since we're going around a linked loop.
			// Test the count to make sure we aren't running around in endless circles as can happen 
			// if there is bad topology.
			for (NodeCt = 0, EdgeToTest = Pt2->ForwardList; EdgeToTest && NodeCt < MaxTestsBeforeBailout;  ++NodeCt, EdgeToTest = TestNode->ForwardList)
				{
				// When more than one edge leaves the current test node we need to test the edge that 
				// forms the smallest clockwise angle with the last edge. This is the only edge that
				// will lead around the inside of the smallest polygon that the current edge is a part of.
				if (EdgeToTest->NextEdgeThisNode)
					{
					BestEdge = NULL;
					BestAngle = 4.0;
					for (; EdgeToTest; EdgeToTest = EdgeToTest->NextEdgeThisNode)
						{
						TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode));
						if (TestAngle < BestAngle)
							{
							BestEdge = EdgeToTest;
							BestAngle = TestAngle;
							} // if
						} // for
					EdgeToTest = BestEdge;
					} // if

				// TestNode is the node we are going to analyze for its distance to the current edge.
				TestNode = EdgeToTest->LeadingNode;
				// Instructions to set the leading edge are carried over from the last test edge loop which must
				// have found a new best node.
				if (SetLeadingEdge)
					FoundBestLeadingEdge = EdgeToTest;
				SetLeadingEdge = false;
					
				// See if we've come full circle and tested each node in the current polygon part.
				if (TestNode == Pt1)
					break;

				if (ContiguousNodeList && ContiguousNumNodes < TotalNumNodes)
					{
					ContiguousNodeList[ContiguousNumNodes++] = TestNode;
					} // if
				// Parts that contain nodes that have been tested for the current edge must be linked to the
				// current edge. The fact that we have touched these parts at this stage means we can ignore
				// them when searching for nodes in subsequent parts.
				EdgeToTest->LeadingPart->Irrelevant = true;
				// Test distance to node from both end points of current edge
				RightSide.SetXY(TestNode, Pt1);
				LeftSide.SetXY(TestNode, Pt2);
				// triangle is invalid if it is 0 area or negative area
				if (CP1.CrossProductXY(&RightSide, &LeftSide) > 0.0)
					{
					TestNodeDistanceSquared = RightSide.LengthXYSquared() + LeftSide.LengthXYSquared();
					if (TestNodeDistanceSquared <= BestNodeDistanceSquared && TestNodeDistanceSquared > MinimumDistanceSquared)
						{
						// We've found a new best node or hit the same node again - it matters which.
						BestNodeDistanceSquared = TestNodeDistanceSquared;
						if (FoundBestNode == TestNode)
							++BestNodeHits;
						FoundBestNode = TestNode;
						FoundBestPart = EdgeToTest->LeadingPart;
						FoundBestTrailingEdge = EdgeToTest;
						FoundBestLeadingEdge = NULL;
						// Set the leading edge on the next edge loop iteration.
						SetLeadingEdge = true;
						} // if
					} // if
				// This is for the next loop but set it here where the current edge is known.
				TBxFrom.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode);
				} // for
			
			// Test all nodes in subsequent parts that are not joined to the current part.
			for (CurTestPart = CurPart->NextPart; CurTestPart; CurTestPart = CurTestPart->NextPart)
				{
				// Skip parts that are incorporated into the main loop by previous connections.
				if ((CurPart->Tested && CurTestPart->Tested) || CurTestPart->Irrelevant)
					continue;
				// Test one of the nodes on this part to see if it is inside the loop of edges that are contiguous 
				// with the current edge.
				if (! CurPart->Tested || TestPointContained(ContiguousNodeList, ContiguousNumNodes, CurTestPart->FirstNode(), 0.0)
					== WCS_TEST_POINT_CONTAINED_INSIDE)
					{
					// Test each node in the test part still looking for a node that is closer than any found thus far.
					for (EdgeToTest = CurTestPart->FirstNode()->ForwardList, NumEdgesTested = 0; EdgeToTest && NumEdgesTested < CurTestPart->NumNodes;  ++NumEdgesTested, EdgeToTest = EdgeToTest->NextEdgeList)
						{
						TestNode = EdgeToTest->LeadingNode;
							
						// Test distance to node from both end points of current edge
						RightSide.SetXY(TestNode, Pt1);
						LeftSide.SetXY(TestNode, Pt2);
						// triangle is invalid if it is 0 area or negative area
						if (CP1.CrossProductXY(&RightSide, &LeftSide) > 0.0)
							{
							TestNodeDistanceSquared = RightSide.LengthXYSquared() + LeftSide.LengthXYSquared();
							if (TestNodeDistanceSquared < BestNodeDistanceSquared && TestNodeDistanceSquared > MinimumDistanceSquared)
								{
								// We've found a new best node or hit the same node again - it matters which.
								BestNodeDistanceSquared = TestNodeDistanceSquared;
								if (FoundBestNode == TestNode)
									++BestNodeHits;
								FoundBestNode = TestNode;
								FoundBestPart = CurTestPart;
								// Set the leading and trailing edges to NULL since they are not going to be used
								FoundBestTrailingEdge = FoundBestLeadingEdge = NULL;
								} // if
							} // if
						} // for
					} // if contained
				else
					CurTestPart->Irrelevant = true;
				} // for
			
			// After testing all the possible nodes that could be connected to the current edge and finding
			// the closest one, we need to detrmine if there is any condition that would prevent us using that node.
			// The conditions that would prevent it would be if another edge of the polygon crosses the new connecting 
			// edges or comes right up to them which would cause a discontinuity in the resulting triangulation.
			if (FoundBestNode)
				{
				TriBounds.SetTriangle(Pt1, Pt2, FoundBestNode);
				// Use the angle that the new edge or edges make with the existing edge to test all the other 
				// nodes of the polygon. Nodes at certain angles to the original edge will be safe and nodes 
				// at other angles will violate the safe rules. To be safe, a node has to not cause the 
				// incursion of another edge into the new triangle or against the new edge(s) being formed.
				// The first test involves determining how extensive the tests need to be. They are simpler 
				// if there is only one new edge being formed - the triangle is formed from two adjacent, 
				// already existing, edges and one new one. This condition comes in two varieties: 
				// The new edge connects Pt1 with FoundBestNode or connects Pt2 with FoundBestNode.
				// If Pt1 connects to FoundBestNode (FBN):
				if (FoundBestTrailingEdge && FoundBestTrailingEdge->TrailingNode == Pt2)
					{
					// Start with the segment Pt1-FBN and set it in TBxFrom
					RightSide.SetXY(FoundBestNode, Pt1);
					// The back segment used for finding the best segment to test
					TBxFrom.SetXY(FoundBestNode, Pt2);
					// For each node between FBN and Pt1 fill in TBxTo with Pt1-testnode
					for (CurTestPart = CurPart; FoundBestNode && CurTestPart; CurTestPart = CurTestPart->NextPart)
						{
						// We need to do the current part and then all subsequent parts that were not hit
						// by following the linked edges around the current part.
						// We bail out when we run out of parts. All edges added at the end of the list will have
						// been tested because they are linked into the chain for CurPart
						if (CurTestPart != CurPart && ((CurPart->Tested && CurTestPart->Tested) || CurTestPart->Irrelevant))
							continue;
						NumEdgesToTest = CurTestPart->NumNodes;
						NumEdgesTested = 0;
						for (EdgeToTest = CurTestPart == CurPart ? FoundBestNode->ForwardList: CurTestPart->FirstNode()->ForwardList; 
							EdgeToTest; 
							++NumEdgesTested, EdgeToTest = CurTestPart == CurPart ? TestNode->ForwardList: EdgeToTest->NextEdgeList)
							{
							if (CurPart == CurTestPart)
								{
								// Original part or CurPart NULL
								if (EdgeToTest->NextEdgeThisNode)
									{
									BestEdge = NULL;
									BestAngle = 4.0;
									for (; EdgeToTest; EdgeToTest = EdgeToTest->NextEdgeThisNode)
										{
										TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode));
										if (TestAngle < BestAngle)
											{
											BestEdge = EdgeToTest;
											BestAngle = TestAngle;
											} // if
										} // for
									EdgeToTest = BestEdge;
									} // if
								// This is for the next loop but set it here because of the continue statement below.
								TBxFrom.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode);
								} // if
							else if (NumEdgesTested == NumEdgesToTest)
								{
								// Some subsequent part
								break;
								} // else

							TestNode = EdgeToTest->LeadingNode;
							if (TestNode == Pt1)
								break;
							if (EdgeToTest->Done)
								continue;

							TestAngle = AngleFinder.FindRelativeAngle(&RightSide, TestSide.SetXY(TestNode, Pt1));
							// If result is >= 2.0 the node is left of the new edge and another test must be made
							if (TestAngle >= 2.0)
								{
								// If testnode is inside the new triangle or on its edge then we need to break
								// and look for another FoundBestNode
								if (TriBounds.TestPointInTriangle(TestNode))
									{
									FoundBestNode = NULL;
									MinimumDistanceSquared = BestNodeDistanceSquared;
									break;
									} // if
								} // if
							} // for each edge
						} // for each part
					// If we didn't break due to finding a point in the triangle then we can proceed to 
					// annoint the new triangle
					} // if
				// Else if Pt2 connects to FoundBestNode:
				else if (FoundBestLeadingEdge && FoundBestLeadingEdge->LeadingNode == Pt1)
					{
					// Start with the segment Pt2-FBN and set it in TBxFrom
					LeftSide.SetXY(FoundBestNode, Pt2);
					// The back segment used for finding the best segment to test
					TBxFrom.SetXY(Pt2, Pt1);
					// For each node between Pt2 and FBN fill in TBxTo with Pt2-testnode
					for (CurTestPart = CurPart; FoundBestNode && CurTestPart; CurTestPart = CurTestPart->NextPart)
						{
						// We need to do the current part and then all subsequent parts that were not hit
						// by following the linked edges around the current part.
						// We bail out when we run out of parts. All edges added at the end of the list will have
						// been tested because they are linked into the chain for CurPart
						if (CurTestPart != CurPart && ((CurPart->Tested && CurTestPart->Tested) || CurTestPart->Irrelevant))
							continue;
						NumEdgesToTest = CurTestPart->NumNodes;
						NumEdgesTested = 0;
						for (EdgeToTest = CurTestPart == CurPart ? Pt2->ForwardList: CurTestPart->FirstNode()->ForwardList; 
							EdgeToTest; 
							++NumEdgesTested, EdgeToTest = CurTestPart == CurPart ? TestNode->ForwardList: EdgeToTest->NextEdgeList)
							{
							if (CurPart == CurTestPart)
								{
								// Original part or CurPart NULL
								if (EdgeToTest->NextEdgeThisNode)
									{
									BestEdge = NULL;
									BestAngle = 4.0;
									for (; EdgeToTest; EdgeToTest = EdgeToTest->NextEdgeThisNode)
										{
										TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode));
										if (TestAngle < BestAngle)
											{
											BestEdge = EdgeToTest;
											BestAngle = TestAngle;
											} // if
										} // for
									EdgeToTest = BestEdge;
									} // if
								// This is for the next loop but set it here because of the continue statement below.
								TBxFrom.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode);
								} // if
							else if (NumEdgesTested == NumEdgesToTest)
								{
								// Some subsequent part
								break;
								} // else

							TestNode = EdgeToTest->LeadingNode;
							if (TestNode == FoundBestNode)
								{
								if (BestNodeHits)
									{
									--BestNodeHits;
									continue;
									} // if
								else
									break;
								} // if
							if (EdgeToTest->Done)
								continue;

							TestAngle = AngleFinder.FindRelativeAngle(&LeftSide, TestSide.SetXY(TestNode, Pt2));
							// If result is <= 2.0 the node is right of the new edge and another test must be made
							if (TestAngle <= 2.0 || TestAngle == 4.0)
								{
								// If testnode is inside the new triangle or on its edge then we need to break
								// and look for another FoundBestNode
								if (TriBounds.TestPointInTriangle(TestNode))
									{
									FoundBestNode = NULL;
									MinimumDistanceSquared = BestNodeDistanceSquared;
									break;
									} // if
								} // if
							} // for each edge
						} // for each part
					// If we didn't break due to finding a point in the triangle then we can proceed to 
					// annoint the new triangle
					} // else if
				// Else two new edges created (possibly the result of a node found on an unlinked subsequent Part)
				else
					{
					int CurPtLeftSide, LastPtLeftSide, CurPtRightSide, LastPtRightSide;
					bool FirstPt = true, MiddlePt = false;

					RightSide.SetXY(FoundBestNode, Pt1);
					LeftSide.SetXY(FoundBestNode, Pt2);
					TBxFrom.SetXY(Pt2, Pt1);

					for (EdgeToTest = Pt2->ForwardList; FoundBestNode && EdgeToTest; EdgeToTest = TestNode->ForwardList)
						{
						if (EdgeToTest->NextEdgeThisNode)
							{
							BestEdge = NULL;
							BestAngle = 4.0;
							for (; EdgeToTest; EdgeToTest = EdgeToTest->NextEdgeThisNode)
								{
								TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode));
								if (TestAngle < BestAngle)
									{
									BestEdge = EdgeToTest;
									BestAngle = TestAngle;
									} // if
								} // for
							EdgeToTest = BestEdge;
							} // if
						// This is for the next loop but set it here because of the continue statement below.
						TBxFrom.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode);

						TestNode = EdgeToTest->LeadingNode;
						if (TestNode == FoundBestNode)
							{
							// reset
							MiddlePt = true;
							continue;
							} // if
						if (TestNode == Pt1)
							{
							if (LastPtRightSide < 1)
								{
								CurPtLeftSide = 1;
								CurPtRightSide = 0;
								} // if
							else
								break;
							} // if
						else
							{
							TestAngle = AngleFinder.FindRelativeAngle(&LeftSide, TestSide.SetXY(TestNode, FoundBestNode));
							CurPtLeftSide = TestAngle < 2.0 ? 1: TestAngle > 2.0 && TestAngle < 4.0 ? -1: 0;
							TestAngle = AngleFinder.FindRelativeAngle(&RightSide, &TestSide);
							CurPtRightSide = TestAngle < 2.0 ? 1: TestAngle > 2.0 && TestAngle < 4.0 ? -1: 0;
							} // if
							
						if (FirstPt)
							{
							FirstPt = false;
							if (CurPtLeftSide > -1)
								{
								// point lies between the segments or one point lies on an extended segment
								// find if test point is at an angle from 2.0 to the angle that Pt2-Pt1 makes
								// If it is in that range then it violates the triangle being formed of Pt1-Pt2-FBN
								TestAngle = AngleFinder.FindRelativeAngle(&LeftSide, TestSide.SetXY(TestNode, Pt2));
								SideAngle = AngleFinder.FindRelativeAngle(&LeftSide, TestSide.SetXY(Pt1, Pt2));
								if (TestAngle <= 2.0 && TestAngle >= SideAngle)
									{
									// violation
									FoundBestNode = NULL;
									MinimumDistanceSquared = BestNodeDistanceSquared;
									break;
									} // if
								} // if
							} // if
						else if (MiddlePt)
							{
							MiddlePt = false;
							if (CurPtLeftSide > -1 && CurPtRightSide < 1)
								{
								// point lies between or on the segments, inside the triangle
								// violation
								FoundBestNode = NULL;
								MinimumDistanceSquared = BestNodeDistanceSquared;
								break;
								} // if
							} // if
						else
							{
							// test segment crosses over or impinges on one of the extended triangle arms
							if (CurPtLeftSide != LastPtLeftSide && CurPtLeftSide > -1)
								{
								// find intersection of test seg with left side
								VI.FindIntersectionExtended(Pt2, FoundBestNode, EdgeToTest->TrailingNode, EdgeToTest->LeadingNode, IntersectionLat, IntersectionLon);
								// find dot product of left side with vec from FBN to intersection
								TestSide.SetXY(IntersectionLon, IntersectionLat, FoundBestNode);
								if ((DotResult = TestSide.DotProductXY(&TestSide, &LeftSide)) < 0)
									{
									TestSide.SetXY(IntersectionLon, IntersectionLat, Pt2);
									if ((DotResult = TestSide.DotProductXY(&TestSide, &LeftSide)) > 0)
										{
										// intersection lies between Pt2 and FBN so violates the left arm of the new triangle
										FoundBestNode = NULL;
										MinimumDistanceSquared = BestNodeDistanceSquared;
										break;
										} // if
									} // if
								} // if 
							if (CurPtRightSide != LastPtRightSide && CurPtRightSide < 1)
								{
								// find intersection of test seg with left side
								VI.FindIntersectionExtended(Pt1, FoundBestNode, EdgeToTest->TrailingNode, EdgeToTest->LeadingNode, IntersectionLat, IntersectionLon);
								// find dot product of right side with vec from FBN to intersection
								TestSide.SetXY(IntersectionLon, IntersectionLat, FoundBestNode);
								if ((DotResult = TestSide.DotProductXY(&TestSide, &RightSide)) < 0)
									{
									TestSide.SetXY(IntersectionLon, IntersectionLat, Pt1);
									if ((DotResult = TestSide.DotProductXY(&TestSide, &RightSide)) > 0)
										{
										// intersection lies between Pt1 and FBN so violates the right arm of the new triangle
										FoundBestNode = NULL;
										MinimumDistanceSquared = BestNodeDistanceSquared;
										break;
										} // if
									} // if
								} // if 
							} // if
						if (TestNode == Pt1)
							break;
						LastPtLeftSide = CurPtLeftSide;
						LastPtRightSide = CurPtRightSide;
						} // for

					// test any other parts that haven't been included already by virtue of having been linked
					// into the main body
					for (CurTestPart = CurPart->NextPart; FoundBestNode && CurTestPart; CurTestPart = CurTestPart->NextPart)
						{
						// We need to do the current part and then all subsequent parts that were not hit
						// by following the linked edges around the current part.
						// We bail out when we run out of parts. All edges added at the end of the list will have
						// been tested because they are linked into the chain for CurPart
						if ((CurPart->Tested && CurTestPart->Tested) || CurTestPart->Irrelevant)
							continue;
						NumEdgesToTest = CurTestPart->NumNodes;
						if (CurTestPart->FirstNode() != FoundBestNode)
							++NumEdgesToTest;
						NumEdgesTested = 0;
						FirstPt = true;
						MiddlePt = false;
						LastNode = NULL;
						for (TestNode = CurTestPart->FirstNode(); NumEdgesTested < NumEdgesToTest; 
							++NumEdgesTested, TestNode = TestNode->NextNode)
							{
							if (TestNode == FoundBestNode)
								{
								MiddlePt = true;
								FirstPt = false;
								LastNode = TestNode;
								continue;
								} // if
							TestAngle = AngleFinder.FindRelativeAngle(&LeftSide, TestSide.SetXY(TestNode, FoundBestNode));
							CurPtLeftSide = TestAngle < 2.0 ? 1: TestAngle > 2.0 && TestAngle < 4.0 ? -1: 0;
							TestAngle = AngleFinder.FindRelativeAngle(&RightSide, &TestSide);
							CurPtRightSide = TestAngle < 2.0 ? 1: TestAngle > 2.0 && TestAngle < 4.0 ? -1: 0;
							if (MiddlePt)
								{
								MiddlePt = false;
								if (CurPtLeftSide > -1 && CurPtRightSide < 1)
									{
									// point lies between or on the segments, maybe inside the triangle
									if (TriBounds.TestPointInTriangle(TestNode))
										{
										FoundBestNode = NULL;
										MinimumDistanceSquared = BestNodeDistanceSquared;
										break;
										} // if
									} // if
								} // else
							else if (FirstPt)
								{
								// find status of first node relative to both edges
								FirstPt = false;
								if (CurPtLeftSide > -1 && CurPtRightSide < 1)
									{
									// point lies between or on the segments, maybe inside the triangle
									if (TriBounds.TestPointInTriangle(TestNode))
										{
										FoundBestNode = NULL;
										MinimumDistanceSquared = BestNodeDistanceSquared;
										break;
										} // if
									} // if
								} // if
							// test segment crosses over or impinges on one of the extended triangle arms
							else
								{
								if (CurPtLeftSide != LastPtLeftSide && CurPtLeftSide > -1)
									{
									// find intersection of test seg with left side
									VI.FindIntersectionExtended(Pt2, FoundBestNode, LastNode, TestNode, IntersectionLat, IntersectionLon);
									// find dot product of left side with vec from FBN to intersection
									TestSide.SetXY(IntersectionLon, IntersectionLat, FoundBestNode);
									if ((DotResult = TestSide.DotProductXY(&TestSide, &LeftSide)) < 0)
										{
										TestSide.SetXY(IntersectionLon, IntersectionLat, Pt2);
										if ((DotResult = TestSide.DotProductXY(&TestSide, &LeftSide)) > 0)
											{
											// intersection lies between Pt2 and FBN so violates the left arm of the new triangle
											FoundBestNode = NULL;
											MinimumDistanceSquared = BestNodeDistanceSquared;
											break;
											} // if
										} // if
									} // if 
								if (CurPtRightSide != LastPtRightSide && CurPtRightSide < 1)
									{
									// find intersection of test seg with left side
									VI.FindIntersectionExtended(Pt1, FoundBestNode, LastNode, TestNode, IntersectionLat, IntersectionLon);
									// find dot product of right side with vec from FBN to intersection
									TestSide.SetXY(IntersectionLon, IntersectionLat, FoundBestNode);
									if ((DotResult = TestSide.DotProductXY(&TestSide, &RightSide)) < 0)
										{
										TestSide.SetXY(IntersectionLon, IntersectionLat, Pt1);
										if ((DotResult = TestSide.DotProductXY(&TestSide, &RightSide)) > 0)
											{
											// intersection lies between Pt1 and FBN so violates the right arm of the new triangle
											FoundBestNode = NULL;
											MinimumDistanceSquared = BestNodeDistanceSquared;
											break;
											} // if
										} // if
									} // if 
								} // else
							LastPtLeftSide = CurPtLeftSide;
							LastPtRightSide = CurPtRightSide;
							LastNode = TestNode;
							} // for
						} // for
					} // else
					
				// If we haven't crossed another segment we are ready to make a polygon
				if (FoundBestNode)
					{
					if (FoundBestTrailingEdge && FoundBestTrailingEdge->TrailingNode == Pt2)
						{
						FoundBestTrailingEdge->Done = true;
						FoundBestTrailingEdge->TrailingNode->RemoveEdgeList(FoundBestTrailingEdge);
						} // if
					if (FoundBestLeadingEdge && FoundBestLeadingEdge->LeadingNode == Pt1)
						{
						FoundBestLeadingEdge->Done = true;
						FoundBestLeadingEdge->TrailingNode->RemoveEdgeList(FoundBestLeadingEdge);
						} // if
					CurEdge->Done = true;
					CurEdge->TrailingNode->RemoveEdgeList(CurEdge);
					if (CurPart == OuterPart || CurPart->Tested)
						FoundBestPart->Tested = true;
					// Make a polygon list
					if (*VPListPtr = new VectorPolygonListDouble())
						{
						// Form a new polygon out of the three nodes using this polygon as a prototype
						#ifdef DEBUG_POLYGONS_TO_VECTOR
						if (PrintToVector)
							{
							sprintf(DbgStr, " EdgeCt %d %p %p %p\n", EdgeCt, Pt1, Pt2, FoundBestNode); 
							OutputDebugString(DbgStr);
							} // if
						#endif // DEBUG_POLYGONS_TO_VECTOR
						if (! (*VPListPtr)->MakeVectorPolygon(Pt1, Pt2, FoundBestNode, this, true))
							Success = false;
						VPListPtr = (VectorPolygonListDouble **)&(*VPListPtr)->NextPolygonList;
						// Make new edge or edges
						if (! FoundBestTrailingEdge || ! FoundBestTrailingEdge->Done)
							{
							if (! FoundBestLeadingEdge || ! FoundBestLeadingEdge->Done)
								{
								// Make an edge from FoundBestNode to Pt2
								if (*EdgeListPtr = new PolygonEdgeList(FoundBestNode, Pt2, CurEdge->LeadingPart))
									{
									FoundBestNode->AddForwardEdge(*EdgeListPtr);
									EdgeListPtr = &(*EdgeListPtr)->NextEdgeList;
									} // if
								else
									{
									Success = false;
									break;
									} // else
								// Make an edge from Pt1 to FoundBestNode
								if (*EdgeListPtr = new PolygonEdgeList(Pt1, FoundBestNode, FoundBestPart))
									{
									Pt1->AddForwardEdge(*EdgeListPtr);
									EdgeListPtr = &(*EdgeListPtr)->NextEdgeList;
									} // if
								else
									{
									Success = false;
									break;
									} // else
								} // if neither done
							else
								{
								// Make an edge from FoundBestNode to Pt2
								if (*EdgeListPtr = new PolygonEdgeList(FoundBestNode, Pt2, CurEdge->LeadingPart))
									{
									FoundBestNode->AddForwardEdge(*EdgeListPtr);
									EdgeListPtr = &(*EdgeListPtr)->NextEdgeList;
									} // if
								else
									{
									Success = false;
									break;
									} // else
								} // else only leading edge done
							} // if
						else if (! FoundBestLeadingEdge || ! FoundBestLeadingEdge->Done)
							{
							// Make an edge from Pt1 to FoundBestNode
							if (*EdgeListPtr = new PolygonEdgeList(Pt1, FoundBestNode, FoundBestPart))
								{
								Pt1->AddForwardEdge(*EdgeListPtr);
								EdgeListPtr = &(*EdgeListPtr)->NextEdgeList;
								} // if
							else
								{
								Success = false;
								break;
								} // else
							} // if
						} // if
					else
						{
						Success = false;
						break;
						} // else
					} // if
				} // if
			else
				{
				// We're screwed
				Success = false;
				break;
				} // else
			} // while
		} // for
	// Remove the nodes from this polygon
	RemoveNodes();
	} // if
	
FinishIt:

// Remove any remaining edge lists

for (CurEdge = EdgeList; CurEdge; CurEdge = EdgeList)
	{
	EdgeList = EdgeList->NextEdgeList;
	if (CurEdge->TrailingNode)
		CurEdge->TrailingNode->RemoveEdgeList(CurEdge);
	delete CurEdge; 
	} // for
if (ContiguousNodeList)
	delete [] ContiguousNodeList;
	
if (! Success)
	{
	// Delete everything created
	for (VectorPolygonListDouble *CurVPList = NewVPList; CurVPList; CurVPList = NewVPList)
		{
		NewVPList = (VectorPolygonListDouble *)NewVPList->NextPolygonList;
		CurVPList->DeletePolygon();
		delete CurVPList;
		} // for
	} // if
	
return (NewVPList);

} // VectorPolygon::Triangulate

/*===========================================================================*/

// Obsolete as of 1/31/08
/*
VectorPolygonListDouble *VectorPolygon::Triangulate(bool &Success)
{
double NodeLat, NodeLon, MinimumDistanceSquared, BestNodeDistanceSquared, TestNodeDistanceSquared,
	BestAngle, TestAngle;
unsigned long NodeCt, NumEdgesToTest, NumEdgesTested, MaxTestsBeforeBailout;
VectorPolygonListDouble *NewVPList = NULL, **VPListPtr;
VectorNode *CurNode, *Pt1, *Pt2, *Pt3, *DelPt, *FoundBestNode, *TestNode;
VectorNode *Max1XRightNode, *Min1XRightNode, *Max1YRightNode, *Min1YRightNode, 
	*Max1XLeftNode, *Min1XLeftNode, *Max1YLeftNode, *Min1YLeftNode, 
	*Max2XNode, *Min2XNode, *Max2YNode, *Min2YNode;
VectorPart *CurPart, *CurTestPart, *FoundBestPart;
PolygonEdgeList *EdgeList = NULL, **EdgeListPtr, *BestEdge, *CurEdge, *FoundBestLeadingEdge, 
	*FoundBestTrailingEdge, *EdgeToTest;
bool UseRight, SetLeadingEdge, PartDone;
TriangleBoundingBoxVector CP1, RightSide, LeftSide, TBxFrom, TBxTo, AngleFinder;
VectorNode TestRightSegStart, TestLeftSegStart, TestSegEnd, EdgeSegStart, EdgeSegEnd;
VectorIntersecter VI;
TriangleBoundingBox TriBounds;

EdgeListPtr = &EdgeList;
VPListPtr = &NewVPList;

// break a polygon down into a set of triangles that do not overlap and cover only the area
// covered by the original polygon. Try to optimize the triangles so they are not slivers along the
// original polygon edges.

// first order of business: determine the depth of the problem.
// If there are only four vertices, find the two closest together and link them as the middle edge
// If there is more than one part, find the closest points between the inner parts and each other and the outer
// polygon and link all the parts and outside together with a pair of edges (coincident).
// Build a set of edge entities that connect all the edges with nodes in an edge-node topology that
// can be followed from one point through all the ponts in the polygon including any inside parts.
// Analyze the closest node to each edge that forms a positive triangle whose edges are not intersected by any other
// edge.

if (TotalNumNodes > 3)
	{
	if (TotalNumNodes == 4)
		{
		if (*VPListPtr = new VectorPolygonListDouble())
			{
			CurPart = GetFirstPart();
			CurNode = CurPart->FirstNode();
			// find the closest pair of opposite nodes
			// test to see if they form a positive triangle if joined.
			// if not, use the other set of opposing nodes to join
			RightSide.SetXY(CurNode->NextNode->NextNode, CurNode);
			LeftSide.SetXY(CurNode->NextNode->NextNode->NextNode, CurNode->NextNode);
			if (RightSide.LengthXYSquared() < LeftSide.LengthXYSquared())
				{
				LeftSide.SetXY(CurNode->NextNode, CurNode);
				UseRight = (CP1.CrossProductXY(&LeftSide, &RightSide) > 0.0);
				}
			else
				{
				RightSide.SetXY(CurNode, CurNode->NextNode);
				UseRight = (! (CP1.CrossProductXY(&LeftSide, &RightSide) > 0.0));
				} // else
			if (UseRight)
				{
				Pt1 = CurNode;
				Pt2 = CurNode->NextNode->NextNode;
				DelPt = Pt3 = CurNode->NextNode->NextNode->NextNode;
				// dissociate fourth node and reduce totals
				Pt2->NextNode = CurNode;
				--TotalNumNodes;
				CurPart->NumNodes = TotalNumNodes;
				} // if
			else
				{
				Pt1 = CurNode->NextNode;
				DelPt = Pt2 = CurNode->NextNode->NextNode;
				Pt3 = CurNode->NextNode->NextNode->NextNode;
				// dissociate second node and reduce totals
				Pt1->NextNode = Pt3;
				--TotalNumNodes;
				CurPart->NumNodes = TotalNumNodes;
				} // else
			// form a new polygon out of the three nodes using this polygon as a prototype
			if (! (*VPListPtr)->MakeVectorPolygon(Pt1, Pt2, Pt3, this, true))
				Success = false;
			delete DelPt;
			goto FinishIt;
			} // if
		Success = false;
		goto FinishIt;
		} // else if exactly 4 nodes in only one part the solution can be quite simple
	// Add edges for each polygon segment
	for (CurPart = GetFirstPart(); CurPart && Success; CurPart = CurPart->NextPart)
		{
		for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; ++NodeCt, CurNode = CurNode->NextNode)
			{
			if (*EdgeListPtr = new PolygonEdgeList(CurNode, CurNode->NextNode, CurPart))
				{
				CurNode->AddForwardEdge(*EdgeListPtr);
				EdgeListPtr = &(*EdgeListPtr)->NextEdgeList;
				} // if
			else
				{
				Success = false;
				break;
				} // else
			} // for
		} // for

	// walk the edge list and solve the triangle for each edge,
	// Add new edges at the end of the list as necessary
	for (CurEdge = EdgeList, CurPart = GetFirstPart(), PartDone = false; CurEdge && Success; CurEdge = CurEdge->NextEdgeList)
		{
		if (PartDone)
			{
			CurPart = CurPart->NextPart;
			PartDone = false;
			} // if
		else if (CurPart && CurEdge->LeadingNode == CurPart->FirstNode())
			PartDone = true;
		if (CurEdge->Done)
			continue;
		Pt1 = CurEdge->TrailingNode;
		Pt2 = CurEdge->LeadingNode;
		MinimumDistanceSquared = 0.0;
		FoundBestNode = NULL;
		MaxTestsBeforeBailout = TotalNumNodes * 2;

		while (! FoundBestNode)
			{
			for (CurTestPart = CurPart; CurTestPart; CurTestPart = CurTestPart->NextPart)
				{
				// initialize Tested flag
				CurTestPart->Tested = false;
				} // for
			FoundBestLeadingEdge = FoundBestTrailingEdge = NULL;
			FoundBestPart = NULL;
			SetLeadingEdge = false;
			BestNodeDistanceSquared = FLT_MAX;
			TBxFrom.SetXY(Pt2, Pt1);
			// test all the edges that are joined to the current edge
			// this may cross to other parts and if they do, those parts should be marked as tested
			// EdgeToTest should always be true since we're going around a linked loop
			// test the count to make sure we aren't running around in endless circles as can happen 
			// if there is bad topology.
			for (NodeCt = 0, EdgeToTest = Pt2->ForwardList; EdgeToTest && NodeCt < MaxTestsBeforeBailout;  ++NodeCt, EdgeToTest = TestNode->ForwardList)
				{
				if (EdgeToTest->NextEdgeThisNode)
					{
					BestEdge = NULL;
					BestAngle = 4.0;
					for (; EdgeToTest; EdgeToTest = EdgeToTest->NextEdgeThisNode)
						{
						TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode));
						if (TestAngle < BestAngle)
							{
							BestEdge = EdgeToTest;
							BestAngle = TestAngle;
							} // if
						} // for
					EdgeToTest = BestEdge;
					} // if

				TestNode = EdgeToTest->LeadingNode;
				if (SetLeadingEdge)
					FoundBestLeadingEdge = EdgeToTest;
				SetLeadingEdge = false;
					
				// see if we've come full circle
				if (TestNode == Pt1)
					break;

				EdgeToTest->LeadingPart->Tested = true;
				// test distance to node from both end points of current edge
				RightSide.SetXY(TestNode, Pt1);
				LeftSide.SetXY(TestNode, Pt2);
				// triangle is invalid if it is 0 area or negative area
				if (CP1.CrossProductXY(&RightSide, &LeftSide) > 0.0)
					{
					TestNodeDistanceSquared = RightSide.LengthXYSquared() + LeftSide.LengthXYSquared();
					if (TestNodeDistanceSquared <= BestNodeDistanceSquared && TestNodeDistanceSquared > MinimumDistanceSquared)
						{
						BestNodeDistanceSquared = TestNodeDistanceSquared;
						FoundBestNode = TestNode;
						FoundBestPart = EdgeToTest->LeadingPart;
						FoundBestTrailingEdge = EdgeToTest;
						FoundBestLeadingEdge = NULL;
						SetLeadingEdge = true;
						} // if
					} // if
				// this is for the next loop but set it here where the current edge is known
				TBxFrom.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode);
				} // for
			
			// test all nodes in subsequent parts that are not joined to the current part
			for (CurTestPart = CurPart ? CurPart->NextPart: NULL; CurTestPart; CurTestPart = CurTestPart->NextPart)
				{
				// skip parts that are incorporated into the main loop by previous attachments
				if (CurTestPart->Tested)
					continue;
				// test each node in the part
				for (EdgeToTest = CurTestPart->FirstNode()->ForwardList, NumEdgesTested = 0; EdgeToTest && NumEdgesTested < CurTestPart->NumNodes;  ++NumEdgesTested, EdgeToTest = EdgeToTest->NextEdgeList)
					{
					TestNode = EdgeToTest->LeadingNode;
						
					// test distance to node from both end points of current edge
					RightSide.SetXY(TestNode, Pt1);
					LeftSide.SetXY(TestNode, Pt2);
					// triangle is invalid if it is 0 area or negative area
					if (CP1.CrossProductXY(&RightSide, &LeftSide) > 0.0)
						{
						TestNodeDistanceSquared = RightSide.LengthXYSquared() + LeftSide.LengthXYSquared();
						if (TestNodeDistanceSquared < BestNodeDistanceSquared && TestNodeDistanceSquared > MinimumDistanceSquared)
							{
							BestNodeDistanceSquared = TestNodeDistanceSquared;
							FoundBestNode = TestNode;
							FoundBestPart = CurTestPart;
							// set the leading and trailing edges to NULL since they are not going to be used
							FoundBestTrailingEdge = FoundBestLeadingEdge = NULL;
							} // if
						} // if
					} // for
				} // for
			
			if (FoundBestNode)
				{
				// if still working on one of the original polygon's sides need to test that no other side crosses
				// the edges that would be created out of FoundBestNode and the nodes of the current edge.
				// CurPart will be NULL if we've passed the end of the original polygon edges.
				//if (CurPart)
					{
					TriBounds.SetTriangle(Pt1, Pt2, FoundBestNode);
					// test all the segments on the loop to see if any cross either of the edges that would be created
					// don't test segments that adjoin any of the three nodes in the new triangle
					TestRightSegStart.Lat = Pt1->Lat;
					TestRightSegStart.Lon = Pt1->Lon;
					TestLeftSegStart.Lat = Pt2->Lat;
					TestLeftSegStart.Lon = Pt2->Lon;
					TestSegEnd.Lat = FoundBestNode->Lat;
					TestSegEnd.Lon = FoundBestNode->Lon;
					// Pt1 -> FoundBestNode
					TestRightSegStart.NextNode = &TestSegEnd;
					// Pt2 -> FoundBestNode
					TestLeftSegStart.NextNode = &TestSegEnd;

					// Pt1 -> FoundBestNode
					Max1XRightNode = TestRightSegStart.Lon > TestSegEnd.Lon ? &TestRightSegStart: &TestSegEnd;
					Min1XRightNode = Max1XRightNode == &TestRightSegStart ? &TestSegEnd: &TestRightSegStart;
					Max1YRightNode = TestRightSegStart.Lat > TestSegEnd.Lat ? &TestRightSegStart: &TestSegEnd;
					Min1YRightNode = Max1YRightNode == &TestRightSegStart ? &TestSegEnd: &TestRightSegStart;

					// Pt2 -> FoundBestNode
					Max1XLeftNode = TestLeftSegStart.Lon > TestSegEnd.Lon ? &TestLeftSegStart: &TestSegEnd;
					Min1XLeftNode = Max1XLeftNode == &TestLeftSegStart ? &TestSegEnd: &TestLeftSegStart;
					Max1YLeftNode = TestLeftSegStart.Lat > TestSegEnd.Lat ? &TestLeftSegStart: &TestSegEnd;
					Min1YLeftNode = Max1YLeftNode == &TestLeftSegStart ? &TestSegEnd: &TestLeftSegStart;

					TBxFrom.SetXY(Pt2, Pt1);
					for (CurTestPart = CurPart; FoundBestNode && ((CurPart && CurTestPart) || (! CurPart && ! CurTestPart)); CurTestPart = CurTestPart ? CurTestPart->NextPart: GetFirstPart())
						{
						// we need to do the current part and then all subsequent parts that were not hit
						// by following the linked edges around the current part.
						// We bail out when we run out of parts. All edges added at the end of the list will have
						// been tested because they are linked into the chain for CurPart
						if (CurTestPart && CurTestPart != CurPart && CurTestPart->Tested)
							continue;
						NumEdgesToTest = CurTestPart ? CurTestPart->NumNodes: 0;
						NumEdgesTested = 0;
						for (EdgeToTest = (! CurTestPart || CurTestPart == CurPart) ? Pt2->ForwardList: CurTestPart->FirstNode()->ForwardList; 
							EdgeToTest; 
							++NumEdgesTested, EdgeToTest = (! CurTestPart || CurTestPart == CurPart) ? TestNode->ForwardList: EdgeToTest->NextEdgeList)
							{
							if (CurPart == CurTestPart)
								{
								// original part or CurPart NULL
								if (EdgeToTest == CurEdge)
									break;
								} // if
							else
								{
								// some subsequent part
								if (NumEdgesTested == NumEdgesToTest)
									break;
								} // else
							
							if (CurTestPart == CurPart && EdgeToTest->NextEdgeThisNode)
								{
								BestEdge = NULL;
								BestAngle = 4.0;
								for (; EdgeToTest; EdgeToTest = EdgeToTest->NextEdgeThisNode)
									{
									TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode));
									if (TestAngle < BestAngle)
										{
										BestEdge = EdgeToTest;
										BestAngle = TestAngle;
										} // if
									} // for
								EdgeToTest = BestEdge;
								} // if
							TestNode = EdgeToTest->LeadingNode;

							// this is for the next loop but set it here because of the continue statement below.
							if (CurTestPart == CurPart)
								TBxFrom.SetXY(EdgeToTest->LeadingNode, EdgeToTest->TrailingNode);
							if (EdgeToTest->Done)
								continue;
							// edges adjoining the best node are not going to cross, so skip them but before 
							// skipping the leading edge, test its leading node
							if (EdgeToTest == FoundBestTrailingEdge)
								continue;
							// test to see if by any chance the leading node of the edge is inside the new triangle
							if (TestNode != Pt1 && TriBounds.TestPointInTriangle(TestNode))
								{
								FoundBestNode = NULL;
								MinimumDistanceSquared = BestNodeDistanceSquared;
								break;
								} // if
							if (EdgeToTest == FoundBestLeadingEdge)
								continue;

							EdgeSegStart.Lat = EdgeToTest->TrailingNode->Lat;
							EdgeSegStart.Lon = EdgeToTest->TrailingNode->Lon;
							EdgeSegEnd.Lat = EdgeToTest->LeadingNode->Lat;
							EdgeSegEnd.Lon = EdgeToTest->LeadingNode->Lon;
							// EdgeToTest->TrailingNode -> EdgeToTest->LeadingNode
							EdgeSegStart.NextNode = &EdgeSegEnd;

							// EdgeToTest->TrailingNode -> EdgeToTest->LeadingNode
							Max2XNode = EdgeSegStart.Lon > EdgeSegEnd.Lon ? &EdgeSegStart: &EdgeSegEnd;
							Min2XNode = Max2XNode == &EdgeSegStart ? &EdgeSegEnd: &EdgeSegStart;
							Max2YNode = EdgeSegStart.Lat > EdgeSegEnd.Lat ? &EdgeSegStart: &EdgeSegEnd;
							Min2YNode = Max2YNode == &EdgeSegStart ? &EdgeSegEnd: &EdgeSegStart;
								
							// compare
							// EdgeToTest->TrailingNode -> EdgeToTest->LeadingNode
							// against
							// Pt1 -> FoundBestNode
							if (Min2XNode->Lon < Max1XRightNode->Lon && Max2XNode->Lon > Min1XRightNode->Lon)
								{
								if (Min2YNode->Lat < Max1YRightNode->Lat && Max2YNode->Lat > Min1YRightNode->Lat)
									{
									if (VI.FindIntersectionContiguous(&TestRightSegStart, &EdgeSegStart,
										Min1XRightNode, Max1XRightNode, Min2XNode, Max2XNode, 
										Min1YRightNode, Max1YRightNode, Min2YNode, Max2YNode, 
										NodeLat, NodeLon))
										{
										FoundBestNode = NULL;
										MinimumDistanceSquared = BestNodeDistanceSquared;
										break;
										} //if
									} // if
								} // if
							// compare
							// EdgeToTest->TrailingNode -> EdgeToTest->LeadingNode
							// against
							// Pt2 -> FoundBestNode
							if (Min2XNode->Lon < Max1XLeftNode->Lon && Max2XNode->Lon > Min1XLeftNode->Lon)
								{
								if (Min2YNode->Lat < Max1YLeftNode->Lat && Max2YNode->Lat > Min1YLeftNode->Lat)
									{
									if (VI.FindIntersectionContiguous(&TestLeftSegStart, &EdgeSegStart,
										Min1XLeftNode, Max1XLeftNode, Min2XNode, Max2XNode, 
										Min1YLeftNode, Max1YLeftNode, Min2YNode, Max2YNode, 
										NodeLat, NodeLon))
										{
										FoundBestNode = NULL;
										MinimumDistanceSquared = BestNodeDistanceSquared;
										break;
										} //if
									} // if
								} // if
							} // for
						} // for
					} // if
					
				// if we haven't crossed another segment we are ready to make a polygon
				if (FoundBestNode)
					{
					if (FoundBestTrailingEdge && FoundBestTrailingEdge->TrailingNode == Pt2)
						{
						FoundBestTrailingEdge->Done = true;
						FoundBestTrailingEdge->TrailingNode->RemoveEdgeList(FoundBestTrailingEdge);
						} // if
					if (FoundBestLeadingEdge && FoundBestLeadingEdge->LeadingNode == Pt1)
						{
						FoundBestLeadingEdge->Done = true;
						FoundBestLeadingEdge->TrailingNode->RemoveEdgeList(FoundBestLeadingEdge);
						} // if
					CurEdge->Done = true;
					CurEdge->TrailingNode->RemoveEdgeList(CurEdge);
					// make a polygon list
					if (*VPListPtr = new VectorPolygonListDouble())
						{
						// form a new polygon out of the three nodes using this polygon as a prototype
						if (! (*VPListPtr)->MakeVectorPolygon(Pt1, Pt2, FoundBestNode, this, true))
							Success = false;
						VPListPtr = (VectorPolygonListDouble **)&(*VPListPtr)->NextPolygonList;
						// make new edge or edges
						if (! FoundBestTrailingEdge || ! FoundBestTrailingEdge->Done)
							{
							if (! FoundBestLeadingEdge || ! FoundBestLeadingEdge->Done)
								{
								// make an edge from FoundBestNode to Pt2
								if (*EdgeListPtr = new PolygonEdgeList(FoundBestNode, Pt2, CurEdge->LeadingPart))
									{
									FoundBestNode->AddForwardEdge(*EdgeListPtr);
									EdgeListPtr = &(*EdgeListPtr)->NextEdgeList;
									} // if
								else
									{
									Success = false;
									break;
									} // else
								// make an edge from Pt1 to FoundBestNode
								if (*EdgeListPtr = new PolygonEdgeList(Pt1, FoundBestNode, FoundBestPart))
									{
									Pt1->AddForwardEdge(*EdgeListPtr);
									EdgeListPtr = &(*EdgeListPtr)->NextEdgeList;
									} // if
								else
									{
									Success = false;
									break;
									} // else
								} // if neither done
							else
								{
								// make an edge from FoundBestNode to Pt2
								if (*EdgeListPtr = new PolygonEdgeList(FoundBestNode, Pt2, CurEdge->LeadingPart))
									{
									FoundBestNode->AddForwardEdge(*EdgeListPtr);
									EdgeListPtr = &(*EdgeListPtr)->NextEdgeList;
									} // if
								else
									{
									Success = false;
									break;
									} // else
								} // else only leading edge done
							} // if
						else if (! FoundBestLeadingEdge || ! FoundBestLeadingEdge->Done)
							{
							// make an edge from Pt1 to FoundBestNode
							if (*EdgeListPtr = new PolygonEdgeList(Pt1, FoundBestNode, FoundBestPart))
								{
								Pt1->AddForwardEdge(*EdgeListPtr);
								EdgeListPtr = &(*EdgeListPtr)->NextEdgeList;
								} // if
							else
								{
								Success = false;
								break;
								} // else
							} // if
						} // if
					else
						{
						Success = false;
						break;
						} // else
					} // if
				} // if
			else
				{
				// we're screwed
				Success = false;
				break;
				} // else
			} // while
		} // for
	// remove the nodes from this polygon
	RemoveNodes();
	} // if
	
FinishIt:

// remove any remaining edge lists

for (CurEdge = EdgeList; CurEdge; CurEdge = EdgeList)
	{
	EdgeList = EdgeList->NextEdgeList;
	if (CurEdge->TrailingNode)
		CurEdge->TrailingNode->RemoveEdgeList(CurEdge);
	delete CurEdge; 
	} // for

if (! Success)
	{
	// delete everything created
	for (VectorPolygonListDouble *CurVPList = NewVPList; CurVPList; CurVPList = NewVPList)
		{
		NewVPList = (VectorPolygonListDouble *)NewVPList->NextPolygonList;
		CurVPList->DeletePolygon();
		delete CurVPList;
		} // for
	} // if
	
return (NewVPList);

} // VectorPolygon::Triangulate
*/

// Obsolete as of 10/18/06
/*
VectorPolygonListDouble *VectorPolygon::Triangulate(void)
{
double TriArea;
VectorPolygonListDouble *NewVPList = NULL, **VPListPtr;
VectorNode *Pt1, *Pt2, *Pt3, *TestPt;
unsigned long Legal, TryCounter;
int Success = 1, PolySign, TriSign;
bool accelerated;
TriangleBoundingBox TriBounds;

VPListPtr = &NewVPList;

// break a polygon down into a set of triangles that do not overlap and cover only the area
// covered by the original polygon

// walk around the outside of the polygon and look for three consecutive vertices that create a positive triangle
// that do not contain any other vertex.
// for each triangle thus found make a polygon and remove the middle vertex from consideration

// to test for each vertex to see if it is inside the triangle first test to see if it is inside a bounding box
// formed by the three triangle vertices.
// then test to see if the cross product of the vertex-corner vector is the same Z sign as the cross product of all
// the pairs of sides

// this will assume a VectorPolygon that links back to the origin at the last node, that has no redundant vertices
// and is not self-intersecting
if (TotalNumNodes > 3)
	{
	accelerated = false;
	if (NumNodes >= VNACCELMINNODES)
		{
		accelerated = true;
		Pt1 = FirstNode();
		VNA.CreateBins(NumNodes);
		VNA.ComputeBinBounds(Pt1);
		VNA.BinAllPoints(Pt1);
		} // if
	TryCounter = 0;
	PolySign = PolygonArea() >= 0.0 ? 1: 0;
	for (Pt1 = FirstNode(), Pt2 = SecondNode(), Pt3 = ThirdNode(); NumNodes > 2; Pt1 = Pt2, Pt2 = Pt3, Pt3 = Pt3->NextNode)
		{
		if (TryCounter > NumNodes)
			{
			break;
			} // if
		++TryCounter;
		// test to see if this is a legal triangle
		Legal = 1;
		TriBounds.SetTriangle(Pt1, Pt2, Pt3);
		TriSign = (TriArea = TriBounds.TriangleArea()) > 0.0 ? 1: 0;
		// triangles are legal if they have no area at all, testing for 0 is approximate
		if (TriSign == PolySign || fabs(TriArea) <= 1.0E-15)
			{
			if (fabs(TriArea) > 1.0E-15)
				{
				if (accelerated)
					{
					Legal = VNA.TestPointsInTriangle(&TriBounds, Pt3->NextNode, Pt1);
					} // if
				else
					{
					for (TestPt = Pt3->NextNode; TestPt != Pt1; TestPt = TestPt->NextNode)
						{
						if (! TestPt->FlagCheck(WCS_VECTORNODE_FLAG_HOLEREMOVALSHARE))
							{
							// see if TestPt is inside a box formed by the three triangle vertices
							// then see if TestPt is inside triangle formed by the three triangle vertices
							// use cross products to test how TestPt lies relative to the third vertex
							// http://www.blackpawn.com/texts/pointinpoly/default.html
							if (TriBounds.TestPointInTriangle(TestPt))
								{
								Legal = 0;
								break;
								} // if
							} // if
						} // for
					} // else
				} // if
			if (Legal)
				{
				// create a new VectorPolygon and remove the middle vertex from the original
				if (*VPListPtr = new VectorPolygonListDouble())
					{
					if ((*VPListPtr)->MakeVectorPolygon(Pt1, Pt2, Pt3, this, true))
						{
						if (NumNodes == 3)
							break;
						Pt1->NextNode = Pt3;
						if (Pt2 == MyNodes)
							MyNodes = Pt3;
						if (accelerated)
							VNA.DeleteNode(Pt2);	// clear the stashed pointer
						delete Pt2;
						Pt2 = Pt3;
						Pt3 = Pt3->NextNode;
						--NumNodes;
						TryCounter = 0;
						} // if
					else
						{
						Success = 0;
						break;
						} // else
					VPListPtr = (VectorPolygonListDouble **)&(*VPListPtr)->NextPolygonList;
					} // if
				} // if
			} // if
		} // for
	if (accelerated)
		VNA.ClearBins();
	} // if

return (NewVPList);

} // VectorPolygon::Triangulate
*/
/*===========================================================================*/

// This method replaces the one below - it does not form a physical link between the outside and
// inside polygons. It just adds parts to the polygon. No test is done to ensure that the inside polygon is
// fully enclosed in the outer polygon. That exercise is left for the calling function if desired.

bool VectorPolygon::RemoveHoles(VectorPolygonListDouble *&InsideList)
{
VectorPart *CurPart, *SourcePart, **NewPartPtr;
VectorPolygonListDouble *CurVPList;
bool Success = true;

// Find the place to start adding parts
for (CurPart = GetFirstPart(); CurPart->NextPart; CurPart = CurPart->NextPart)
	{
	} // for
NewPartPtr = &CurPart->NextPart;

for (CurVPList = InsideList; CurVPList; CurVPList = (VectorPolygonListDouble *)CurVPList->NextPolygonList)
	{
	// look through the inside polygon's linked nodes and anything linked to the inside polygon
	// change to link to this polygon instead
	CurVPList->MyPolygon->ReplaceLinkedReferences(this);
	for (SourcePart = CurVPList->MyPolygon->GetFirstPart(); SourcePart; SourcePart = SourcePart->NextPart)
		{
		if (SourcePart->NumNodes > 2)
			{
			if (*NewPartPtr = new VectorPart())
				{
				CurPart = *NewPartPtr;
				CurPart->MyNodes = SourcePart->MyNodes;
				SourcePart->MyNodes = NULL;
				CurPart->NumNodes = SourcePart->NumNodes;
				TotalNumNodes += SourcePart->NumNodes;
				SourcePart->NumNodes = 0;
				NewPartPtr = &(*NewPartPtr)->NextPart;
				} // if
			else
				{
				Success = false;
				break;
				} // else
			} // if
		} // for
	} // for

// there shouldn't be anything worthwhile left in this list so delete it all
for (CurVPList = InsideList; CurVPList; CurVPList = InsideList)
	{
	InsideList = (VectorPolygonListDouble *)InsideList->NextPolygonList;
	CurVPList->DeletePolygon();
	delete CurVPList;
	} // for

return (Success);

} // VectorPolygon::RemoveHoles

/*===========================================================================*/

bool VectorPolygon::RemoveConnectingParts(void)
{
VectorPart *OutsidePart, *InsidePart, *PrevPart;
VectorNode *OutsideNode, *InsideNode, *OutsidePointOfContact, *InsidePointOfContact, *StashNode;
unsigned long OutNodeCt, InNodeCt;
bool NoEasyFix = false, Success = true;

// find any inside nodes that are the same as an outside node and make some special geometry for them,
// taking the inside loop into the outside polygon and removing a part
OutsidePart = GetFirstPart();
PrevPart = OutsidePart;
for (InsidePart = OutsidePart->NextPart; InsidePart && ! NoEasyFix; InsidePart = InsidePart->NextPart)
	{
	OutsidePointOfContact = InsidePointOfContact = NULL;
	for (OutsideNode = OutsidePart->FirstNode(), OutNodeCt = 0 && ! NoEasyFix; OutNodeCt < OutsidePart->NumNodes; ++OutNodeCt, OutsideNode = OutsideNode->NextNode)
		{
		for (InsideNode = InsidePart->FirstNode(), InNodeCt = 0; InNodeCt < InsidePart->NumNodes; ++InNodeCt, InsideNode = InsideNode->NextNode)
			{
			if (InsideNode->SamePointLatLon(OutsideNode))
				{
				if (OutsidePointOfContact)
					{
					// one point of contact already found for this part so now we're into deep doodoo.
					NoEasyFix = true;
					break;
					} // if
				OutsidePointOfContact = OutsideNode;
				InsidePointOfContact = InsideNode;
				} // if
			} // for
		} // for
	if (! NoEasyFix && OutsidePointOfContact)
		{
		// alter the structure of the outer polygon to include the inner part.
		StashNode = OutsidePointOfContact->NextNode;
		OutsidePointOfContact->NextNode = InsidePointOfContact->NextNode;
		InsidePointOfContact->NextNode = StashNode;
		OutsidePart->NumNodes += InsidePart->NumNodes;
		InsidePart->MyNodes = NULL;
		InsidePart->NumNodes = 0;
		PrevPart->NextPart = InsidePart->NextPart;
		delete InsidePart;
		InsidePart = PrevPart;
		} // if
	PrevPart = InsidePart;
	} // for

if (NoEasyFix)
	{
	UserMessageOK("Polygon Error", "VNS does not allow an inside part of a polygon to have more than one point of contact with the outside part.");
	Success = false;
	} // if
	
return (Success);

} // VectorPolygon::RemoveConnectingParts

/*===========================================================================*/
/*
// This method is obsolete 10/13/06
// polygons with internal holes will be tied together by bi-directional edges that connect
// outside and inside vectors. nodes are replicated on both inside and outside polygons so that
// directionality is maintained from outside to inside and back to outside again.

//	 _______       _______
//	|  ___  |     |\ ___  |
//	| |   | | ==> | |   | |
//	| |___| |     | |___| |
//	|_______|     |_______|

// an effort to assure that the closest inside polygons to the outside are resolved first
// so that connecting lines don't cross each other or another inside polygon.
// turned out to be prohibitively slow for large numbers of inside polygons and large outside polygons

bool VectorPolygon::RemoveHoles(VectorPolygonListDouble *&InsideList)
{
double dx, dy, NearDist, TestDist;
unsigned long int TestOutNode, TestInNode, FoundInNodeCt, FoundOutNodeCt;
bool Success = true;
VectorPolygonListDouble *CurVPList, *PrevVPList, *FoundList, *LastList;
VectorNode *CurOutNode, *PreviousOutNode, *CurInNode, *FoundOutNode, *FoundInNode, *TempNode, *StashNextOutNode;
VectorNodeLink *CurLinkedNode;

// outside polygon is assumed to be in clockwise or countercw order and 
// the inside polygons to be in the opposite order.
// polygons are assumed to be non self-intersecting and the inside polygons do not intersect each other.

// go through the entire list looking for the closest point in each polygon
for (CurVPList = InsideList, LastList = PrevVPList = FoundList = NULL; CurVPList; CurVPList = (VectorPolygonListDouble *)CurVPList->NextPolygonList)
	{
	if (CurVPList->MyPolygon->NumNodes > 2)
		{
		NearDist = FLT_MAX;
		for (CurOutNode = FirstNode(), TestOutNode = 0; TestOutNode < NumNodes; CurOutNode = CurOutNode->NextNode, ++TestOutNode)
			{
//			if (CurOutNode->FlagCheck(WCS_VECTORNODE_FLAG_HOLEREMOVALSHARE))
//				continue;
			for (CurInNode = CurVPList->MyPolygon->FirstNode(), TestInNode = 0; TestInNode < CurVPList->MyPolygon->NumNodes; CurInNode = CurInNode->NextNode, ++TestInNode)
				{
				// Computations are being done inline to avoid function call overhead.
				// Access Latitude first & Longitude will automatically be in the cache lines.
				// We also access CurOutNode first since it's available sooner from the CPU (on modern CPU's all instructions
				// are done in parallel, and stalls in pipelnes occur if an operation can't proceed because data isn't ready).
				// See: http://arstechnica.com/articles/paedia/cpu/pipelining-2.ars/3
				dy = CurOutNode->Lat - CurInNode->Lat;
				dx = CurOutNode->Lon - CurInNode->Lon;
				TestDist = dy * dy + dx * dx;
				if (TestDist < NearDist)
					{
					NearDist = TestDist;
					} // if
				} // for
			} // for
		CurVPList->DoubleVal = NearDist;
		} // if
	} // for

// connect each polygon in order of closest to furthest
// once connected, remove the polygon from the list
// find the point of the inside polygon nearest a point in the outside polygon
while (InsideList)
	{
	// find the closest polygon left
	NearDist = FLT_MAX;
	for (CurVPList = InsideList, LastList = PrevVPList = FoundList = NULL; CurVPList; CurVPList = (VectorPolygonListDouble *)CurVPList->NextPolygonList)
		{
		if (CurVPList->MyPolygon->NumNodes > 2)
			{
			if ((TestDist = CurVPList->DoubleVal) < NearDist)
				{
				NearDist = TestDist;
				FoundList = CurVPList;
				PrevVPList = LastList;
				} // if
			} // if
		LastList = CurVPList;
		} // for


	NearDist = FLT_MAX;
	FoundInNode = FoundOutNode = NULL;
	if (FoundList)
		{
		double NextAngle, TestAngle;
		TriangleBoundingBoxVector TBxFrom, TBxTo, AngleFinder;
		
		for (PreviousOutNode = FirstNode(), CurOutNode = SecondNode(), TestOutNode = 0; TestOutNode < NumNodes; CurOutNode = CurOutNode->NextNode, ++TestOutNode)
			{
//			if (CurOutNode->FlagCheck(WCS_VECTORNODE_FLAG_HOLEREMOVALSHARE))
//				continue;
			for (CurInNode = FoundList->MyPolygon->FirstNode(), TestInNode = 0; TestInNode < FoundList->MyPolygon->NumNodes; CurInNode = CurInNode->NextNode, ++TestInNode)
				{
				dy = CurOutNode->Lat - CurInNode->Lat;
				dx = CurOutNode->Lon - CurInNode->Lon;
				TestDist = dy * dy + dx * dx;
				if (TestDist < NearDist)
					{
					// see if the angle formed between the previous outside segment and
					// the inside node is less than the angle to the next segment in the
					// outside polygon
					TBxFrom.SetXY(CurOutNode, PreviousOutNode);
					NextAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(CurOutNode->NextNode, CurOutNode));
					TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(CurInNode, CurOutNode));
					if (TestAngle < NextAngle)
						{
						NearDist = TestDist;
						FoundOutNode = CurOutNode;
						FoundInNode = CurInNode;
						FoundInNodeCt = TestInNode;
						FoundOutNodeCt = TestOutNode;
						} // if
					} // if
				} // for
			PreviousOutNode = CurOutNode;
			} // for
		} // if

	if (FoundInNode && FoundOutNode)
		{
		// stash the point following insert point
		StashNextOutNode = FoundOutNode->NextNode;

		// connect outside to inside
		FoundOutNode->NextNode = FoundInNode;

		// replicate connection point on inside polygon
		for (TempNode = FoundInNode; TempNode->NextNode != FoundInNode; TempNode = TempNode->NextNode)
			{
			// nothing to do, just walking
			} // for
		// connect inside to outside
		if (TempNode->NextNode = new VectorNode(FoundInNode))
			{
			TempNode = TempNode->NextNode;

			// link new node to all the nodes the original was linked to
			for (CurLinkedNode = FoundInNode->LinkedNodes; CurLinkedNode; CurLinkedNode = (VectorNodeLink *)CurLinkedNode->NextNodeList)
				{
				// create a link to the linked node
				if (! TempNode->AddCrossLinks(CurLinkedNode->MyNode, this, CurLinkedNode->LinkedPolygon))
					{
					Success = false;
					break;
					} // if
				} // for
				
			// link the new node with the original
			if (! FoundInNode->AddCrossLinks(TempNode, this, this))
				Success = false;
			FoundInNode->FlagSet(WCS_VECTORNODE_FLAG_HOLEREMOVALSHARE);
			TempNode->FlagSet(WCS_VECTORNODE_FLAG_HOLEREMOVALSHARE);

			if (TempNode->NextNode = new VectorNode(FoundOutNode))
				{
				TempNode = TempNode->NextNode;
				TempNode->NextNode = StashNextOutNode;

				// link new node to all the nodes the original was linked to
				for (CurLinkedNode = FoundOutNode->LinkedNodes; CurLinkedNode; CurLinkedNode = (VectorNodeLink *)CurLinkedNode->NextNodeList)
					{
					// create a link to the linked node
					if (! TempNode->AddCrossLinks(CurLinkedNode->MyNode, this, CurLinkedNode->LinkedPolygon))
						{
						Success = false;
						break;
						} // if
					} // for
					
				// link the new node with the original
				if (! FoundOutNode->AddCrossLinks(TempNode, this, this))
					Success = false;
				FoundOutNode->FlagSet(WCS_VECTORNODE_FLAG_HOLEREMOVALSHARE);
				TempNode->FlagSet(WCS_VECTORNODE_FLAG_HOLEREMOVALSHARE);

				// supplement outside points, deplete inside points
				NumNodes += FoundList->MyPolygon->NumNodes + 2;
				FoundList->MyPolygon->NumNodes = 0;
				FoundList->MyPolygon->MyNodes = NULL;
				} // if
			else
				{
				Success = false;
				break;
				} // else
			} // if
		else
			{
			Success = false;
			break;
			} // else

		// remove the resolved polygon from the inside list
		if (PrevVPList)
			PrevVPList->NextPolygonList = FoundList->NextPolygonList;
		else
			InsideList = (VectorPolygonListDouble *)FoundList->NextPolygonList;
		FoundList->DeletePolygon();
		delete FoundList;
		} // if
	else
		{
		// found no more close points
		// this shouldn't happen
		break;
		} // else
	} // while

// there shouldn't be anything left in this list but just in case, clean it up
for (CurVPList = InsideList; CurVPList; CurVPList = InsideList)
	{
	InsideList = (VectorPolygonListDouble *)InsideList->NextPolygonList;
	CurVPList->DeletePolygon();
	delete CurVPList;
	} // for
	
return (Success);

} // VectorPolygon::RemoveHoles
*/
/*===========================================================================*/

double VectorPolygon::NodeToNodeDistanceSquared(VectorNode *Node1, VectorNode *Node2)
{
TriangleBoundingBoxVector Vec;

Vec.SetXY(Node1, Node2);
return (Vec.LengthXYSquared());

} // VectorPolygon::NodeToNodeDistanceSquared

/*===========================================================================*/

// a quicker version that does all the work in this method, and avoids extra constructors & paramater passing
double VectorPolygon::NodeToNodeDistanceSquaredQ(VectorNode *Node1, VectorNode *Node2)
{
double x, y;

// access Latitude first & Longitude will automatically be in the cache lines
y = Node1->Lat - Node2->Lat;
x = Node1->Lon - Node2->Lon;

return (y * y + x * x);

} // VectorPolygon::NodeToNodeDistanceSquaredQ

/*===========================================================================*/

ENUM_TEST_POINT_CONTAINED VectorPolygon::TestPointContained(VectorNode *TestNode, double Tolerance)
{
ENUM_TEST_POINT_CONTAINED Result = WCS_TEST_POINT_CONTAINED_OUTSIDE, TempResult;
unsigned long Contained = 0, NodeCt;
VectorNode *CurNode;
VectorPart *CurPart;

if ((BBox || SetBoundingBox()) && BBox->TestPointInBox(TestNode->Lon, TestNode->Lat))
	{
	for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
		{
		for (CurNode = CurPart->FirstNode(), NodeCt = 0; NodeCt < CurPart->NumNodes; CurNode = CurNode->NextNode, ++NodeCt)
			{
			// see if we are on or inside at least one of the vertices
			if (TestNode->Lon <= CurNode->Lon || TestNode->Lon <= CurNode->NextNode->Lon) // test hoisted from subroutine to eliminate some argument passing
				{
				//TempResult = TestPointRelationToSegment(TestNode, CurNode, CurNode->NextNode, Tolerance);
				TempResult = TestPointRelationToSegment(TestNode, CurNode, Tolerance);
				if (TempResult == WCS_TEST_POINT_CONTAINED_EDGE)
					return (WCS_TEST_POINT_CONTAINED_EDGE);
				if (TempResult == WCS_TEST_POINT_CONTAINED_INSIDE)
					Contained ++;
				} // if
			} // for
		} // for
	Result = Contained % 2 ? WCS_TEST_POINT_CONTAINED_INSIDE: WCS_TEST_POINT_CONTAINED_OUTSIDE;
	} // if

return (Result);

} // VectorPolygon::TestPointContained

/*===========================================================================*/

ENUM_TEST_POINT_CONTAINED VectorPolygon::TestPointContained(VectorNode **NodeLoop, unsigned long NumLoopNodes, 
	VectorNode *TestNode, double Tolerance)
{
ENUM_TEST_POINT_CONTAINED Result = WCS_TEST_POINT_CONTAINED_OUTSIDE, TempResult;
double Crossing;
unsigned long Contained = 0, NodeCt, LastCt = NumLoopNodes - 1;
VectorNode *CurNode, *NextNode;
PolygonBoundingBox TempBox;

TempBox.UpdateBounds(NodeLoop, NumLoopNodes);

if (TempBox.TestPointInBox(TestNode->Lon, TestNode->Lat))
	{
	for (NodeCt = 0; NodeCt < NumLoopNodes; ++NodeCt)
		{
		CurNode = NodeLoop[NodeCt];
		NextNode = NodeCt < LastCt ? NodeLoop[NodeCt + 1]: NodeLoop[0];
		// see if we are on or inside at least one of the vertices
		if (TestNode->Lon <= CurNode->Lon || TestNode->Lon <= NextNode->Lon) // test hoisted from subroutine to eliminate some argument passing
			{
			TempResult = WCS_TEST_POINT_CONTAINED_OUTSIDE;
			if ((TestNode->Lat > CurNode->Lat && TestNode->Lat < NextNode->Lat) || (TestNode->Lat < CurNode->Lat && TestNode->Lat > NextNode->Lat))
				{
				// see if we are clearly inside both vertices
				if (TestNode->Lon < CurNode->Lon && TestNode->Lon < NextNode->Lon)
					TempResult = WCS_TEST_POINT_CONTAINED_INSIDE;
				// see if the vertices are on a vertical line which would mean we are also on that line given the tests above
				else if (CurNode->Lon == NextNode->Lon)
					TempResult = WCS_TEST_POINT_CONTAINED_EDGE;
				else
					{
					// we are inside or on the east or west edge of a box that contains a diagonal line segment.
					// calculate the longitude of the crossing of our extrapolated line with the vector segment
					Crossing = (TestNode->Lat - CurNode->Lat) * ((NextNode->Lon - CurNode->Lon) / (NextNode->Lat - CurNode->Lat)) + CurNode->Lon;
					//DoubleMaskPoint(&Crossing);
					// if we are on the crossing we are on an edge
					if (TestNode->Lon >= Crossing - Tolerance && TestNode->Lon <= Crossing + Tolerance)
						TempResult = WCS_TEST_POINT_CONTAINED_EDGE;
					// are we inside the crossing?
					else if (TestNode->Lon < Crossing)
						TempResult = WCS_TEST_POINT_CONTAINED_INSIDE;
					} // else
				} // if
			else if (TestNode->Lat == CurNode->Lat)
				{
				// see if we're on a horizontal line
				if (CurNode->Lat == NextNode->Lat)
					{
					// see if we are within the line or on either vertex
					if (TestNode->Lon >= CurNode->Lon || TestNode->Lon >= NextNode->Lon)
						TempResult = WCS_TEST_POINT_CONTAINED_EDGE;
					} // if
				// see if we're on the vertex
				else if (TestNode->Lon == CurNode->Lon)
					TempResult = WCS_TEST_POINT_CONTAINED_EDGE;
				} // else if
			else if (TestNode->Lat == NextNode->Lat)
				{
				// see if we're on the vertex
				if (TestNode->Lon == NextNode->Lon)
					TempResult = WCS_TEST_POINT_CONTAINED_EDGE;
				// see if we are on inside of vertex
				else if (TestNode->Lon < NextNode->Lon)
					{
					unsigned long NextNextNodeCt = NodeCt + 2;
					VectorNode *NextNextNode;
					
					// examine the next point to see if there is a reversal of direction or continuation in same direction
					if (NextNextNodeCt >= NumLoopNodes)
						NextNextNodeCt = NextNextNodeCt - NumLoopNodes;
					NextNextNode = NodeLoop[NextNextNodeCt];
					// need a node that is not also on same horizontal as test point
					// if node falls to inside of test point then we are on an edge but it will be discovered
					// in testing a later segment, so fail now
					while (TestNode->Lat == NextNextNode->Lat && NextNextNode->Lon > TestNode->Lon)
						{
						++NextNextNodeCt;
						if (NextNextNodeCt >= NumLoopNodes)
							NextNextNodeCt = NextNextNodeCt - NumLoopNodes;
						NextNextNode = NodeLoop[NextNextNodeCt];
						} // while
					// continuation of direction is required to make this test "inside"
					// reversal of direction indicates a terminal branch of the shape that the current test point
					// is merely tangent to the ultimate tip. On cannot be "inside" such a tip but one could be on the
					// edge which is discovered above or on a different segment.

					//        /           /                   x      ___   x         //
					//       /  x     ___/  x             /\        /   \            //
					//      /        /                   /  \      /     \           //
					//   two versions of continuity         reversals                //

					if (TestNode->Lat < CurNode->Lat && TestNode->Lat > NextNextNode->Lat)
						TempResult = WCS_TEST_POINT_CONTAINED_INSIDE;
					else if (TestNode->Lat > CurNode->Lat && TestNode->Lat < NextNextNode->Lat)
						TempResult = WCS_TEST_POINT_CONTAINED_INSIDE;
					} // if
				} // else if
			if (TempResult == WCS_TEST_POINT_CONTAINED_EDGE)
				return (WCS_TEST_POINT_CONTAINED_EDGE);
			if (TempResult == WCS_TEST_POINT_CONTAINED_INSIDE)
				++Contained;
			} // if
		} // for
	Result = Contained % 2 ? WCS_TEST_POINT_CONTAINED_INSIDE: WCS_TEST_POINT_CONTAINED_OUTSIDE;
	} // if

return (Result);

} // VectorPolygon::TestPointContained

/*===========================================================================*/

//ENUM_TEST_POINT_CONTAINED VectorPolygon::TestPointRelationToSegment(VectorNode *TestNode, VectorNode *CurNode, 
//	VectorNode *NextNode, double Tolerance)
ENUM_TEST_POINT_CONTAINED VectorPolygon::TestPointRelationToSegment(VectorNode *TestNode, VectorNode *CurNode, double Tolerance)
{
double Crossing;
VectorNode *NextNode = CurNode->NextNode, *NextNextNode;
// this takes a point and extrapolates a line to infinity in one direction (+Lon)
// then determines if it intersects a line segment.
// The function is used on each segment of a polygon to test whether the point is contained.
// The function should work in all cases so long as all segments are tested.
// Results are independent of polygon direction.
// It is assumed that each node has a next node in an endless loop.

// see if we are on or inside at least one of the vertices
//if (TestNode->Lon <= CurNode->Lon || TestNode->Lon <= NextNode->Lon)
//	{
	// see if we are within the latitude range but not equal to either vertex lat
	// if we are then we know that the two vertex latitudes are different and can be used as a divisor
	if ((TestNode->Lat > CurNode->Lat && TestNode->Lat < NextNode->Lat) || (TestNode->Lat < CurNode->Lat && TestNode->Lat > NextNode->Lat))
		{
		// see if we are clearly inside both vertices
		if (TestNode->Lon < CurNode->Lon && TestNode->Lon < NextNode->Lon)
			return (WCS_TEST_POINT_CONTAINED_INSIDE);
		// see if the vertices are on a vertical line which would mean we are also on that line given the tests above
		if (CurNode->Lon == NextNode->Lon)
			return (WCS_TEST_POINT_CONTAINED_EDGE);
		// we are inside or on the east or west edge of a box that contains a diagonal line segment.
		// calculate the longitude of the crossing of our extrapolated line with the vector segment
		Crossing = (TestNode->Lat - CurNode->Lat) * ((NextNode->Lon - CurNode->Lon) / (NextNode->Lat - CurNode->Lat)) + CurNode->Lon;
		//DoubleMaskPoint(&Crossing);
		// if we are on the crossing we are on an edge
		if (TestNode->Lon >= Crossing - Tolerance && TestNode->Lon <= Crossing + Tolerance)
			return (WCS_TEST_POINT_CONTAINED_EDGE);
		// are we inside the crossing?
		if (TestNode->Lon < Crossing)
			return (WCS_TEST_POINT_CONTAINED_INSIDE);
		} // if
	else if (TestNode->Lat == CurNode->Lat)
		{
		// see if we're on a horizontal line
		if (CurNode->Lat == NextNode->Lat)
			{
			// see if we are within the line or on either vertex
			if (TestNode->Lon >= CurNode->Lon || TestNode->Lon >= NextNode->Lon)
				return (WCS_TEST_POINT_CONTAINED_EDGE);
			} // if
		// see if we're on the vertex
		else if (TestNode->Lon == CurNode->Lon)
			return (WCS_TEST_POINT_CONTAINED_EDGE);
		} // else if
	else if (TestNode->Lat == NextNode->Lat)
		{
		// see if we're on the vertex
		if (TestNode->Lon == NextNode->Lon)
			return (WCS_TEST_POINT_CONTAINED_EDGE);
		// see if we are on inside of vertex
		if (TestNode->Lon < NextNode->Lon)
			{
			// examine the next point to see if there is a reversal of direction or continuation in same direction
			NextNextNode = NextNode->NextNode;
			// need a node that is not also on same horizontal as test point
			// if node falls to inside of test point then we are on an edge but it will be discovered
			// in testing a later segment, so fail now
			while (TestNode->Lat == NextNextNode->Lat && NextNextNode->Lon > TestNode->Lon)
				{
				NextNextNode = NextNextNode->NextNode;
				} // while
			// continuation of direction is required to make this test "inside"
			// reversal of direction indicates a terminal branch of the shape that the current test point
			// is merely tangent to the ultimate tip. On cannot be "inside" such a tip but one could be on the
			// edge which is discovered above or on a different segment.

			//        /           /                   x      ___   x         //
			//       /  x     ___/  x             /\        /   \            //
			//      /        /                   /  \      /     \           //
			//   two versions of continuity         reversals                //

			if (TestNode->Lat < CurNode->Lat && TestNode->Lat > NextNextNode->Lat)
				return (WCS_TEST_POINT_CONTAINED_INSIDE);
			if (TestNode->Lat > CurNode->Lat && TestNode->Lat < NextNextNode->Lat)
				return (WCS_TEST_POINT_CONTAINED_INSIDE);
			} // if
		} // else if
//	} // if

// sorry Charlie.
return (WCS_TEST_POINT_CONTAINED_OUTSIDE);

} // VectorPolygon::TestPointRelationToSegment

/*===========================================================================*/

bool VectorPolygon::TestNodeOnOrNearSegment(VectorNode *TestNode, VectorNode *CurNode, VectorNode *NextNode, 
	double &NodeLat, double &NodeLon, double Tolerance)
{
double Crossing;

// returns true only if the test point is within Tolerance distance of the segment in one direction
// and not within or equal to Tolerance distance from either end node

if (CurNode->Lon == NextNode->Lon)
	{
	// segment is vertical
	if (TestNode->Lon >= CurNode->Lon - Tolerance && TestNode->Lon <= CurNode->Lon + Tolerance)
		{
		// within correct longitude range
		if ((TestNode->Lat > CurNode->Lat + Tolerance && TestNode->Lat < NextNode->Lat - Tolerance)
			|| (TestNode->Lat > NextNode->Lat + Tolerance && TestNode->Lat < CurNode->Lat - Tolerance))
			{
			// within correct latitude range excluding the Tolerance distance from end nodes
			NodeLat = TestNode->Lat;
			NodeLon = CurNode->Lon;
			return (true);
			} // if
		} // if
	} // if
else if (CurNode->Lat == NextNode->Lat)
	{
	// segment is horizontal
	if (TestNode->Lat >= CurNode->Lat - Tolerance && TestNode->Lat <= CurNode->Lat + Tolerance)
		{
		// within correct latitude range
		if ((TestNode->Lon > CurNode->Lon + Tolerance && TestNode->Lon < NextNode->Lon - Tolerance)
			|| (TestNode->Lon > NextNode->Lon + Tolerance && TestNode->Lon < CurNode->Lon - Tolerance))
			{
			// within correct longitude range excluding the Tolerance distance from end nodes
			NodeLat = CurNode->Lat;
			NodeLon = TestNode->Lon;
			return (true);
			} // if
		} // if
	} // if
else if (fabs(CurNode->Lon - NextNode->Lon) > fabs(CurNode->Lat - NextNode->Lat))
	{
	// longitude range is larger
	if ((TestNode->Lon > CurNode->Lon + Tolerance && TestNode->Lon < NextNode->Lon - Tolerance)
		|| (TestNode->Lon > NextNode->Lon + Tolerance && TestNode->Lon < CurNode->Lon - Tolerance))
		{
		Crossing = (TestNode->Lon - CurNode->Lon) * ((NextNode->Lat - CurNode->Lat) / (NextNode->Lon - CurNode->Lon)) + CurNode->Lat;
		if (TestNode->Lat >= Crossing - Tolerance && TestNode->Lat <= Crossing + Tolerance)
			{
			NodeLat = Crossing;
			NodeLon = TestNode->Lon;
			return (true);
			} // if
		} // if
	} // else if
else
	{
	// latitude range is larger
	if ((TestNode->Lat > CurNode->Lat + Tolerance && TestNode->Lat < NextNode->Lat - Tolerance)
		|| (TestNode->Lat > NextNode->Lat + Tolerance && TestNode->Lat < CurNode->Lat - Tolerance))
		{
		Crossing = (TestNode->Lat - CurNode->Lat) * ((NextNode->Lon - CurNode->Lon) / (NextNode->Lat - CurNode->Lat)) + CurNode->Lon;
		if (TestNode->Lon >= Crossing - Tolerance && TestNode->Lon <= Crossing + Tolerance)
			{
			NodeLat = TestNode->Lat;
			NodeLon = Crossing;
			return (true);
			} // if
		} // if
	} // else

return (false);

} // VectorPolygon::TestNodeOnOrNearSegment

/*===========================================================================*/

ENUM_BOUNDING_BOX_CONTAINED VectorPolygon::TestBoxContained(PolygonBoundingBox *IsItInsideMe)
{
ENUM_BOUNDING_BOX_CONTAINED rVal = WCS_BOUNDING_BOX_CONTAINED_OUTSIDE;

if (BBox || SetBoundingBox())
	rVal = BBox->TestBoxContained(IsItInsideMe);

return (rVal);

} // VectorPolygon::TestBoxContained

/*===========================================================================*/

//#define GORILLA

bool VectorPolygon::TrulyEncloses(VectorPolygon *AmIEnclosed)
{
#ifdef GORILLA
double iMaxX, iMaxY, iMinX, iMinY;
#endif // GORILLA
double Tolerance, NodeLat, NodeLon;
#ifdef GORILLA
VectorNode *outNodes[65535];
#endif // GORILLA
VectorNode *Max1XNode, *Min1XNode, *Max1YNode, *Min1YNode, *Max2XNode, *Min2XNode, *Max2YNode, *Min2YNode;
VectorNode *InnerNode, *OuterNode;
VectorPart *InnerPart, *OuterPart;
//static unsigned long callnum = 0;
//unsigned long trapcall = 11;
unsigned long InnerNodeCt, OuterNodeCt;
unsigned long outCount = 0;
VectorIntersecter VI;	// Hoisted out so constructor doesn't get called all the time!
//char dmsg[256];
bool ContactFound, rVal = false;

//callnum++;
//sprintf(dmsg, "Call %d\n", callnum);
//OutputDebugString(dmsg);

//if (callnum == trapcall)
//	printf("here");

Tolerance = WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;

#ifdef GORILLA
// determine region for intersection testing
iMaxX = AmIEnclosed->BBox->MaxX + WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
iMinX = AmIEnclosed->BBox->MinX - WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
iMaxY = AmIEnclosed->BBox->MaxY + WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
iMinY = AmIEnclosed->BBox->MinY - WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;

// make a list of only those nodes that are in the intersection region
for (OuterPart = GetFirstPart(); OuterPart; OuterPart = OuterPart->NextPart)
	{
	for (OuterNode = OuterPart->FirstNode(), OuterNodeCt = 0; OuterNodeCt < OuterPart->NumNodes; ++OuterNodeCt, OuterNode = OuterNode->NextNode)
		{
		unsigned long xflags = 0, yflags = 0;
		bool test = true;

#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
		//DevCounter[45]++;
#endif // FRANK or GARY
		_mm_prefetch((char *)OuterNode->NextNode, _MM_HINT_T0);	// SSE intrinsic - requires at least SSE build support
		/***
		if ((OuterNode->Lat < iMinY) || (OuterNode->NextNode->Lat < iMinY))
			yflags |= 4;	// bottom
		if ((OuterNode->Lat > iMaxY) || (OuterNode->NextNode->Lat > iMaxY))
			yflags |= 8;	// top
		if ((OuterNode->Lat >= iMinY) && (OuterNode->Lat <= iMaxY) ||
			(OuterNode->NextNode->Lat >= iMinY) && (OuterNode->NextNode->Lat <= iMaxY))
			yflags |= 32;	// interior
		if ((OuterNode->Lon < iMinX) || (OuterNode->NextNode->Lon < iMinX))
			xflags |= 1;	// left side
		if ((OuterNode->Lon > iMaxX) || (OuterNode->NextNode->Lon > iMaxX))
			xflags |= 2;	// right side
		if ((OuterNode->Lon >= iMinX) && (OuterNode->Lon <= iMaxX) ||
			(OuterNode->NextNode->Lon >= iMinX) && (OuterNode->NextNode->Lon <= iMaxX))
			xflags |= 16;	// interior
		***/
		yflags |= ((OuterNode->Lat < iMinY) || (OuterNode->NextNode->Lat < iMinY)) << 2;	// bottom
		yflags |= ((OuterNode->Lat > iMaxY) || (OuterNode->NextNode->Lat > iMaxY)) << 3;	// top
		yflags |= ((OuterNode->Lat >= iMinY) && (OuterNode->Lat <= iMaxY) ||
			(OuterNode->NextNode->Lat >= iMinY) && (OuterNode->NextNode->Lat <= iMaxY)) << 5;	// interior
		xflags |= ((OuterNode->Lon < iMinX) || (OuterNode->NextNode->Lon < iMinX));			// left side
		xflags |= ((OuterNode->Lon > iMaxX) || (OuterNode->NextNode->Lon > iMaxX)) << 1;	// right side
		xflags |= ((OuterNode->Lon >= iMinX) && (OuterNode->Lon <= iMaxX) ||
			(OuterNode->NextNode->Lon >= iMinX) && (OuterNode->NextNode->Lon <= iMaxX)) << 4;	// interior

		if ((xflags == 1) || (xflags == 2) || (yflags == 4) || (yflags == 8))	// if completely outside
			test = false;

		if (test)
			{
			//if (callnum == trapcall)
			//	{
			//	sprintf(dmsg, "Tabling Outer Node %d (%p)\n", OuterNodeCt, OuterNode);
			//	OutputDebugString(dmsg);
			//	} // if
			outNodes[outCount++] = OuterNode;
			} // if
		} // for
	} // for
#endif // GORILLA

for (InnerPart = AmIEnclosed->GetFirstPart(); InnerPart; InnerPart = InnerPart->NextPart)
	{
	for (InnerNode = InnerPart->FirstNode(), InnerNodeCt = 0; InnerNodeCt < InnerPart->NumNodes; ++InnerNodeCt, InnerNode = InnerNode->NextNode)
		{
		_mm_prefetch((char *)InnerNode->NextNode, _MM_HINT_T0);	// SSE intrinsic - requires at least SSE build support
		//if (callnum == trapcall)
		//	{
		//	sprintf(dmsg, "+ Testing Inner Node #%d\n", InnerNodeCt);
		//	OutputDebugString(dmsg);
		//	} // if
//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
		//DevCounter[11]++;
//#endif // FRANK or GARY
		// The prefetch will attempt to grab our next data & put it in the L1 cache while the rest of the computations are being done
		// The prefetch brings in an entire cache line, which is 64 bytes of data, so there's no need to actually fetch a specific piece from structures
		Max1XNode = InnerNode->Lon > InnerNode->NextNode->Lon ? InnerNode: InnerNode->NextNode;
		Min1XNode = Max1XNode == InnerNode ? InnerNode->NextNode: InnerNode;
		Max1YNode = InnerNode->Lat > InnerNode->NextNode->Lat ? InnerNode: InnerNode->NextNode;
		Min1YNode = Max1YNode == InnerNode ? InnerNode->NextNode: InnerNode;
#ifdef GORILLA
		for (unsigned long outloop = 0; outloop < outCount; outloop++)
			{
			OuterNode = outNodes[outloop];
#else // GORILLA
		for (OuterPart = GetFirstPart(); OuterPart; OuterPart = OuterPart->NextPart)
			{
			for (OuterNode = OuterPart->FirstNode(), OuterNodeCt = 0; OuterNodeCt < OuterPart->NumNodes; ++OuterNodeCt, OuterNode = OuterNode->NextNode)
				{
#endif // GORILLA
				_mm_prefetch((char *)OuterNode->NextNode, _MM_HINT_T0);	// SSE intrinsic - requires at least SSE build support
				// The prefetch will attempt to grab our next data & put it in the L1 cache while the rest of the computations are being done
				// The prefetch brings in an entire cache line, which is 64 bytes of data, so there's no need to actually fetch a specific piece from structures
				Max2XNode = OuterNode->Lon > OuterNode->NextNode->Lon ? OuterNode: OuterNode->NextNode;
				Min2XNode = Max2XNode == OuterNode ? OuterNode->NextNode: OuterNode;
				Max2YNode = OuterNode->Lat > OuterNode->NextNode->Lat ? OuterNode: OuterNode->NextNode;
				Min2YNode = Max2YNode == OuterNode ? OuterNode->NextNode: OuterNode;
				//if (callnum == trapcall)
				//	{
				//	sprintf(dmsg, "x Testing Outer Node %p\n", OuterNode);
				//	OutputDebugString(dmsg);
				//	} // if
//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
				//DevCounter[49]++;
//#endif // FRANK or GARY
				if ((Min2XNode->Lon <= Max1XNode->Lon + Tolerance && Max2XNode->Lon >= Min1XNode->Lon - Tolerance) &&
					(Min2YNode->Lat <= Max1YNode->Lat + Tolerance && Max2YNode->Lat >= Min1YNode->Lat - Tolerance))
					//(VI.TestSegmentsTouch(AmIEnclosed, InnerNode, OuterNode, Max1XNode, Min1XNode, Max1YNode, Min1YNode, Max2XNode, Min2XNode, Max2YNode, Min2YNode)))
					{
					ContactFound = false;

					//if (callnum == trapcall)
					//	{
					//	sprintf(dmsg, "*** Intersecting %p\n", OuterNode);
					//	OutputDebugString(dmsg);
					//	} // if
//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
					//DevCounter[10]++;
//#endif // FRANK or GARY
					//if (callnum == trapcall)
					//	{
					//	sprintf(dmsg, "Inner = %p, Outer = %p\n", InnerNode, OuterNode);
					//	OutputDebugString(dmsg);
					//	} // if
					// are any of the vertices the same?
					if (InnerNode->SamePointLatLon(OuterNode))
						{
						//sprintf(dmsg, "--> Pass 30\n");
						//OutputDebugString(dmsg);
						//DevCounter[30]++;
						ContactFound = true;
						} // if
					else if (OuterNode->SamePointLatLon(InnerNode->NextNode))
						{
						//sprintf(dmsg, "--> Pass 31\n");
						//OutputDebugString(dmsg);
						//DevCounter[31]++;
						ContactFound = true;
						} // else if
					else if (InnerNode->SamePointLatLon(OuterNode->NextNode))
						{
						//sprintf(dmsg, "--> Pass 32\n");
						//OutputDebugString(dmsg);
						//DevCounter[32]++;
						ContactFound = true;
						} // else if
					else if (Min2XNode->Lon < Max1XNode->Lon && Max2XNode->Lon > Min1XNode->Lon && Min2YNode->Lat < Max1YNode->Lat && Max2YNode->Lat > Min1YNode->Lat
						&& VI.FindIntersectionContained(InnerNode, OuterNode, Min1XNode, Max1XNode, Min2XNode, Max2XNode, 
						Min1YNode, Max1YNode, Min2YNode, Max2YNode, NodeLat, NodeLon)) 
						{
						//sprintf(dmsg, "--> Pass 33\n");
						//OutputDebugString(dmsg);
						//sprintf(dmsg, "VI.FIC with Inner %p, Outer %p\n", InnerNode, OuterNode);
						//OutputDebugString(dmsg);
						//DevCounter[33]++;
						// intersection falls between end nodes on both segments
						ContactFound = true;
						} // else if
					else if (AmIEnclosed->TestNodeOnOrNearSegment(InnerNode, OuterNode, OuterNode->NextNode, NodeLat, NodeLon, Tolerance))
						{
						//sprintf(dmsg, "--> Pass 34\n");
						//OutputDebugString(dmsg);
						//DevCounter[34]++;
						ContactFound = true;
						} // else if
					else if (AmIEnclosed->TestNodeOnOrNearSegment(OuterNode, InnerNode, InnerNode->NextNode, NodeLat, NodeLon, Tolerance))
						{
						//sprintf(dmsg, "--> Pass 35\n");
						//OutputDebugString(dmsg);
						//DevCounter[35]++;
						ContactFound = true;
						} // else if
					else if (InnerNode->SimilarPointLatLon(OuterNode, Tolerance))
						{
						//sprintf(dmsg, "--> Pass 36\n");
						//OutputDebugString(dmsg);
						//DevCounter[36]++;
						ContactFound = true;
						} // else if
					// some kind of contact has been made
					if (ContactFound)
						{
						//sprintf(dmsg, "+ Contact @%d\n", callnum);
						//OutputDebugString(dmsg);
						//DevCounter[37]++;
						return(rVal);	// false
						} // if
					} // if
#ifndef GORILLA
				} // for
#endif // !GORILLA
			} // for
		} // for
	} // for

// if there are no intersections and at least one of the vertices is inside the other polygon then it must 
// be a fully contained polygon

// test to see if a node of Smaller is inside Larger, any node will do for the test. Since we know there are
// no intersections all nodes are either inside or outside
if (TestPointContained(AmIEnclosed->PolyFirstNode(), 0.0) == WCS_TEST_POINT_CONTAINED_INSIDE)
	{
	//sprintf(dmsg, "O Contained @%d\n", callnum);
	//OutputDebugString(dmsg);
	//DevCounter[38]++;
	rVal = true;
	} // if

//DevCounter[39]++;
return(rVal);	// false unless set by preceding test

} // VectorPolygon::TrulyEncloses

/*===========================================================================*/

bool VectorPolygon::PolygonEnclosesPart(VectorPart *InnerPart, VectorPolygon *InnerPolygon)
{
//double iMaxX, iMaxY, iMinX, iMinY;
double Tolerance, NodeLat, NodeLon;
//VectorNode *info[4096];
VectorNode *Max1XNode, *Min1XNode, *Max1YNode, *Min1YNode, *Max2XNode, *Min2XNode, *Max2YNode, *Min2YNode;
VectorNode *InnerNode, *OuterNode;
VectorPart *OuterPart;
unsigned long InnerNodeCt, OuterNodeCt;
//unsigned long outCount = 0;
bool ContactFound;

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//DevCounter[6]++;
//#endif // FRANK or GARY

Tolerance = WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;

// determine region of intersection
//if (this->BBox == NULL)
//	this->SetBoundingBox();

//iMaxX = _MIN(this->BBox->MaxX, InnerPolygon->BBox->MaxX) + WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
//iMinX = _MAX(this->BBox->MinX, InnerPolygon->BBox->MinX) - WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
//iMaxY = _MIN(this->BBox->MaxY, InnerPolygon->BBox->MaxY) + WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
//iMinY = _MAX(this->BBox->MinY, InnerPolygon->BBox->MinY) - WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;

//OuterPart = GetFirstPart();
//for (OuterNode = OuterPart->FirstNode(), OuterNodeCt = 0; OuterNodeCt < OuterPart->NumNodes; ++OuterNodeCt, OuterNode = OuterNode->NextNode)
//	{
//	if (((OuterNode->Lon >= iMinX) && (OuterNode->Lon <= iMaxX)) || ((OuterNode->NextNode->Lon >= iMinX) && (OuterNode->NextNode->Lon <= iMaxX)) ||
//		((OuterNode->Lat >= iMinY) && (OuterNode->Lat <= iMaxY)) || ((OuterNode->NextNode->Lat >= iMinY) && (OuterNode->NextNode->Lat <= iMaxY)))
//		{
//		info[outCount++] = OuterNode;
//		} // if
//	} // for

for (InnerNode = InnerPart->FirstNode(), InnerNodeCt = 0; InnerNodeCt < InnerPart->NumNodes; ++InnerNodeCt, InnerNode = InnerNode->NextNode)
	{
	_mm_prefetch((char *)InnerNode->NextNode, _MM_HINT_T0);	// SSE intrinsic - requires at least SSE build support
//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//	DevCounter[7]++;
//#endif // FRANK or GARY
	// The prefetch will attempt to grab our next data & put it in the L1 cache while the rest of the computations are being done
	// The prefetch brings in an entire cache line, which is 64 bytes of data, so there's no need to actually fetch a specific piece from structures
	OuterPart = GetFirstPart();
	Max1XNode = InnerNode->Lon > InnerNode->NextNode->Lon ? InnerNode: InnerNode->NextNode;
	Min1XNode = Max1XNode == InnerNode ? InnerNode->NextNode: InnerNode;
	Max1YNode = InnerNode->Lat > InnerNode->NextNode->Lat ? InnerNode: InnerNode->NextNode;
	Min1YNode = Max1YNode == InnerNode ? InnerNode->NextNode: InnerNode;
	for (OuterNode = OuterPart->FirstNode(), OuterNodeCt = 0; OuterNodeCt < OuterPart->NumNodes; ++OuterNodeCt, OuterNode = OuterNode->NextNode)
	//for (unsigned long loopy = 0; loopy < outCount; loopy++)
		{
	//	OuterNode = info[loopy];
//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//		DevCounter[8]++;
//#endif // FRANK or GARY
		_mm_prefetch((char *)OuterNode->NextNode, _MM_HINT_T0);	// SSE intrinsic - requires at least SSE build support
		// The prefetch will attempt to grab our next data & put it in the L1 cache while the rest of the computations are being done
		// The prefetch brings in an entire cache line, which is 64 bytes of data, so there's no need to actually fetch a specific piece from structures
		Max2XNode = OuterNode->Lon > OuterNode->NextNode->Lon ? OuterNode: OuterNode->NextNode;
		Min2XNode = Max2XNode == OuterNode ? OuterNode->NextNode: OuterNode;
		Max2YNode = OuterNode->Lat > OuterNode->NextNode->Lat ? OuterNode: OuterNode->NextNode;
		Min2YNode = Max2YNode == OuterNode ? OuterNode->NextNode: OuterNode;
		if ((Min2XNode->Lon <= Max1XNode->Lon + Tolerance && Max2XNode->Lon >= Min1XNode->Lon - Tolerance) &&
			(Min2YNode->Lat <= Max1YNode->Lat + Tolerance && Max2YNode->Lat >= Min1YNode->Lat - Tolerance))
			//(VI.TestSegmentsTouch(InnerPolygon, InnerNode, OuterNode, Max1XNode, Min1XNode, Max1YNode, Min1YNode, Max2XNode, Min2XNode, Max2YNode, Min2YNode)))
			{
			VectorIntersecter VI;

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//			DevCounter[9]++;
//#endif // FRANK or GARY
			ContactFound = false;
			// are any of the vertices the same?
			if (InnerNode->SamePointLatLon(OuterNode))
				{
				ContactFound = true;
				} // if
			else if (OuterNode->SamePointLatLon(InnerNode->NextNode))
				{
				ContactFound = true;
				} // else if
			else if (InnerNode->SamePointLatLon(OuterNode->NextNode))
				{
				ContactFound = true;
				} // else if
			else if (Min2XNode->Lon < Max1XNode->Lon && Max2XNode->Lon > Min1XNode->Lon && Min2YNode->Lat < Max1YNode->Lat && Max2YNode->Lat > Min1YNode->Lat
				&& VI.FindIntersectionContained(InnerNode, OuterNode, Min1XNode, Max1XNode, Min2XNode, Max2XNode, 
				Min1YNode, Max1YNode, Min2YNode, Max2YNode, NodeLat, NodeLon)) 
				{
				// intersection falls between end nodes on both segments
				ContactFound = true;
				} // else if
			else if (InnerPolygon->TestNodeOnOrNearSegment(InnerNode, OuterNode, OuterNode->NextNode, NodeLat, NodeLon, Tolerance))
				{
				ContactFound = true;
				} // else if
			else if (InnerPolygon->TestNodeOnOrNearSegment(OuterNode, InnerNode, InnerNode->NextNode, NodeLat, NodeLon, Tolerance))
				{
				ContactFound = true;
				} // else if
			else if (InnerNode->SimilarPointLatLon(OuterNode, Tolerance))
				{
				ContactFound = true;
				} // else if
			// some kind of contact has been made
			if (ContactFound)
				return (false);
			} // if
		} // for
	} // for
	
// if there are no intersections and at least one of the vertices is inside the other polygon then it must 
// be a fully contained polygon

// test to see if a node of Smaller is inside Larger, any node will do for the test. Since we know there are
// no intersections all nodes are either inside or outside
if (TestPointContained(InnerPart->FirstNode(), 0.0) == WCS_TEST_POINT_CONTAINED_INSIDE)
	{
	return (true);
	} // if

return (false);

} // VectorPolygon::PolygonEnclosesPart

/*===========================================================================*/

void VectorPolygon::SetLinkedPolygonClonesToNULL(void)
{
unsigned long NodeCt;
VectorNode *CurNode;
VectorNodeLink *CurLinkedNode;
VectorPart *CurPart;

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; CurNode = CurNode->NextNode, ++NodeCt)
		{
		for (CurLinkedNode = CurNode->LinkedNodes; CurLinkedNode; CurLinkedNode = CurLinkedNode ? (VectorNodeLink *)CurLinkedNode->NextNodeList: CurNode->LinkedNodes)
			{
			if (CurLinkedNode->LinkedPolygon)
				CurLinkedNode->LinkedPolygon->CloneOfThis = NULL;
			} // for
		} // for
	} // for

} // VectorPolygon::SetLinkedPolygonClonesToNULL

/*===========================================================================*/

void VectorPolygon::ReplaceNodeLinkPolygons(void)
{
VectorNode *CurNode, *NodeInOld, *NodeInNew;
VectorNodeLink *LastLinkedNode, *CurLinkedNode, *TempNodeLink;
VectorPart *CurPart, *PartInOld, *PartInNew;
unsigned long NodeCt, NodeInOldCt;
bool FoundNode;

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//DevCounter[12]++;

//char dmsg[256];

//sprintf(dmsg, "RNLP: %d\n", this->TotalNumNodes);
//OutputDebugString(dmsg);
//#endif // FRANK or GARY

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; CurNode = CurNode->NextNode, ++NodeCt)
		{
		// set the flag to tell that this is a vector-derived node
		// the flag will be sued to speed up effect evaluation.
		CurNode->FlagSet(WCS_VECTORNODE_FLAG_VECTORNODE);
		LastLinkedNode = NULL;
		for (CurLinkedNode = CurNode->LinkedNodes; CurLinkedNode; CurLinkedNode = CurLinkedNode ? (VectorNodeLink *)CurLinkedNode->NextNodeList: CurNode->LinkedNodes)
			{
			// linked polygon is CurLinkedNode->LinkedPolygon
			// copy of linked polygon is CurLinkedNode->LinkedPolygon->CloneOfThis
			if (CurLinkedNode->LinkedPolygon && CurLinkedNode->LinkedPolygon->CloneOfThis)
				{
				// find the new node in CurLinkedNode->LinkedPolygon->CloneOfThis that has the same
				// node count as CurLinkedNode->MyNode and the same part number
				_mm_prefetch((char *)CurLinkedNode->MyNode, _MM_HINT_T0);	// SSE intrinsic - requires at least SSE build support
				FoundNode = false;
				for (PartInOld = CurLinkedNode->LinkedPolygon->GetFirstPart(), 
					PartInNew = CurLinkedNode->LinkedPolygon->CloneOfThis->GetFirstPart(); 
					PartInOld && PartInNew && ! FoundNode; PartInOld = PartInOld->NextPart, PartInNew = PartInNew->NextPart)
					{
					for (NodeInOldCt = 0, NodeInOld = PartInOld->FirstNode(), 
						NodeInNew = PartInNew->FirstNode(); 
						NodeInOldCt < PartInOld->NumNodes;
						NodeInOld = NodeInOld->NextNode, NodeInNew = NodeInNew->NextNode, ++NodeInOldCt)
						{
						_mm_prefetch((char *)NodeInNew->NextNode, _MM_HINT_T0);	// SSE intrinsic - requires at least SSE build support
//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//						DevCounter[14]++;
//#endif // FRANK or GARY
						if (NodeInOld == CurLinkedNode->MyNode)
							{
//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//							DevCounter[19]++;
//#endif // FRANK or GARY
							CurLinkedNode->MyNode = NodeInNew;
							CurLinkedNode->LinkedPolygon = CurLinkedNode->LinkedPolygon->CloneOfThis;
							FoundNode = true;
							break;
							} // if
						} // for
					} // for
				} // if
			else
				{
				// remove this link from the polygon copy
				TempNodeLink = CurLinkedNode;
				if (LastLinkedNode)
					{
					LastLinkedNode->NextNodeList = (VectorNodeLink *)CurLinkedNode->NextNodeList;
					CurLinkedNode = LastLinkedNode;
					} // if
				else
					{
					CurNode->LinkedNodes = (VectorNodeLink *)CurLinkedNode->NextNodeList;
					CurLinkedNode = NULL;
					} // else
				delete TempNodeLink;
				} // else
			LastLinkedNode = CurLinkedNode;
			} // for
		} // for
	} // for
	
} // VectorPolygon::ReplaceNodeLinkPolygons

/*===========================================================================*/

// This searches the linked nodes in a polygon looking for instances of linkage to this 
// polygon and when found, replaces the linked polygon pointer with a different one.
// Note that there is an assumption that the nodes in the new polygon are the same nodes as in the old polygon,
// not that new nodes are being created.
void VectorPolygon::ReplaceLinkedReferences(VectorPolygon *Replacement)
{
unsigned long NodeCt;
VectorNode *CurNode, *ActualLinkedNode;
VectorNodeLink *ForLinkedNode, *BackLinkedNode;
VectorPart *CurPart;

for (CurPart = GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
	{
	for (NodeCt = 0, CurNode = CurPart->MyNodes; NodeCt < CurPart->NumNodes; CurNode = CurNode->NextNode, ++NodeCt)
		{
		for (ForLinkedNode = CurNode->LinkedNodes; ForLinkedNode; ForLinkedNode = (VectorNodeLink *)ForLinkedNode->NextNodeList)
			{
			ActualLinkedNode = ForLinkedNode->MyNode;
			for (BackLinkedNode = ActualLinkedNode->LinkedNodes; BackLinkedNode; BackLinkedNode = (VectorNodeLink *)BackLinkedNode->NextNodeList)
				{
				// linked polygon is CurLinkedNode->LinkedPolygon
				if (BackLinkedNode->LinkedPolygon == this)
					BackLinkedNode->LinkedPolygon = Replacement;
				} // for
			} // for
		} // for
	} // for
	
} // VectorPolygon::ReplaceLinkedReferences

/*===========================================================================*/

bool VectorPolygon::CompareEffectsSame(EffectJoeList *CompareMe)
{
EffectJoeList *FirstEffect, *SecondEffect;
bool FoundOne = true;

for (FirstEffect = MyEffects; FirstEffect && FoundOne; FirstEffect = FirstEffect->Next)
	{
	FoundOne = false;
	for (SecondEffect = CompareMe; SecondEffect; SecondEffect = SecondEffect->Next)
		{
		if (FirstEffect->MyEffect == SecondEffect->MyEffect && FirstEffect->MyJoe == SecondEffect->MyJoe)
			{
			FoundOne = true;
			break;
			} // if
		} // for
	} // for

return (FoundOne);

} // VectorPolygon::CompareEffectsSame

/*===========================================================================*/

bool VectorPolygon::GetNormals(CoordSys *PolyCoords, double PlanetRad, double *PolyNormal, double &PolyWt, 
	VectorNode *RefNode)
{
Point3d P1, P2;
VertexDEM Origin, Vert;

Origin.Lat = RefNode->Lat;
Origin.Lon = RefNode->Lon;
Origin.Elev = RefNode->Elev;
#ifdef WCS_BUILD_VNS
PolyCoords->DegToCart(&Origin);
#else // WCS_BUILD_VNS
Origin.DegToCart(PlanetRad);
#endif // WCS_BUILD_VNS
	
Vert.Lat = RefNode->NextNode->Lat;
Vert.Lon = RefNode->NextNode->Lon;
Vert.Elev = RefNode->NextNode->Elev;
#ifdef WCS_BUILD_VNS
PolyCoords->DegToCart(&Vert);
#else // WCS_BUILD_VNS
Vert.DegToCart(PlanetRad);
#endif // WCS_BUILD_VNS
FindPosVector(P1, Vert.XYZ, Origin.XYZ);

Vert.Lat = RefNode->NextNode->NextNode->Lat;
Vert.Lon = RefNode->NextNode->NextNode->Lon;
Vert.Elev = RefNode->NextNode->NextNode->Elev;
#ifdef WCS_BUILD_VNS
PolyCoords->DegToCart(&Vert);
#else // WCS_BUILD_VNS
Vert.DegToCart(PlanetRad);
#endif // WCS_BUILD_VNS
FindPosVector(P2, Vert.XYZ, Origin.XYZ);

if (Normals[3] == 0.0f)
	{
	SurfaceNormal(PolyNormal, P1, P2);
	Normals[0] = (float)PolyNormal[0];
	Normals[1] = (float)PolyNormal[1];
	Normals[2] = (float)PolyNormal[2];
	} // if
else
	{
	PolyNormal[0] = Normals[0];
	PolyNormal[1] = Normals[1];
	PolyNormal[2] = Normals[2];
	} // else

// These both seem to work about the same and the first is less expensive
PolyWt = 1.0 - VectorAngle(P1, P2);
//PolyWt = acos(VectorAngle(P1, P2));
Normals[3] = (float)PolyWt;
	
return (PolyNormal[0] != 0.0 || PolyNormal[1] != 0.0 || PolyNormal[2] != 0.0);	// points do not lie on a straight line

} // VectorPolygon::GetNormals

/*===========================================================================*/

bool VectorPolygon::GetWaterNormals(CoordSys *PolyCoords, double PlanetRad, double *PolyNormal, double &PolyWt,
	VectorNode *RefNode, double Elev1, double Elev2, double Elev3)
{
Point3d P1, P2;
VertexDEM Origin, Vert;

Origin.Lat = RefNode->Lat;
Origin.Lon = RefNode->Lon;
Origin.Elev = Elev1;
#ifdef WCS_BUILD_VNS
PolyCoords->DegToCart(&Origin);
#else // WCS_BUILD_VNS
Origin.DegToCart(PlanetRad);
#endif // WCS_BUILD_VNS
	
Vert.Lat = RefNode->NextNode->Lat;
Vert.Lon = RefNode->NextNode->Lon;
Vert.Elev = Elev2;
#ifdef WCS_BUILD_VNS
PolyCoords->DegToCart(&Vert);
#else // WCS_BUILD_VNS
Vert.DegToCart(PlanetRad);
#endif // WCS_BUILD_VNS
FindPosVector(P1, Vert.XYZ, Origin.XYZ);

Vert.Lat = RefNode->NextNode->NextNode->Lat;
Vert.Lon = RefNode->NextNode->NextNode->Lon;
Vert.Elev = Elev3;
#ifdef WCS_BUILD_VNS
PolyCoords->DegToCart(&Vert);
#else // WCS_BUILD_VNS
Vert.DegToCart(PlanetRad);
#endif // WCS_BUILD_VNS
FindPosVector(P2, Vert.XYZ, Origin.XYZ);

SurfaceNormal(PolyNormal, P1, P2);

// These both seem to work about the same and the first is less expensive
PolyWt = 1.0 - VectorAngle(P1, P2);
//PolyWt = acos(VectorAngle(P1, P2));
	
return (PolyNormal[0] != 0.0 || PolyNormal[1] != 0.0 || PolyNormal[2] != 0.0);	// points do not lie on a straight line

} // VectorPolygon::GetWaterNormals

/*===========================================================================*/
/*===========================================================================*/

VectorPolygon *VectorPolygonList::MakeVectorPolygon(VectorNode *Pt1, VectorNode *Pt2, VectorNode *Pt3, VectorPolygon *Template)
{

if (MyPolygon = new VectorPolygon(Pt1, Pt2, Pt3, Template))
	{
	if (MyPolygon->TotalNumNodes < 3)
		{
		delete MyPolygon;
		MyPolygon = NULL;
		} // if
	} // if

return (MyPolygon);

} // VectorPolygonList::MakeVectorPolygon

/*===========================================================================*/

VectorPolygon *VectorPolygonList::MakeVectorPolygon(VectorNode *Pt1, VectorNode *Pt2, VectorNode *Pt3, 
	VectorPolygon *Template, bool AddLinkBack)
{

if (MyPolygon = new VectorPolygon(Pt1, Pt2, Pt3, Template, AddLinkBack))
	{
	if (MyPolygon->TotalNumNodes < 3)
		{
		delete MyPolygon;
		MyPolygon = NULL;
		} // if
	} // if

return (MyPolygon);

} // VectorPolygonList::MakeVectorPolygon

/*===========================================================================*/

VectorPolygon *VectorPolygonList::MakeVectorPolygon(Database *DBHost, GeneralEffect *NewEffect, Joe *NewVector)
{
bool InsufficientNodes = false;

if (MyPolygon = new VectorPolygon(DBHost, NewEffect, NewVector, false, InsufficientNodes))
	{
	if (MyPolygon->TotalNumNodes < 3)
		{
		delete MyPolygon;
		MyPolygon = NULL;
		FlagSet(WCS_VECTORPOLYGONLIST_FLAG_INSUFFICIENTNODES);
		} // if
	} // if

return (MyPolygon);

} // VectorPolygonList::MakeVectorPolygon

/*===========================================================================*/

VectorPolygon *VectorPolygonList::MakeVectorPolygon(VectorNode **Pts, unsigned long NumPts, VectorPolygon *Template)
{

if (MyPolygon = new VectorPolygon(Pts, NumPts, Template))
	{
	if (MyPolygon->TotalNumNodes < 3)
		{
		delete MyPolygon;
		MyPolygon = NULL;
		} // if
	} // if

return (MyPolygon);

} // VectorPolygonList::MakeVectorPolygon

/*===========================================================================*/

VectorPolygon *VectorPolygonList::MakeVectorPolygon(VectorPolygon * const CloneMe, VectorNode *NewNodes, bool AddLinkBack)
{

if (MyPolygon = new VectorPolygon(CloneMe, NewNodes, AddLinkBack))
	{
	if (MyPolygon->TotalNumNodes < 3)
		{
		delete MyPolygon;
		MyPolygon = NULL;
		} // if
	} // if

return (MyPolygon);


} // VectorPolygonList::MakeVectorPolygon

/*===========================================================================*/

VectorPolygon *VectorPolygonList::MakeVectorPolygon(VectorPart * const ClonePart, bool AddLinkBack, VectorPolygon *PartOwner)
{

if (MyPolygon = new VectorPolygon(ClonePart, AddLinkBack, PartOwner))
	{
	if (MyPolygon->TotalNumNodes < 3)
		{
		delete MyPolygon;
		MyPolygon = NULL;
		} // if
	} // if

return (MyPolygon);


} // VectorPolygonList::MakeVectorPolygon

/*===========================================================================*/

void VectorPolygonList::DeletePolygon(void)
{

if (MyPolygon)
	delete MyPolygon;
MyPolygon = NULL;

} // VectorPolygonList::DeletePolygon

/*===========================================================================*/

bool VectorPolygonList::ConvertToDefGeo(void)
{
bool Success = false;

if (MyPolygon)
	{
	Success = MyPolygon->ConvertToDefGeo();
	} // if
	
return (Success);

} // VectorPolygonList::ConvertToDefGeo

/*===========================================================================*/

bool VectorPolygonList::MergeAdjoiningNodes(bool &NodeReplicationExists)
{
bool Success = false;

if (MyPolygon)
	{
	// this might also reduce the nodes to less than the legal minimum of 3
	if (Success = MyPolygon->MergeAdjoiningNodes(NodeReplicationExists))
		{
		if (! MyPolygon->PolyFirstNode() || MyPolygon->TotalNumNodes < 3)
			{
			DeletePolygon();
			} // if
		} // if
	} // if

return (Success);

} // VectorPolygonList::MergeAdjoiningNodes

/*===========================================================================*/

void VectorPolygonList::RemoveVestigialSegments(void)
{

if (MyPolygon)
	{
	// this might also reduce the nodes to less than the legal minimum of 3
	MyPolygon->RemoveVestigialSegments();
	if (! MyPolygon->PolyFirstNode() || MyPolygon->TotalNumNodes < 3)
		{
		DeletePolygon();
		} // if
	} // if

} // VectorPolygonList::RemoveVestigialSegments

/**************************************************************************/
/**************************************************************************/

bool VectorPolygonListDouble::NormalizeDirection(void)
{
bool Success = true;

// in area is negative for a counter-clockwise polygon
// change the area sign since we know we are in geographic notation
// reverse the counter cw ones
if (DoubleVal < 0.0)
	{
	Success = MyPolygon->ReverseDirection();
	DoubleVal = -DoubleVal;
	} // if

return (Success);

} // VectorPolygonListDouble::NormalizeDirection

/*===========================================================================*/

VectorPolygonListDouble *VectorPolygonListDouble::ResolveSelfIntersections(bool &Success, bool &LinksAdded, 
	EffectEval *Evaluator)
{
VectorPolygonListDouble *NewList = NULL;

if (MyPolygon)
	{
	MyPolygon->RemoveVestigialSegments();
	if (MyPolygon->TotalNumNodes >= 3)
		{
		// this might also reduce the nodes to less than the legal minimum of 3
		if ((Success = MyPolygon->ResolveSelfIntersections(LinksAdded)) && LinksAdded)
			{
			if (NewList = new VectorPolygonListDouble())
				{
				NewList->MyPolygon = MyPolygon;
				MyPolygon = NULL;
				Success = Evaluator->MergePolygons(NewList, 0.0, NULL, NULL, true, false);
				} // if
			} // if
		} // if
	else
		{
		delete MyPolygon;
		MyPolygon = NULL;
		} // else
	} // if

return (NewList);

} // VectorPolygonListDouble::ResolveSelfIntersections

/*===========================================================================*/

VectorPolygonListDouble *VectorPolygonListDouble::Triangulate(bool &Success)
{
VectorPolygonListDouble *NewList = NULL;

if (MyPolygon)
	{
	// this might also reduce the nodes to less than the legal minimum of 3
	NewList = MyPolygon->Triangulate(Success);
	if (! MyPolygon->PolyFirstNode() || MyPolygon->TotalNumNodes < 3)
		{
		DeletePolygon();
		} // if
	} // if

return (NewList);

} // VectorPolygonListDouble::Triangulate

/*===========================================================================*/
/*===========================================================================*/

TriangleBoundingBox::TriangleBoundingBox()
{

Pt1 = Pt2 = Pt3 = NULL;

} // TriangleBoundingBox::TriangleBoundingBox

/*===========================================================================*/

void TriangleBoundingBox::SetTriangle(VectorNode *Pt1Source, VectorNode *Pt2Source, VectorNode *Pt3Source)
{

MaxX = MaxY = -FLT_MAX;
MinX = MinY = FLT_MAX;

Pt1 = Pt1Source;
Pt2 = Pt2Source;
Pt3 = Pt3Source;

if (Pt1->Lat > MaxY)
	MaxY = Pt1->Lat;
if (Pt1->Lat < MinY)
	MinY = Pt1->Lat;
if (Pt1->Lon > MaxX)
	MaxX = Pt1->Lon;
if (Pt1->Lon < MinX)
	MinX = Pt1->Lon;

if (Pt2->Lat > MaxY)
	MaxY = Pt2->Lat;
if (Pt2->Lat < MinY)
	MinY = Pt2->Lat;
if (Pt2->Lon > MaxX)
	MaxX = Pt2->Lon;
if (Pt2->Lon < MinX)
	MinX = Pt2->Lon;

if (Pt3->Lat > MaxY)
	MaxY = Pt3->Lat;
if (Pt3->Lat < MinY)
	MinY = Pt3->Lat;
if (Pt3->Lon > MaxX)
	MaxX = Pt3->Lon;
if (Pt3->Lon < MinX)
	MinX = Pt3->Lon;

} // TriangleBoundingBox::SetTriangle

/*===========================================================================*/

int TriangleBoundingBox::TestPointInBox(VectorNode *TestPt)
{

// point must be actually inside the box to return TRUE, not just on the edge of the box
return (TestPt->Lat >= MinY && TestPt->Lat <= MaxY && 
	TestPt->Lon >= MinX && TestPt->Lon <= MaxX);

} // TriangleBoundingBox::TestPointInBox

/*===========================================================================*/

int TriangleBoundingBox::TestPointInTriangle(VectorNode *TestPt)
{
int SSResult = 0;

// first tests if the point is in the bounding box
// then tests the point three times, once for each pair of triangle sides
// to see if the point lies between each pair
// Given the conditions for the function SameSide, if the test point lies on one of the triangle edges
// or at one of the triangle vertices, the test will return FALSE
if (TestPointInBox(TestPt))
	{
	SSResult = SameSide(Pt1, Pt2, TestPt, Pt3) && SameSide(Pt2, Pt3, TestPt, Pt1) && SameSide(Pt3, Pt1, TestPt, Pt2);
	if (SSResult)
		{
		if ((TestPt->Lat == Pt1->Lat && TestPt->Lon == Pt1->Lon
			&& TestPt->Elev == Pt1->Elev)
			|| (TestPt->Lat == Pt2->Lat && TestPt->Lon == Pt2->Lon
			&& TestPt->Elev == Pt2->Elev)
			|| (TestPt->Lat == Pt3->Lat && TestPt->Lon == Pt3->Lon
			&& TestPt->Elev == Pt3->Elev))
			SSResult = 0;
		} // if
	} // if

return(SSResult);

} // TriangleBoundingBox::TestPointInTriangle

/*===========================================================================*/

int TriangleBoundingBox::SameSide(VectorNode *pOrigin, VectorNode *pSide, VectorNode *pTest, VectorNode *pConfirm)
{
TriangleBoundingBoxVector CP1, CP2, Vec1, Vec2;

// determines if the cross product of two vectors has the same orientation in Z space as 
// one of the vectors and a third vector. all vectors originate at pOrigin
// if the test point lies on the side vector the function will return false.
// if the confirm point is on the same line as the side, 
// the function will return true if the test point is not on the same line
CP1.CrossProductXY(Vec1.SetXY(pSide, pOrigin), Vec2.SetXY(pTest, pOrigin));

// if test point is on the line, it is not inside so return false
if (CP1.Z == 0.0)// && pTest->FlagCheck(WCS_VECTORNODE_FLAG_HOLEREMOVALSHARE))
	return (false);
CP2.CrossProductXY(Vec1.SetXY(pSide, pOrigin), Vec2.SetXY(pConfirm, pOrigin));

return (CP1.DotProductZ(&CP2) >= 0.0);

} // TriangleBoundingBox::SameSide

/*===========================================================================*/

double TriangleBoundingBox::TriangleArea(void)
{
double Area, OriginY;

OriginY = Pt1->Lat;
Area = ((Pt2->Lat - Pt1->Lat) * (Pt2->Lon - Pt1->Lon) * .5);
Area += ((Pt3->Lat - Pt2->Lat) * (Pt3->Lon - Pt2->Lon) * .5 + (Pt2->Lat - OriginY) * (Pt3->Lon - Pt2->Lon));
Area += ((Pt1->Lat - Pt3->Lat) * (Pt1->Lon - Pt3->Lon) * .5 + (Pt3->Lat - OriginY) * (Pt1->Lon - Pt3->Lon));

return (Area);

} // TriangleBoundingBox::TriangleArea

/*===========================================================================*/

bool TriangleBoundingBox::PartIntersectsTriangle(VectorPolygon *Poly, VectorPart *TestPart)
{
VectorNode *CurPartNode;
unsigned long NodeCt;
VectorIntersecter VI;

// see if part and triangle bounding boxes overlap

for (CurPartNode = TestPart->FirstNode(), NodeCt = 0; NodeCt < TestPart->NumNodes; CurPartNode = CurPartNode->NextNode, ++NodeCt)
	{
	// test intersection of part segment with all three sides of triangle
	if (VI.TestSegmentsTouchAbsolute(Poly, CurPartNode, CurPartNode->NextNode, Pt1, Pt2)
		|| VI.TestSegmentsTouchAbsolute(Poly, CurPartNode, CurPartNode->NextNode, Pt2, Pt3)
		|| VI.TestSegmentsTouchAbsolute(Poly, CurPartNode, CurPartNode->NextNode, Pt3, Pt1))
		{
		return (true);
		} // if
	} // for

return (false);

} // TriangleBoundingBox::PartIntersectsTriangle

bool TriangleBoundingBox::PartIntersectsTriangleExcludeNode(VectorPolygon *Poly, VectorPart *TestPart, VectorNode *ExcludeMe)
{
VectorNode *CurPartNode;
unsigned long NodeCt;
VectorIntersecter VI;

// see if part and triangle bounding boxes overlap

for (CurPartNode = TestPart->FirstNode(), NodeCt = 0; NodeCt < TestPart->NumNodes; CurPartNode = CurPartNode->NextNode, ++NodeCt)
	{
	if (CurPartNode == ExcludeMe || CurPartNode->NextNode == ExcludeMe)
		continue;
	// test intersection of part segment with all three sides of triangle
	if (VI.TestSegmentsTouchAbsolute(Poly, CurPartNode, CurPartNode->NextNode, Pt1, Pt2)
		|| VI.TestSegmentsTouchAbsolute(Poly, CurPartNode, CurPartNode->NextNode, Pt2, Pt3)
		|| VI.TestSegmentsTouchAbsolute(Poly, CurPartNode, CurPartNode->NextNode, Pt3, Pt1))
		{
		return (true);
		} // if
	} // for

return (false);

} // TriangleBoundingBox::PartIntersectsTriangleExcludeNode

/*===========================================================================*/
/*===========================================================================*/

TriangleBoundingBoxVector *TriangleBoundingBoxVector::SetXY(VectorNode *pSide, VectorNode *pOrigin)
{

// creates a 2D vector with the Z value ignored
X = pSide->Lon - pOrigin->Lon;
Y = pSide->Lat - pOrigin->Lat;
Z = 0.0;

return (this);

} // TriangleBoundingBoxVector::SetXY

/*===========================================================================*/

TriangleBoundingBoxVector *TriangleBoundingBoxVector::SetXY(VectorPoint *pSide, VectorPoint *pOrigin)
{

// creates a 2D vector with the Z value ignored
X = pSide->Longitude - pOrigin->Longitude;
Y = pSide->Latitude - pOrigin->Latitude;
Z = 0.0;

return (this);

} // TriangleBoundingBoxVector::SetXY

/*===========================================================================*/

TriangleBoundingBoxVector *TriangleBoundingBoxVector::SetXY(double SideLon, double SideLat, VectorNode *pOrigin)
{

// creates a 2D vector with the Z value ignored
X = SideLon - pOrigin->Lon;
Y = SideLat - pOrigin->Lat;
Z = 0.0;

return (this);

} // TriangleBoundingBoxVector::SetXY

/*===========================================================================*/

double TriangleBoundingBoxVector::CrossProductXY(TriangleBoundingBoxVector *V1, TriangleBoundingBoxVector *V2)
{

// since the Z value is 0 for both vectors the cross product is on the Z axis and
// only one multiplication is necessary
X = Y = 0.0;

return (Z = V1->X * V2->Y - V1->Y * V2->X);

} // TriangleBoundingBoxVector::CrossProductXY

/*===========================================================================*/

double TriangleBoundingBoxVector::DotProductXY(TriangleBoundingBoxVector *V1, TriangleBoundingBoxVector *V2)
{

return(V1->X * V2->X + V1->Y * V2->Y);

} // TriangleBoundingBoxVector::DotProductXY

/*===========================================================================*/

double TriangleBoundingBoxVector::DotProductZ(TriangleBoundingBoxVector *V2)
{

// ignores any X or Y values and only considers the Z value. A positive result indicates both vectors
// lie on the same side relative to the XY plane
return (Z = Z * V2->Z);

} // TriangleBoundingBoxVector::DotProductZ

/*===========================================================================*/

double TriangleBoundingBoxVector::LengthXYSquared(void)
{

return (X * X + Y * Y);

} // TriangleBoundingBoxVector::LengthXYSquared

/*===========================================================================*/

double TriangleBoundingBoxVector::FindRelativeAngle(TriangleBoundingBoxVector *V1, TriangleBoundingBoxVector *V2)
{
double Angle;

if (V1->TestVecNonZero() && V2->TestVecNonZero())
	{
	Angle = CrossProductXY(V1, V2);
	// this approach needs validation
	Angle = Angle > 0.0 ? Angle * Angle: - Angle * Angle;
	Angle /= (V1->LengthXYSquared() * V2->LengthXYSquared());
	// this is the proper way to do it but involves a square root
	//Angle /= sqrt(V1->LengthXYSquared() * V2->LengthXYSquared());
	if (DotProductXY(V1, V2) > 0.0)
		{
		Angle = 2.0 - Angle;
		} // if
	else if (Angle <= 0.0)
		Angle += 4.0;
	} // if
else
	Angle = 4.0;	// maximum angle this function can return if calculated normally

return (Angle);

} // TriangleBoundingBoxVector::FindRelativeAngle

/*===========================================================================*/
/*===========================================================================*/

PolygonBoundingBox::PolygonBoundingBox(VectorPolygon *MyPoly)
{

UpdateBounds(MyPoly);

} // PolygonBoundingBox::PolygonBoundingBox

/*===========================================================================*/

void PolygonBoundingBox::Copy(PolygonBoundingBox *CopyFrom)
{

MaxX = CopyFrom->MaxX;
MaxY = CopyFrom->MaxY;
MinX = CopyFrom->MinX;
MinY = CopyFrom->MinY;

} // PolygonBoundingBox::Copy

/*===========================================================================*/

#if _M_IX86_FP < 2

void PolygonBoundingBox::UpdateBounds(VectorPolygon *MyPoly)
{
VectorNode *CurNode;

MaxY = MaxX = -FLT_MAX;
MinY = MinX = FLT_MAX;

for (CurNode = MyPoly->PolyFirstNode(); CurNode; CurNode = MyPoly->PolyNextNode(CurNode))
	{
	if (CurNode->Lat > MaxY)
		MaxY = CurNode->Lat;
	if (CurNode->Lat < MinY)
		MinY = CurNode->Lat;
	if (CurNode->Lon > MaxX)
		MaxX = CurNode->Lon;
	if (CurNode->Lon < MinX)
		MinX = CurNode->Lon;
	} // for

} // PolygonBoundingBox::UpdateBounds
#else // _M_IX86_FP < 2
// SSE2 intrinsics version
void PolygonBoundingBox::UpdateBounds(VectorPolygon *MyPoly)
{
__m128d lon_lat, maxs, mins;
double fmax = -FLT_MAX, fmin = FLT_MAX;
VectorNode *CurNode;

maxs = _mm_load1_pd(&fmax);	// copy double precision value to both halves of xmm register
mins = _mm_load1_pd(&fmin);

for (CurNode = MyPoly->PolyFirstNode(); CurNode; CurNode = MyPoly->PolyNextNode(CurNode))
	{
	lon_lat = _mm_load_sd(&CurNode->Lat);			// load lat into lower half of xmm register, zero upper half
	lon_lat = _mm_loadh_pd(lon_lat, &CurNode->Lon);	// load lon into upper half of xmm register, keeping lower half
	maxs = _mm_max_pd(maxs, lon_lat);				// compute maxs & keep in xmm register
	mins = _mm_min_pd(mins, lon_lat);				// compute mins & keep in xmm register
	} // for

_mm_storeh_pd(&MaxX, maxs);	// store upper half of maxs
_mm_storel_pd(&MaxY, maxs);	// store lower half of maxs
_mm_storeh_pd(&MinX, mins);	// store upper half of mins
_mm_storel_pd(&MinY, mins);	// store lower half of mins

} // PolygonBoundingBox::UpdateBounds

#endif // _M_IX86_FP < 2

/*===========================================================================*/

#if _M_IX86_FP < 2

void PolygonBoundingBox::UpdateBounds(VectorNode **NodeList, unsigned long NumNodes)
{
unsigned long NodeCt;
VectorNode *CurNode;

MaxY = MaxX = -FLT_MAX;
MinY = MinX = FLT_MAX;

for (NodeCt = 0; NodeCt < NumNodes; ++NodeCt)
	{
	CurNode = NodeList[NodeCt];
	if (CurNode->Lat > MaxY)
		MaxY = CurNode->Lat;
	if (CurNode->Lat < MinY)
		MinY = CurNode->Lat;
	if (CurNode->Lon > MaxX)
		MaxX = CurNode->Lon;
	if (CurNode->Lon < MinX)
		MinX = CurNode->Lon;
	} // for

} // PolygonBoundingBox::UpdateBounds
#else // _M_IX86_FP < 2
// SSE2 intrinsics version
void PolygonBoundingBox::UpdateBounds(VectorNode **NodeList, unsigned long NumNodes)
{
__m128d lon_lat, maxs, mins;
double fmax = -FLT_MAX, fmin = FLT_MAX;
unsigned long NodeCt;
VectorNode *CurNode;

maxs = _mm_load1_pd(&fmax);	// copy double precision value to both halves of xmm register
mins = _mm_load1_pd(&fmin);

for (NodeCt = 0; NodeCt < NumNodes; ++NodeCt)
	{
	CurNode = NodeList[NodeCt];
	lon_lat = _mm_load_sd(&CurNode->Lat);			// load lat into lower half of xmm register, zero upper half
	lon_lat = _mm_loadh_pd(lon_lat, &CurNode->Lon);	// load lon into upper half of xmm register, keeping lower half
	maxs = _mm_max_pd(maxs, lon_lat);				// compute maxs & keep in xmm register
	mins = _mm_min_pd(mins, lon_lat);				// compute mins & keep in xmm register
	} // for

_mm_storeh_pd(&MaxX, maxs);	// store upper half of maxs
_mm_storel_pd(&MaxY, maxs);	// store lower half of maxs
_mm_storeh_pd(&MinX, mins);	// store upper half of mins
_mm_storel_pd(&MinY, mins);	// store lower half of mins

} // PolygonBoundingBox::UpdateBounds

#endif // _M_IX86_FP < 2

/*===========================================================================*/

bool PolygonBoundingBox::TestPointInBox(double X, double Y)
{

if ((Y > MaxY) || (Y < MinY) || (X > MaxX) || (X < MinX))
	return (false);

return (true);

} // PolygonBoundingBox::TestPointInBox

/*===========================================================================*/

bool PolygonBoundingBox::TestPointInBox(VectorNode *TestNode)
{

if ((TestNode->Lat > MaxY) || (TestNode->Lat < MinY) || (TestNode->Lon > MaxX) || (TestNode->Lon < MinX))
	return (false);

return (true);

} // PolygonBoundingBox::TestPointContained

/*===========================================================================*/

ENUM_BOUNDING_BOX_CONTAINED PolygonBoundingBox::TestBoxContained(PolygonBoundingBox *IsItInsideMe)
{
double xX, xY, nX, nY;

xX = IsItInsideMe->MaxX;
xY = IsItInsideMe->MaxY;
nX = IsItInsideMe->MinX;
nY = IsItInsideMe->MinY;

// if _this_ is totally inside IsItInsideMe return WCS_BOUNDING_BOX_CONTAINED_INSIDE
// if _this_ is partially inside IsItInsideMe return WCS_BOUNDING_BOX_CONTAINED_EDGE
// if _this_ is totally outside IsItInsideMe return WCS_BOUNDING_BOX_CONTAINED_OUTSIDE
// if IsItInsideMe is totally inside _this_ return WCS_BOUNDING_BOX_CONTAINED_ENCLOSED

// do they overlap at all? No.
if (MinX > xX || MinY > xY || MaxX < nX || MaxY < nY)
	return (WCS_BOUNDING_BOX_CONTAINED_OUTSIDE);

// Yes, at least some overlap.
// is _this_ contained completely by IsItInsideMe
if (MaxX < xX && MinX > nX && MaxY < xY && MinY > nY)
	return (WCS_BOUNDING_BOX_CONTAINED_INSIDE);

// is IsItInsideMe contained completely by _this_
if (xX < MaxX && nX > MinX && xY < MaxY && nY > MinY)
	return (WCS_BOUNDING_BOX_CONTAINED_ENCLOSED);

// bounding boxes overlap
return (WCS_BOUNDING_BOX_CONTAINED_EDGE);

} // PolygonBoundingBox::TestThisBoxContained
