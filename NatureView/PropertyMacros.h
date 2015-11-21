// PropertyMacros.h
// Macros for managing data members as properties
// with automatic accessor methods (setters/getters)
// Basically copied and cleaned up from public-domain sources on the Internet
// http://users.pandora.be/bart.demeyere/WritingMacros.html
// no new copyright is claimed
// 10/26/04 by CXH

#ifndef PROPERTYMACROS_H
#define PROPERTYMACROS_H

// remember that using PROPERTY macros changes the
// access qualifier (PUBLIC/PROTECTED/PRIVATE) in
// effect at that point in the code

// read-write property
#define PROPERTY_RW(paType, paName) \
  private: \
    paType _ ## paName; \
  public: \
    paType Get ## paName(void) const \
    { return _ ## paName; } \
    void Set ## paName(const paType & paName) \
    { _ ## paName = paName; }

// read-only property
#define PROPERTY_R(paType, paName) \
  private: \
    paType _ ## paName; \
  public: \
    paType Get ## paName(void) const \
    { return _ ## paName; } \
  private: \
    void Set ## paName(const paType & paName) \
    { _ ## paName = paName; }

// write-only property (?)
#define PROPERTY_W(paType, paName) \
  private: \
    paType _ ## paName; \
  public: \
    void Set ## paName(const paType & paName) \
    { _ ## paName = paName; } \
  private: \
    paType Get ## paName(void) const \
    { return _ ## paName; }

#endif // PROPERTYMACROS_H