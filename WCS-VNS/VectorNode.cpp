// VectorNode.cpp
// Source file for VectorNode class
// Created by Gary R. Huber 1/24/06
// Copyright 2006 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "VectorNode.h"
#include "AppMem.h"
#include "Points.h"
#include "VectorPolygon.h"
#include "Render.h"
#include "EffectsLib.h"

#include "Useful.h"	// for DEBUGOUT

using namespace std;

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//extern unsigned __int64 DevCounter[50];
//#endif // FRANK or GARY

QuantaAllocatorPool *MaterialList::MyPool;
QuantaAllocatorPool *MatListWaterProperties::MyPool;
QuantaAllocatorPool *MatListVectorProperties::MyPool;
QuantaAllocatorPool *VectorNodeRenderData::MyPool;
QuantaAllocatorPool *VectorNodeList::MyPool;
QuantaAllocatorPool *VectorNodeLink::MyPoolD1;
QuantaAllocatorPool *VectorNode::MyPool;

#ifdef _DEBUG
unsigned long NumberOfNodes;
unsigned long NumberOfNodeLinks;
#endif // _DEBUG


VectorNode::VectorNode()
{

LinkedNodes = NULL;
NextNode = PrevNode = NULL;
NodeData = NULL;
Flags = 0;
Elev = Lon = Lat = 0.0;
RightSideList = NULL;
ForwardList = NULL;
#ifdef _DEBUG
++NumberOfNodes;
#endif // _DEBUG

} // VectorNode::VectorNode

/*===========================================================================*/

VectorNode::VectorNode(VectorPoint *SetPoint)
{

LinkedNodes = NULL;
NextNode = PrevNode = NULL;
NodeData = NULL;
Flags = 0;
Lat = SetPoint->Latitude;
Lon = SetPoint->Longitude;
Elev = SetPoint->Elevation;
RightSideList = NULL;
ForwardList = NULL;
#ifdef _DEBUG
++NumberOfNodes;
#endif // _DEBUG

} // VectorNode::VectorNode

/*===========================================================================*/

VectorNode::VectorNode(VectorNode *Template)
{

LinkedNodes = NULL;
NextNode = PrevNode = NULL;
NodeData = NULL;
Flags = 0;
FlagSet(Template->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE | WCS_VECTORNODE_FLAG_ORIGINALVECTORNODE | 
	WCS_VECTORNODE_FLAG_VECTORNODE));
Lat = Template->Lat;
Lon = Template->Lon;
Elev = Template->Elev;
RightSideList = NULL;
ForwardList = NULL;
#ifdef _DEBUG
++NumberOfNodes;
#endif // _DEBUG

} // VectorNode::VectorNode

/*===========================================================================*/

VectorNode::~VectorNode()
{

RemoveLinkedNodes();
RemoveRightSideList();
RemoveEdgeList(NULL);

if (NodeData)
	delete NodeData;
NodeData = NULL;

} // VectorNode::~VectorNode

/*===========================================================================*/

bool VectorNode::AddRightSideMember(VectorPolygon *NewSideMember, bool EdgeIsShared)
{
VectorPolygonList **NewListPtr;

for (NewListPtr = &RightSideList; *NewListPtr; NewListPtr = &(*NewListPtr)->NextPolygonList)
	{
	if ((*NewListPtr)->MyPolygon == NewSideMember)
		return (true);
	} // for

if (*NewListPtr = new VectorPolygonList())
	{
	(*NewListPtr)->MyPolygon = NewSideMember;
	if (EdgeIsShared)
		(*NewListPtr)->FlagSet(WCS_VECTORPOLYGONLIST_FLAG_RIGHTSHARE);
	return (true);
	} // if
	
return (false);
	
} // VectorNode::AddRightSideMember

/*===========================================================================*/

void VectorNode::RemoveRightSideList(void)
{

for (VectorPolygonList *CurList = RightSideList; CurList; CurList = RightSideList)
	{
	RightSideList = RightSideList->NextPolygonList;
	delete CurList; 
	} // for
	
} // VectorNode::RemoveRightSideList

/*===========================================================================*/

void VectorNode::AddForwardEdge(PolygonEdgeList *NewEdge)
{
PolygonEdgeList **NewListPtr;

for (NewListPtr = &ForwardList; *NewListPtr; NewListPtr = &(*NewListPtr)->NextEdgeThisNode)
	{
	if (*NewListPtr == NewEdge)
		return;
	} // for

*NewListPtr = NewEdge;
	
} // VectorNode::AddForwardEdge

/*===========================================================================*/

void VectorNode::RemoveEdgeList(PolygonEdgeList *RemoveEdge)
{
PolygonEdgeList *CurList, *PrevList;

if (RemoveEdge)
	{
	PrevList = NULL;
	for (CurList = ForwardList; CurList; CurList = CurList->NextEdgeThisNode)
		{
		if (CurList == RemoveEdge)
			{
			if (PrevList)
				PrevList->NextEdgeThisNode = CurList->NextEdgeThisNode;
			else
				ForwardList = CurList->NextEdgeThisNode;
			CurList->TrailingNode = NULL;
			CurList->NextEdgeThisNode = NULL;
			break;
			} // if
		PrevList = CurList;
		} // for
	} // if
else
	{
	for (CurList = ForwardList; CurList; CurList = ForwardList)
		{
		ForwardList = ForwardList->NextEdgeThisNode;
		CurList->TrailingNode = NULL;
		CurList->NextEdgeThisNode = NULL; 
		} // for
	} // else
	
} // VectorNode::RemoveEdgeList

