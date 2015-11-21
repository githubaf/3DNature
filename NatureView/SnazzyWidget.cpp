// SnazzyWidget.cpp
// Code to support raster-based graphical interactive widget
// Created from scratch using bits of WCS/VNS WCSWidgets.cpp
// on 10/25/04 by CXH (my birthday is tomorrow!)
// WIN32-only


#include <windows.h>
#include <windowsx.h> // for GET_X_LPARAM and GET_Y_LPARAM
#include <commctrl.h>
#include <limits.h>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include "SnazzyWidget.h"
#include "SnazzyBlit.h"
#include "ToolTips.h"

extern ToolTipSupport *GlobalTipSupport;


long FAR PASCAL SnazzyWidgetWndProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);

SnazzyWidgetContainerFactory::SnazzyWidgetContainerFactory(HINSTANCE Instance)
{
WNDCLASS InquireRegister;

_CachedInstance = Instance;


if(!GetClassInfo(Instance, SW_WNDCLASSNAME, &InquireRegister))
	{
	InquireRegister.style			= 0; //CS_DBLCLKS;
	InquireRegister.lpfnWndProc		= SnazzyWidgetWndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 4; // just a pointer to the real data
	InquireRegister.hInstance		= _CachedInstance;
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	InquireRegister.hbrBackground	= NULL; // (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= SW_WNDCLASSNAME;
	if(RegisterClass(&InquireRegister) == NULL)
		{
		// failure
		} // if
	} // if

} // SnazzyWidgetContainerFactory::SnazzyWidgetContainerFactory



SnazzyWidgetContainerFactory::~SnazzyWidgetContainerFactory()
{
UnregisterClass(SW_WNDCLASSNAME, _CachedInstance);

} // SnazzyWidgetContainerFactory::~SnazzyWidgetContainerFactory


SnazzyWidgetContainer *SnazzyWidgetContainerFactory::CreateSnazzyWidgetContainer(HWND ParentWin, SnazzyWidgetEventObserver *Observer, SnazzyWidgetImageCollection *Images)
{
SnazzyWidgetContainer *Result = NULL;
// create the widget container object
try
	{
	Result = new SnazzyWidgetContainer(_CachedInstance, ParentWin, Observer, Images);
	}
catch(std::bad_alloc)
	{
	return(NULL);
	} // catch
if(Result->GetContainerNativeWinOk())
	{
	return(Result);
	} // if
else
	{ // clean up and delete container object
	delete Result;
	Result = NULL;
	return(NULL);
	} // else
} // SnazzyWidgetContainerFactory::CreateSnazzyWidgetContainer









SnazzyWidgetContainer::SnazzyWidgetContainer(const HINSTANCE & Instance, const HWND & ParentWin, SnazzyWidgetEventObserver *Observer, SnazzyWidgetImageCollection *Images)
{
_CachedInstance = Instance;
_ContainerNativeWin = NULL;
unsigned long int WStyle = WS_VISIBLE | WS_POPUP;
_DragWidget = NULL; // may not have one
_ActiveSlideWidget = NULL; // no Slide Widget starts actively sliding

if(ParentWin) // can't go anywhere without a host window
	{
	unsigned long int NumPixels = Images->GetNumPixels();
	SetDragInProgress(false);
	_EventObserver = Observer;
	_Images = Images;
	_ImageWidth  = Images->GetWidth();
	_ImageHeight = Images->GetHeight();
	_FinalImage[0].resize(NumPixels);
	_FinalImage[1].resize(NumPixels);
	_FinalImage[2].resize(NumPixels);
	_ImageState.resize(NumPixels);
	fill(_ImageState.begin(), _ImageState.end(), 0); // clear all widgets to Normal state
	// pre-load 'Normal' image into _FinalImage
	BlitSimple(NumPixels, &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0], Images->GetR_Normal(), Images->GetG_Normal(), Images->GetB_Normal());
	
	_WidgetCollection.resize(1);
	_WidgetCollection[0] = NULL;

	if(_ContainerNativeWin = CreateWindowEx(0, SW_WNDCLASSNAME, "", WStyle,
	0, 0, _ImageWidth, _ImageHeight, ParentWin, NULL, Instance, NULL))
		{
		SetWindowLong(_ContainerNativeWin, 0, (long)this);
		} // if/
	} // if

} // SnazzyWidgetContainer::SnazzyWidgetContainer


SnazzyWidgetContainer::~SnazzyWidgetContainer()
{
if(_ContainerNativeWin)
	{
	DestroyWindow(_ContainerNativeWin);
	_ContainerNativeWin = NULL;
	} // if
} // SnazzyWidgetContainer::~SnazzyWidgetContainer



SnazzyWidget *SnazzyWidgetContainer::GetWidgetByNum(const unsigned char & WidgetNum) const
{
if(WidgetNum < _WidgetCollection.size())
	{
	return(_WidgetCollection[WidgetNum]);
	} // if
return(NULL);
} // SnazzyWidgetContainer::GetWidgetByNum




SnazzyCheckWidget *SnazzyWidgetContainer::CreateSnazzyCheckWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption)
{
SnazzyCheckWidget *Result = NULL;
if(Result = new SnazzyCheckWidget(this, WidgetNum, WidgetID, HelpCaption))
	{
	} // if

return(Result);
} // SnazzyWidgetContainer::CreateSnazzyCheckWidget



SnazzyPushWidget *SnazzyWidgetContainer::CreateSnazzyPushWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption)
{
SnazzyPushWidget *Result = NULL;

// this auto-adds to the _WidgetCollection
Result = new SnazzyPushWidget(this, WidgetNum, WidgetID, HelpCaption);

return(Result);
} // SnazzyWidgetContainer::CreateSnazzyPushWidget


SnazzyDragWidget *SnazzyWidgetContainer::CreateSnazzyDragWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption)
{
SnazzyDragWidget *Result = NULL;
if(Result = new SnazzyDragWidget(this, WidgetNum, WidgetID, HelpCaption))
	{
	} // if

return(Result);
} // SnazzyWidgetContainer::CreateSnazzyDragWidget


SnazzySlideXYWidget *SnazzyWidgetContainer::CreateSnazzySlideXYWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption)
{
SnazzySlideXYWidget *Result = NULL;
if(Result = new SnazzySlideXYWidget(this, WidgetNum, WidgetID, Image, HelpCaption))
	{
	} // if

return(Result);
} // SnazzyWidgetContainer::CreateSnazzySlideXYWidget

SnazzySlideXWidget *SnazzyWidgetContainer::CreateSnazzySlideXWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption)
{
SnazzySlideXWidget *Result = NULL;
if(Result = new SnazzySlideXWidget(this, WidgetNum, WidgetID, Image, HelpCaption))
	{
	} // if

return(Result);
} // SnazzyWidgetContainer::CreateSnazzySlideXWidget


