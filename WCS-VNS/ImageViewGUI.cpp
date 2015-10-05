// ImageViewGUI.cpp
// Code for Image Viewer
// Built from scratch on 7/25/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ImageViewGUI.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "Raster.h"
#include "Interactive.h"
#include "EffectsLib.h"
#include "Texture.h"
#include "resource.h"
#include "AppMem.h"

extern unsigned char ThumbNailR[2048 * 3], ThumbNailG[2048], ThumbNailB[2048];

NativeGUIWin ImageViewGUI::Open(Project *Moi)
{

if (ViewWin)
	{
	ViewWin->Open(GlobalApp->MainProj);
	if (! AlreadyDrawn)
		{
		ViewWin->SetupForDrawing();
		PaintImage();
		AlreadyDrawn = 1;
		} // if
	} // if

GlobalApp->MCP->AddWindowToMenuList(this);

return (NULL);

} // ImageViewGUI::Open

/*===========================================================================*/

NativeGUIWin ImageViewGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_IMAGE_VIEW, LocalWinSys()->RootWin);

	} // if
 
return (NativeWin);

} // ImageViewGUI::Construct

/*===========================================================================*/

ImageViewGUI::ImageViewGUI(ImageLib *ImageSource, Raster *ActiveSource, int LoadLastRendered, int LoadAsRenderedSource)
: GUIFenetre('IMVG', this, "Image View") // Yes, I know...
{

ConstructError = 0;
ImageHost = ImageSource;
Active = ActiveSource;
Closing = AlreadyDrawn = Sampling = 0;
LoadAsRendered = LoadAsRenderedSource;
ViewWin = NULL;

// if not provided get a file and create a raster
if (! Active)
	{
	Active = MakeNewRaster(LoadLastRendered);
	MadeNewRaster = 1;
	} // if
else
	MadeNewRaster = 0;
if (ImageSource && Active)
	{
	if (! (ViewWin = ConstructView()))
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

} // ImageViewGUI::ImageViewGUI

/*===========================================================================*/

ImageViewGUI::~ImageViewGUI()
{

GlobalApp->MCP->RemoveWindowFromMenuList(this);

if(ViewWin)
	delete ViewWin;
ViewWin = NULL;

// this prevents the Raster destructor from trying to close this window too
// which it is trained to do, ah so intelligently, to prevent anyone from
// messing with this window when its raison d'etre is dead.
Closing = 1;

if (Active && MadeNewRaster)
	{
	delete Active;
	Active = NULL;
	} // if

} // ImageViewGUI::~ImageViewGUI()

/*===========================================================================*/

long ImageViewGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_IVG, 0);

return(1);

} // ImageViewGUI::HandleCloseWin

/*===========================================================================*/

long ImageViewGUI::HandleLeftButtonDown(long X, long Y, char Alt, char Control, char Shift)
{

SampleImage(X, Y, WCS_DIAGNOSTIC_ITEM_MOUSEDOWN);
Sampling = 1;

return (0);

} // ImageViewGUI::HandleRightButtonDown

/*===========================================================================*/

long ImageViewGUI::HandleLeftButtonUp(long X, long Y, char Alt, char Control, char Shift)
{

Sampling = 0;

return (0);

} // ImageViewGUI::HandleRightButtonDown

/*===========================================================================*/

