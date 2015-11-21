// NVParsing.h
// Code to read and parse NVW scene files
// mostly broken off of main.cpp on 1/31/06 by CXH


#ifndef NVW_PARSING_H
#define NVW_PARSING_H

#include <osg/Vec4>

unsigned long int ReadScene(const char *OrigSceneName);

bool ParseBooleanPresenceIsTrue(char *InputString);

osg::Vec4 DecodeHTMLHexColor(char *InputText);
osg::Vec4 DecodeColor(char *InputText);

int HexToDecDigit(unsigned char InputDigit);

#endif // !NVW_PARSING_H
