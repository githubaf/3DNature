// GRWidget.cpp
// Code for TimeLine Widget
// Built from V1 TimeLineSupport.c on 5/25/96 by CXH and GRH
// Original code ** By Gary R. Huber, 1994.
//

#include "stdafx.h"
#include "WCSWidgets.h"
#include "GraphWidget.h"
#include "Notify.h"
#include "Types.h"
#include "Useful.h"
#include "WidgetSupport.h"
#include "GraphData.h"
#include "Fenetre.h"

static char GraphStr[40];

#ifdef _WIN32
long WINAPI AnimGraphWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
ULONG GR_Draw(NativeControl hwnd, HDC MyDC, struct GRData *data);
ULONG GR_HandleInput(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
#endif // _WIN32

/*===========================================================================*/

void ConfigureGR(NativeControl Dest, int Target, struct GRData *NewData)
{
NativeControl DestWidget;
GUIFenetre *GF;

if(GF = (GUIFenetre *)GetWindowLong(Dest, GWL_USERDATA))
	{
	if(DestWidget = GF->GetWidgetFromID(Target))
		{
		ConfigureGR(DestWidget, NewData);
		} // if
	} // if

} // ConfigureGR

/*===========================================================================*/

void ConfigureGR(NativeControl Dest, struct GRData *NewData)
{
#ifdef _WIN32
SetWindowLong(Dest, WCSW_GR_OFF_GRDATA, (long)NewData);
InvalidateRect(Dest, NULL, NULL);
#endif //_WIN32
} // ConfigureGR

/*===========================================================================*/

/*
** Draw method is called whenever MUI feels we should render
** our object. This usually happens after layout is finished
** or when we need to refresh in a simplerefresh window.
** Note: You may only render within the rectangle
**       _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj).
*/

#ifdef _WIN32
ULONG GR_Draw(NativeControl hwnd, HDC MyDC, struct GRData *data)
#endif // _WIN32
{
double X[2], Y[2], TempX[2], TempY[2], Value, Dist;
long Frame;
short halftextht, textht, textkludge = 0, SelectedColorOn, textwidthlabel;
short mright, mtop, mbottom, mleft;
GraphNode *MyNode;
#ifdef _WIN32
RECT RCT;
#endif // _WIN32
void *DrawPort;
VectorClipper VC;
char j;

#ifdef _WIN32
GetWindowRect(hwnd, &RCT);
mleft = 0; mtop = 0;
mright = (short)(RCT.right - RCT.left);
mbottom = (short)(RCT.bottom - RCT.top);
DrawPort = MyDC;
#endif // _WIN32

/* compute values for redraw */
if (data->DistancePrototype)
	{
	textht = (short)WCSW_GetTextHeight(DrawPort, "A", 1);
	halftextht = textht / 2;
	#ifdef _WIN32
	textkludge = textht;
	#endif // _WIN32
	} // if
else
	{
	textht = halftextht = 0;
	} // else

data->right 	 = mright - 4;
data->top 		 = mtop + 2 + halftextht;
data->bottom 	 = mbottom - 4 - textht;
data->textbottom = mbottom - 2;

// find the visible distance range
Value = data->HVisible * data->MaxDistRange / 100.0;	// Value is the visible distance	// <<<>>> Distance
data->LowDrawDist = data->HPan * (data->MaxDistRange - Value) / 100.0;
data->HighDrawDist = data->LowDrawDist + Value;

// find an appropriate distance grid interval
if (data->Crit->TimeDude())
	{
	Frame = quickftol(Value * data->FrameRate);
	if ( (Frame) > 5000) data->DistGridInt = 500 / data->FrameRate;
	else if ( (Frame) > 1000) data->DistGridInt = 100 / data->FrameRate;
	else if ( (Frame) > 500) data->DistGridInt = 50 / data->FrameRate;
	else if ( (Frame) > 100) data->DistGridInt = 10 / data->FrameRate;
	else if ( (Frame) > 50) data->DistGridInt = 5 / data->FrameRate;
	else data->DistGridInt = 1 / data->FrameRate;
	data->DistInterval = 1.0 / data->FrameRate;
	} // if
else
	{
	if ( (Value) > 5000) data->DistGridInt = 500.0;
	else if ( (Value) > 1000) data->DistGridInt = 100.0;
	else if ( (Value) > 500) data->DistGridInt = 50.0;
	else if ( (Value) > 100) data->DistGridInt = 10.0;
	else if ( (Value) > 50) data->DistGridInt = 5.0;
	else if ( (Value) > 10) data->DistGridInt = 1.0;
	else if ( (Value) > 5) data->DistGridInt = .5;
	else data->DistGridInt = .1;
	data->DistInterval = data->DistGridInt / 5.0;
	} // else
if (data->DisplayByFrame)
	{
	data->TextLowDrawDist = data->LowDrawDist;// * data->FrameRate;
	data->TextHighDrawDist = data->HighDrawDist;// * data->FrameRate;
	} // if
else
	{
	data->TextLowDrawDist = data->LowDrawDist;
	data->TextHighDrawDist = data->HighDrawDist;
	} // else

// find the base distance grid which will be a dark grid line
data->DistGridBase = WCS_floor(data->LowDrawDist / (data->DistGridInt * data->DistGridLg)) * data->DistGridInt * data->DistGridLg;


Value = data->VVisible * fabs(data->MaxHighDrawVal - data->MinLowDrawVal) / 100.0;	// Value is the visible height <<<>>> Value
data->LowDrawVal = data->MinLowDrawVal + data->VPan * fabs(data->MaxHighDrawVal - data->MinLowDrawVal - Value) / 100.0;
//data->LowDrawVal = data->MinLowDrawVal + data->VPan * (data->MaxHighDrawVal - Value) / 100.0;
data->HighDrawVal = data->LowDrawVal + Value;
// values may still be equal which causes divide by 0
if (data->HighDrawVal == data->LowDrawVal)
	data->HighDrawVal = data->LowDrawVal + fabs(data->LowDrawVal) * .001;

// find the appropriate value grid interval
Value = fabs(data->HighDrawVal - data->LowDrawVal);
if ( (Value) > 10000.0f)
	data->ValGridInt = 5000.0f;
else if ( (Value) > 5000.0f)
	data->ValGridInt = 1000.0f;
else if ( (Value) > 1000.0f)
	data->ValGridInt = 500.0f;
else if ( (Value) > 500.0f)
	data->ValGridInt = 100.0f;
else if ( (Value) > 100.0f)
	data->ValGridInt = 50.0f;
else if ( (Value) > 50.0f)
	data->ValGridInt = 10.0f;
else if ( (Value) > 10.0f)
	data->ValGridInt = 5.0f;
else if ( (Value) > 5.0f)
	data->ValGridInt = 1.0f;
else if ( (Value) > 1.0f)
	data->ValGridInt = .5f;
else if ( (Value) > .5f)
	data->ValGridInt = .1f;
else if ( (Value) > .1f)
	data->ValGridInt = .05f;
else
	data->ValGridInt = .01f;

// find the base value grid which will be the first visible grid line
data->ValGridBase = ceil(data->LowDrawVal / data->ValGridInt) * data->ValGridInt;

if (data->ValuePrototype)
	{
	FormatAsPreferredUnit(GraphStr, data->ValuePrototype, data->HighDrawVal);
	data->textwidthtop 	= (short)WCSW_GetTextLength(DrawPort, GraphStr, (int)strlen(GraphStr));

	FormatAsPreferredUnit(GraphStr, data->ValuePrototype, data->LowDrawVal);
	data->textwidthbottom = (short)WCSW_GetTextLength(DrawPort, GraphStr, (int)strlen(GraphStr));
	data->textwidthzero = (short)WCSW_GetTextLength(DrawPort, "0.00", 4);
	} // if
else
	{
	data->textwidthtop = data->textwidthbottom = data->textwidthzero = 0;
	} // else

if (data->Crit->GetType() == WCS_ANIMCRITTER_TYPE_DOUBLECURVE && ((AnimDoubleCurve *)data->Crit)->YLabel[0])
	{
	textwidthlabel = (short)WCSW_GetTextLength(DrawPort, ((AnimDoubleCurve *)data->Crit)->YLabel, (int)strlen(((AnimDoubleCurve *)data->Crit)->YLabel));
	} // if
else
	textwidthlabel = 0;

data->left 		= max(data->textwidthtop, data->textwidthbottom);
data->left 		= mleft	+ 4 + max(data->left, textwidthlabel);
data->PixelsPerDist = ((double)data->right - data->left) / (data->HighDrawDist - data->LowDrawDist);
data->PixelsPerDistGrid = (double)data->DistGridInt * data->PixelsPerDist;
data->PixelsPerVal = ((double)data->bottom - data->top) / (data->HighDrawVal - data->LowDrawVal);
data->PixelsPerValGrid = data->ValGridInt * data->PixelsPerVal;
data->textzero = (short)(data->bottom - (-data->LowDrawVal) * data->PixelsPerVal);

/*
** if WCSW_GRW_MADF_DRAWOBJECT isn't set, we shouldn't draw anything.
** MUI just wanted to update the frame or something like that.
*/

if (WCSW_WIDGET_FLAGS & WCSW_GRW_MADF_DRAWUPDATE) // called from our input method 
	{
	// draw key frame points 
	WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_WHITE);
	SelectedColorOn = 0;
	for (j = 0; j < data->NumGraphs; j++)
		{
		for (MyNode = data->Crit->GetFirstNode(j); MyNode; MyNode = data->Crit->GetNextNode(j, MyNode))
			{
			Dist = MyNode->GetDistance();// <<<>>> Distance
			if (MyNode->GetSelectedState())
				{
				if (! SelectedColorOn)
					{
					WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_YELLOW);
					SelectedColorOn = 1;
					} // if
				} // if
			else if (SelectedColorOn)
				{
				WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_WHITE);
				SelectedColorOn = 0;
				} // else if
			X[0] = data->left + data->PixelsPerDist * (Dist - data->LowDrawDist);
			Y[0] = (double)(data->bottom - data->PixelsPerVal * (MyNode->GetValue() - data->LowDrawVal));// <<<>>> Value
			if (X[0] >= data->left && X[0] <= data->right + 1
				&& Y[0] > data->top && Y[0] <= data->bottom)
				{
				long x0, y0;

				x0 = quickftol(X[0]);
				y0 = quickftol(Y[0]);
				WCSW_Move(DrawPort, x0, y0 - 3);
				WCSW_Draw(DrawPort, x0 + 3, y0 + 2);
				WCSW_Draw(DrawPort, x0 - 3, y0 + 2);
				WCSW_Draw(DrawPort, x0, y0 - 3);
				} // if in bounds 
			} // for draw key frame triangles 
		} // for
	} // if update only 