SnazzySlideYWidget *SnazzyWidgetContainer::CreateSnazzySlideYWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption)
{
SnazzySlideYWidget *Result = NULL;
if(Result = new SnazzySlideYWidget(this, WidgetNum, WidgetID, Image, HelpCaption))
	{
	} // if

return(Result);
} // SnazzyWidgetContainer::CreateSnazzySlideYWidget


SnazzyTextWidget *SnazzyWidgetContainer::CreateSnazzyTextWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption)
{
SnazzyTextWidget *Result = NULL;
if(Result = new SnazzyTextWidget(this, WidgetNum, WidgetID, HelpCaption))
	{
	} // if

return(Result);
} // SnazzyWidgetContainer::CreateSnazzyTextWidget



SnazzyHotPadWidget *SnazzyWidgetContainer::CreateSnazzyHotPadWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption)
{
SnazzyHotPadWidget *Result = NULL;
if(Result = new SnazzyHotPadWidget(this, WidgetNum, WidgetID, Image, HelpCaption))
	{
	} // if

return(Result);
} // SnazzyWidgetContainer::CreateSnazzyHotPadWidget





// Lookup a widget ID from a pixel coordinate
// returns  for nothing found
unsigned char SnazzyWidgetContainer::GetWidgetNumFromXY(const unsigned long int & X, const unsigned long int & Y) const 
{
unsigned char WidgetNum = 0;
if(X < _Images->GetWidth() && Y < _Images->GetHeight())
	{
	WidgetNum = _Images->Get_Index()[_Images->Subscript(X, Y)];
	} // if
return(WidgetNum);
} // SnazzyWidgetContainer::GetWidgetNumFromXY

// Lookup a Widget pointer from a pixel coordinate
// returns NULL for nothing found
SnazzyWidget *SnazzyWidgetContainer::GetSnazzyWidgetFromXY(const unsigned long int & X, const unsigned long int & Y) const 
{
unsigned char IDVal = 0;

if(IDVal = GetWidgetNumFromXY(X, Y))
	{
	return(GetWidgetByNum(IDVal));
	} // if

return(NULL);
} // SnazzyWidgetContainer::GetSnazzyWidgetFromXY



// external Windows-flavored WndProc stub that just bridges into the C++ internal version
long FAR PASCAL SnazzyWidgetWndProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
SnazzyWidgetContainer *SWC;
if(SWC     = (SnazzyWidgetContainer *)GetWindowLong(hwnd, 0))
	{
	return(SWC->SnazzyWidgetWndProcInternal(hwnd, message, wParam, lParam));
	} // if

return(DefWindowProc(hwnd, message, wParam, lParam));

} // SnazzyWidgetWndProc





unsigned long int SnazzyWidgetContainer::ClearAllFocus(void)
{
std::vector<SnazzyWidget *>::iterator Walk;
unsigned long int NumFocused = 0;

if(_CurrentFocusedWidgetNum)
	{
	for(Walk = _WidgetCollection.begin(); Walk != _WidgetCollection.end(); Walk++)
		{
		if(*Walk)
			{
			if((*Walk)->GetFocused())
				{
				NumFocused++;
				(*Walk)->SetFocused(false);
				UpdateWidgetVisualState(*Walk);
				} // if
			} // if
		} // for
	} // if

_CurrentFocusedWidgetNum = 0; // no widget has focus



return(NumFocused);
} // SnazzyWidgetContainer::ClearAllFocus


void SnazzyWidgetContainer::ForceRepaintAll(void)
{
//InvalidateRect(_ContainerNativeWin, NULL, TRUE);
HDC Canvas;
if(Canvas = GetDC(_ContainerNativeWin))
	{
	RastBlastBlock(Canvas, 0, 0, GetImageWidth(), GetImageHeight(),
	 GetFinalImageR(), GetFinalImageG(), GetFinalImageB());
	// call PerformPaint on any widgets that support/require it
	std::vector<SnazzyWidget *>::iterator Walk;
	for(Walk = _WidgetCollection.begin(); Walk != _WidgetCollection.end(); Walk++)
		{
		if(*Walk)
			{
			(*Walk)->PerformPaint();
			} // if
		} // for

	ReleaseDC(_ContainerNativeWin, Canvas);
	} // if
} // SnazzyWidgetContainer::ForceRepaintAll



void SnazzyWidgetContainer::UpdateWidgetVisualState(SnazzyWidget *UpdateWidget)
{
if(UpdateWidget->GetEnabled())
	{
	if(UpdateWidget->GetFocused())
		{
		if(UpdateWidget->GetSelected())
			{ // selected-focused (selected hover)
			if(_Images->GetSelectedHoverValid()) // prefer SelectedHover
				{
				BlitWhere(_Images->GetNumPixels(), &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0],
				_Images->GetR_SelectedHover(), _Images->GetG_SelectedHover(), _Images->GetB_SelectedHover(),
				_Images->Get_Index(), UpdateWidget->GetWidgetNum());
				} // if
			else if(_Images->GetSelectedValid()) // at least we can show selected, if not hover
				{
				BlitWhere(_Images->GetNumPixels(), &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0],
				_Images->GetR_Selected(), _Images->GetG_Selected(), _Images->GetB_Selected(),
				_Images->Get_Index(), UpdateWidget->GetWidgetNum());
				} // else if
			else if(_Images->GetNormalHoverValid()) // can we at least show hover state?
				{
				BlitWhere(_Images->GetNumPixels(), &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0],
				_Images->GetR_NormalHover(), _Images->GetG_NormalHover(), _Images->GetB_NormalHover(),
				_Images->Get_Index(), UpdateWidget->GetWidgetNum());
				} // else if
			else // // ok, we must at least have Normal...
				{
				BlitWhere(_Images->GetNumPixels(), &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0],
				_Images->GetR_Normal(), _Images->GetG_Normal(), _Images->GetB_Normal(),
				_Images->Get_Index(), UpdateWidget->GetWidgetNum());
				} // else
			} // if
		else
			{ // focused (hover)
			if(_Images->GetNormalHoverValid()) // prefer NormalHover
				{
				BlitWhere(_Images->GetNumPixels(), &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0],
				_Images->GetR_NormalHover(), _Images->GetG_NormalHover(), _Images->GetB_NormalHover(),
				_Images->Get_Index(), UpdateWidget->GetWidgetNum());
				} // if
			else // ok, we must at least have Normal...
				{
				BlitWhere(_Images->GetNumPixels(), &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0],
				_Images->GetR_Normal(), _Images->GetG_Normal(), _Images->GetB_Normal(),
				_Images->Get_Index(), UpdateWidget->GetWidgetNum());
				} // else
			} // else
		} // if
	else
		{
		if(UpdateWidget->GetSelected())
			{ // selected
			if(_Images->GetSelectedValid()) // prefer selected
				{
				BlitWhere(_Images->GetNumPixels(), &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0],
				_Images->GetR_Selected(), _Images->GetG_Selected(), _Images->GetB_Selected(),
				_Images->Get_Index(), UpdateWidget->GetWidgetNum());
				} // if
			else // ok, we must at least have Normal...
				{
				BlitWhere(_Images->GetNumPixels(), &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0],
				_Images->GetR_Selected(), _Images->GetG_Selected(), _Images->GetB_Selected(),
				_Images->Get_Index(), UpdateWidget->GetWidgetNum());
				} // else
			} // if
		else
			{ // normal
			BlitWhere(_Images->GetNumPixels(), &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0],
			_Images->GetR_Normal(), _Images->GetG_Normal(), _Images->GetB_Normal(),
			_Images->Get_Index(), UpdateWidget->GetWidgetNum());
			} // else
		} // else
	} // if
