/* Params.c (ne gisparams.c 14 Jan 1994 CXH)
** All kinds of functions to set, compute, load, and magle values and params.
** Built/ripped from gis.c on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code by the incomparable Gary R. Huber.
*/

#include "WCS.h"
#include "GUIDefines.h"
#include "GUIExtras.h"


STATIC_FCN void UnsetKeyItem(union KeyFrame *Key); // used locally only -> static, AF 24.7.2021
STATIC_FCN void SetKeyFrame(short i, short frame, short group, short item); // used locally only -> static, AF 24.7.2021
STATIC_FCN short SplineAllKeys(void); // used locally only -> static, AF 24.7.2021
//short GetNextKeyItem(short group, short curitem, short dir); // AF, not used 26.July 2021
STATIC_FCN void SetKeyTableEntry(union KeyFrame **Key, short group, short item); // used locally only -> static, AF 24.7.2021


void MergeKeyFrames(union KeyFrame *MF, short MFKeys, union KeyFrame **OF,
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
     else groupname = "ecosystem";
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

} /* MergeKeyFrames() */

/************************************************************************/

short MakeKeyFrame(short frame, short group, short item)
{
 short i;

/* check to see if key frame already exists */
 if (SearchKeyFrame(frame, group, item) >= 0)
  {
  DeleteKeyFrame(frame, group, item, 0, 0);
  } /* if key frame already exists */

/* check for available space in key frame array */
 if ((ParHdr.KeyFrames + 1) * sizeof (union KeyFrame) > KFsize)
  {
  if (! AllocNewKeyArray(&KF, &KFsize)) return (0);
  } /* if not large enough */

/* find place in list for new key frame */
 if (ParHdr.KeyFrames > 0)
  {
  i = ParHdr.KeyFrames;
  while (frame < KF[i - 1].MoKey.KeyFrame)
   {
   i--;
   if (i == 0) break;
   } /* while */
  memmove(&KF[i+1], &KF[i], (ParHdr.KeyFrames - i) * sizeof (union KeyFrame));
  } /* if already some key frames */
 else i = 0;

 ParHdr.KeyFrames ++;

 memset(&KF[i], 0, sizeof (union KeyFrame));

 SetKeyFrame(i, frame, group, item);

 return (1);

} /* MakeKeyFrame() */

/************************************************************************/

short AllocNewKeyArray(union KeyFrame **KF, long *KFsize)
{
 union KeyFrame *NewKF;
 long NewKFsize;

 NewKFsize = *KFsize + 20 * sizeof (union KeyFrame);
 if ((NewKF = (union KeyFrame *)get_Memory(NewKFsize, MEMF_CLEAR)) == NULL)
  {
  User_Message("Key Frame Module",
	"Out of memory allocating new key frame!\nOperation terminated.", "OK", "o");
  return (0);
  } /* if memory bust */

 memcpy(NewKF, *KF, *KFsize);

 free_Memory(*KF, *KFsize);

 *KF = NewKF;
 *KFsize = NewKFsize;

 return (1);

} /* AllocNewKeyArray() */

/************************************************************************/

short DeleteKeyFrame(short frame, short group, short Item,
	short DeleteAll, short DeleteGp)
{
 short i, found = 0;

 for (i=ParHdr.KeyFrames-1; i>=0; i--)
  {
  if (frame == KF[i].MoKey.KeyFrame)
   {
   if (DeleteAll)
    {
    memmove(&KF[i], &KF[i+1], (ParHdr.KeyFrames - i - 1) * sizeof (union KeyFrame));
    ParHdr.KeyFrames --;
    found = 1;
    continue;
    }
   if (group == KF[i].MoKey.Group)
    {
    if (DeleteGp)
     {
     if (  (KF[i].MoKey.Item == item[0] && incr[0] != 0.0)
	|| (KF[i].MoKey.Item == item[1] && incr[1] != 0.0)
	|| (KF[i].MoKey.Item == item[2] && incr[2] != 0.0))
      {
      memmove(&KF[i], &KF[i+1], (ParHdr.KeyFrames - i - 1) * sizeof (union KeyFrame));
      ParHdr.KeyFrames --;
      found = 1;
      }
     continue;
     }
    if (Item == KF[i].MoKey.Item)
     {
     memmove(&KF[i], &KF[i+1], (ParHdr.KeyFrames - i - 1) * sizeof (union KeyFrame));
     ParHdr.KeyFrames --;
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

} /* DeleteKeyFrame() */

/************************************************************************/

short SearchKeyFrame(short frame, short group, short item)
{
 short i, found = -1;

 for (i=0; i<ParHdr.KeyFrames; i++)
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
/*  else if (KF[i].MoKey.KeyFrame > frame) break;*/
  } /* for i=0... */

 return (found);

} /* SearchKeyFrame() */

/************************************************************************/

STATIC_FCN void SetKeyFrame(short i, short frame, short group, short item) // used locally only -> static, AF 24.7.2021
{

 switch (group)
  {
  case 0:
   {
   KF[i].MoKey.KeyFrame = frame;
   KF[i].MoKey.Group = 0;
   KF[i].MoKey.Item = item;
   if (EM_Win)
    {
    if (item == EM_Win->MoItem)
     {
     KF[i].MoKey.TCB[0] = EM_Win->TCB[0];
     KF[i].MoKey.TCB[1] = EM_Win->TCB[1];
     KF[i].MoKey.TCB[2] = EM_Win->TCB[2];
     KF[i].MoKey.Linear = EM_Win->Linear;
     } /* if item is active item, prevents Update Keys from changing them */
    } /* if motion editor open - only if MakeKey is invoked from Map View might it not be */
   else
    {
    KF[i].MoKey.TCB[0] = 0;
    KF[i].MoKey.TCB[1] = 0;
    KF[i].MoKey.TCB[2] = 0;
    KF[i].MoKey.Linear = 0;
    } /* else */
   KF[i].MoKey.Value = PAR_FIRST_MOTION(item);
   break;
   } /* motion */
  case 1:
   {
   KF[i].CoKey.KeyFrame = frame;
   KF[i].CoKey.Group = 1;
   KF[i].CoKey.Item = item;
   if (item == EC_Win->PalItem)
    {
    KF[i].CoKey.TCB[0] = EC_Win->TCB[0];
    KF[i].CoKey.TCB[1] = EC_Win->TCB[1];
    KF[i].CoKey.TCB[2] = EC_Win->TCB[2];
    KF[i].CoKey.Linear = EC_Win->Linear;
    } /* if */
   KF[i].CoKey.Value[0] = PAR_FIRST_COLOR(item, 0);
   KF[i].CoKey.Value[1] = PAR_FIRST_COLOR(item, 1);
   KF[i].CoKey.Value[2] = PAR_FIRST_COLOR(item, 2);
   break;
   } /* color */
  case 2:
   {
   KF[i].EcoKey.KeyFrame = frame;
   KF[i].EcoKey.Group = 2;
   KF[i].EcoKey.Item = item;
   if (item == EE_Win->EcoItem)
    {
    KF[i].EcoKey.TCB[0] = EE_Win->TCB[0];
    KF[i].EcoKey.TCB[1] = EE_Win->TCB[1];
    KF[i].EcoKey.TCB[2] = EE_Win->TCB[2];
    KF[i].EcoKey.Linear = EE_Win->Linear;
    } /* if */
   KF[i].EcoKey.Line = 		PAR_FIRSTLN_ECO(item);
   KF[i].EcoKey.Skew = 		PAR_FIRSTSK_ECO(item);
   KF[i].EcoKey.SkewAz = 	PAR_FIRSTSA_ECO(item);
   KF[i].EcoKey.RelEl = 	PAR_FIRSTRE_ECO(item);
   KF[i].EcoKey.MaxRelEl = 	PAR_FIRSTXR_ECO(item);
   KF[i].EcoKey.MinRelEl = 	PAR_FIRSTNR_ECO(item);
   KF[i].EcoKey.MaxSlope = 	PAR_FIRSTXS_ECO(item);
   KF[i].EcoKey.MinSlope = 	PAR_FIRSTNS_ECO(item);
   KF[i].EcoKey.Density = 	PAR_FIRSTDN_ECO(item);
   KF[i].EcoKey.Height = 	PAR_FIRSTHT_ECO(item);
   break;
   } /* ecosystem */
  } /* switch group */

} /* SetKeyFrame() */

/************************************************************************/

short UnsetKeyFrame(short frame, short group, short item, short unset)
{
 short i, found = -1, Prev = -1, Next = -1, KeysExist = 0, ItemKeys = 0;

 for (i=0; i<ParHdr.KeyFrames; i++)
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
     UnsetKeyItem(&KF[i]);
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
     UnsetKeyItem(&KF[i]);
    }
   } /* if group match */
  } /* for i=0... */

 switch (group)
  {
  case 0:
   {
   EM_Win->PrevKey = Prev;		/* previous key for this item */
   EM_Win->NextKey = Next;		/* next key for this item */
   EM_Win->KeysExist = KeysExist;	/* num items with keys at this frame */
   EM_Win->ItemKeys = ItemKeys;
   EM_Win->IsKey = found;
   if (found < 0)
    {
    EM_Win->TCB[0] = EM_Win->TCB[1] = EM_Win->TCB[2] = 0.0;
    EM_Win->Linear = 0;
    }
   break;
   } /* motion */
  case 1:
   {
   EC_Win->PrevKey = Prev;
   EC_Win->NextKey = Next;
   EC_Win->KeysExist = KeysExist;
   EC_Win->ItemKeys = ItemKeys;
   EC_Win->IsKey = found;
   if (found < 0)
    {
    EC_Win->TCB[0] = EC_Win->TCB[1] = EC_Win->TCB[2] = 0.0;
    EC_Win->Linear = 0;
    }
   break;
   } /* color */
  case 2:
   {
   EE_Win->PrevKey = Prev;
   EE_Win->NextKey = Next;
   EE_Win->KeysExist = KeysExist;
   EE_Win->ItemKeys = ItemKeys;
   EE_Win->IsKey = found;
   if (found < 0)
    {
    EE_Win->TCB[0] = EE_Win->TCB[1] = EE_Win->TCB[2] = 0.0;
    EE_Win->Linear = 0;
    }
   break;
   } /* ecosystem */
  } /* switch group */

 return (found);

} /* UnsetKeyFrame() */

