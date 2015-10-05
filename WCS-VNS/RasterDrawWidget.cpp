// RasterWidget.cpp
// Code for RasterDraw Widget
// Built from AnimGraphWidget.cpp by GRH
// Copyright 2001 3D Nature, LLC

#include "stdafx.h"
#include "WCSWidgets.h"
#include "WidgetSupport.h"
#include "RasterDrawWidget.h"
#include "Fenetre.h"
#include "Raster.h"
#include "AppModule.h"
#include "Application.h"
#include "UsefulMath.h"

extern WCSApp *GlobalApp;

long WINAPI RasterWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
ULONG RasterWidget_Draw(NativeControl hwnd, HDC MyDC, struct RasterWidgetData *data);
ULONG RasterWidget_HandleInput(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
extern unsigned char ThumbNailR[2048 * 3], ThumbNailG[2048], ThumbNailB[2048];

/*===========================================================================*/

void ConfigureRD(NativeControl Dest, int Target, struct RasterWidgetData *NewData)
{
NativeControl DestWidget;
GUIFenetre *GF;

if (GF = (GUIFenetre *)GetWindowLong(Dest, GWL_USERDATA))
	{
	if (DestWidget = GF->GetWidgetFromID(Target))
		{
		ConfigureRD(DestWidget, NewData);
		} // if
	} // if

} // ConfigureRD

/*===========================================================================*/

void ConfigureRD(NativeControl Dest, struct RasterWidgetData *NewData)
{

SetWindowLong(Dest, WCSW_RD_OFF_RDDATA, (long)NewData);
//InvalidateRect(Dest, NULL, NULL);
struct RasterWidgetData *MyData;
MyData = (struct RasterWidgetData *)GetWindowLong(Dest, WCSW_RD_OFF_RDDATA);
RasterWidget_Draw(Dest, NULL, MyData);

} // ConfigureRD

/*===========================================================================*/

/*
** Draw method is called whenever MUI feels we should render
** our object. This usually happens after layout is finished
** or when we need to refresh in a simplerefresh window.
** Note: You may only render within the rectangle
**       _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj).
*/

// <<<<< >>>>>   Uh, this routine only returns the value 0
ULONG RasterWidget_Draw(NativeControl hwnd, HDC MyDC, struct RasterWidgetData *data)
{
double X, Y, XInc, YInc, dYSample, dXSample, oXInc, oYInc, odYSample, odXSample, Visible;
UBYTE *Red, *Green, *Blue, *oRed, *oGreen, *oBlue;
Raster *MainSubject, *OverSubject;
long YSample, XSample, oYSample, oXSample, Sample;
unsigned short Wide;
NativeControl Stamp;
RECT XLate;
HDC Chalk = NULL;
RC Cola;

Stamp = hwnd;

if (!data) return(0);

GetWindowRect(Stamp, &XLate);
if (!MyDC)
	Chalk = GetDC(Stamp);
else
	Chalk = MyDC;

Cola.xLeft  = 0;
Cola.yTop   = 0;
Cola.xRight = XLate.right - XLate.left;
Cola.yBot   = XLate.bottom - XLate.top;

Draw3dButtonIn(hwnd, Chalk, &Cola, 0x01, false, false);

XLate.right -= (XLate.left + 4);
XLate.bottom -= (XLate.top + 4);

XLate.left = 0;
XLate.top = 0;

Visible = data->Visible > 0.0 ? data->Visible: 1.0;

if ((MainSubject = data->MainRast) && MainSubject->ByteMap[0] && MainSubject->ByteMap[1] && MainSubject->ByteMap[2])
	{
	Red = MainSubject->ByteMap[0];
	Green = MainSubject->ByteMap[1];
	Blue = MainSubject->ByteMap[2];
	YInc = (double)(MainSubject->Rows - 1) / (double)(XLate.bottom - 1);
	XInc = (double)(MainSubject->Cols - 1) / (double)(XLate.right - 1);
	YInc *= Visible;
	XInc *= Visible;

	if ((OverSubject = data->OverRast) && OverSubject->ByteMap[0] && OverSubject->ByteMap[1] && OverSubject->ByteMap[2])
		{
		oRed = OverSubject->ByteMap[0];
		oGreen = OverSubject->ByteMap[1];
		oBlue = OverSubject->ByteMap[2];
		oYInc = (double)(OverSubject->Rows - 1) / (double)(XLate.bottom - 1);
		oXInc = (double)(OverSubject->Cols - 1) / (double)(XLate.right - 1);
		oYInc *= Visible;
		oXInc *= Visible;
		} // if overlay
	else
		{
		OverSubject = NULL;
		oYInc = oXInc = 0.0;
		} // else

	dYSample = data->OffsetY * (MainSubject->Rows - 1);
	if (OverSubject)
		odYSample = data->OffsetY * (OverSubject->Rows - 1);
	else
		odYSample = 0.0;
	for (Y = (double)XLate.top; Y < XLate.bottom; Y++, dYSample += YInc, odYSample += oYInc)
		{
		if (dYSample >= MainSubject->Rows)
			dYSample = MainSubject->Rows - 1;
		YSample = quickftol(dYSample) * MainSubject->Cols;
		if (OverSubject)
			{
			if (odYSample >= OverSubject->Rows)
				odYSample = OverSubject->Rows - 1;
			oYSample = quickftol(odYSample) * OverSubject->Cols;
			} // if

		Wide = 0;
		dXSample = data->OffsetX * (MainSubject->Cols - 1);
		if (OverSubject)
			odXSample = data->OffsetX * (OverSubject->Cols - 1);
		else
			odXSample = 0.0;
		for (X = (double)XLate.left; X < XLate.right; X++, dXSample += XInc, odXSample += oXInc)
			{
			XSample = quickftol(dXSample);
			if (XSample >= MainSubject->Cols)
				XSample = MainSubject->Cols - 1;
			Sample = YSample + XSample;
			ThumbNailR[(int)X] = Red[Sample];
			ThumbNailG[(int)X] = Green[Sample];
			ThumbNailB[(int)X] = Blue[Sample];
			if (OverSubject)
				{
				oXSample = quickftol(odXSample);
				if (oXSample >= OverSubject->Cols)
					oXSample = OverSubject->Cols - 1;
				Sample = oYSample + oXSample;
				if (oRed[Sample] || oGreen[Sample] || oBlue[Sample])
					{
					ThumbNailR[(int)X] = oRed[Sample];
					ThumbNailG[(int)X] = oGreen[Sample];
					ThumbNailB[(int)X] = oBlue[Sample];
					} // if
				} // if
			Wide = (unsigned short)X;
			//VSetPixel(Chalk, (int)X + 2, (int)Y + 2, WINGDI_RGB(175, 175, 175));
			} // for
		RastBlast(Chalk, (unsigned short)(XLate.left + 2), (unsigned short)(Y + 2), Wide,
		 (unsigned char *)ThumbNailR, (unsigned char *)ThumbNailG, (unsigned char *)ThumbNailB);
		} // for
	} // if
else
	{
	for (Y = (double)XLate.top; Y < XLate.bottom; Y++)
		{
		Wide = 0;
		for (X = (double)XLate.left; X < XLate.right; X++)
			{
			ThumbNailR[(int)X] = 0; ThumbNailG[(int)X] = 0; ThumbNailB[(int)X] = 0; Wide = (unsigned short)X;
			//VSetPixel(Chalk, (int)X + 2, (int)Y + 2, WINGDI_RGB(175, 175, 175));
			} // for
		RastBlast(Chalk, (unsigned short)(XLate.left + 2), (unsigned short)(Y + 2), Wide,
		 (unsigned char *)ThumbNailR, (unsigned char *)ThumbNailG, (unsigned char *)ThumbNailB);
		} // for
	} // else

if (Chalk && !MyDC)
	{
	ReleaseDC(Stamp, Chalk);
	} // if

return(0);

} // RasterWidget_Draw() 

/*===========================================================================*/

ULONG RasterWidget_HandleInput(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
struct RasterWidgetData *data;
int CtrlID;
char Alt, Ctrl, Shift;

CtrlID = GetWindowLong(hwnd, GWL_ID);
data = (struct RasterWidgetData *)GetWindowLong(hwnd, WCSW_RD_OFF_RDDATA);
if (data)
	{
	switch (WCSW_WIDGET_ACTION)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			{
			Ctrl = (wParam & MK_CONTROL ? 1 : 0);
			Shift = (wParam & MK_SHIFT ? 1 : 0);
			Alt = GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_ALT);
			data->x = WCSW_WIDGET_X;
			data->y = WCSW_WIDGET_Y;
			if (WCSW_WIDGET_ACTION == WM_LBUTTONDOWN)
				{
				data->CollectMouseMove = 1;
				data->RDWin->HandleLeftButtonDown(CtrlID, data->x - 2, data->y - 2, Alt, Ctrl, Shift);
				} // if SELECTDOWN/WM_LBUTTONDOWN 
			else
				{ // duplicated in WM_MOUSEMOVE below if LBUTTON not down
				data->CollectMouseMove = 0;
				data->RDWin->HandleLeftButtonUp(CtrlID, data->x - 2, data->y - 2, Alt, Ctrl, Shift);
				} // else SELECTUP/WM_LBUTTONUP
			return(1);
			} // MOUSEBUTTONS/WM_LBUTTONDOWN/WM_LBUTTONUP

		case WM_MOUSEMOVE:
			{
			Ctrl = (wParam & MK_CONTROL ? 1 : 0);
			Shift = (wParam & MK_SHIFT ? 1 : 0);
			Alt = GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_ALT);
			bool Left = wParam & MK_LBUTTON ? true : false;
			data->x = WCSW_WIDGET_X;
			data->y = WCSW_WIDGET_Y;
			data->RDWin->HandleMouseMove(CtrlID, data->x - 2, data->y - 2, Alt, Ctrl, Shift, Left ? 1 : 0, 0, 0);
			if (data->CollectMouseMove && !Left)
				{ // duplicated from WM_LBUTTONUP above
				data->CollectMouseMove = 0;
				data->RDWin->HandleLeftButtonUp(CtrlID, data->x - 2, data->y - 2, Alt, Ctrl, Shift);
				} // else
			return(1);
			} // WM_MOUSEMOVE 
		default:
			break;
		} // switch msg->imsg->Class 
	} // if msg->imsg 

