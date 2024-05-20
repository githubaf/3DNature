/* GenericParams.c (ne gisparams.c 14 Jan 1994 CXH)
** All kinds of functions to set, compute, load, and magle values and params.
** Built/ripped from gis.c on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code and major modifications by the incomparable Gary R. Huber.
** Copyright 1995 by Questar Productions.
*/

#define CATCOMP_NUMBERS 1
#include "WCS_locale.h"

#include "WCS.h"
#include "GUIDefines.h"
#include "GUIExtras.h"
#include "GenericParams.h"

/**************************************************************************/

STATIC_FCN short SearchGenericKeyFrame(union KeyFrame *KF, short KeyFrames,
        short frame, short group, short item); // used locally only -> static, AF 19.7.2021

STATIC_FCN void UnsetGenericKeyItem(union KeyFrame *Key,
        short ItemMatch, short NumValues, double *DblValue, float *FltValue,
        short *ShortValue, float *TCB, short *LinearPtr, short Precision); // used locally only -> static, AF 23.7.2021
STATIC_FCN short AllocNewGenericKeyArray(union KeyFrame **KF, long *KFsize); // used locally only -> static, AF 23.7.2021
STATIC_FCN void SetGenericKeyFrame(union KeyFrame *KF, short i, short frame, short group,
        short item, short ItemMatch, short NumValues, double *DblValue,
        float *FltValue, short *ShortValue, float *TCB, short Linear,
        short Precision); // used locally only -> static, AF 23.7.2021
//short GetNextGenericKeyItem(union KeyFrame *KF, short KeyFrames,
//        short group, short curitem, short dir); // AF, not used 26.July 2021
STATIC_FCN void SetGenericKeyTableEntry(union KeyFrame *KF, union KeyFrame **Key,
        short NumKeys, short Group, short Item); // used locally only -> static, AF 23.7.2021
//short CountGenericKeyFrames(union KeyFrame *KF, short KeyFrames,
//        short group, short item); // AF, not used 26.July 2021

#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
void MergeGenericKeyFrames(union KeyFrame *MF, short MFKeys, union KeyFrame **OF,
	short *OFKeys, long *OFsize, short group)
{
 short i, j, frame, OrigKeys;
 union KeyFrame *OFptr;

 OFptr = *OF;
 OrigKeys = *OFKeys;
 for (i=OrigKeys-1; i>=0; i--)
  {
  if (group == OFptr[i].MoKey.Group)
   {
   memmove(&OFptr[i], &OFptr[i+1], (*OFKeys - i - 1) * sizeof (union KeyFrame));
   *OFKeys -= 1;
   } /* if group match */
  } /* for i=... */

 for (i=0; i<MFKeys; i++)
  {
  if (group == MF[i].MoKey.Group)
   {
   frame = MF[i].MoKey.KeyFrame;
   if ((*OFKeys + 1) * sizeof (union KeyFrame) > *OFsize)
    {
    if (! AllocNewKeyArray(OF, OFsize))
     {
     char *groupname;

     if (group == 0) groupname = "motion";
     else if (group == 1) groupname = "color";
     else if (group == 2) groupname = "ecosystem";
     else if (group == 3) groupname = "cloud";
     else if (group == 4) groupname = "wave";
     else groupname = "generic";
     sprintf(str,
	 "Out of memory restoring old key frames!\nSome %s keys may be lost.",
	 groupname);
     User_Message("Key Frame: Cancel", str, "OK", "o");
     break;
     }
    OFptr = *OF;
    } /* if not large enough */

/* find place in list for new key frame */
   if (*OFKeys > 0)
    {
    j = *OFKeys;
    while (frame < OFptr[j - 1].MoKey.KeyFrame)
     {
     j--;
     if (j == 0) break;
     } /* while */
    memmove(&OFptr[j+1], &OFptr[j], (*OFKeys - j) * sizeof (union KeyFrame));
    } /* if already some key frames */
   else j = 0;

   memcpy(&OFptr[j], &MF[i], sizeof (union KeyFrame));

   *OFKeys += 1;
   } /* if group match */
  } /* for i=0... */

} /* MergeGenericKeyFrames() */
#endif
/************************************************************************/