/************************************************************************/

STATIC_FCN void UnsetKeyItem(union KeyFrame *Key) // used locally only -> static, AF 24.7.2021
{
 short item;

 item = Key->MoKey.Item;
 switch (Key->MoKey.Group)
  {
  case 0:
   {
   if (item == EM_Win->MoItem)
    {
    EM_Win->TCB[0] = Key->MoKey.TCB[0];
    EM_Win->TCB[1] = Key->MoKey.TCB[1];
    EM_Win->TCB[2] = Key->MoKey.TCB[2];
    EM_Win->Linear = Key->MoKey.Linear;
    } /* if item is currently active one */
   PAR_FIRST_MOTION(item) = Key->MoKey.Value;
   break;
   } /* motion */
  case 1:
   {
   if (item == EC_Win->PalItem)
    {
    EC_Win->TCB[0] = Key->CoKey.TCB[0];
    EC_Win->TCB[1] = Key->CoKey.TCB[1];
    EC_Win->TCB[2] = Key->CoKey.TCB[2];
    EC_Win->Linear = Key->CoKey.Linear;
    } /* if */
   PAR_FIRST_COLOR(item, 0) = Key->CoKey.Value[0];
   PAR_FIRST_COLOR(item, 1) = Key->CoKey.Value[1];
   PAR_FIRST_COLOR(item, 2) = Key->CoKey.Value[2];
   break;
   } /* color */
  case 2:
   {
   if (item == EE_Win->EcoItem)
    {
    EE_Win->TCB[0] = Key->EcoKey.TCB[0];
    EE_Win->TCB[1] = Key->EcoKey.TCB[1];
    EE_Win->TCB[2] = Key->EcoKey.TCB[2];
    EE_Win->Linear = Key->EcoKey.Linear;
    } /* if */
   PAR_FIRSTLN_ECO(item) = Key->EcoKey.Line;
   PAR_FIRSTSK_ECO(item) = Key->EcoKey.Skew;
   PAR_FIRSTSA_ECO(item) = Key->EcoKey.SkewAz;
   PAR_FIRSTRE_ECO(item) = Key->EcoKey.RelEl;
   PAR_FIRSTXR_ECO(item) = Key->EcoKey.MaxRelEl;
   PAR_FIRSTNR_ECO(item) = Key->EcoKey.MinRelEl;
   PAR_FIRSTXS_ECO(item) = Key->EcoKey.MaxSlope;
   PAR_FIRSTNS_ECO(item) = Key->EcoKey.MinSlope;
   PAR_FIRSTDN_ECO(item) = Key->EcoKey.Density;
   PAR_FIRSTHT_ECO(item) = Key->EcoKey.Height;
   break;
   } /* ecosystem */
  } /* switch group */

} /* UnsetKeyItem() */

/************************************************************************/

void UpdateKeyFrames(short frame, short group, short Item,
	short UpdateAll, short UpdateGp)
{
 short i;

 for (i=0; i<ParHdr.KeyFrames; i++)
  {
  if (frame == KF[i].MoKey.KeyFrame)
   {
   if (UpdateAll)
    {
    SetKeyFrame(i, frame, group, KF[i].MoKey.Item);
    continue;
    }
   if (group == KF[i].MoKey.Group)
    {
    if (UpdateGp)
     {
     if (  (KF[i].MoKey.Item == item[0] && incr[0] != 0.0)
	|| (KF[i].MoKey.Item == item[1] && incr[1] != 0.0)
	|| (KF[i].MoKey.Item == item[2] && incr[2] != 0.0))
      {
      SetKeyFrame(i, frame, group, KF[i].MoKey.Item);
      } /* if match item */
     continue;
     } /* if update motion group */
    if (Item == KF[i].MoKey.Item)
     {
     SetKeyFrame(i, frame, group, KF[i].MoKey.Item);
     break;
     } /* if item match */
    } /* if group match */
   } /* if frame match */
  else if (KF[i].MoKey.KeyFrame > frame) break;
  } /* for i=0... */

} /* UpdateKeyFrames() */

/************************************************************************/