return(0);

} // RasterWidget_HandleInput

/*===========================================================================*/

long WINAPI RasterWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
long Handled = 0;
RECT Square;
HDC Canvas;
PAINTSTRUCT PS;
struct RasterWidgetData *MyData;
char Shift, Control;

MyData = (struct RasterWidgetData *)GetWindowLong(hwnd, WCSW_RD_OFF_RDDATA);

if (MyData)
	{
	Shift = wParam & MK_SHIFT;
	Control = wParam & MK_CONTROL;
	switch(message)
		{
		case WM_LBUTTONDOWN:
			{
			SetFocus(hwnd);
			RasterWidget_HandleInput(hwnd, message, wParam, lParam);
			Handled = 1;
			break;
			} // WM_LBUTTONDOWN
		case WM_LBUTTONUP:
			{
			RasterWidget_HandleInput(hwnd, message, wParam, lParam);
			Handled = 1;
			break;
			} // WM_LBUTTONDOWN
		case WM_MOUSEMOVE:
			{
			// 043003 CXH: Feed mouse movement events all the time, not just when MB is pressed.
			// This allows for brush outline tracking even when not painting. 
			// EDSS GUI needs to be updated to filter out events it doesn't need, as it was
			// expecting only move events when mousebutton was down.
			//if (wParam & MK_LBUTTON)
				RasterWidget_HandleInput(hwnd, message, wParam, lParam);
			Handled = 1;
			break;
			} // WM_LBUTTONDOWN
		case WM_PAINT:
			{
			if (GetUpdateRect(hwnd, &Square, 0))
				{
				GetWindowRect(hwnd, &Square);
				Canvas = BeginPaint(hwnd, &PS);
				// Force full refresh
				RasterWidget_Draw(hwnd, Canvas, MyData);
				EndPaint(hwnd, &PS);
				Canvas = NULL;
				} // if
			Handled = 1;
			break;
			} // WM_PAINT
		default:
			break;
		} // switch
	} // if

if (Handled)
	{
	return(0);
	} // if
else
	{
	return(DefWindowProc(hwnd, message, wParam, lParam));
	} // else

} // RasterWidgetWndProc
