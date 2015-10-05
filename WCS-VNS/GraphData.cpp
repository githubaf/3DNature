// GraphData.cpp
// For storage and manipulation of GraphData for World Construction Set v4.
// Built from scratch on 4/13/98 by Gary R. Huber
// Copyright 1998 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "Application.h"
#include "GraphData.h"
#include "AppMem.h"
#include "Useful.h"
#include "EffectsIO.h"
#include "Interactive.h"
#include "Requester.h"
#include "Raster.h"
#include "Project.h"
#include "Texture.h"
#include "EffectsLib.h"
#include "Conservatory.h"
#include "DragnDropListGUI.h"
#include "Toolbar.h"
#include "ColorEditGUI.h"
#include "AnimGraphGUI.h"
#include "Random.h"
#include "Lists.h"

static int SiblingOpInProgress;

CrossSectionData::CrossSectionData(RasterAnimHost *Parent)
{
double RangeDefaults[2][3] = {1000.0, 0.0, .1, 5.0, 0.0, .01};

Eco = NULL;
EcoMixing.SetDefaults(Parent, 0, 0.0);
EcoMixing.SetRangeDefaults(RangeDefaults[0]);
Roughness.SetDefaults(Parent, 1, 1.0);
Roughness.SetRangeDefaults(RangeDefaults[1]);
Priority = 0;

EcoMixing.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
Roughness.SetMultiplier(100.0);
EcoMixing.SetNoNodes(1);
Roughness.SetNoNodes(1);

} // CrossSectionData::CrossSectionData

/*===========================================================================*/

void CrossSectionData::Copy(CrossSectionData *CopyTo, CrossSectionData *CopyFrom)
{

if (CopyFrom->Eco)
	CopyTo->Eco = (EcosystemEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Eco);
else
	CopyTo->Eco = NULL;
EcoMixing.Copy(&CopyTo->EcoMixing, &CopyFrom->EcoMixing);
Roughness.Copy(&CopyTo->Roughness, &CopyFrom->Roughness);
CopyTo->Priority = CopyFrom->Priority;

} // CrossSectionData::Copy

/*===========================================================================*/

void CrossSectionData::SetEco(EcosystemEffect *NewEco)
{

Eco = NewEco;

} // CrossSectionData::SetEco

/*===========================================================================*/

long CrossSectionData::InitImageIDs(long &ImageID)
{
long NumImages = 0;

if (Eco)
	NumImages += Eco->InitImageIDs(ImageID);

return (NumImages);

} // CrossSectionData::InitImageIDs

/*===========================================================================*/

int CrossSectionData::BuildFileComponentsList(EffectList **Ecosystems, EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
EffectList **ListPtr;

if (Eco)
	{
	ListPtr = Ecosystems;
	while (*ListPtr)
		{
		if ((*ListPtr)->Me == Eco)
			break;
		ListPtr = &(*ListPtr)->Next;
		} // if
	if (! (*ListPtr))
		{
		if (*ListPtr = new EffectList())
			(*ListPtr)->Me = Eco;
		else
			return (0);
		} // if
	if (! Eco->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
		return (0);
	} // if

return (1);

} // CrossSectionData::BuildFileComponentsList

/*===========================================================================*/

int CrossSectionData::SetToTime(double Time)
{
long Found = 0;

if (EcoMixing.SetToTime(Time))
	Found = 1;
if (Roughness.SetToTime(Time))
	Found = 1;

return (Found);

} // CrossSectionData::SetToTime

/*===========================================================================*/

int CrossSectionData::ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop)
{
int Success = 0;

if (EcoMixing.ScaleDeleteAnimNodes(Prop))
	Success = 1;
if (Roughness.ScaleDeleteAnimNodes(Prop))
	Success = 1;

return (Success);

} // CrossSectionData::ScaleDeleteAnimNodes

/*===========================================================================*/

int CrossSectionData::GetNextAnimNode(RasterAnimHostProperties *Prop)
{
int Success = 0;

if (EcoMixing.GetNextAnimNode(Prop))
	Success = 1;
if (Roughness.GetNextAnimNode(Prop))
	Success = 1;

return (Success);

} // CrossSectionData::GetNextAnimNode

/*===========================================================================*/

ULONG CrossSectionData::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char EcoName[256];

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} /* switch */

				switch (ItemTag & 0xffff0000)
					{
					case WCS_CROSSSECTION_PRIORITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Priority, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_CROSSSECTION_ECOMIXING:
						{
						BytesRead = EcoMixing.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_CROSSSECTION_ROUGHNESS:
						{
						BytesRead = Roughness.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_CROSSSECTION_ECONAME:
						{
						BytesRead = ReadBlock(ffile, (char *)EcoName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (EcoName[0])
							{
							Eco = (EcosystemEffect *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_ECOSYSTEM, EcoName);
							} // if
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // CrossSectionData::Load

/*===========================================================================*/

unsigned long CrossSectionData::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_CROSSSECTION_PRIORITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Priority)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_CROSSSECTION_ECOMIXING + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = EcoMixing.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if ecomixing saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_CROSSSECTION_ROUGHNESS + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Roughness.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if roughness saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

if (Eco)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_CROSSSECTION_ECONAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Eco->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Eco->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // CrossSectionData::Save

/*===========================================================================*/
/*===========================================================================*/

GraphNode::GraphNode()
{

SetDefaults();

} // GraphNode::GraphNode

/*===========================================================================*/

GraphNode::~GraphNode()
{

if (Data)
	delete Data;
Data = NULL;

} // GraphNode::~GraphNode

/*===========================================================================*/

void GraphNode::SetDefaults(void)
{

Value = Distance = TCB[0] = TCB[1] = TCB[2] = 0.0;
Linear = 0;
Next = Prev = NULL;
Flags = 0;
Data = NULL;

} // GraphNode::SetDefaults

/*===========================================================================*/

CrossSectionData *GraphNode::AddData(RasterAnimHost *Parent)
{

return (Data = new CrossSectionData(Parent));

} // GraphNode::AddData

/*===========================================================================*/

void GraphNode::Copy(GraphNode *CopyTo, GraphNode *CopyFrom, RasterAnimHost *Parent)
{

CopyTo->Value = CopyFrom->Value;
CopyTo->Distance = CopyFrom->Distance;
CopyTo->TCB[0] = CopyFrom->TCB[0];
CopyTo->TCB[1] = CopyFrom->TCB[1];
CopyTo->TCB[2] = CopyFrom->TCB[2];
CopyTo->Linear = CopyFrom->Linear;
if (CopyFrom->Data)
	{
	if (CopyTo->Data || CopyTo->AddData(Parent))
		CopyTo->Data->Copy(CopyTo->Data, CopyFrom->Data);
	} // if
else if (CopyTo->Data)
	{
	delete CopyTo->Data;
	CopyTo->Data = NULL;
	} // else if
} // GraphNode::Copy

/*===========================================================================*/

ULONG GraphNode::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, RasterAnimHost *Parent)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_GRAPHNODE_DISTANCE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Distance, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_GRAPHNODE_VALUE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Value, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_GRAPHNODE_TENSION:
						{
						BytesRead = ReadBlock(ffile, (char *)&TCB[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_GRAPHNODE_CONTINUITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&TCB[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_GRAPHNODE_BIAS:
						{
						BytesRead = ReadBlock(ffile, (char *)&TCB[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_GRAPHNODE_LINEAR:
						{
						BytesRead = ReadBlock(ffile, (char *)&Linear, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_GRAPHNODE_CROSSSECTIONDATA:
						{
						if (Data || AddData(Parent))
							BytesRead = Data->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
		else
			break;
	} // while 

return (TotalRead);

} // GraphNode::Load

/*===========================================================================*/

unsigned long GraphNode::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHNODE_DISTANCE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Distance)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHNODE_VALUE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Value)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHNODE_TENSION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&TCB[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHNODE_CONTINUITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&TCB[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHNODE_BIAS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&TCB[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHNODE_LINEAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Linear)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (Data)
	{
	ItemTag = WCS_GRAPHNODE_CROSSSECTIONDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Data->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if cross-section data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // GraphNode::Save

/*===========================================================================*/

GraphData::GraphData()
{

SetDefaults();

} // GraphData::GraphData

/*===========================================================================*/

GraphData::~GraphData()
{

ReleaseNodes();

} // GraphData::~GraphData

/*===========================================================================*/

void GraphData::SetDefaults(void)
{

Nodes = NULL;
NumNodes = 0;
FirstDist = LastDist = 0.0;

} // GraphData::SetDefaults

/*===========================================================================*/

void GraphData::Copy(GraphData *CopyTo, GraphData *CopyFrom, RasterAnimHost *Parent)
{
GraphNode *TempNodeA, *TempNodeB;

CopyTo->ReleaseNodes();
if (CopyFrom->Nodes && CopyTo->NewNodes(CopyFrom->NumNodes))
	{
	TempNodeA = CopyFrom->Nodes;
	TempNodeB = CopyTo->Nodes;
	while (TempNodeA && TempNodeB)
		{
		TempNodeB->Copy(TempNodeB, TempNodeA, Parent);
		TempNodeA = TempNodeA->Next;
		TempNodeB = TempNodeB->Next;
		} // for
	CopyTo->FirstDist = CopyFrom->FirstDist;
	CopyTo->LastDist = CopyFrom->LastDist;
	} // if

} // GraphData::Copy

/*===========================================================================*/

void GraphData::ReleaseNodes()
{
GraphNode *TempNode;

while (Nodes)
	{
	TempNode = Nodes->Next;
	delete Nodes;
	Nodes = TempNode;
	} // while
NumNodes = 0;
FirstDist = LastDist = 0.0;

} // GraphData::ReleaseNodes

/*===========================================================================*/

GraphNode *GraphData::NewNodes(long NewNumNodes)
{
GraphNode *PrevNode;
long Ct;

ReleaseNodes();
if (NewNumNodes > 0)
	{
	if (Nodes = new GraphNode)
		{
		NumNodes = 1;
		PrevNode = Nodes;
		for (Ct = 1; Ct < NewNumNodes; Ct ++)
			{
			if (PrevNode->Next = new GraphNode)
				{
				PrevNode->Next->Prev = PrevNode;
				PrevNode = PrevNode->Next;
				NumNodes ++;
				} // if
			else
				{
				ReleaseNodes();
				return (NULL);
				} // else
			} // for
		} // for
	} // if
FirstDist = LastDist = 0.0;

return (Nodes);

} // GraphData::NewNodes

/*===========================================================================*/

void GraphData::SortNodesByDistance(void)
{
int MoreToDo = 1;
GraphNode *MyNode;

if (NumNodes > 0)
	{
	while (MoreToDo)
		{
		MoreToDo = 0;
		MyNode = Nodes;
		while (MyNode->Next)
			{
			if (MyNode->Distance < 0.0)
				{
				DeleteNode(MyNode->Next, MyNode);
				} // else if
			else if (MyNode->Distance > MyNode->Next->Distance)
				{
				SwapNodes(MyNode, MyNode->Next);
				MoreToDo = 1;
				} // if
			else if (MyNode->Distance == MyNode->Next->Distance)
				{
				DeleteNode(MyNode->Next, MyNode);
				} // else if
			else
				{
				MyNode = MyNode->Next;
				} // else
			} // while
		} // while
	} // if

} // GraphData::SortNodesByDistance

/*===========================================================================*/

GraphNode *GraphData::SwapNodes(GraphNode *First, GraphNode *Last)
{
GraphNode *TempNode;

if (First && Last)
	{
	TempNode = Last->Next;
	if (First->Prev)
		First->Prev->Next = Last;
	Last->Prev = First->Prev;
	Last->Next = First;
	First->Prev = Last;
	First->Next = TempNode;
	if (First->Next)
		First->Next->Prev = First;
	return (Last);
	} // if

return (First);

} // GraphData::SwapNodes

/*===========================================================================*/
// swiped the spline code from Joe::SplineElevation
double GraphData::GetValue(double Dist)
{
GraphNode *PrevPrevNode, *PrevNode, *NextNode, *NextNextNode;
double P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4, Space;

if (FindSplineNodes(Dist, PrevPrevNode, PrevNode, NextNode, NextNextNode))
	{
	if (Dist <= PrevNode->Distance)
		return (PrevNode->Value);
	if (NextNode && (Space = NextNode->Distance - PrevNode->Distance) > 0.0)
		{
		P1 = PrevNode->Value;
		P2 = NextNode->Value;

		if (NextNode->Linear)
			{
			return (P1 + (P2 - P1) * ((Dist - PrevNode->Distance) / Space));
			} // if
		else
			{
			D1 = PrevPrevNode ?
				((.5 * (P1 - PrevPrevNode->Value)
				* (1.0 - PrevNode->TCB[0])
				* (1.0 + PrevNode->TCB[1])
				* (1.0 + PrevNode->TCB[2]))
				+ (.5 * (P2 - P1)
				* (1.0 - PrevNode->TCB[0])
				* (1.0 - PrevNode->TCB[1])
				* (1.0 - PrevNode->TCB[2])))
				* (2.0 * Space / ((PrevNode->Distance - PrevPrevNode->Distance) + Space)):
				((.5 * (P2 - P1)
				* (1.0 - PrevNode->TCB[0])
				* (1.0 + PrevNode->TCB[1])
				* (1.0 + PrevNode->TCB[2]))
				+ (.5 * (P2 - P1)
				* (1.0 - PrevNode->TCB[0])
				* (1.0 - PrevNode->TCB[1])
				* (1.0 - PrevNode->TCB[2])));

			D2 = NextNextNode ?
				((.5 * (P2 - P1)
				* (1.0 - NextNode->TCB[0])
				* (1.0 - NextNode->TCB[1])
				* (1.0 + NextNode->TCB[2]))
				+ (.5 * (NextNextNode->Value - P2)
				* (1.0 - NextNode->TCB[0])
				* (1.0 + NextNode->TCB[1])
				* (1.0 - NextNode->TCB[2])))
				* (2.0 * Space / (Space + (NextNextNode->Distance - NextNode->Distance))):
				((.5 * (P2 - P1)
				* (1.0 - NextNode->TCB[0])
				* (1.0 - NextNode->TCB[1])
				* (1.0 + NextNode->TCB[2]))
				+ (.5 * (P2 - P1)
				* (1.0 - NextNode->TCB[0])
				* (1.0 + NextNode->TCB[1])
				* (1.0 - NextNode->TCB[2])));

			S1 = (Dist - PrevNode->Distance) / Space;
			S2 = S1 * S1;
			S3 = S1 * S2;
			h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
			h2 = -2.0 * S3 + 3.0 * S2;
			h3 = S3 - 2.0 * S2 + S1;
			h4 = S3 - S2;
			return (P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4);
			} // else
		} // if nextnode is valid - interesting point lies between prev and next
	return (PrevNode->Value);		
	} // if Prevnode is valid
return (0.0);

} // GraphData::GetValue

/*===========================================================================*/

/*===========================================================================*/
// swiped the spline code from Joe::SplineElevation
double GraphData::GetValueBoolean(double Dist)
{
GraphNode *PrevPrevNode, *PrevNode, *NextNode, *NextNextNode;

if (FindSplineNodes(Dist, PrevPrevNode, PrevNode, NextNode, NextNextNode))
	{
	return (PrevNode->Value > .5 ? 1.0: 0.0);		
	} // if Prevnode is valid
return (0.0);

} // GraphData::GetValueBoolean

/*===========================================================================*/

GraphNode *GraphData::FindNearestNode(double Dist)
{
GraphNode *TempNode = Nodes, *Found = NULL;
double TempDist, NearestDist = FLT_MAX;

while (TempNode)
	{
	if ((TempDist = fabs(Dist - TempNode->Distance)) < NearestDist)
		{
		NearestDist = TempDist;
		Found = TempNode;
		if (TempNode->Distance >= Dist)
			break;
		} // if
	TempNode = TempNode->Next;
	} // while

return (Found);

} // GraphData::FindNearestNode

/*===========================================================================*/

GraphNode *GraphData::FindNode(double Dist, double DistRange, GraphNode *&PrevNode)
{
GraphNode *TempNode = Nodes;

PrevNode = NULL;

while (TempNode)
	{
	if (TempNode->Distance == Dist)
		break;	// we found a match
	if (TempNode->Distance > Dist - DistRange)
		{
		if (TempNode->Distance < Dist + DistRange)
			{
			break;
			} // if we found a match
		else
			{
			TempNode = NULL;
			break;
			} // else we've gone past it
		} // if
	PrevNode = TempNode;
	TempNode = TempNode->Next;
	} // while

return (TempNode);

} // GraphData::FindNode

/*===========================================================================*/

GraphNode *GraphData::FindNode(double Dist, double DistRange, double Value, double ValueRange)
{
GraphNode *TempNode = Nodes;

while (TempNode)
	{
	if (TempNode->Distance >= Dist - DistRange)
		{
		if (TempNode->Distance <= Dist + DistRange)
			{
			if (TempNode->Value >= Value - ValueRange)
				{
				if (TempNode->Value <= Value + ValueRange)
					{
					break;
					} // if values match
				} // if
			} // if distances match
		else
			{
			TempNode = NULL;
			break;
			} // else we've gone past it
		} // if
	TempNode = TempNode->Next;
	} // while

return (TempNode);

} // GraphData::FindNode

/*===========================================================================*/
// this could be optimized if needed for building a table of values - don't re-init the node pointers each time
GraphNode *GraphData::FindSplineNodes(double Dist, GraphNode *&PrevPrevNode, GraphNode *&PrevNode, GraphNode *&NextNode, GraphNode *&NextNextNode)
{

PrevPrevNode = NextNode = NextNextNode = NULL;
if (PrevNode = Nodes)
	{
	NextNode = Nodes->Next;

	while (PrevNode)
		{
		if (PrevNode->Distance == Dist)
			{
			PrevPrevNode = NextNode = NULL;
			break;
			} // if
		if (NextNode)
			{
			if (Dist < NextNode->Distance)
				{
				NextNextNode = NextNode->Next;
				break;
				} // if
			PrevPrevNode = PrevNode;
			PrevNode = NextNode;
			NextNode = NextNode->Next;
			} // if
		else
			break;
		} // while
	} // if

return (PrevNode);

} // GraphData::FindNode

/*===========================================================================*/

GraphNode *GraphData::InsertNode(double Dist, GraphNode *&PrevNode)
{
GraphNode *TempNode;

if (TempNode = new GraphNode)
	{
	TempNode->Distance = Dist;
	if (PrevNode)
		{
		if (TempNode->Next = PrevNode->Next)
			TempNode->Next->Prev = TempNode;
		TempNode->Prev = PrevNode;
		PrevNode->Next = TempNode;
		} // if
	else
		{
		if (TempNode->Next = Nodes)
			TempNode->Next->Prev = TempNode;
		Nodes = TempNode;
		} // else
	if (! TempNode->Next)
		LastDist = Dist;
	if (! TempNode->Prev)
		FirstDist = Dist;
	NumNodes ++;
	} // if

return (TempNode);

} // GraphData::InsertNode

/*===========================================================================*/

GraphNode *GraphData::DeleteNode(GraphNode *TempNode, GraphNode *&PrevNode)
{
GraphNode *NewActive = NULL;

if (TempNode)
	{
	if (PrevNode)
		{
		if (PrevNode->Next = TempNode->Next)
			TempNode->Next->Prev = PrevNode;
		NewActive = PrevNode->Next ? PrevNode->Next: PrevNode;
		} // if
	else
		{
		if (Nodes = TempNode->Next)
			TempNode->Next->Prev = NULL;
		NewActive = Nodes;
		} // else
	if (! TempNode->Next)
		LastDist = TempNode->Prev ? TempNode->Prev->Distance: 0.0;
	if (! TempNode->Prev)
		FirstDist = TempNode->Next ? TempNode->Next->Distance: 0.0;
	delete TempNode;
	NumNodes --;
	} // if

return (NewActive);

} // GraphData::DeleteNode

/*===========================================================================*/

GraphNode *GraphData::AddNode(double Dist, double DistRange, double NewVal)
{
GraphNode *TempNode, *PrevNode;

if ((TempNode = FindNode(Dist, DistRange, PrevNode)) || (TempNode = InsertNode(Dist, PrevNode)))
	{
	TempNode->Value = NewVal;
	} // if
return (TempNode);

} // GraphData::AddNode

/*===========================================================================*/

// This is used for building AnimDoubleDistance classes from vectors. There is no checking to see if
// minimum distance spacing between nodes is maintained. Do not use this function to create
// any kind of Cross section profile for terraffectors.
GraphNode *GraphData::AddNodeEnd(GraphNode *PrevNode, double Dist, double DistRange, double NewVal)
{
GraphNode *TempNode;

if (TempNode = InsertNode(Dist, PrevNode))
	{
	TempNode->Value = NewVal;
	} // if
return (TempNode);

} // GraphData::AddNodeEnd

/*===========================================================================*/

GraphNode *GraphData::RemoveNode(double Dist, double DistRange)
{
GraphNode *TempNode, *PrevNode;

if (TempNode = FindNode(Dist, DistRange, PrevNode))
	{
	TempNode = DeleteNode(TempNode, PrevNode);
	} // if
return (TempNode);

} // GraphData::RemoveNode

/*===========================================================================*/

GraphNode *GraphData::UpdateNode(double Dist, double DistRange, double NewVal)
{
GraphNode *TempNode, *PrevNode;

if (TempNode = FindNode(Dist, DistRange, PrevNode))
	{
	TempNode->Value = NewVal;
	} // if
return (TempNode);

} // GraphData::UpdateNode

/*===========================================================================*/

void GraphData::SetNodeSelectedAll(void)
{
GraphNode *MyNode = Nodes;

while (MyNode)
	{
	MyNode->SetFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
	MyNode = MyNode->Next;
	} // while

} // GraphData::SetNodeSelectedAll

/*===========================================================================*/

void GraphData::ClearNodeSelectedAll(void)
{
GraphNode *MyNode = Nodes;

while (MyNode)
	{
	MyNode->ClearFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
	MyNode = MyNode->Next;
	} // while

} // GraphData::ClearNodeSelectedAll

/*===========================================================================*/

void GraphData::ToggleNodeSelectedAll(void)
{
GraphNode *MyNode = Nodes;

while (MyNode)
	{
	MyNode->ToggleFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
	MyNode = MyNode->Next;
	} // while

} // GraphData::ToggleNodeSelectedAll

/*===========================================================================*/

void GraphData::SetNodeModifiedAll(void)
{
GraphNode *MyNode = Nodes;

while (MyNode)
	{
	MyNode->SetFlag(WCS_GRAPHNODE_FLAGS_MODIFIED);
	MyNode = MyNode->Next;
	} // while

} // GraphData::SetNodeModifiedAll

/*===========================================================================*/

void GraphData::ClearNodeModifiedAll(void)
{
GraphNode *MyNode = Nodes;

while (MyNode)
	{
	MyNode->ClearFlag(WCS_GRAPHNODE_FLAGS_MODIFIED);
	MyNode = MyNode->Next;
	} // while

} // GraphData::ClearNodeModifiedAll

/*===========================================================================*/

void GraphData::SetNodeLinearAll(char NewValue)
{
GraphNode *MyNode = Nodes;

while (MyNode)
	{
	MyNode->SetLinear(NewValue);
	MyNode = MyNode->Next;
	} // while

} // GraphData::SetNodeLinearAll

/*===========================================================================*/

long GraphData::CountSelectedNodes(void)
{
GraphNode *MyNode = Nodes;
long Selected = 0;

while (MyNode)
	{
	if (MyNode->GetSelectedState())
		Selected ++;
	MyNode = MyNode->Next;
	} // while
return (Selected);

} // GraphData::CountSelectedNodes

/*===========================================================================*/

int GraphData::GetMinMaxVal(double &FindMin, double &FindMax)
{
GraphNode *MyNode;
int Found = 1;

FindMax = -FLT_MAX;
FindMin = FLT_MAX;

if (MyNode = Nodes)
	{
	while (MyNode)
		{
		if (MyNode->Value < FindMin)
			FindMin = MyNode->Value;
		if (MyNode->Value > FindMax)
			FindMax = MyNode->Value;
		MyNode = MyNode->Next;
		} // while
	} // if
else
	{
	FindMax = FindMin = 0.0;
	Found = 0;
	} // if
return (Found);

} // GraphData::GetMinMaxVal

/*===========================================================================*/

int GraphData::GetMinMaxDist(double &FindMin, double &FindMax)
{
GraphNode *MyNode;
int Found = 1;

if (MyNode = Nodes)
	{
	while (MyNode->Next)
		{
		MyNode = MyNode->Next;
		} // while
	FindMax = MyNode->Distance;
	FindMin = Nodes->Distance;
	Found = 1;
	} // if
else
	{
	FindMax = FindMin = 0.0;
	Found = 0;
	} // if
return (Found);

} // GraphData::GetMinMaxDist

/*===========================================================================*/

int GraphData::GetNextDist(double &NewDist, short Direction, double CurrentDist)
{
GraphNode *MyNode;
int Found = 0;

if (MyNode = Nodes)
	{
	if (Direction > 0)
		{
		while (MyNode)
			{
			if (MyNode->Distance > CurrentDist)
				{
				NewDist = MyNode->Distance;
				return (1);
				} // if
			MyNode = MyNode->Next;
			} // while
		} // if
	else
		{
		while (MyNode)
			{
			if (MyNode->Distance < CurrentDist)
				{
				NewDist = MyNode->Distance;
				Found = 1;
				} // if
			else
				break;
			MyNode = MyNode->Next;
			} // while
		} // else
	} // if
if (! Found)
	{
	NewDist = 0.0;
	} // if
return (Found);

} // GraphData::GetNextDist

/*===========================================================================*/

int GraphData::GetNextAnimNode(RasterAnimHostProperties *Prop)
{
int Found = 0;
double NewDist;

if (GetNextDist(NewDist, 1, Prop->KeyNodeRange[1]))
	{
	if (NewDist < Prop->NewKeyNodeRange[1])
		{
		Prop->NewKeyNodeRange[1] = NewDist;
		Found = 1;
		} // if
	} // if
if (GetNextDist(NewDist, 0, Prop->KeyNodeRange[0]))
	{
	if (NewDist > Prop->NewKeyNodeRange[0])
		{
		Prop->NewKeyNodeRange[0] = NewDist;
		Found = 1;
		} // if
	} // if

return (Found);

} // GraphData::GetNextAnimNode

/*===========================================================================*/

int GraphData::DeleteDistRange(double DeleteFirstDist, double DeleteLastDist)
{
GraphNode *MyNode;
int Found = 0;

if (MyNode = Nodes)
	{
	while (MyNode)
		{
		if (MyNode->Distance >= DeleteFirstDist && MyNode->Distance <= DeleteLastDist)
			{
			Found = 1;
			MyNode = RemoveNode(MyNode->Distance, 0.0);
			} // if
		else
			MyNode = MyNode->Next;
		} // while
	} // if

return (Found);

} // GraphData::DeleteDistRange

/*===========================================================================*/

int GraphData::AdjustDistRange(double OldFirstDist, double OldLastDist, double NewFirstDist, double NewLastDist)
{
GraphNode *MyNode;
int Found = 0;
double ScaleFactor;

if (MyNode = Nodes)
	{
	while (MyNode)
		{
		if (MyNode->Distance >= OldFirstDist && MyNode->Distance <= OldLastDist)
			{
			Found = 1;
			if (OldLastDist > OldFirstDist)
				{
				ScaleFactor = (NewLastDist - NewFirstDist) / (OldLastDist - OldFirstDist);
				MyNode->Distance = (MyNode->Distance - OldFirstDist) * ScaleFactor + NewFirstDist;
				if (MyNode->Distance < 0.0)
					MyNode->Distance = 0.0;
				} // if
			else
				MyNode->Distance = NewFirstDist;
			} // if
		MyNode = MyNode->Next;
		} // while
	if (Found)
		SortNodesByDistance();
	} // if
	
return (Found);

} // GraphData::AdjustDistRange

/*===========================================================================*/

int GraphData::ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop)
{

if (Prop->KeyframeOperation == WCS_KEYOPERATION_DELETE)
	{
	if (Prop->FrameOperator == WCS_KEYOPERATION_ONEKEY)
		return (DeleteDistRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[0]));
	else if (Prop->FrameOperator == WCS_KEYOPERATION_KEYRANGE)
		return (DeleteDistRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]));
	else if (Prop->FrameOperator == WCS_KEYOPERATION_ALLKEYS)
		return (DeleteDistRange(-DBL_MAX, DBL_MAX));
	else
		return (0);
	} // if
else if (Prop->KeyframeOperation == WCS_KEYOPERATION_SCALE)
	{
	if (Prop->FrameOperator == WCS_KEYOPERATION_ONEKEY)
		return (AdjustDistRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[0], Prop->NewKeyNodeRange[0], Prop->NewKeyNodeRange[0]));
	else if (Prop->FrameOperator == WCS_KEYOPERATION_KEYRANGE)
		return (AdjustDistRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1], Prop->NewKeyNodeRange[0], Prop->NewKeyNodeRange[1]));
	else if (Prop->FrameOperator == WCS_KEYOPERATION_ALLKEYS)
		return (AdjustDistRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1], Prop->NewKeyNodeRange[0], Prop->NewKeyNodeRange[1]));
	else
		return (0);
	} // else
else
	return (0);

} // GraphData::ScaleDeleteAnimNodes

/*===========================================================================*/

GraphNode *GraphData::GetFirstSelectedNode(void)
{
GraphNode *MyNode;

if (MyNode = Nodes)
	{
	while (MyNode)
		{
		if (MyNode->GetSelectedState())
			return (MyNode);
		MyNode = MyNode->Next;
		} // while
	} // if

return (NULL);

} // GraphData::GetFirstSelectedNode

/*===========================================================================*/

ULONG GraphData::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, AnimCritter *Parent)
{
double LastNodeDistance = 0.0;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
long NodeNum = -1;
GraphNode *TempNode = NULL;

ReleaseNodes();

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch

				switch (ItemTag & 0xffff0000)
					{
					case WCS_GRAPHDATA_NODE:
						{
						if (Nodes && NodeNum >= 0 && NodeNum < NumNodes && TempNode)
							{
							BytesRead = TempNode->Load(ffile, Size, ByteFlip, Parent);
							if (NodeNum > 0)
								{
								if (TempNode->Distance < LastNodeDistance + Parent->GetMinDistSeparation())
									TempNode->Distance = LastNodeDistance + Parent->GetMinDistIncrement();
								} // if
							LastNodeDistance = TempNode->Distance;
							TempNode = TempNode->Next;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_GRAPHDATA_NUMNODES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumNodes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						Nodes = NewNodes(NumNodes);
						TempNode = Nodes;
						break;
						}
					case WCS_GRAPHDATA_NODENUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&NodeNum, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_GRAPHDATA_FIRSTDIST:
						{
						BytesRead = ReadBlock(ffile, (char *)&FirstDist, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_GRAPHDATA_LASTDIST:
						{
						BytesRead = ReadBlock(ffile, (char *)&LastDist, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read
			else
				break;
			} // if not done flag
		} // if tag block read
	else
		break;
	} // while

if (Nodes)
	LastDist = LastNodeDistance;

return (TotalRead);

} // GraphData::Load

/*===========================================================================*/

unsigned long GraphData::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, NodeNum;
GraphNode *TempNode;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHDATA_FIRSTDIST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&FirstDist)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHDATA_LASTDIST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&LastDist)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (Nodes)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHDATA_NUMNODES, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&NumNodes)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	for (NodeNum = 0, TempNode = Nodes; NodeNum < NumNodes && TempNode; NodeNum ++, TempNode = TempNode->Next)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHDATA_NODENUM, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, (char *)&NodeNum)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		ItemTag = WCS_GRAPHDATA_NODE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TempNode->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if node saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // for
	} // if nodes
