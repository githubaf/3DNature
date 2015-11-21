// SnazzyWidget.h
// Classes and such for raster-based graphical interactive widget
// Created from scratch using bits of WCS/VNS WCSWidgets.cpp
// on 10/25/04 by CXH (my birthday is tomorrow!)


#ifndef SNAZZYWIDGET_H
#define SNAZZYWIDGET_H

#include <vector>
#include <windows.h>
#include "PropertyMacros.h"

#define WIDGET_CLASSPREFIX	"WLIB_"


/* *************************** Message ID Bases *********************** */

#define WM_WIDGETS_MSG_BASE			0xC000

#define WM_WIDGETS_SW_BASE			(WM_WIDGETS_MSG_BASE + 32) // lower 32 might clash with other widget classes we know of

// Messages supported by SnazzyWidget
#define WM_WIDGETS_SW_SETUP	WM_WIDGETS_SW_BASE

// Style defines for SnazzyWidget
/*
#define WIDGETS_SW_STYLE_TOG		0x01
#define WIDGETS_SW_STYLE_TRUECOLOR	0x02
#define WIDGETS_SW_STYLE_NOCHANGE	0x04
#define WIDGETS_SW_STYLE_CORNERTAB	0x08
#define WIDGETS_SW_STYLE_ISTHUMB	0x10
#define WIDGETS_SW_STYLE_NOFOCUS	0x20
#define WIDGETS_SW_STYLE_ALLOWRIGHT	0x40
*/

#define SW_WNDCLASSNAME		WIDGET_CLASSPREFIX "_" "Snazzy"

class SnazzyWidgetContainerFactory;
class SnazzyWidgetContainer;
class SnazzyWidget;
class SnazzyCheckWidget;
class SnazzyPushWidget;
class SnazzyDragWidget;
class SnazzyWidgetImageCollection;
class SnazzyWidgetImage;
class SnazzyWidgetEventObserver;
class SnazzySlideYWidget;
class SnazzySlideXWidget;
class SnazzySlideXYWidget;
class SnazzyHotPadWidget;
class SnazzyTextWidget;

// SnazzyWidgetContainerFactory is essentially the Widget Library, that handles
// all the setup and OS interaction necessary to prepare for widget
// creation.
// Once you have a SnazzyWidgetContainerFactory, you can ask it to create a
// SnazzyWidgetContainer, which is the only way to create SnazzyWidgetContainers
class SnazzyWidgetContainerFactory
	{
	private:
		HINSTANCE _CachedInstance;
		HWND MasterTip;

	public:
		SnazzyWidgetContainerFactory(HINSTANCE Instance);
		~SnazzyWidgetContainerFactory();
		
		SnazzyWidgetContainer *CreateSnazzyWidgetContainer(HWND ParentWin, SnazzyWidgetEventObserver *Observer, SnazzyWidgetImageCollection *Images);
	}; // SnazzyWidgetContainerFactory