short MakeGenericKeyFrame(union KeyFrame **KFPtr, long *KFSizePtr,
	short *KeyFramesPtr, short frame, short group, short item,
	short ItemMatch, short NumValues, double *DblValue, float *FltValue,
	short *ShortValue, float *TCB, short Linear, short Precision)
{
 short i;
 union KeyFrame *KF;
 short KeyFrames;
 long KFSize;

 KF = *KFPtr;
 KeyFrames = *KeyFramesPtr;
 KFSize = *KFSizePtr;

/* check to see if key frame already exists */
 if (SearchGenericKeyFrame(KF, KeyFrames, frame, group, item) >= 0)
  {
  DeleteGenericKeyFrame(KF, KeyFramesPtr, frame, group, item, 0, 0, NULL, NULL);
  } /* if key frame already exists */

/* check for available space in key frame array */
 if ((KeyFrames + 1) * sizeof (union KeyFrame) > KFSize)
  {
  if (! AllocNewGenericKeyArray(KFPtr, KFSizePtr))
   return (0);
  KF = *KFPtr;
  } /* if not large enough */

/* find place in list for new key frame */
 if (KeyFrames > 0)
  {
  i = KeyFrames;
  while (frame < KF[i - 1].MoKey.KeyFrame)
   {
   i--;
   if (i == 0) break;
   } /* while */
  memmove(&KF[i+1], &KF[i], (KeyFrames - i) * sizeof (union KeyFrame));
  } /* if already some key frames */
 else i = 0;

 (*KeyFramesPtr) ++;

 memset(&KF[i], 0, sizeof (union KeyFrame));

 SetGenericKeyFrame(KF, i, frame, group, item, ItemMatch, NumValues, DblValue,
	FltValue, ShortValue, TCB, Linear, Precision);

 return (1);

} /* MakeGenericKeyFrame() */

/************************************************************************/

STATIC_FCN short AllocNewGenericKeyArray(union KeyFrame **KF, long *KFsize) // used locally only -> static, AF 23.7.2021
{
 union KeyFrame *NewKF;
 long NewKFsize;

 NewKFsize = *KFsize + 20 * sizeof (union KeyFrame);
 if ((NewKF = (union KeyFrame *)get_Memory(NewKFsize, MEMF_CLEAR)) == NULL)
  {
  User_Message(GetString( MSG_GENPAR_KEYFRAMEMODULE ),                                      // "Key Frame Module"
               GetString( MSG_PARAMS_OUTOFMEMORYALLOCATINGNEWKEYFRAMEPERATIONTERMINATED ),  // "Out of memory allocating new key frame!\nOperation terminated."
              GetString( MSG_GLOBAL_OK ),                                                   // "OK"
              (CONST_STRPTR)"o");
  return (0);
  } /* if memory bust */

 memcpy(NewKF, *KF, *KFsize);

 free_Memory(*KF, *KFsize);

 *KF = NewKF;
 *KFsize = NewKFsize;

 return (1);

} /* AllocNewKeyArray() */

/************************************************************************/

short DeleteGenericKeyFrame(union KeyFrame *KF, short *KeyFramesPtr,
	short frame, short group, short Item, short DeleteAll, short DeleteGp,
	short *ItemVals, double *IncrVals)
{
 short i, found = 0;

 for (i=*KeyFramesPtr-1; i>=0; i--)
  {
  if (frame == KF[i].MoKey.KeyFrame)
   {
   if (DeleteAll)
    {
    memmove(&KF[i], &KF[i+1], (*KeyFramesPtr - i - 1) * sizeof (union KeyFrame));
    (*KeyFramesPtr) --;
    found = 1;
    continue;
    }
   if (group == KF[i].MoKey.Group)
    {
    if (DeleteGp)
     {
     if (  (KF[i].MoKey.Item == ItemVals[0] && IncrVals[0] != 0.0)
	|| (KF[i].MoKey.Item == ItemVals[1] && IncrVals[1] != 0.0)
	|| (KF[i].MoKey.Item == ItemVals[2] && IncrVals[2] != 0.0))
      {
      memmove(&KF[i], &KF[i+1], (*KeyFramesPtr - i - 1) * sizeof (union KeyFrame));
      (*KeyFramesPtr) --;
      found = 1;
      }
     continue;
     }
    if (Item == KF[i].MoKey.Item)
     {
     memmove(&KF[i], &KF[i+1], (*KeyFramesPtr - i - 1) * sizeof (union KeyFrame));
     (*KeyFramesPtr) --;
     found = 1;
     break;
     } /* if match found */
    } /* if group match */
   } /* if frame match */
  else if (KF[i].MoKey.KeyFrame < frame) break;
  } /* for i=0... */

 if (found)
  {
  return (1);
  } /* if key frame found */

 return (0);

} /* DeleteGenericKeyFrame() */