else
	{
	NumNodes = 0;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRAPHDATA_NUMNODES, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&NumNodes)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // else

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // GraphData::Save

/*===========================================================================*/

AnimCritter::AnimCritter()
: RasterAnimHost(NULL)
{

SetDefaults();

} // AnimCritter::AnimCritter

/*===========================================================================*/

AnimCritter::~AnimCritter()
{
char Ct;

for (Ct = 0; Ct < WCS_ANIM_MAX_NUMGRAPHS; Ct ++)
	{
	ReleaseGraph(Ct);
	} // if

} // AnimCritter::~AnimCritter

/*===========================================================================*/

void AnimCritter::SetDefaults(void)
{
char Ct;

for (Ct = 0; Ct < WCS_ANIM_MAX_NUMGRAPHS; Ct ++)
	{
	Graph[Ct] = NULL;
	} // if
MaxMin[0] = FLT_MAX;
MaxMin[1] = -FLT_MAX;
Increment = Multiplier = 1.0;
NotifyItem = 0;
MetricType = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
FlagBits = 0;

} // AnimCritter::SetDefaults

/*===========================================================================*/

void AnimCritter::SetDefaults(RasterAnimHost *RAHost, unsigned char Item)
{
long Ct;

for (Ct = 0; Ct < WCS_ANIM_MAX_NUMGRAPHS; Ct ++)
	{
	Graph[Ct] = NULL;
	} // if
MaxMin[0] = FLT_MAX;
MaxMin[1] = -FLT_MAX;
Increment = 1.0;
RAParent = RAHost;
NotifyItem = Item;

} // AnimCritter::SetDefaults

/*===========================================================================*/

void AnimCritter::SetRangeDefaults(double *Defaults)
{

MaxMin[0] = Defaults[0];
MaxMin[1] = Defaults[1];
Increment = Defaults[2];

} // AnimCritter::SetRangeDefaults

/*===========================================================================*/

void AnimCritter::GetRangeDefaults(double *Defaults)
{

Defaults[0] = MaxMin[0];
Defaults[1] = MaxMin[1];
Defaults[2] = Increment;

} // AnimCritter::GetRangeDefaults

/*===========================================================================*/

void AnimCritter::Copy(AnimCritter *CopyTo, AnimCritter *CopyFrom)
{
char Ct;

for (Ct = 0; Ct < WCS_ANIM_MAX_NUMGRAPHS; Ct ++)
	{
	CopyTo->ReleaseGraph(Ct);
	if (! CopyTo->TestFlags(WCS_ANIMCRITTER_FLAG_NONODES) && CopyFrom->Graph[Ct] && CopyTo->NewGraph(Ct))
		CopyTo->Graph[Ct]->Copy(CopyTo->Graph[Ct], CopyFrom->Graph[Ct], this);
	} // if

CopyTo->MaxMin[0] = CopyFrom->MaxMin[0];
CopyTo->MaxMin[1] = CopyFrom->MaxMin[1];
CopyTo->Increment = CopyFrom->Increment;
CopyTo->NotifyItem = CopyFrom->NotifyItem;
CopyTo->Multiplier = CopyFrom->Multiplier;
CopyTo->MetricType = CopyFrom->MetricType;
CopyTo->FlagBits = CopyFrom->FlagBits;

} // AnimCritter::Copy

/*===========================================================================*/

void AnimCritter::CopyRangeDefaults(AnimCritter *CopyFrom)
{

MaxMin[0] = CopyFrom->MaxMin[0];
MaxMin[1] = CopyFrom->MaxMin[1];
Increment = CopyFrom->Increment;
NotifyItem = CopyFrom->NotifyItem;
Multiplier = CopyFrom->Multiplier;
MetricType = CopyFrom->MetricType;
FlagBits = CopyFrom->FlagBits;

} // AnimCritter::CopyRangeDefaults

/*===========================================================================*/

void AnimCritter::ReleaseGraph(char GraphNum)
{

if (Graph[GraphNum])
	delete Graph[GraphNum];
Graph[GraphNum] = NULL;

} // AnimCritter::ReleaseGraph

/*===========================================================================*/

GraphData *AnimCritter::NewGraph(char GraphNum)
{

ReleaseGraph(GraphNum);

return (Graph[GraphNum] = new GraphData);

} // AnimCritter::NewGraph

/*===========================================================================*/

GraphNode *AnimCritter::GetFirstSelectedNode(char GraphNum)
{
GraphNode *Found;

if (Graph[GraphNum] && (Found = Graph[GraphNum]->GetFirstSelectedNode()))
	return (Found);

if (Found = GetFirstNode(GraphNum))
	{
	Found->SetFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
	return (Found);
	} // if

return (NULL);

} // AnimCritter::GetFirstSelectedNode

/*===========================================================================*/

GraphNode *AnimCritter::GetFirstSelectedNodeNoSet(char GraphNum)
{
GraphNode *Found;

if (Graph[GraphNum] && (Found = Graph[GraphNum]->GetFirstSelectedNode()))
	return (Found);

return (NULL);

} // AnimCritter::GetFirstSelectedNodeNoSet

/*===========================================================================*/

GraphNode *AnimCritter::GetNextSelectedNode(GraphNode *CurNode)
{

while (CurNode)
	{
	if (CurNode = CurNode->Next)
		{
		if (CurNode->GetSelectedState())
			return (CurNode);
		} // if
	} // while

return (NULL);

} // AnimCritter::GetNextSelectedNode

/*===========================================================================*/

const char *AnimCritter::GetMetricSpecifier(void)
{

switch (MetricType)
	{
	case WCS_ANIMDOUBLE_METRIC_HEIGHT:
	case WCS_ANIMDOUBLE_METRIC_DISTANCE:
		{
		return ("m");
		} // 
	case WCS_ANIMDOUBLE_METRIC_ANGLE:
	case WCS_ANIMDOUBLE_METRIC_LATITUDE:
	case WCS_ANIMDOUBLE_METRIC_LONGITUDE:
		{
		return ("deg");
		} // 
	case WCS_ANIMDOUBLE_METRIC_TIME:
		{
		return ("sec");
		} // 
	case WCS_ANIMDOUBLE_METRIC_VELOCITY:
		{
		return ("m/sec");
		} // 
	} // switch

if (Multiplier == 100.0)
	return ("%");

return ("");

} // AnimCritter::GetMetricSpecifier

/*===========================================================================*/

void AnimCritter::AboutToChange(void)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ANIM_ABOUTTOCHANGE);
Changes[1] = NULL;
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY)) GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // AnimCritter::AboutToChange

/*===========================================================================*/

void AnimCritter::ValueChanged(void)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Changes[1] = NULL;
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY)) GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE)) RasterAnimHost::SetActiveRAHost(this);

} // AnimCritter::ValueChanged

/*===========================================================================*/

void AnimCritter::PositionChanged(void)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ANIM_POSITIONCHANGED);
Changes[1] = NULL;
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY)) GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE)) RasterAnimHost::SetActiveRAHost(this);

} // AnimCritter::PositionChanged

/*===========================================================================*/

void AnimCritter::SelectionChanged(void)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ANIM_SELECTIONCHANGED);
Changes[1] = NULL;
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY)) GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE)) RasterAnimHost::SetActiveRAHost(this);

} // AnimCritter::SelectionChanged

/*===========================================================================*/

void AnimCritter::ObjectChanged(void)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY)) GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // AnimCritter::ObjectChanged

/*===========================================================================*/

void AnimCritter::NodeAdded(void)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Changes[1] = NULL;
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY)) 
	{
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	GlobalApp->MCP->FrameScroll();	// can't think of a faster way to reset the frame scroll extents
	} // if
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE)) RasterAnimHost::SetActiveRAHost(this);

} // AnimCritter::NodeAdded

/*===========================================================================*/

void AnimCritter::NodeRemoved(void)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Changes[1] = NULL;
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY)) 
	{
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	GlobalApp->MCP->FrameScroll();	// can't think of a faster way to reset the frame scroll extents
	} // if
if(!TestFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE)) RasterAnimHost::SetActiveRAHost(this);

} // AnimCritter::NodeRemoved

/*===========================================================================*/

GraphNode *AnimCritter::RemoveNode(double Dist, double DistRange)
{
GraphNode *Found = NULL;

AboutToChange();
if (Graph[0])
	{
	if (Found = Graph[0]->RemoveNode(Dist, DistRange))	// returns next node, or previous node if there is no next
		{
		SetNodeModified(Found, 0);
		SetNodeModified(Found->Next, 0);
		SetNodeModified(Found->Prev, 0);
		} // if
	} // if
if (Graph[1])
	{
	if (Found = Graph[1]->RemoveNode(Dist, DistRange))	// returns next node, or previous node if there is no next
		{
		SetNodeModified(Found, 0);
		SetNodeModified(Found->Next, 0);
		SetNodeModified(Found->Prev, 0);
		} // if
	} // if
if (Graph[2])
	{
	if (Found = Graph[2]->RemoveNode(Dist, DistRange))	// returns next node, or previous node if there is no next
		{
		SetNodeModified(Found, 0);
		SetNodeModified(Found->Next, 0);
		SetNodeModified(Found->Prev, 0);
		} // if
	} // if
NodeRemoved();
return (Found);

} // AnimCritter::RemoveNode

/*===========================================================================*/

GraphNode *AnimCritter::RemoteAddNode(double Dist)
{
char Ct;
GraphNode *Found = NULL;
AnimCritter *CurSib;

if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES))
	{
	AboutToChange();
	for (Ct = 0; Ct < GetNumGraphs(); Ct ++)
		{
		if (! Graph[Ct])
			NewGraph(Ct);
		if (Graph[Ct])
			{
			if (Found = Graph[Ct]->AddNode(Dist, GetMinDistSeparation(), GetValue(Ct, Dist)))
				{
				SetNodeModified(Found, 0);
				SetNodeSelected(Found, 0);
				SetNodeModified(Found->Prev, 0);
				SetNodeModified(Found->Next, 0);
				} // if
			} // if
		else
			goto ErrorCleanup;
		} // for

	if (Found && NodeDataAvailable())
		Found->AddData(this);
	NodeAdded();

	if (GlobalApp->MainProj->GetKeyGroupMode() && ! SiblingOpInProgress && RAParent)
		{
		SiblingOpInProgress = 1;
		CurSib = this;
		while ((CurSib = (AnimCritter *)RAParent->GetNextGroupSibling(CurSib)) && CurSib != this)
			{
			CurSib->RemoteAddNode(Dist);
			} // while
		SiblingOpInProgress = 0;
		} // if

	if (! SiblingOpInProgress && TestFlags(WCS_ANIMCRITTER_FLAG_FLOATING))
		{
		GetRAHostRoot()->SetFloating(0);
		} // if
	} // if

return (Found);

ErrorCleanup:

ReleaseGraph(0);
ReleaseGraph(1);
ReleaseGraph(2);
return (NULL);

} // AnimCritter::RemoteAddNode

/*===========================================================================*/

GraphNode *AnimCritter::RemoteRemoveSelectedNode(void)
{
GraphNode *MyNode0, *MyNode1, *MyNode2, *Found = NULL, *TempNode;
char DontAdvance[3];
double RemoveDistance = 0.0;
AnimCritter *CurSib;

AboutToChange();
for (MyNode0 = GetFirstNode(0), MyNode1 = GetFirstNode(1), MyNode2 = GetFirstNode(2);
	MyNode0 || MyNode1 || MyNode2;
	MyNode0 = DontAdvance[0] ? MyNode0: GetNextNode(0, MyNode0), MyNode1 = DontAdvance[1] ? MyNode1: GetNextNode(1, MyNode1), MyNode2 = DontAdvance[2] ? MyNode2: GetNextNode(2, MyNode2))
	{
	DontAdvance[0] = DontAdvance[1] = DontAdvance[2] = 0;

	if ((MyNode0 && MyNode0->GetSelectedState()) || (MyNode1 && MyNode1->GetSelectedState()) || (MyNode2 && MyNode2->GetSelectedState()))
		{
		if (MyNode0)
			{
			TempNode = MyNode0->Next;
			RemoveDistance = MyNode0->GetDistance();
			if (Found = Graph[0]->RemoveNode(MyNode0->GetDistance(), 0.0))
				{
				SetNodeModified(Found, 0);
				SetNodeModified(Found->Prev, 0);
				SetNodeModified(Found->Next, 0);
				} // if
			MyNode0 = TempNode;
			DontAdvance[0] = 1;
			} // if
		if (MyNode1)
			{
			TempNode = MyNode1->Next;
			RemoveDistance = MyNode1->GetDistance();
			if (Found = Graph[1]->RemoveNode(MyNode1->GetDistance(), 0.0))
				{
				SetNodeModified(Found, 0);
				SetNodeModified(Found->Prev, 0);
				SetNodeModified(Found->Next, 0);
				} // if
			MyNode1 = TempNode;
			DontAdvance[1] = 1;
			} // if
		if (MyNode2)
			{
			TempNode = MyNode2->Next;
			RemoveDistance = MyNode2->GetDistance();
			if (Found = Graph[2]->RemoveNode(MyNode2->GetDistance(), 0.0))
				{
				SetNodeModified(Found, 0);
				SetNodeModified(Found->Prev, 0);
				SetNodeModified(Found->Next, 0);
				} // if
			MyNode2 = TempNode;
			DontAdvance[2] = 1;
			} // if
		if (GlobalApp->MainProj->GetKeyGroupMode() && ! SiblingOpInProgress && RAParent)
			{
			SiblingOpInProgress = 1;
			CurSib = this;
			while ((CurSib = (AnimCritter *)RAParent->GetNextGroupSibling(CurSib)) && CurSib != this)
				{
				CurSib->RemoveNode(RemoveDistance, 0.0);
				} // while
			SiblingOpInProgress = 0;
			} // if
		} // if
	} // for

NodeRemoved();

return (Found);

} // AnimCritter::RemoteRemoveSelectedNode

/*===========================================================================*/

GraphNode *AnimCritter::RemoteAlterSelectedNodeValue(double NewValue, double OldValue)
{
GraphNode *MyNode0, *MyNode1, *MyNode2;
double Difference;

Difference = NewValue - OldValue;

if (Difference != 0.0)
	{
	for (MyNode0 = GetFirstNode(0), MyNode1 = GetFirstNode(1), MyNode2 = GetFirstNode(2);
		MyNode0 || MyNode1 || MyNode2;
		MyNode0 = GetNextNode(0, MyNode0), MyNode1 = GetNextNode(1, MyNode1), MyNode2 = GetNextNode(2, MyNode2))
		{
		if (MyNode0 && MyNode0->GetSelectedState())
			{
			NewValue = MyNode0->GetValue() + Difference;
			NewValue = AnimClamp(NewValue);
			if (NewValue != MyNode0->GetValue())
				{
				MyNode0->SetValue(NewValue);
				SetNodeModified(MyNode0, 0);
				} // if
			} // if
		if (MyNode1 && MyNode1->GetSelectedState())
			{
			NewValue = MyNode1->GetValue() + Difference;
			NewValue = AnimClamp(NewValue);
			if (NewValue != MyNode1->GetValue())
				{
				MyNode1->SetValue(NewValue);
				SetNodeModified(MyNode1, 0);
				} // if
			} // if
		if (MyNode2 && MyNode2->GetSelectedState())
			{
			NewValue = MyNode2->GetValue() + Difference;
			NewValue = AnimClamp(NewValue);
			if (NewValue != MyNode2->GetValue())
				{
				MyNode2->SetValue(NewValue);
				SetNodeModified(MyNode2, 0);
				} // if
			} // if
		} // for
	
	SetToTime(GlobalApp->MainProj->Interactive->GetActiveTime());
	ValueChanged();
	return (MyNode0);
	} // if
return (NULL);

} // AnimCritter::RemoteAlterSelectedNodeValue

/*===========================================================================*/

GraphNode *AnimCritter::RemoteAlterSelectedNodePosition(double NewDist, double OldDist)
{
GraphNode *MyNode0, *MyNode1, *MyNode2, 
	*PrevNode0 = NULL, *PrevNode1 = NULL, *PrevNode2 = NULL, *NextNode0, *NextNode1, *NextNode2,
	*TempNode, *TempPrevNode;
double Difference, TempDistance = 0.0;
AnimCritter *CurSib;

Difference = NewDist - OldDist;

if (Difference != 0.0)
	{
	for (MyNode0 = GetFirstNode(0), MyNode1 = GetFirstNode(1), MyNode2 = GetFirstNode(2);
		MyNode0 || MyNode1 || MyNode2;
		MyNode0 = GetNextNode(0, MyNode0), MyNode1 = GetNextNode(1, MyNode1), MyNode2 = GetNextNode(2, MyNode2))
		{
		if ((MyNode0 && MyNode0->GetSelectedState()) || (MyNode1 && MyNode1->GetSelectedState()) || (MyNode2 && MyNode2->GetSelectedState()))
			{
			if (MyNode0)
				{
				NextNode0 = GetNextNode(0, MyNode0);
				NewDist = MyNode0->GetDistance() + Difference;
				if (NewDist < 0.0 || (PrevNode0 && NewDist < PrevNode0->GetDistance() + GetMinDistSeparation()) || (NextNode0 && NewDist > NextNode0->GetDistance() - GetMinDistSeparation()))
					{
					return (NULL);
					} // if
				} // if
			if (MyNode1)
				{
				NextNode1 = GetNextNode(1, MyNode1);
				NewDist = MyNode1->GetDistance() + Difference;
				if (NewDist < 0.0 || (PrevNode1 && NewDist < PrevNode1->GetDistance() + GetMinDistSeparation()) || (NextNode1 && NewDist > NextNode1->GetDistance() - GetMinDistSeparation()))
					{
					return (NULL);
					} // if
				} // if
			if (MyNode2)
				{
				NextNode2 = GetNextNode(2, MyNode2);
				NewDist = MyNode2->GetDistance() + Difference;
				if (NewDist < 0.0 || (PrevNode2 && NewDist < PrevNode2->GetDistance() + GetMinDistSeparation()) || (NextNode2 && NewDist > NextNode2->GetDistance() - GetMinDistSeparation()))
					{
					return (NULL);
					} // if
				} // if
			} // if
		PrevNode0 = MyNode0;
		PrevNode1 = MyNode1;
		PrevNode2 = MyNode2;
		} // for
	for (MyNode0 = GetFirstNode(0), MyNode1 = GetFirstNode(1), MyNode2 = GetFirstNode(2);
		MyNode0 || MyNode1 || MyNode2;
		MyNode0 = GetNextNode(0, MyNode0), MyNode1 = GetNextNode(1, MyNode1), MyNode2 = GetNextNode(2, MyNode2))
		{
		TempDistance = -1.0;
		if ((MyNode0 && MyNode0->GetSelectedState()) || (MyNode1 && MyNode1->GetSelectedState()) || (MyNode2 && MyNode2->GetSelectedState()))
			{
			if (MyNode0)
				{
				TempDistance = MyNode0->GetDistance();
				NewDist = MyNode0->GetDistance() + Difference;
				MyNode0->SetDistance(NewDist);
				SetNodeModified(MyNode0, 0);
				} // if
			if (MyNode1)
				{
				TempDistance = MyNode1->GetDistance();
				NewDist = MyNode1->GetDistance() + Difference;
				MyNode1->SetDistance(NewDist);
				SetNodeModified(MyNode1, 0);
				} // if
			if (MyNode2)
				{
				TempDistance = MyNode2->GetDistance();
				NewDist = MyNode2->GetDistance() + Difference;
				MyNode2->SetDistance(NewDist);
				SetNodeModified(MyNode2, 0);
				} // if
			} // if
		// key group mode ops
		if (TempDistance >= 0.0 && GlobalApp->MainProj->GetKeyGroupMode() && ! SiblingOpInProgress && RAParent)
			{
			SiblingOpInProgress = 1;
			CurSib = this;
			while ((CurSib = (AnimCritter *)RAParent->GetNextGroupSibling(CurSib)) && CurSib != this)
				{
				if (CurSib->Graph[0])
					{
					if (TempNode = CurSib->Graph[0]->FindNode(TempDistance, 0.0, TempPrevNode))
						TempNode->SetDistance(NewDist);
					} // if
				if (CurSib->Graph[1])
					{
					if (TempNode = CurSib->Graph[1]->FindNode(TempDistance, 0.0, TempPrevNode))
						TempNode->SetDistance(NewDist);
					} // if
				if (CurSib->Graph[2])
					{
					if (TempNode = CurSib->Graph[2]->FindNode(TempDistance, 0.0, TempPrevNode))
						TempNode->SetDistance(NewDist);
					} // if
				CurSib->SortNodesByDistance(0);
				CurSib->SortNodesByDistance(1);
				CurSib->SortNodesByDistance(2);
				CurSib->SetToTime(GlobalApp->MainProj->Interactive->GetActiveTime());
				CurSib->PositionChanged();
				} // while
			SiblingOpInProgress = 0;
			} // if
		} // for
	SortNodesByDistance(0);
	SortNodesByDistance(1);
	SortNodesByDistance(2);
	SetToTime(GlobalApp->MainProj->Interactive->GetActiveTime());
	PositionChanged();
	return (MyNode0);
	} // if
return (NULL);

} // AnimCritter::RemoteAlterSelectedNodePosition

