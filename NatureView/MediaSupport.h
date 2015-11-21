// MediaSupport.h
// Routines for playing various media
// Created from scratch by CXH
// Copyright 2005

#ifndef NVW_MEDIASUPPORT_H
#define NVW_MEDIASUPPORT_H

bool PlaySoundAsync(const char *SoundFilePath);
void CancelSounds(void);
void CleanupSounds(void);

int OpenURLExternally(const char *URL);
int OpenURLInternally(const char *URL);
int OpenImageInternally(const char *URL);
int OpenMediaInternally(const char *URL);
bool TestFileExecutable(const char *Input);
bool TestFileKnownImage(const char *Input);
bool TestFileKnownSound(const char *Input);
bool TestFileKnownMedia(const char *Input);

bool TestFileKnownExtension(const char *Input, char *ExtTable[]);

bool ValidateURL(const char *Input, bool PermitLocalWebImageFormats = false);

#endif // NVW_MEDIASUPPORT_H
