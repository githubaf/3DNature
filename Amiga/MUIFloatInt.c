/* MUIFloatInt.c
**
** An MUI class, similar to MUI3's numeric class, that builds
** a string gadget adjacent to a pair of increase/decrease buttons.
**
** This class keeps a pointer to a variable that actually stores the
** current value (thus saving lots of get/set calls) and keeps/enforces
** a max and min value on any numbers entered or inc/dec'ed.
**
** The increment and decrement are handled transparently to the
** application, and any widget-generated change can generate a
** notify.
**
** A sliding scale feature can also be specified that allows the
** increment/decrement values to be a fraction of the current value.
** 
** Built on 09 Jan 1996 from MUI3's UserData.c by CXH.
** Copyright 1996.
*/

/* I know we don't normally use this in .C files, but it's an extern thing */
#ifndef MUI_FLOATINT_C
#define MUI_FLOATINT_C


#include <SDI_compiler.h>  // AF

// struct MUI_CustomClass *FloatIntInit(void); // AF, not used 26.July 2021

/* Includes */
#include <libraries/mui.h>
#include <dos/dos.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/muimaster_protos.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/muimaster_pragmas.h>

#include "WCS.h"
#include "MUIFloatInt.h"


STATIC_FCN double CalcIncDec(double Quantity, struct FloatIntData *data, char Action); // used locally only -> static, AF 19.7.2021
STATIC_FCN ULONG DoIncDec(struct IClass *cl,Object *obj,Msg msg, char Action); // used locally only -> static, AF 19.7.2021

STATIC_VAR char *FWT[] =
	{
	"",
	"1",
	"12",
	"123",
	"1234",
	"12345",
	"123456",
	"1234567",
	"12345678",
	"123456789",
	"1234567890"
	}; /* FWT */

STATIC_VAR struct MUI_CustomClass *FloatIntClassPointer;

struct Library *UtilityBase;

