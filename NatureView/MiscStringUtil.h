// MiscStringUtil.h

#ifndef MISCSTRINGUTIL_H
#define MISCSTRINGUTIL_H

#include <vector>
#include <string>


// string tokenizer
// from http://www.linuxselfhelp.com/HOWTO/C++Programming-HOWTO-7.html
void Tokenize(const std::string& str,
                      std::vector<std::string>& tokens,
                      const std::string& delimiters = " ");

#endif // MISCSTRINGUTIL_H