/*===========================================================================*/

void VectorNode::RemoveLinkedNodes(void)
{
VectorNode *TheOtherNode;
VectorNodeLink *CurGuestNodeLink, *LastGuestNodeLink;

for (VectorNodeLink *CurNodeLink = LinkedNodes; CurNodeLink; CurNodeLink = LinkedNodes)
	{
	// F2_NOTE: vvv CPI = 10+
	LinkedNodes = (VectorNodeLink *)LinkedNodes->NextNodeList;
	// delete the link from the other direction
	if (TheOtherNode = CurNodeLink->MyNode)
		{
		if (NodeData && TheOtherNode->NodeData && 
			NodeData->Materials == TheOtherNode->NodeData->Materials && 
			NodeData->MaterialListOwned)
			{
			TheOtherNode->NodeData->Materials = NULL;
			TheOtherNode->NodeData->MaterialListOwned = true;
			TheOtherNode->FlagClear(WCS_VECTORNODE_FLAG_NONELEVATIONITEMSALL);
			} // if
		// F2_NOTE: vvv CPI = 10+
		for (CurGuestNodeLink = TheOtherNode->LinkedNodes, LastGuestNodeLink = NULL; CurGuestNodeLink; CurGuestNodeLink = (VectorNodeLink *)CurGuestNodeLink->NextNodeList)
			{
			if (CurGuestNodeLink->MyNode == this)
				{
				if (LastGuestNodeLink)
					{
					LastGuestNodeLink->NextNodeList = CurGuestNodeLink->NextNodeList;
					} // if
				else
					{
					TheOtherNode->LinkedNodes = (VectorNodeLink *)CurGuestNodeLink->NextNodeList;
					} // else
				delete CurGuestNodeLink;
				break;
				} // if
			// F2_NOTE: vvv CPI = 10+
			LastGuestNodeLink = CurGuestNodeLink;
			} // for
		} // if
	delete CurNodeLink;
	} // for

} // VectorNode::RemoveLinkedNodes

/*===========================================================================*/

void VectorNode::RemoveLink(VectorNode *TheOtherNode)
{
VectorNodeLink *PrevLink = NULL, *CurLink;

for (CurLink = LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList)
	{
	if (TheOtherNode == CurLink->MyNode)
		{
		if (PrevLink)
			{
			PrevLink->NextNodeList = CurLink->NextNodeList;
			} // if
		else
			{
			LinkedNodes = (VectorNodeLink *)CurLink->NextNodeList;
			} // else
		delete CurLink;
		break;
		} // if
	PrevLink = CurLink;
	} // for

} // VectorNode::RemoveLink

/*===========================================================================*/

void VectorNode::DeleteLinksNoCrossCheck(void)
{

for (VectorNodeLink *CurNode = LinkedNodes; CurNode; CurNode = LinkedNodes)
	{
	LinkedNodes = (VectorNodeLink *)LinkedNodes->NextNodeList;
	delete CurNode;
	} // for

} // VectorNode::DeleteLinksNoCrossCheck

/*===========================================================================*/

bool VectorNode::SamePoint(VectorNode *CompareMe)
{
bool rVal = false;

// uses a hundredth of a meter elevation tolerance
if (fabs(Elev - CompareMe->Elev) < .01)
	{
	// this requires absolute equality
	if ((Lat == CompareMe->Lat) && (Lon == CompareMe->Lon))
		rVal = true;
	} // if

return (rVal);

} // VectorNode::SamePoint

/*===========================================================================*/

bool VectorNode::SamePointLatLon(VectorNode *CompareMe)
{
bool rVal = false;

// this requires absolute equality
if ((Lat == CompareMe->Lat) && (Lon == CompareMe->Lon))
	rVal = true;

return (rVal);

} // VectorNode::SamePointLatLon

/*===========================================================================*/

bool VectorNode::SamePointLatLonOrLinked(VectorNode *CompareMe)
{
VectorNodeLink *CurLink;
bool rVal = false;

// this requires absolute equality
if ((Lat == CompareMe->Lat) && (Lon == CompareMe->Lon))
	rVal = true;

for (CurLink = LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList)
	{
	if (CurLink->MyNode == CompareMe)
		{
		rVal = true;
		break;
		} // if
	} // for
	
return (rVal);

} // VectorNode::SamePointLatLonOrLinked

/*===========================================================================*/

bool VectorNode::SimilarPointLatLon(VectorNode *CompareMe, double Tolerance)
{
bool rVal = false;

if ((fabs(Lat - CompareMe->Lat) <= Tolerance) && (fabs(Lon - CompareMe->Lon) <= Tolerance))
	rVal = true;

return (rVal);

} // VectorNode::SimilarPointLatLon

/*===========================================================================*/

