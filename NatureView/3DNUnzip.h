// 3DNUnzip.h
// extra function protos for unzipping
// Created from scratch on 10/29/03 by CXH

#include "unzip.h"

char *FindFileExtensionInZip(unzFile uf, char *Extension);
char *CheckForFileExtensionInZIP(const char *ZipFileName, char *Extension);

// returns number of files extracted
int ExtractAllFilesFromZip(const char *ZipFileName);