/*===========================================================================*/

GraphNode *AnimCritter::RemoteAlterSelectedNodeLinear(char NewValue)
{
GraphNode *MyNode0, *MyNode1, *MyNode2;

for (MyNode0 = GetFirstNode(0), MyNode1 = GetFirstNode(1), MyNode2 = GetFirstNode(2);
	MyNode0 || MyNode1 || MyNode2;
	MyNode0 = GetNextNode(0, MyNode0), MyNode1 = GetNextNode(1, MyNode1), MyNode2 = GetNextNode(2, MyNode2))
	{
	if (MyNode0 && MyNode0->GetSelectedState())
		{
		if (NewValue != MyNode0->GetLinear())
			{
			MyNode0->SetLinear(NewValue);
			SetNodeModified(MyNode0, 0);
			} // if
		} // if
	if (MyNode1 && MyNode1->GetSelectedState())
		{
		if (NewValue != MyNode1->GetLinear())
			{
			MyNode1->SetLinear(NewValue);
			SetNodeModified(MyNode1, 0);
			} // if
		} // if
	if (MyNode2 && MyNode2->GetSelectedState())
		{
		if (NewValue != MyNode2->GetLinear())
			{
			MyNode2->SetLinear(NewValue);
			SetNodeModified(MyNode2, 0);
			} // if
		} // if
	} // for

ValueChanged();
return (MyNode0);

} // AnimCritter::RemoteAlterSelectedNodeLinear

/*===========================================================================*/

GraphNode *AnimCritter::RemoteAlterSelectedNodeTCB(char Channel, double NewValue)
{
GraphNode *MyNode0, *MyNode1, *MyNode2;

for (MyNode0 = GetFirstNode(0), MyNode1 = GetFirstNode(1), MyNode2 = GetFirstNode(2);
	MyNode0 || MyNode1 || MyNode2;
	MyNode0 = GetNextNode(0, MyNode0), MyNode1 = GetNextNode(1, MyNode1), MyNode2 = GetNextNode(2, MyNode2))
	{
	if (MyNode0 && MyNode0->GetSelectedState())
		{
		if (NewValue != MyNode0->GetTCB(Channel))
			{
			MyNode0->SetTCB(Channel, NewValue);
			SetNodeModified(MyNode0, 0);
			} // if
		} // if
	if (MyNode1 && MyNode1->GetSelectedState())
		{
		if (NewValue != MyNode1->GetTCB(Channel))
			{
			MyNode1->SetTCB(Channel, NewValue);
			SetNodeModified(MyNode1, 0);
			} // if
		} // if
	if (MyNode2 && MyNode2->GetSelectedState())
		{
		if (NewValue != MyNode2->GetTCB(Channel))
			{
			MyNode2->SetTCB(Channel, NewValue);
			SetNodeModified(MyNode2, 0);
			} // if
		} // if
	} // for

ValueChanged();
return (MyNode0);

} // AnimCritter::RemoteAlterSelectedNodeTCB

/*===========================================================================*/

GraphNode *AnimCritter::RemoteAlterSelectedNodePriority(short NewValue)
{
GraphNode *MyNode0, *MyNode1, *MyNode2;

if (NodeDataAvailable())
	{
	for (MyNode0 = GetFirstNode(0), MyNode1 = GetFirstNode(1), MyNode2 = GetFirstNode(2);
		MyNode0 || MyNode1 || MyNode2;
		MyNode0 = GetNextNode(0, MyNode0), MyNode1 = GetNextNode(1, MyNode1), MyNode2 = GetNextNode(2, MyNode2))
		{
		if (MyNode0 && MyNode0->GetSelectedState() && MyNode0->Data)
			{
			if (NewValue != MyNode0->Data->GetPriority())
				{
				MyNode0->Data->SetPriority(NewValue);
				SetNodeModified(MyNode0, 0);
				} // if
			} // if
		if (MyNode1 && MyNode1->GetSelectedState() && MyNode1->Data)
			{
			if (NewValue != MyNode1->Data->GetPriority())
				{
				MyNode1->Data->SetPriority(NewValue);
				SetNodeModified(MyNode1, 0);
				} // if
			} // if
		if (MyNode2 && MyNode2->GetSelectedState() && MyNode2->Data)
			{
			if (NewValue != MyNode2->Data->GetPriority())
				{
				MyNode2->Data->SetPriority(NewValue);
				SetNodeModified(MyNode2, 0);
				} // if
			} // if
		} // for

	ValueChanged();
	return (MyNode0);
	} // if

return (NULL);

} // AnimCritter::RemoteAlterSelectedNodePriority

/*===========================================================================*/

GraphNode *AnimCritter::RemoteAlterSelectedNodeRoughness(double NewValue)
{
GraphNode *MyNode0, *MyNode1, *MyNode2;

if (NodeDataAvailable())
	{
	for (MyNode0 = GetFirstNode(0), MyNode1 = GetFirstNode(1), MyNode2 = GetFirstNode(2);
		MyNode0 || MyNode1 || MyNode2;
		MyNode0 = GetNextNode(0, MyNode0), MyNode1 = GetNextNode(1, MyNode1), MyNode2 = GetNextNode(2, MyNode2))
		{
		if (MyNode0 && MyNode0->GetSelectedState() && MyNode0->Data)
			{
			if (NewValue != MyNode0->Data->GetRoughness())
				{
				MyNode0->Data->SetRoughness(NewValue);
				SetNodeModified(MyNode0, 0);
				} // if
			} // if
		if (MyNode1 && MyNode1->GetSelectedState() && MyNode1->Data)
			{
			if (NewValue != MyNode1->Data->GetRoughness())
				{
				MyNode1->Data->SetRoughness(NewValue);
				SetNodeModified(MyNode1, 0);
				} // if
			} // if
		if (MyNode2 && MyNode2->GetSelectedState() && MyNode2->Data)
			{
			if (NewValue != MyNode2->Data->GetRoughness())
				{
				MyNode2->Data->SetRoughness(NewValue);
				SetNodeModified(MyNode2, 0);
				} // if
			} // if
		} // for

	ValueChanged();
	return (MyNode0);
	} // if

return (NULL);

} // AnimCritter::RemoteAlterSelectedNodeRoughness

/*===========================================================================*/

GraphNode *AnimCritter::RemoteAlterSelectedNodeEcoMixing(double NewValue)
{
GraphNode *MyNode0, *MyNode1, *MyNode2;

if (NodeDataAvailable())
	{
	for (MyNode0 = GetFirstNode(0), MyNode1 = GetFirstNode(1), MyNode2 = GetFirstNode(2);
		MyNode0 || MyNode1 || MyNode2;
		MyNode0 = GetNextNode(0, MyNode0), MyNode1 = GetNextNode(1, MyNode1), MyNode2 = GetNextNode(2, MyNode2))
		{
		if (MyNode0 && MyNode0->GetSelectedState() && MyNode0->Data)
			{
			if (NewValue != MyNode0->Data->GetEcoMixing())
				{
				MyNode0->Data->SetEcoMixing(NewValue);
				SetNodeModified(MyNode0, 0);
				} // if
			} // if
		if (MyNode1 && MyNode1->GetSelectedState() && MyNode1->Data)
			{
			if (NewValue != MyNode1->Data->GetEcoMixing())
				{
				MyNode1->Data->SetEcoMixing(NewValue);
				SetNodeModified(MyNode1, 0);
				} // if
			} // if
		if (MyNode2 && MyNode2->GetSelectedState() && MyNode2->Data)
			{
			if (NewValue != MyNode2->Data->GetEcoMixing())
				{
				MyNode2->Data->SetEcoMixing(NewValue);
				SetNodeModified(MyNode2, 0);
				} // if
			} // if
		} // for

	ValueChanged();
	return (MyNode0);
	} // if

return (NULL);

} // AnimCritter::RemoteAlterSelectedNodeEcoMixing

/*===========================================================================*/

GraphNode *AnimCritter::RemoteAlterSelectedNodeEcosystem(EcosystemEffect *NewValue)
{
GraphNode *MyNode0, *MyNode1, *MyNode2;

if (NodeDataAvailable())
	{
	for (MyNode0 = GetFirstNode(0), MyNode1 = GetFirstNode(1), MyNode2 = GetFirstNode(2);
		MyNode0 || MyNode1 || MyNode2;
		MyNode0 = GetNextNode(0, MyNode0), MyNode1 = GetNextNode(1, MyNode1), MyNode2 = GetNextNode(2, MyNode2))
		{
		if (MyNode0 && MyNode0->GetSelectedState() && MyNode0->Data)
			{
			if (NewValue != MyNode0->Data->GetEcosystem())
				{
				MyNode0->Data->SetEco(NewValue);
				SetNodeModified(MyNode0, 0);
				} // if
			} // if
		if (MyNode1 && MyNode1->GetSelectedState() && MyNode1->Data)
			{
			if (NewValue != MyNode1->Data->GetEcosystem())
				{
				MyNode1->Data->SetEco(NewValue);
				SetNodeModified(MyNode1, 0);
				} // if
			} // if
		if (MyNode2 && MyNode2->GetSelectedState() && MyNode2->Data)
			{
			if (NewValue != MyNode2->Data->GetEcosystem())
				{
				MyNode2->Data->SetEco(NewValue);
				SetNodeModified(MyNode2, 0);
				} // if
			} // if
		} // for

	ValueChanged();
	return (MyNode0);
	} // if

return (NULL);

} // AnimCritter::RemoteAlterSelectedNodeEcosystem

/*===========================================================================*/

GraphNode *AnimCritter::SetNodeSelected(GraphNode *Select, char Notify)
{

if (Select)
	{
	AboutToChange();
	Select->SetFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
	if (Notify)
		SelectionChanged();
	} // if
return (Select);

} // AnimCritter::SetNodeSelected

/*===========================================================================*/

GraphNode *AnimCritter::ClearNodeSelected(GraphNode *Clear, char Notify)
{

if (Clear)
	{
	AboutToChange();
	Clear->ClearFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
	if (Notify)
		SelectionChanged();
	} // if
return (Clear);

} // AnimCritter::ClearNodeSelected

/*===========================================================================*/

GraphNode *AnimCritter::ToggleNodeSelected(GraphNode *Toggle, char Notify)
{

if (Toggle)
	{
	AboutToChange();
	Toggle->ToggleFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
	if (Notify)
		SelectionChanged();
	} // if
return (Toggle);

} // AnimCritter::ToggleNodeSelected

/*===========================================================================*/

GraphNode *AnimCritter::SetNodeModified(GraphNode *Select, char Notify)
{

if (Select)
	{
//	AboutToChange();
	Select->SetFlag(WCS_GRAPHNODE_FLAGS_MODIFIED);
	if (Notify)
		ValueChanged();
	} // if
return (Select);

} // AnimCritter::SetNodeModified

/*===========================================================================*/

GraphNode *AnimCritter::ClearNodeModified(GraphNode *Clear)
{

if (Clear)
	{
	Clear->ClearFlag(WCS_GRAPHNODE_FLAGS_MODIFIED);
	} // if
return (Clear);

} // AnimCritter::ClearNodeModified

/*===========================================================================*/

void AnimCritter::SetNodeSelectedAll(void)
{
char Ct;

//AboutToChange();
for (Ct = 0; Ct < GetNumGraphs(); Ct ++)
	{
	if (Graph[Ct])
		Graph[Ct]->SetNodeSelectedAll();
	} // for
SelectionChanged();

} // AnimCritter::SetNodeSelectedAll

/*===========================================================================*/

void AnimCritter::ClearNodeSelectedAll(void)
{
char Ct;

//AboutToChange();
for (Ct = 0; Ct < GetNumGraphs(); Ct ++)
	{
	if (Graph[Ct])
		Graph[Ct]->ClearNodeSelectedAll();
	} // for
SelectionChanged();

} // AnimCritter::ClearNodeSelectedAll

/*===========================================================================*/

void AnimCritter::ToggleNodeSelectedAll(void)
{
char Ct;

//AboutToChange();
for (Ct = 0; Ct < GetNumGraphs(); Ct ++)
	{
	if (Graph[Ct])
		Graph[Ct]->ToggleNodeSelectedAll();
	} // for
SelectionChanged();

} // AnimCritter::ToggleNodeSelectedAll

/*===========================================================================*/

void AnimCritter::SetNodeModifiedAll(void)
{
char Ct;

//AboutToChange();
for (Ct = 0; Ct < GetNumGraphs(); Ct ++)
	{
	if (Graph[Ct])
		Graph[Ct]->SetNodeModifiedAll();
	} // for
ValueChanged();

} // AnimCritter::SetNodeModifiedAll

/*===========================================================================*/

void AnimCritter::ClearNodeModifiedAll(void)
{
char Ct;

for (Ct = 0; Ct < GetNumGraphs(); Ct ++)
	{
	if (Graph[Ct])
		Graph[Ct]->ClearNodeModifiedAll();
	} // for

} // AnimCritter::ClearNodeModifiedAll

/*===========================================================================*/

void AnimCritter::SetNodeLinearAll(char NewValue)
{
int Ct;

for (Ct = 0; Ct < GetNumGraphs(); Ct ++)
	{
	if (Graph[Ct])
		Graph[Ct]->SetNodeLinearAll(NewValue);
	} // for

} // AnimCritter::SetNodeLinearAll

/*===========================================================================*/

GraphNode *AnimCritter::FindNode(double Dist, double DistRange, double Val, double ValRange)
{
char Ct;
GraphNode *Found;

for (Ct = 0; Ct < GetNumGraphs(); Ct ++)
	{
	if (Graph[Ct] && (Found = Graph[Ct]->FindNode(Dist, DistRange, Val, ValRange)))
		return (Found);
	} // for

return (NULL);

} // AnimCritter::FindNode

/*===========================================================================*/

long AnimCritter::GetNumSelectedNodes(void)
{
char Ct;
long Selected = 0;

for (Ct = 0; Ct < GetNumGraphs(); Ct ++)
	{
	if (Graph[Ct])
		Selected += Graph[Ct]->CountSelectedNodes();
	} // for

return (Selected);

} // AnimCritter::GetNumSelectedNodes

/*===========================================================================*/

GraphNode *AnimCritter::FindNearestSiblingNode(double Dist, GraphNode *CompareNode)
{
GraphNode *TempNode;
AnimCritter *CurSib;

if (Graph[0] && (TempNode = Graph[0]->FindNearestNode(Dist)))
	{
	if (CompareNode)
		{
		if (fabs(Dist - TempNode->GetDistance()) < fabs(Dist - CompareNode->GetDistance()))
			CompareNode = TempNode;
		} // if
	else
		CompareNode = TempNode;
	} // if
if (Graph[1] && (TempNode = Graph[1]->FindNearestNode(Dist)))
	{
	if (CompareNode)
		{
		if (fabs(Dist - TempNode->GetDistance()) < fabs(Dist - CompareNode->GetDistance()))
			CompareNode = TempNode;
		} // if
	else
		CompareNode = TempNode;
	} // if
if (Graph[2] && (TempNode = Graph[2]->FindNearestNode(Dist)))
	{
	if (CompareNode)
		{
		if (fabs(Dist - TempNode->GetDistance()) < fabs(Dist - CompareNode->GetDistance()))
			CompareNode = TempNode;
		} // if
	else
		CompareNode = TempNode;
	} // if

if (! SiblingOpInProgress && RAParent)
	{
	SiblingOpInProgress = 1;
	CurSib = this;
	while ((CurSib = (AnimCritter *)RAParent->GetNextGroupSibling(CurSib)) && CurSib != this)
		{
		if (TempNode = CurSib->FindNearestSiblingNode(Dist, CompareNode))
			CompareNode = TempNode;
		} // while
	SiblingOpInProgress = 0;
	} // if

return (CompareNode);

} // AnimCritter::FindNearestSiblingNode

/*===========================================================================*/

void AnimCritter::ScaleValues(double ScaleFactor)
{
char Ct;
GraphNode *Node;

for (Ct = 0; Ct < WCS_ANIM_MAX_NUMGRAPHS; Ct ++)
	{
	if (Node = GetFirstNode(Ct))
		{
		while (Node)
			{
			Node->Value *= ScaleFactor;
			Node->Value = AnimClamp(Node->Value);
			Node = GetNextNode(Ct, Node);
			} // while
		} // if
	} // for

} // AnimCritter::ScaleValues

/*===========================================================================*/

void AnimCritter::EnforceRange(void)
{
char Ct;
GraphNode *Node;

for (Ct = 0; Ct < WCS_ANIM_MAX_NUMGRAPHS; Ct ++)
	{
	if (Node = GetFirstNode(Ct))
		{
		while (Node)
			{
			Node->Value = AnimClamp(Node->Value);
			Node = GetNextNode(Ct, Node);
			} // while
		} // if
	} // for

} // AnimCritter::ScaleValues

/*===========================================================================*/

GraphNode *AnimCritter::ValidateNode(GraphNode *CheckNode)
{
long CurGraph;
GraphNode *CurNode;

for (CurGraph = 0; CurGraph < 3; CurGraph ++)
	{
	if (Graph[CurGraph])
		{
		for (CurNode = Graph[CurGraph]->GetFirstNode(); CurNode; CurNode = Graph[CurGraph]->GetNextNode(CurNode))
			{
			if (CurNode == CheckNode)
				return (CheckNode);
			} // for
		} // if
	} // for

return (Graph[0] ? Graph[0]->GetFirstSelectedNode(): NULL);

} // AnimCritter::ValidateNode

/*===========================================================================*/

int AnimCritter::ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop)
{
int Success = 0;
RasterAnimHost *CurSib;

if (Graph[0] && Graph[0]->ScaleDeleteAnimNodes(Prop))
	Success = 1;
if (Graph[1] && Graph[1]->ScaleDeleteAnimNodes(Prop))
	Success = 1;
if (Graph[2] && Graph[2]->ScaleDeleteAnimNodes(Prop))
	Success = 1;
if (Prop->ItemOperator == WCS_KEYOPERATION_CUROBJGROUP && RAParent)
	{
	Prop->ItemOperator = WCS_KEYOPERATION_CUROBJ;
	CurSib = this;
	while ((CurSib = RAParent->GetNextGroupSibling(CurSib)) && CurSib != this)
		{
		if (CurSib->ScaleDeleteAnimNodes(Prop))
			Success = 1;
		} // while
	Prop->ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;
	} // if

return (Success);

} // AnimCritter::ScaleDeleteAnimNodes

/*===========================================================================*/

int AnimCritter::GetNextAnimNode(RasterAnimHostProperties *Prop)
{
int Success = 0;
RasterAnimHost *CurSib;

if (Graph[0] && Graph[0]->GetNextAnimNode(Prop))
	Success = 1;
if (Graph[1] && Graph[1]->GetNextAnimNode(Prop))
	Success = 1;
if (Graph[2] && Graph[2]->GetNextAnimNode(Prop))
	Success = 1;
if (Prop->ItemOperator == WCS_KEYOPERATION_CUROBJGROUP && RAParent)
	{
	Prop->ItemOperator = WCS_KEYOPERATION_CUROBJ;
	CurSib = this;
	while ((CurSib = RAParent->GetNextGroupSibling(CurSib)) && CurSib != this)
		{
		if (CurSib->GetNextAnimNode(Prop))
			Success = 1;
		} // while
	Prop->ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;
	} // if

return (Success);

} // AnimCritter::GetNextAnimNode

/*===========================================================================*/

int AnimCritter::GetRAHostAnimated(void)
{
char Ct;

for (Ct = 0; Ct < GetNumGraphs(); Ct ++)
	{
	if (Graph[Ct] && Graph[Ct]->GetNumNodes() > 0)
		return (1);
	} // for

return (0);

} // AnimCritter::GetRAHostAnimated

/*===========================================================================*/

void AnimCritter::OpenTimeline(void)
{
char WinNum;

#ifdef WCS_BUILD_DEMO
DONGLE_INLINE_CHECK()
#else // !WCS_BUILD_DEMO


if(!GlobalApp->Sentinal->CheckDongle()) return;


#endif // !WCS_BUILD_DEMO

for (WinNum = 0; WinNum < 10; WinNum ++)
	{
	if (GlobalApp->GUIWins->GRG[WinNum] && GlobalApp->GUIWins->GRG[WinNum]->GetActive() == this)
		{
		GlobalApp->GUIWins->GRG[WinNum]->Open(GlobalApp->MainProj);
		RasterAnimHost::SetActiveRAHost(this);
		return;
		} // if
	} // for

if ((WinNum = GlobalApp->MCP->GetAvailableGraphNumber()) >= 0)
	{
	GlobalApp->GUIWins->GRG[WinNum] = new AnimGraphGUI(this, WinNum, GlobalApp->AppEx);
	if(GlobalApp->GUIWins->GRG[WinNum])
		{
		GlobalApp->GUIWins->GRG[WinNum]->Open(GlobalApp->MainProj);
		}
	} // if
else
	UserMessageOK("Open TimeLine", "All Graphs are in use.\nClose one of the open graphs and try again.");

} // AnimCritter::OpenTimeline

/*===========================================================================*/

ULONG AnimCritter::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
signed char GraphNum = -1;
char Ct;

for (Ct = 0; Ct < WCS_ANIM_MAX_NUMGRAPHS; Ct ++)
	{
	ReleaseGraph(Ct);
	} // if

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_ANIMCRITTER_GRAPHDATA:
						{
						if (GraphNum >= 0 && GraphNum < WCS_ANIM_MAX_NUMGRAPHS && (Graph[GraphNum] = new GraphData))
							{
							BytesRead = Graph[GraphNum]->Load(ffile, Size, ByteFlip, this);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ANIMCRITTER_GRAPHNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&GraphNum, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // AnimCritter::Load

/*===========================================================================*/

unsigned long AnimCritter::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
char GraphNum;

for (GraphNum = 0; GraphNum < WCS_ANIM_MAX_NUMGRAPHS; GraphNum ++)
	{
	if (Graph[GraphNum] && Graph[GraphNum]->GetNumNodes() > 0)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_ANIMCRITTER_GRAPHNUM, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (char *)&GraphNum)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		ItemTag = WCS_ANIMCRITTER_GRAPHDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Graph[GraphNum]->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if graphdata saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // for

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // AnimCritter::Save

/*===========================================================================*/

AnimDouble::AnimDouble()
{

CurValue = 0.0;

} // AnimDouble::AnimDouble

/*===========================================================================*/

void AnimDouble::SetDefaults(RasterAnimHost *RAHost, char Item, double Default)
{

AnimCritter::SetDefaults(RAHost, Item);
CurValue = Default;

} // AnimDouble::SetDefaults

/*===========================================================================*/

void AnimDouble::Copy(AnimCritter *CopyTo, AnimCritter *CopyFrom)
{

AnimCritter::Copy(CopyTo, CopyFrom);
((AnimDouble *)CopyTo)->CurValue = ((AnimDouble *)CopyFrom)->CurValue;

} // AnimDouble::Copy

/*===========================================================================*/

GraphNode *AnimDouble::AddNode(double Dist, double DistRange)
{
GraphNode *Found = NULL;
AnimDouble *CurSib;

if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES))
	{
	if (DistRange == 0.0)
		DistRange = GetMinDistSeparation();
	if (! Graph[0])
		NewGraph(0);
	if (Graph[0])
		{
		AboutToChange();
		if (Found = Graph[0]->AddNode(Dist, DistRange, AnimClamp(CurValue)))	// returns new node or existing node if one at same range
			{
			SetNodeModified(Found, 0);
			SetNodeModified(Found->Next, 0);
			SetNodeModified(Found->Prev, 0);
			NodeAdded();
			} // if
		} // if

	if (GlobalApp->MainProj->GetKeyGroupMode() && ! SiblingOpInProgress && RAParent)
		{
		SiblingOpInProgress = 1;
		CurSib = this;
		while ((CurSib = (AnimDouble *)RAParent->GetNextGroupSibling(CurSib)) && CurSib != this)
			{
			CurSib->AddNode(Dist, DistRange);
			} // while
		SiblingOpInProgress = 0;
		} // if

	if (! SiblingOpInProgress && TestFlags(WCS_ANIMCRITTER_FLAG_FLOATING))
		{
		GetRAHostRoot()->SetFloating(0);
		} // if
	} // if

return (Found);

} // AnimDouble::AddNode

/*===========================================================================*/

GraphNode *AnimDouble::AddNode(double Dist, double NewValue, double DistRange)
{
GraphNode *Found = NULL;

if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES))
	{
	if (DistRange == 0.0)
		DistRange = GetMinDistSeparation();
	if (! Graph[0])
		NewGraph(0);
	if (Graph[0])
		{
		AboutToChange();
		if (Found = Graph[0]->AddNode(Dist, DistRange, AnimClamp(NewValue)))	// returns new node or existing node if one at same range
			{
			SetNodeModified(Found, 0);
			SetNodeModified(Found->Next, 0);
			SetNodeModified(Found->Prev, 0);
			NodeAdded();
			} // if
		} // if
	} // if