short BuildKeyTable(void)
{
 short i, error = 0;

 if (KT) FreeKeyTable();
 KT_MaxFrames = 0;

 if ((KT = (struct KeyTable *)
	get_Memory((USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS)
	* sizeof (struct KeyTable), MEMF_CLEAR)) == NULL)
  {
  return (0);
  } /* if out of memory */

 for (i=0; i<USEDMOTIONPARAMS; i++)
  {
  KT[i].NumKeys = CountKeyFrames(0, i);
  }
 for (i=USEDMOTIONPARAMS; i<USEDMOTIONPARAMS + COLORPARAMS; i++)
  {
  KT[i].NumKeys = CountKeyFrames(1, i - USEDMOTIONPARAMS);
  }
 for (i=USEDMOTIONPARAMS + COLORPARAMS; i<USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS; i++)
  {
  KT[i].NumKeys = CountKeyFrames(2, i - USEDMOTIONPARAMS - COLORPARAMS);
  }

 for (i=0; i<USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS; i++)
  {
  if (KT[i].NumKeys > 0)
   {
   if ((KT[i].Key = (union KeyFrame **)
	get_Memory(KT[i].NumKeys * sizeof (union KeyFrame *), MEMF_CLEAR))
	== NULL)
    {
    error = 1;
    break;
    } /* if out of memory */
   } /* if key frames exist */
  } /* for i=0... */

 if (error)
  {
  for (i=0; i<USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS; i++)
   {
   if (KT[i].Key)
    {
    free_Memory(KT[i].Key, KT[i].NumKeys * sizeof (union KeyFrame *));
    } /* if */
   } /* for i=0... */
   free_Memory(KT, (USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS)
	* sizeof (struct KeyTable));
   return (0);
  } /* if memory error */

 for (i=0; i<USEDMOTIONPARAMS; i++)
  {
  if (KT[i].Key)
   {
   SetKeyTableEntry(KT[i].Key, 0, i);
   } /* if key frames exist */
  } /* for i=0... */
 for (i=USEDMOTIONPARAMS; i<USEDMOTIONPARAMS + COLORPARAMS; i++)
  {
  if (KT[i].Key)
   {
   SetKeyTableEntry(KT[i].Key, 1, i - USEDMOTIONPARAMS);
   } /* if key frames exist */
  } /* for i=0... */
 for (i=USEDMOTIONPARAMS + COLORPARAMS; i<USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS; i++)
  {
  if (KT[i].Key)
   {
   SetKeyTableEntry(KT[i].Key, 2, i - USEDMOTIONPARAMS - COLORPARAMS);
   } /* if key frames exist */
  } /* for i=0... */

 return (SplineAllKeys());

} /* BuildKeyTable() */

/************************************************************************/

short BuildSingleKeyTable(short group, short item)
{
 short error = 0, keyframes;

 if ((keyframes = CountKeyFrames(group, item)) < 2)
  return (0);

 if (SKT[group])
  {
  switch (group)
   {
   case 0: FreeSingleKeyTable(group, EMTL_Win->Frames); break;
   case 1: FreeSingleKeyTable(group, ECTL_Win->Frames); break;
   case 2: FreeSingleKeyTable(group, EETL_Win->Frames); break;
   } /* switch group */
  } /* if */

 if ((SKT[group] = (struct KeyTable *)
	get_Memory(sizeof (struct KeyTable), MEMF_CLEAR)) == NULL)
  {
  return (0);
  } /* if out of memory */

 SKT[group]->NumKeys = keyframes;

 if ((SKT[group]->Key = (union KeyFrame **)
	get_Memory(SKT[group]->NumKeys * sizeof (union KeyFrame *), MEMF_CLEAR))
	== NULL)
  {
  error = 1;
  } /* if out of memory */

 if (error)
  {
  if (SKT[group]->Key)
   {
   free_Memory(SKT[group]->Key, SKT[group]->NumKeys * sizeof (union KeyFrame *));
   } /* if */
  free_Memory(SKT[group], sizeof (struct KeyTable));
  return (0);
  } /* if memory error */

 if (SKT[group]->Key)
  {
  SetKeyTableEntry(SKT[group]->Key, group, item);
  } /* if key frames exist */

 return (SplineSingleKey(group, 1));

} /* BuildSingleKeyTable() */

/*********************************************************************/

void GetKeyTableValues(short group, short item, short allvalues)
{
 short i, j, k, frame, focframe;

 if (BuildKeyTable())
  {
  switch (group)
   {
   case 0:
    {
    frame = EM_Win->Frame > KT_MaxFrames ? KT_MaxFrames: EM_Win->Frame;

    for (i=0; i<USEDMOTIONPARAMS; i++)
     {
     if (KT[i].Key)
      {
      if (allvalues || i == item)
       PAR_FIRST_MOTION(i) = KT[i].Val[0][frame];
      }
     } /* for i=0... */
    if (settings.lookahead && allvalues)
     {
     focframe = EM_Win->Frame + settings.lookaheadframes > KT_MaxFrames ?
		 KT_MaxFrames: EM_Win->Frame + settings.lookaheadframes;
     if (frame == focframe && frame > 0)
      {
      if (KT[0].Key)
       PAR_FIRST_MOTION(3) = KT[0].Val[0][frame] + (KT[0].Val[0][frame] - KT[0].Val[0][frame - 1]);
      if (KT[1].Key)
       PAR_FIRST_MOTION(4) = KT[1].Val[0][frame] + (KT[1].Val[0][frame] - KT[1].Val[0][frame - 1]);
      if (KT[2].Key)
       PAR_FIRST_MOTION(5) = KT[2].Val[0][frame] + (KT[2].Val[0][frame] - KT[2].Val[0][frame - 1]);
      } /* if last frame */
     else
      {
      if (KT[0].Key)
       PAR_FIRST_MOTION(3) = KT[0].Val[0][focframe];
      if (KT[1].Key)
       PAR_FIRST_MOTION(4) = KT[1].Val[0][focframe];
      if (KT[2].Key)
       PAR_FIRST_MOTION(5) = KT[2].Val[0][focframe];
      } /* else normal look ahead */
     } /* if look ahead */
    break;
    } /* motion */
   case 1:
    {
    frame = EC_Win->Frame > KT_MaxFrames ? KT_MaxFrames: EC_Win->Frame;

    for (i=USEDMOTIONPARAMS, j=0; i<USEDMOTIONPARAMS + COLORPARAMS; i++, j++)
     {
     if (KT[i].Key)
      {
      if (allvalues || i == item)
       {
       for (k=0; k<3; k++)
        PAR_FIRST_COLOR(j, k) = KT[i].Val[k][frame];
       } /* if */
      } /* if key frames */
     } /* for i=0... */
    break;
    } /* color */
   case 2:
    {
    frame = EE_Win->Frame > KT_MaxFrames ? KT_MaxFrames: EE_Win->Frame;

    for (i=USEDMOTIONPARAMS + COLORPARAMS, j=0;
	 i<USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS; i++, j++)
     {
     if (KT[i].Key)
      {
      if (allvalues || i == item)
       {
       PAR_FIRSTLN_ECO(j) = KT[i].Val[0][frame];
       PAR_FIRSTSK_ECO(j) = KT[i].Val[1][frame];
       PAR_FIRSTSA_ECO(j) = KT[i].Val[2][frame];
       PAR_FIRSTRE_ECO(j) = KT[i].Val[3][frame]; 
       PAR_FIRSTXR_ECO(j) = KT[i].Val[4][frame];
       PAR_FIRSTNR_ECO(j) = KT[i].Val[5][frame];
       PAR_FIRSTXS_ECO(j) = KT[i].Val[6][frame];
       PAR_FIRSTNS_ECO(j) = KT[i].Val[7][frame];
       PAR_FIRSTDN_ECO(j) = KT[i].Val[8][frame];
       PAR_FIRSTHT_ECO(j) = KT[i].Val[9][frame];
       } /* if */
      } /* if key frames */
     } /* for i=0... */
    break;
    } /* ecosystem */
   } /* switch */
  FreeKeyTable();
  } /* if key table */

} /* GetKeyTableValues() */

/************************************************************************/

short CountKeyFrames(short group, short item)
{
 short i, KeyFrames = 0;

 for (i=0; i<ParHdr.KeyFrames; i++)
  {
  if (group == KF[i].MoKey.Group)
   {
   if (item == KF[i].MoKey.Item)
    {
    KeyFrames ++;
    } /* if item match */
   } /* if group match */
  } /* for i=0... */

 return (KeyFrames);

} /* CountKeyFrames() */

/***********************************************************************/

short GetActiveKey(struct KeyTable *KTbl, short frame)
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

} /* GetActiveKey() */