// SnazzyWidgetContainer is a widget that fills a given space (like an entire Window)
// From the perspective of the Operating/Windowing system, it is one Widget.
// However, within its space can exist numerous SnazzyWidgets, which each behave like
// individual Widgets. This allows Widgets to appear to have non-rectangular bounds,
// and overlap in organic ways.
// SnazzyWidgetContainer is also a Factory for creating SnazzyWidgets, and is the only
// legal way to create them.
// SnazzyWidgetContainer throws std::bad_alloc during construction for dynamic errors
class SnazzyWidgetContainer
	{
	friend class SnazzyWidgetContainerFactory;
	friend class SnazzyWidget;
	friend class SnazzyCheckWidget;
	friend class SnazzyPushWidget;
	friend class SnazzyDragWidget;
	friend class SnazzySlideYWidget;
	friend class SnazzySlideXWidget;
	friend class SnazzySlideXYWidget;
	friend class SnazzyHotPadWidget;
	friend class SnazzyTextWidget;
	
	PROPERTY_R(unsigned long int, ImageWidth);
	PROPERTY_R(unsigned long int, ImageHeight);
	PROPERTY_R(unsigned long int, CurrentFocusedWidgetNum);
	PROPERTY_R(unsigned long int, DragInProgress);
	private:
		HINSTANCE _CachedInstance;
		HWND _ContainerNativeWin;
		SnazzyWidgetEventObserver *_EventObserver;
		SnazzyDragWidget *_DragWidget; // only one per window, please
		SnazzyWidget *_ActiveSlideWidget; // only one can be active at a time
		// the collection of widgets in this container, sorted by WidgetNum
		// ID 0 is not used, slight waste of resources, but it makes more sense that way
		std::vector<SnazzyWidget *> _WidgetCollection;
		SnazzyWidgetImageCollection *_Images; // we don't free this ourselves
		std::vector<unsigned char> _FinalImage[3], _ImageState; // _ImageState 0:Normal, 1:NormalHover, 2:Sel, 3:SelHover

		// private, so we can only be created from a SnazzyWidgetContainerFactory
		SnazzyWidgetContainer(const HINSTANCE & Instance, const HWND & ParentWin, SnazzyWidgetEventObserver *Observer, SnazzyWidgetImageCollection *Images);

		// safe way to request a widget by WidgetNum
		SnazzyWidget *GetWidgetByNum(const unsigned char & WidgetNum) const;

		// Used by WndProc WM_PAINT
		unsigned char *GetFinalImageR(void) {return(&_FinalImage[0][0]);};
		unsigned char *GetFinalImageG(void) {return(&_FinalImage[1][0]);};
		unsigned char *GetFinalImageB(void) {return(&_FinalImage[2][0]);};
		
		// Lookup a widget ID from a pixel coordinate
		// returns 0 for nothing found
		unsigned char GetWidgetNumFromXY(const unsigned long int & X, const unsigned long int &Y) const;
		
		// un-focus all widgets
		// returns number that were previously focused (normally 0 or 1)
		unsigned long int ClearAllFocus(void);
		
		// update the visual state of a widget
		void UpdateWidgetVisualState(SnazzyWidget *UpdateWidget);
		
		// Lookup a Widget pointer from a pixel coordinate
		// returns NULL for nothing found
		SnazzyWidget *GetSnazzyWidgetFromXY(const unsigned long int & X, const unsigned long int & Y) const;
		
		// Register a widget as the active Slide Widget
		void SetActiveSlideWidget(SnazzyWidget *NewActiveSlideWidget) {_ActiveSlideWidget = NewActiveSlideWidget;};
		SnazzyWidget *GetActiveSlideWidget(void) const {return(_ActiveSlideWidget);};
	
	public:
		~SnazzyWidgetContainer();

		// entrypoint for event handling within C++ code
		long SnazzyWidgetWndProcInternal(HWND hwnd, UINT message, UINT wParam, LONG lParam);

		// this should be used by anyone wanting only to check the validity of the allocation of the ContainerNativeWin
		bool GetContainerNativeWinOk(void) const {return(_ContainerNativeWin != NULL);};

		// This is used by the Snazzy*Widget friend classes to generate
		// events back to the real Window hosting the widgets
		// It can also be used to get the Window handle for positional changes.
		HWND GetContainerNativeWin(void) const {return(_ContainerNativeWin);};
		

		void Show(const bool & Shown) const {if(GetContainerNativeWinOk()) ShowWindow(GetContainerNativeWin(), Shown ? SW_SHOW : SW_HIDE);};

		// Force the windowing system to request us to repaint
		// Useful after multiple (silent) widget state updates/changes
		void ForceRepaintAll(void);
		
		SnazzyCheckWidget *CreateSnazzyCheckWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption = "");
		SnazzyPushWidget *CreateSnazzyPushWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption = "");
		SnazzyDragWidget *CreateSnazzyDragWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption = "");
		SnazzySlideXYWidget *CreateSnazzySlideXYWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption = "");
		SnazzySlideXWidget *CreateSnazzySlideXWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption = "");
		SnazzySlideYWidget *CreateSnazzySlideYWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption = "");
		SnazzyHotPadWidget *CreateSnazzyHotPadWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption = "");
		SnazzyTextWidget *CreateSnazzyTextWidget(const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption = "");
	}; // SnazzyWidgetContainer

