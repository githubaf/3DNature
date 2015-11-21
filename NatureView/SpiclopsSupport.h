// SpiclopsSupport.h
// Code to support eLumens dome systems via their spiclops DLL
// Some code taken from osgspiclops.cpp by Farshid Lashkari

#ifndef NVW_SPICLOPSSUPPORT_H
#define NVW_SPICLOPSSUPPORT_H

#ifdef NVW_SUPPORT_SPICLOPS
#include "spiclopsExt.h"

#include <osgProducer/Viewer>

unsigned long int GetSPIChannels(void);
unsigned long int GetSPIMode(void);
bool GetSPIEnabled(void);
void SetSPIChannels(unsigned long int NewChannels);
void SetSPIMode(unsigned long int NewMode);
void SetSPIModeFromString(const char *ModeString);
void SetSPIEnabled(bool NewEnabled);

bool QuerySPIDLLAvailable(void);

// not  used currently
//unsigned long int MapChannelNumToChannelVal(unsigned long int ChannelNum);

void initSpiclops(void *context, int numChannels, osgProducer::Viewer *viewer);

// example of how we did this in WCS's Fenetre.cpp
// typedef WINGDIAPI void (APIENTRY * DGLBEGIN)(GLenum);
typedef SPIDLL_API void* (MSFTCALLCONV * DLLSPIINITIALIZE)(void*, int);
typedef SPIDLL_API void  (MSFTCALLCONV * DLLSPIPRERENDER)(void*, int);
typedef SPIDLL_API void  (MSFTCALLCONV * DLLSPIPOSTRENDER)(void*, int);
typedef SPIDLL_API void  (MSFTCALLCONV * DLLSPIFLUSH)(void*, int);
typedef SPIDLL_API void  (MSFTCALLCONV * DLLSPIBEGIN)(void*);
typedef SPIDLL_API void  (MSFTCALLCONV * DLLSPIEND)(void*);
typedef SPIDLL_API int   (MSFTCALLCONV * DLLSPISETCHANTEXTUREID) ( void* _s, int _wall, int _id );


typedef SPIDLL_API void  (MSFTCALLCONV * DLLSPISETCHANORIGIN)(void*, int, int, int);
typedef SPIDLL_API int   (MSFTCALLCONV * DLLSPISETCHANSIZE)(void*, int, int, int);
typedef SPIDLL_API void	 (MSFTCALLCONV * DLLSPISETCHANFOV)(void*, int, float,  float);
typedef SPIDLL_API void  (MSFTCALLCONV * DLLSPISETCHANTESSLEVEL)(void*, int, int);

// class for support of optional late-binding of SPI DLL
class SPIDLLLoader
	{
	private:
		HMODULE SPIDLLHM;
		bool InitOK;
	public:
		SPIDLLLoader();
		~SPIDLLLoader();
		// SPIDLL_API void* MSFTCALLCONV spiInitialize  ( void* _s, int _pformat );
		DLLSPIINITIALIZE dllspiInitialize;
		// SPIDLL_API void  MSFTCALLCONV spiPreRender   ( void* _s, int _wall );
		DLLSPIPRERENDER dllspiPreRender;
		// SPIDLL_API void  MSFTCALLCONV spiPostRender  ( void* _s, int _wall );
		DLLSPIPOSTRENDER dllspiPostRender;
		// SPIDLL_API void  MSFTCALLCONV spiFlush       ( void* _s, int _wall );
		DLLSPIFLUSH dllspiFlush;
		// SPIDLL_API void  MSFTCALLCONV spiBegin       ( void* _s );
		DLLSPIBEGIN dllspiBegin;
		// SPIDLL_API void  MSFTCALLCONV spiEnd         ( void* _s );
		DLLSPIEND dllspiEnd;
		// SPIDLL_API int   MSFTCALLCONV spiSetChanTextureID ( void* _s, int _wall, int _id );
		DLLSPISETCHANTEXTUREID dllspiSetChanTextureID;
		
		// SPIDLL_API void  MSFTCALLCONV spiSetChanOrigin    ( void* _s, int _wall, int x, int y );
		DLLSPISETCHANORIGIN dllspiSetChanOrigin;
		// SPIDLL_API int   MSFTCALLCONV spiSetChanSize      ( void* _s, int _wall, int _w, int _h );
		DLLSPISETCHANSIZE dllspiSetChanSize;
		// SPIDLL_API void	 MSFTCALLCONV spiSetChanFOV       ( void* _s, int _wall, float _fovH,  float _fovV );
		DLLSPISETCHANFOV dllspiSetChanFOV;
		// SPIDLL_API void  MSFTCALLCONV spiSetChanTessLevel ( void* _s, int _wall, int _tl );
		DLLSPISETCHANTESSLEVEL dllspiSetChanTessLevel;
		
		bool GetInitOK(void) {return(InitOK);};
	}; // SPIDLLLoader

// Realize callback for creating a SPI context
class InitSpiclopsCallback : public Producer::RenderSurface::Callback
{
public:
	InitSpiclopsCallback(int channels,int format) : m_context(NULL),m_numChannels(channels),m_pixelFormat(format) {}

	virtual void operator()( const Producer::RenderSurface &rs );

	void* getContext() { return m_context; }

protected:
	void *m_context;
	int m_numChannels;
	int m_pixelFormat;
}; 



#endif // NVW_SUPPORT_SPICLOPS


#endif // NVW_SPICLOPSSUPPORT_H

