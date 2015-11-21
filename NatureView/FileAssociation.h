// FileAssociation.h
// Code (primarily from WCS's Application.cpp) to associate a file extension with out executable

int SetupFileAssociation(char *Extension, bool Force);
int FindExePathAndName(char *ProgName, char *ProgDir);