else
	{ // disabled
	/*
	BlitWhere(_Images->GetNumPixels(), &_FinalImage[0][0], &_FinalImage[1][0], &_FinalImage[2][0],
	_Images->GetR_Disabled(), _Images->GetG_Disabled(), _Images->GetB_Disabled(),
	_Images->Get_Index(), UpdateWidget->GetWidgetNum());
	*/
	} // else

// add any widget-specific decorations (floating knobs, etc)
UpdateWidget->PerformRedraw();

} // SnazzyWidgetContainer::UpdateWidgetVisualState

























SnazzyWidget::SnazzyWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption)
{
if(Host)
	{
	_Enabled = true;
	_Focused = false;
	_Selected = false;
	// set bounds to inverse maxima
	_ULX = _ULY = SHRT_MAX;
	_LRX = _LRY = -SHRT_MAX;
	_Host = Host;
	SetWidgetNum(WidgetNum);
	SetWidgetID(WidgetID);
	
	_HelpCaption = HelpCaption;
	if(Host->_WidgetCollection.size() < (unsigned)(WidgetNum + 1))
		{
		Host->_WidgetCollection.resize(1 + WidgetNum); // ensure container is big enough
		} // if
	Host->_WidgetCollection[WidgetNum] = this;
	UpdateBounds();
	} // if
} // SnazzyWidget::SnazzyWidget


// determine pixel bounds of region where this widget's WidgetNum is found
void SnazzyWidget::UpdateBounds(void)
{
unsigned char *IndexBuf = _Host->_Images->Get_Index();
for(signed long int YLoop = 0; (unsigned)YLoop < _Host->_Images->GetHeight(); YLoop++)
	{
	for(signed long int XLoop = 0; (unsigned)XLoop < _Host->_Images->GetWidth(); XLoop++)
		{
		// is the current pixel part of our widget?
		if(IndexBuf[_Host->_Images->Subscript(XLoop, YLoop)] == _WidgetNum)
			{
			if(XLoop < _ULX) _ULX = XLoop;
			if(XLoop > _LRX) _LRX = XLoop;
			if(YLoop < _ULY) _ULY = YLoop;
			if(YLoop > _LRY) _LRY = YLoop;
			} // if
		} // for
	} // for

} // SnazzyWidget::UpdateBounds


void SnazzyWidget::SetSelected(const bool & NewSelected)
{
SetSelectedSilent(NewSelected);
_Host->ForceRepaintAll();
} // SnazzyWidget::SetSelected


void SnazzyWidget::SetSelectedSilent(const bool & NewSelected)
{
_Selected = NewSelected;
_Host->UpdateWidgetVisualState(this);
} // SnazzyWidget::SetSelectedSilent



SnazzyCheckWidget::SnazzyCheckWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption)
: SnazzyWidget(Host, WidgetNum, WidgetID, HelpCaption)
{
} // SnazzyCheckWidget::SnazzyCheckWidget



SnazzyPushWidget::SnazzyPushWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption)
: SnazzyWidget(Host, WidgetNum, WidgetID, HelpCaption)
{
} // SnazzyPushWidget::SnazzyPushWidget


SnazzyDragWidget::SnazzyDragWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption)
: SnazzyWidget(Host, WidgetNum, WidgetID, HelpCaption)
{
_Host->_DragWidget = this; // record ourselves at the drag widget
SetClickOriginX(-SHRT_MAX);
SetClickOriginY(-SHRT_MAX);
} // SnazzyDragWidget::SnazzyDragWidget


SnazzyDragWidget::~SnazzyDragWidget()
{
if(_Host->_DragWidget == this)
	{
	_Host->_DragWidget = NULL; // remove ourselves from being listed as the drag widget
	} // if
} // SnazzyDragWidget::~SnazzyDragWidget


SnazzySlideXYWidget::SnazzySlideXYWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption)
: SnazzyWidget(Host, WidgetNum, WidgetID, HelpCaption)
{
_KnobImage = Image;
} // SnazzySlideXYWidget::SnazzySlideXYWidget


SnazzySlideXWidget::SnazzySlideXWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption)
: SnazzyWidget(Host, WidgetNum, WidgetID, HelpCaption)
{
_KnobImage = Image;
} // SnazzySlideXWidget::SnazzySlideXWidget


SnazzySlideYWidget::SnazzySlideYWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption)
: SnazzyWidget(Host, WidgetNum, WidgetID, HelpCaption)
{
_KnobImage = Image;
_PositionY = 0.0f; // don't use SetPositionY, as it will trigger a redraw
_Host->UpdateWidgetVisualState(this);
} // SnazzySlideYWidget::SnazzySlideYWidget


SnazzyTextWidget::SnazzyTextWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption)
: SnazzyWidget(Host, WidgetNum, WidgetID, HelpCaption)
{

} // SnazzyTextWidget::SnazzyTextWidget

SnazzyHotPadWidget::SnazzyHotPadWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption)
: SnazzyWidget(Host, WidgetNum, WidgetID, HelpCaption)
{
_KnobImage = Image; // not supported right now
_PositionY = _PositionX = 0.0f;
_Host->UpdateWidgetVisualState(this);
} // SnazzyHotPadWidget::SnazzyHotPadWidget






void SnazzyCheckWidget::HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY)
{
SetSelected(!GetSelected());
_Host->UpdateWidgetVisualState(this);
if(_Host->_EventObserver)
	{ // call the Observer to notify it of the event that occurred
	_Host->_EventObserver->Handle(GetWidgetID());
	} // if

} // SnazzyCheckWidget::HandleLButtonDown



void SnazzyPushWidget::HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY)
{
SetSelected(true);
_Host->UpdateWidgetVisualState(this);
if(_Host->_EventObserver)
	{ // call the Observer to notify it of the event that occurred
	_Host->_EventObserver->Handle(GetWidgetID());
	} // if
} // SnazzyPushWidget::HandleLButtonDown

