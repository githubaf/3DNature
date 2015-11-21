// StringSupport.cpp
// Support code for std::string objects
// Created from misc resources on 2/7/05 by CXH
// Sources credited where non-original

#include "StringSupport.h"

// from http://www.experts-exchange.com/Programming/Programming_Languages/Cplusplus/Q_21282559.html

void Trim(std::string& str, const std::string & ChrsToTrim, int TrimDir)
{
size_t startIndex = str.find_first_not_of(ChrsToTrim);
if (startIndex == std::string::npos){str.erase(); return;}
if (TrimDir < 2) str = str.substr(startIndex, str.size()-startIndex);
if (TrimDir!=1) str = str.substr(0, str.find_last_not_of(ChrsToTrim) + 1);
} // Trim

void TrimRight(std::string& str, const std::string & ChrsToTrim)
{
Trim(str, ChrsToTrim, 2);
} // TrimRight

void TrimLeft(std::string& str, const std::string & ChrsToTrim)
{
Trim(str, ChrsToTrim, 1);
} // TrimLeft