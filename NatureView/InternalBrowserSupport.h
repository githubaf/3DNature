// InternalBrowserSupport.h
// Code below here taken from Simple.c from http://www.codeproject.com/com/cwebpage.asp
// Written by Jeff Glatt jglatt@borg.com
// Modified somewhat by CXH, 2/15/05

#ifndef NVW_INTERNALBROWSERSUPPORT_H
#define NVW_INTERNALBROWSERSUPPORT_H


// actions for DoPageAction()
#define NVW_INTERNALBROWSERSUPPORT_GOBACK		0
#define NVW_INTERNALBROWSERSUPPORT_GOFORWARD	1
#define NVW_INTERNALBROWSERSUPPORT_GOHOME		2
#define NVW_INTERNALBROWSERSUPPORT_SEARCH		3
#define NVW_INTERNALBROWSERSUPPORT_REFRESH		4
#define NVW_INTERNALBROWSERSUPPORT_STOP		5


int InitHTMLEmbed(void);
void CleanupHTMLEmbed(void);
long EmbedBrowserObject(HWND hwnd, RECT *ClientPosition); // ClientPosition=NULL means use entire area
void UnEmbedBrowserObject(HWND hwnd);
void ResizeBrowser(HWND hwnd, DWORD width, DWORD height);
void ResizeBrowserRECT(HWND hwnd, RECT Position); // right and bottom are RELATIVE width and height not abs coords
long DisplayHTMLPage(HWND hwnd, LPTSTR webPageName);
long DisplayHTMLStr(HWND hwnd, LPCTSTR string);
void DoPageAction(HWND hwnd, DWORD action);
long FetchTitle(HWND hwnd, char *TitleBuffer, int TitleBufferSize);
long FetchURL(HWND hwnd, char *URLBuffer, int URLBufferSize);
long SetToolbar(HWND hwnd, int ToolbarState);

// called from browser object, implemented by user-code outside of InternalBrowserSupport.c
void StatusTextUpdateCallback(LPCOLESTR pszStatusText);


#endif // NVW_INTERNALBROWSERSUPPORT_H