if (WCSW_WIDGET_FLAGS & WCSW_GRW_MADF_DRAWOBJECT)
	{
	// draw outline box and scales 
	WCSW_BlackOut(DrawPort, mleft, mtop, mright, mbottom);

	WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_LTBLUE);

	WCSW_Move(DrawPort, data->left, data->top);
	WCSW_Draw(DrawPort, data->right, data->top);
	WCSW_Draw(DrawPort, data->right, data->bottom);
	WCSW_Draw(DrawPort, data->left, data->bottom);
	WCSW_Draw(DrawPort, data->left, data->top);

	if (data->ValuePrototype)
		{
		FormatAsPreferredUnit(GraphStr, data->ValuePrototype, data->HighDrawVal);
		WCSW_Move(DrawPort, data->left - 2 - data->textwidthtop, data->top + halftextht - textkludge);
		WCSW_Text(DrawPort, GraphStr, (int)strlen(GraphStr));

		FormatAsPreferredUnit(GraphStr, data->ValuePrototype, data->LowDrawVal);
		WCSW_Move(DrawPort, data->left - 2 - data->textwidthbottom, data->bottom + halftextht - textkludge);
		WCSW_Text(DrawPort, GraphStr, (int)strlen(GraphStr));

		if (data->textzero >= data->top + textht && data->textzero < data->bottom - textht)
			{
			sprintf(GraphStr, "%3.2f", 0.00);
			WCSW_Move(DrawPort, data->left - 2 - data->textwidthzero, data->textzero + halftextht - textkludge);
			WCSW_Text(DrawPort, GraphStr, (int)strlen(GraphStr));
			} // if 

		if (data->Crit->GetType() == WCS_ANIMCRITTER_TYPE_DOUBLECURVE && ((AnimDoubleCurve *)data->Crit)->YLabel[0])
			{
			WCSW_Move(DrawPort, data->left - 2 - textwidthlabel, (data->bottom + data->top) / 2 + halftextht - textkludge);
			WCSW_Text(DrawPort, ((AnimDoubleCurve *)data->Crit)->YLabel, (int)strlen(((AnimDoubleCurve *)data->Crit)->YLabel));
			} // if

		} // if

	if (data->DistancePrototype)
		{
		FormatAsPreferredUnit(GraphStr, data->DistancePrototype, data->TextLowDrawDist);
		WCSW_Move(DrawPort, data->left, data->textbottom - textkludge);
		WCSW_Text(DrawPort, GraphStr, (int)strlen(GraphStr));

		FormatAsPreferredUnit(GraphStr, data->DistancePrototype, data->TextHighDrawDist);
		WCSW_Move(DrawPort, data->right - WCSW_GetTextLength(DrawPort, GraphStr, (int)strlen(GraphStr)), data->textbottom - textkludge);
		WCSW_Text(DrawPort, GraphStr, (int)strlen(GraphStr));

		if (data->Crit->GetType() == WCS_ANIMCRITTER_TYPE_DOUBLECURVE && ((AnimDoubleCurve *)data->Crit)->XLabel[0])
			{
			WCSW_Move(DrawPort, (data->right + data->left) / 2 - WCSW_GetTextLength(DrawPort, ((AnimDoubleCurve *)data->Crit)->XLabel, (int)(strlen(((AnimDoubleCurve *)data->Crit)->XLabel)) / 2), data->textbottom - textkludge);
			WCSW_Text(DrawPort, ((AnimDoubleCurve *)data->Crit)->XLabel, (int)strlen(((AnimDoubleCurve *)data->Crit)->XLabel));
			} // if
		} // if

	// draw grids 
	if (data->drawgrid)
		{
		Y[0] = data->bottom - data->PixelsPerValGrid * (data->ValGridBase - data->LowDrawVal) / data->ValGridInt;
		WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_DKBLUE);
		for (Value = data->ValGridBase; Value < data->HighDrawVal; Value += data->ValGridInt)
			{
			long y0 = quickftol(Y[0]);

			WCSW_Move(DrawPort, data->left + 1, y0); 
			WCSW_Draw(DrawPort, data->right - 1, y0); 
			Y[0] -= data->PixelsPerValGrid;
			} // for value=..., draw value grid 

		X[0] = data->left + data->PixelsPerDist * ((double)data->DistGridBase - data->LowDrawDist);
		for (Frame = 0, Dist = data->DistGridBase; Dist < data->HighDrawDist; Dist += data->DistGridInt, Frame ++, X[0] += data->PixelsPerDistGrid)
			{
			long x0 = quickftol(X[0]);

			if (Dist < data->LowDrawDist)
				{
				continue;
				} // if
			if (Frame % data->DistGridLg)
				{
				WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_DKBLUE);
				}
			else
				{
				WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_LTBLUE);
				}
			WCSW_Move(DrawPort, x0, data->top + 1); 
			WCSW_Draw(DrawPort, x0, data->bottom - 1); 
			} // for frame=..., draw frame grid 
		} // if draw grid 
	} // IF DRAW OBJECT