/***********************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
short GetNextKeyItem(short group, short curitem, short dir)
{
 short i, nextitem;

 if (dir < 0)
  {
  nextitem = -1;
  for (i=0; i<ParHdr.KeyFrames; i++)
   {
   if (KF[i].MoKey.Group == group)
    {
    if (KF[i].MoKey.Item < curitem && KF[i].MoKey.Item > nextitem)
     {
     if (CountKeyFrames(group, KF[i].MoKey.Item) > 1)
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
  for (i=0; i<ParHdr.KeyFrames; i++)
   {
   if (KF[i].MoKey.Group == group)
    {
    if (KF[i].MoKey.Item > curitem && KF[i].MoKey.Item < nextitem)
     {
     if (CountKeyFrames(group, KF[i].MoKey.Item) > 1)
      {
      nextitem = KF[i].MoKey.Item;
      } /* if more than one key frame designated */
     } /* if item match */
    } /* if group match */
   } /* for i=0... */
  if (nextitem == 100) nextitem = curitem;
  } /* if looking for next lower */

 return (nextitem);

} /* GetNextKeyItem() */
#endif
/***********************************************************************/

STATIC_FCN void SetKeyTableEntry(union KeyFrame **Key, short group, short item) // used locally only -> static, AF 24.7.2021
{
 short i, KeyFrame = 0;

 for (i=0; i<ParHdr.KeyFrames; i++)
  {
  if (group == KF[i].MoKey.Group)
   {
   if (item == KF[i].MoKey.Item)
    {
    Key[KeyFrame] = &KF[i];
    KeyFrame ++;
    } /* if item match */
   } /* if group match */
  } /* for i=0... */

} /* SetKeyTableEntry() */

/***********************************************************************/

void FreeKeyTable(void)
{
 short i, j;

 for (i=0; i<USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS; i++)
  {
  if (KT[i].Key)
   {
   for (j=0; j<10; j++)
    {
    if (KT[i].Val[j]) 
	free_Memory(KT[i].Val[j], (KT_MaxFrames + 1) * sizeof (double));
    } /* for j=0... */
   free_Memory(KT[i].Key, KT[i].NumKeys * sizeof (union KeyFrame *));
   } /* if */
  } /* for i=0... */

 free_Memory(KT, (USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS)
	* sizeof (struct KeyTable));
 KT = NULL;
 KT_MaxFrames = 0;

} /* FreeKeyTable() */

/***********************************************************************/

void FreeSingleKeyTable(short group, short frames)
{
 short j;

  if (SKT[group]->Key)
   {
   for (j=0; j<10; j++)
    {
    if (SKT[group]->Val[j]) 
	free_Memory(SKT[group]->Val[j], ((frames + 1) * sizeof (double)));
    } /* for j=0... */
   free_Memory(SKT[group]->Key, SKT[group]->NumKeys * sizeof (union KeyFrame *));
   } /* if */

 free_Memory(SKT[group], sizeof (struct KeyTable));
 SKT[group] = NULL;

} /* FreeSingleKeyTable() */

/***********************************************************************/

