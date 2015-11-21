// StringSupport.h
// Support code for std::string objects
// Created from misc resources on 2/7/05 by CXH
// Sources credited where non-original

#ifndef NVW_STRINGSUPPORT_H
#define NVW_STRINGSUPPORT_H

#include <string>

void Trim(std::string& str, const std::string & ChrsToTrim = " \t\n\r", int TrimDir = 0);
void TrimRight(std::string& str, const std::string & ChrsToTrim = " \t\n\r");
void TrimLeft(std::string& str, const std::string & ChrsToTrim = " \t\n\r");

#endif // NVW_STRINGSUPPORT_H