/************************************************************************/

STATIC_FCN short SearchGenericKeyFrame(union KeyFrame *KF, short KeyFrames,
	short frame, short group, short item) // used locally only -> static, AF 19.7.2021
{
 short i, found = -1;

 for (i=0; i<KeyFrames; i++)
  {
  if (frame == KF[i].MoKey.KeyFrame)
   {
   if (group == KF[i].MoKey.Group)
    {
    if (item == KF[i].MoKey.Item)
     {
     found = i;
     break;
     } /* if match found */
    } /* if group match */
   } /* if frame match */
  } /* for i=0... */

 return (found);

} /* SearchGenericKeyFrame() */

/************************************************************************/

STATIC_FCN void SetGenericKeyFrame(union KeyFrame *KF, short i, short frame, short group,
	short item, short ItemMatch, short NumValues, double *DblValue,
	float *FltValue, short *ShortValue, float *TCB, short Linear,
	short Precision) // used locally only -> static, AF 23.7.2021
{

 KF[i].MoKey.KeyFrame = frame;
 KF[i].MoKey.Group = group;
 KF[i].MoKey.Item = item;
 if (ItemMatch >= 0 && item == ItemMatch && TCB)
  {
  KF[i].MoKey.TCB[0] = TCB[0];
  KF[i].MoKey.TCB[1] = TCB[1];
  KF[i].MoKey.TCB[2] = TCB[2];
  KF[i].MoKey.Linear = Linear;
  } /* if item is active item, prevents Update Keys from changing them */
   /* if motion editor open - only if MakeKey is invoked from Map View might it not be */
 else
  {
  KF[i].MoKey.TCB[0] = 0;
  KF[i].MoKey.TCB[1] = 0;
  KF[i].MoKey.TCB[2] = 0;
  KF[i].MoKey.Linear = 0;
  } /* else */

 for (item=0; item<NumValues; item++)
  {
  if (DblValue)
   {
   if (Precision == WCS_KFPRECISION_DOUBLE)
    KF[i].MoKey2.Value[item] = DblValue[item];
   else if (Precision == WCS_KFPRECISION_FLOAT)
    KF[i].EcoKey2.Value[item] = DblValue[item];
   else if (Precision == WCS_KFPRECISION_SHORT)
    KF[i].CoKey.Value[item] = DblValue[item];
   } /* motion */
  else if (ShortValue)
   {
   if (Precision == WCS_KFPRECISION_DOUBLE)
    KF[i].MoKey2.Value[item] = ShortValue[item];
   else if (Precision == WCS_KFPRECISION_FLOAT)
    KF[i].EcoKey2.Value[item] = ShortValue[item];
   else if (Precision == WCS_KFPRECISION_SHORT)
    KF[i].CoKey.Value[item] = ShortValue[item];
   } /* color */
  else if (FltValue)
   {
   if (Precision == WCS_KFPRECISION_DOUBLE)
    KF[i].MoKey2.Value[item] = FltValue[item];
   else if (Precision == WCS_KFPRECISION_FLOAT)
    KF[i].EcoKey2.Value[item] = FltValue[item];
   else if (Precision == WCS_KFPRECISION_SHORT)
    KF[i].CoKey.Value[item] = FltValue[item];
   } /* ecosystem */
  } /* for i=0... */

} /* SetGenericKeyFrame() */

/************************************************************************/