bool VectorNode::SimilarPointLatLon(double CompareLat, double CompareLon, double Tolerance)
{
bool rVal = false;

if ((fabs(Lat - CompareLat) <= Tolerance) && (fabs(Lon - CompareLon) <= Tolerance))
	rVal = true;

return (rVal);

} // VectorNode::SimilarPointLatLon

/*===========================================================================*/

// adapted from VectorPoint class
bool VectorNode::ProjToDefDeg(CoordSys *SourceCoords, VertexDEM *Vert)
{

#ifdef WCS_BUILD_VNS
// translate projected coords to global cartesian and then to default degrees
if (SourceCoords)
	{
	Vert->xyz[0] = Lon;
	Vert->xyz[1] = Lat;
	Vert->xyz[2] = Elev;
	if (SourceCoords->ProjToDefDeg(Vert))
		return (1);
	} // if
else
	{
#endif // WCS_BUILD_VNS
	// coordinates are already in default degrees
	Vert->Lon = Lon;
	Vert->Lat = Lat;
	Vert->Elev = Elev;
	return (1);
#ifdef WCS_BUILD_VNS
	} // else

return (0);
#endif // WCS_BUILD_VNS

} // VectorNode::ProjToDefDeg

/*===========================================================================*/

bool VectorNode::AddCrossLinks(VectorNode *LinkMe, VectorPolygon *SourcePolyThis, VectorPolygon *SourcePolyThat)
{
unsigned long FlagTest;

if (FlagTest = FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE))
	LinkMe->FlagSet(FlagTest);
else if (FlagTest = LinkMe->FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE))
	FlagSet(FlagTest);
return (AddLink(LinkMe, SourcePolyThat) && LinkMe->AddLink(this, SourcePolyThis));
//return (AddLink(LinkMe, SourcePolyThat, (1 << 7)) && LinkMe->AddLink(this, SourcePolyThis, (1 << 7)));

} // VectorNode::AddCrossLinks

/*===========================================================================*/

bool VectorNode::AddLink(VectorNode *LinkMe, VectorPolygon *SourcePoly, unsigned char NewFlags)
{
VectorNodeLink **NodePtr = &LinkedNodes;
//unsigned long count = 0;
bool Success = true, Found = false;
//char text[32];

#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//DevCounter[47]++;
#endif // FRANK or GARY
while (*NodePtr)
	{
	if ((*NodePtr)->MyNode == LinkMe)
		{
#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//		DevCounter[48]++;
#endif // FRANK or GARY
		Found = true;
		break;
		} // if
	NodePtr = (VectorNodeLink **)&(*NodePtr)->NextNodeList;
//	count++;
	} // while

if (! Found)
	{
	if (! (*NodePtr = new VectorNodeLink(LinkMe, SourcePoly, NewFlags)))
		Success = false;
	} // if

#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//DevCounter[46] += count;
//OutputDebugString(text);
#endif // FRANK or GARY

return (Success);

} // VectorNode::AddLink

/*===========================================================================*/

VectorNode *VectorNode::FindLink(VectorPolygon *FindMe)
{

for (VectorNodeLink *CurLink = LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList)
	{
	if (CurLink->LinkedPolygon == FindMe)
		return (CurLink->MyNode);
	} // for

return (NULL);

} // VectorNode::FindLink

/*===========================================================================*/

void VectorNode::RemoveTempLinks(void)
{
VectorNodeLink *PrevLink = NULL;

for (VectorNodeLink *CurLink = LinkedNodes; CurLink; CurLink = CurLink ? (VectorNodeLink *)CurLink->NextNodeList: LinkedNodes)
	{
	if (CurLink->FlagCheck(WCS_VECTORNODELINK_FLAG_TEMPORARY))
		{
		if (PrevLink)
			{
			PrevLink->NextNodeList = CurLink->NextNodeList;
			delete CurLink;
			CurLink = PrevLink;
			} // if
		else
			{
			LinkedNodes = (VectorNodeLink *)CurLink->NextNodeList;
			delete CurLink;
			CurLink = NULL;
			} // else
		} // if
	PrevLink = CurLink;
	} // for

} // VectorNode::RemoveTempLinks

/*===========================================================================*/

VectorNodeRenderData *VectorNode::AddNodeData(void)
{

return (NodeData = new VectorNodeRenderData());

} // VectorNode::AddNodeData

/*===========================================================================*/

VectorNodeRenderData *VectorNode::CopyElevationData(VectorNode *CopyFrom)
{

if (CopyFrom->NodeData)
	{
	if (! NodeData)
		AddNodeData();
	if (NodeData)
		NodeData->CopyElevationData(CopyFrom->NodeData);
	FlagSet(WCS_VECTORNODE_FLAG_ELEVEVALUATED);
	} // if
Elev = CopyFrom->Elev;

return (NodeData);

} // VectorNode::CopyElevationData

/*===========================================================================*/

VectorNodeRenderData *VectorNode::CopyNormalData(VectorNode *CopyFrom)
{

if (CopyFrom->NodeData)
	{
	if (! NodeData)
		AddNodeData();
	if (NodeData)
		NodeData->CopyNormalData(CopyFrom->NodeData);
	FlagSet(WCS_VECTORNODE_FLAG_NORMALSEVALUATED);
	} // if

return (NodeData);

} // VectorNode::CopyNormalData