long ImageViewGUI::HandleMouseMove(long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{

if (Sampling)
	SampleImage(X, Y, WCS_DIAGNOSTIC_ITEM_MOUSEDRAG);

return (0);

} // ImageViewGUI::HandleMouseMove

/*===========================================================================*/

Raster *ImageViewGUI::MakeNewRaster(int LoadLastRendered)
{
Raster *NewRast = NULL;
PathAndFile TestPAF;
char filename[256];

if (LoadLastRendered)
	{
	TestPAF.SetPath(GlobalApp->MainProj->framepath);
	TestPAF.SetName(GlobalApp->MainProj->framefile);
	} // if
else
	TestPAF.SetPath(GlobalApp->MainProj->imagepath);

if (LoadLastRendered || GetFileNamePtrn(0, "View Image File", (char *)TestPAF.GetPath(), (char *)TestPAF.GetName(), WCS_REQUESTER_WILDCARD, WCS_PATHANDFILE_NAME_LEN))
	{
	if (! LoadLastRendered)
		strcpy(GlobalApp->MainProj->imagepath, TestPAF.GetPath());
	if (! TestPAF.GetValidPathAndName(filename))
		{
		return (NULL);
		} // if

	if (NewRast = new Raster)
		{
		NewRast->PAF.Copy(&NewRast->PAF, &TestPAF);
		// try loading the image
		if (! NewRast->LoadnPrepImage(FALSE, FALSE))
			{
			delete NewRast;
			NewRast = NULL;
			} // if bad news
		} // if
	} // if

return (NewRast);

} // ImageViewGUI::MakeNewRaster

/*===========================================================================*/

DrawingFenetre *ImageViewGUI::ConstructView(void)
{
DrawingFenetre *Pane = NULL;
long ScreenWidth, ScreenHeight;

SampleRate = 1;
ScreenWidth = GlobalApp->WinSys->InquireDisplayWidth();
ScreenHeight = GlobalApp->WinSys->InquireDisplayHeight();

GlobalApp->MainProj->InquireWindowCoords('FOLP', WinLeft, WinTop, WinWidth, WinHeight);
WinWidth = Active->Cols < 32767 ? (short)Active->Cols: 32767;
WinHeight = Active->Rows < 32767 ? (short)Active->Rows: 32767;

while (WinWidth >= ScreenWidth || WinHeight >= ScreenHeight)
	{
	WinWidth /= 2;
	WinHeight /= 2;
	SampleRate *= 2;
	} // while
InsideWidth = WinWidth;
InsideHeight = WinHeight;

while ( ! Pane && WinWidth > 2 && WinHeight > 2)
	{
	WinWidth = max(30, InsideWidth);
	WinHeight = InsideHeight;
	W = WinWidth;
	H = WinHeight;
	LocalWinSys()->RationalizeWinCoords(WinLeft, WinTop, W, H);
	GlobalApp->MainProj->SetWindowCoords('FOLP', WinLeft, WinTop, WinWidth, WinHeight);

	Pane = new DrawingFenetre('FOLP', this, Active->GetUserName());

	if (! Pane)
		{
		if (WinTop > 1 || WinLeft > 1)
			{
			WinTop = 1;
			WinLeft = 1;
			} // if
		else
			{
			InsideWidth /= 2;
			InsideHeight /= 2;
			SampleRate *= 2;
			} // else
		} // if no window 
	if(Pane)
		{
		Pane->SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK);
		Pane->SetDrawingAreaSize(WinWidth, WinHeight);
		} // if

	} // while not open 

return (Pane);

} // ImageViewGUI::ConstructView

/*===========================================================================*/