short SplineSingleKey(short group, short newkey)
{
 short j, k, CurFr, NxtFr, IbtFr, LstInt, NxtInt, Frames, item, error = 0;
 double Delta, Diff, P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4;
 double *Ptr;

 if (! SKT[group]->Key) return (0);

 switch (group)
  {
  case 0:
   {
   Frames = SKT[group]->Key[SKT[group]->NumKeys - 1]->MoKey.KeyFrame;
   EMTL_Win->Frames = Frames < 100 ? 100: Frames;
   if (newkey)
    {
    if ((Ptr = (double *)
	get_Memory((EMTL_Win->Frames + 1) * sizeof (double), MEMF_CLEAR))
	== NULL)
     {
     error = 1;
     break;
     } /* if out of memory */
    } /* if newkey */
   else Ptr = SKT[group]->Val[0];
   EMTL_Win->MaxMin[0][0] = -LARGENUM;			/* max value */
   EMTL_Win->MaxMin[0][1] = +LARGENUM;			/* min value */
   for (j=0; j<SKT[group]->NumKeys - 1; j++)
    {
    CurFr = SKT[group]->Key[j]->MoKey.KeyFrame;
    NxtFr = SKT[group]->Key[j + 1]->MoKey.KeyFrame;
    IbtFr = NxtFr - CurFr;	/* actually 1 more than inbetween frames for
				 computational efficiency */
    P1 = SKT[group]->Key[j]->MoKey.Value;
    P2 = SKT[group]->Key[j + 1]->MoKey.Value;

    if (IbtFr > 1)
     {
     Delta = 1.0 / (float)IbtFr;

     if (SKT[group]->Key[j + 1]->MoKey.Linear)
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
       LstInt = CurFr - SKT[group]->Key[j - 1]->MoKey.KeyFrame;
      if (j < SKT[group]->NumKeys - 2)
       NxtInt = SKT[group]->Key[j + 2]->MoKey.KeyFrame - NxtFr;

      D1 = j > 0 ?
	((.5 * (P1 - SKT[group]->Key[j - 1]->MoKey.Value)
	 * (1.0 - SKT[group]->Key[j]->MoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j]->MoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j]->MoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j]->MoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j]->MoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j]->MoKey.TCB[2])))
	 * (2.0 * IbtFr / (LstInt + IbtFr)):
	((.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j]->MoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j]->MoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j]->MoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j]->MoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j]->MoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j]->MoKey.TCB[2])));

      D2 = j < SKT[group]->NumKeys - 2 ?
	((.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j + 1]->MoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j + 1]->MoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j + 1]->MoKey.TCB[2]))
	 + (.5 * (SKT[group]->Key[j + 2]->MoKey.Value - P2)
	 * (1.0 - SKT[group]->Key[j + 1]->MoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j + 1]->MoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j + 1]->MoKey.TCB[2])))
	 * (2.0 * IbtFr / (IbtFr + NxtInt)):
	((.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j + 1]->MoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j + 1]->MoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j + 1]->MoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j + 1]->MoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j + 1]->MoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j + 1]->MoKey.TCB[2])));

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
       if (Ptr[CurFr + k] > EMTL_Win->MaxMin[0][0])
	 EMTL_Win->MaxMin[0][0] = Ptr[CurFr + k];
       if (Ptr[CurFr + k] < EMTL_Win->MaxMin[0][1])
	 EMTL_Win->MaxMin[0][1] = Ptr[CurFr + k];
       } /* for k=1... */
      } /* else not linear */
     } /* if at least one inbetween frame */
    Ptr[CurFr] = P1;	
    if (P1 > EMTL_Win->MaxMin[0][0]) EMTL_Win->MaxMin[0][0] = P1;
    if (P1 < EMTL_Win->MaxMin[0][1]) EMTL_Win->MaxMin[0][1] = P1;

    } /* for j=0... */

   Ptr[NxtFr] = P2;
   if (P2 > EMTL_Win->MaxMin[0][0]) EMTL_Win->MaxMin[0][0] = P2;
   if (P2 < EMTL_Win->MaxMin[0][1]) EMTL_Win->MaxMin[0][1] = P2;

   if (SKT[group]->Key[0]->MoKey.KeyFrame > 0)
    {
    for (j=0; j< SKT[group]->Key[0]->MoKey.KeyFrame; j++)
     {
     Ptr[j] = Ptr[SKT[group]->Key[0]->MoKey.KeyFrame];
     } /* for j=0... */
    } /* if first key frame not frame 1 */

   if (Frames < EMTL_Win->Frames)
    {
    for (j=Frames + 1; j<=EMTL_Win->Frames; j++)
     {
     Ptr[j] = Ptr[Frames];
     } /* for j=... */
    } /* if highest key frame not == to maximum for all frames */

   SKT[group]->Val[0] = Ptr;
   break;
   } /* motion key */

  case 1:
   {
   Frames = SKT[group]->Key[SKT[group]->NumKeys - 1]->CoKey.KeyFrame;
   ECTL_Win->Frames = Frames < 100 ? 100: Frames;
   for (item=0; item<3; item++)
    {
    if (newkey)
     {
     if ((Ptr = (double *)
	get_Memory((ECTL_Win->Frames + 1) * sizeof (double), MEMF_CLEAR)) == NULL)
      {
      error = 1;
      break;
      } /* if out of memory */
     } /* if newkey */
    else Ptr = SKT[group]->Val[item];
    ECTL_Win->MaxMin[item][0] = -LARGENUM;		/* max value */
    ECTL_Win->MaxMin[item][1] = +LARGENUM;		/* min value */
    for (j=0; j<SKT[group]->NumKeys - 1; j++)
     {
     CurFr = SKT[group]->Key[j]->CoKey.KeyFrame;
     NxtFr = SKT[group]->Key[j + 1]->CoKey.KeyFrame;
     IbtFr = NxtFr - CurFr;

     P1 = SKT[group]->Key[j]->CoKey.Value[item];
     P2 = SKT[group]->Key[j + 1]->CoKey.Value[item];

     if (IbtFr > 1)
      {
      Delta = 1.0 / (float)IbtFr;

      if (SKT[group]->Key[j + 1]->CoKey.Linear)
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
        LstInt = CurFr - SKT[group]->Key[j - 1]->CoKey.KeyFrame;
       if (j < SKT[group]->NumKeys - 2)
        NxtInt = SKT[group]->Key[j + 2]->CoKey.KeyFrame - NxtFr;

       D1 = j > 0 ?
	((.5 * (P1 - SKT[group]->Key[j - 1]->CoKey.Value[item])
	 * (1.0 - SKT[group]->Key[j]->CoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j]->CoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j]->CoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j]->CoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j]->CoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j]->CoKey.TCB[2])))
	 * (2.0 * IbtFr / (LstInt + IbtFr)):
	((.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j]->CoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j]->CoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j]->CoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j]->CoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j]->CoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j]->CoKey.TCB[2])));

       D2 = j < SKT[group]->NumKeys - 2 ?
	((.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j + 1]->CoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j + 1]->CoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j + 1]->CoKey.TCB[2]))
	 + (.5 * (SKT[group]->Key[j + 2]->CoKey.Value[item] - P2)
	 * (1.0 - SKT[group]->Key[j + 1]->CoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j + 1]->CoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j + 1]->CoKey.TCB[2])))
	 * (2.0 * IbtFr / (IbtFr + NxtInt)):
	((.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j + 1]->CoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j + 1]->CoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j + 1]->CoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j + 1]->CoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j + 1]->CoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j + 1]->CoKey.TCB[2])));

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
        if (Ptr[CurFr + k] > 255) Ptr[CurFr + k] = 255;
        else if (Ptr[CurFr + k] < 0) Ptr[CurFr + k] = 0;
        if (Ptr[CurFr + k] > ECTL_Win->MaxMin[item][0])
	 ECTL_Win->MaxMin[item][0] = Ptr[CurFr + k];
        if (Ptr[CurFr + k] < ECTL_Win->MaxMin[item][1])
	 ECTL_Win->MaxMin[item][1] = Ptr[CurFr + k];
        } /* for k=1... */
       } /* else not linear */
      } /* if at least one inbetween frame */
     Ptr[CurFr] = P1;	
     if (P1 > ECTL_Win->MaxMin[item][0]) ECTL_Win->MaxMin[item][0] = P1;
     if (P1 < ECTL_Win->MaxMin[item][1]) ECTL_Win->MaxMin[item][1] = P1;

     } /* for j=0... */

    Ptr[NxtFr] = P2;
    if (P2 > ECTL_Win->MaxMin[item][0]) ECTL_Win->MaxMin[item][0] = P2;
    if (P2 < ECTL_Win->MaxMin[item][1]) ECTL_Win->MaxMin[item][1] = P2;

    if (SKT[group]->Key[0]->CoKey.KeyFrame > 0)
     {
     for (j=0; j< SKT[group]->Key[0]->CoKey.KeyFrame; j++)
      {
      Ptr[j] = Ptr[SKT[group]->Key[0]->CoKey.KeyFrame];
      } /* for j=0... */
     } /* if first key frame not frame 1 */
    if (Frames < ECTL_Win->Frames)
     {
     for (j=Frames + 1; j<=ECTL_Win->Frames; j++)
      {
      Ptr[j] = Ptr[Frames];
      } /* for j=0... */
     } /* if highest key frame not == to maximum for all frames */

    SKT[group]->Val[item] = Ptr;
    } /* for item=0... */
   break;
   } /* color key */

  case 2:
   {
   Frames = SKT[group]->Key[SKT[group]->NumKeys - 1]->EcoKey2.KeyFrame;
   EETL_Win->Frames = Frames < 100 ? 100: Frames;
   for (item=0; item<10; item++)
    {
    if (newkey)
     {
     if ((Ptr = (double *)
	get_Memory((EETL_Win->Frames + 1) * sizeof (double), MEMF_CLEAR)) == NULL)
      {
      error = 1;
      break;
      } /* if out of memory */
     } /* if newkey */
    else Ptr = SKT[group]->Val[item];
    EETL_Win->MaxMin[item][0] = -LARGENUM;		/* max value */
    EETL_Win->MaxMin[item][1] = +LARGENUM;		/* min value */
    for (j=0; j<SKT[group]->NumKeys - 1; j++)
     {
     CurFr = SKT[group]->Key[j]->EcoKey2.KeyFrame;
     NxtFr = SKT[group]->Key[j + 1]->EcoKey2.KeyFrame;
     IbtFr = NxtFr - CurFr;

     P1 = SKT[group]->Key[j]->EcoKey2.Value[item];
     P2 = SKT[group]->Key[j + 1]->EcoKey2.Value[item];

     if (IbtFr > 1)
      {
      Delta = 1.0 / (float)IbtFr;

      if (SKT[group]->Key[j + 1]->EcoKey2.Linear)
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
        LstInt = CurFr - SKT[group]->Key[j - 1]->EcoKey2.KeyFrame;
       if (j < SKT[group]->NumKeys - 2)
        NxtInt = SKT[group]->Key[j + 2]->EcoKey2.KeyFrame - NxtFr;

       D1 = j > 0 ?
	((.5 * (P1 - SKT[group]->Key[j - 1]->EcoKey2.Value[item])
	 * (1.0 - SKT[group]->Key[j]->EcoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j]->EcoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j]->EcoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j]->EcoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j]->EcoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j]->EcoKey.TCB[2])))
	 * (2.0 * IbtFr / (LstInt + IbtFr)):
	((.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j]->EcoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j]->EcoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j]->EcoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j]->EcoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j]->EcoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j]->EcoKey.TCB[2])));

       D2 = j < SKT[group]->NumKeys - 2 ?
	((.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j + 1]->EcoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j + 1]->EcoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j + 1]->EcoKey.TCB[2]))
	 + (.5 * (SKT[group]->Key[j + 2]->EcoKey2.Value[item] - P2)
	 * (1.0 - SKT[group]->Key[j + 1]->EcoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j + 1]->EcoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j + 1]->EcoKey.TCB[2])))
	 * (2.0 * IbtFr / (IbtFr + NxtInt)):
	((.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j + 1]->EcoKey.TCB[0])
	 * (1.0 - SKT[group]->Key[j + 1]->EcoKey.TCB[1])
	 * (1.0 + SKT[group]->Key[j + 1]->EcoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - SKT[group]->Key[j + 1]->EcoKey.TCB[0])
	 * (1.0 + SKT[group]->Key[j + 1]->EcoKey.TCB[1])
	 * (1.0 - SKT[group]->Key[j + 1]->EcoKey.TCB[2])));

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
        if (Ptr[CurFr + k] > EETL_Win->MaxMin[item][0])
	 EETL_Win->MaxMin[item][0] = Ptr[CurFr + k];
        if (Ptr[CurFr + k] < EETL_Win->MaxMin[item][1])
	 EETL_Win->MaxMin[item][1] = Ptr[CurFr + k];
        } /* for k=1... */
       } /* else not linear */
      } /* if at least one inbetween frame */
     Ptr[CurFr] = P1;	
     if (P1 > EETL_Win->MaxMin[item][0]) EETL_Win->MaxMin[item][0] = P1;
     if (P1 < EETL_Win->MaxMin[item][1]) EETL_Win->MaxMin[item][1] = P1;

     } /* for j=0... */

    Ptr[NxtFr] = P2;
    if (P2 > EETL_Win->MaxMin[item][0]) EETL_Win->MaxMin[item][0] = P2;
    if (P2 < EETL_Win->MaxMin[item][1]) EETL_Win->MaxMin[item][1] = P2;

    if (SKT[group]->Key[0]->EcoKey2.KeyFrame > 0)
     {
     for (j=0; j< SKT[group]->Key[0]->EcoKey2.KeyFrame; j++)
      {
      Ptr[j] = Ptr[SKT[group]->Key[0]->EcoKey2.KeyFrame];
      } /* for j=0... */
     } /* if first key frame not frame 1 */
    if (Frames < EETL_Win->Frames)
     {
     for (j=Frames + 1; j<=EETL_Win->Frames; j++)
      {
      Ptr[j] = Ptr[Frames];
      } /* for j=0... */
     } /* if highest key frame not == to maximum for all frames */

    SKT[group]->Val[item] = Ptr;
    } /* for item=0... */
   break;
   } /* ecosystem key */

  } /* switch group */

 if (error) return (0);
 return (1);

} /* SplineSingleKey() */