short UnsetGenericKeyFrame(union KeyFrame *KF, short KeyFrames,
	struct WindowKeyStuff *WKS,
	short frame, short group, short item, short unset,
	short ItemMatch, short NumValues, double *DblValue, float *FltValue,
	short *ShortValue, float *TCB, short *LinearPtr, short Precision)
{
 short i, found = -1, Prev = -1, Next = -1, KeysExist = 0, ItemKeys = 0;

 for (i=0; i<KeyFrames; i++)
  {
  if (group == KF[i].MoKey.Group)
   {
   if (item == KF[i].MoKey.Item)
    {
    ItemKeys ++;
    if (frame == KF[i].MoKey.KeyFrame)
     {
     found = frame;
     KeysExist ++;
     UnsetGenericKeyItem(&KF[i], ItemMatch, NumValues, DblValue,
	FltValue, ShortValue, TCB, LinearPtr, Precision);
     } /* if frame match found */
    else if (frame > KF[i].MoKey.KeyFrame)
     {
     Prev = KF[i].MoKey.KeyFrame;
     } /* else if prev key */
    else if (Next < 0)
     {
     Next = KF[i].MoKey.KeyFrame;
     } /* else next key */
    } /* if item match */
   else if (frame == KF[i].MoKey.KeyFrame)
    {
    KeysExist ++;
    if (unset)
     UnsetGenericKeyItem(&KF[i], ItemMatch, NumValues, DblValue,
	FltValue, ShortValue, TCB, LinearPtr, Precision);
    }
   } /* if group match */
  } /* for i=0... */

 if (WKS)
  {
  WKS->PrevKey = Prev;		/* previous key for this item */
  WKS->NextKey = Next;		/* next key for this item */
  WKS->KeysExist = KeysExist;	/* num items with keys at this frame */
  WKS->ItemKeys = ItemKeys;
  WKS->IsKey = found;
  } /* if */

 if (found < 0)
  {
  if (TCB)
   TCB[0] = TCB[1] = TCB[2] = 0.0;
  if (LinearPtr)
   *LinearPtr = 0;
  } /* if */

 return (found);

} /* UnsetGenericKeyFrame() */

/************************************************************************/

STATIC_FCN void UnsetGenericKeyItem(union KeyFrame *Key,
	short ItemMatch, short NumValues, double *DblValue, float *FltValue,
	short *ShortValue, float *TCB, short *LinearPtr, short Precision) // used locally only -> static, AF 23.7.2021
{
short i, item;

 item = Key->MoKey.Item;

 if (item == ItemMatch && TCB)
  {
  TCB[0] = Key->MoKey.TCB[0];
  TCB[1] = Key->MoKey.TCB[1];
  TCB[2] = Key->MoKey.TCB[2];
  *LinearPtr = Key->MoKey.Linear;
  } /* if item is currently active one */

 for (i=0; i<NumValues; i++)
  {
  if (DblValue)
   {
   if (Precision == WCS_KFPRECISION_DOUBLE)
    DblValue[i] = Key->MoKey2.Value[i];
   else if (Precision == WCS_KFPRECISION_FLOAT)
    DblValue[i] = Key->EcoKey2.Value[i];
   else if (Precision == WCS_KFPRECISION_SHORT)
    DblValue[i] = Key->CoKey.Value[i];
   } /* motion */
  else if (ShortValue)
   {
   if (Precision == WCS_KFPRECISION_DOUBLE)
    ShortValue[i] = Key->MoKey2.Value[i];
   else if (Precision == WCS_KFPRECISION_FLOAT)
    ShortValue[i] = Key->EcoKey2.Value[i];
   else if (Precision == WCS_KFPRECISION_SHORT)
    ShortValue[i] = Key->CoKey.Value[i];
   ShortValue[i] = Key->CoKey.Value[i];
   }
  else if (FltValue)
   {
   if (Precision == WCS_KFPRECISION_DOUBLE)
    FltValue[i] = Key->MoKey2.Value[i];
   else if (Precision == WCS_KFPRECISION_FLOAT)
    FltValue[i] = Key->EcoKey2.Value[i];
   else if (Precision == WCS_KFPRECISION_SHORT)
    FltValue[i] = Key->CoKey.Value[i];
   } /* ecosystem */
  } /* for i=0... */

} /* UnsetGenericKeyItem() */

/************************************************************************/