if (WCSW_WIDGET_FLAGS & (WCSW_GRW_MADF_DRAWOBJECT | WCSW_GRW_MADF_DRAWGRAPH))
	{
	// draw graph 
	VC.LowX = data->left;
	VC.HighX = data->right + 1;
	VC.LowY = data->top;
	VC.HighY = data->bottom;
	for (j = 0; j < data->NumGraphs; j ++)
		{
		if (j == 0)
			WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_RED);
		else if (j == 1)
			WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_GREEN);
		else
			WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_LTBLUE);
		X[0] = data->left;
		Y[0] = (double)(data->bottom - data->PixelsPerVal * (data->Crit->GetValue(j, data->LowDrawDist) - data->LowDrawVal));// <<<>>> Distance & Value
		for (Dist = data->LowDrawDist + data->DistInterval; Dist <= data->HighDrawDist + data->DistInterval; Dist += data->DistInterval)
			{
			TempX[0] = X[0];
			TempY[0] = Y[0];
			TempX[1] = X[1] = data->left + data->PixelsPerDist * (Dist - data->LowDrawDist);
			TempY[1] = Y[1] = data->bottom - data->PixelsPerVal * (data->Crit->GetValue(j, Dist) - data->LowDrawVal);// <<<>>> Distance & Value
			if (VC.ClipSeg(TempX[0], TempY[0], TempX[1], TempY[1]))
				{
				WCSW_Move(DrawPort, quickftol(TempX[0]), quickftol(TempY[0])); 
				WCSW_Draw(DrawPort, quickftol(TempX[1]), quickftol(TempY[1]));
				} // if
			X[0] = X[1];
			Y[0] = Y[1];
			} // for frames
		} // for j
	if (data->ElevCrit)
		{
		WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_GREEN);
		X[0] = data->left;
		Y[0] = (double)(data->bottom - data->PixelsPerVal * (data->ElevCrit->GetValue(0, data->LowDrawDist) - data->LowDrawVal));// <<<>>> Distance & Value
		for (Dist = data->LowDrawDist + data->DistInterval; Dist <= data->HighDrawDist + data->DistInterval; Dist += data->DistInterval)
			{
			TempX[0] = X[0];
			TempY[0] = Y[0];
			TempX[1] = X[1] = data->left + data->PixelsPerDist * (Dist - data->LowDrawDist);
			TempY[1] = Y[1] = data->bottom - data->PixelsPerVal * (data->ElevCrit->GetValue(0, Dist) - data->LowDrawVal);// <<<>>> Distance & Value
			if (VC.ClipSeg(TempX[0], TempY[0], TempX[1], TempY[1]))
				{
				WCSW_Move(DrawPort, quickftol(TempX[0]), quickftol(TempY[0])); 
				WCSW_Draw(DrawPort, quickftol(TempX[1]), quickftol(TempY[1]));
				} // if
			X[0] = X[1];
			Y[0] = Y[1];
			} // for frames
		} // if 

	// draw key frame points 
	WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_WHITE);
	SelectedColorOn = 0;
	for (j = 0; j < data->NumGraphs; j++)
		{
		for (MyNode = data->Crit->GetFirstNode(j); MyNode; MyNode = data->Crit->GetNextNode(j, MyNode))
			{
			Dist = MyNode->GetDistance();// <<<>>> Distance
			if (MyNode->GetSelectedState())
				{
				if (! SelectedColorOn)
					{
					WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_YELLOW);
					SelectedColorOn = 1;
					} // if
				} // if
			else if (SelectedColorOn)
				{
				WCSW_SetWidgetDrawColor(DrawPort, WCS_WIDGET_COLOR_WHITE);
				SelectedColorOn = 0;
				} // else if
			X[0] = data->left + data->PixelsPerDist * (Dist - data->LowDrawDist);
			Y[0] = (double)(data->bottom - data->PixelsPerVal * (MyNode->GetValue() - data->LowDrawVal));// <<<>>> Value
			if (X[0] >= data->left && X[0] <= data->right + 1
				&& Y[0] > data->top && Y[0] <= data->bottom)
				{
				long x0, y0;

				x0 = quickftol(X[0]);
				y0 = quickftol(Y[0]);
				WCSW_Move(DrawPort, x0, y0 - 3);
				WCSW_Draw(DrawPort, x0 + 3, y0 + 2);
				WCSW_Draw(DrawPort, x0 - 3, y0 + 2);
				WCSW_Draw(DrawPort, x0, y0 - 3);
				} // if in bounds 
			} // for draw key frame triangles 
		} // for
	} // else if draw entire object 

