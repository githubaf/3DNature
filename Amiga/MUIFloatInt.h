/* MUIFloatInt.h
**
** Header file for using Xen's FloatInt class.
**
** Built on 09 Jan 1996 from scratch by CXH.
** Copyright 1996.
*/

#ifndef MUI_FLOATINT_H
#define MUI_FLOATINT_H

#define MUIC_FloatInt FloatIntClassPointer

#define CLASS	    MUIC_FloatInt
#define SUPERCLASS MUIC_Group

#define MUIA_FloatInt_VarPtr       ((TAG_USER | ( 1 << 16)) | 0x0001) /*IS*/
#define MUIA_FloatInt_VarType	     ((TAG_USER | ( 1 << 16)) | 0x0002) /*IS*/
#define MUIA_FloatInt_IncDecInt    ((TAG_USER | ( 1 << 16)) | 0x0003) /*IS*/
#define MUIA_FloatInt_IncDecDouble ((TAG_USER | ( 1 << 16)) | 0x0004) /*IS*/
#define MUIA_FloatInt_IDFracDouble ((TAG_USER | ( 1 << 16)) | 0x0005) /*IS*/
#define MUIA_FloatInt_MaxValDouble ((TAG_USER | ( 1 << 16)) | 0x0006) /*IS*/
#define MUIA_FloatInt_MinValDouble ((TAG_USER | ( 1 << 16)) | 0x0007) /*IS*/
#define MUIA_FloatInt_MaxValInt	  ((TAG_USER | ( 1 << 16)) | 0x0008) /*IS*/
#define MUIA_FloatInt_MinValInt    ((TAG_USER | ( 1 << 16)) | 0x0009) /*IS*/
#define MUIA_FloatInt_LabelText    ((TAG_USER | ( 1 << 16)) | 0x000A) /*I*/
#define MUIA_FloatInt_FieldWidth   ((TAG_USER | ( 1 << 16)) | 0x000B) /*I*/
/* This one is just here to trigger notifies off of. */
#define MUIA_FloatInt_Contents     ((TAG_USER | ( 1 << 16)) | 0x000B) /*SG*/

#define MUIA_FloatInt_Focus     ((TAG_USER | ( 1 << 16)) | 0x000B) /*SG*/

/* Including this tag, with a TRUE when setting the above attributes,
** will suppress the automatic resync action. For example, if you were
** changing both the VarPtr and then the VarType, you would want to
** suppress autosync during the VarPtr change. otherwise the widget will
** try to re-read your variable, but using the wrong type info.
** Potentially bad. Therefore, use InhibitAutoSync when changing the
** first attribute, and leave it off on the second to allow the resync.
*/
#define MUIV_FloatInt_InhibitAutoSync  ((TAG_USER | ( 1 << 16)) | 0x0101) /*S*/



#define MUIM_FloatInt_Inc    ((TAG_USER | ( 1 << 16)) | 0x0101)
#define MUIM_FloatInt_Dec    ((TAG_USER | ( 1 << 16)) | 0x0102)
#define MUIM_FloatInt_Str    ((TAG_USER | ( 1 << 16)) | 0x0103)
#define MUIM_FloatInt_Sync   ((TAG_USER | ( 1 << 16)) | 0x0104)
/* This will be sent by specially-configured WindowClass objects when
** the FloatInt loses the keyboard focus. We should just call Sync
** when it does. */
#define MUIM_FloatInt_LoseFocus   ((TAG_USER | ( 1 << 16)) | 0x0105)
#define MUIM_FloatInt_ChangeFocus   ((TAG_USER | ( 1 << 16)) | 0x0106)


#define FloatIntObject NewObject(FloatIntClassPointer->mcc_Class, NULL

#define FIOFlag_Frac			1 << 0
#define FIOFlag_Float		1 << 1
#define FIOFlag_Double		1 << 2
#define FIOFlag_Char			1 << 3
#define FIOFlag_Short		1 << 4
#define FIOFlag_Long			1 << 5
#define FIOFlag_Unsigned	1 << 6

#define FI_TypeMask (FIOFlag_Frac | FIOFlag_Float | FIOFlag_Double | FIOFlag_Char | FIOFlag_Short | FIOFlag_Long | FIOFlag_Unsigned)

struct MUI_CustomClass *FloatIntInit(void);
void FloatIntCleanup(void);
ULONG DoIncDec(struct IClass *cl,Object *obj,Msg msg, char Action);

struct FloatIntData
{
    Object *group;
    Object *string;
    Object *incbutton, *decbutton;

    unsigned long int FIFlags;
    void *MasterVariable;
    double IncDecAmount, MaxAmount, MinAmount;
    char *LabelText;
};

double CalcIncDec(double Quantity, struct FloatIntData *data, char Action);
#endif /* MUI_FLOATINT_H */