return (Found);

} // AnimDouble::AddNode

/*===========================================================================*/

GraphNode *AnimDouble::AddNodeEnd(GraphNode *PrevNode, double Dist, double NewValue, double DistRange)
{
GraphNode *Found = NULL;

if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES))
	{
	if (! Graph[0])
		NewGraph(0);
	if (Graph[0])
		{
		if (Found = Graph[0]->AddNodeEnd(PrevNode, Dist, DistRange, AnimClamp(NewValue)))	// returns new node or existing node if one at same range
			{
			SetNodeModified(Found, 0);
			SetNodeModified(Found->Next, 0);
			SetNodeModified(Found->Prev, 0);
			} // if
		} // if
	} // if

return (Found);

} // AnimDouble::AddNodeEnd

/*===========================================================================*/
/*
GraphNode *AnimDouble::RemoveNode(double Dist, double DistRange)
{
GraphNode *Found = NULL;

if (! Graph[0])
	return (NULL);
AboutToChange();
if (Found = Graph[0]->RemoveNode(Dist, DistRange))	// returns next node, or previous node if there is no next
	{
	SetNodeModified(Found, 0);
	SetNodeModified(Found->Next, 0);
	SetNodeModified(Found->Prev, 0);
	} // if
NodeRemoved();
return (Found);

} // AnimDouble::RemoveNode
*/
/*===========================================================================*/

GraphNode *AnimDouble::UpdateNode(double Dist, double DistRange)
{
GraphNode *Found = NULL;

if (! Graph[0])
	return (NULL);
if (Found = Graph[0]->UpdateNode(Dist, DistRange, CurValue))
	{
	SetNodeModified(Found, 1);
	}
	
return (Found);	// returns node if one was found at the distance

} // AnimDouble::UpdateNode

/*===========================================================================*/

int AnimDouble::GetMinMaxDist(double &FindMin, double &FindMax)
{

if (Graph[0] && Graph[0]->GetNumNodes())
	{
	return (Graph[0]->GetMinMaxDist(FindMin, FindMax));
	} // if
else
	{
	FindMax = FindMin = 0.0;
	return (0);
	} // else

} // AnimDouble::GetMinMaxDist

/*===========================================================================*/
/*
int AnimDouble::DeleteDistRange(double FirstDist, double LastDist)
{

if (Graph[0] && Graph[0]->GetNumNodes())
	{
	return (Graph[0]->DeleteDistRange(FirstDist, LastDist));
	} // if

return (0);

} // AnimDouble::DeleteDistRange
*/
/*===========================================================================*/

int AnimDouble::GetNextDist(double &NewDist, short Direction, double CurrentDist)
{

if (Graph[0] && Graph[0]->GetNumNodes())
	{
	return (Graph[0]->GetNextDist(NewDist, Direction, CurrentDist));
	} // if
else
	{
	NewDist = 0.0;
	return (0);
	} // else

} // AnimDouble::GetNextDist

/*===========================================================================*/

int AnimDouble::GetMinMaxVal(double &FindMin, double &FindMax)
{

FindMax = -FLT_MAX;
FindMin = FLT_MAX;
if (Graph[0] && Graph[0]->GetMinMaxVal(FindMin, FindMax))
	{
	return (1);
	} // if
else
	{
	FindMax = FindMin = CurValue;
	return (0);
	} // else

} // AnimDouble::GetMinMaxVal

/*===========================================================================*/

double AnimDouble::GetValue(char Channel, double Dist)
{
double Value;

Value = (Graph[0] && Graph[0]->GetNumNodes()) ? Graph[0]->GetValue(Dist): CurValue;

return (AnimClamp(Value));

} // AnimDouble::GetValue

/*===========================================================================*/

void AnimDouble::SetCurValue(double NewValue)
{
double Time, TimeRange;

CurValue = AnimClamp(NewValue);

if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES) && GlobalApp->MainProj->Prefs.RecordMode)  // RecordMode is currently unused and may be retired eventually
	{
	Time = GlobalApp->MainProj->Interactive->GetActiveTime();

	TimeRange = .05 / GlobalApp->MainProj->Interactive->GetFrameRate();

	AnimDouble::AddNode(Time, TimeRange);
	} // if

if (TestFlags(WCS_ANIMCRITTER_FLAG_FLOATING))
	{
	GetRAHostRoot()->SetFloating(0);
	} // if

ValueChanged();

} // AnimDouble::SetCurValue

/*===========================================================================*/

void AnimDouble::ScaleValues(double ScaleFactor)
{

CurValue *= ScaleFactor;
CurValue = AnimClamp(CurValue);

AnimCritter::ScaleValues(ScaleFactor);
ValueChanged();

} // AnimDouble::ScaleValues

/*===========================================================================*/

void AnimDouble::EnforceRange(void)
{

CurValue = AnimClamp(CurValue);

AnimCritter::EnforceRange();
ValueChanged();

} // AnimDouble::ScaleValues

/*===========================================================================*/

void AnimDouble::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = RAParent ? RAParent->GetCritterName(this): (char*)"";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = "";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetMinMaxDist(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_INTERFLAGS)
	{
	GetInterFlags(Prop, this);
	} // if

} // AnimDouble::GetRAHostProperties

/*===========================================================================*/

int AnimDouble::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		if (Prop->ItemOperator == WCS_KEYOPERATION_CUROBJGROUP && RAParent)
			{
			Changes[0] = MAKE_ID(RAParent->GetNotifyClass(), RAParent->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			} // if
		else
			{
			if (Prop->KeyframeOperation == WCS_KEYOPERATION_DELETE)
				Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
			else
				Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ANIM_POSITIONCHANGED);
			} // else
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

return (Success);

} // AnimDouble::SetRAHostProperties

/*===========================================================================*/

ULONG AnimDouble::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

 while (ItemTag != WCS_PARAM_DONE)
  {
  /* read block descriptor tag from file */
  if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
   		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
   {
   TotalRead += BytesRead;
   if (ItemTag != WCS_PARAM_DONE)
    {
	/* read block size from file */
    if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
     {
     TotalRead += BytesRead;
     BytesRead = 0;
     switch (ItemTag & 0xff)
      {
      case WCS_BLOCKSIZE_CHAR:
       {
       Size = MV.Char[0];
       break;
       }
      case WCS_BLOCKSIZE_SHORT:
       {
       Size = MV.Short[0];
       break;
       }
      case WCS_BLOCKSIZE_LONG:
       {
       Size = MV.Long;
       break;
	   }
      } /* switch */

     switch (ItemTag & 0xffff0000)
      {
      case WCS_ANIM_ANIMCRITTER:
       {
       BytesRead = AnimCritter::Load(ffile, Size, ByteFlip);
       break;
	   }
      case WCS_ANIMDOUBLE_VALUE:
       {
       BytesRead = ReadBlock(ffile, (char *)&CurValue, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      default:
       {
	   if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   } 
	  } /* switch */

     TotalRead += BytesRead;
     if (BytesRead != Size)
      break;
     } /* if size block read */
    else
     break;
    } /* if not done flag */
   } /* if tag block read */
  else
   break;
  } /* while */

 return (TotalRead);

} // AnimDouble::Load

/*===========================================================================*/

unsigned long AnimDouble::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_ANIMDOUBLE_VALUE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&CurValue)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_ANIM_ANIMCRITTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = AnimCritter::Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if animcritter saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // AnimDouble::Save

/*===========================================================================*/

AnimColor::AnimColor()
{
double RangeDefaults[3] = {1.0, 0.0, 1.0 / 255.0};

SetRangeDefaults(RangeDefaults);
CurValue[0] = CurValue[1] = CurValue[2] = .5;
SetMultiplier(255.0);

} // AnimColor::AnimColor

/*===========================================================================*/

void AnimColor::SetDefaults(RasterAnimHost *RAHost, char Item, double Default)
{
double RangeDefaults[3] = {1.0, 0.0, 1.0 / 255.0};

AnimCritter::SetDefaults(RAHost, Item);
SetRangeDefaults(RangeDefaults);
CurValue[0] = CurValue[1] = CurValue[2] = Default;
SetMultiplier(255.0);

} // AnimColor::SetDefaults

/*===========================================================================*/

void AnimColor::Copy(AnimCritter *CopyTo, AnimCritter *CopyFrom)
{

AnimCritter::Copy(CopyTo, CopyFrom);
((AnimColor *)CopyTo)->CurValue[0] = ((AnimColor *)CopyFrom)->CurValue[0];
((AnimColor *)CopyTo)->CurValue[1] = ((AnimColor *)CopyFrom)->CurValue[1];
((AnimColor *)CopyTo)->CurValue[2] = ((AnimColor *)CopyFrom)->CurValue[2];

} // AnimColor::Copy

/*===========================================================================*/

GraphNode *AnimColor::AddNode(double Dist, double DistRange)
{
GraphNode *Found;

if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES))
	{
	if (DistRange == 0.0)
		DistRange = GetMinDistSeparation();
	if (! Graph[0])
		NewGraph(0);
	if (! Graph[1])
		NewGraph(1);
	if (! Graph[2])
		NewGraph(2);
	if (Graph[0] && Graph[1] && Graph[2])
		{
		AboutToChange();
		if (Found = Graph[0]->AddNode(Dist, DistRange, AnimClamp(CurValue[0])))
			{
			SetNodeModified(Found, 0);
			SetNodeModified(Found->Prev, 0);
			SetNodeModified(Found->Next, 0);
			} // if
		if (Found = Graph[1]->AddNode(Dist, DistRange, AnimClamp(CurValue[1])))
			{
			SetNodeModified(Found, 0);
			SetNodeModified(Found->Prev, 0);
			SetNodeModified(Found->Next, 0);
			} // if
		if (Found = Graph[2]->AddNode(Dist, DistRange, AnimClamp(CurValue[2])))
			{
			SetNodeModified(Found, 0);
			SetNodeModified(Found->Prev, 0);
			SetNodeModified(Found->Next, 0);
			} // if
		NodeAdded();
		return (Found);
		} // if
	ReleaseGraph(0);
	ReleaseGraph(1);
	ReleaseGraph(2);
	} // if

return (NULL);

} // AnimColor::AddNode

/*===========================================================================*/

GraphNode *AnimColor::AddNode(double Dist, double NewValue[3], double DistRange)
{
GraphNode *Found;

if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES))
	{
	if (DistRange == 0.0)
		DistRange = GetMinDistSeparation();
	if (! Graph[0])
		NewGraph(0);
	if (! Graph[1])
		NewGraph(1);
	if (! Graph[2])
		NewGraph(2);
	if (Graph[0] && Graph[1] && Graph[2])
		{
		AboutToChange();
		if (Found = Graph[0]->AddNode(Dist, DistRange, AnimClamp(NewValue[0])))
			{
			SetNodeModified(Found, 0);
			SetNodeModified(Found->Prev, 0);
			SetNodeModified(Found->Next, 0);
			} // if
		if (Found = Graph[1]->AddNode(Dist, DistRange, AnimClamp(NewValue[1])))
			{
			SetNodeModified(Found, 0);
			SetNodeModified(Found->Prev, 0);
			SetNodeModified(Found->Next, 0);
			} // if
		if (Found = Graph[2]->AddNode(Dist, DistRange, AnimClamp(NewValue[2])))
			{
			SetNodeModified(Found, 0);
			SetNodeModified(Found->Prev, 0);
			SetNodeModified(Found->Next, 0);
			} // if
		NodeAdded();
		return (Found);
		} // if
	ReleaseGraph(0);
	ReleaseGraph(1);
	ReleaseGraph(2);
	} // if

return (NULL);

} // AnimColor::AddNode

/*===========================================================================*/
/*
GraphNode *AnimColor::RemoveNode(double Dist, double DistRange)
{
GraphNode *Found = NULL;

AboutToChange();
if (Graph[0])
	{
	if (Found = Graph[0]->RemoveNode(Dist, DistRange))
		{
		SetNodeModified(Found, 0);
		SetNodeModified(Found->Prev, 0);
		SetNodeModified(Found->Next, 0);
		} // if
	} // if
if (Graph[1])
	{
	if (Found = Graph[1]->RemoveNode(Dist, DistRange))
		{
		SetNodeModified(Found, 0);
		SetNodeModified(Found->Prev, 0);
		SetNodeModified(Found->Next, 0);
		} // if
	} // if
if (Graph[2])
	{
	if (Found = Graph[2]->RemoveNode(Dist, DistRange))
		{
		SetNodeModified(Found, 0);
		SetNodeModified(Found->Prev, 0);
		SetNodeModified(Found->Next, 0);
		} // if
	} // if
NodeRemoved();
return (Found);

} // AnimColor::RemoveNode
*/
/*===========================================================================*/

GraphNode *AnimColor::UpdateNode(double Dist, double DistRange)
{
GraphNode *Found = NULL;

if (! (Graph[0] && Graph[1] && Graph[2]))
	return (NULL);
AboutToChange();
if (Found = Graph[0]->UpdateNode(Dist, DistRange, CurValue[0]))
	{
	SetNodeModified(Found, 0);
	}
if (Found = Graph[1]->UpdateNode(Dist, DistRange, CurValue[1]))
	{
	SetNodeModified(Found, 0);
	}
if (Found = Graph[2]->UpdateNode(Dist, DistRange, CurValue[2]))
	{
	return (SetNodeModified(Found, 0));
	}
ValueChanged();
return (Found);	// returns node if one was found at the distance

} // AnimColor::UpdateNode

/*===========================================================================*/

int AnimColor::GetMinMaxDist(double &FindMin, double &FindMax)
{
char Ct, Found = 0;
double TestMax, TestMin;

FindMax = 0.0;
FindMin = FLT_MAX;
for (Ct = 0; Ct < 3; Ct ++)
	{
	if (Graph[Ct] && Graph[Ct]->GetNumNodes())
		{
		if (Graph[Ct]->GetMinMaxDist(TestMin, TestMax))
			{
			if (TestMax > FindMax)
				FindMax = TestMax;
			if (TestMin < FindMin)
				FindMin = TestMin;
			} // if
		Found = 1;
		} // if
	} // for
if (Found)
	{
	return (1);
	} // if
else
	{
	FindMax = FindMin = 0.0;
	return (0);
	} // else

} // AnimColor::GetMinMaxDist

/*===========================================================================*/
/*
int AnimColor::GetNextDist(double &NewDist, short Direction, double CurrentDist)
{
char Ct, Found = 0;
double TestDist;

if (Direction > 0)
	NewDist = FLT_MAX;
else
	NewDist = 0.0;
for (Ct = 0; Ct < 3; Ct ++)
	{
	if (Graph[Ct] && Graph[Ct]->GetNumNodes())
		{
		if (Graph[Ct]->GetNextDist(TestDist, Direction, CurrentDist))
			{
			if ((Direction > 0 && TestDist < NewDist) || (Direction < 0 && TestDist > NewDist))
				NewDist = TestDist;
			Found = 1;
			} // if
		} // if
	} // for
if (Found)
	{
	return (1);
	} // if
else
	{
	NewDist = 0.0;
	return (0);
	} // else

} // AnimColor::GetNextDist
*/
/*===========================================================================*/
/*
int AnimColor::DeleteDistRange(double FirstDist, double LastDist)
{
char Ct, Found = 0;

for (Ct = 0; Ct < 3; Ct ++)
	{
	if (Graph[Ct] && Graph[Ct]->GetNumNodes())
		{
		if (Graph[Ct]->DeleteDistRange(FirstDist, LastDist))
			{
			Found = 1;
			} // if
		} // if
	} // for

return (Found);

} // AnimColor::DeleteDistRange
*/
/*===========================================================================*/

int AnimColor::GetMinMaxVal(double &FindMin, double &FindMax)
{
int Ct, Found = 0;
double TestMax, TestMin;

FindMax = -FLT_MAX;
FindMin = FLT_MAX;
for (Ct = 0; Ct < 3; Ct ++)
	{
	if (Graph[Ct] && Graph[Ct]->GetMinMaxVal(TestMin, TestMax))
		{
		if (TestMin < FindMin)
			FindMin = TestMin;
		if (TestMax > FindMax)
			FindMax = TestMax;
		Found = 1;
		} // if
	else
		{
		if (CurValue[Ct] < FindMin)
			FindMin = CurValue[Ct];
		if (CurValue[Ct] > FindMax)
			FindMax = CurValue[Ct];
		} // else
	} // for
return (Found);

} // AnimColor::GetMinMaxVal

/*===========================================================================*/

double AnimColor::GetValue(char Channel, double Dist)
{
double Value;

Value = (Graph[Channel] && Graph[Channel]->GetNumNodes()) ? Graph[Channel]->GetValue(Dist): CurValue[Channel];

return (AnimClamp(Value));

} // AnimColor::GetValue

/*===========================================================================*/

void AnimColor::SetCurValue(char Channel, double NewValue)
{
double Time, TimeRange;

CurValue[Channel] = AnimClamp(NewValue);

if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES) && GlobalApp->MainProj->Prefs.RecordMode)  // RecordMode is currently unused and may be retired eventually
	{
	Time = GlobalApp->MainProj->Interactive->GetActiveTime();

	TimeRange = .05 / GlobalApp->MainProj->Interactive->GetFrameRate();

	AnimColor::AddNode(Time, TimeRange);
	} // if

ValueChanged();

} // AnimColor::SetCurValue

/*===========================================================================*/

void AnimColor::ScaleValues(double ScaleFactor)
{

CurValue[0] *= ScaleFactor;
CurValue[1] *= ScaleFactor;
CurValue[2] *= ScaleFactor;
CurValue[0] = AnimClamp(CurValue[0]);
CurValue[1] = AnimClamp(CurValue[1]);
CurValue[2] = AnimClamp(CurValue[2]);

AnimCritter::ScaleValues(ScaleFactor);
ValueChanged();

} // AnimColor::ScaleValues

/*===========================================================================*/

void AnimColor::RGBtoHSV(double HSV[3])
{

::RGBtoHSV(HSV, CurValue);
/*
double mmax, mmin, mmid;
int sign;

mmax = MAX3(CurValue[0], CurValue[1], CurValue[2]);
mmin = MIN3(CurValue[0], CurValue[1], CurValue[2]);
mmid = MID3(CurValue[0], CurValue[1], CurValue[2]);
if (mmax >= 1.0)
	HSV[2] = 100.0;
else
	HSV[2] = (100 * mmax);
if (mmax <= 0.0)
	HSV[1] = 0.0;
else
	HSV[1] = ((100 * (mmax - mmin)) / mmax);
if (mmax == mmin)
	{
	HSV[0] = 0.0;
	return;
	} // if R=G=B 
if (mmax == CurValue[0])
	{
	if (mmid == CurValue[1])
		sign = +1;
	else
		sign = -1;
	HSV[0] = (0 + (sign * 60 * (mmid - mmin)) / (mmax - mmin));
	if (HSV[0] < 0)
		HSV[0] += 360;
	} // if red max 
else if (mmax == CurValue[1])
	{
	if (mmid == CurValue[2])
		sign = +1;
	else
		sign = -1;
	HSV[0] = (120.0 + (sign * 60 * (mmid - mmin)) / (mmax - mmin));
	} // else if grn max 
else
	{
	if (mmid == CurValue[0])
		sign = +1;
	else
		sign = -1;
	HSV[0] = (240.0 + (sign * 60 * (mmid - mmin)) / (mmax - mmin));
	} // else blu max 
*/
} // AnimColor::RGBtoHSV

/*===========================================================================*/

void AnimColor::HSVtoRGB(double HSV[3])
{

::HSVtoRGB(HSV, CurValue);
/*
double mmax, mmin, hueshift;

mmax = HSV[2] / 100;
mmin = mmax - (HSV[1] * mmax) / 100;
if (HSV[0] >= 60.0 && HSV[0] < 180.0)
	{
	CurValue[1] = mmax;
	hueshift = HSV[0] - 120.0;
	if (hueshift < 0)
		{
		CurValue[2] = mmin;
		CurValue[0] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60.0);
		} // if shift toward red 
	else
		{
		CurValue[0] = mmin;
		CurValue[2] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60.0);
		} // else shift toward blu 
	} // if dominant hue grn 
else if (HSV[0] >= 180.0 && HSV[0] < 300.0)
	{
	CurValue[2] = mmax;
	hueshift = HSV[0] - 240.0;
	if (hueshift < 0)
		{
		CurValue[0] = mmin;
		CurValue[1] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60);
		} // if shift toward grn 
	else
		{
		CurValue[1] = mmin;
		CurValue[0] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60);
		} // else shift toward red 
	} // else if dominant hue blu 
else
	{
	CurValue[0] = mmax;
	hueshift = HSV[0] < 120.0 ? HSV[0]: HSV[0] - 360.0;
	if (hueshift < 0)
		{
		CurValue[1] = mmin;
		CurValue[2] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60);
		} // if shift toward blu 
	else
		{
		CurValue[2] = mmin;
		CurValue[1] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60);
		} // else shift toward grn 
	} // else dominant hue red 
*/
CurValue[0] = AnimClamp(CurValue[0]);
CurValue[1] = AnimClamp(CurValue[1]);
CurValue[2] = AnimClamp(CurValue[2]);

} // AnimColor::HSVtoRGB

/*===========================================================================*/

ULONG AnimColor::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

 while (ItemTag != WCS_PARAM_DONE)
  {
  /* read block descriptor tag from file */
  if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
   		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
   {
   TotalRead += BytesRead;
   if (ItemTag != WCS_PARAM_DONE)
    {
	/* read block size from file */
    if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
     {
     TotalRead += BytesRead;
     BytesRead = 0;
     switch (ItemTag & 0xff)
      {
      case WCS_BLOCKSIZE_CHAR:
       {
       Size = MV.Char[0];
       break;
       }
      case WCS_BLOCKSIZE_SHORT:
       {
       Size = MV.Short[0];
       break;
       }
      case WCS_BLOCKSIZE_LONG:
       {
       Size = MV.Long;
       break;
	   }
      } /* switch */

     switch (ItemTag & 0xffff0000)
      {
      case WCS_ANIM_ANIMCRITTER:
       {
       BytesRead = AnimCritter::Load(ffile, Size, ByteFlip);
       break;
	   }
      case WCS_ANIMCOLOR_VALUE1:
       {
       BytesRead = ReadBlock(ffile, (char *)&CurValue[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_ANIMCOLOR_VALUE2:
       {
       BytesRead = ReadBlock(ffile, (char *)&CurValue[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_ANIMCOLOR_VALUE3:
       {
       BytesRead = ReadBlock(ffile, (char *)&CurValue[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      default:
       {
	   if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   } 
	  } /* switch */

     TotalRead += BytesRead;
     if (BytesRead != Size)
      break;
     } /* if size block read */
    else
     break;
    } /* if not done flag */
   } /* if tag block read */
  else
   break;
  } /* while */

 return (TotalRead);

} // AnimColor::Load

/*===========================================================================*/

unsigned long AnimColor::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_ANIMCOLOR_VALUE1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&CurValue[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_ANIMCOLOR_VALUE2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&CurValue[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_ANIMCOLOR_VALUE3, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&CurValue[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_ANIM_ANIMCRITTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = AnimCritter::Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if animcritter saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // AnimColor::Save

/*===========================================================================*/

AnimDoubleTime::~AnimDoubleTime()
{
long Ct;

if (GlobalApp && GlobalApp->GUIWins) // if these are already gone there are no windows left to close
	{
	for (Ct = 0; Ct < 10; Ct ++)
		{
		if (GlobalApp->GUIWins->GRG[Ct] && GlobalApp->GUIWins->GRG[Ct]->GetActive() == this)
			{
			delete GlobalApp->GUIWins->GRG[Ct];
			GlobalApp->GUIWins->GRG[Ct] = NULL;
			} // if
		} // for
	} // if

} // AnimDoubleTime::~AnimDoubleTime

/*===========================================================================*/

int AnimDoubleTime::SetToTime(double Time)
{
double TempVal;

if (Graph[0] && Graph[0]->GetNumNodes())
	{
	TempVal = Graph[0]->GetValue(Time);
	CurValue = AnimClamp(TempVal);
	return (1);
	} // if
return (0);

} // AnimDoubleTime::SetToTime

/*===========================================================================*/

GraphNode *AnimDoubleTime::AddNode(void)
{
double Time, TimeRange;

// find out what time or frame
if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES))
	{
	if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
		{
		Time = GetInputTime("Enter frame number for new Key Frame. Decimal frames are allowed.");
		} // if
	else
		{
		Time = GetInputTime("Enter time (seconds) for new Key Frame.");
		} // else

	TimeRange = .05 / GlobalApp->MainProj->Interactive->GetFrameRate();

	return (AnimDouble::AddNode(Time, TimeRange));
	} // if

return (NULL);

} // AnimDoubleTime::AddNode

/*===========================================================================*/

GraphNode *AnimDoubleTime::RemoveNode(void)
{
double Time, TimeRange;

Time = GlobalApp->MainProj->Interactive->GetActiveTime();
TimeRange = .05 / GlobalApp->MainProj->Interactive->GetFrameRate();

return (AnimDouble::RemoveNode(Time, TimeRange));

} // AnimDoubleTime::RemoveNode

/*===========================================================================*/

GraphNode *AnimDoubleTime::UpdateNode(void)
{
double Time, TimeRange;

Time = GlobalApp->MainProj->Interactive->GetActiveTime();
TimeRange = .05 / GlobalApp->MainProj->Interactive->GetFrameRate();

return (AnimDouble::UpdateNode(Time, TimeRange));

} // AnimDoubleTime::UpdateNode

/*===========================================================================*/

int AnimDoubleTime::ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop)
{

return (AnimCritter::ScaleDeleteAnimNodes(Prop));

} // AnimDoubleTime::ScaleDeleteAnimNodes

/*===========================================================================*/

int AnimDoubleTime::GetNextAnimNode(RasterAnimHostProperties *Prop)
{

return (AnimCritter::GetNextAnimNode(Prop));

} // AnimDoubleTime::GetNextAnimNode

/*===========================================================================*/

int AnimDoubleTime::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success = 0;
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (AnimDoubleTime *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (AnimCritter *)DropSource->DropSource);
			Success = 1;
			ObjectChanged();
			} // if
		} // if
	} // if

return (Success);

} // AnimDoubleTime::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long AnimDoubleTime::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATABLE)
	{
	if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES))
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATABLE;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_ANIMDOUBLETIME |
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimDoubleTime::GetRAFlags

/*===========================================================================*/

AnimDoubleDistance::~AnimDoubleDistance()
{
long Ct;

if (GlobalApp->GUIWins)
	{
	for (Ct = 0; Ct < 10; Ct ++)
		{
		if (GlobalApp->GUIWins->GRG[Ct] && GlobalApp->GUIWins->GRG[Ct]->GetActive() == this)
			{
			delete GlobalApp->GUIWins->GRG[Ct];
			GlobalApp->GUIWins->GRG[Ct] = NULL;
			} // if
		} // for
	} // if

} // AnimDoubleDistance::~AnimDoubleDistance

/*===========================================================================*/

int AnimDoubleDistance::SetToDistance(double Dist)
{

if (Graph[0] && Graph[0]->GetNumNodes())
	{
	CurValue = Graph[0]->GetValue(Dist);
	return (1);
	} // if
return (0);

} // AnimDoubleDistance::SetToDistance

/*===========================================================================*/

int AnimDoubleDistance::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success = 0;
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (AnimDoubleDistance *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (AnimCritter *)DropSource->DropSource);
			Success = 1;
			ObjectChanged();
			} // if
		} // if
	} // if

