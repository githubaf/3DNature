/* GenericParams.c (ne gisparams.c 14 Jan 1994 CXH)
** All kinds of functions to set, compute, load, and magle values and params.
** Built/ripped from gis.c on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code and major modifications by the incomparable Gary R. Huber.
** Copyright 1995 by Questar Productions.
*/

#ifndef GENERICPARAMS_H
#define GENERICPARAMS_H

/* these are used for converting formats between key frame data and key table arrays */
#define WCS_KFPRECISION_DOUBLE	8
#define WCS_KFPRECISION_FLOAT	6
#define WCS_KFPRECISION_LONG	4	/* currently not used or implemented */
#define WCS_KFPRECISION_SHORT	2
#define WCS_KFPRECISION_BYTE	1	/* currently not used or implemented */

/* function protos */

//void MergeGenericKeyFrames(union KeyFrame *MF, short MFKeys, union KeyFrame **OF,
//	short *OFKeys, long *OFsize, short group); // used locally only -> static, AF 23.7.2021
short MakeGenericKeyFrame(union KeyFrame **KFPtr, long *KFSizePtr,
	short *KeyFramesPtr, short frame, short group, short item,
	short ItemMatch, short NumValues, double *DblValue, float *FltValue,
	short *ShortValue, float *TCB, short Linear, short Precision);
// short AllocNewGenericKeyArray(union KeyFrame **KF, long *KFsize); // used locally only -> static, AF 23.7.2021
short DeleteGenericKeyFrame(union KeyFrame *KF, short *KeyFramesPtr,
	short frame, short group, short Item, short DeleteAll, short DeleteGp,
	short *ItemVals, double *IncrVals);
/*short SearchGenericKeyFrame(union KeyFrame *KF, short KeyFrames,
	short frame, short group, short item);*/  // used locally only -> static, AF 19.7.2021
//void SetGenericKeyFrame(union KeyFrame *KF, short i, short frame, short group,
//	short item, short ItemMatch, short NumValues, double *DblValue,
//	float *FltValue, short *ShortValue, float *TCB, short Linear,
//	short Precision); // used locally only -> static, AF 23.7.2021
short UnsetGenericKeyFrame(union KeyFrame *KF, short KeyFrames,
	struct WindowKeyStuff *WKS,
	short frame, short group, short item, short unset,
	short ItemMatch, short NumValues, double *DblValue, float *FltValue,
	short *ShortValue, float *TCB, short *LinearPtr, short Precision);
static void UnsetGenericKeyItem(union KeyFrame *Key,
	short ItemMatch, short NumValues, double *DblValue, float *FltValue,
	short *ShortValue, float *TCB, short *LinearPtr, short Precision);   // used locally only -> static, AF 23.7.2021
void UpdateGenericKeyFrames(union KeyFrame *KF, short KeyFrames,
	short frame, short group, short Item, short ItemMatch, short NumValues,
	double *DblValue, float *FltValue, short *ShortValue, float *TCB,
	short Linear,
	short UpdateAll, short UpdateGp, short *ItemVals, double *IncrVals,
	short Precision);
void GetGenericKeyTableValues(struct KeyTable **KTPtr, union KeyFrame *KF,
	short NumKeys, short *MaxFramesPtr, short Elements,
	short group, short item, short frame,
	double *DblValue, float *FltValue, short *ShortValue, short Precision);
//short CountGenericKeyFrames(union KeyFrame *KF, short KeyFrames,
//	short group, short item); // used locally only -> static, AF 23.7.2021
short GetActiveGenericKey(struct KeyTable *KTbl, short frame);
// short GetNextGenericKeyItem(union KeyFrame *KF, short KeyFrames, 
//	short group, short curitem, short dir);  // used locally only -> static, AF 23.7.2021
short BuildGenericKeyTable(struct KeyTable **KTPtr, union KeyFrame *KF,
	short NumKeys, short *MaxFramesPtr, short Group, short Item,
	short Elements, short Precision, float *MaxMin);
// void SetGenericKeyTableEntry(union KeyFrame *KF, union KeyFrame **Key,
//	short NumKeys, short Group, short Item); // used locally only -> static, AF 23.7.2021
void FreeGenericKeyTable(struct KeyTable **KTPtr, short *MaxFramesPtr);
short SplineGenericKeys(struct KeyTable **KTPtr, short *MaxFrames,
	short Elements, short Precision, float *MaxMin);

#endif /* GENERICPARAMS_H */