char Scratch[50];

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN SAVEDS ULONG mSync(struct IClass *cl,Object *obj,Msg msg) // used locally only -> static, AF 19.7.2021
{
struct FloatIntData *data;
double DConv;
unsigned long int ULConv;
signed long int LConv;

data = INST_DATA(cl,obj);

Scratch[0] = 0;

if(data->FIFlags & FIOFlag_Float)
	{
	DConv = *(float *)data->MasterVariable;
	sprintf(Scratch, "%f", DConv);
	} /* if */
else if(data->FIFlags & FIOFlag_Double)
	{
	DConv = *(double *)data->MasterVariable;
	sprintf(Scratch, "%f", DConv);
	} /* if */
else if(data->FIFlags & FIOFlag_Char)
	{
	if(data->FIFlags && FIOFlag_Unsigned)
		{
		ULConv = *(unsigned char *)data->MasterVariable;
		sprintf(Scratch, "%lu", ULConv);
		} /* if */
	else
		{
		LConv = *(char *)data->MasterVariable;
		sprintf(Scratch, "%ld", LConv);
		} /* else */
	} /* if */
else if(data->FIFlags & FIOFlag_Short)
	{
	if(data->FIFlags && FIOFlag_Unsigned)
		{
		ULConv = *(unsigned short *)data->MasterVariable;
		sprintf(Scratch, "%lu", ULConv);
		} /* if */
	else
		{
		LConv = *(short *)data->MasterVariable;
		sprintf(Scratch, "%ld", LConv);
		} /* else */
	} /* if */
else if(data->FIFlags & FIOFlag_Long)
	{
	if(data->FIFlags && FIOFlag_Unsigned)
		{
		ULConv = *(unsigned long *)data->MasterVariable;
		sprintf(Scratch, "%lu", ULConv);
		} /* if */
	else
		{
		LConv = *(long *)data->MasterVariable;
		sprintf(Scratch, "%ld", LConv);
		} /* else */
	} /* if */

set(data->string, MUIA_String_Contents, (ULONG)Scratch);

/* No-op to invoke triggers, if any */
ULConv = *(unsigned long *)data->MasterVariable;
/* The following results in infinite recursion:
** set(obj, MUIA_FloatInt_Contents, ULConv);
**
** Use this instead: */
SetAttrs(obj,MUIV_FloatInt_InhibitAutoSync,TRUE,MUIA_FloatInt_Contents, ULConv,TAG_DONE);

return(0);
}
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN SAVEDS ULONG mNew(struct IClass *cl,Object *obj,struct opSet *msg) // used locally only -> static, AF 19.7.2021
{
	struct FloatIntData *data;
	double *D;
	ULONG Store;
	signed long int Limit;
	char *AllowChars;
	unsigned char Width;  // unsigned, ALEXANDER
	APTR MyWin;

	/* This looks like cookbook code. */
	if (!(obj = (Object *)DoSuperMethodA(cl,obj,(Msg)msg)))
		return(0);
	data = INST_DATA(cl,obj);

	data->FIFlags = 0;
	data->MasterVariable = NULL;
	data->IncDecAmount = 1.0;
	data->MinAmount = 0;
	data->MaxAmount = FLT_MAX;
	
	if((Store = GetTagData(MUIA_FloatInt_IncDecInt, 0, msg->ops_AttrList)))
		{
		Limit = *(signed long int *)(&Store);
		data->IncDecAmount = Limit;
		data->FIFlags &= ~FIOFlag_Frac;
		} /* if */

	if((Store = GetTagData(MUIA_FloatInt_IncDecDouble, 0, msg->ops_AttrList)))
		{
		D = (double *)Store;
		data->IncDecAmount = *D;
		data->FIFlags &= ~FIOFlag_Frac;
		} /* if */

	if((Store = GetTagData(MUIA_FloatInt_IDFracDouble, 0, msg->ops_AttrList)))
		{
		D = (double *)Store;
		data->IncDecAmount = *D;
		data->FIFlags |= FIOFlag_Frac;
		} /* if */

	if((Store = GetTagData(MUIA_FloatInt_MaxValDouble, 0, msg->ops_AttrList)))
		{
		D = (double *)Store;
		data->MaxAmount = *D;
		} /* if */

	if((Store = GetTagData(MUIA_FloatInt_MinValDouble, 0, msg->ops_AttrList)))
		{
		D = (double *)Store;
		data->MinAmount = *D;
		} /* if */

	if((Store = GetTagData(MUIA_FloatInt_MaxValInt, 0, msg->ops_AttrList)))
		{
		Limit = *(signed long int *)(&Store);
		data->MaxAmount = Limit;
		} /* if */

	if((Store = GetTagData(MUIA_FloatInt_MinValInt, 0, msg->ops_AttrList)))
		{
		Limit = *(signed long int *)(&Store);
		data->MinAmount = Limit;
		} /* if */

	Width = GetTagData(MUIA_FloatInt_FieldWidth, 5, msg->ops_AttrList);
	if(Width > 10) Width = 10;
	if(Width < 1)  Width = 1;

	data->FIFlags        |= GetTagData(MUIA_FloatInt_VarType, 0, msg->ops_AttrList);
	data->MasterVariable  = (void *)GetTagData(MUIA_FloatInt_VarPtr, 0, msg->ops_AttrList);
	data->LabelText       = (char *)GetTagData(MUIA_FloatInt_LabelText, 0, msg->ops_AttrList);

	/* make sure we have required args */
	if(data->LabelText && data->MasterVariable && (data->FIFlags & FI_TypeMask))
		{
		if(data->FIFlags & (FIOFlag_Float | FIOFlag_Double)) /* is it floating-point? */
			{
			AllowChars = ".+-0123456789";
			} /* if */
		else
			{
			AllowChars = "+-0123456789";
			} /* else */
		data->group = 	HGroup, MUIA_Group_HorizSpacing, 0,
								Child, Label2(data->LabelText),
								Child, data->string = StringObject, StringFrame,
									MUIA_String_Accept, AllowChars,
									MUIA_FixWidthTxt, FWT[Width], End,
								Child, data->decbutton = ImageButton(MUII_ArrowLeft),
								Child, data->incbutton = ImageButton(MUII_ArrowRight),
								End;
		} /* if */

	if (!data->group)
	{
		CoerceMethod(cl,obj,OM_DISPOSE);
		return(0);
	}


	get(data->string, MUIA_WindowObject, &MyWin);

   /* Set up notification for change of ActiveObject */
   
   DoMethod(MyWin, MUIM_Notify, MUIA_Window_ActiveObject, MUIV_EveryTime,
    obj, 3, MUIM_FloatInt_ChangeFocus, MUIA_FloatInt_Focus, MUIV_TriggerValue);

	/* Set up notification for increment/decrement buttons */
	DoMethod(data->decbutton, MUIM_Notify, MUIA_Pressed, FALSE, obj,
	 1, MUIM_FloatInt_Dec);
	DoMethod(data->incbutton, MUIM_Notify, MUIA_Pressed, FALSE, obj,
	 1, MUIM_FloatInt_Inc);
	
	/* Set up notification for string gadget */
	DoMethod(data->string, MUIM_Notify, MUIA_Selected, FALSE, obj,
	 1, MUIM_FloatInt_Str);


	DoMethod(obj,OM_ADDMEMBER,data->group);
	
	mSync(cl, obj, (Msg)msg);
	
	return((ULONG)obj);
}
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN SAVEDS ULONG mDispose(struct IClass *cl,Object *obj,Msg msg) // used locally only -> static, AF 19.7.2021
{
	return(DoSuperMethodA(cl,obj,msg));
}
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN SAVEDS ULONG mInc(struct IClass *cl,Object *obj,Msg msg) // used locally only -> static, AF 19.7.2021
{
return(DoIncDec(cl,obj,msg,1));
}
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN SAVEDS ULONG mDec(struct IClass *cl,Object *obj,Msg msg)
{
return(DoIncDec(cl,obj,msg,0));
}
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN SAVEDS ULONG mStr(struct IClass *cl,Object *obj,Msg msg) // used locally only -> static, AF 19.7.2021
{
struct FloatIntData *data;
char ReSync = 0;

double *D;
float *F;
char *C, *Str;
unsigned char *UC;
short *S;
unsigned short *US;
long *L;
unsigned long *UL;

data = INST_DATA(cl,obj);

get(data->string, MUIA_String_Contents, &Str);

if(data->FIFlags & FIOFlag_Float)
	{
	F = (float *)data->MasterVariable;
	*F = atof(Str);
	} /* if */
else if(data->FIFlags & FIOFlag_Double)
	{
	D = (double *)data->MasterVariable;
	*D = atof(Str);
	} /* if */
else if(data->FIFlags & FIOFlag_Char)
	{
	if(data->FIFlags && FIOFlag_Unsigned)
		{
		C = (char *)data->MasterVariable;
		*C = (char)atoi(Str);
		if(*C > data->MaxAmount) {*C = (char)data->MaxAmount; ReSync = 1;}
		if(*C < data->MinAmount) {*C = (char)data->MinAmount; ReSync = 1;}
		} /* if */
	else
		{
		UC = (unsigned char *)data->MasterVariable;
		*UC = (unsigned char)atoi(Str);
		if(*UC > data->MaxAmount) {*UC = (unsigned char)data->MaxAmount; ReSync = 1;}
		if(*UC < data->MinAmount) {*UC = (unsigned char)data->MinAmount; ReSync = 1;}
		} /* else */
	} /* if */
else if(data->FIFlags & FIOFlag_Short)
	{
	if(data->FIFlags && FIOFlag_Unsigned)
		{
		S = (short *)data->MasterVariable;
		*S = (short)atoi(Str);
		if(*S > data->MaxAmount) {*S = (short)data->MaxAmount; ReSync = 1;}
		if(*S < data->MinAmount) {*S = (short)data->MinAmount; ReSync = 1;}
		} /* if */
	else
		{
		US = (unsigned short *)data->MasterVariable;
		*US = (unsigned short)atoi(Str);
		if(*US > data->MaxAmount) {*US = (unsigned short)data->MaxAmount; ReSync = 1;}
		if(*US < data->MinAmount) {*US = (unsigned short)data->MinAmount; ReSync = 1;}
		} /* else */
	} /* if */
else if(data->FIFlags & FIOFlag_Long)
	{
	if(data->FIFlags && FIOFlag_Unsigned)
		{
		L = (long *)data->MasterVariable;
		*L = (long)atol(Str);
		if(*L > data->MaxAmount) {*L = (long)data->MaxAmount; ReSync = 1;}
		if(*L < data->MinAmount) {*L = (long)data->MinAmount; ReSync = 1;}
		} /* if */
	else
		{
		UL = (unsigned long *)data->MasterVariable;
		*UL = (unsigned long)atol(Str);
		if(*UL > data->MaxAmount) {*UL = (unsigned long)data->MaxAmount; ReSync = 1;}
		if(*UL < data->MinAmount) {*UL = (unsigned long)data->MinAmount; ReSync = 1;}
		} /* else */
	} /* if */

if(ReSync)
	{
	/* Update string gadget to reflect max/min limiting */
	/* This will trigger notifies */
	mSync(cl, obj, msg);
	} /* if */
else
	{
	/* No-op to invoke triggers, if any */
	UL = (unsigned long *)data->MasterVariable;
	/* set(obj, MUIA_FloatInt_Contents, *(UL)); */
	SetAttrs(obj,MUIV_FloatInt_InhibitAutoSync,TRUE,MUIA_FloatInt_Contents, *(UL),TAG_DONE);
	} /* else */

return(0);
}
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN SAVEDS ULONG mSet(struct IClass *cl,Object *obj,struct opSet *msg) // used locally only -> static, AF 19.7.2021
{
struct FloatIntData *data = INST_DATA(cl,obj);
struct TagItem *tags,*tag;
ULONG Temp;
signed long int Limit;
double *D;

for (tags=msg->ops_AttrList;(tag=NextTagItem(&tags));)
	{
	switch (tag->ti_Tag)
		{
		case MUIA_FloatInt_IncDecInt:
			{
			Temp = tag->ti_Data;
			Limit = *(signed long int *)(&Temp);
			data->IncDecAmount = Limit;
			/* No reason to Sync */
			break;
			} /* IncDecInt */
		case MUIA_FloatInt_IncDecDouble:
			{
			D = (double *)tag->ti_Data;
			if(D) /* prevent enforcer hits from stupidity */
				{
				data->IncDecAmount = *D;
				data->FIFlags &= ~FIOFlag_Frac;
				} /* if */
			/* No reason to Sync */
			break;
			} /* IncDecDouble */
		case MUIA_FloatInt_IDFracDouble:
			{
			D = (double *)tag->ti_Data;
			if(D) /* prevent enforcer hits from stupidity */
				{
				data->IncDecAmount = *D;
				data->FIFlags |= FIOFlag_Frac;
				} /* if */
			/* No reason to Sync */
			break;
			} /* IDFracDouble */

		case MUIA_FloatInt_MaxValDouble:
			{
			D = (double *)tag->ti_Data;
			if(D) /* prevent enforcer hits from stupidity */
				{
				data->MaxAmount = *D;
				if(!GetTagData(MUIV_FloatInt_InhibitAutoSync, (ULONG)NULL, msg->ops_AttrList))
					{
					mSync(cl, obj, (Msg)msg);
					} /* if */
				} /* if */
			break;
			} /* MaxValDouble */
		case MUIA_FloatInt_MinValDouble:
			{
			D = (double *)tag->ti_Data;
			if(D) /* prevent enforcer hits from stupidity */
				{
				data->MinAmount = *D;
				if(!GetTagData(MUIV_FloatInt_InhibitAutoSync, (ULONG)NULL, msg->ops_AttrList))
					{
					mSync(cl, obj, (Msg)msg);
					} /* if */
				} /* if */
			break;
			} /* MinValDouble */

		case MUIA_FloatInt_MaxValInt:
			{
			Temp = tag->ti_Data;
			Limit = *(signed long int *)(&Temp);
			data->MaxAmount = Limit;
			if(!GetTagData(MUIV_FloatInt_InhibitAutoSync, (ULONG)NULL, msg->ops_AttrList))
				{
				mSync(cl, obj, (Msg)msg);
				} /* if */
			break;
			} /* MaxValInt */
		case MUIA_FloatInt_MinValInt:
			{
			Temp = tag->ti_Data;
			Limit = *(signed long int *)(&Temp);
			data->MinAmount = Limit;
			if(!GetTagData(MUIV_FloatInt_InhibitAutoSync, (ULONG)NULL, msg->ops_AttrList))
				{
				mSync(cl, obj, (Msg)msg);
				} /* if */
			break;
			} /* MinValInt */


		case MUIA_FloatInt_VarPtr:
			{
			data->MasterVariable = (void *)tag->ti_Data;
			if(!GetTagData(MUIV_FloatInt_InhibitAutoSync, (ULONG)NULL, msg->ops_AttrList))
				{
				mSync(cl, obj, (Msg)msg);
				} /* if */
			break;
			} /* VarPtr */
		case MUIA_FloatInt_VarType:
			{
			Temp = tag->ti_Data;
			if(Temp & FI_TypeMask)
				{
				data->FIFlags &= ~FI_TypeMask;
				data->FIFlags |= Temp;
				if(!GetTagData(MUIV_FloatInt_InhibitAutoSync, (ULONG)NULL, msg->ops_AttrList))
					{
					mSync(cl, obj, (Msg)msg);
					} /* if */
				} /* if */
			break;
			} /* VarType */
		case MUIA_FloatInt_Contents:
			{
			if(data->MasterVariable)
				{
				*((long *)(data->MasterVariable)) = (long)tag->ti_Data;
				if(!GetTagData(MUIV_FloatInt_InhibitAutoSync, (ULONG)NULL, msg->ops_AttrList))
					{
					mSync(cl, obj, (Msg)msg);
					} /* if */
				} /* if */
			break;
			} /* VarPtr */
		}
	}

return(DoSuperMethodA(cl,obj,(Msg)msg));
}
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN SAVEDS ULONG mGet(struct IClass *cl,Object *obj,struct opSet *msg) // used locally only -> static, AF 19.7.2021
{
struct FloatIntData *data = INST_DATA(cl,obj);
struct TagItem *tags,*tag;

for (tags=msg->ops_AttrList;(tag=NextTagItem(&tags));)
	{
	switch (tag->ti_Tag)
		{
		case MUIA_FloatInt_Contents:
			{
			return(*((ULONG *)(data->MasterVariable)));
			break;
			} /* VarPtr */
		}
	}

return(DoSuperMethodA(cl,obj,(Msg)msg));
}
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN SAVEDS ULONG mChangeFocus(struct IClass *cl,Object *obj,struct opSet *msg) // used locally only -> static, AF 19.7.2021
{
/* struct FloatIntData *data = INST_DATA(cl,obj); */
struct TagItem *tags,*tag;

for (tags=msg->ops_AttrList;(tag=NextTagItem(&tags));)
	{
	switch (tag->ti_Tag)
		{
		case MUIA_FloatInt_Focus:
			{
			printf("New Focus: %lx\n", tag->ti_Data);
			return(0);
			break;
			} /* Focus */
		}
	}

return(DoSuperMethodA(cl,obj,(Msg)msg));
}
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN SAVEDS ULONG mLoseFocus(struct IClass *cl,Object *obj,struct opSet *msg) // used locally only -> static, AF 19.7.2021
{
/* struct FloatIntData *data = INST_DATA(cl,obj);
struct TagItem *tags,*tag; */

return(DoSuperMethodA(cl,obj,(Msg)msg));
}
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
SAVEDS ASM ULONG Dispatcher(REG(a0, struct IClass *cl),
				   REG(a2, Object *obj),
				   REG(a1, Msg msg))
{
	switch (msg->MethodID)
	{
		case OM_NEW		     : return(mNew        (cl,obj,(APTR)msg));
		case OM_SET		     : return(mSet        (cl,obj,(APTR)msg));
		case OM_GET		     : return(mGet        (cl,obj,(APTR)msg));
		case OM_DISPOSE 	  : return(mDispose    (cl,obj,(APTR)msg));

		case MUIM_FloatInt_Inc   : return(mInc      (cl,obj,(APTR)msg));
		case MUIM_FloatInt_Dec   : return(mDec      (cl,obj,(APTR)msg));
		case MUIM_FloatInt_Str   : return(mStr      (cl,obj,(APTR)msg));
		case MUIM_FloatInt_Sync  : return(mSync     (cl,obj,(APTR)msg));

		case MUIM_FloatInt_ChangeFocus  : return(mChangeFocus     (cl,obj,(APTR)msg));
		case MUIM_FloatInt_LoseFocus  : return(mLoseFocus     (cl,obj,(APTR)msg));
	}

	return(DoSuperMethodA(cl,obj,msg));
}
#endif