/*===========================================================================*/

VectorNodeRenderData *VectorNode::CopyNonElevationNormalData(VectorNode *CopyFrom)
{

if (CopyFrom->NodeData)
	{
	if (NodeData || AddNodeData())
		{
		NodeData->CopyNonElevationNormalData(CopyFrom->NodeData);
		FlagSet(CopyFrom->FlagCheck(WCS_VECTORNODE_FLAG_NONELEVATIONITEMSALL));
		if (NodeData->Materials && NodeData->Materials->WaterProp && NodeData->Materials->WaterProp->WaterNormal[0] == 0.0f)
			{
			FlagClear(WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED);
			} // if
		} // if
	} // if

return (NodeData);

} // VectorNode::CopyNonElevationNormalData

/*===========================================================================*/

void VectorNode::InheritBoundaryFlagsFromParents(VectorNode *Parent1, VectorNode *Parent2)
{
unsigned long Flags1, Flags2;

if ((Flags1 = Parent1->FlagCheck(WCS_VECTORNODE_FLAG_EASTEDGE)) && (Flags2 = Parent2->FlagCheck(WCS_VECTORNODE_FLAG_EASTEDGE)))
	FlagSet(WCS_VECTORNODE_FLAG_EASTEDGE);
if ((Flags1 = Parent1->FlagCheck(WCS_VECTORNODE_FLAG_WESTEDGE)) && (Flags2 = Parent2->FlagCheck(WCS_VECTORNODE_FLAG_WESTEDGE)))
	FlagSet(WCS_VECTORNODE_FLAG_WESTEDGE);
if ((Flags1 = Parent1->FlagCheck(WCS_VECTORNODE_FLAG_NORTHEDGE)) && (Flags2 = Parent2->FlagCheck(WCS_VECTORNODE_FLAG_NORTHEDGE)))
	FlagSet(WCS_VECTORNODE_FLAG_NORTHEDGE);
if ((Flags1 = Parent1->FlagCheck(WCS_VECTORNODE_FLAG_SOUTHEDGE)) && (Flags2 = Parent2->FlagCheck(WCS_VECTORNODE_FLAG_SOUTHEDGE)))
	FlagSet(WCS_VECTORNODE_FLAG_SOUTHEDGE);
if ((Flags1 = Parent1->FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE)) && (Flags2 = Parent2->FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE)))
	FlagSet(WCS_VECTORNODE_FLAG_VECTORNODE);

} // VectorNode::InheritBoundaryFlagsFromParents

/*===========================================================================*/

void VectorNode::InheritBoundaryFlagsFromParent(VectorNode *Parent1)
{
unsigned long Flags1;

if ((Flags1 = Parent1->FlagCheck(WCS_VECTORNODE_FLAG_EASTEDGE)))
	FlagSet(WCS_VECTORNODE_FLAG_EASTEDGE);
if ((Flags1 = Parent1->FlagCheck(WCS_VECTORNODE_FLAG_WESTEDGE)))
	FlagSet(WCS_VECTORNODE_FLAG_WESTEDGE);
if ((Flags1 = Parent1->FlagCheck(WCS_VECTORNODE_FLAG_NORTHEDGE)))
	FlagSet(WCS_VECTORNODE_FLAG_NORTHEDGE);
if ((Flags1 = Parent1->FlagCheck(WCS_VECTORNODE_FLAG_SOUTHEDGE)))
	FlagSet(WCS_VECTORNODE_FLAG_SOUTHEDGE);
if ((Flags1 = Parent1->FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE)))
	FlagSet(WCS_VECTORNODE_FLAG_VECTORNODE);

} // VectorNode::InheritBoundaryFlagsFromParents

/*===========================================================================*/

// This function builds a list of all the nodes that are linked to this node (list also includes this node)
// The result is a VectorNodeLink series either appended to an originally supplied series
// or a brand new series.
// The owning polygons are not built into the list but probably could be if it proves necessary.
VectorNodeLink **VectorNode::MakeLinkedNodeList(VectorNodeLink *NodeList, VectorNodeLink **NewListPtr, VectorPolygon *NodeOwner)
{
VectorNodeLink *CurLink;
bool FoundNode;
VectorNodeLink *TestList;
VectorNode *LinkedNode;

// first node in the list will be this node
if (*NewListPtr = new VectorNodeLink(this, NodeOwner, 0))
//if (*NewListPtr = new VectorNodeLink(this, NodeOwner, NodeList ? (1 << 6): (1 << 7)))
	{
	if (! NodeList)
		NodeList = *NewListPtr;
	NewListPtr = (VectorNodeLink **)&(*NewListPtr)->NextNodeList;
	// look for linked nodes that are not already in the list
	for (CurLink = LinkedNodes; CurLink; CurLink = (VectorNodeLink *)CurLink->NextNodeList)
		{
		LinkedNode = CurLink->MyNode;
		FoundNode = false;
		for (TestList = NodeList; TestList; TestList = (VectorNodeLink *)TestList->NextNodeList)
			{
			if (TestList->MyNode == LinkedNode)
				{
				FoundNode = true;
				break;
				} // if
			} // for
		if (! FoundNode)
			{
			// linked node is not already in the list so list it and any nodes it is also linked to
			NewListPtr = LinkedNode->MakeLinkedNodeList(NodeList, NewListPtr, CurLink->LinkedPolygon);
			} // if
		} // for
	} // if

return (NewListPtr);

} // VectorNode::MakeLinkedNodeList