// This is the root class that defines the common interface for all SnazzyWidget widgets
class SnazzyWidget
	{
	friend class SnazzyWidgetContainer;
	friend class SnazzyCheckWidget;
	friend class SnazzyPushWidget;
	friend class SnazzyDragWidget;
	friend class SnazzySlideYWidget;
	friend class SnazzySlideXWidget;
	friend class SnazzySlideXYWidget;
	friend class SnazzyTextWidget;
	friend class SnazzyHotPadWidget;
	private:
		bool _Focused;
		
		SnazzyWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int &WidgetID, std::string HelpCaption);
		SnazzyWidgetContainer *_Host;
		bool _Selected;
		
		// determine pixel bounds of region where this widget's WidgetNum is found
		void UpdateBounds(void);

		// Event-handling is only called from the Container WndProc
		// These don't HAVE to be overridden if you don't need to implement them all
		virtual void HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY) {};
		virtual void HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY) {};
		virtual void HandleMouseMove(const signed long int & MouseX, const signed long int & MouseY) {};

		// This is called after the main widget draws the basic imagery
		// based on the widget state
		// Code in this method can add extra decoration on top of that.
		virtual void PerformRedraw(void) {};
		
		// This one is called after the main widget does its blit, to perform GDI-level
		// drawing on top of it (as opposed to PerformRedraw, above, which just draws into the
		// blit source buffer)
		virtual void PerformPaint(void) {};

	public:
		~SnazzyWidget() {};
		
		bool GetFocused(void) const {return(_Focused);};
		bool GetSelected(void) const {return(_Selected);};

		virtual void SetFocused(const bool & NewFocus) {_Focused = NewFocus; if(NewFocus) _Host->_CurrentFocusedWidgetNum = _WidgetNum; else if(_Host->_CurrentFocusedWidgetNum == _WidgetNum) _Host->_CurrentFocusedWidgetNum = NULL;};
		// causes an immediate repaint
		virtual void SetSelected(const bool & NewSelected);
		// doesn't repaint automatically
		virtual void SetSelectedSilent(const bool & NewSelected);
		
		signed long int GetWidth(void) const {return(GetLRX() - GetULX());};
		signed long int GetHeight(void) const {return(GetLRY() - GetULY());};

		// Properties
		PROPERTY_RW(bool, Enabled)
		//PROPERTY_RW(bool, Selected)
		PROPERTY_RW(unsigned long int, WidgetID)
		PROPERTY_RW(unsigned char, WidgetNum)
		PROPERTY_RW(std::string, HelpCaption)

		PROPERTY_R(signed long int, ULX)
		PROPERTY_R(signed long int, ULY)
		PROPERTY_R(signed long int, LRX)
		PROPERTY_R(signed long int, LRY)

	}; // SnazzyWidget

class SnazzyCheckWidget : public SnazzyWidget
	{
	friend class SnazzyWidgetContainer;
	private:
		SnazzyCheckWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption);

		void HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY);
	
	public:
		~SnazzyCheckWidget() {};

	}; // SnazzyCheckWidget

class SnazzyPushWidget : public SnazzyWidget
	{
	friend class SnazzyWidgetContainer;
	private:
		SnazzyPushWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption);

		void HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY);
		void HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY);
	
	public:
		~SnazzyPushWidget() {};
		virtual void SetFocused(const bool & NewFocus) {_Focused = NewFocus; SetSelected(false); _Host->_CurrentFocusedWidgetNum = _WidgetNum;};

	}; // SnazzyPushWidget

class SnazzyDragWidget : public SnazzyWidget
	{
	friend class SnazzyWidgetContainer;

		PROPERTY_R(signed long int, ClickOriginX)
		PROPERTY_R(signed long int, ClickOriginY)

	private:
		SnazzyDragWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption);

		void HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY);
		void HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY);
		void HandleMouseDrag(void); // called from container for drag ops, gets its own screen coords
		void InitiateDrag(signed long int MouseOriginX, signed long int MouseOriginY); // called from HandleLButtonDown
		void CancelDrag(void); // called from HandleLButtonUp and possibly HandleMouseDrag

	
	public:
		~SnazzyDragWidget();

	}; // SnazzyDragWidget



