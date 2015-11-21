// NVWidgets.h
// Oh, shocker, I have to write custom widgets to get around limitations in Windows' basic widget set
// Created originally from parts of WCSWidgets on 2/15/05 by CXH

#ifndef NVW_NVWIDGETS_H
#define NVW_NVWIDGETS_H

#include <windows.h>

int RegisterNVWidgets(HINSTANCE Instance);
void UnRegisterNVWidgets(HINSTANCE Instance);

#endif // NVW_NVWIDGETS_H