/***********************************************************************/

STATIC_FCN short SplineAllKeys(void) // used locally only -> static, AF 24.7.2021
{
 short i, j, k, CurFr, NxtFr, IbtFr, LstInt, NxtInt, Frames, item, error = 0,
	MaxCamFrames = 0, MaxFocFrames = 0;
 double Delta, Diff, P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4;
 double *Ptr;
 struct VelocityDistr Vel;


 for (i=0; i<3; i++)
  {
  if (KT[i].Key)
   {
   Frames = KT[i].Key[KT[i].NumKeys - 1]->MoKey.KeyFrame;
   if (Frames > KT_MaxFrames) KT_MaxFrames = MaxCamFrames = Frames;
   } /* if */
  } /* for i=0... */
 for (i=3; i<6; i++)
  {
  if (KT[i].Key)
   {
   Frames = KT[i].Key[KT[i].NumKeys - 1]->MoKey.KeyFrame;
   if (Frames > KT_MaxFrames) KT_MaxFrames = Frames;
   if (Frames > MaxFocFrames) MaxFocFrames = Frames;
   } /* if */
  } /* for i=0... */
 for (i=6; i<USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS; i++)
  {
  if (KT[i].Key)
   {
   Frames = KT[i].Key[KT[i].NumKeys - 1]->MoKey.KeyFrame;
   if (Frames > KT_MaxFrames) KT_MaxFrames = Frames;
   } /* if */
  } /* for i=0... */
 if (KT_MaxFrames < 100)
  KT_MaxFrames = 100;

 for (i=0; i<USEDMOTIONPARAMS; i++)
  {
  if (KT[i].Key)
   {
   Frames = KT[i].Key[KT[i].NumKeys - 1]->MoKey.KeyFrame;
   if ((Ptr = (double *)
	get_Memory((KT_MaxFrames + 1) * sizeof (double), MEMF_CLEAR)) == NULL)
    {
    error = 1;
    break;
    }
   if (KT[i].NumKeys == 1)
    {
    for (j=0; j<=KT_MaxFrames; j++)
     {
     Ptr[j] = KT[i].Key[0]->MoKey.Value;
     } /* for j=0... */
    KT[i].Val[0] = Ptr;
    continue;
    } /* if only one key frame */
   for (j=0; j<KT[i].NumKeys - 1; j++)
    {
    CurFr = KT[i].Key[j]->MoKey.KeyFrame;
    NxtFr = KT[i].Key[j + 1]->MoKey.KeyFrame;
    IbtFr = NxtFr - CurFr;

    P1 = KT[i].Key[j]->MoKey.Value;
    P2 = KT[i].Key[j + 1]->MoKey.Value;

    if (IbtFr < 2)
     {
     Ptr[CurFr] = P1;
     Ptr[NxtFr] = P2;
     continue;
     } /* if no inbetween frames */

    Delta = 1.0 / (float)IbtFr;

    if (KT[i].Key[j + 1]->MoKey.Linear)
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
      LstInt = CurFr - KT[i].Key[j - 1]->MoKey.KeyFrame;
     if (j < KT[i].NumKeys - 2)
      NxtInt = KT[i].Key[j + 2]->MoKey.KeyFrame - NxtFr;

     D1 = j > 0 ?
	((.5 * (P1 - KT[i].Key[j - 1]->MoKey.Value)
	 * (1.0 - KT[i].Key[j]->MoKey.TCB[0])
	 * (1.0 + KT[i].Key[j]->MoKey.TCB[1])
	 * (1.0 + KT[i].Key[j]->MoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j]->MoKey.TCB[0])
	 * (1.0 - KT[i].Key[j]->MoKey.TCB[1])
	 * (1.0 - KT[i].Key[j]->MoKey.TCB[2])))
	 * (2.0 * IbtFr / (LstInt + IbtFr)):
	((.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j]->MoKey.TCB[0])
	 * (1.0 + KT[i].Key[j]->MoKey.TCB[1])
	 * (1.0 + KT[i].Key[j]->MoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j]->MoKey.TCB[0])
	 * (1.0 - KT[i].Key[j]->MoKey.TCB[1])
	 * (1.0 - KT[i].Key[j]->MoKey.TCB[2])));

     D2 = j < KT[i].NumKeys - 2 ?
	((.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j + 1]->MoKey.TCB[0])
	 * (1.0 - KT[i].Key[j + 1]->MoKey.TCB[1])
	 * (1.0 + KT[i].Key[j + 1]->MoKey.TCB[2]))
	 + (.5 * (KT[i].Key[j + 2]->MoKey.Value - P2)
	 * (1.0 - KT[i].Key[j + 1]->MoKey.TCB[0])
	 * (1.0 + KT[i].Key[j + 1]->MoKey.TCB[1])
	 * (1.0 - KT[i].Key[j + 1]->MoKey.TCB[2])))
	 * (2.0 * IbtFr / (IbtFr + NxtInt)):
	((.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j + 1]->MoKey.TCB[0])
	 * (1.0 - KT[i].Key[j + 1]->MoKey.TCB[1])
	 * (1.0 + KT[i].Key[j + 1]->MoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j + 1]->MoKey.TCB[0])
	 * (1.0 + KT[i].Key[j + 1]->MoKey.TCB[1])
	 * (1.0 - KT[i].Key[j + 1]->MoKey.TCB[2])));

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
    } /* for j=0... */

   Ptr[NxtFr] = P2;

   if (KT[i].Key[0]->MoKey.KeyFrame > 0)
    {
    for (j=0; j< KT[i].Key[0]->MoKey.KeyFrame; j++)
     {
     Ptr[j] = Ptr[KT[i].Key[0]->MoKey.KeyFrame];
     } /* for j=0... */
    } /* if first key frame not frame 0 */
   if (Frames < KT_MaxFrames)
    {
    for (j=Frames + 1; j<=KT_MaxFrames; j++)
     {
     Ptr[j] = Ptr[Frames];
     } /* for j=0... */
    } /* if highest key frame not == to maximum for all frames */
   KT[i].Val[0] = Ptr;
   } /* if */
  } /* for i=0... */

 if (error)
  {
  FreeKeyTable();
  return (0);
  } /* if allocation error */

