
#include <osg/Image>

#ifndef NVW_INTERNALIMAGE_H
#define NVW_INTERNALIMAGE_H

osg::Image *ExportAndLoadImage(char *TempName, unsigned char *DataBlock, unsigned long int DataBlockSize);
bool ExportImage(char *TempName, unsigned char *DataBlock, unsigned long int DataBlockSize, char *TempFilePath);
void CleanupExportedImage(char *TempName);


#endif // !NVW_INTERNALIMAGE_H