return (Success);

} // AnimDoubleDistance::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long AnimDoubleDistance::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_ANIMDOUBLEDISTANCE |
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimDoubleDistance::GetRAFlags

/*===========================================================================*/
/*===========================================================================*/

AnimDoubleCurve::AnimDoubleCurve()
{
double RangeDefaults[3] = {FLT_MAX, -FLT_MAX, 1.0};

SetRangeDefaults(RangeDefaults);

AddNode(0.0, 0.0, 0.0);
AddNode(10.0, 1.0, 0.0);

XLabel[0] = YLabel[0] = 0;
HorMetric = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;

} // AnimDoubleCurve::AnimDoubleCurve

/*===========================================================================*/

AnimDoubleCurve::~AnimDoubleCurve()
{
long Ct;

if (GlobalApp->GUIWins)
	{
	for (Ct = 0; Ct < 10; Ct ++)
		{
		if (GlobalApp->GUIWins->GRG[Ct] && GlobalApp->GUIWins->GRG[Ct]->GetActive() == this)
			{
			delete GlobalApp->GUIWins->GRG[Ct];
			GlobalApp->GUIWins->GRG[Ct] = NULL;
			} // if
		} // for
	} // if

} // AnimDoubleCurve::~AnimDoubleCurve

/*===========================================================================*/

void AnimDoubleCurve::SetDefaults(RasterAnimHost *RAHost, unsigned char Item)
{

RAParent = RAHost;
NotifyItem = Item;

} // AnimDoubleCurve::SetDefaults

/*===========================================================================*/

int AnimDoubleCurve::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success = 0;
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (AnimDoubleCurve *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (AnimDoubleCurve *)DropSource->DropSource);
			Success = 1;
			ObjectChanged();
			} // if
		} // if
	} // if

return (Success);

} // AnimDoubleCurve::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long AnimDoubleCurve::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_GRADIENTPROFILE |
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimDoubleCurve::GetRAFlags

/*===========================================================================*/
/*===========================================================================*/

AnimDoubleBoolean::AnimDoubleBoolean()
{
double RangeDefaults[3] = {1.0, 0.0, 1.0};

SetRangeDefaults(RangeDefaults);
SetMultiplier(1.0);

} // AnimDoubleBoolean::AnimDoubleBoolean

/*===========================================================================*/

unsigned long AnimDoubleBoolean::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_ENVELOPE |
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimDoubleBoolean::GetRAFlags

/*===========================================================================*/

double AnimDoubleBoolean::GetValue(char Channel, double Dist)
{
double Value;

Value = (Graph[0] && Graph[0]->GetNumNodes()) ? Graph[0]->GetValueBoolean(Dist + .00001): CurValue > .5 ? 1.0: 0.0;

return (Value);

} // AnimDoubleBoolean::GetValue

/*===========================================================================*/

void AnimDoubleBoolean::SetCurValue(char Component, double NewValue)
{

NewValue = NewValue > .5 ? 1.0: 0.0;
AnimDouble::SetCurValue(NewValue);

} // AnimDoubleBoolean::SetCurValue

/*===========================================================================*/

GraphNode *AnimDoubleBoolean::RemoteAlterSelectedNodeValue(double NewValue, double OldValue)
{

return (AnimCritter::RemoteAlterSelectedNodeValue(NewValue > .5 ? 1.0: 0.0, OldValue));

} // AnimDoubleBoolean::RemoteAlterSelectedNodeValue

/*===========================================================================*/
/*===========================================================================*/

AnimDoubleEnvelope::AnimDoubleEnvelope()
{
double RangeDefaults[3] = {1.0, 0.0, .01};
GraphNode *Node;

SetRangeDefaults(RangeDefaults);
SetMultiplier(100.0);

if (Node = AddNode(0.0, 0.0, 0.0))
	{
	Node->TCB[0] = -1.0;
	} // if
if (Node = AddNode(1.0, 1.0, 0.0))
	{
	Node->TCB[0] = 1.0;
	} // if
if (Node = AddNode(10.0, 0.0, 0.0))
	{
	Node->TCB[0] = 1.0;
	} // if

} // AnimDoubleEnvelope::AnimDoubleEnvelope

/*===========================================================================*/

void AnimDoubleEnvelope::SetDefaults(RasterAnimHost *RAHost, unsigned char Item)
{

RAParent = RAHost;
NotifyItem = Item;

} // AnimDoubleEnvelope::SetDefaults

/*===========================================================================*/

unsigned long AnimDoubleEnvelope::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_ENVELOPE |
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimDoubleEnvelope::GetRAFlags

/*===========================================================================*/
/*===========================================================================*/

AnimDoubleProfile::AnimDoubleProfile()
{
double RangeDefaults[3] = {1.0, 0.0, .01};
GraphNode *Node;

SetRangeDefaults(RangeDefaults);
SetMultiplier(100.0);

if (Node = AddNode(0.0, 0.0, 0.0))
	{
	Node->TCB[0] = 1.0;
	} // if
if (Node = AddNode(10.0, 1.0, 0.0))
	{
	Node->TCB[0] = 1.0;
	} // if

} // AnimDoubleProfile::AnimDoubleProfile

/*===========================================================================*/

void AnimDoubleProfile::SetDefaults(RasterAnimHost *RAHost, unsigned char Item)
{

RAParent = RAHost;
NotifyItem = Item;

} // AnimDoubleProfile::SetDefaults

/*===========================================================================*/

unsigned long AnimDoubleProfile::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_GRADIENTPROFILE |
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimDoubleProfile::GetRAFlags

/*===========================================================================*/
/*===========================================================================*/

AnimDoubleVerticalProfile::AnimDoubleVerticalProfile()
{
double RangeDefaults[3] = {1.0, 0.0, .01};
GraphNode *Node;

SetRangeDefaults(RangeDefaults);
SetMultiplier(100.0);

if (Node = AddNode(0.0, 0.0, 0.0))
	{
	Node->TCB[0] = -1.0;
	} // if
if (Node = AddNode(1.0, 0.0, 0.0))
	{
	Node->TCB[0] = -1.0;
	} // if
AddNode(.4, 1.0, 0.0);

} // AnimDoubleVerticalProfile::AnimDoubleVerticalProfile

/*===========================================================================*/

void AnimDoubleVerticalProfile::SetDefaults(RasterAnimHost *RAHost, unsigned char Item)
{

RAParent = RAHost;
NotifyItem = Item;

} // AnimDoubleVerticalProfile::SetDefaults

/*===========================================================================*/

unsigned long AnimDoubleVerticalProfile::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_ENVELOPE |
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimDoubleVerticalProfile::GetRAFlags

/*===========================================================================*/
/*===========================================================================*/

AnimDoubleCrossSection::AnimDoubleCrossSection()
{
double RangeDefaults[3] = {FLT_MAX, -FLT_MAX, .1};
GraphNode *Node;

SetRangeDefaults(RangeDefaults);
SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);

if (Node = AddNode(0.0, 0.0, 0.0))
	{
	Node->AddData(this);
	} // if

} // AnimDoubleCrossSection::AnimDoubleCrossSection

/*===========================================================================*/

void AnimDoubleCrossSection::SetDefaults(RasterAnimHost *RAHost, unsigned char Item)
{

RAParent = RAHost;
NotifyItem = Item;

} // AnimDoubleCrossSection::SetDefaults

/*===========================================================================*/

void *AnimDoubleCrossSection::GetNextSegmentWidth(void *PlaceHolder, double &SegWidth)
{
GraphNode *CurNode, *PrevNode;

if (! PlaceHolder)
	{
	if (PrevNode = Graph[0] ? Graph[0]->Nodes: NULL)
		{
		if (PrevNode->Distance > 0.0)
			{
			SegWidth = PrevNode->Distance;
			return (PrevNode);
			} // if
		} // if
	else
		return (NULL);
	} // if
else
	PrevNode = (GraphNode *)PlaceHolder;
	
if (CurNode = PrevNode->Next)
	{
	SegWidth = CurNode->Distance - PrevNode->Distance;
	return (CurNode);
	} // if

return (NULL);

} // AnimDoubleCrossSection::GetNextSegmentWidth

/*===========================================================================*/

// careful, segment width can be 0
void AnimDoubleCrossSection::FetchSegmentData(short &Priority, double &Roughness, double &EcoMixing, 
	double &DistFromPrevNode, double &SegmentWidth, double Dist)
{
GraphNode *CurNode, *PrevNode = NULL;

CurNode = Graph[0] ? Graph[0]->Nodes: NULL;

while (CurNode && CurNode->Distance < Dist)
	{
	PrevNode = CurNode;
	CurNode = CurNode->Next;
	} // while
if (CurNode && CurNode->Data)
	{
	Priority = CurNode->Data->Priority;
	Roughness = CurNode->Data->Roughness.CurValue;
	EcoMixing = CurNode->Data->EcoMixing.CurValue;
	DistFromPrevNode = PrevNode ? Dist - PrevNode->Distance: Dist;
	SegmentWidth = PrevNode ? CurNode->Distance - PrevNode->Distance: CurNode->Distance;
	} // if
else
	{
	Priority = 0;
	Roughness = EcoMixing = DistFromPrevNode = SegmentWidth = 0.0;
	} // else

} // AnimDoubleCrossSection::FetchSegmentData

/*===========================================================================*/

// Used with VectorPolygons in VNS 3
// careful, segment width can be 0
void AnimDoubleCrossSection::FetchSegmentData(short &Priority, double &Roughness, double &EcoMixing, 
	EcosystemEffect *&Eco, double &DistFromPrevNode, double &SegmentWidth, short SegNumber, double Dist)
{
GraphNode *CurNode, *PrevNode = NULL;
short CurSegCt;

CurNode = Graph[0] ? Graph[0]->Nodes: NULL;
Priority = 0;
Roughness = DistFromPrevNode = SegmentWidth = 0.0;
Eco = NULL;

for (CurSegCt = 0; CurSegCt <= SegNumber && CurNode; CurNode = CurNode->Next)
	{
	if (CurNode->Distance > 0.0)
		{
		if (CurSegCt == SegNumber)
			{
			if (CurNode->Data)
				{
				Priority = CurNode->Data->Priority;
				Roughness = CurNode->Data->Roughness.CurValue;
				EcoMixing = CurNode->Data->EcoMixing.CurValue;
				Eco = CurNode->Data->Eco;
				DistFromPrevNode = PrevNode ? Dist - PrevNode->Distance: Dist;
				SegmentWidth = PrevNode ? CurNode->Distance - PrevNode->Distance: CurNode->Distance;
				break;
				} // if
			} // if
		++CurSegCt;
		} // if
	PrevNode = CurNode;
	} // for
	
} // AnimDoubleCrossSection::FetchSegmentData

/*===========================================================================*/

void AnimDoubleCrossSection::FetchSegmentEco(EcosystemEffect *&Eco, double Dist)
{
GraphNode *CurNode;

CurNode = Graph[0] ? Graph[0]->Nodes: NULL;

while (CurNode && CurNode->Distance < Dist)
	{
	CurNode = CurNode->Next;
	} // while
if (CurNode && CurNode->Data)
	{
	Eco = CurNode->Data->Eco;
	} // if
else
	{
	Eco = NULL;
	} // else

} // AnimDoubleCrossSection::FetchSegmentEco

/*===========================================================================*/

void AnimDoubleCrossSection::FetchSegmentEcoPriority(EcosystemEffect *&Eco, short &Priority, double Dist)
{
GraphNode *CurNode;

CurNode = Graph[0] ? Graph[0]->Nodes: NULL;

while (CurNode && CurNode->Distance < Dist)
	{
	CurNode = CurNode->Next;
	} // while
if (CurNode && CurNode->Data)
	{
	Eco = CurNode->Data->Eco;
	Priority = CurNode->Data->Priority;
	} // if
else
	{
	Eco = NULL;
	Priority = 0;
	} // else

} // AnimDoubleCrossSection::FetchSegmentEcoPriority

/*===========================================================================*/

int AnimDoubleCrossSection::FindnRemoveEcosystems(EcosystemEffect *RemoveMe)
{
char QueryStr[256];
GraphNode *CurNode;
int Warned = 0;
NotifyTag Changes[2];

CurNode = Graph[0] ? Graph[0]->Nodes: NULL;

while (CurNode)
	{
	if (CurNode->Data && CurNode->Data->Eco == RemoveMe)
		{
		sprintf(QueryStr, "Remove %s %s from %s %s?", RemoveMe->GetName(), RemoveMe->GetRAHostTypeString(),
			GetRAHostRoot()->GetRAHostName(), GetRAHostRoot()->GetRAHostTypeString());
		if (Warned || UserMessageOKCAN(RemoveMe->GetName(), QueryStr))
			{
			Warned = 1;
			CurNode->Data->Eco = NULL;
			} // if
		else
			return (0);
		} // if
	CurNode = CurNode->Next;
	} // while

if (Warned)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (1);

} // AnimDoubleCrossSection::FindnRemoveEcosystems

/*===========================================================================*/

int AnimDoubleCrossSection::SetToTime(double Time)
{
int Found = 0;
GraphNode *CurNode;

CurNode = Graph[0] ? Graph[0]->Nodes: NULL;

while (CurNode)
	{
	if (CurNode->Data && CurNode->Data->SetToTime(Time))
		Found = 1;
	CurNode = CurNode->Next;
	} // while

return (Found);

} // AnimDoubleCrossSection::SetToTime

/*===========================================================================*/

long AnimDoubleCrossSection::InitImageIDs(long &ImageID)
{
long NumImages = 0;
GraphNode *CurNode;

CurNode = Graph[0] ? Graph[0]->Nodes: NULL;

while (CurNode)
	{
	if (CurNode->Data)
		NumImages += CurNode->Data->InitImageIDs(ImageID);
	CurNode = CurNode->Next;
	} // while

return (NumImages);

} // AnimDoubleCrossSection::InitImageIDs

/*===========================================================================*/

int AnimDoubleCrossSection::BuildFileComponentsList(EffectList **Ecosystems, EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
GraphNode *CurNode;

CurNode = Graph[0] ? Graph[0]->Nodes: NULL;

while (CurNode)
	{
	if (CurNode->Data)
		{
		if (! CurNode->Data->BuildFileComponentsList(Ecosystems, Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
			return (0);
		} // if
	CurNode = CurNode->Next;
	} // while

return (1);

} // AnimDoubleCrossSection::BuildFileComponentsList

/*===========================================================================*/

int AnimDoubleCrossSection::ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop)
{
int Success = 0;
GraphNode *CurNode;

CurNode = Graph[0] ? Graph[0]->Nodes: NULL;

while (CurNode)
	{
	if (CurNode->Data && CurNode->Data->ScaleDeleteAnimNodes(Prop))
		Success = 1;
	CurNode = CurNode->Next;
	} // while

return (Success);

} // AnimDoubleCrossSection::ScaleDeleteAnimNodes

/*===========================================================================*/

int AnimDoubleCrossSection::GetNextAnimNode(RasterAnimHostProperties *Prop)
{
int Success = 0;
GraphNode *CurNode;

CurNode = Graph[0] ? Graph[0]->Nodes: NULL;

while (CurNode)
	{
	if (CurNode->Data && CurNode->Data->GetNextAnimNode(Prop))
		Success = 1;
	CurNode = CurNode->Next;
	} // while

return (Success);

} // AnimDoubleCrossSection::GetNextAnimNode

/*===========================================================================*/

unsigned long AnimDoubleCrossSection::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_CROSSSECTION |
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimDoubleCrossSection::GetRAFlags

/*===========================================================================*/
/*===========================================================================*/

AnimColorTime::AnimColorTime()
{
double IntensityRangeDefaults[3] = {1000.0, 0.0, .01};

Intensity.SetDefaults(this, 1, 1.0);
Intensity.SetRangeDefaults(IntensityRangeDefaults);

} // AnimColorTime::AnimColorTime

/*===========================================================================*/

AnimColorTime::~AnimColorTime()
{
long Ct;

if (GlobalApp && GlobalApp->GUIWins) // if these are already gone there are no windows left to close
	{
	for (Ct = 0; Ct < 10; Ct ++)
		{
		if (GlobalApp->GUIWins->GRG[Ct] && GlobalApp->GUIWins->GRG[Ct]->GetActive() == this)
			{
			delete GlobalApp->GUIWins->GRG[Ct];
			GlobalApp->GUIWins->GRG[Ct] = NULL;
			} // if
		} // for
	if (GlobalApp->GUIWins->CEG && GlobalApp->GUIWins->CEG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->CEG;
		GlobalApp->GUIWins->CEG = NULL;
		} // if
	} // if

} // AnimColorTime::~AnimColorTime

/*===========================================================================*/

void AnimColorTime::SetDefaults(RasterAnimHost *RAHost, char Item)
{
double IntensityRangeDefaults[3] = {10.0, 0.0, .01};

AnimColor::SetDefaults(RAHost, Item, .5);
CurValue[0] = xrand48();
CurValue[1] = xrand48();
CurValue[2] = xrand48();
Intensity.SetDefaults(this, 1, 1.0);
Intensity.SetRangeDefaults(IntensityRangeDefaults);

Intensity.SetMultiplier(100.0);

} // AnimColorTime::SetDefaults

/*===========================================================================*/

void AnimColorTime::Copy(AnimColorTime *CopyTo, AnimColorTime *CopyFrom)
{

AnimColor::Copy(CopyTo, CopyFrom);
Intensity.Copy((AnimDouble *)&CopyTo->Intensity, (AnimDouble *)&CopyFrom->Intensity);

} // AnimColorTime::Copy

/*===========================================================================*/

void AnimColorTime::OpenEditor(void)
{
NotifyTag Changes[2];
DiagnosticData Data;

#ifdef WCS_BUILD_DEMO
DONGLE_INLINE_CHECK()
#else // !WCS_BUILD_DEMO

if(!GlobalApp->Sentinal->CheckDongle()) return;

#endif // !WCS_BUILD_DEMO

if(GlobalApp->GUIWins->CEG)
	{
	if (GlobalApp->GUIWins->CEG->GetActive() == this)
		{
		GlobalApp->GUIWins->CEG->Open(GlobalApp->MainProj);
		RasterAnimHost::SetActiveRAHost(this);
		return;
		} // if just want to pop to front
	else if (GlobalApp->GUIWins->CEG->GetResponseEnabled())
		{
		Data.DataRGB[0] = (unsigned char)(CurValue[0] * 255.0);
		Data.DataRGB[1] = (unsigned char)(CurValue[1] * 255.0);
		Data.DataRGB[2] = (unsigned char)(CurValue[2] * 255.0);
		Data.ValueValid[WCS_DIAGNOSTIC_RGB] = 1;
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, WCS_DIAGNOSTIC_ITEM_MOUSEDOWN, 0);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, &Data);
		return;
		} // if just gathering color data from color pot
	else
		delete GlobalApp->GUIWins->CEG;
	}
GlobalApp->GUIWins->CEG = new ColorEditGUI(this);
if(GlobalApp->GUIWins->CEG)
	{
	GlobalApp->GUIWins->CEG->Open(GlobalApp->MainProj);
	RasterAnimHost::SetActiveRAHost(this);
	}

} // AnimColorTime::OpenEditor

/*===========================================================================*/

int AnimColorTime::SetToTime(double Time)
{
int Found = 0;
double TempVal;

if (Graph[0] && Graph[0]->GetNumNodes())
	{
	TempVal = Graph[0]->GetValue(Time);
	CurValue[0] = AnimClamp(TempVal);
	Found = 1;
	} // if
if (Graph[1] && Graph[1]->GetNumNodes())
	{
	TempVal = Graph[1]->GetValue(Time);
	CurValue[1] = AnimClamp(TempVal);
	Found = 1;
	} // if
if (Graph[2] && Graph[2]->GetNumNodes())
	{
	TempVal = Graph[2]->GetValue(Time);
	CurValue[2] = AnimClamp(TempVal);
	Found = 1;
	} // if
if (Intensity.SetToTime(Time))
	Found = 1;

return (Found);

} // AnimColorTime::SetToTime

/*===========================================================================*/

GraphNode *AnimColorTime::AddNode(void)
{
double Time, TimeRange;

// find out what time or frame
if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES))
	{
	if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
		{
		Time = GetInputTime("Enter frame number for new Key Frame. Decimal frames are allowed.");
		} // if
	else
		{
		Time = GetInputTime("Enter time (seconds) for new Key Frame.");
		} // else

	TimeRange = .05 / GlobalApp->MainProj->Interactive->GetFrameRate();

	return (AnimColor::AddNode(Time, TimeRange));
	} // if

return (NULL);

} // AnimColorTime::AddNode

/*===========================================================================*/

GraphNode *AnimColorTime::RemoveNode(void)
{
double Time, TimeRange;

Time = GlobalApp->MainProj->Interactive->GetActiveTime();
TimeRange = .05 / GlobalApp->MainProj->Interactive->GetFrameRate();

return (AnimColor::RemoveNode(Time, TimeRange));

} // AnimColorTime::RemoveNode

/*===========================================================================*/

GraphNode *AnimColorTime::UpdateNode(void)
{
double Time, TimeRange;

Time = GlobalApp->MainProj->Interactive->GetActiveTime();
TimeRange = .05 / GlobalApp->MainProj->Interactive->GetFrameRate();

return (AnimColor::UpdateNode(Time, TimeRange));

} // AnimColorTime::UpdateNode

/*===========================================================================*/

int AnimColorTime::GetRAHostAnimated(void)
{

if (AnimColor::GetRAHostAnimated())
	return (1);
if (Intensity.GetRAHostAnimated())
	return (1);

return (0);

} // AnimColorTime::GetRAHostAnimated

/*===========================================================================*/

long AnimColorTime::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;

if (GetMinMaxDist(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (Intensity.GetMinMaxDist(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // AnimColorTime::GetKeyFrameRange

/*===========================================================================*/

int AnimColorTime::ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop)
{
int Success = 0;

if (AnimCritter::ScaleDeleteAnimNodes(Prop))
	Success = 1;
if (Intensity.ScaleDeleteAnimNodes(Prop))
	Success = 1;

return (Success);

} // AnimColorTime::ScaleDeleteAnimNodes

/*===========================================================================*/

int AnimColorTime::GetNextAnimNode(RasterAnimHostProperties *Prop)
{
int Success = 0;

if (AnimCritter::GetNextAnimNode(Prop))
	Success = 0;
if (Intensity.GetNextAnimNode(Prop))
	Success = 0;

return (Success);

} // AnimColorTime::GetNextAnimNode

/*===========================================================================*/

char AnimColorTime::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);

return (0);

} // AnimColorTime::GetRAHostDropOK

/*===========================================================================*/

int AnimColorTime::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success = 0;
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (AnimColorTime *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (AnimColorTime *)DropSource->DropSource);
			Success = 1;
			ObjectChanged();
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = Intensity.ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // AnimColorTime::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long AnimColorTime::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATABLE)
	{
	if (! TestFlags(WCS_ANIMCRITTER_FLAG_NONODES))
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATABLE;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_ANIMCOLORTIME | WCS_RAHOST_FLAGBIT_CHILDREN |
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimColorTime::GetRAFlags

/*===========================================================================*/

void AnimColorTime::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = RAParent ? RAParent->GetCritterName(this): (char*)"";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_INTERFLAGS)
	{
	GetInterFlags(Prop, this);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // AnimColorTime::GetRAHostProperties

/*===========================================================================*/

int AnimColorTime::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		if (Prop->KeyframeOperation == WCS_KEYOPERATION_DELETE)
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
		else
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), NotifyItem, WCS_NOTIFYCOMP_ANIM_POSITIONCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

return (Success);

} // AnimColorTime::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *AnimColorTime::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{

if (! Current)
	return (&Intensity);

return (NULL);

} // AnimColorTime::GetRAHostChild

/*===========================================================================*/

char *AnimColorTime::GetCritterName(RasterAnimHost *Test)
{

if (Test == &Intensity)
	return ("Intensity (%)");

return ("");

} // AnimColorTime::GetCritterName

/*===========================================================================*/

int AnimColorTime::GetAffiliates(RasterAnimHost *ChildA, AnimCritter *&AnimAffil)
{

AnimAffil = NULL;

if (ChildA)
	{
	if (ChildA == &Intensity)
		{
		AnimAffil = &Intensity;
		return (1);
		} // if
	} // if

return (0);

} // AnimColorTime::GetAffiliates