#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
struct MUI_CustomClass *FloatIntInit(void)
{
if(!FloatIntClassPointer)
	{
	FloatIntClassPointer = MUI_CreateCustomClass(NULL,SUPERCLASS,NULL,sizeof(struct FloatIntData),Dispatcher);
	if(FloatIntClassPointer)
		{
		UtilityBase   = FloatIntClassPointer->mcc_UtilityBase;
/*		DOSBase       = FloatIntClassPointer->mcc_DOSBase;
		IntuitionBase = FloatIntClassPointer->mcc_IntuitionBase; */
		} /* if */
	} /* if */
return(FloatIntClassPointer);
} /* FloatIntInit() */
#endif


#ifdef UNUSED_FUNCTIONS  // AF, not used 19.July 2021
void FloatIntCleanup(void)
{
if (FloatIntClassPointer)
	{
	MUI_DeleteCustomClass(FloatIntClassPointer);
	FloatIntClassPointer = NULL;
	} /* if */
} /* FloatIntCleanup() */
#endif
#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN ULONG DoIncDec(struct IClass *cl,Object *obj,Msg msg, char Action) // used locally only -> static, AF 19.7.2021
{
double *D;
float *F;
char *C;
unsigned char *UC;
short *S;
unsigned short *US;
long *L;
unsigned long *UL;

struct FloatIntData *data;

data = INST_DATA(cl,obj);

if(!data->MasterVariable) return(0);

if(data->FIFlags & FIOFlag_Float)
	{
	F = (float *)data->MasterVariable;
	*F = (float)CalcIncDec((double)*F, data, Action);
	} /* if */
else if(data->FIFlags & FIOFlag_Double)
	{
	D = (double *)data->MasterVariable;
	*D = CalcIncDec((double)*D, data, Action);
	} /* if */
else if(data->FIFlags & FIOFlag_Char)
	{
	if(data->FIFlags && FIOFlag_Unsigned)
		{
		UC = (unsigned char *)data->MasterVariable;
		*UC = (unsigned char)CalcIncDec((double)*UC, data, Action);
		} /* if */
	else
		{
		C = (char *)data->MasterVariable;
		*C = (char)CalcIncDec((double)*C, data, Action);
		} /* else */
	} /* if */
else if(data->FIFlags & FIOFlag_Short)
	{
	if(data->FIFlags && FIOFlag_Unsigned)
		{
		US = (unsigned short *)data->MasterVariable;
		*US = (unsigned short)CalcIncDec((double)*US, data, Action);
		} /* if */
	else
		{
		S = (short *)data->MasterVariable;
		*S = (short)CalcIncDec((double)*S, data, Action);
		} /* else */
	} /* if */
else if(data->FIFlags & FIOFlag_Long)
	{
	if(data->FIFlags && FIOFlag_Unsigned)
		{
		UL = (unsigned long *)data->MasterVariable;
		*UL = (unsigned long)CalcIncDec((double)*UL, data, Action);
		} /* if */
	else
		{
		L = (long *)data->MasterVariable;
		*L = (long)CalcIncDec((double)*L, data, Action);
		} /* else */
	} /* if */

/* Resync the string gadget */
return(mSync(cl,obj,msg));
} /* DoIncDec() */
#endif

#ifdef UNUSED_FUNCTIONS_GC  // AF, not used 17.May 2022 found with -gc
STATIC_FCN double CalcIncDec(double Quantity, struct FloatIntData *data, char Action) // used locally only -> static, AF 19.7.2021
{
double Step;

if(data->FIFlags & FIOFlag_Frac)
	{
	Step = Quantity * data->IncDecAmount;
	} /* if */
else
	{
	Step = data->IncDecAmount;
	} /* else */

if(Action > 0)
	{
	if(((Quantity + Step) <= data->MaxAmount) &&
	 ((Quantity + Step) >= data->MinAmount))
		{
		return(Quantity + Step);
		} /* if */
	} /* if */
else if(Action == 0)
	{
	if(((Quantity - Step) <= data->MaxAmount) &&
	 ((Quantity - Step) >= data->MinAmount))
		{
		return(Quantity - Step);
		} /* if */
	} /* if */

return(Quantity);
} /* CalcIncDec() */
#endif

#endif /* MUI_FLOATINT_C */