class SnazzySlideXYWidget : public SnazzyWidget
	{
	friend class SnazzyWidgetContainer;

		PROPERTY_RW(signed long int, PositionX)
		PROPERTY_RW(signed long int, PositionY)

	private:
		SnazzyWidgetImage *_KnobImage;

		SnazzySlideXYWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption);

		void HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY);
		void HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY);
		void HandleMouseMove(const signed long int & MouseX, const signed long int & MouseY);

	
	public:
		~SnazzySlideXYWidget();

	}; // SnazzySlideXYWidget




class SnazzySlideXWidget : public SnazzyWidget
	{
	friend class SnazzyWidgetContainer;

		PROPERTY_RW(signed long int, PositionX)
		

	private:
		SnazzyWidgetImage *_KnobImage;

		SnazzySlideXWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption);

		void HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY);
		void HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY);
		void HandleMouseMove(const signed long int & MouseX, const signed long int & MouseY);

	
	public:
		~SnazzySlideXWidget();

	}; // SnazzySlideXWidget



class SnazzySlideYWidget : public SnazzyWidget
	{
	friend class SnazzyWidgetContainer;

	private:
		SnazzyWidgetImage *_KnobImage;
		float _PositionY;
	
		SnazzySlideYWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption);

		void HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY);
		void HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY);
		void HandleMouseMove(const signed long int & MouseX, const signed long int & MouseY);
		void PerformRedraw(void);
		void HandleNewPosition(const signed long int & MouseX, const signed long int & MouseY);


	
	public:
		~SnazzySlideYWidget();
		float GetPositionY(void) const {return(_PositionY);};
		void SetPositionY(const float & NewPositionY) {_PositionY = NewPositionY; _Host->UpdateWidgetVisualState(this); _Host->ForceRepaintAll();};

	}; // SnazzySlideYWidget



class SnazzyTextWidget : public SnazzyWidget
	{
	friend class SnazzyWidgetContainer;
	private:
		std::string _Text;
		SnazzyTextWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, std::string HelpCaption);
		void PerformPaint(void); // actually rasterize the text

	public:
		~SnazzyTextWidget() {};
		void SetText(const char *NewText);

	}; // SnazzyTextWidget



class SnazzyHotPadWidget : public SnazzyWidget
	{
	friend class SnazzyWidgetContainer;

		PROPERTY_RW(bool, Exponential)

	private:
		float _PositionY, _PositionX;
		SnazzyWidgetImage *_KnobImage;

		SnazzyHotPadWidget(SnazzyWidgetContainer *Host, const unsigned char & WidgetNum, const unsigned long int & WidgetID, SnazzyWidgetImage *Image, std::string HelpCaption);

		void HandleLButtonDown(const signed long int & MouseX, const signed long int & MouseY);
		void HandleLButtonUp(const signed long int & MouseX, const signed long int & MouseY);
		void HandleMouseMove(const signed long int & MouseX, const signed long int & MouseY);
		void PerformRedraw(void);
		void HandleNewPosition(const signed long int & MouseX, const signed long int & MouseY);

	public:
		~SnazzyHotPadWidget() {};

		float GetPositionX(void) const {return(_PositionX);};
		void SetPositionX(const float & NewPositionX) {_PositionX = NewPositionX; _Host->UpdateWidgetVisualState(this); _Host->ForceRepaintAll();};
		float GetPositionY(void) const {return(_PositionY);};
		void SetPositionY(const float & NewPositionY) {_PositionY = NewPositionY; _Host->UpdateWidgetVisualState(this); _Host->ForceRepaintAll();};
	
	}; // SnazzyHotPadWidget





// This is just a helper/bundle class to hold a couple pieces of related
// data. It's essentially just a structure, but it has memory management in the
// form of allocation and deallocation of image data
// It will throw a std::bad_alloc from the constructor if allocation at the
// desired size fails
/*
try
	{
	} // try
catch(std::bad_alloc)
	{
	} // catch
*/