/*===========================================================================*/

int AnimColorTime::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, AnimAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, NULL, NULL));
	} // if

return (0);

} // AnimColorTime::GetPopClassFlags

/*===========================================================================*/

int AnimColorTime::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, AnimAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, NULL, NULL));
	} // if

return(0);

} // AnimColorTime::AddSRAHBasePopMenus

/*===========================================================================*/

int AnimColorTime::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, AnimAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, NULL, NULL, NULL, NULL));
	} // if

return(0);

} // AnimColorTime::HandleSRAHPopMenuSelection

/*===========================================================================*/

ULONG AnimColorTime::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

 while (ItemTag != WCS_PARAM_DONE)
  {
  /* read block descriptor tag from file */
  if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
   		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
   {
   TotalRead += BytesRead;
   if (ItemTag != WCS_PARAM_DONE)
    {
	/* read block size from file */
    if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
     {
     TotalRead += BytesRead;
     BytesRead = 0;
     switch (ItemTag & 0xff)
      {
      case WCS_BLOCKSIZE_CHAR:
       {
       Size = MV.Char[0];
       break;
       }
      case WCS_BLOCKSIZE_SHORT:
       {
       Size = MV.Short[0];
       break;
       }
      case WCS_BLOCKSIZE_LONG:
       {
       Size = MV.Long;
       break;
	   }
      } /* switch */

     switch (ItemTag & 0xffff0000)
      {
      case WCS_ANIM_ANIMCRITTER:
       {
       BytesRead = AnimCritter::Load(ffile, Size, ByteFlip);
       break;
	   }
      case WCS_ANIMCOLOR_VALUE1:
       {
       BytesRead = ReadBlock(ffile, (char *)&CurValue[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_ANIMCOLOR_VALUE2:
       {
       BytesRead = ReadBlock(ffile, (char *)&CurValue[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_ANIMCOLOR_VALUE3:
       {
       BytesRead = ReadBlock(ffile, (char *)&CurValue[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_ANIMCOLORTIME_INTENSITY:
       {
       BytesRead = Intensity.Load(ffile, Size, ByteFlip);
       break;
	   }
      default:
       {
	   if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   } 
	  } /* switch */

     TotalRead += BytesRead;
     if (BytesRead != Size)
      break;
     } /* if size block read */
    else
     break;
    } /* if not done flag */
   } /* if tag block read */
  else
   break;
  } /* while */

 return (TotalRead);

} // AnimColorTime::Load

/*===========================================================================*/

unsigned long AnimColorTime::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_ANIMCOLOR_VALUE1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&CurValue[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ANIMCOLOR_VALUE2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&CurValue[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ANIMCOLOR_VALUE3, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&CurValue[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_ANIM_ANIMCRITTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = AnimCritter::Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if animcritter saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_ANIMCOLORTIME_INTENSITY + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Intensity.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if animcritter saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // AnimColorTime::Save

/*===========================================================================*/

GradientCritter::GradientCritter(RasterAnimHost *RAHost)
{
double PositionRangeDefaults[3] = {1.0, 0.0, .01};

Position.SetDefaults(RAHost, 0, 0.0);
Position.SetRangeDefaults(PositionRangeDefaults);

BlendStyle = WCS_GRADIENTCRITTER_BLENDSTYLE_FULL;
GradientThing = NULL;
Next = Prev = NULL;

Position.SetMultiplier(100.0);

} // GradientCritter::GradientCritter

/*===========================================================================*/

GradientCritter::~GradientCritter()
{

// GradThing will be deleted by the parent class since it knows how to delete the GradientThing

} // GradientCritter::~GradientCritter

/*===========================================================================*/

void GradientCritter::Copy(GradientCritter *CopyTo, GradientCritter *CopyFrom)
{

Position.Copy((AnimCritter *)&CopyTo->Position, (AnimCritter *)&CopyFrom->Position);
CopyTo->BlendStyle = CopyFrom->BlendStyle;
// derived classes will handle copying GradientThing

} // GradientCritter::Copy

/*===========================================================================*/

int GradientCritter::SetToTime(double Time)
{
int Found = 0;

if (Position.SetToTime(Time))
	Found = 1;
// GradientThing is handled by derivative classes

return (Found);

} // GradientCritter::SetToTime

/*===========================================================================*/

int GradientCritter::IsAnimated(void)
{

if (Position.GetNumNodes(0) > 1)
	return (1);

return (0);

} // GradientCritter::IsAnimated

/*===========================================================================*/

int GradientCritter::GetRAHostAnimated(void)
{

if (Position.GetRAHostAnimated())
	return (1);

return (0);

} // GradientCritter::GetRAHostAnimated

/*===========================================================================*/

int GradientCritter::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil)
{

AnimAffil = NULL;

if (ChildA)
	{
	if (ChildA == &Position)
		{
		AnimAffil = &Position;
		return (1);
		} // if
	} // if

return (0);

} // GradientCritter::GetAffiliates

/*===========================================================================*/

ULONG GradientCritter::AnimColorGradient_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, RasterAnimHost *RAHost, unsigned char ApplyToEcosys)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	/* read block descriptor tag from file */
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			/* read block size from file */
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} /* switch */

				switch (ItemTag & 0xffff0000)
					{
					case WCS_GRADIENTCRITTER_POSITION:
						{
						BytesRead = Position.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_GRADIENTCRITTER_BLENDSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&BlendStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_GRADIENTCRITTER_COLORTEXTURE:
						{
						// BIG Mystery: If you allocate and test GradientThing in one step
						// the test for truth will fail even though allocation succeeds and GradientThing is non-NULL.
						// Do it in two steps and everybody happy! Go figger.
						GradientThing = new ColorTextureThing(RAHost, ApplyToEcosys);
						if (GradientThing)
							BytesRead = ((ColorTextureThing *)GradientThing)->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} /* switch */

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} /* if size block read */
			else
				break;
			} /* if not done flag */
		} /* if tag block read */
	else
		break;
	} /* while */

return (TotalRead);

} // GradientCritter::AnimColorGradient_Load

/*===========================================================================*/

unsigned long GradientCritter::AnimColorGradient_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

ItemTag = WCS_GRADIENTCRITTER_POSITION + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Position.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if position saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRADIENTCRITTER_BLENDSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BlendStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (GradientThing)
	{
	ItemTag = WCS_GRADIENTCRITTER_COLORTEXTURE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = ((ColorTextureThing *)GradientThing)->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if ColorTextureThing saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // GradientCritter::AnimColorGradient_Save

/*===========================================================================*/

ULONG GradientCritter::AnimMaterialGradient_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, 
	RasterAnimHost *RAHost, char MaterialType)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch

				switch (ItemTag & 0xffff0000)
					{
					case WCS_GRADIENTCRITTER_POSITION:
						{
						BytesRead = Position.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_GRADIENTCRITTER_BLENDSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&BlendStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_GRADIENTCRITTER_MATERIAL:
						{
						// BIG Mystery: If you allocate and test GradientThing in one step
						// the test for truth will fail even though allocation succeeds and GradientThing is non-NULL.
						// Do it in two steps and everybody happy! Go figger.
						
						assert(!GradientThing);	// if you've trapped this in a debugger, Lint was right!
						GradientThing = new MaterialEffect(RAHost, MaterialType);	// Lint warns of a possible memory leak
						if (GradientThing)
							BytesRead = ((MaterialEffect *)GradientThing)->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read
			else
				break;
			} // if not done flag
		} // if tag block read
	else
		break;
	} // while

return (TotalRead);

} // GradientCritter::AnimMaterialGradient_Load

/*===========================================================================*/

unsigned long GradientCritter::AnimMaterialGradient_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

ItemTag = WCS_GRADIENTCRITTER_POSITION + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Position.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if position saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_GRADIENTCRITTER_BLENDSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BlendStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (GradientThing)
	{
	ItemTag = WCS_GRADIENTCRITTER_MATERIAL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = ((MaterialEffect *)GradientThing)->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if ColorTextureThing saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // GradientCritter::AnimMaterialGradient_Save

/*===========================================================================*/

AnimGradient::AnimGradient(RasterAnimHost *RAHost, unsigned char EcosystemSource)
: RasterAnimHost(RAHost)
{

Grad = ActiveNode = NULL;
ApplyToEcosys = EcosystemSource;

} // AnimGradient::AnimGradient

/*===========================================================================*/

AnimGradient::~AnimGradient()
{

// Grad will be deleted by the derived class since it knows how to delete the GradientThing

} // AnimGradient::~AnimGradient

/*===========================================================================*/

GradientCritter *AnimGradient::AddNodeNotify(double NewDist, int Interpolate)
{
GradientCritter *AddedNode;
NotifyTag Changes[2];

if (AddedNode = AddNode(NewDist, Interpolate))	// virtual method
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (AddedNode);

} // AnimGradient::AddNodeNotify

/*===========================================================================*/

double AnimGradient::CertifyNodePosition(GradientCritter *CheckNode)
{
GradientCritter *CurGrad = Grad, *LastGrad = NULL;
double LastPos = 0.0, Incr = .01;

while (CurGrad)
	{
	if (CurGrad == CheckNode)
		{
		if (LastGrad && CurGrad->Position.CurValue <= LastPos)
			{
			if (LastPos + Incr > 1.0)
				Incr = (1.0 - LastPos);
			if (CurGrad->Next && LastPos + Incr >= CurGrad->Next->Position.CurValue)
				Incr = (CurGrad->Next->Position.CurValue - LastPos) * 0.5;
			SetNodeDistance(CurGrad, LastPos + Incr);
			} // if
		else if (CurGrad->Next && CurGrad->Position.CurValue >= CurGrad->Next->Position.CurValue)
			{
			if (CurGrad->Next->Position.CurValue - Incr < 0.0)
				Incr = (CurGrad->Next->Position.CurValue - Incr);
			if (LastGrad && CurGrad->Next->Position.CurValue - Incr <= LastPos)
				Incr = (CurGrad->Next->Position.CurValue - LastPos) * 0.5;
			SetNodeDistance(CurGrad, CurGrad->Next->Position.CurValue - Incr);
			} // else if
		} // if found it
	LastGrad = CurGrad;
	LastPos = CurGrad->Position.CurValue;
	CurGrad = CurGrad->Next;
	} // while

return (CheckNode->Position.CurValue);

} // AnimGradient::CertifyNodePosition

/*===========================================================================*/

int AnimGradient::RemoveNode(long RemoveNum)
{
GradientCritter *RemoveMe;
long Ct = 0;

RemoveMe = Grad;
while (RemoveMe && Ct < RemoveNum)
	{
	RemoveMe = RemoveMe->Next;
	} // while
if (RemoveMe)
	{
	if (RemoveMe == Grad && ! Grad->Next)
		return (0);		// can't remove the last one
	return (RemoveNode(RemoveMe));
	} // if

return (0);

} // AnimGradient::RemoveNode

/*===========================================================================*/

int AnimGradient::RemoveNodeSetup(GradientCritter *RemoveMe)
{
GradientCritter *PrevGrad, *NextGrad;

if (RemoveMe)
	{
	PrevGrad = RemoveMe->Prev;
	NextGrad = RemoveMe->Next;
	if (PrevGrad)
		PrevGrad->Next = NextGrad;
	else
		Grad = NextGrad;
	if (NextGrad)
		NextGrad->Prev = PrevGrad;
	return (1);
	} // while

return (0);

} // AnimGradient::RemoveNodeSetup

/*===========================================================================*/

int AnimGradient::AddNodeSetup(double NewDist, GradientCritter *&AddBefore, GradientCritter *&AddAfter)
{
GradientCritter *Current = Grad, *NextGrad;

while (Current)
	{
	NextGrad = Current->Next;
	if (NewDist < Current->GetDistance())
		{
		AddBefore = Current;
		break;
		}
	else if (NewDist == Current->GetDistance())
		{
		return (0);
		} // else if
	else if (NextGrad)
		{
		if (NewDist < NextGrad->GetDistance())
			{
			AddBefore = NextGrad;
			break;
			} // if
		else if (NewDist == NextGrad->GetDistance())
			{
			return (0);
			} // else if
		Current = NextGrad;
		NextGrad = NextGrad->Next;
		} //else if
	else
		{
		AddAfter = Current;
		break;
		}
	} // while

return (1);

} // AnimGradient::AddNodeSetup

/*===========================================================================*/

GradientCritter *AnimGradient::FindNode(double MatchDist, double DistRange)
{
GradientCritter *Current = Grad;
double MatchDistLow, MatchDistHigh;

MatchDistLow = MatchDist - fabs(DistRange);
MatchDistHigh = MatchDist + fabs(DistRange);

while (Current)
	{
	if (Current->GetDistance() <= MatchDistHigh && Current->GetDistance() >= MatchDistLow)
		{
		return (Current);
		} // else if
	Current = Current->Next;
	} // while

return (NULL);

} // AnimGradient::FindNode

/*===========================================================================*/

GradientCritter *AnimGradient::ValidateNode(GradientCritter *CheckNode)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad == CheckNode)
		return (CheckNode);
	CurGrad = CurGrad->Next;
	} // while

return (NULL);

} // AnimGradient::ValidateNode

/*===========================================================================*/

long AnimGradient::CountNodes(void)
{
GradientCritter *CurGrad = Grad;
long NodeCt = 0;

while (CurGrad)
	{
	++ NodeCt;
	CurGrad = CurGrad->Next;
	} // while

return (NodeCt);

} // AnimGradient::CountNodes

/*===========================================================================*/

GradientCritter *AnimGradient::SetActiveNode(GradientCritter *NewActive)
{
NotifyTag Changes[2];

if (ValidateNode(NewActive))
	ActiveNode = NewActive;

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

if (GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT)
	RasterAnimHost::SetActiveRAHost(ActiveNode ? (ColorTextureThing *)ActiveNode->GetThing(): NULL);
else if (GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT)
	RasterAnimHost::SetActiveRAHost(ActiveNode ? (MaterialEffect *)ActiveNode->GetThing(): NULL);

return (ActiveNode);

} // AnimGradient::SetActiveNode

/*===========================================================================*/

int AnimGradient::SetNodeDistance(GradientCritter *SetNode, double NewDistance)
{
GradientCritter *CurGrad = Grad, *LastGrad = NULL;
double LastPos = 0.0;
NotifyTag Changes[2];

if (NewDistance < 0.0)
	NewDistance = 0.0;
else if (NewDistance > 1.0)
	NewDistance = 1.0;

while (CurGrad)
	{
	if (CurGrad == SetNode)
		{
		if (NewDistance != CurGrad->Position.CurValue)
			{
			if (! LastGrad || NewDistance > LastPos)
				{
				if (! CurGrad->Next || NewDistance < CurGrad->Next->Position.CurValue)
					{
					CurGrad->SetDistance(NewDistance);
					Changes[0] = MAKE_ID(CurGrad->Position.GetNotifyClass(), CurGrad->Position.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, CurGrad->Position.GetRAHostRoot());
					} // if
				} // if
			} // if position changing
		return (0);
		} // if
	LastGrad = CurGrad;
	LastPos = CurGrad->Position.CurValue;
	CurGrad = CurGrad->Next;
	} // while

return (0);

} // AnimGradient::SetNodeDistance

/*===========================================================================*/

char *AnimGradient::GetCritterName(RasterAnimHost *Test)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (Test == CurGrad->GetThing())
		return ("");
	if (Test == &CurGrad->Position)
		return ("Gradient Position");
	CurGrad = CurGrad->Next;
	} // while

return ("");

} // AnimGradient::GetCritterName

/*===========================================================================*/

int AnimGradient::GetDeletable(RasterAnimHost *Test)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (Test == CurGrad->GetThing())
		return (1);
	CurGrad = CurGrad->Next;
	} // while

return (0);

} // AnimGradient::GetDeletable

/*===========================================================================*/

int AnimGradient::RemoveRAHost(RasterAnimHost *RemoveMe)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing() == (void *)RemoveMe)
		{
		RemoveNode(CurGrad);

		return (1);
		} // if
	CurGrad = CurGrad->Next;
	} // while

return (0);

} // AnimGradient::RemoveRAHost

/*===========================================================================*/

int AnimGradient::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil)
{
GradientCritter *CurGrad = Grad;

AnimAffil = NULL;

while (CurGrad)
	{
	if (CurGrad->GetAffiliates(ChildA, ChildB, AnimAffil))
		{
		return (1);
		} // if
	CurGrad = CurGrad->Next;
	} // while

return (0);

} // AnimGradient::GetAffiliates

/*===========================================================================*/

int AnimGradient::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, NULL, NULL));
	} // if

return (0);

} // AnimGradient::GetPopClassFlags

/*===========================================================================*/

int AnimGradient::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, NULL, NULL));
	} // if

return(0);

} // AnimGradient::AddSRAHBasePopMenus

/*===========================================================================*/

int AnimGradient::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, NULL, NULL, NULL, NULL));
	} // if

return(0);

} // AnimGradient::HandleSRAHPopMenuSelection

/*===========================================================================*/
/*===========================================================================*/

ColorTextureThing::ColorTextureThing(RasterAnimHost *RAHost, unsigned char EcosystemSource)
: RasterAnimHost(RAHost)
{

Tex = NULL;
Color.SetDefaults(this, 0);
ApplyToEcosys = EcosystemSource;

} // ColorTextureThing::ColorTextureThing

/*===========================================================================*/

ColorTextureThing::~ColorTextureThing()
{
Texture *TempTex;

if (Tex)
	{
	// null out the pointer before deleting object or notifications can 
	// cause attempted reading from object while it is being destroyed
	TempTex = Tex;
	Tex = NULL;
	delete TempTex;
	} // if

} // ColorTextureThing::~ColorTextureThing

/*===========================================================================*/

void ColorTextureThing::Copy(ColorTextureThing *CopyTo, ColorTextureThing *CopyFrom)
{
RasterAnimHostProperties Prop;

if (CopyTo->RAParent)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	CopyTo->RAParent->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT)
		{
		CopyTo->ApplyToEcosys = ((AnimColorGradient *)RAParent)->ApplyToEcosys;
		} // if
	} // if
else
	CopyTo->ApplyToEcosys = CopyFrom->ApplyToEcosys;

Color.Copy(&CopyTo->Color, &CopyFrom->Color);
if (CopyTo->Tex)
	delete CopyTo->Tex;
CopyTo->Tex = NULL;
if (CopyFrom->Tex)
	{
	if (CopyTo->Tex = RootTexture::NewTexture(this, CopyFrom->Tex, CopyFrom->Tex->GetTexType(), WCS_TEXTURE_COLOR1))
		{
		CopyTo->Tex->Copy(CopyTo->Tex, CopyFrom->Tex);
		} // if
	} // if

} // ColorTextureThing::Copy

/*===========================================================================*/

Texture *ColorTextureThing::NewTexture(Texture *Proto)
{
NotifyTag Changes[2];

if (Tex)
	delete Tex;
if (Tex = RootTexture::NewTexture(this, Proto, Proto ? Proto->GetTexType(): WCS_TEXTURE_TYPE_FRACTALNOISE, WCS_TEXTURE_COLOR1))
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	if (Proto)
		{
		Tex->Copy(Tex, Proto);
		Changes[0] = MAKE_ID(Tex->GetNotifyClass(), Tex->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Tex->GetRAHostRoot());
		} // if
	Tex->ApplyToColor = 1;
	Tex->ApplyToEcosys = ApplyToEcosys;
	} // if

return (Tex);

} // ColorTextureThing::NewTexture

/*===========================================================================*/

int ColorTextureThing::SetToTime(double Time)
{
int Found = 0;

if (Color.SetToTime(Time))
	Found = 1;
if (Tex && Tex->SetToTime(Time))
	Found = 1;

return (Found);

} // ColorTextureThing::SetToTime

/*===========================================================================*/

void ColorTextureThing::SetTexture(Texture *NewTex)
{

if (Tex)
	delete Tex;
Tex = NewTex;

} // ColorTextureThing::SetTexture

/*===========================================================================*/

void ColorTextureThing::SetTexturePrev(Texture *SetPrev)
{

if (Tex)
	Tex->Prev = SetPrev;

} // ColorTextureThing::SetTexturePrev

/*===========================================================================*/

int ColorTextureThing::GetRAHostAnimated(void)
{

if (Color.GetRAHostAnimated())
	return (1);
if (Tex && Tex->GetRAHostAnimated())
	return (1);

return (0);

} // ColorTextureThing::GetRAHostAnimated

/*===========================================================================*/

bool ColorTextureThing::AnimateMaterials(void)
{

if (Tex && Tex->GetRAHostAnimatedInclVelocity())
	return (true);

return (false);

} // ColorTextureThing::AnimateMaterials

/*===========================================================================*/

long ColorTextureThing::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;

if (Color.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (Tex && Tex->GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // ColorTextureThing::GetKeyFrameRange

/*===========================================================================*/

char *ColorTextureThing::GetCritterName(RasterAnimHost *Test)
{

if (Test == &Color)
	return ("Color");
if (Test == Tex)
	return ("Color");

return ("");

} // ColorTextureThing::GetCritterName

/*===========================================================================*/

int ColorTextureThing::GetDeletable(RasterAnimHost *Test)
{

if (Test == Tex)
	return (1);

return (0);

} // ColorTextureThing::GetDeletable

/*===========================================================================*/

char ColorTextureThing::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropType == WCS_RAHOST_OBJTYPE_TEXTURE)
	return (1);

return (0);

} // ColorTextureThing::GetRAHostDropOK

/*===========================================================================*/

int ColorTextureThing::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success = 0;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (ColorTextureThing *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (ColorTextureThing *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = Color.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	if (Tex || (Tex = NewTexture((Texture *)DropSource->DropSource)))
		Success = Tex->ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // ColorTextureThing::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long ColorTextureThing::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_COLORTEXTURE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // ColorTextureThing::GetRAFlags

/*===========================================================================*/

void ColorTextureThing::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = RAParent ? RAParent->GetCritterName(this): (char*)"";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_INTERFLAGS)
	{
	GetInterFlags(Prop, this);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // ColorTextureThing::GetRAHostProperties

/*===========================================================================*/

int ColorTextureThing::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

return (Success);

} // ColorTextureThing::SetRAHostProperties

/*===========================================================================*/

int ColorTextureThing::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil)
{

AnimAffil = NULL;

if (ChildA)
	{
	if (ChildA == &Color)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	} // if

return (0);

} // ColorTextureThing::GetAffiliates

/*===========================================================================*/

int ColorTextureThing::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, NULL, NULL));
	} // if

return (0);

} // ColorTextureThing::GetPopClassFlags

/*===========================================================================*/

int ColorTextureThing::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, NULL, NULL));
	} // if

return(0);

} // ColorTextureThing::AddSRAHBasePopMenus

/*===========================================================================*/

int ColorTextureThing::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, NULL, NULL, NULL, NULL));
	} // if

return(0);

} // ColorTextureThing::HandleSRAHPopMenuSelection

/*===========================================================================*/

RasterAnimHost *ColorTextureThing::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{

if (! Current)
	return (&Color);
if (Current == &Color)
	return (Tex);

return (NULL);

} // ColorTextureThing::GetRAHostChild

/*===========================================================================*/

long ColorTextureThing::InitImageIDs(long &ImageID)
{
long NumImages = 0;

if (Tex)
	{
	NumImages += Tex->InitImageIDs(ImageID);
	} // if

return (NumImages);

} // ColorTextureThing::InitImageIDs

/*===========================================================================*/

int ColorTextureThing::InitToRender(void)
{

if (Tex)
	{
	if (! Tex->InitAAChain())
		return (0);
	} // if

return (1);

} // ColorTextureThing::InitToRender

/*===========================================================================*/

int ColorTextureThing::RemoveRAHost(RasterAnimHost *RemoveMe)
{
NotifyTag Changes[2];

if (Tex == (Texture *)RemoveMe)
	{
	delete Tex;
	Tex = NULL;

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (0);

} // ColorTextureThing::RemoveRAHost

/*===========================================================================*/

ULONG ColorTextureThing::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TextureTypeStr[64];
char NewTexType;

while (ItemTag != WCS_PARAM_DONE)
	{
	/* read block descriptor tag from file */
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			/* read block size from file */
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} /* switch */

				switch (ItemTag & 0xffff0000)
					{
					case WCS_COLORTEXTURE_COLOR:
						{
						BytesRead = Color.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_COLORTEXTURE_TEXTURETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)TextureTypeStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if ((NewTexType = RootTexture::IdentifyTextureType(TextureTypeStr)) >= 0)
							{
							Tex = RootTexture::NewTexture(this, NULL, NewTexType, WCS_TEXTURE_COLOR1);
							} // if
						break;
						}
					case WCS_COLORTEXTURE_TEXTURE:
						{
						if (Tex)
							{
							BytesRead = Tex->Load(ffile, Size, ByteFlip);
							Tex->Prev = NULL;
							Tex->InitNoise();	// do this in case any of the values loaded differ from defaults
							Tex->InitBasis();
							}
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} /* switch */

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} /* if size block read */
			else
				break;
			} /* if not done flag */
		} /* if tag block read */
	else
		break;
	} /* while */

return (TotalRead);

} // ColorTextureThing::Load

/*===========================================================================*/
	
unsigned long ColorTextureThing::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

ItemTag = WCS_COLORTEXTURE_COLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Color.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if color saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

if (Tex)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_COLORTEXTURE_TEXTURETYPE, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(RootTexture::GetTextureName(Tex->GetTexType())) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)RootTexture::GetTextureName(Tex->GetTexType()))) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	ItemTag = WCS_COLORTEXTURE_TEXTURE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Tex->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block
				else
					goto WriteError;
				} // if texture saved
			else
				goto WriteError;
			} // if size written
		else
			goto WriteError;
		} // if tag written
	else
		goto WriteError;
	}

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // ColorTextureThing::Save

