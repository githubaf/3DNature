// ImageDlg.h
// Interfaces to image-display dialog

// Black box...

#ifndef NVW_IMAGEDLG_H
#define NVW_IMAGEDLG_H

#include <windows.h>
#include "Types.h"


class ImageDlg
	{
	private:
		static NativeGUIWin DlgHandle;
	public:

		ImageDlg(bool Show);
		~ImageDlg();

		static int ProcessAndHandleEvents();
		static int SetDataText(const char *NewText);
		static void Show(bool NewState = true);
	}; // ImageDlg


#endif // !NVW_IMAGEDLG_H