void UpdateGenericKeyFrames(union KeyFrame *KF, short KeyFrames,
	short frame, short group, short Item, short ItemMatch, short NumValues,
	double *DblValue, float *FltValue, short *ShortValue, float *TCB,
	short Linear,
	short UpdateAll, short UpdateGp, short *ItemVals, double *IncrVals,
	short Precision)
{
 short i;

 for (i=0; i<KeyFrames; i++)
  {
  if (frame == KF[i].MoKey.KeyFrame)
   {
   if (UpdateAll)
    {
    SetGenericKeyFrame(KF, i, frame, group, KF[i].MoKey.Item, ItemMatch,
	NumValues, DblValue, FltValue, ShortValue, TCB, Linear, Precision);
    continue;
    }
   if (group == KF[i].MoKey.Group)
    {
    if (UpdateGp && ItemVals && IncrVals)
     {
     if (  (KF[i].MoKey.Item == ItemVals[0] && IncrVals[0] != 0.0)
	|| (KF[i].MoKey.Item == ItemVals[1] && IncrVals[1] != 0.0)
	|| (KF[i].MoKey.Item == ItemVals[2] && IncrVals[2] != 0.0))
      {
      SetGenericKeyFrame(KF, i, frame, group, KF[i].MoKey.Item, ItemMatch,
	NumValues, DblValue, FltValue, ShortValue, TCB, Linear, Precision);
      } /* if match item */
     continue;
     } /* if update motion group */
    if (Item == KF[i].MoKey.Item)
     {
     SetGenericKeyFrame(KF, i, frame, group, KF[i].MoKey.Item, ItemMatch,
	NumValues, DblValue, FltValue, ShortValue, TCB, Linear, Precision);
     break;
     } /* if item match */
    } /* if group match */
   } /* if frame match */
  else if (KF[i].MoKey.KeyFrame > frame) break;
  } /* for i=0... */

} /* UpdateGenericKeyFrames() */

/************************************************************************/

/* note this function is not the equivalent of GetKeyTableValues() which
** gets the values for all motion, color or ecosystem parameters in one
** swell foop */

void GetGenericKeyTableValues(struct KeyTable **KTPtr, union KeyFrame *KF,
	short NumKeys, short *MaxFramesPtr, short Elements,
	short group, short item, short frame,
	double *DblValue, float *FltValue, short *ShortValue, short Precision)
{
 struct KeyTable *KT;
 short i;

 if (BuildGenericKeyTable(KTPtr, KF, NumKeys, MaxFramesPtr, group, item,
	Elements, Precision, NULL))
  {
  KT = *KTPtr;
  frame = frame > *MaxFramesPtr ? *MaxFramesPtr: frame;
  if (DblValue)
   {
   if (KT[item].Key)
    {
    for (i=0; i<Elements; i++)
     DblValue[i] = KT[item].Val[i][frame];
    } /* if a key exists */
   } /* motion */
  else if (ShortValue)
   {
   if (KT[item].Key)
    {
    for (i=0; i<Elements; i++)
     ShortValue[i] = KT[item].Val[i][frame];
    } /* if */
   } /* color */
  else if (FltValue)
   {
   if (KT[item].Key)
    {
    for (i=0; i<Elements; i++)
     FltValue[i] = KT[item].Val[i][frame];
    } /* if */
   } /* ecosystem */
  FreeGenericKeyTable(KTPtr, MaxFramesPtr);
  } /* if key table */

} /* GetGenericKeyTableValues() */

/************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
short CountGenericKeyFrames(union KeyFrame *KF, short KeyFrames,
	short group, short item)
{
 short i, CountedKeyFrames = 0;

 for (i=0; i<KeyFrames; i++)
  {
  if (group == KF[i].MoKey.Group)
   {
   if (item == KF[i].MoKey.Item)
    {
    CountedKeyFrames ++;
    } /* if item match */
   } /* if group match */
  } /* for i=0... */

 return (CountedKeyFrames);

} /* CountGenericKeyFrames() */
#endif
/***********************************************************************/

short GetActiveGenericKey(struct KeyTable *KTbl, short frame)
{
 short i, prevkey = 0;

 for (i=0; i<KTbl->NumKeys; i++)
  {
  if (frame > KTbl->Key[i]->MoKey.KeyFrame)
   {
   prevkey = i;
   continue;
   }
  if (frame <= KTbl->Key[i]->MoKey.KeyFrame) break;
  } /* for i=0... */

 if (i >= KTbl->NumKeys) return (prevkey);
 return (i);

} /* GetActiveGenericKey() */

