// FSSupport.h
// Filesystem-related operations
// gathered here from various odd places it started out.

// Hunt for a CD-ROM with a volume name beginning with "WCSV"
char *FindCDRoot(void);

// this can be an expensive operation, so don't use too lightly
// It also doesn't always return a useful answer, -1 means "don't know"
int IsNetworkPath(char *InputPath);