/*===========================================================================*/
/*===========================================================================*/

MaterialList::MaterialList(MaterialEffect *NewMat, GeneralEffect *NewEffect, Joe *NewJoe, float NewCoverage)
{

MatCoverage = NewCoverage;
MyVec = NewJoe;
MyMat = NewMat;
MyEffect = NewEffect;
NextMaterial = NULL;
WaterProp = NULL;
VectorProp = NULL;

} // MaterialList::MaterialList

/*===========================================================================*/

MaterialList::MaterialList()
{

MatCoverage = 0.0f;
MyVec = NULL;
MyMat = NULL;
MyEffect = NULL;
NextMaterial = NULL;
WaterProp = NULL;
VectorProp = NULL;

} // MaterialList::MaterialList

MaterialList::~MaterialList()
{

RemoveWaterProperties();
RemoveVectorProperties();

} // MaterialList::~MaterialList

/*===========================================================================*/

MatListWaterProperties *MaterialList::AddWaterProperties(void)
{

if (! WaterProp)
	WaterProp = new MatListWaterProperties();
	
return (WaterProp);

} // MaterialList::AddWaterProperties

/*===========================================================================*/

MatListVectorProperties *MaterialList::AddVectorProperties(void)
{

if (! VectorProp)
	VectorProp = new MatListVectorProperties();
	
return (VectorProp);

} // MaterialList::AddVectorProperties

/*===========================================================================*/

void MaterialList::RemoveWaterProperties(void)
{

if (WaterProp)
	{
	delete WaterProp;
	WaterProp = NULL;
	} // if

} // MaterialList::RemoveWaterProperties

/*===========================================================================*/

void MaterialList::RemoveVectorProperties(void)
{

if (VectorProp)
	{
	delete VectorProp;
	VectorProp = NULL;
	} // if
	
} // MaterialList::RemoveVectorProperties

/*===========================================================================*/
/*===========================================================================*/

MatListWaterProperties::MatListWaterProperties()
{

WaveHeight = WaterDepth = WaterNormal[0] = WaterNormal[1] = WaterNormal[2] = 0.0f;	
BeachOwner = false;

} // MatListWaterProperties::MatListWaterProperties

/*===========================================================================*/
/*===========================================================================*/

MatListVectorProperties::MatListVectorProperties()
{

VectorType = 0;
VectorSlope = VecOffsets[0] = VecOffsets[1] = VecOffsets[2] = 0.0f;	

} // MatListVectorProperties::MatListVectorProperties

/*===========================================================================*/
/*===========================================================================*/

VectorNodeRenderData::VectorNodeRenderData()
{

NodeEnvironment = NULL;
//LatSeed = LonSeed = 0;
RelEl = Normal[2] = Normal[1] = Normal[0] = Slope = Aspect = TexDisplacement = TfxDisplacement = 0.0f;
BeachLevel = (float)FLT_MAX;
Materials = NULL;
MaterialListOwned = true;

} // VectorNodeRenderData::VectorNodeRenderData

/*===========================================================================*/

VectorNodeRenderData::~VectorNodeRenderData()
{

if (MaterialListOwned)
	{
	for (MaterialList *CurMaterial = Materials; CurMaterial; CurMaterial = Materials)
		{
		Materials = Materials->NextMaterial;
		delete CurMaterial;
		} // for
	} // else
else
	Materials = NULL;

} // VectorNodeRenderData::~VectorNodeRenderData

/*===========================================================================*/

MaterialList *VectorNodeRenderData::AddMaterial(MaterialEffect *NewMat, GeneralEffect *NewEffect, Joe *NewJoe, float NewCoverage)
{

return (TestAddMaterial(NewMat, NewEffect, NewJoe, NewCoverage));

} // VectorNodeRenderData::AddMaterial

/*===========================================================================*/

MaterialList *VectorNodeRenderData::AddWaterMaterial(MaterialEffect *NewMat, GeneralEffect *NewEffect, Joe *NewJoe, float NewCoverage)
{
MaterialList *TestMatList;

if (TestMatList = TestAddMaterial(NewMat, NewEffect, NewJoe, NewCoverage))
	{
	if (! TestMatList->WaterProp)
		{
		if (! TestMatList->AddWaterProperties())
			return (NULL);
		} // if
	} // if

return (TestMatList);

} // VectorNodeRenderData::AddWaterMaterial

/*===========================================================================*/

MaterialList *VectorNodeRenderData::AddVectorMaterial(MaterialEffect *NewMat, GeneralEffect *NewEffect, Joe *NewJoe, float NewCoverage)
{
MaterialList *TestMatList;

if (TestMatList = TestAddMaterial(NewMat, NewEffect, NewJoe, NewCoverage))
	{
	if (! TestMatList->VectorProp)
		{
		if (! TestMatList->AddVectorProperties())
			return (NULL);
		} // if
	} // if

return (TestMatList);

} // VectorNodeRenderData::AddVectorMaterial