/*===========================================================================*/

AnimColorGradient::AnimColorGradient(RasterAnimHost *RAHost, unsigned char EcosystemSource)
: AnimGradient(RAHost, EcosystemSource)
{

ApplyToEcosys = EcosystemSource;

if (Grad = new GradientCritter(this))
	{
	Grad->SetThing(new ColorTextureThing(this, ApplyToEcosys));
	} // if

} // AnimColorGradient::AnimColorGradient

/*===========================================================================*/

AnimColorGradient::~AnimColorGradient()
{
GradientCritter *CurGrad, *ScanGrad = Grad;
void *TempThing;

Grad = NULL;
ActiveNode = NULL;
while (ScanGrad)
	{
	CurGrad = ScanGrad;
	ScanGrad = ScanGrad->Next;
	if (TempThing = CurGrad->GetThing())
		{
		CurGrad->SetThing(NULL);	// not really necessary but it feels like the right thing to do
		delete (ColorTextureThing *)TempThing;
		} // if
	delete CurGrad;
	} // while

} // AnimColorGradient::~AnimColorGradient

/*===========================================================================*/

void AnimColorGradient::Copy(AnimColorGradient *CopyTo, AnimColorGradient *CopyFrom)
{
GradientCritter *NextGrad, *ScanGrad, *ToGrad = NULL;
void *TempThing;
RasterAnimHostProperties Prop;

if (CopyTo->RAParent)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	CopyTo->RAParent->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
		{
		CopyTo->ApplyToEcosys = ((Texture *)RAParent)->ApplyToEcosys;
		} // if
	} // if
else
	CopyTo->ApplyToEcosys = CopyFrom->ApplyToEcosys;

ScanGrad = CopyTo->Grad;
CopyTo->ActiveNode = NULL;
CopyTo->Grad = NULL;
while (ScanGrad)
	{
	NextGrad = ScanGrad->Next;
	if (TempThing = ScanGrad->GetThing())
		{
		ScanGrad->SetThing(NULL);
		delete (ColorTextureThing *)TempThing;
		} // if
	delete ScanGrad;
	ScanGrad = NextGrad;
	} // while
if (NextGrad = CopyFrom->Grad)
	{
	if (CopyTo->Grad = new GradientCritter(CopyTo))
		{
		ToGrad = CopyTo->Grad;
		ToGrad->Copy(ToGrad, NextGrad);
		if (NextGrad->GetThing())
			{
			ToGrad->SetThing(new ColorTextureThing(CopyTo, ApplyToEcosys));
			if (ToGrad->GetThing())
				{
				((ColorTextureThing *)ToGrad->GetThing())->Copy((ColorTextureThing *)ToGrad->GetThing(), (ColorTextureThing *)NextGrad->GetThing());
				} // if
			} // if
		NextGrad = NextGrad->Next;
		} // if
	while (NextGrad && ToGrad)
		{
		if (ToGrad->Next = new GradientCritter(CopyTo))
			{
			ToGrad->Next->Prev = ToGrad;
			ToGrad = ToGrad->Next;
			ToGrad->Copy(ToGrad, NextGrad);
			if (NextGrad->GetThing())
				{
				ToGrad->SetThing(new ColorTextureThing(CopyTo, ApplyToEcosys));
				if (ToGrad->GetThing())
					{
					((ColorTextureThing *)ToGrad->GetThing())->Copy((ColorTextureThing *)ToGrad->GetThing(), (ColorTextureThing *)NextGrad->GetThing());
					} // if
				} // if
			} // if
		NextGrad = NextGrad->Next; 
		} // while
	} // if

} // AnimColorGradient::Copy

/*===========================================================================*/

GradientCritter *AnimColorGradient::AddNode(double NewDist, int Interpolate)
{
GradientCritter *AddBefore = NULL, *AddAfter = NULL, *Stash;
double Red, Green, Blue;

if (AddNodeSetup(NewDist, AddBefore, AddAfter))
	{
	// find color at new distance
	if (AddBefore)
		{
		GetBasicColor(Red, Green, Blue, NewDist);
		Stash = AddBefore->Prev;
		if (AddBefore->Prev = new GradientCritter(this))
			{
			AddBefore->Prev->SetThing(new ColorTextureThing(this, ApplyToEcosys));
			AddBefore->Prev->SetDistance(NewDist);
			AddBefore->Prev->Prev = Stash;
			if (Stash)
				Stash->Next = AddBefore->Prev;
			AddBefore->Prev->Next = AddBefore;
			if (! Stash)
				Grad = AddBefore->Prev;
			if (Interpolate && AddBefore->Prev->GetThing())
				((ColorTextureThing *)AddBefore->Prev->GetThing())->Color.SetValue3(Red, Green, Blue);
			return (AddBefore->Prev);
			} // if
		} // if
	else if (AddAfter)
		{
		GetBasicColor(Red, Green, Blue, NewDist);
		Stash = AddAfter->Next;
		if (AddAfter->Next = new GradientCritter(this))
			{
			AddAfter->Next->SetThing(new ColorTextureThing(this, ApplyToEcosys));
			AddAfter->Next->SetDistance(NewDist);
			AddAfter->Next->Next = Stash;
			if (Stash)
				Stash->Prev = AddAfter->Next;
			AddAfter->Next->Prev = AddAfter;
			if (Interpolate && AddAfter->Next->GetThing())
				((ColorTextureThing *)AddAfter->Next->GetThing())->Color.SetValue3(Red, Green, Blue);
			return (AddAfter->Next);
			} // if
		} // else if
	else
		{
		if (Grad = new GradientCritter(this))
			{
			Grad->SetThing(new ColorTextureThing(this, ApplyToEcosys));
			return (Grad);
			} // if
		} // else
	} // if
else
	{
	return (FindNode(NewDist));
	} // else

return (NULL);

} // AnimColorGradient::AddNode

/*===========================================================================*/

int AnimColorGradient::RemoveNode(GradientCritter *RemoveMe)
{
NotifyTag Changes[2];

if (RemoveNodeSetup(RemoveMe))
	{
	if (RemoveMe->GetThing())
		delete (ColorTextureThing *)RemoveMe->GetThing();
	RemoveMe->SetThing(NULL);	// not really necessary but it feels like the right thing to do
	delete RemoveMe;

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (0);

} // AnimColorGradient::RemoveNode

/*===========================================================================*/

double AnimColorGradient::GetValue(char Channel)
{

if (Grad && Grad->GetThing())
	return (((ColorTextureThing *)Grad->GetThing())->Color.CurValue[Channel]);

return (0.0);

} // AnimColorGradient::GetValue

/*===========================================================================*/

int AnimColorGradient::GetBasicColor(unsigned char &Red, unsigned char &Green, unsigned char &Blue, double Pos)
{
double DblRed, DblGreen, DblBlue;

if (GetBasicColor(DblRed, DblGreen, DblBlue, Pos))
	{
	if (DblRed > 1.0)
		DblRed = 1.0;
	if (DblGreen > 1.0)
		DblGreen = 1.0;
	if (DblBlue > 1.0)
		DblBlue = 1.0;
	DblRed = DblRed * 255.99;
	DblGreen = DblGreen * 255.99;
	DblBlue = DblBlue * 255.99;
	Red = (unsigned char)min(255, DblRed);
	Green = (unsigned char)min(255, DblGreen);
	Blue = (unsigned char)min(255, DblBlue);
	return (1);
	} // if

return (0);

} // AnimColorGradient::GetBasicColor

/*===========================================================================*/

int AnimColorGradient::GetBasicColor(double &Red, double &Green, double &Blue, double Pos)
{
GradientCritter *CurGrad = Grad, *LastGrad = NULL;
double LastPos = 0.0, Diff, Covg[2];
ColorTextureThing *Thing[2];

Thing[0] = Thing[1] = NULL;

while (CurGrad)
	{
	if (CurGrad->Position.CurValue == Pos)
		{
		if (Thing[0] = (ColorTextureThing *)CurGrad->GetThing())
			{
			Red = Thing[0]->Color.GetCompleteValue(0);
			Green = Thing[0]->Color.GetCompleteValue(1);
			Blue = Thing[0]->Color.GetCompleteValue(2);
			return (1);
			} // if
		// returns 0 if the color is invalid
		return (0);
		} // if
	if (CurGrad->Position.CurValue > Pos)
		{
		if (LastGrad)
			{
			Thing[0] = (ColorTextureThing *)LastGrad->GetThing();
			Thing[1] = (ColorTextureThing *)CurGrad->GetThing();
			if (Thing[0] && Thing[1])
				{
				if ((Diff = CurGrad->Position.CurValue - LastPos) > 0.0)
					{
					switch (CurGrad->BlendStyle)
						{
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SHARP:	// 0% overlap
							{
							LastPos = CurGrad->Position.CurValue;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SOFT:	// 10% overlap
							{
							LastPos = LastPos + .9 * Diff;
							Diff *= .1;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_QUARTER:	// 25% overlap
							{
							LastPos = LastPos + .75 * Diff;
							Diff *= .25;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_HALF:	// 50% overlap
							{
							LastPos = LastPos + .5 * Diff;
							Diff *= .5;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_FULL:	// 100% overlap
						case WCS_GRADIENTCRITTER_BLENDSTYLE_FASTINCREASE:	// 100% overlap
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SLOWINCREASE:	// 100% overlap
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SCURVE:	// 100% overlap
							{
							// nothing to do
							break;
							} // 
						} // switch
					if (Pos > LastPos)
						{
						Covg[1] = (Pos - LastPos) / Diff;
						if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_SLOWINCREASE)
							{
							Covg[1] *= Covg[1];
							} // if
						else if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_FASTINCREASE)
							{
							Covg[1] = 1.0 - Covg[1];
							Covg[1] *= Covg[1];
							Covg[1] = 1.0 - Covg[1];
							} // if
						else if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_SCURVE)
							{
							Covg[1] *= Covg[1] * (3.0 - 2.0 * Covg[1]);	// same as PERLIN_s_curve()
							} // if
						Covg[0] = 1.0 - Covg[1];
						Red = Thing[0]->Color.GetCompleteValue(0) * Covg[0] 
							+ Thing[1]->Color.GetCompleteValue(0) * Covg[1];
						Green = Thing[0]->Color.GetCompleteValue(1) * Covg[0] 
							+ Thing[1]->Color.GetCompleteValue(1) * Covg[1];
						Blue = Thing[0]->Color.GetCompleteValue(2) * Covg[0] 
							+ Thing[1]->Color.GetCompleteValue(2) * Covg[1];
						} // if
					else
						{
						Red = Thing[0]->Color.GetCompleteValue(0);
						Green = Thing[0]->Color.GetCompleteValue(1);
						Blue = Thing[0]->Color.GetCompleteValue(2);
						} // else
					} // if
				else
					{
					Red = Thing[1]->Color.GetCompleteValue(0);
					Green = Thing[1]->Color.GetCompleteValue(1);
					Blue = Thing[1]->Color.GetCompleteValue(2);
					} // else
				return (1);
				} // if
			if (Thing[0])
				{
				Red = Thing[0]->Color.GetCompleteValue(0);
				Green = Thing[0]->Color.GetCompleteValue(1);
				Blue = Thing[0]->Color.GetCompleteValue(2);
				return (1);
				} // if
			if (Thing[1])
				{
				Red = Thing[1]->Color.GetCompleteValue(0);
				Green = Thing[1]->Color.GetCompleteValue(1);
				Blue = Thing[1]->Color.GetCompleteValue(2);
				return (1);
				} // if
			return (0);
			} // if
		else
			{
			if (Thing[0] = (ColorTextureThing *)CurGrad->GetThing())
				{
				Red = Thing[0]->Color.GetCompleteValue(0);
				Green = Thing[0]->Color.GetCompleteValue(1);
				Blue = Thing[0]->Color.GetCompleteValue(2);
				return (1);
				} // if
			return (0);
			} // else
		} // if
	LastGrad = CurGrad;
	LastPos = CurGrad->Position.CurValue;
	CurGrad = CurGrad->Next;
	} // if

// only found one critter
if (LastGrad)
	{
	if (Thing[0] = (ColorTextureThing *)LastGrad->GetThing())
		{
		Red = Thing[0]->Color.GetCompleteValue(0);
		Green = Thing[0]->Color.GetCompleteValue(1);
		Blue = Thing[0]->Color.GetCompleteValue(2);
		return (1);
		} // if
	} // if

// couldn't find a valid gradient critter
return (0);

} // AnimColorGradient::GetBasicColor

/*===========================================================================*/

int AnimColorGradient::GetNodeColor(unsigned char &Red, unsigned char &Green, unsigned char &Blue, GradientCritter *CurNode)
{
ColorTextureThing *Thing;

if (Thing = (ColorTextureThing *)CurNode->GetThing())
	{
	Red = (unsigned char)(Thing->Color.GetClampedCompleteValue(0) * 255.0);
	Green = (unsigned char)(Thing->Color.GetClampedCompleteValue(1) * 255.0);
	Blue = (unsigned char)(Thing->Color.GetClampedCompleteValue(2) * 255.0);
	return (1);
	} // if

return (0);

} // AnimColorGradient::GetNodeColor

/*===========================================================================*/

void AnimColorGradient::EditNodeColor(GradientCritter *EdNode)
{
ColorTextureThing *Thing;

if (Thing = (ColorTextureThing *)EdNode->GetThing())
	{
	Thing->Color.EditRAHost();
	} // if

} // AnimColorGradient::EditNodeColor

/*===========================================================================*/

void AnimColorGradient::SetTexturePrev(Texture *SetPrev)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		((ColorTextureThing *)CurGrad->GetThing())->SetTexturePrev(SetPrev);
	CurGrad = CurGrad->Next;
	} // while

} // AnimColorGradient::SetTexturePrev

/*===========================================================================*/

int AnimColorGradient::SetToTime(double Time)
{
int Found = 0;
GradientCritter *Current = Grad;

if (AnimGradient::SetToTime(Time))
	Found = 1;
while (Current)
	{
	if (Current->Position.SetToTime(Time))
		Found = 1;
	if (Current->GetThing())
		{
		if (((ColorTextureThing *)Current->GetThing())->SetToTime(Time))
			Found = 1;
		} // if
	Current = Current->Next;
	} // while

return (Found);

} // AnimColorGradient::SetToTime

/*===========================================================================*/

int AnimColorGradient::GetRAHostAnimated(void)
{
GradientCritter *Current = Grad;

if (AnimGradient::GetRAHostAnimated())
	return (1);
while (Current)
	{
	if (Current->Position.GetRAHostAnimated())
		return (1);
	if (Current->GetThing())
		{
		if (((ColorTextureThing *)Current->GetThing())->GetRAHostAnimated())
			return (1);
		} // if
	Current = Current->Next;
	} // while

return (0);

} // AnimColorGradient::GetRAHostAnimated

/*===========================================================================*/

bool AnimColorGradient::AnimateMaterials(void)
{
GradientCritter *Current = Grad;

if (AnimGradient::GetRAHostAnimated())
	return (true);
while (Current)
	{
	if (Current->Position.GetRAHostAnimated())
		return (true);
	if (Current->GetThing())
		{
		if (((ColorTextureThing *)Current->GetThing())->AnimateMaterials())
			return (true);
		} // if
	Current = Current->Next;
	} // while

return (false);

} // AnimColorGradient::AnimateMaterials

/*===========================================================================*/

long AnimColorGradient::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->Position.GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	CurGrad = CurGrad->Next;
	} // while

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // AnimColorGradient::GetKeyFrameRange

/*===========================================================================*/

char AnimColorGradient::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropType == WCS_RAHOST_OBJTYPE_COLORTEXTURE
	|| DropType == WCS_RAHOST_OBJTYPE_TEXTURE)
	return (1);

return (0);

} // AnimColorGradient::GetRAHostDropOK

/*===========================================================================*/

int AnimColorGradient::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int QueryResult, Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
GradientCritter *CurGrad;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (AnimColorGradient *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (AnimColorGradient *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_COLORTEXTURE
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	Success = -1;
	if (! Grad)
		{
		if (CurGrad = AddNode(0.0))
			{
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			if (CurGrad->GetThing())
				{
				if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_COLORTEXTURE)
					{
					((ColorTextureThing *)CurGrad->GetThing())->Copy(((ColorTextureThing *)CurGrad->GetThing()), (ColorTextureThing *)DropSource->DropSource);
					Changes[0] = MAKE_ID(((ColorTextureThing *)CurGrad->GetThing())->GetNotifyClass(), ((ColorTextureThing *)CurGrad->GetThing())->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, ((ColorTextureThing *)CurGrad->GetThing())->GetRAHostRoot());
					Success = 1;
					} // if
				else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
					{
					((ColorTextureThing *)CurGrad->GetThing())->Color.Copy(&((ColorTextureThing *)CurGrad->GetThing())->Color, (AnimColorTime *)DropSource->DropSource);
					Changes[0] = MAKE_ID(((ColorTextureThing *)CurGrad->GetThing())->GetNotifyClass(), ((ColorTextureThing *)CurGrad->GetThing())->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, ((ColorTextureThing *)CurGrad->GetThing())->GetRAHostRoot());
					Success = 1;
					} // if
				else
					{
					Success = ((ColorTextureThing *)CurGrad->GetThing())->NewTexture((Texture *)DropSource->DropSource) ? 1: 0;
					} // else
				} // if
			} // if
		} // if
	else
		{
		strcpy(QueryStr, "Add a new Color/Texture Combo or replace an existing one?");
		if (QueryResult = UserMessageCustom(NameStr, QueryStr, "Add New", "Cancel", "Replace", 0))
			{
			if (QueryResult == 1)
				{
				QueryStr[0] = 0;
				if (GetInputString("Enter a Position (% of Gradient Range) for new Color/Texture Combo.", WCS_REQUESTER_POSDIGITS_ONLY, QueryStr) && QueryStr[0])
					{
					if (CurGrad = AddNode(atof(QueryStr) / 100.0))
						{
						Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
						Changes[1] = NULL;
						GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
						if (CurGrad->GetThing())
							{
							if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_COLORTEXTURE)
								{
								if ((ColorTextureThing *)CurGrad->GetThing() != (ColorTextureThing *)DropSource->DropSource)
									{
									((ColorTextureThing *)CurGrad->GetThing())->Copy(((ColorTextureThing *)CurGrad->GetThing()), (ColorTextureThing *)DropSource->DropSource);
									Changes[0] = MAKE_ID(((ColorTextureThing *)CurGrad->GetThing())->GetNotifyClass(), ((ColorTextureThing *)CurGrad->GetThing())->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
									Changes[1] = NULL;
									GlobalApp->AppEx->GenerateNotify(Changes, ((ColorTextureThing *)CurGrad->GetThing())->GetRAHostRoot());
									Success = 1;
									} // if
								} // if
							if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
								{
								if (&((ColorTextureThing *)CurGrad->GetThing())->Color != (AnimColorTime *)DropSource->DropSource)
									{
									((ColorTextureThing *)CurGrad->GetThing())->Color.Copy(&((ColorTextureThing *)CurGrad->GetThing())->Color, (AnimColorTime *)DropSource->DropSource);
									Changes[0] = MAKE_ID(((ColorTextureThing *)CurGrad->GetThing())->GetNotifyClass(), ((ColorTextureThing *)CurGrad->GetThing())->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
									Changes[1] = NULL;
									GlobalApp->AppEx->GenerateNotify(Changes, ((ColorTextureThing *)CurGrad->GetThing())->GetRAHostRoot());
									Success = 1;
									} // if
								} // if
							else
								{
								if (((ColorTextureThing *)CurGrad->GetThing())->Tex != (Texture *)DropSource->DropSource)
									{
									Success = ((ColorTextureThing *)CurGrad->GetThing())->NewTexture((Texture *)DropSource->DropSource) ? 1: 0;
									} // if
								} // else
							} // if
						} // if
					} // if
				} // if
			else if (QueryResult == 2)
				{
				for (Ct = 0, CurGrad = Grad; CurGrad; CurGrad = CurGrad->Next)
					{
					if (CurGrad->GetThing())
						{
						if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_COLORTEXTURE)
							{
							if ((ColorTextureThing *)CurGrad->GetThing() != (ColorTextureThing *)DropSource->DropSource)
								{
								TargetList[Ct] = (ColorTextureThing *)CurGrad->GetThing();
								Ct ++;
								} // if
							} // if
						if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
							{
							if (&((ColorTextureThing *)CurGrad->GetThing())->Color != (AnimColorTime *)DropSource->DropSource)
								{
								TargetList[Ct] = &((ColorTextureThing *)CurGrad->GetThing())->Color;
								Ct ++;
								} // if
							} // if
						else if (((ColorTextureThing *)CurGrad->GetThing())->Tex)
							{
							if (((ColorTextureThing *)CurGrad->GetThing())->Tex != (Texture *)DropSource->DropSource)
								{
								TargetList[Ct] = ((ColorTextureThing *)CurGrad->GetThing())->Tex;
								Ct ++;
								} // if
							} // else
						} // if
					} // for
				NumListItems = Ct;
				if (! NumListItems)
					UserMessageOK(NameStr, "There are no suitable items in this gradient to replace.");
				} // else if
			} // if
		} // else
	} // else if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, NULL, NULL);
		if(GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // AnimColorGradient::ProcessRAHostDragDrop

/*===========================================================================*/

int AnimColorGradient::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

return (Success);

} // AnimColorGradient::SetRAHostProperties

/*===========================================================================*/

unsigned long AnimColorGradient::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_ANIMCOLORGRADIENT | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimColorGradient::GetRAFlags

/*===========================================================================*/

void AnimColorGradient::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = RAParent ? RAParent->GetCritterName(this): (char*)"";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_INTERFLAGS)
	{
	GetInterFlags(Prop, this);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // AnimColorGradient::GetRAHostProperties

/*===========================================================================*/

RasterAnimHost *AnimColorGradient::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;
GradientCritter *CurGrad;

if (! Current)
	Found = 1;
CurGrad = Grad;
while (CurGrad)
	{
	if (Found && CurGrad->GetThing())
		return ((ColorTextureThing *)CurGrad->GetThing());
	if (Current == CurGrad->GetThing())
		return (&CurGrad->Position);
	if (Current == &CurGrad->Position)
		Found = 1;
	CurGrad = CurGrad->Next;
	} // while

return (NULL);

} // AnimColorGradient::GetRAHostChild

/*===========================================================================*/

long AnimColorGradient::InitImageIDs(long &ImageID)
{
long NumImages = 0;
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		NumImages += ((ColorTextureThing *)CurGrad->GetThing())->InitImageIDs(ImageID);
	CurGrad = CurGrad->Next;
	} // if

return (NumImages);

} // AnimColorGradient::InitImageIDs

/*===========================================================================*/

int AnimColorGradient::InitToRender(void)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		{
		if (! ((ColorTextureThing *)CurGrad->GetThing())->InitToRender())
			return (0);
		} // if
	CurGrad = CurGrad->Next;
	} // if

return (1);

} // AnimColorGradient::InitToRender

/*===========================================================================*/

ULONG AnimColorGradient::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
GradientCritter *CurGrad, *ScanGrad, *PrevGrad = NULL, **LoadTo = &Grad;
void *TempThing;

ScanGrad = Grad;
ActiveNode = NULL;
Grad = NULL;
while (ScanGrad)
	{
	CurGrad = ScanGrad;
	ScanGrad = ScanGrad->Next;
	if (TempThing = CurGrad->GetThing())
		{
		CurGrad->SetThing(NULL);	// not really necessary but it feels like the right thing to do
		delete (ColorTextureThing *)TempThing;
		} // if
	delete CurGrad;
	} // while

while (ItemTag != WCS_PARAM_DONE)
	{
	/* read block descriptor tag from file */
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			/* read block size from file */
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} /* switch */

				switch (ItemTag & 0xffff0000)
					{
					case WCS_ANIMCOLORGRADIENT_GRADIENTCRITTER:
						{
						if (*LoadTo = new GradientCritter(this))
							{
							BytesRead = (*LoadTo)->AnimColorGradient_Load(ffile, Size, ByteFlip, this, ApplyToEcosys);
							(*LoadTo)->Prev = PrevGrad;
							PrevGrad = *LoadTo;
							LoadTo = &(*LoadTo)->Next;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} /* switch */

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} /* if size block read */
			else
				break;
			} /* if not done flag */
		} /* if tag block read */
	else
		break;
	} /* while */

return (TotalRead);

} // AnimColorGradient::Load

/*===========================================================================*/

unsigned long AnimColorGradient::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	ItemTag = WCS_ANIMCOLORGRADIENT_GRADIENTCRITTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurGrad->AnimColorGradient_Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if grad saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	CurGrad = CurGrad->Next;
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // AnimColorGradient::Save

/*===========================================================================*/

AnimMaterialGradient::AnimMaterialGradient(RasterAnimHost *RAHost, unsigned char EcosystemSource, char NewMatType)
: AnimGradient(RAHost, EcosystemSource)
{

MaterialType = NewMatType;

if (Grad = new GradientCritter(this))
	{
	Grad->SetThing(new MaterialEffect(this, NewMatType));
	} // if

} // AnimMaterialGradient::AnimMaterialGradient

/*===========================================================================*/

AnimMaterialGradient::~AnimMaterialGradient()
{
GradientCritter *CurGrad, *ScanGrad = Grad;
void *TempThing;

Grad = NULL;
ActiveNode = NULL;
while (ScanGrad)
	{
	CurGrad = ScanGrad;
	ScanGrad = ScanGrad->Next;
	if (TempThing = CurGrad->GetThing())
		{
		CurGrad->SetThing(NULL);	// not really necessary but it feels like the right thing to do
		delete (MaterialEffect *)TempThing;
		} // if
	delete CurGrad;
	} // while

} // AnimMaterialGradient::~AnimMaterialGradient

/*===========================================================================*/

