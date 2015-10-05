/* ScratchPad.c
** Stuff to handle an off-display rendering area
** created from scratch on 24 Feb 1994 by Chris "Xenon" Hanson
** Subsequently hacked to support VGL on 04 Mar 1994
** Copyright 1994 by Chris Hanson
*/

#include "WCS.h"
#include "vgl.h"

void ScratchRast_CornerTurn(struct vgl_pixmap *This, struct RastPort *ScratchRast)
{

HK4M(ScratchRast->BitMap->Planes[0], ScratchRast->BitMap->Planes[1],
 ScratchRast->BitMap->Planes[2], ScratchRast->BitMap->Planes[3],
 ScratchRast->BitMap->Planes[7], /* Mask */
 ((unsigned long)(ScratchRast->RP_User) * ScratchRast->BitMap->Rows),
 This->pixdata->data);

} /* ScratchRast_CornerTurn() */



struct RastPort *ScratchRast_New(int Width, int Height, char Planes)
{
struct RastPort *This;
int loop;

/* Planes must be < 8, since we steal the topmost entry for our Mask */

if(Planes > 7)
	return(NULL); /* Zo zorry. */

Width = ROUNDUP(Width,32);

if(This=AllocMem(sizeof(struct RastPort), MEMF_CLEAR))
	{
	InitRastPort(This);
	This->RP_User=(void *)Width; /* cope with it. */
	if(This->BitMap = AllocMem(sizeof(struct BitMap), MEMF_CLEAR))
		{
		InitBitMap(This->BitMap, Planes, Width, Height);
		for(loop = 0; loop < Planes; loop++)
			{
			if(!(This->BitMap->Planes[loop] = AllocRaster(Width, Height)))
				{
				break;
				} /* if */
			} /* for */
		if(loop == Planes)
			{
			This->BitMap->Planes[7] = AllocRaster(Width, Height);
			} /* if */
		if(loop != Planes) /* aborted early */
			{
			for(loop = 0; loop < Planes; loop++)
				{
				if(This->BitMap->Planes[loop])
					{
					FreeRaster(This->BitMap->Planes[loop], Width, Height);
					This->BitMap->Planes[loop] = NULL;
					} /* if */
				} /* for */
			} /* if */
		else /* Success! Rice */
			{
			return(This);
			} /* else */
		FreeMem(This->BitMap, sizeof(struct BitMap));
		} /* if */
	FreeMem(This, sizeof(struct RastPort));
	} /* if */

return(NULL);
} /* ScratchRast_New() */

void ScratchRast_Del(struct RastPort *This)
{
int loop;

if(This)
	{
	if(This->BitMap)
		{
		if(This->BitMap->Planes[7])
			{
			FreeRaster(This->BitMap->Planes[7], (int)This->RP_User, This->BitMap->Rows);
			This->BitMap->Planes[7] = NULL;
			} /* if */
		for(loop = 0; loop < This->BitMap->Depth; loop++)
			{
			if(This->BitMap->Planes[loop])
				{
				FreeRaster(This->BitMap->Planes[loop], (int)This->RP_User, This->BitMap->Rows);
				} /* if */
			} /* for */
		FreeMem(This->BitMap, sizeof(struct BitMap));
		} /* if */
	FreeMem(This, sizeof(struct RastPort));
	} /* if */

} /* ScratchRast_Del() */