#ifdef _WIN32
WCSW_WIDGET_FLAGS = NULL;
#endif // _WIN32

return(0);

} // GR_Draw() 


/*===========================================================================*/


#ifdef _WIN32
ULONG GR_HandleInput(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
#endif // _WIN32
{
//#define _between(a,x,b) ((x)>=(a) && (x)<=(b))
//#define _isinobject(x,y) (_between(data->left,(x),data->right) && _between(data->top,(y),data->bottom))

double Dist, DistRange, Value, ValueRange;
#ifdef _WIN32
struct GRData *data;
data = (struct GRData *)GetWindowLong(hwnd, WCSW_GR_OFF_GRDATA);
#endif // _WIN32
GraphNode *ClickedNode;

/* Mouse events */
#ifdef _WIN32
if (data)
#endif // _WIN32
	{
	switch (WCSW_WIDGET_ACTION)
		{
		#ifdef _WIN32
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		#endif // _WIN32
			{
			#ifdef _WIN32
			if (WCSW_WIDGET_ACTION == WM_LBUTTONDOWN)
			#endif // _WIN32
				{
					{
					data->x = WCSW_WIDGET_X;
					data->y = WCSW_WIDGET_Y;
					switch (data->inputflags)
						{
						case WCSW_GRW_NODE_NEW:
							{
							data->inputflags = 0;
							Dist = data->LowDrawDist + ((data->x - data->left) / data->PixelsPerDist);
							if (data->DisplayByFrame)
								{
								Dist = WCS_floor(Dist * data->FrameRate + .5) / data->FrameRate;
								} // if
							if (ClickedNode = data->Crit->RemoteAddNode(Dist))// <<<>>> Distance
								{
								data->GrWin->NewActiveNode(ClickedNode);
								data->Crit->ClearNodeSelectedAll();
								data->Crit->SetNodeSelected(ClickedNode, 1);
								} // if
							break;
							} //new key frame position
						case WCSW_GRW_NODE_SELECT:
						case WCSW_GRW_NODE_ALTERPOSITION:
							{
							// convert clicked point into distance and value
							Dist = data->LowDrawDist + ((data->x - data->left) / data->PixelsPerDist);
							Value = data->LowDrawVal + ((data->bottom - data->y) / data->PixelsPerVal);
							DistRange = 4 / data->PixelsPerDist;
							ValueRange = 4 / data->PixelsPerVal;
							if (ClickedNode = data->Crit->FindNode(Dist, DistRange, Value, ValueRange))// <<<>>> Distance & Value
								{
								data->ActiveNode = ClickedNode;
								if (ClickedNode->GetSelectedState())
									{
									data->CollectMouseMove = 1;
									if (! (data->inputflags & WCSW_GRW_NODE_ALTERPOSITION))
										data->inputflags |= WCSW_GRW_NODE_ALTERVALUE;
									data->Crit->AboutToChange();
									} // if
								else
									{
									data->inputflags = 0;
									data->Crit->ClearNodeSelectedAll();
									data->Crit->SetNodeSelected(ClickedNode, 1);
									data->GrWin->NewActiveNode(ClickedNode);
									} // else
								} // if
							break;
							} // WCSW_GRW_NODE_SELECT
						case WCSW_GRW_NODE_TOGGLESELECT:
							{
							// convert clicked point into distance and value
							Dist = data->LowDrawDist + ((data->x - data->left) / data->PixelsPerDist);
							Value = data->LowDrawVal + ((data->bottom - data->y) / data->PixelsPerVal);
							DistRange = 4 / data->PixelsPerDist;
							ValueRange = 4 / data->PixelsPerVal;
							if (ClickedNode = data->Crit->FindNode(Dist, DistRange, Value, ValueRange))// <<<>>> Distance & Value
								{
								data->inputflags = 0;
								data->ActiveNode = ClickedNode;
								data->Crit->ToggleNodeSelected(ClickedNode, 1);
								data->GrWin->NewActiveNode(ClickedNode);
								} // if
							break;
							} // WCSW_GRW_NODE_TOGGLESELECT
						default:
							break;
						} // switch data->inputflags
					} // if within area
				} // if SELECTDOWN/WM_LBUTTONDOWN
			else
				{
				data->CollectMouseMove = 0;
				data->inputflags = 0;
				} // else SELECTUP/WM_LBUTTONUP
			return(1);
			} // MOUSEBUTTONS/WM_LBUTTONDOWN/WM_LBUTTONUP

		#ifdef _WIN32
		case WM_MOUSEMOVE:
		#endif // _WIN32
			{
			if (data->CollectMouseMove && data->ActiveNode)
				{
				data->x = WCSW_WIDGET_X;
				data->y = WCSW_WIDGET_Y;
				switch (data->inputflags)
					{
					case WCSW_GRW_NODE_ALTERVALUE:
						{
						data->sy = data->y - WCSW_WIDGET_Y;
						Value = data->LowDrawVal + ((data->bottom - data->y) / data->PixelsPerVal);
						data->Crit->RemoteAlterSelectedNodeValue(Value, data->ActiveNode->GetValue());// <<<>>> Value
						break;
						} // WCSW_GRW_NODE_ALTERVALUE
					case WCSW_GRW_NODE_ALTERPOSITION:
						{
						data->sx = data->x - WCSW_WIDGET_X;
						Value = data->LowDrawDist + ((data->x - data->left) / data->PixelsPerDist);
						if (data->SnapToInt)
							{
							if (data->DisplayByFrame)
								{
								Value *= data->FrameRate;
								Value = WCS_floor(Value + .5);
								Value /= data->FrameRate;
								} // if
							else
								Value = WCS_floor(Value + .5);
							} // if
						data->Crit->RemoteAlterSelectedNodePosition(Value, data->ActiveNode->GetDistance());// <<<>>> Distance
						break;
						} // WCSW_GRW_NODE_ALTERPOSITION
					default:
						break;
					} // switch
				} // if
			return(1);
			} // MOUSEMOVE
		} // switch msg->imsg->Class
	} // if msg->imsg

return(0);

} // GR_HandleInput

/*===========================================================================*/
/*
** Here comes the dispatcher for our custom class. We only need to
** care about MUIM_AskMinMax and MUIM_Draw in this simple case.
** Unknown/unused methods are passed to the superclass immediately.
*/

#ifdef _WIN32
long WINAPI AnimGraphWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
long Handled = 0;
RECT Square;
HDC Canvas;
PAINTSTRUCT PS;
struct GRData *MyData;
char Shift, Control;

MyData = (struct GRData *)GetWindowLong(hwnd, WCSW_GR_OFF_GRDATA);

if(MyData)
	{
	Shift = wParam & MK_SHIFT;
	Control = wParam & MK_CONTROL;
	switch(message)
		{
		case WM_LBUTTONDOWN:
			{
			SetFocus(hwnd);
			if (Control)
				MyData->inputflags |= WCSW_GRW_NODE_ALTERPOSITION;
			if (Shift)
				MyData->inputflags |= WCSW_GRW_NODE_TOGGLESELECT;
			} // WM_LBUTTONDOWN
			//lint -fallthrough
		case WM_LBUTTONUP:
			{
			GR_HandleInput(hwnd, message, wParam, lParam);
			Handled = 1;
			break;
			} // WM_LBUTTONDOWN
		case WM_MOUSEMOVE:
			{
			if (wParam & MK_LBUTTON)
				GR_HandleInput(hwnd, message, wParam, lParam);
			else if ((MyData->inputflags & WCSW_GRW_NODE_ALTERPOSITION) || (MyData->inputflags & WCSW_GRW_NODE_ALTERVALUE))
				MyData->inputflags = 0;
			Handled = 1;
			break;
			} // WM_LBUTTONDOWN
		case WM_PAINT:
			{
			if(GetUpdateRect(hwnd, &Square, 0))
				{
				GetWindowRect(hwnd, &Square);
				Canvas = BeginPaint(hwnd, &PS);
				// Force full refresh
				MyData->drawflags = WCSW_GRW_MADF_DRAWOBJECT;
				GR_Draw(hwnd, Canvas, MyData);
				EndPaint(hwnd, &PS);
				Canvas = NULL;
				} // if
			Handled = 1;
			break;
			} // PAINT
		default:
			break;
		} // switch
	} // if

if(Handled)
	{
	return(0);
	} // if
else
	{
	return(DefWindowProc(hwnd, message, wParam, lParam));
	} // else

} // GraphWidgetWndProc

#endif // _WIN32