/***********************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
short GetNextGenericKeyItem(union KeyFrame *KF, short KeyFrames,
	short group, short curitem, short dir)
{
 short i, nextitem;

 if (dir < 0)
  {
  nextitem = -1;
  for (i=0; i<KeyFrames; i++)
   {
   if (KF[i].MoKey.Group == group)
    {
    if (KF[i].MoKey.Item < curitem && KF[i].MoKey.Item > nextitem)
     {
     if (CountGenericKeyFrames(KF, KeyFrames, group, KF[i].MoKey.Item) > 1)
      {
      nextitem = KF[i].MoKey.Item;
      } /* if more than one key frame designated */
     } /* if item match */
    } /* if group match */
   } /* for i=0... */
  if (nextitem == -1) nextitem = curitem;
  } /* if looking for next lower */
 else
  {
  nextitem = 100;
  for (i=0; i<KeyFrames; i++)
   {
   if (KF[i].MoKey.Group == group)
    {
    if (KF[i].MoKey.Item > curitem && KF[i].MoKey.Item < nextitem)
     {
     if (CountGenericKeyFrames(KF, KeyFrames, group, KF[i].MoKey.Item) > 1)
      {
      nextitem = KF[i].MoKey.Item;
      } /* if more than one key frame designated */
     } /* if item match */
    } /* if group match */
   } /* for i=0... */
  if (nextitem == 100) nextitem = curitem;
  } /* if looking for next lower */

 return (nextitem);

} /* GetNextGenericKeyItem() */
#endif
/***********************************************************************/

short BuildGenericKeyTable(struct KeyTable **KTPtr, union KeyFrame *KF,
	short NumKeys, short *MaxFramesPtr, short Group, short Item,
	short Elements, short Precision, float *MaxMin)
{
 struct KeyTable *KT;

 if (*KTPtr) FreeGenericKeyTable(KTPtr, MaxFramesPtr);

 if ((*KTPtr = (struct KeyTable *)
	get_Memory(sizeof (struct KeyTable), MEMF_CLEAR)) == NULL)
  {
  return (0);
  } /* if out of memory */

 KT = *KTPtr;

 if (NumKeys > 0)
  {
  if ((KT->Key = (union KeyFrame **)
	get_Memory(NumKeys * sizeof (union KeyFrame *), MEMF_CLEAR))
	== NULL)
   {
   free_Memory(KT, sizeof (struct KeyTable));
   *KTPtr = NULL;
   return (0);
   } /* if out of memory */
  KT->NumKeys = NumKeys;
  } /* if key frames exist */

 if (KT->Key)
  {
  SetGenericKeyTableEntry(KF, KT->Key, KT->NumKeys, Group, Item);
  } /* if key frames exist */

 return (SplineGenericKeys(KTPtr, MaxFramesPtr, Elements, Precision, MaxMin));

} /* BuildGenericKeyTable() */

/*********************************************************************/

STATIC_FCN void SetGenericKeyTableEntry(union KeyFrame *KF, union KeyFrame **Key,
	short NumKeys, short Group, short Item) // used locally only -> static, AF 23.7.2021
{
 short i, KeyFrame = 0;

 for (i=0; i<NumKeys; i++)
  {
  if (Group == KF[i].EcoKey2.Group)
   {
   if (Item == KF[i].EcoKey2.Item)
    {
    Key[KeyFrame] = &KF[i];
    KeyFrame ++;
    } /* if item match */
   } /* if group match */
  } /* for i=0... */

} /* SetGenericKeyTableEntry() */

/***********************************************************************/

void FreeGenericKeyTable(struct KeyTable **KTPtr, short *MaxFramesPtr)
{
 struct KeyTable *KT;
 short j;

 KT = *KTPtr;

 if (KT->Key)
  {
  for (j=0; j<10; j++)
   {
   if (KT->Val[j]) 
	free_Memory(KT->Val[j], (*MaxFramesPtr + 1) * sizeof (double));
   } /* for j=0... */
  free_Memory(KT->Key, KT->NumKeys * sizeof (union KeyFrame *));
  } /* if */

 free_Memory(KT, sizeof (struct KeyTable));
 *KTPtr = NULL;
 *MaxFramesPtr = 0;

} /* FreeGenericKeyTable() */

