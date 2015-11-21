// Credits.h
// Required and recommended credits
// Created from scratch on 2/17/05 by CXH

#ifndef NVW_CREDITS_H
#define NVW_CREDITS_H

#include "IdentityDefines.h"

// Used for Notice UI, which uses same framework (HTMLDlg) as Credits
void SetNoticeDisplayed(bool NewState);
bool GetNoticeDisplayed(void);

int DisplayCredits(void);
void PrepCreditsImage(char *TempFilePath); // creates Logo temp file
void CleanupCredits(void); // removes temp files

#endif // NVW_CREDITS_H