/*===========================================================================*/

MaterialList *VectorNodeRenderData::AddWaterVectorMaterial(MaterialEffect *NewMat, GeneralEffect *NewEffect, Joe *NewJoe, float NewCoverage)
{
MaterialList *TestMatList;

if (TestMatList = TestAddMaterial(NewMat, NewEffect, NewJoe, NewCoverage))
	{
	if (! TestMatList->WaterProp)
		{
		if (! TestMatList->AddWaterProperties())
			return (NULL);
		} // if
	if (! TestMatList->VectorProp)
		{
		if (! TestMatList->AddVectorProperties())
			return (NULL);
		} // if
	} // if

return (TestMatList);

} // VectorNodeRenderData::AddWaterVectorMaterial

/*===========================================================================*/

MaterialList *VectorNodeRenderData::TestAddMaterial(MaterialEffect *TestMat, GeneralEffect *TestEffect, Joe *TestJoe, float TestCoverage)
{
MaterialList *TestMatList, *LastMatList = NULL;

for (TestMatList = Materials; TestMatList; TestMatList = TestMatList->NextMaterial)
	{
	if (TestMatList->MyMat == TestMat && TestMatList->MyEffect == TestEffect && TestMatList->MyVec == TestJoe)
		return (TestMatList);
	LastMatList = TestMatList;
	} // for

if (LastMatList)
	{
	return (LastMatList->NextMaterial = new MaterialList(TestMat, TestEffect, TestJoe, TestCoverage));
	} // if
else
	{
	return (Materials = new MaterialList(TestMat, TestEffect, TestJoe, TestCoverage));
	} // if

} // VectorNodeRenderData::TestForMaterial

/*===========================================================================*/

MaterialList *VectorNodeRenderData::FindFirstEffect(long EffectType)
{
MaterialList *TestMatList;

for (TestMatList = Materials; TestMatList; TestMatList = TestMatList->NextMaterial)
	{
	if (TestMatList->MyEffect->EffectType == EffectType)
		return (TestMatList);
	} // for

return (NULL);

} // VectorNodeRenderData::FindFirstEffect

/*===========================================================================*/

MaterialList *VectorNodeRenderData::FindNextEffect(MaterialList *LastMatList, long EffectType)
{
MaterialList *TestMatList;

for (TestMatList = LastMatList ? LastMatList->NextMaterial: Materials; TestMatList; TestMatList = TestMatList->NextMaterial)
	{
	if (TestMatList->MyEffect->EffectType == EffectType)
		return (TestMatList);
	} // for

return (NULL);

} // VectorNodeRenderData::FindNextEffect

/*===========================================================================*/

MaterialList *VectorNodeRenderData::FindFirstMaterial(long MaterialType)
{
MaterialList *TestMatList;

for (TestMatList = Materials; TestMatList; TestMatList = TestMatList->NextMaterial)
	{
	if (TestMatList->MyMat->MaterialType == MaterialType)
		return (TestMatList);
	} // for

return (NULL);

} // VectorNodeRenderData::FindFirstMaterial

/*===========================================================================*/

MaterialList *VectorNodeRenderData::FindNextMaterial(MaterialList *LastMatList, long MaterialType)
{
MaterialList *TestMatList;

for (TestMatList = LastMatList ? LastMatList->NextMaterial: Materials; TestMatList; TestMatList = TestMatList->NextMaterial)
	{
	if (TestMatList->MyMat->MaterialType == MaterialType)
		return (TestMatList);
	} // for

return (NULL);

} // VectorNodeRenderData::FindNextMaterial

/*===========================================================================*/

void VectorNodeRenderData::CopyElevationData(VectorNodeRenderData *CopyFrom)
{

//LatSeed = CopyFrom->LatSeed;
//LonSeed = CopyFrom->LonSeed;

RelEl = CopyFrom->RelEl;
TexDisplacement = CopyFrom->TexDisplacement;
TfxDisplacement = CopyFrom->TfxDisplacement;

} // VectorNodeRenderData::CopyElevationData

/*===========================================================================*/

void VectorNodeRenderData::CopyNormalData(VectorNodeRenderData *CopyFrom)
{

Normal[0] = CopyFrom->Normal[0];
Normal[1] = CopyFrom->Normal[1];
Normal[2] = CopyFrom->Normal[2];
Slope = CopyFrom->Slope;
Aspect = CopyFrom->Aspect;

} // VectorNodeRenderData::CopyNormalData

/*===========================================================================*/

void VectorNodeRenderData::CopyNonElevationNormalData(VectorNodeRenderData *CopyFrom)
{

BeachLevel = CopyFrom->BeachLevel;
NodeEnvironment = CopyFrom->NodeEnvironment;
Materials = CopyFrom->Materials;
MaterialListOwned = false;

} // VectorNodeRenderData::CopyNonElevationNormalData

/*===========================================================================*/
/*===========================================================================*/