void ImageViewGUI::PaintImage(void)
{
double RenderFrame = GlobalApp->MainProj->Interactive->GetActiveFrame();
#ifdef WCS_IMAGE_MANAGEMENT
NotifyTag Changes[2];
#endif // WCS_IMAGE_MANAGEMENT
long i = 0, j, k, l, zip;
unsigned short Wide = 0;
char FrameTxt[60];

if (Active)
	{
	if (LoadAsRendered)
		{
		if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
			{
			RenderFrame = GlobalApp->MainProj->Interactive->GetActiveFrame();
			sprintf(FrameTxt, "%f", RenderFrame);
			TrimZeros(FrameTxt);
			if (! GetInputString("Enter the frame (decimals OK) at which to view the Image Object.", WCS_REQUESTER_POSDIGITS_ONLY, FrameTxt))
				return;
			RenderFrame = atof(FrameTxt) / GlobalApp->MainProj->Interactive->GetFrameRate();
			} // if
		else
			{
			RenderFrame = GlobalApp->MainProj->Interactive->GetActiveTime();
			sprintf(FrameTxt, "%f", RenderFrame);
			TrimZeros(FrameTxt);
			if (! GetInputString("Enter the time in seconds (decimals OK) at which to view the Image Object.", WCS_REQUESTER_POSDIGITS_ONLY, FrameTxt))
				return;
			RenderFrame = atof(FrameTxt);
			} // else
		} // if
	#ifdef WCS_IMAGE_MANAGEMENT
	if(Active->ImageManagerEnabled && Active->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER))
		{
		BusyWin *BWIM = NULL;
		unsigned char SampleGuns[3];
		int AbortFlag = 0, UserAbort = 0;
		double TNX, TNY, TNUniformScaleFactor;
		double AverageColor[4];
		Raster *TNRas = NULL;
		BWIM = new BusyWin("Displaying Image", Active->Rows, 'BWIM', 0);
		// prepare thumbnail accumulator and average color vars
		TNX = (double)WCS_RASTER_TNAIL_SIZE / Active->Cols;
		TNY = (double)WCS_RASTER_TNAIL_SIZE / Active->Rows;
		TNUniformScaleFactor = min(TNX, TNY);
		TNX = Active->Cols * TNUniformScaleFactor;
		TNY = Active->Rows * TNUniformScaleFactor;
		BWIM->Update(i);
		if(TNRas = new Raster)
			{
			TNRas->Cols = (unsigned long)TNX;
			TNRas->Rows = (unsigned long)TNY;
			TNRas->ByteBands = 3;
			TNRas->ByteBandSize = TNRas->Cols * TNRas->Rows;
			TNRas->AlphaAvailable = TNRas->AlphaEnabled = 0;
			TNRas->ByteMap[0] = (UBYTE *)AppMem_Alloc(TNRas->Cols * TNRas->Rows, APPMEM_CLEAR, "Thumbnail Bitmaps");
			TNRas->ByteMap[1] = (UBYTE *)AppMem_Alloc(TNRas->Cols * TNRas->Rows, APPMEM_CLEAR, "Thumbnail Bitmaps");
			TNRas->ByteMap[2] = (UBYTE *)AppMem_Alloc(TNRas->Cols * TNRas->Rows, APPMEM_CLEAR, "Thumbnail Bitmaps");

			if(!(TNRas->ByteMap[0] && TNRas->ByteMap[1] && TNRas->ByteMap[2])) // everything ok?
				{
				delete TNRas;
				TNRas = NULL;
				} // if
			} // if
		BWIM->Update(i);
		AverageColor[0] = AverageColor[1] = AverageColor[2] = AverageColor[3] = 0.0; // [3] is counter for later normalization division 
		for (i = k = 0; i < Active->Rows; i += SampleRate, k ++)
			{
			for (j = l = 0, zip = i * Active->Cols; j < Active->Cols; j += SampleRate, zip += SampleRate, l ++)
				{
				Wide = 0;
				Active->SampleByteCell3(SampleGuns, j, i, AbortFlag);
				AverageColor[0] += ThumbNailR[l] = SampleGuns[0];
				AverageColor[1] += ThumbNailG[l] = SampleGuns[1];
				AverageColor[2] += ThumbNailB[l] = SampleGuns[2];
				AverageColor[3] += 1.0;
				Wide = (unsigned short)l;
				if(TNRas) // are we generating an updated thumbnail?
					{
					unsigned long TNRasX, TNRasY;
					TNRasX = (unsigned long)(j * TNUniformScaleFactor);
					TNRasY = (unsigned long)(i * TNUniformScaleFactor);
					if(TNRasX < TNX - 1 &&  TNRasY < TNY - 1)
						{
						unsigned long TNzip;
						TNzip = TNRasX + TNRasY * TNRas->Cols;
						TNRas->ByteMap[0][TNzip] = SampleGuns[0];
						TNRas->ByteMap[1][TNzip] = SampleGuns[1];
						TNRas->ByteMap[2][TNzip] = SampleGuns[2];
						} // if
					} // if
				} // for j
			ViewWin->BGDrawLine24(0, (unsigned short)k, Wide, (unsigned char *)ThumbNailR, (unsigned char *)ThumbNailG, (unsigned char *)ThumbNailB);
			ViewWin->SyncBackground(0, (unsigned short)k, Wide, 1); // progressive display

			if (BWIM)
				{
				if(BWIM->Update(i))	// don't need this for abort checking
					{
					UserAbort = 1;
					break;
					} // if
				} // if
			} // for i
		ViewWin->SyncBackground(0, 0, Wide, (unsigned short)(Active->Rows - 1)); // wholesale (re)display at end
		if(!UserAbort)
			{ // update thumbnail since we have all our image data handy
			if(TNRas)
				{
				TNRas->SetInitFlags(WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_DOWNSAMPLED);
				TNRas->CreateThumbNail(WCS_RASTER_BANDSET_BYTE, TNRas->ByteMap[WCS_RASTER_IMAGE_BAND_RED], TNRas->ByteMap[WCS_RASTER_IMAGE_BAND_GREEN], TNRas->ByteMap[WCS_RASTER_IMAGE_BAND_BLUE]);
				if(TNRas->Thumb)
					{ // steal it over into our raster
					if(Active->Thumb)
						{ // delete our previous thumbnail
						delete Active->Thumb;
						Active->Thumb = NULL;
						} // if
					Active->Thumb = TNRas->Thumb;
					TNRas->Thumb = NULL;
					} // if
				delete TNRas;
				TNRas = NULL;
				} // if

			// update Average Color after normalization
			if(AverageColor[3] != 0.0)
				{
				Active->AverageBand[0] = (AverageColor[0] / AverageColor[3]) / 255.0;
				Active->AverageBand[1] = (AverageColor[1] / AverageColor[3]) / 255.0;
				Active->AverageBand[2] = (AverageColor[2] / AverageColor[3]) / 255.0;
				} // if
			} // if
		// updated image thumbnail
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

		if (BWIM)
			delete BWIM;
		if (SampleRate > 1)
			UserMessageOK("Image Library: View Image", "Image has been downsampled to fit on the screen.");
		} // if
	else
	#endif // WCS_IMAGE_MANAGEMENT
		{ // do it the old way
		if ((! LoadAsRendered && Active->Red) || (! LoadAsRendered && Active->LoadnProcessImage(TRUE)) || (LoadAsRendered && Active->LoadToRender(RenderFrame, GlobalApp->MainProj->Interactive->GetFrameRate(), WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_FORCERELOAD)))
			{
			for (i = k = 0; i < Active->Rows; i += SampleRate, k ++)
				{
				for (j = l = 0, zip = i * Active->Cols; j < Active->Cols; j += SampleRate, zip += SampleRate, l ++)
					{
					Wide = 0;
					if(Active->Red)
						{
						if(Active->Green && Active->Blue)
							{
							//use ThumbNailR ThumbNailG ThumbNailB
							ThumbNailR[l] = Active->Red[zip];
							ThumbNailG[l] = Active->Green[zip];
							ThumbNailB[l] = Active->Blue[zip];
							} // if
						else
							{
							ThumbNailR[l] = ThumbNailG[l] = ThumbNailB[l] = Active->Red[zip];
							} // else
						Wide = (unsigned short)l;
						} // if
					} // for j
				ViewWin->BGDrawLine24(0, (unsigned short)k, Wide, (unsigned char *)ThumbNailR, (unsigned char *)ThumbNailG, (unsigned char *)ThumbNailB);
				} // for i
			ViewWin->SyncBackground(0, 0, Wide, (unsigned short)(Active->Rows - 1));
			if (SampleRate > 1)
				UserMessageOK("Image Library: View Image",
					"Image has been downsampled to fit on the screen.");
			} // if image loaded 
		else
			UserMessageOK("View Image",	"Unable to load image file for viewing!");
		} // else
	} // if 

} // ImageViewGUI::PaintImage

