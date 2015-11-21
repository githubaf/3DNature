// NVSigCheck.h

#ifndef NVW_NVSIGCHECK_H
#define NVW_NVSIGCHECK_H

void InitSigCaches(void);

bool CheckDependentBinaryFileSignature(const char *InputSig, const char *FileNameAndPath);

bool CheckNVWFileSignature(const char *FileNameAndPath);

void SetGlobalSigInvalid(bool NewState);
bool GetGlobalSigInvalid(void);

#endif // NVW_NVSIGCHECK_H