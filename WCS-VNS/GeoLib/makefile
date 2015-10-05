# GCC makefile for GeoLib

CFG=GeoLib - Win32 GCC Release

# build configs supported:
# "GeoLib - Win32 GCC Release"
# "GeoLib - Win32 GCC Debug"

CXX = g++
C++FLAGS = -g -Wall -D_WIN32 -fno-const-strings
OURDEFS = -DNDEBUG -DBYTEORDER_LITTLEENDIAN -DWCS_BUILD_W6 -DNOSERVICE -DNOMCX -DNOIME -DWIN32_LEAN_AND_MEAN -DVC_EXTRALEAN -DWCS_ENABLE_COORDSCALC
ADDFLAGS = -mcpu=pentium2 -mpreferred-stack-boundary=4 -mieee-fp -isystem "C:\Program Files\Microsoft Visual Studio\VC98\include"

LIB_FILES = GeoLib.cpp alberfor.cpp alberinv.cpp alconfor.cpp alconinv.cpp azimfor.cpp aziminv.cpp cproj.cpp eqconfor.cpp eqconinv.cpp \
	equifor.cpp equiinv.cpp for_init.cpp gnomfor.cpp gnominv.cpp goodfor.cpp goodinv.cpp gvnspfor.cpp gvnspinv.cpp hamfor.cpp haminv.cpp imolwfor.cpp \
	imolwinv.cpp inv_init.cpp lamazfor.cpp lamazinv.cpp lamccfor.cpp lamccinv.cpp merfor.cpp merinv.cpp millfor.cpp millinv.cpp molwfor.cpp \
	molwinv.cpp nzmgfor.cpp nzmginv.cpp obleqfor.cpp obleqinv.cpp omerfor.cpp omerinv.cpp orthfor.cpp orthinv.cpp paksz.cpp polyfor.cpp polyinv.cpp \
	psfor.cpp psinv.cpp report.cpp robfor.cpp robinv.cpp sinfor.cpp sininv.cpp somfor.cpp sominv.cpp sphdz.cpp sterfor.cpp sterinv.cpp stplnfor.cpp \
	stplninv.cpp tmfor.cpp tminv.cpp untfz.cpp utmfor.cpp utminv.cpp vandgfor.cpp vandginv.cpp wivfor.cpp wivinv.cpp wviifor.cpp wviiinv.cpp

# GCC Release Config
ifeq ($(CFG),GeoLib - Win32 GCC Release)

OUTDIR=.\GeoLib___Win32_GCC_Release
INTDIR=.\GeoLib___Win32_GCC_Release

else

OUTDIR=.\GeoLib___Win32_GCC_Debug
INTDIR=.\GeoLib___Win32_GCC_Debug

endif

#
LIBOFILES = $(LIB_FILES:%.cpp=%.o)

#
#ALL : libGeoLib.a"

%.o: %.cpp
	$(CXX) $(C++FLAGS) $(OURDEFS) $(ADDFLAGS) -c $<

#
libGeoLib.a: $(LIBOFILES)
	rm -f $@
	ar cq $@ $(LIBOFILES)
	ranlib libGeoLib.a