void SnazzyPushWidget::HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY)
{
SetSelected(false);
_Host->UpdateWidgetVisualState(this);
if(_Host->_EventObserver)
	{ // call the Observer to notify it of the event that occurred
	// (actually, push button doesn't really seem to want a buttonup event)
	// _Host->_EventObserver->Handle(GetWidgetID()); 
	} // if
} // SnazzyPushWidget::HandleLButtonUp


void SnazzyDragWidget::HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY)
{
POINT MousePosScreen;

// we need absolute screen coords for window drag operations
GetCursorPos(&MousePosScreen);

InitiateDrag(MousePosScreen.x, MousePosScreen.y);

} // SnazzyDragWidget::HandleLButtonDown

void SnazzyDragWidget::HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY)
{
CancelDrag();
} // SnazzyDragWidget::HandleLButtonUp

void SnazzyDragWidget::HandleMouseDrag()
{
POINT MousePosScreen;

if(GetSelected())
	{
	if((GetClickOriginX() != -SHRT_MAX) && (GetClickOriginY() != -SHRT_MAX))
		{

		// check to see if LMB has been released without us noticing, and abort drag if it has.
		bool LogicalLButtonIsDown = false;
		if(GetSystemMetrics(SM_SWAPBUTTON))
			{ // check the RButton
			if(GetAsyncKeyState(VK_RBUTTON)  & 0x800000)
				{
				LogicalLButtonIsDown = true;
				} // if
			} // if
		else
			{ // check the LButton
			if(GetAsyncKeyState(VK_LBUTTON)  & 0x800000)
				{
				LogicalLButtonIsDown = true;
				} // if
			} // else

		if(!LogicalLButtonIsDown)
			{
			CancelDrag();
			return;
			} // if

		signed int DeltaX, DeltaY;
		RECT Bounds;
		// we need absolute screen coords for window drag operations
		GetCursorPos(&MousePosScreen);
		
		// Calculate where we've moved since last event
		DeltaX = MousePosScreen.x - GetClickOriginX();
		DeltaY = MousePosScreen.y - GetClickOriginY();
		
		// update coords to account for this move
		SetClickOriginX(MousePosScreen.x);
		SetClickOriginY(MousePosScreen.y);
		
		// Move our window
		GetWindowRect(_Host->_ContainerNativeWin, &Bounds);
		Bounds.left += DeltaX;
		Bounds.top  += DeltaY;
		SetWindowPos(_Host->_ContainerNativeWin, NULL, Bounds.left, Bounds.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		} // if
	} // if

} // SnazzyDragWidget::HandleMouseDrag


void SnazzyDragWidget::InitiateDrag(signed long int MouseOriginX, signed long int MouseOriginY)
{
_Host->SetDragInProgress(true); // prevent someone else from cancelling our capture
SetCapture(_Host->_ContainerNativeWin);
SetSelected(true);
_Host->UpdateWidgetVisualState(this);

SetClickOriginX(MouseOriginX);
SetClickOriginY(MouseOriginY);
} // SnazzyDragWidget::InitiateDrag


void SnazzyDragWidget::CancelDrag(void)
{
if(GetCapture() == _Host->_ContainerNativeWin)
	{
	_Host->SetDragInProgress(false); // this prevented someone else from cancelling our capture
	ReleaseCapture();
	} // if
SetSelected(false);
_Host->UpdateWidgetVisualState(this);
SetClickOriginX(-SHRT_MAX);
SetClickOriginY(-SHRT_MAX);
} // SnazzyDragWidget::CancelDrag





void SnazzySlideXYWidget::HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY)
{
} // SnazzySlideXYWidget::HandleLButtonDown

void SnazzySlideXYWidget::HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY)
{
} // SnazzySlideXYWidget::HandleLButtonUp

void SnazzySlideXYWidget::HandleMouseMove(const signed long int & MouseX, const signed long int & MouseY)
{
} // SnazzySlideXYWidget::HandleMouseMove







void SnazzySlideXWidget::HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY)
{
} // SnazzySlideXWidget::HandleLButtonDown

void SnazzySlideXWidget::HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY)
{
} // SnazzySlideXWidget::HandleLButtonUp

void SnazzySlideXWidget::HandleMouseMove(const signed long int & MouseX, const signed long int & MouseY)
{
} // SnazzySlideXWidget::HandleMouseMove






void SnazzySlideYWidget::HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY)
{
_Host->SetDragInProgress(true); // prevent someone else from cancelling our capture
_Host->SetActiveSlideWidget(this); // we're the active slide widget
SetCapture(_Host->_ContainerNativeWin);
SetSelected(true);
_Host->UpdateWidgetVisualState(this);
HandleNewPosition(MouseX, MouseY);
} // SnazzySlideYWidget::HandleLButtonDown

void SnazzySlideYWidget::HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY)
{
if(GetCapture() == _Host->_ContainerNativeWin)
	{
	_Host->SetDragInProgress(false); // this prevented someone else from cancelling our capture
	ReleaseCapture();
	} // if
if(_Host->GetActiveSlideWidget() == this) // are we the active Slide Widget (we should be, but...)
	{
	_Host->SetActiveSlideWidget(NULL); // now nobody is the active slide widget
	} // if
SetSelected(false);
_Host->UpdateWidgetVisualState(this);
} // SnazzySlideYWidget::HandleLButtonUp


void SnazzySlideYWidget::HandleMouseMove(const signed long int & MouseX, const signed long int & MouseY)
{
HandleNewPosition(MouseX, MouseY);
} // SnazzySlideYWidget::HandleMouseMove


void SnazzySlideYWidget::HandleNewPosition(const signed long int & MouseX, const signed long int & MouseY)
{
int RelativeMouseY, InverseMouseY, YSize;
float Fraction;

if(GetSelected())
	{
	if(GetLRY() != GetULY()) // prevent /0 errors calculating fraction
		{
		if(MouseY < 0) // mouse is captured an out of window beyond top edge, special case
			{
			Fraction = 1.0; // top of control, just trust me on this one.
			} // if
		else
			{
			if(_KnobImage)
				{
				RelativeMouseY = MouseY - GetULY();
				RelativeMouseY -= (_KnobImage->GetHeight() / 2); // offset to account for upper half-knob-height margin
				YSize = (GetLRY() - GetULY()) - _KnobImage->GetHeight(); // reduce Y range by one knob height worth of margins
				InverseMouseY = YSize - RelativeMouseY;
				Fraction = (float)InverseMouseY / (float)YSize;
				if(Fraction < 0.0)
					Fraction = 0.0; // constrain fraction within range, even if mouse is in vertical margins but within widget
				if(Fraction > 1.0)
					Fraction = 1.0;
				} // if
			else
				{ // no margins, use full size
				RelativeMouseY = MouseY - GetULY();
				YSize = (GetLRY() - GetULY());
				InverseMouseY = YSize - RelativeMouseY;
				Fraction = (float)InverseMouseY / (float)YSize;
				} // else
			} // else
		SetPositionY(Fraction);
		if(_Host->_EventObserver)
			{ // call the Observer to notify it of the event that occurred
			_Host->_EventObserver->Handle(GetWidgetID());
			} // if
		} // if
	} // if

} // SnazzySlideYWidget::HandleNewPosition


