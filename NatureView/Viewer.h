
#ifndef NVW_VIEWER_H
#define NVW_VIEWER_H

#include <windows.h>
#include <osgProducer/Viewer>

osgProducer::Viewer *GetGlobalViewer(void);
void SetGlobalViewer(osgProducer::Viewer *NewViewer);
HWND GetGlobalViewerHWND(void);
void SignalApplicationExit(void);
// returns false for failure
bool GetGlobalViewerDims(unsigned long int &Width, unsigned long int &Height);

// returns 0 for failure
unsigned int GetGlobalViewerWidth(void);
// returns 0 for failure
unsigned int GetGlobalViewerHeight(void);

// returns false for failure
bool GetGlobalViewerCoords(signed long int &X, signed long int &Y);
// returns 0 for failure
signed int GetGlobalViewerX(void);
// returns 0 for failure
signed int GetGlobalViewerY(void);

#endif // !NVW_VIEWER_H