/**********************************************************************/

short SplineGenericKeys(struct KeyTable **KTPtr, short *MaxFrames,
	short Elements, short Precision, float *MaxMin)
{
 short j, k, CurFr, NxtFr, IbtFr, LstInt, NxtInt, OldFrames, Frames, item,
	error = 0, NewArray;
 double Delta, Diff, P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4;
 double *Ptr;
 struct KeyTable *KT;

 KT = *KTPtr;
 OldFrames = *MaxFrames;

 if (KT->Key)
  {
  Frames = KT->Key[KT->NumKeys - 1]->EcoKey2.KeyFrame;
  *MaxFrames = Frames > 100 ? Frames: 100;
  } /* if */

/* Ecosystems */

 if (KT->Key)
  {
  Frames = KT->Key[KT->NumKeys - 1]->EcoKey2.KeyFrame;
  for (item=0, NewArray=1; item<Elements; item++, NewArray=1)
   {
   if (OldFrames == *MaxFrames)
    {
    if (KT->Val[item])
     {
     NewArray = 0;
     Ptr = KT->Val[item];
     } /* if */
    } /* if */
   else if (KT->Val[item])
    free_Memory(KT->Val[item], (OldFrames + 1) * sizeof (double));

   if (NewArray)
    {
    if ((Ptr = (double *)
	get_Memory((*MaxFrames + 1) * sizeof (double), MEMF_CLEAR)) == NULL)
     {
     error = 1;
     break;
     } /* if */
    } /* if */
   if (KT->NumKeys == 1)
    {
    for (j=0; j<=*MaxFrames; j++)
     {
     if (Precision == WCS_KFPRECISION_DOUBLE)
      Ptr[j] = KT->Key[0]->MoKey2.Value[item];
     else if (Precision == WCS_KFPRECISION_FLOAT)
      Ptr[j] = KT->Key[0]->EcoKey2.Value[item];
     else if (Precision == WCS_KFPRECISION_SHORT)
      Ptr[j] = KT->Key[0]->CoKey.Value[item];
     } /* for j=0... */
    KT->Val[item] = Ptr;
    if (MaxMin)
     {
     MaxMin[item * 2] = Ptr[j - 1];			/* max value */
     MaxMin[item * 2 + 1] = Ptr[j - 1];			/* min value */
     } /* if */
    continue;
    } /* if only one key frame */
   if (MaxMin)
    {
    MaxMin[item * 2] = -LARGENUM;			/* max value */
    MaxMin[item * 2 + 1] = +LARGENUM;			/* min value */
    } /* if */
   for (j=0; j<KT->NumKeys - 1; j++)
    {
    CurFr = KT->Key[j]->EcoKey2.KeyFrame;
    NxtFr = KT->Key[j + 1]->EcoKey2.KeyFrame;
    IbtFr = NxtFr - CurFr;

    if (Precision == WCS_KFPRECISION_DOUBLE)
     {
     P1 = KT->Key[j]->MoKey2.Value[item];
     P2 = KT->Key[j + 1]->MoKey2.Value[item];
     }
    else if (Precision == WCS_KFPRECISION_FLOAT)
     {
     P1 = KT->Key[j]->EcoKey2.Value[item];
     P2 = KT->Key[j + 1]->EcoKey2.Value[item];
     }
    else if (Precision == WCS_KFPRECISION_SHORT)
     {
     P1 = KT->Key[j]->CoKey.Value[item];
     P2 = KT->Key[j + 1]->CoKey.Value[item];
     }

    if (IbtFr < 2)
     {
     Ptr[CurFr] = P1;
     Ptr[NxtFr] = P2;
     continue;
     } /* if no inbetween frames */

    Delta = 1.0 / (float)IbtFr;

    if (KT->Key[j + 1]->EcoKey2.Linear)
     {
     Diff = P2 - P1;
     for (k=1; k<IbtFr; k++)
      {
      Ptr[CurFr + k] = P1 + Diff * k * Delta;
      } /* for k=1... */
     } /* if linear segment */
    else
     {
     if (j > 0)
      LstInt = CurFr - KT->Key[j - 1]->EcoKey2.KeyFrame;
     if (j < KT->NumKeys - 2)
      NxtInt = KT->Key[j + 2]->EcoKey2.KeyFrame - NxtFr;

     if (Precision == WCS_KFPRECISION_DOUBLE)
      {
      D1 = j > 0 ?
	((.5 * (P1 - KT->Key[j - 1]->MoKey2.Value[item])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[2])))
	 * (2.0 * IbtFr / (LstInt + IbtFr)):
	((.5 * (P2 - P1)
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[2])));

      D2 = j < KT->NumKeys - 2 ?
	((.5 * (P2 - P1)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[2]))
	 + (.5 * (KT->Key[j + 2]->MoKey2.Value[item] - P2)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[2])))
	 * (2.0 * IbtFr / (IbtFr + NxtInt)):
	((.5 * (P2 - P1)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[2])));
      } /* if double precision */
     if (Precision == WCS_KFPRECISION_FLOAT)
      {
      D1 = j > 0 ?
	((.5 * (P1 - KT->Key[j - 1]->EcoKey2.Value[item])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[2])))
	 * (2.0 * IbtFr / (LstInt + IbtFr)):
	((.5 * (P2 - P1)
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[2])));

      D2 = j < KT->NumKeys - 2 ?
	((.5 * (P2 - P1)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[2]))
	 + (.5 * (KT->Key[j + 2]->EcoKey2.Value[item] - P2)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[2])))
	 * (2.0 * IbtFr / (IbtFr + NxtInt)):
	((.5 * (P2 - P1)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[2])));
      } /* if float precision */
     else if (Precision == WCS_KFPRECISION_SHORT)
      {
      D1 = j > 0 ?
	((.5 * (P1 - KT->Key[j - 1]->CoKey.Value[item])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[2])))
	 * (2.0 * IbtFr / (LstInt + IbtFr)):
	((.5 * (P2 - P1)
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j]->EcoKey2.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j]->EcoKey2.TCB[2])));

      D2 = j < KT->NumKeys - 2 ?
	((.5 * (P2 - P1)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[2]))
	 + (.5 * (KT->Key[j + 2]->CoKey.Value[item] - P2)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[2])))
	 * (2.0 * IbtFr / (IbtFr + NxtInt)):
	((.5 * (P2 - P1)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[0])
	 * (1.0 + KT->Key[j + 1]->EcoKey2.TCB[1])
	 * (1.0 - KT->Key[j + 1]->EcoKey2.TCB[2])));
      } /* if short precision */

     for (k=1; k<IbtFr; k++)
      {
      S1 = k * Delta;
      S2 = S1 * S1;
      S3 = S1 * S2;
      h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
      h2 = -2.0 * S3 + 3.0 * S2;
      h3 = S3 - 2.0 * S2 + S1;
      h4 = S3 - S2;
      Ptr[CurFr + k] = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;
      } /* for k=1... */
     } /* else not linear */
    Ptr[CurFr] = P1;
    if (MaxMin)
     {	
     if (P1 > MaxMin[item * 2]) MaxMin[item * 2] = P1;
     if (P1 < MaxMin[item * 2 + 1]) MaxMin[item * 2 + 1] = P1;
     } /* if */
    } /* for j=0... */

   Ptr[NxtFr] = P2;
   if (MaxMin)
    {	
    if (P2 > MaxMin[item * 2]) MaxMin[item * 2] = P2;
    if (P2 < MaxMin[item * 2 + 1]) MaxMin[item * 2 + 1] = P2;
    } /* if */

   if (KT->Key[0]->EcoKey2.KeyFrame > 0)
    {
    for (j=0; j<KT->Key[0]->EcoKey2.KeyFrame; j++)
     {
     Ptr[j] = Ptr[KT->Key[0]->EcoKey2.KeyFrame];
     } /* for j=0... */
    } /* if first key frame not frame 0 */
   if (Frames < *MaxFrames)
    {
    for (j=Frames + 1; j<=*MaxFrames; j++)
     {
     Ptr[j] = Ptr[Frames];
     } /* for j=0... */
    } /* if highest key frame not == to maximum for all frames */
   KT->Val[item] = Ptr;
   } /* for item=0... */
  } /* if */

 if (error)
  {
  FreeGenericKeyTable(KTPtr, MaxFrames);
  return (0);
  } /* if allocation error */

 return (1);

} /* SplineGenericKeys() */