void SnazzySlideYWidget::PerformRedraw(void)
{
int CenterX, CoordY, YSize;
unsigned long int Subscript;

YSize = GetLRY() - GetULY();
CenterX = (GetLRX() - GetULX()) / 2;
CenterX += GetULX(); // offset into container coordinates


if(_KnobImage)
	{
	unsigned long int OffX = 0, OffY = 0;

	// calculate PositionY, setting aside buffering margins at
	// top and bottom for one-half the height of the knob
	YSize -= _KnobImage->GetHeight(); // account for half knob margins at top and bottom
	CoordY = YSize - (int)(YSize * GetPositionY());
	CoordY += GetULY() + (_KnobImage->GetHeight() / 2); // offset into container coordinates, and offset by knob margin

	// OffX and OffY are in container-relative coords from the start,
	// because they're relative to CenterX and CoordY which already are
	OffX = CenterX - (_KnobImage->GetWidth() / 2);
	OffY = CoordY - (_KnobImage->GetHeight() / 2);

	// Do the drawing deed
	BlitAlpha(&_Host->_FinalImage[0][0], &_Host->_FinalImage[1][0], &_Host->_FinalImage[2][0], _Host->_Images->GetWidth(), _Host->_Images->GetHeight(),
	 _KnobImage->GetR_Normal(), _KnobImage->GetG_Normal(), _KnobImage->GetB_Normal(), _KnobImage->GetA_Normal(),
	 _KnobImage->GetWidth(), _KnobImage->GetHeight(), OffX, OffY);
	} // if
else
	{ // draw a simple red line the width of the widget space
	int yi = 0, xi = 0;

	// calculate PositionY using full vertical 'live' range of widget
	// since knob is infinitely small, height-wise
	CoordY = YSize - (int)(YSize * GetPositionY());
	CoordY += GetULY(); // offset into container coordinates

	for(xi = GetULX(); xi < GetLRX(); xi++)
		{
		Subscript = _Host->_Images->Subscript(xi, CoordY + yi);
		_Host->_FinalImage[0][Subscript] = 255;
		_Host->_FinalImage[1][Subscript] = 0;
		_Host->_FinalImage[2][Subscript] = 0;
		} // for
	} // else

} // SnazzySlideYWidget::PerformRedraw










void SnazzyHotPadWidget::HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY)
{
_Host->SetDragInProgress(true); // prevent someone else from cancelling our capture
_Host->SetActiveSlideWidget(this); // we're the active slide widget
SetCapture(_Host->_ContainerNativeWin);
SetSelected(true);
_Host->UpdateWidgetVisualState(this);
HandleNewPosition(MouseX, MouseY);
} // SnazzyHotPadWidget::HandleLButtonDown

void SnazzyHotPadWidget::HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY)
{
if(GetCapture() == _Host->_ContainerNativeWin)
	{
	_Host->SetDragInProgress(false); // this prevented someone else from cancelling our capture
	ReleaseCapture();
	} // if
if(_Host->GetActiveSlideWidget() == this) // are we the active Slide Widget (we should be, but...)
	{
	_Host->SetActiveSlideWidget(NULL); // now nobody is the active slide widget
	} // if
SetSelected(false);
SetPositionY(0.0f);
SetPositionX(0.0f);
_Host->UpdateWidgetVisualState(this);
// necessary to generate one final notify to register the detent position
if(_Host->_EventObserver)
	{ // call the Observer to notify it of the event that occurred
	_Host->_EventObserver->Handle(GetWidgetID());
	} // if
} // SnazzyHotPadWidget::HandleLButtonUp


void SnazzyHotPadWidget::HandleMouseMove(const signed long int & MouseX, const signed long int & MouseY)
{
// filter out mouse movements that are not within our mask
if(GetSelected()) // nothing to do if we're not even selected
	{
	if(_Host->GetWidgetNumFromXY(MouseX, MouseY) == GetWidgetNum()) // is the widget under the cursor us?
		{
		HandleNewPosition(MouseX, MouseY);
		} // if
	} // if
} // SnazzyHotPadWidget::HandleMouseMove


void SnazzyHotPadWidget::HandleNewPosition(const signed long int & MouseX, const signed long int & MouseY)
{
int RelativeMouseY, InverseMouseY, YSize;
int RelativeMouseX, /* InverseMouseX, */ XSize;
float FractionX = 0.0f, FractionY = 0.0f;

if(GetLRY() != GetULY()) // prevent /0 errors calculating fraction
	{
	if(MouseY < 0) // mouse is captured an out of window beyond top edge, special case
		{
		FractionY = 1.0; // top of control, just trust me on this one.
		} // if
	else
		{
		if(_KnobImage)
			{
			RelativeMouseY = MouseY - GetULY();
			RelativeMouseY -= (_KnobImage->GetHeight() / 2); // offset to account for upper half-knob-height margin
			YSize = (GetLRY() - GetULY()) - _KnobImage->GetHeight(); // reduce Y range by one knob height worth of margins
			InverseMouseY = YSize - RelativeMouseY;
			FractionY = (float)InverseMouseY / (float)YSize;
			if(FractionY < 0.0)
				FractionY = 0.0; // constrain fraction within range, even if mouse is in vertical margins but within widget
			if(FractionY > 1.0)
				FractionY = 1.0;
			} // if
		else
			{ // no margins, use full size
			RelativeMouseY = MouseY - GetULY();
			YSize = (GetLRY() - GetULY());
			InverseMouseY = YSize - RelativeMouseY;
			FractionY = (float)InverseMouseY / (float)YSize;
			} // else
		} // else
	} // if
if(GetLRX() != GetULX()) // prevent /0 errors calculating fraction
	{
	if(MouseX < 0) // mouse is captured an out of window beyond top edge, special case
		{
		FractionX = 1.0; // top of control, just trust me on this one.
		} // if
	else
		{
		if(_KnobImage)
			{
			RelativeMouseX = MouseX - GetULX();
			RelativeMouseX -= (_KnobImage->GetWidth() / 2); // offset to account for half-knob-width margin
			XSize = (GetLRX() - GetULX()) - _KnobImage->GetWidth(); // reduce X range by one knob width worth of margins
			//InverseMouseX = XSize - RelativeMouseX;
			//FractionX = (float)InverseMouseX / (float)XSize;
			FractionX = (float)RelativeMouseX / (float)XSize;
			if(FractionX < 0.0)
				FractionX = 0.0; // constrain fraction within range, even if mouse is in vertical margins but within widget
			if(FractionX > 1.0)
				FractionX = 1.0;
			} // if
		else
			{ // no margins, use full size
			RelativeMouseX = MouseX - GetULX();
			XSize = (GetLRX() - GetULX());
			//InverseMouseX = XSize - RelativeMouseX;
			//FractionX = (float)InverseMouseX / (float)XSize;
			FractionX = (float)RelativeMouseX / (float)XSize;
			} // else
		} // else
	} // if
// recenter
FractionX -= 0.5f;
FractionY -= 0.5f;

// clamp
if(FractionX < -0.5f) FractionX = -0.5f;
if(FractionX > 0.5f) FractionX = 0.5f;
if(FractionY < -0.5f) FractionY = -0.5f;
if(FractionY > 0.5f) FractionY = 0.5f;

if(GetExponential())
	{
	if(FractionX < 0)
		{
		FractionX = -(FractionX * FractionX);
		} // if
	else
		{
		FractionX = (FractionX * FractionX);
		} // else

	if(FractionY < 0)
		{
		FractionY = -(FractionY * FractionY);
		} // if
	else
		{
		FractionY = (FractionY * FractionY);
		} // else

	} // if

SetPositionY(FractionY);
SetPositionX(FractionX);

if(_Host->_EventObserver)
	{ // call the Observer to notify it of the event that occurred
	_Host->_EventObserver->Handle(GetWidgetID());
	} // if

} // SnazzyHotPadWidget::HandleNewPosition