/*===========================================================================*/

void ImageViewGUI::SampleImage(long X, long Y, unsigned char EventType)
{
double dX, dY;
DiagnosticData Data;
RasterAttribute *MyAttr;
GeoRefShell *MyShell;
NotifyTag Changes[2];
int ElevReject;

dX = (double)X * SampleRate / Active->Cols;
dY = (double)Y * SampleRate / Active->Rows;

if (Active->SampleBytePoint(dX, dY, Active->Red, Active->Green, Active->Blue, Data.DataRGB[0], Data.DataRGB[1], Data.DataRGB[2]) >= 0)
	{
	Data.PixelX = X;
	Data.PixelY = Y;
	Data.ValueValid[WCS_DIAGNOSTIC_RGB] = 1;

	if ((MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && (MyShell = (GeoRefShell *)MyAttr->GetShell()))
		{
		if (MyShell->SampleLatLonElev(dX, dY, Data.Value[WCS_DIAGNOSTIC_LATITUDE], 
			Data.Value[WCS_DIAGNOSTIC_LONGITUDE], Data.Value[WCS_DIAGNOSTIC_ELEVATION], ElevReject))
			{
			Data.ValueValid[WCS_DIAGNOSTIC_LATITUDE] = Data.ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
			Data.ValueValid[WCS_DIAGNOSTIC_ELEVATION] = (! ElevReject);
			} // if
		} // if
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, EventType, 0);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, &Data);
	} // if

} // ImageViewGUI::SampleImage