void AnimMaterialGradient::Copy(AnimMaterialGradient *CopyTo, AnimMaterialGradient *CopyFrom)
{
GradientCritter *NextGrad, *ScanGrad, *ToGrad = NULL;
void *TempThing;

CopyTo->ActiveNode = NULL;
ScanGrad = CopyTo->Grad;
CopyTo->Grad = NULL;
while (ScanGrad)
	{
	NextGrad = ScanGrad->Next;
	if (TempThing = ScanGrad->GetThing())
		{
		ScanGrad->SetThing(NULL);
		delete (MaterialEffect *)TempThing;
		} // if
	delete ScanGrad;
	ScanGrad = NextGrad;
	} // while
if (NextGrad = CopyFrom->Grad)
	{
	if (CopyTo->Grad = new GradientCritter(CopyTo))
		{
		ToGrad = CopyTo->Grad;
		ToGrad->Copy(ToGrad, NextGrad);
		if (NextGrad->GetThing())
			{
			ToGrad->SetThing(new MaterialEffect(CopyTo, CopyTo->MaterialType));
			if (ToGrad->GetThing())
				{
				((MaterialEffect *)ToGrad->GetThing())->Copy((MaterialEffect *)ToGrad->GetThing(), (MaterialEffect *)NextGrad->GetThing());
				} // if
			} // if
		NextGrad = NextGrad->Next;
		} // if
	while (NextGrad && ToGrad)
		{
		if (ToGrad->Next = new GradientCritter(CopyTo))
			{
			ToGrad->Next->Prev = ToGrad;
			ToGrad = ToGrad->Next;
			ToGrad->Copy(ToGrad, NextGrad);
			if (NextGrad->GetThing())
				{
				ToGrad->SetThing(new MaterialEffect(CopyTo, CopyTo->MaterialType));
				if (ToGrad->GetThing())
					{
					((MaterialEffect *)ToGrad->GetThing())->Copy((MaterialEffect *)ToGrad->GetThing(), (MaterialEffect *)NextGrad->GetThing());
					} // if
				} // if
			} // if
		NextGrad = NextGrad->Next; 
		} // while
	} // if

} // AnimMaterialGradient::Copy

/*===========================================================================*/

GradientCritter *AnimMaterialGradient::AddNode(double NewDist, int Interpolate)
{
GradientCritter *AddBefore = NULL, *AddAfter = NULL, *Stash;

if (AddNodeSetup(NewDist, AddBefore, AddAfter))
	{
	if (AddBefore)
		{
		Stash = AddBefore->Prev;
		if (AddBefore->Prev = new GradientCritter(this))
			{
			AddBefore->Prev->SetThing(new MaterialEffect(this, MaterialType));
			AddBefore->Prev->SetDistance(NewDist);
			AddBefore->Prev->Prev = Stash;
			if (Stash)
				Stash->Next = AddBefore->Prev;
			AddBefore->Prev->Next = AddBefore;
			if (! Stash)
				Grad = AddBefore->Prev;
			return (AddBefore->Prev);
			} // if
		} // if
	else if (AddAfter)
		{
		Stash = AddAfter->Next;
		if (AddAfter->Next = new GradientCritter(this))
			{
			AddAfter->Next->SetThing(new MaterialEffect(this, MaterialType));
			AddAfter->Next->SetDistance(NewDist);
			AddAfter->Next->Next = Stash;
			if (Stash)
				Stash->Prev = AddAfter->Next;
			AddAfter->Next->Prev = AddAfter;
			return (AddAfter->Next);
			} // if
		} // else if
	else
		{
		if (Grad = new GradientCritter(this))
			{
			Grad->SetThing(new MaterialEffect(this, MaterialType));
			return (Grad);
			} // if
		} // else
	} // if
else
	{
	return (FindNode(NewDist));
	} // else

return (NULL);

} // AnimMaterialGradient::AddNode

/*===========================================================================*/

int AnimMaterialGradient::RemoveNode(GradientCritter *RemoveMe)
{
NotifyTag Changes[2];

if (RemoveNodeSetup(RemoveMe))
	{
	if (RemoveMe->GetThing())
		delete (MaterialEffect *)RemoveMe->GetThing();
	RemoveMe->SetThing(NULL);	// not really necessary but it feels like the right thing to do
	delete RemoveMe;

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (0);

} // AnimMaterialGradient::RemoveNode

/*===========================================================================*/

int AnimMaterialGradient::GetBasicColor(unsigned char &Red, unsigned char &Green, unsigned char &Blue, double Pos)
{
GradientCritter *CurGrad = Grad, *LastGrad = NULL;
double LastPos = 0.0, Diff, Covg[2];
MaterialEffect *Thing[2];

Thing[0] = Thing[1] = NULL;

while (CurGrad)
	{
	if (CurGrad->Position.CurValue == Pos)
		{
		if (Thing[0] = (MaterialEffect *)CurGrad->GetThing())
			{
			Red = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(0) * 255.0);
			Green = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(1) * 255.0);
			Blue = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(2) * 255.0);
			return (1);
			} // if
		// returns 0 if the color is invalid
		return (0);
		} // if
	if (CurGrad->Position.CurValue > Pos)
		{
		if (LastGrad)
			{
			Thing[0] = (MaterialEffect *)LastGrad->GetThing();
			Thing[1] = (MaterialEffect *)CurGrad->GetThing();
			if (Thing[0] && Thing[1])
				{
				if ((Diff = CurGrad->Position.CurValue - LastPos) > 0.0)
					{
					switch (CurGrad->BlendStyle)
						{
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SHARP:	// 0% overlap
							{
							LastPos = CurGrad->Position.CurValue;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SOFT:	// 10% overlap
							{
							LastPos = LastPos + .9 * Diff;
							Diff *= .1;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_QUARTER:	// 25% overlap
							{
							LastPos = LastPos + .75 * Diff;
							Diff *= .25;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_HALF:	// 50% overlap
							{
							LastPos = LastPos + .5 * Diff;
							Diff *= .5;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_FULL:	// 100% overlap
						case WCS_GRADIENTCRITTER_BLENDSTYLE_FASTINCREASE:	// 100% overlap
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SLOWINCREASE:	// 100% overlap
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SCURVE:	// 100% overlap
							{
							// nothing to do
							break;
							} // 
						} // switch
					if (Pos > LastPos)
						{
						Covg[1] = (Pos - LastPos) / Diff;
						if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_SLOWINCREASE)
							{
							Covg[1] *= Covg[1];
							} // if
						else if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_FASTINCREASE)
							{
							Covg[1] = 1.0 - Covg[1];
							Covg[1] *= Covg[1];
							Covg[1] = 1.0 - Covg[1];
							} // if
						else if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_SCURVE)
							{
							Covg[1] *= Covg[1] * (3.0 - 2.0 * Covg[1]);	// same as PERLIN_s_curve()
							} // if
						Covg[0] = 1.0 - Covg[1];
						Red = (unsigned char)((Thing[0]->DiffuseColor.GetClampedCompleteValue(0) * Covg[0] 
							+ Thing[1]->DiffuseColor.GetClampedCompleteValue(0) * Covg[1]) * 255.0);
						Green = (unsigned char)((Thing[0]->DiffuseColor.GetClampedCompleteValue(1) * Covg[0] 
							+ Thing[1]->DiffuseColor.GetClampedCompleteValue(1) * Covg[1]) * 255.0);
						Blue = (unsigned char)((Thing[0]->DiffuseColor.GetClampedCompleteValue(2) * Covg[0] 
							+ Thing[1]->DiffuseColor.GetClampedCompleteValue(2) * Covg[1]) * 255.0);
						} // if
					else
						{
						Red = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(0) * 255.0);
						Green = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(1) * 255.0);
						Blue = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(2) * 255.0);
						} // else
					} // if
				else
					{
					Red = (unsigned char)(Thing[1]->DiffuseColor.GetClampedCompleteValue(0) * 255.0);
					Green = (unsigned char)(Thing[1]->DiffuseColor.GetClampedCompleteValue(1) * 255.0);
					Blue = (unsigned char)(Thing[1]->DiffuseColor.GetClampedCompleteValue(2) * 255.0);
					} // else
				return (1);
				} // if
			if (Thing[0])
				{
				Red = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(0) * 255.0);
				Green = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(1) * 255.0);
				Blue = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(2) * 255.0);
				return (1);
				} // if
			if (Thing[1])
				{
				Red = (unsigned char)(Thing[1]->DiffuseColor.GetClampedCompleteValue(0) * 255.0);
				Green = (unsigned char)(Thing[1]->DiffuseColor.GetClampedCompleteValue(1) * 255.0);
				Blue = (unsigned char)(Thing[1]->DiffuseColor.GetClampedCompleteValue(2) * 255.0);
				return (1);
				} // if
			return (0);
			} // if
		else
			{
			if (Thing[0] = (MaterialEffect *)CurGrad->GetThing())
				{
				Red = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(0) * 255.0);
				Green = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(1) * 255.0);
				Blue = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(2) * 255.0);
				return (1);
				} // if
			return (0);
			} // else
		} // if
	LastGrad = CurGrad;
	LastPos = CurGrad->Position.CurValue;
	CurGrad = CurGrad->Next;
	} // if

// only found one critter
if (LastGrad)
	{
	if (Thing[0] = (MaterialEffect *)LastGrad->GetThing())
		{
		Red = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(0) * 255.0);
		Green = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(1) * 255.0);
		Blue = (unsigned char)(Thing[0]->DiffuseColor.GetClampedCompleteValue(2) * 255.0);
		return (1);
		} // if
	} // if

// couldn't find a valid gradient critter
return (0);

} // AnimMaterialGradient::GetBasicColor

/*===========================================================================*/

int AnimMaterialGradient::GetNodeColor(unsigned char &Red, unsigned char &Green, unsigned char &Blue, GradientCritter *CurNode)
{
MaterialEffect *Thing;

if (Thing = (MaterialEffect *)CurNode->GetThing())
	{
	Red = (unsigned char)(Thing->DiffuseColor.GetClampedCompleteValue(0) * 255.0);
	Green = (unsigned char)(Thing->DiffuseColor.GetClampedCompleteValue(1) * 255.0);
	Blue = (unsigned char)(Thing->DiffuseColor.GetClampedCompleteValue(2) * 255.0);
	return (1);
	} // if

return (0);

} // AnimMaterialGradient::GetNodeColor

/*===========================================================================*/

void AnimMaterialGradient::EditNodeColor(GradientCritter *EdNode)
{
MaterialEffect *Thing;

if (Thing = (MaterialEffect *)EdNode->GetThing())
	{
	Thing->DiffuseColor.EditRAHost();
	} // if

} // AnimMaterialGradient::EditNodeColor

/*===========================================================================*/

int AnimMaterialGradient::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{
GradientCritter *Current = Grad;

while (Current)
	{
	if (Current->GetThing())
		{
		if (! ((MaterialEffect *)Current->GetThing())->FindnRemove3DObjects(RemoveMe))
			return (0);
		} // if
	Current = Current->Next;
	} // while

return (1);

} // AnimMaterialGradient::FindnRemove3DObjects

/*===========================================================================*/

int AnimMaterialGradient::SetToTime(double Time)
{
int Found = 0;
GradientCritter *Current = Grad;

if (AnimGradient::SetToTime(Time))
	Found = 1;
while (Current)
	{
	if (Current->Position.SetToTime(Time))
		Found = 1;
	if (Current->GetThing())
		{
		if (((MaterialEffect *)Current->GetThing())->SetToTime(Time))
			Found = 1;
		} // if
	Current = Current->Next;
	} // while

return (Found);

} // AnimMaterialGradient::SetToTime

/*===========================================================================*/

char *AnimMaterialGradient::GetCritterName(RasterAnimHost *Test)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (Test == CurGrad->GetThing())
		return (((MaterialEffect *)CurGrad->GetThing())->GetName());
	if (Test == &CurGrad->Position)
		return ("Gradient Position");
	CurGrad = CurGrad->Next;
	} // while

return ("");

} // AnimMaterialGradient::GetCritterName

/*===========================================================================*/

int AnimMaterialGradient::AnimateEcoShadows(void)
{
GradientCritter *Current = Grad;

if (IsAnimated() > 1)
	return (1);
while (Current)
	{
	if (Current->GetThing())
		{
		if (Current->IsAnimated() > 1)
			return (1);
		if (((MaterialEffect *)Current->GetThing())->GetEnabled() && 
			((MaterialEffect *)Current->GetThing())->AnimateEcoShadows())
			return (1);
		} // if
	Current = Current->Next;
	} // while

return (0);

} // AnimMaterialGradient::AnimateEcoShadows

/*===========================================================================*/

int AnimMaterialGradient::Animate3DShadows(void)
{
GradientCritter *Current = Grad;

if (IsAnimated() > 1)
	return (1);
while (Current)
	{
	if (Current->GetThing())
		{
		if (Current->IsAnimated() > 1)
			return (1);
		if (((MaterialEffect *)Current->GetThing())->GetEnabled() && 
			((MaterialEffect *)Current->GetThing())->AnimateShadow3D())
			return (1);
		} // if
	Current = Current->Next;
	} // while

return (0);

} // AnimMaterialGradient::Animate3DShadows

/*===========================================================================*/

int AnimMaterialGradient::GetRAHostAnimated(void)
{
GradientCritter *Current = Grad;

if (AnimGradient::GetRAHostAnimated())
	return (1);
while (Current)
	{
	if (Current->Position.GetRAHostAnimated())
		return (1);
	if (Current->GetThing())
		{
		if (((MaterialEffect *)Current->GetThing())->GetRAHostAnimated())
			return (1);
		} // if
	Current = Current->Next;
	} // while

return (0);

} // AnimMaterialGradient::GetRAHostAnimated

/*===========================================================================*/

bool AnimMaterialGradient::AnimateMaterials(void)
{
GradientCritter *Current = Grad;

if (AnimGradient::GetRAHostAnimated())
	return (true);
while (Current)
	{
	if (Current->Position.GetRAHostAnimated())
		return (true);
	if (Current->GetThing())
		{
		if (((MaterialEffect *)Current->GetThing())->AnimateMaterials())
			return (true);
		} // if
	Current = Current->Next;
	} // while

return (false);

} // AnimMaterialGradient::AnimateMaterials

/*===========================================================================*/

long AnimMaterialGradient::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->Position.GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	if (CurGrad->GetThing() && ((MaterialEffect *)CurGrad->GetThing())->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	CurGrad = CurGrad->Next;
	} // while

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // AnimMaterialGradient::GetKeyFrameRange

/*===========================================================================*/

char AnimMaterialGradient::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (DropType == WCS_EFFECTSSUBCLASS_MATERIAL
	|| DropType == WCS_EFFECTSSUBCLASS_WAVE
	|| DropType == WCS_RAHOST_OBJTYPE_MATERIALSTRATA
	|| DropType == WCS_RAHOST_OBJTYPE_ECOTYPE
	|| DropType == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP
	|| DropType == WCS_RAHOST_OBJTYPE_FOLIAGE)
	return (1);

return (0);

} // AnimMaterialGradient::GetRAHostDropOK

/*===========================================================================*/

int AnimMaterialGradient::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int QueryResult, Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
GradientCritter *CurGrad;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (AnimMaterialGradient *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (AnimMaterialGradient *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL)
	{
	Success = -1;
	if (! Grad)
		{
		if (CurGrad = AddNode(0.0))
			{
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			if (CurGrad->GetThing())
				{
				((MaterialEffect *)CurGrad->GetThing())->Copy((MaterialEffect *)CurGrad->GetThing(), (MaterialEffect *)DropSource->DropSource);
				Changes[0] = MAKE_ID(((MaterialEffect *)CurGrad->GetThing())->GetNotifyClass(), ((MaterialEffect *)CurGrad->GetThing())->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, ((MaterialEffect *)CurGrad->GetThing())->GetRAHostRoot());
				} // if
			} // if
		} // if
	else
		{
		strcpy(QueryStr, "Add a new Material or replace an existing one?");
		if (QueryResult = UserMessageCustom(NameStr, QueryStr, "Add New", "Cancel", "Replace", 0))
			{
			if (QueryResult == 1)
				{
				QueryStr[0] = 0;
				if (GetInputString("Enter a Position (% of Gradient Range) for new Material.", WCS_REQUESTER_POSDIGITS_ONLY, QueryStr) && QueryStr[0])
					{
					if (CurGrad = AddNode(atof(QueryStr) / 100.0))
						{
						Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
						Changes[1] = NULL;
						GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
						if (CurGrad->GetThing())
							{
							((MaterialEffect *)CurGrad->GetThing())->Copy((MaterialEffect *)CurGrad->GetThing(), (MaterialEffect *)DropSource->DropSource);
							Changes[0] = MAKE_ID(((MaterialEffect *)CurGrad->GetThing())->GetNotifyClass(), ((MaterialEffect *)CurGrad->GetThing())->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
							Changes[1] = NULL;
							GlobalApp->AppEx->GenerateNotify(Changes, ((MaterialEffect *)CurGrad->GetThing())->GetRAHostRoot());
							} // if
						} // if
					} // if
				} // if
			else if (QueryResult == 2)
				{
				for (Ct = 0, CurGrad = Grad; CurGrad; CurGrad = CurGrad->Next)
					{
					if (CurGrad->GetThing())
						{
						if ((MaterialEffect *)CurGrad->GetThing() != (MaterialEffect *)DropSource->DropSource)
							{
							TargetList[Ct] = (MaterialEffect *)CurGrad->GetThing();
							Ct ++;
							} // if
						} // if
					} // for
				NumListItems = Ct;
				if (! NumListItems)
					UserMessageOK(NameStr, "There are no suitable Materials in this gradient to replace.");
				} // else if
			} // if
		} // else
	} // else if
else
	{
	for (Ct = 0, CurGrad = Grad; CurGrad; CurGrad = CurGrad->Next)
		{
		if (CurGrad->GetThing())
			{
			if ((MaterialEffect *)CurGrad->GetThing() != (MaterialEffect *)DropSource->DropSource)
				{
				TargetList[Ct] = (MaterialEffect *)CurGrad->GetThing();
				Ct ++;
				} // if
			} // if
		} // for
	NumListItems = Ct;
	if (! NumListItems)
		UserMessageOK(NameStr, "There are no suitable Materials in this gradient to receive dropped item.");
	} // else

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, NULL, NULL);
		if(GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // AnimMaterialGradient::ProcessRAHostDragDrop

/*===========================================================================*/

int AnimMaterialGradient::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

return (Success);

} // AnimMaterialGradient::SetRAHostProperties

/*===========================================================================*/

unsigned long AnimMaterialGradient::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_ANIMMATERIALGRADIENT | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AnimMaterialGradient::GetRAFlags

/*===========================================================================*/

void AnimMaterialGradient::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = RAParent ? RAParent->GetCritterName(this): (char*)"";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_INTERFLAGS)
	{
	GetInterFlags(Prop, this);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // AnimMaterialGradient::GetRAHostProperties

/*===========================================================================*/

RasterAnimHost *AnimMaterialGradient::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;
GradientCritter *CurGrad;

if (! Current)
	Found = 1;
CurGrad = Grad;
while (CurGrad)
	{
	if (Found && CurGrad->GetThing())
		return ((MaterialEffect *)CurGrad->GetThing());
	if (Current == CurGrad->GetThing())
		return (&CurGrad->Position);
	if (Current == &CurGrad->Position)
		Found = 1;
	CurGrad = CurGrad->Next;
	} // while

return (NULL);

} // AnimMaterialGradient::GetRAHostChild

/*===========================================================================*/

long AnimMaterialGradient::InitImageIDs(long &ImageID)
{
long NumImages = 0;
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		NumImages += ((MaterialEffect *)CurGrad->GetThing())->InitImageIDs(ImageID);
	CurGrad = CurGrad->Next;
	} // if

return (NumImages);

} // AnimMaterialGradient::InitImageIDs

/*===========================================================================*/

int AnimMaterialGradient::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		{
		if (! ((MaterialEffect *)CurGrad->GetThing())->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
			return (0);
		} // if
	CurGrad = CurGrad->Next;
	} // if

return (1);

} // AnimMaterialGradient::BuildFileComponentsList

/*===========================================================================*/

int AnimMaterialGradient::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		{
		if (! ((MaterialEffect *)CurGrad->GetThing())->InitToRender(Opt, Buffers))
			return (0);
		} // if
	CurGrad = CurGrad->Next;
	} // if

return (1);

} // AnimMaterialGradient::InitToRender

/*===========================================================================*/

int AnimMaterialGradient::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		{
		if (! ((MaterialEffect *)CurGrad->GetThing())->InitFrameToRender(Lib, Rend))
			return (0);
		} // if
	CurGrad = CurGrad->Next;
	} // if

return (1);

} // AnimMaterialGradient::InitFrameToRender

/*===========================================================================*/

int AnimMaterialGradient::IsThereOpticallyTransparentMaterial(void)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		{
		if (((MaterialEffect *)CurGrad->GetThing())->IsThereOpticallyTransparentMaterial())
			return (1);
		} // if
	CurGrad = CurGrad->Next;
	} // if

return (0);

} // AnimMaterialGradient::IsThereOpticallyTransparentMaterial

/*===========================================================================*/

int AnimMaterialGradient::IsThereTransparentMaterial(void)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		{
		if (((MaterialEffect *)CurGrad->GetThing())->IsThereTransparentMaterial())
			return (1);
		} // if
	CurGrad = CurGrad->Next;
	} // if

return (0);

} // AnimMaterialGradient::IsThereTransparentMaterial

/*===========================================================================*/

int AnimMaterialGradient::AreThereEnabledTextures(void)
{
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		{
		if (((MaterialEffect *)CurGrad->GetThing())->AreThereEnabledTextures())
			return (1);
		} // if
	CurGrad = CurGrad->Next;
	} // if

return (0);

} // AnimMaterialGradient::AreThereEnabledTextures

/*===========================================================================*/

int AnimMaterialGradient::AreTexturesTileable(double &TileWidth, double &TileHeight, double &TileCenterX, double &TileCenterY)
{
double CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY;
GradientCritter *CurGrad = Grad;
int OneTileDone = 0, Tileable = 1;

TileWidth = TileHeight = 1.0;
TileCenterX = TileCenterY = 0.0;

while (CurGrad && Tileable)
	{
	if (CurGrad->GetThing())
		{
		if (Tileable = ((MaterialEffect *)CurGrad->GetThing())->AreTexturesTileable(CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY))
			{
			if (OneTileDone)
				{
				if (fabs(CurTileCenterX - TileCenterX) <= .01 && fabs(CurTileCenterY - TileCenterY) <= .01)
					{
					// find common size
					if (fabs(CurTileWidth - TileWidth) > .01 || fabs(CurTileHeight - TileHeight) > .01)
						{
						TileWidth = WCS_lcm(TileWidth, CurTileWidth, .05);
						TileHeight = WCS_lcm(TileHeight, CurTileHeight, .05);
						} // if
					} // if
				else
					Tileable = 0;
				} // if
			else
				{
				TileCenterX = CurTileCenterX;
				TileCenterY = CurTileCenterY;
				TileWidth = CurTileWidth;
				TileHeight = CurTileHeight;
				OneTileDone = 1;
				} // else
			} // if
		} // if
	CurGrad = CurGrad->Next;
	} // if

return (Tileable);

} // AnimMaterialGradient::AreTexturesTileable

/*===========================================================================*/

void AnimMaterialGradient::GetWaterDepthAndWaveRange(double &MaximumMod, double &MinimumMod)
{
GradientCritter *CurGrad = Grad;
double TestMaximumMod, TestMinimumMod;

MaximumMod = MinimumMod = 0.0;

while (CurGrad)
	{
	if (CurGrad->GetThing())
		{
		TestMaximumMod = TestMinimumMod = 0.0;
		((MaterialEffect *)CurGrad->GetThing())->GetWaterDepthAndWaveRange(TestMaximumMod, TestMinimumMod);
		if (TestMaximumMod > MaximumMod)
			MaximumMod = TestMaximumMod;
		if (TestMinimumMod < MinimumMod)
			MinimumMod = TestMinimumMod;
		} // if
	CurGrad = CurGrad->Next;
	} // if

} // AnimMaterialGradient::GetWaterDepthAndWaveRange

/*===========================================================================*/

ULONG AnimMaterialGradient::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
GradientCritter *CurGrad, *ScanGrad, *PrevGrad = NULL, **LoadTo = &Grad;
void *TempThing;

ScanGrad = Grad;
Grad = NULL;
ActiveNode = NULL;
while (ScanGrad)
	{
	CurGrad = ScanGrad;
	ScanGrad = ScanGrad->Next;
	if (TempThing = CurGrad->GetThing())
		{
		CurGrad->SetThing(NULL);	// not really necessary but it feels like the right thing to do
		delete (MaterialEffect *)TempThing;
		} // if
	delete CurGrad;
	} // while

while (ItemTag != WCS_PARAM_DONE)
	{
	/* read block descriptor tag from file */
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			/* read block size from file */
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} /* switch */

				switch (ItemTag & 0xffff0000)
					{
					case WCS_ANIMMATERIALGRADIENT_GRADIENTCRITTER:
						{
						if (*LoadTo = new GradientCritter(this))
							{
							BytesRead = (*LoadTo)->AnimMaterialGradient_Load(ffile, Size, ByteFlip, this, MaterialType);
							(*LoadTo)->Prev = PrevGrad;
							PrevGrad = *LoadTo;
							LoadTo = &(*LoadTo)->Next;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} /* switch */

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} /* if size block read */
			else
				break;
			} /* if not done flag */
		} /* if tag block read */
	else
		break;
	} /* while */

return (TotalRead);

} // AnimMaterialGradient::Load

/*===========================================================================*/

unsigned long AnimMaterialGradient::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
GradientCritter *CurGrad = Grad;

while (CurGrad)
	{
	ItemTag = WCS_ANIMMATERIALGRADIENT_GRADIENTCRITTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurGrad->AnimMaterialGradient_Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if grad saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	CurGrad = CurGrad->Next;
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // AnimMaterialGradient::Save

/*===========================================================================*/
/*===========================================================================*/