VectorNodeLink::VectorNodeLink()
{

LinkedPolygon = 0L; 
Flags = 0; 
#ifdef _DEBUG
++NumberOfNodeLinks;
#endif // _DEBUG

} // VectorNodeLink::VectorNodeLink

/*===========================================================================*/

VectorNodeLink::VectorNodeLink(VectorNode *NewNode, VectorPolygon *NewPolygon, unsigned char NewFlags)
: VectorNodeList(NewNode)
{

LinkedPolygon = NewPolygon;
Flags = NewFlags;
#ifdef _DEBUG
++NumberOfNodeLinks;
#endif // _DEBUG

} // VectorNodeLink::VectorNodeLink

/*===========================================================================*/

VectorNodeLink::~VectorNodeLink()
{
#ifdef _DEBUG
--NumberOfNodeLinks;
#endif // _DEBUG
} // VectorNodeLink::VectorNodeLink

/*===========================================================================*/
/*===========================================================================*/

VNAccelerator::VNAccelerator(void)
{
} // VNAcclerator::VNAccelerator

/*===========================================================================*/

VNAccelerator::~VNAccelerator()
{
} // VNAccelerator::~VNAccelerator

/*===========================================================================*/

void VNAccelerator::BinAllPoints(VectorNode* firstVN)
{
VectorNode* curVN = firstVN;

do
	{
	for (unsigned long binNum = 0; binNum < VNNUMACCELBINS; ++binNum)
		{
		if ((curVN->Lat >= accelBins[binNum]->latMin) && (curVN->Lat <= accelBins[binNum]->latMax) &&
			(curVN->Lon >= accelBins[binNum]->lonMin) && (curVN->Lon <= accelBins[binNum]->lonMax))
			{
#ifdef VNA_FAST_DELETE
			curVN->VNAbinNum = binNum;
#endif // VNA_FAST_DELETE
			accelBins[binNum]->AddNode(curVN);
			break;
			} // if
		} // for
	curVN = curVN->NextNode;
	} while (curVN != firstVN);

} // VNAccelerator::BinAllPoints

/*===========================================================================*/

void VNAccelerator::CreateBins(unsigned long numNodes)
{
unsigned long binSize;

binSize = numNodes * 2 / VNNUMACCELBINS;	// compute a reasonable starting bin size that is needed
if (binSize < 32)
	binSize = 32;

for (unsigned long binNum = 0; binNum < VNNUMACCELBINS; ++binNum)
	{
	if (accelBins[binNum]->binCapacity < binSize)
		{
		accelBins[binNum]->VNAccelMinBinSize(binSize);
		} // else if
	} // for

} // VNAccelerator::CreateBins

/*===========================================================================*/

void VNAccelerator::ComputeBinBounds(VectorNode* firstVN)
{
double curLat, curLon, latStep, lonStep, minLat, maxLat, minLon, maxLon;
VectorNode* curVN = firstVN;
unsigned long binNum = 0, x, y;

minLat = minLon = FLT_MAX;
maxLat = maxLon = -FLT_MAX;

do
	{
	if (curVN->Lat < minLat)
		minLat = curVN->Lat;
	if (curVN->Lat > maxLat)
		maxLat = curVN->Lat;
	if (curVN->Lon < minLon)
		minLon = curVN->Lon;
	if (curVN->Lon > maxLon)
		maxLon = curVN->Lon;
	curVN = curVN->NextNode;
	} while (curVN != firstVN);

latStep = (maxLat - minLat) / VNBINSPERSIDE;
lonStep = (maxLon - minLon) / VNBINSPERSIDE;

for (curLat = minLat, y = 0; y < VNBINSPERSIDE; ++y, curLat += latStep)
	{
	for (curLon = minLon, x = 0; x < VNBINSPERSIDE; ++x, curLon += lonStep, binNum++)
		{
		accelBins[binNum]->latMin = curLat;
		accelBins[binNum]->latMax = curLat + latStep;
		accelBins[binNum]->lonMin = curLon;
		accelBins[binNum]->lonMax = curLon + lonStep;
		} // for
	} // for

} // VNAccelerator::ComputeBinBounds

/*===========================================================================*/

void VNAccelerator::ClearBins(void)
{

for (unsigned long i = 0; i < VNNUMACCELBINS; ++i)
	accelBins[i]->numNodes = 0;

} // VNAccelerator::ClearBins

/*===========================================================================*/

void VNAccelerator::DeleteNode(VectorNode* deleteMe)
{
#ifndef VNA_FAST_DELETE
bool looking = true;

for (unsigned long binNum = 0; looking && (binNum < VNNUMACCELBINS); ++binNum)
	{
	if ((deleteMe->Lon >= accelBins[binNum]->lonMin) && (deleteMe->Lon <= accelBins[binNum]->lonMax) &&
		(deleteMe->Lat >= accelBins[binNum]->latMin) && (deleteMe->Lat <= accelBins[binNum]->latMax))
		{
		// Node is in this bin - test points
		for (unsigned long curNode = 0; curNode < accelBins[binNum]->numNodes; curNode++)
			{
			if (accelBins[binNum]->numNodes)
				{
				if (accelBins[binNum]->binNodes[curNode] == deleteMe)
					{
					accelBins[binNum]->binNodes[curNode] = NULL;
					looking = false;
					break;
					} // if
				} // if
			} // for
		} // if
	} // for
	
#else // VNA_FAST_DELETE

accelBins[deleteMe->VNAbinNum]->binNodes[deleteMe->VNAslotNum] = NULL;

#endif // VNA_FAST_DELETE

} // VNAccelerator::DeleteNode