/* Colors */
 for (i=USEDMOTIONPARAMS; i<USEDMOTIONPARAMS + COLORPARAMS; i++)
  {
  if (KT[i].Key)
   {
   Frames = KT[i].Key[KT[i].NumKeys - 1]->CoKey.KeyFrame;
   for (item=0; item<3; item++)
    {
    if ((Ptr = (double *)
	get_Memory((KT_MaxFrames + 1) * sizeof (double), MEMF_CLEAR)) == NULL)
     {
     error = 1;
     break;
     }
    if (KT[i].NumKeys == 1)
     {
     for (j=0; j<=KT_MaxFrames; j++)
      {
      Ptr[j] = KT[i].Key[0]->CoKey.Value[item];
      } /* for j=0... */
     KT[i].Val[item] = Ptr;
     continue;
     } /* if only one key frame */
    for (j=0; j<KT[i].NumKeys - 1; j++)
     {
     CurFr = KT[i].Key[j]->CoKey.KeyFrame;
     NxtFr = KT[i].Key[j + 1]->CoKey.KeyFrame;
     IbtFr = NxtFr - CurFr;

     P1 = KT[i].Key[j]->CoKey.Value[item];
     P2 = KT[i].Key[j + 1]->CoKey.Value[item];

     if (IbtFr < 2)
      {
      Ptr[CurFr] = P1;
      Ptr[NxtFr] = P2;
      continue;
      } /* if no inbetween frames */

     Delta = 1.0 / (float)IbtFr;

     if (KT[i].Key[j + 1]->CoKey.Linear)
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
       LstInt = CurFr - KT[i].Key[j - 1]->CoKey.KeyFrame;
      if (j < KT[i].NumKeys - 2)
       NxtInt = KT[i].Key[j + 2]->CoKey.KeyFrame - NxtFr;

      D1 = j > 0 ?
	((.5 * (P1 - KT[i].Key[j - 1]->CoKey.Value[item])
	 * (1.0 - KT[i].Key[j]->CoKey.TCB[0])
	 * (1.0 + KT[i].Key[j]->CoKey.TCB[1])
	 * (1.0 + KT[i].Key[j]->CoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j]->CoKey.TCB[0])
	 * (1.0 - KT[i].Key[j]->CoKey.TCB[1])
	 * (1.0 - KT[i].Key[j]->CoKey.TCB[2])))
	 * (2.0 * IbtFr / (LstInt + IbtFr)):
	((.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j]->CoKey.TCB[0])
	 * (1.0 + KT[i].Key[j]->CoKey.TCB[1])
	 * (1.0 + KT[i].Key[j]->CoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j]->CoKey.TCB[0])
	 * (1.0 - KT[i].Key[j]->CoKey.TCB[1])
	 * (1.0 - KT[i].Key[j]->CoKey.TCB[2])));

      D2 = j < KT[i].NumKeys - 2 ?
	((.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j + 1]->CoKey.TCB[0])
	 * (1.0 - KT[i].Key[j + 1]->CoKey.TCB[1])
	 * (1.0 + KT[i].Key[j + 1]->CoKey.TCB[2]))
	 + (.5 * (KT[i].Key[j + 2]->CoKey.Value[item] - P2)
	 * (1.0 - KT[i].Key[j + 1]->CoKey.TCB[0])
	 * (1.0 + KT[i].Key[j + 1]->CoKey.TCB[1])
	 * (1.0 - KT[i].Key[j + 1]->CoKey.TCB[2])))
	 * (2.0 * IbtFr / (IbtFr + NxtInt)):
	((.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j + 1]->CoKey.TCB[0])
	 * (1.0 - KT[i].Key[j + 1]->CoKey.TCB[1])
	 * (1.0 + KT[i].Key[j + 1]->CoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j + 1]->CoKey.TCB[0])
	 * (1.0 + KT[i].Key[j + 1]->CoKey.TCB[1])
	 * (1.0 - KT[i].Key[j + 1]->CoKey.TCB[2])));

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
       if (Ptr[CurFr + k] > 255) Ptr[CurFr + k] = 255;
       else if (Ptr[CurFr + k] < 0) Ptr[CurFr + k] = 0;
       } /* for k=1... */
      } /* else not linear */
     Ptr[CurFr] = P1;	
     } /* for j=0... */

    Ptr[NxtFr] = P2;

    if (KT[i].Key[0]->CoKey.KeyFrame > 0)
     {
     for (j=0; j< KT[i].Key[0]->CoKey.KeyFrame; j++)
      {
      Ptr[j] = Ptr[KT[i].Key[0]->CoKey.KeyFrame];
      } /* for j=0... */
     } /* if first key frame not frame 0 */
    if (Frames < KT_MaxFrames)
     {
     for (j=Frames + 1; j<=KT_MaxFrames; j++)
      {
      Ptr[j] = Ptr[Frames];
      } /* for j=0... */
     } /* if highest key frame not == to maximum for all frames */

/* Used this to create a strata texture map
if (item == 0)
 {
 for (j=0; j<=KT_MaxFrames; j+=10)
  printf("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d,\n",
	(int)Ptr[j], (int)Ptr[j+1],(int)Ptr[j+2], (int)Ptr[j+3], (int)Ptr[j+4],
	(int)Ptr[j+5], (int)Ptr[j+6], (int)Ptr[j+7], (int)Ptr[j+8], (int)Ptr[j+9]);
 }
*/
    KT[i].Val[item] = Ptr;
    } /* for item=0... */
   if (error) break;
   } /* if */
  } /* for i=0... */

 if (error)
  {
  FreeKeyTable();
  return (0);
  } /* if allocation error */