void SnazzyHotPadWidget::PerformRedraw(void)
{
/*
int CenterX, CenterY, CoordX, CoordY, XSize, YSize;
unsigned long int Subscript;

YSize = GetLRY() - GetULY();
XSize = GetLRX() - GetULX();

CenterX = (GetLRX() - GetULX()) / 2;
CenterX += GetULX(); // offset into container coordinates
CenterY = (GetLRY() - GetULY()) / 2;
CenterY += GetULY(); // offset into container coordinates

if(_KnobImage)
	{
	unsigned long int OffX = 0, OffY = 0;
	// calculate CoordY, setting aside buffering margins at
	// top and bottom for one-half the height of the knob
	YSize -= _KnobImage->GetHeight(); // account for half knob margins at top and bottom
	CoordY = YSize - (int)(YSize * (GetPositionY() + 0.5f));
	CoordY += GetULY() + (_KnobImage->GetHeight() / 2); // offset into container coordinates, and offset by knob margin

	// calculate CoordX, setting aside buffering margins at
	XSize -= _KnobImage->GetWidth(); // account for half knob margins at left and right
	CoordX = (int)(XSize * (GetPositionX() + 0.5f));
	CoordX += GetULX() + (_KnobImage->GetWidth() / 2); // offset into container coordinates, and offset by knob margin

	// OffX and OffY are in container-relative coords from the start,
	// because they're relative to CenterX and CoordY which already are
	OffX = CoordX - (_KnobImage->GetWidth() / 2);
	OffY = CoordY - (_KnobImage->GetHeight() / 2);

	// Do the drawing deed
	BlitAlpha(&_Host->_FinalImage[0][0], &_Host->_FinalImage[1][0], &_Host->_FinalImage[2][0], _Host->_Images->GetWidth(), _Host->_Images->GetHeight(),
	 _KnobImage->GetR_Normal(), _KnobImage->GetG_Normal(), _KnobImage->GetB_Normal(), _KnobImage->GetA_Normal(),
	 _KnobImage->GetWidth(), _KnobImage->GetHeight(), OffX, OffY);
	} // if
else
	{ // draw a simple red marker dot
	int yi = 0, xi = 0;

	// calculate CoordY using full vertical 'live' range of widget
	// since knob is infinitely small, height-wise
	CoordY = YSize - (int)(YSize * (GetPositionY() + 0.5f));
	CoordY += GetULY(); // offset into container coordinates

	// calculate CoordX using full horizontal 'live' range of widget
	// since knob is infinitely small, width-wise
	CoordX = (int)(XSize * (GetPositionX() + 0.5f));
	CoordX += GetULX(); // offset into container coordinates

	Subscript = _Host->_Images->Subscript(CoordX, CoordY);
	_Host->_FinalImage[0][Subscript] = 255;
	_Host->_FinalImage[1][Subscript] = 0;
	_Host->_FinalImage[2][Subscript] = 0;
	} // else
*/

} // SnazzyHotPadWidget::PerformRedraw



void SnazzyTextWidget::PerformPaint(void)
{
HDC ContainerDC;

if(ContainerDC = GetDC(_Host->GetContainerNativeWin()))
	{
	HFONT OldFont;
	RECT TextRect;
	OldFont = (HFONT)SelectObject(ContainerDC, GetStockObject(ANSI_VAR_FONT));
    SetTextColor(ContainerDC, RGB(0, 0, 0));
    SetBkMode(ContainerDC, TRANSPARENT);
    
    TextRect.left = GetULX(); // in Window/Container coords already -- perfect!
    TextRect.top = GetULY();
    TextRect.right = GetLRX();
    TextRect.bottom = GetLRY();
    
    DrawText(ContainerDC, _Text.c_str(), _Text.length(), &TextRect, DT_TOP | DT_LEFT | DT_NOPREFIX);

	ReleaseDC(_Host->GetContainerNativeWin(), ContainerDC);
	SelectObject(ContainerDC, OldFont);
	} // if

} // SnazzyTextWidget::PerformPaint


void SnazzyTextWidget::SetText(const char *NewText)
{
if(strcmp(_Text.c_str(), NewText))
	{
	_Text = NewText;
	_Host->ForceRepaintAll();
	} // if

} // SnazzyTextWidget::SetText








SnazzyWidgetImageCollection::SnazzyWidgetImageCollection(const unsigned long int & Width, const unsigned long int & Height)
{
_Width = Width;
_Height = Height;
_NumPixels = (_Width * _Height);

_NormalValid = _NormalHoverValid = _SelectedValid = _SelectedHoverValid = false;

_Normal[0].resize(_NumPixels);
_Normal[1].resize(_NumPixels);
_Normal[2].resize(_NumPixels);
_NormalHover[0].resize(_NumPixels);
_NormalHover[1].resize(_NumPixels);
_NormalHover[2].resize(_NumPixels);
_Selected[0].resize(_NumPixels);
_Selected[1].resize(_NumPixels);
_Selected[2].resize(_NumPixels);
_SelectedHover[0].resize(_NumPixels);
_SelectedHover[1].resize(_NumPixels);
_SelectedHover[2].resize(_NumPixels);
_Index.resize(_NumPixels);

} // SnazzyWidgetImageCollection::SnazzyWidgetImageCollection





SnazzyWidgetImage::SnazzyWidgetImage(const unsigned long int & Width, const unsigned long int & Height)
{
_Width = Width;
_Height = Height;
_NumPixels = (_Width * _Height);

_Normal[0].resize(_NumPixels);
_Normal[1].resize(_NumPixels);
_Normal[2].resize(_NumPixels);
_Normal[3].resize(_NumPixels);

} // SnazzyWidgetImage::SnazzyWidgetImage