class SnazzyWidgetImageCollection
	{
		PROPERTY_R(unsigned long int, Width);
		PROPERTY_R(unsigned long int, Height);
		PROPERTY_R(unsigned long int, NumPixels);

	private:
		std::vector<unsigned char> _Normal[3], _NormalHover[3], _Selected[3], _SelectedHover[3], _Index;
		bool _NormalValid, _NormalHoverValid, _SelectedValid, _SelectedHoverValid;

	public:
		SnazzyWidgetImageCollection(const unsigned long int & Width, const unsigned long int & Height);
		unsigned char *GetR_Normal(void) {return(&_Normal[0][0]);};
		unsigned char *GetG_Normal(void) {return(&_Normal[1][0]);};
		unsigned char *GetB_Normal(void) {return(&_Normal[2][0]);};
		unsigned char *GetR_NormalHover(void) {return(&_NormalHover[0][0]);};
		unsigned char *GetG_NormalHover(void) {return(&_NormalHover[1][0]);};
		unsigned char *GetB_NormalHover(void) {return(&_NormalHover[2][0]);};
		unsigned char *GetR_Selected(void) {return(&_Selected[0][0]);};
		unsigned char *GetG_Selected(void) {return(&_Selected[1][0]);};
		unsigned char *GetB_Selected(void) {return(&_Selected[2][0]);};
		unsigned char *GetR_SelectedHover(void) {return(&_SelectedHover[0][0]);};
		unsigned char *GetG_SelectedHover(void) {return(&_SelectedHover[1][0]);};
		unsigned char *GetB_SelectedHover(void) {return(&_SelectedHover[2][0]);};
		unsigned char *Get_Index(void) {return(&_Index[0]);};
		
		bool GetNormalValid(void) const {return(_NormalValid);};
		bool GetNormalHoverValid(void) const {return(_NormalHoverValid);};
		bool GetSelectedValid(void) const {return(_SelectedValid);};
		bool GetSelectedHoverValid(void) const {return(_SelectedHoverValid);};
		
		void SetNormalValid(bool NewState) {_NormalValid = NewState;};
		void SetNormalHoverValid(bool NewState) {_NormalHoverValid = NewState;};
		void SetSelectedValid(bool NewState) {_SelectedValid = NewState;};
		void SetSelectedHoverValid(bool NewState) {_SelectedHoverValid = NewState;};

		unsigned long int Subscript(const unsigned long int & X, const unsigned long int & Y) const {return(X + (Y * _Width));};
		//~SnazzyWidgetImageCollection(); // default destructor is fine
	}; // SnazzyWidgetImageCollection


// this is a single-image-layer version of the above, and it can handle alpha channels
// commonly used for slider knobs, which can be semi-transparent and non-rectangular
class SnazzyWidgetImage
	{
		PROPERTY_R(unsigned long int, Width);
		PROPERTY_R(unsigned long int, Height);
		PROPERTY_R(unsigned long int, NumPixels);

	private:
		std::vector<unsigned char> _Normal[4]; // we have alpha too

	public:
		SnazzyWidgetImage(const unsigned long int & Width, const unsigned long int & Height);
		unsigned char *GetR_Normal(void) {return(&_Normal[0][0]);};
		unsigned char *GetG_Normal(void) {return(&_Normal[1][0]);};
		unsigned char *GetB_Normal(void) {return(&_Normal[2][0]);};
		unsigned char *GetA_Normal(void) {return(&_Normal[3][0]);};
		
		unsigned long int Subscript(const unsigned long int & X, const unsigned long int & Y) const {return(X + (Y * _Width));};
	}; // SnazzyWidgetImage






// This is a functor (function object) base class for collecting Widget events from
// the SnazzyWidget. Derive a class from this and implement any local storage members
// and an actual operator() to do whatever you wish
class SnazzyWidgetEventObserver
	{
	public:
		virtual void Handle(unsigned long int WidgetID) = 0;
	}; // SnazzyWidgetEventObserver


#endif // SNAZZYWIDGET_H