/* Ecosystems */
 for (i=USEDMOTIONPARAMS + COLORPARAMS; i<USEDMOTIONPARAMS + COLORPARAMS + ECOPARAMS; i++)
  {
  if (KT[i].Key)
   {
   Frames = KT[i].Key[KT[i].NumKeys - 1]->EcoKey2.KeyFrame;
   for (item=0; item<10; item++)
    {
    if ((Ptr = (double *)
	get_Memory((KT_MaxFrames + 1) * sizeof (double), MEMF_CLEAR)) == NULL)
     {
     error = 1;
     break;
     }
    if (KT[i].NumKeys == 1)
     {
     for (j=0; j<=KT_MaxFrames; j++)
      {
      Ptr[j] = KT[i].Key[0]->EcoKey2.Value[item];
      } /* for j=0... */
     KT[i].Val[item] = Ptr;
     continue;
     } /* if only one key frame */
    for (j=0; j<KT[i].NumKeys - 1; j++)
     {
     CurFr = KT[i].Key[j]->EcoKey2.KeyFrame;
     NxtFr = KT[i].Key[j + 1]->EcoKey2.KeyFrame;
     IbtFr = NxtFr - CurFr;

     P1 = KT[i].Key[j]->EcoKey2.Value[item];
     P2 = KT[i].Key[j + 1]->EcoKey2.Value[item];

     if (IbtFr < 2)
      {
      Ptr[CurFr] = P1;
      Ptr[NxtFr] = P2;
      continue;
      } /* if no inbetween frames */

     Delta = 1.0 / (float)IbtFr;

     if (KT[i].Key[j + 1]->EcoKey2.Linear)
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
       LstInt = CurFr - KT[i].Key[j - 1]->EcoKey2.KeyFrame;
      if (j < KT[i].NumKeys - 2)
       NxtInt = KT[i].Key[j + 2]->EcoKey2.KeyFrame - NxtFr;

      D1 = j > 0 ?
	((.5 * (P1 - KT[i].Key[j - 1]->EcoKey2.Value[item])
	 * (1.0 - KT[i].Key[j]->EcoKey.TCB[0])
	 * (1.0 + KT[i].Key[j]->EcoKey.TCB[1])
	 * (1.0 + KT[i].Key[j]->EcoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j]->EcoKey.TCB[0])
	 * (1.0 - KT[i].Key[j]->EcoKey.TCB[1])
	 * (1.0 - KT[i].Key[j]->EcoKey.TCB[2])))
	 * (2.0 * IbtFr / (LstInt + IbtFr)):
	((.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j]->EcoKey.TCB[0])
	 * (1.0 + KT[i].Key[j]->EcoKey.TCB[1])
	 * (1.0 + KT[i].Key[j]->EcoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j]->EcoKey.TCB[0])
	 * (1.0 - KT[i].Key[j]->EcoKey.TCB[1])
	 * (1.0 - KT[i].Key[j]->EcoKey.TCB[2])));

      D2 = j < KT[i].NumKeys - 2 ?
	((.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j + 1]->EcoKey.TCB[0])
	 * (1.0 - KT[i].Key[j + 1]->EcoKey.TCB[1])
	 * (1.0 + KT[i].Key[j + 1]->EcoKey.TCB[2]))
	 + (.5 * (KT[i].Key[j + 2]->EcoKey2.Value[item] - P2)
	 * (1.0 - KT[i].Key[j + 1]->EcoKey.TCB[0])
	 * (1.0 + KT[i].Key[j + 1]->EcoKey.TCB[1])
	 * (1.0 - KT[i].Key[j + 1]->EcoKey.TCB[2])))
	 * (2.0 * IbtFr / (IbtFr + NxtInt)):
	((.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j + 1]->EcoKey.TCB[0])
	 * (1.0 - KT[i].Key[j + 1]->EcoKey.TCB[1])
	 * (1.0 + KT[i].Key[j + 1]->EcoKey.TCB[2]))
	 + (.5 * (P2 - P1)
	 * (1.0 - KT[i].Key[j + 1]->EcoKey.TCB[0])
	 * (1.0 + KT[i].Key[j + 1]->EcoKey.TCB[1])
	 * (1.0 - KT[i].Key[j + 1]->EcoKey.TCB[2])));

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
     } /* for j=0... */

    Ptr[NxtFr] = P2;

    if (KT[i].Key[0]->EcoKey2.KeyFrame > 0)
     {
     for (j=0; j< KT[i].Key[0]->EcoKey2.KeyFrame; j++)
      {
      Ptr[j] = Ptr[KT[i].Key[0]->EcoKey2.KeyFrame];
      } /* for j=0... */
     } /* if first key frame not frame 0 */
    if (Frames < KT_MaxFrames)
     {
     for (j=Frames + 1; j<=KT_MaxFrames; j++)
      {
      Ptr[j] = Ptr[Frames];
      } /* for j=0... */
     } /* if highest key frame not == to maximum for all frames */
    KT[i].Val[item] = Ptr;
    } /* for item=0... */
   if (error) break;
   } /* if */
  } /* for i=0... */

 if (error)
  {
  FreeKeyTable();
  return (0);
  } /* if allocation error */
 else if (settings.velocitydistr && KT_MaxFrames > 2)
  {
  Vel.PtsIn = MaxCamFrames;
  Vel.PtsOut = MaxCamFrames;
  Vel.EaseIn = settings.easein;
  Vel.EaseOut = settings.easeout;
  Vel.Base = 0;
  if (Vel.EaseIn + Vel.EaseOut > KT_MaxFrames)
   {
   if (! User_Message("Parameters Module: Velocity Distribution",
	"\"Ease In\" plus \"Ease Out\" frame values exceed total number of animated frames.\n\
This is illegal! Do you wish to continue without Velocity Distribution?",
	"OK|Cancel", "oc"))
    return (0);
   else
    return (1);
   } /* if illegal values */
/* camera */
  if (KT[1].Key)
   Vel.Lat = KT[1].Val[0];
  else
   Vel.Lat = NULL;
  if (KT[2].Key)
   Vel.Lon = KT[2].Val[0];
  else
   Vel.Lon = NULL;
  if (KT[0].Key)
   Vel.Alt = KT[0].Val[0];
  else
   Vel.Alt = NULL;
  DistributeVelocity(&Vel);

/* focus */
  Vel.PtsIn = MaxFocFrames;
  Vel.PtsOut = MaxFocFrames;
  if (KT[4].Key)
   Vel.Lat = KT[4].Val[0];
  else
   Vel.Lat = NULL;
  if (KT[5].Key)
   Vel.Lon = KT[5].Val[0];
  else
   Vel.Lon = NULL;
  if (KT[3].Key)
   Vel.Alt = KT[3].Val[0];
  else
   Vel.Alt = NULL;
  DistributeVelocity(&Vel);

  } /* else */

 return (1);

} /* SplineAllKeys() */



/***********************************************************************/
/* Play Functions */

void Play_Colors(void)
{
 short frame;
 ULONG InputID = 0;
 struct PaletteItem Pal;

 while (InputID != ID_ECTL_PLAY)
  {
  for (frame=0; frame<ECTL_Win->Frames; frame++)
   {
   Pal.red = SKT[1]->Val[0][frame];
   Pal.grn = SKT[1]->Val[1][frame];
   Pal.blu = SKT[1]->Val[2][frame];
   SetScreen_8(&Pal);
   sprintf(str, "%1d", frame);
   set(ECTL_Win->FrameTxt, MUIA_Text_Contents, str);
   Delay(1);
   if ((InputID = CheckInput_ID()) == ID_ECTL_PLAY) break;
   } /* for frame=1... */
  } /* while */

} /* PlayColors() */


#ifdef GJGJHJGJHGJH
// Spline junk - just a simplified accounting of the path spline function.

    IbtFr = NxtFr - CurFr;
    if (j > 0)
     LstInt = CurFr - KT[i].Key[j - 1]->EcoKey2.KeyFrame;
    if (j < SKT[group]->NumKeys - 2)
     NxtInt = KT[i].Key[j + 2]->EcoKey2.KeyFrame - NxtFr;

    Delta = 1.0 / (float)IbtFr;
    P0 = LastValue;
    P1 = CurValue;
    P2 = NextValue;
    P3 = 2ndNextValue;
    CT = CurTension;
    CC = CurContinuity;
    CB = CurBias;
    NT = NextTension;
    NC = NextContinuity;
    NB = NextBias;


/* Note: for second conditions (first and last intervals) of both D1 and D2
	no frame spacing need be taken into account because both factors
	would be the same and cancel */

    D1 = j > 0 ?
	((.5 * (P1 - P0) * (1.0 - CT) * (1.0 + CC) * (1.0 + CB))
	 + (.5 * (P2 - P1) * (1.0 - CT) * (1.0 - CC) * (1.0 - CB)))
	 * (2.0 * IbtFr / (LstInt + IbtFr)):

	((.5 * (P2 - P1) * (1.0 - CT) * (1.0 + CC) * (1.0 + CB))
	 + (.5 * (P2 - P1) * (1.0 - CT) * (1.0 - CC) * (1.0 - CB)));




    D2 = j < SKT[group]->NumKeys - 2 ?
	((.5 * (P2 - P1) * (1.0 - NT) * (1.0 - NC) * (1.0 + NB))
	 + (.5 * (P3 - P2) * (1.0 - NT) * (1.0 + NC) * (1.0 - NB)))
	 * (2.0 * IbtFr / (IbtFr + NxtInt)):


	((.5 * (P2 - P1) * (1.0 - NT) * (1.0 - NC) * (1.0 + NB))
	 + (.5 * (P2 - P1) * (1.0 - NT) * (1.0 + NC) * (1.0 - NB)));


#endif