long SnazzyWidgetContainer::SnazzyWidgetWndProcInternal(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
long Handled = 0;

WORD ID;
signed long int X, Y;
HWND Parent;
RECT Square;
HDC Canvas;
PAINTSTRUCT PS;
LPTOOLTIPTEXT lpTTT;
char New = 0, OutBounds = 0;
unsigned long int WStyle;

Parent  = GetParent(hwnd);
ID      = (WORD) GetWindowLong(hwnd, GWL_ID);
WStyle  = GetWindowLong(hwnd, GWL_STYLE);
GetClientRect(hwnd, &Square);

switch(message)
	{
	case WM_NOTIFY:
		{
		if(lpTTT = (LPTOOLTIPTEXT)lParam)
			{
			if(lpTTT->hdr.code == TTN_NEEDTEXT)
				{
				POINT MousePosScreen;
				SnazzyWidget *SW;

				GetCursorPos(&MousePosScreen);
				ScreenToClient(hwnd, &MousePosScreen);
				if(SW = GetSnazzyWidgetFromXY(MousePosScreen.x, MousePosScreen.y))
					{
					strcpy(lpTTT->szText, SW->_HelpCaption.c_str());
					} // if
				} // if
			else if(lpTTT->hdr.code == TTN_SHOW)
				{
				// Version 4.70. To display the ToolTip in its default location, return zero.
				// To customize the ToolTip position, reposition the ToolTip window with the
				// SetWindowPos function and return TRUE.
				// Note: For versions earlier than 4.70, there is no return value.
				return(0);
				//SetWindowPos(MasterTip, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
				} // if
			} // if
		Handled = 1;
		break;
		} // NOTIFY
/*
	case WM_SETFONT:
		{
		WThis->Font = (HFONT)wParam;
		Handled = 1;
		HCti.cbSize = sizeof(TOOLINFO);
		HCti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		HCti.lpszText = LPSTR_TEXTCALLBACK;
		HCti.hwnd = hwnd;
		HCti.uId = (UINT)hwnd;
		HCti.hinst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
		SendMessage(MasterTip, TTM_ADDTOOL, 0, (LPARAM)&HCti);
		break;
		} // SETFONT
*/
	case WM_ERASEBKGND:
		{
		return(1);
		break;
		} // SETFONT
	case WM_KILLFOCUS:
		{
		if(!GetDragInProgress())
			{
			if(GetCapture() == _ContainerNativeWin)
				{
				ReleaseCapture();
				} // if
			} // if
		ClearAllFocus();
		ForceRepaintAll();
		// fall through to PAINT, below
		} // KILLFOCUS
	case WM_PAINT:
		{
		if(GetUpdateRect(hwnd, NULL, 0))
			{
			Canvas = BeginPaint(hwnd, &PS);
			RastBlastBlock(Canvas, 0, 0, GetImageWidth(), GetImageHeight(),
			 GetFinalImageR(), GetFinalImageG(), GetFinalImageB());
			 
			// call PerformPaint on any widgets that support/require it
			std::vector<SnazzyWidget *>::iterator Walk;
			for(Walk = _WidgetCollection.begin(); Walk != _WidgetCollection.end(); Walk++)
				{
				if(*Walk)
					{
					(*Walk)->PerformPaint();
					} // if
				} // for

			EndPaint(hwnd, &PS);
			} // if
		Handled = 1;
		break;
		} // PAINT
/*
	case WM_SETFOCUS:
		{
		SetFocus(hwnd);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
		} // SETFOCUS
*/
	case WM_MOUSEMOVE:
		{
		if(_DragWidget) _DragWidget->HandleMouseDrag();
		X = GET_X_LPARAM(lParam); Y = GET_Y_LPARAM(lParam);
		if(_ActiveSlideWidget) _ActiveSlideWidget->HandleMouseMove(X, Y);
		if(X > Square.right || X < 0 || Y > Square.bottom || Y < 0)
			{
			OutBounds = 1;
			if(!GetDragInProgress())
				{
				if(GetCapture() == _ContainerNativeWin)
					{
					ReleaseCapture();
					} // if
				} // if
			if(ClearAllFocus())
				{
				ForceRepaintAll();
				} // if
			} // if
		else
			{
			OutBounds = 0;
			SnazzyWidget *SW;
			if(SW = GetSnazzyWidgetFromXY(X, Y))
				{
				if(SW->GetWidgetNum() != GetCurrentFocusedWidgetNum())
					{ // toggle ToolTip enabled state to reset it when moving between
					// sub-widgets without encountering a non-subwidget region
					if(GlobalTipSupport)
						{
						GlobalTipSupport->SetEnabled(0);
						GlobalTipSupport->SetEnabled(1);
						} // if
					} // if
				ClearAllFocus();
				SW->SetFocused(true);
				SetCapture(hwnd);
				UpdateWidgetVisualState(SW);
				ForceRepaintAll();
				if(GlobalTipSupport)
					{ // enable tips only when over a sub-widget
					GlobalTipSupport->SetEnabled(1);
					} // if
				SW->HandleMouseMove(X, Y);
				} // if
			else
				{
				if(_CurrentFocusedWidgetNum)
					{
					if(GlobalTipSupport)
						{ // disable tips when not over a sub-widget
						GlobalTipSupport->SetEnabled(0);
						} // if
					ClearAllFocus();
					if(!GetDragInProgress())
						{
						if(GetCapture() == _ContainerNativeWin)
							{
							ReleaseCapture();
							} // if
						} // if
					ForceRepaintAll();
					} // if
				} // else
/*
			if(!WThis->Capture)
				{
				SetCapture(hwnd);
				WThis->Capture = 1;
				} // if
			if(!WThis->State && !(WStyle & WCSW_TB_STYLE_TOG))
				{
				if(wParam & MK_LBUTTON)
					{
					if((!WThis->State) && (WThis->Trigger))
						{
						WThis->State = 1;
						New = 1;
						} // if
					} // if
				} // if
*/
			} // else
		if(OutBounds)
			{
/*
			WThis->Trigger = 0;
			if(!WThis->State || (WStyle & WCSW_TB_STYLE_TOG) || (WStyle & WCSW_TB_STYLE_NOCHANGE))
				{
				ReleaseCapture();

				WThis->Capture = 0;
				} // if
			if(WThis->State && !(WStyle & WCSW_TB_STYLE_TOG))
				{
				if(!(WStyle & WCSW_TB_STYLE_NOCHANGE)) // inhibited
					{
					WThis->State = 0;
					New = 1;
					} // if
				} // if
*/
			} // if
		break;
		} // MOUSEMOVE
	case WM_CANCELMODE:
		{
		if(!GetDragInProgress())
			{
			if(GetCapture() == _ContainerNativeWin)
				{
				ReleaseCapture();
				} // if
			} // if
		if(ClearAllFocus())
			{
			ForceRepaintAll();
			} // if
		break;
		} // CANCELMODE
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case 0x020A: // WM_MOUSEWHEEL
		{ // forward certain events to parent
		PostMessage(Parent, message, wParam, lParam);
		break;
		} // key codes
	case WM_RBUTTONDOWN:
		{
		// fall through to LBUTTON code if RBUTTON permitted, else break
/*
		if(!(WStyle & WCSW_TB_STYLE_ALLOWRIGHT))
			{
			break;
			} // if
*/
		} // RBUTTONDOWN
	case WM_LBUTTONDOWN:
		{
		X = GET_X_LPARAM(lParam); Y = GET_Y_LPARAM(lParam);
		if(X > Square.right || X < 1 || Y > Square.bottom || Y < 1)
			{
			// do nothing
			} // if
		else
			{
			SnazzyWidget *SW;
			SetFocus(hwnd);
			if(SW = GetSnazzyWidgetFromXY(X, Y))
				{
				SW->HandleLButtonDown(X, Y);
				//UpdateWidgetVisualState(SW); // not needed, done by HandleLButtonDown
				ForceRepaintAll();
				} // if
			else
				{
				// do nothing
				} // else
			} // else
			
/*
		if(!(WStyle & WCSW_TB_STYLE_NOFOCUS)) SetFocus(hwnd);
		if(!(wParam & MK_CONTROL))
			{
			if(!(WStyle & WCSW_TB_STYLE_NOCHANGE)) // inhibited
				{
				if(WStyle & WCSW_TB_STYLE_TOG)
					{
					WThis->State = !WThis->State; New = 1;
					X = GET_X_LPARAM(lParam); Y = GET_Y_LPARAM(lParam);
					if(!(X > Square.right || X < 1 || Y > Square.bottom || Y < 1))
		 				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
					} // if
				else
					{
					WThis->State = 1; New = 1;
					WThis->Trigger = 1;
					} // else
				} // if
			else if(WStyle & WCSW_TB_STYLE_ISTHUMB)
				{
				X = GET_X_LPARAM(lParam); Y = GET_Y_LPARAM(lParam);
				if(TNailData.ValueValid[WCS_DIAGNOSTIC_RGB] = GlobalApp->WinSys->WL->SampleTNailColor(hwnd, (Raster *)WThis->Normal, X, Y, TNailData.DataRGB[0], TNailData.DataRGB[1], TNailData.DataRGB[2]))
					{
					TNailChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, WCS_DIAGNOSTIC_ITEM_MOUSEDOWN, 0);
					TNailChanges[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(TNailChanges, &TNailData);
					} // if
				if(WThis->Normal && ((Raster *)WThis->Normal)->Thumb)
					{
					TNailChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_THUMBNAIL, 0, 0);
					TNailChanges[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(TNailChanges, ((Raster *)WThis->Normal)->Thumb);
					} // if
				} // else
			} // if
*/
		break;
		} // LBUTTONDOWN
/*
	case WM_CREATE:
		{
		} // CREATE
*/
	case WM_DESTROY:
		{
/*
		HCti.hwnd = hwnd;
		HCti.uId = (UINT)hwnd;
		HCti.hinst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
		SendMessage(MasterTip, TTM_DELTOOL, 0, (LPARAM)&HCti);
*/
		return(0);
		} // DESTROY
	case WM_RBUTTONDBLCLK:
		{
		// fall through to LBUTTON code if RBUTTON permitted, else break
/*
		if(!(WStyle & WCSW_TB_STYLE_ALLOWRIGHT))
			{
			break;
			} // if
*/
		} // WM_RBUTTONDBLCLK
	case WM_LBUTTONDBLCLK:
		{
/*
		if(!(wParam & MK_CONTROL))
			{
			X = GET_X_LPARAM(lParam); Y = GET_Y_LPARAM(lParam);
			if(!(X > Square.right || X < 1 || Y > Square.bottom || Y < 1))
				{
				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_DBLCLK << 16), (LONG)hwnd);
				} // if
			} // if
*/
		break;
		} // WM_LBUTTONDBLCLK
	case WM_RBUTTONUP:
		{
/*
		// fall through to LBUTTON code if RBUTTON permitted, else break
		if(!(WStyle & WCSW_TB_STYLE_ALLOWRIGHT))
			{
			break;
			} // if
*/
		} // WM_RBUTTONUP
	case WM_LBUTTONUP:
		{
		X = GET_X_LPARAM(lParam); Y = GET_Y_LPARAM(lParam);
		if(_ActiveSlideWidget) _ActiveSlideWidget->HandleLButtonUp(X, Y);
		if(X > Square.right || X < 1 || Y > Square.bottom || Y < 1)
			{
			// do nothing
			} // if
		else
			{
			SnazzyWidget *SW;
			if(SW = GetSnazzyWidgetFromXY(X, Y))
				{
				SW->HandleLButtonUp(X, Y);
				//UpdateWidgetVisualState(SW); // not needed, handle by HandleLButtonUp
				ForceRepaintAll();
				} // if
			else
				{
				// do nothing
				} // else
			} // else
/*
		if(wParam & MK_CONTROL)
			{ // Misuse the BN_HILITE notify
			if(!(X > Square.right || X < 1 || Y > Square.bottom || Y < 1))
				{
				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_HILITE << 16), (LONG)hwnd);
				} // if
			} // if
		else if(!(WStyle & WCSW_TB_STYLE_TOG))
			{
			if(WThis->Capture)
				{
				ReleaseCapture();
				WThis->Capture = 0;
				if(!(WStyle & WCSW_TB_STYLE_NOCHANGE)) // inhibited
					{
					WThis->State = 0; New = 1;
					if(WThis->Trigger)
						{
						if(!(X > Square.right || X < 1 || Y > Square.bottom || Y < 1))
							{
							WNDPROC ControlWndProc;
							ControlWndProc = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
							PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
							} // if
						WThis->Trigger = 0;
						} // if
					} // if
				} // if
			} // if
*/
		break;
		} // LBUTTONUP

	} // switch

if(New)
	{
	Canvas = GetDC(hwnd);
	//GlobalApp->WinSys->WL->DrawToolButtonImage(Canvas, hwnd, WThis->Normal, WThis->Hilite, TCI, WThis->State);
	//if(!(WStyle & WCSW_TB_STYLE_NOCHANGE))
	//	DrawButtonFocus(hwnd, Canvas, WCSW_TOOLBUTTON_FOCUS_BORDER);
	ReleaseDC(hwnd, Canvas);
	New = 0;
	} // if


if(!Handled)
	{
	return(DefWindowProc(hwnd, message, wParam, lParam));
	} // if
else
	{
	return(0);
	} // else

} // SnazzyWidgetContainer::SnazzyWidgetWndProcInternal