/*===========================================================================*/

// Call once at end of render
void VNAccelerator::FreeAll(void)
{

if (accelBins)
	{
	for (unsigned long i = 0; i < VNNUMACCELBINS; ++i)
		{
		if (accelBins[i])
			AppMem_Free(accelBins[i], sizeof(VNAccelBin));
		} // for
	AppMem_Free(accelBins, VNNUMACCELBINS * sizeof(VNAccelBin *));
	} // if

} // VNAccelerator::FreeAll

/*===========================================================================*/

// Call once at beginning of render
void VNAccelerator::Initialize(void)
{

accelBins = (VNAccelBin **)AppMem_Alloc(VNNUMACCELBINS * sizeof(VNAccelBin *), APPMEM_CLEAR);
if (accelBins)
	{
	for (unsigned long i = 0; i < VNNUMACCELBINS; i++)
		accelBins[i] = (VNAccelBin *)AppMem_Alloc(sizeof(VNAccelBin), APPMEM_CLEAR);
	} // if

} // VNAccelerator::Initialize

/*===========================================================================*/

// Find bins that this TriangleBoundingBox covers, then test the points in those bins for containtment in triangle
int VNAccelerator::TestPointsInTriangle(TriangleBoundingBox* triBounds, VectorNode* firstNode, VectorNode* lastNode)
{
VectorNode *testNode;
int rVal = 1;

for (unsigned long binNum = 0; rVal && (binNum < VNNUMACCELBINS); binNum++)
	{
	if ((triBounds->MinX <= accelBins[binNum]->lonMax) && (triBounds->MaxX >= accelBins[binNum]->lonMin) &&
		(triBounds->MinY <= accelBins[binNum]->latMax) && (triBounds->MaxY >= accelBins[binNum]->latMin))
		{
		// triangle covers this bin - test points
		for (unsigned long curSlot = 0; rVal && (curSlot < accelBins[binNum]->numNodes); curSlot++)
			{
			if ((accelBins[binNum]->binNodes[curSlot]) && (! accelBins[binNum]->binNodes[curSlot]->LinkedNodes))
				{
				testNode = accelBins[binNum]->binNodes[curSlot];
				if ((testNode != triBounds->Pt1) && (testNode != triBounds->Pt2) && (testNode != triBounds->Pt3))
					{
					if (triBounds->TestPointInTriangle(testNode))
						rVal = 0;
					} // if
				} // if
			} // for
		} // if
	} // for

return rVal;

} // VNAccelerator::TestPointsInTriangle

/*===========================================================================*/
/*===========================================================================*/

VNAccelBin::VNAccelBin(unsigned long binSize)
{

latMin = lonMin = FLT_MAX;
latMax = lonMax = -FLT_MAX;
binCapacity = 0;
numNodes = 0;
binNodes = NULL;

} // VNAccelBin::VNAccelBin

/*===========================================================================*/

VNAccelBin::~VNAccelBin()
{

if (binNodes)
	{
	AppMem_Free(binNodes, binCapacity * sizeof(VectorNode *));
	binNodes = NULL;
	} // if

} // VNAccelBin::~VNAccelBin

/*===========================================================================*/

void VNAccelBin::AddNode(VectorNode* newNode)
{

if (numNodes != binCapacity)
	{
	binNodes[numNodes] = newNode;
#ifdef VNA_FAST_DELETE
	newNode->VNAslotNum = numNodes;
#endif // VNA_FAST_DELETE
	numNodes++;
	} // if
else
	{
	// make this bin twice as large
	VNAccelMinBinSize(binCapacity * 2);
	binNodes[numNodes] = newNode;	
#ifdef VNA_FAST_DELETE
	newNode->VNAslotNum = numNodes;
#endif // VNA_FAST_DELETE
	numNodes++;
	} // else

} // VNAccelBin::AddPoint

/*===========================================================================*/

void VNAccelBin::VNAccelMinBinSize(unsigned long binSize)
{
VectorNode **lastBinSet;

if (binNodes == NULL)
	{
	if (binNodes = (VectorNode **)AppMem_Alloc(binSize * sizeof(VectorNode *), APPMEM_CLEAR))
		binCapacity = binSize;
	} // if
else
	{
	// reallocate this bin to a larger size
	lastBinSet = binNodes;
	if (binNodes = (VectorNode **)AppMem_Alloc(binSize * sizeof(VectorNode *), APPMEM_CLEAR))
		{
		memcpy(binNodes, lastBinSet, binCapacity * sizeof(VectorNode *));
		AppMem_Free(lastBinSet, binCapacity * sizeof(VectorNode *));
		binCapacity = binSize;
		} // if
	} // else

} // VNAccelBin::VNAccelMinBinSize

/*===========================================================================*/
