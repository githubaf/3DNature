/*
 * AF HGW, 2.Jan 2023
 * For i386-AROS we need a C-Version of the assembly file. This is for non-68k only.
 * Others use HyperKhorner4M-1.asm or HyperKhorner4M-1_gcc.asm
 * Give it a try...
 *
See also HerKhorner4M-1.asm from SAS/C-Amiga Project and especially from the derived gcc-asm version

| Err, I'm not exactly sure what I'm doing, y'know.
| But it better be fast.
*/

#include <SDI_compiler.h>
#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <proto/graphics.h>
/*
    void *regs [2];
    regs[0]=ScratchRast;
    regs[1]=This->pixdata->data;

 */

void ASM HK4M(REG(a0, void **registers))
{
    struct RastPort *rp     =((struct RastPort**)registers)[0];
    UBYTE           *Pixels =       ((UBYTE**)   registers)[1];

    struct BitMap *temp_bm = AllocBitMap((IPTR)rp->RP_User-1, 1, 4, BMF_STANDARD | BMF_CLEAR, NULL);
    if (temp_bm)
    {
        struct RastPort temp_rp;

        InitRastPort(&temp_rp);
        temp_rp.BitMap = temp_bm;

        WritePixelArray8 (rp,0,0,(IPTR)rp->RP_User-1,rp->BitMap->Rows-1,Pixels,&temp_rp);

        FreeBitMap(temp_bm);
    }
}
