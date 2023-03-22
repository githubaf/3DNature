/* Foliage.c
** Foliage and Ecotype-related functions
** Built 11/95 by Gary R. Huber.
** Copyright Questar Productions.
*/

#include "WCS.h"
#include "Foliage.h"
#include "BigEndianReadWrite.h"

STATIC_FCN ULONG ReadBlock(FILE *ffile, char *Block, ULONG Flags);  // used locally only -> static, AF 19.7.2021
STATIC_FCN ULONG WriteBlock(FILE *ffile, char *Block, ULONG Flags); // used locally only -> static, AF 19.7.2021
STATIC_FCN ULONG Foliage_Save(struct Foliage *This, FILE *ffile); // used locally only -> static, AF 24.7.2021
//long Rootstock_Save(struct Rootstock *This, FILE *ffile); // AF, not used 26.July 2021
STATIC_FCN struct Foliage *Foliage_Load(FILE *ffile, ULONG ReadSize); // used locally only -> static, AF 24.7.2021


#ifndef C_PLUS_PLUS
/************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 17.July 2021
struct Rootstock *Rootstock_New(void)
	{

	return ((struct Rootstock *)get_Memory
			(sizeof (struct Rootstock), MEMF_CLEAR));

	} /* Rootstock_New() */
#endif
/************************************************************************/

struct Ecotype *Ecotype_New(void)
	{

	return ((struct Ecotype *)get_Memory
			(sizeof (struct Ecotype), MEMF_CLEAR));

	} /* Ecotype_New() */

/************************************************************************/

struct FoliageGroup *FoliageGroup_New(void)
	{

	return ((struct FoliageGroup *)get_Memory
			(sizeof (struct FoliageGroup), MEMF_CLEAR));

	} /* FoliageGroup_New() */

/************************************************************************/

struct Foliage *Foliage_New(void)
	{

	return ((struct Foliage *)get_Memory
			(sizeof (struct Foliage), MEMF_CLEAR));

	} /* Foliage_New() */

/************************************************************************/

void DisposeEcotypes(void)
{
short i;
 
 for (i=0; i<ECOPARAMS; i++)
  {
  if (EcoShift[i].Ecotype)
   {
   Ecotype_Del(EcoShift[i].Ecotype);
   EcoShift[i].Ecotype = NULL;
   } /* if */
  } /* for i=0... */

} /* DisposeEcotypes() */

/************************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 17.July 2021
void Rootstock_Del(struct Rootstock *This)
	{

	if (This)
		{
		if (This->Name)
			free_Memory(This->Name, This->NameSize);
		if (This->ColorName)
			free_Memory(This->ColorName, This->ColorNameSize);
		free_Memory(This, sizeof (struct Rootstock));
		} /* if */

	} /* Rootstock_Del() */
#endif
/************************************************************************/

void Ecotype_Del(struct Ecotype *This)
	{

	if (This)
		{
		if (This->FolGp)
			FoliageGroup_Del(This->FolGp, 1);
		if (This->Root.Name)
			free_Memory(This->Root.Name, This->Root.NameSize);
		if (This->Root.ColorName)
			free_Memory(This->Root.ColorName, This->Root.ColorNameSize);
		free_Memory(This, sizeof (struct Ecotype));
		} /* if */

	} /* Ecotype_Del() */

/************************************************************************/

void FoliageGroup_Del(struct FoliageGroup *This, short DelAll)
	{

	if (This)
		{
		if (This->Next && DelAll)
			FoliageGroup_Del(This->Next, 1);
		if (This->Fol)
			Foliage_Del(This->Fol, 1);
		if (This->Root.Name)
			free_Memory(This->Root.Name, This->Root.NameSize);
		if (This->Root.ColorName)
			free_Memory(This->Root.ColorName, This->Root.ColorNameSize);
		free_Memory(This, sizeof (struct FoliageGroup));
		} /* if */

	} /* FoliageGroup_Del() */

/************************************************************************/

void Foliage_Del(struct Foliage *This, short DelAll)
	{

	if (This)
		{
		if (This->Next && DelAll)
			Foliage_Del(This->Next, 1);
		if (This->Root.Name)
			free_Memory(This->Root.Name, This->Root.NameSize);
		if (This->Root.ColorName)
			free_Memory(This->Root.ColorName, This->Root.ColorNameSize);
		free_Memory(This, sizeof (struct Foliage));
		} /* if */

	} /* Foliage_Del() */

/************************************************************************/

void Rootstock_SetValue(struct Rootstock *This,
			ULONG Flags, float FlVal, short ShtVal)
	{

	switch (Flags)
		{
		case WCS_ECOTYPE_HEIGHT:
			{
			This->Height = FlVal;
			break;
			}
		case WCS_ECOTYPE_DENSITY:
			{
			This->Density = FlVal;
			break;
			}
		case WCS_ECOTYPE_USEIMGCOL:
			{
			This->UseImgCol = ShtVal;
			break;
			}
		case WCS_ECOTYPE_PALCOL:
			{
			This->PalCol = ShtVal;
			break;
			}
		case WCS_ECOTYPE_MAXIMGHT:
			{
			This->MaxImgHeight = ShtVal;
			break;
			}
		} /* switch */

	} /* Rootstock_SetValue() */

/************************************************************************/

char *Rootstock_SetName(struct Rootstock *This, char *NameStr)
	{
	short Len;

	if (This->Name)
		free_Memory(This->Name, This->NameSize);
	Len = strlen(NameStr) + 1;
	if ((This->Name = (char *)get_Memory(Len, MEMF_ANY)))
		{
		This->NameSize = Len;
		strcpy(This->Name, NameStr);
		} /* if */
	else
		This->NameSize = 0;

	return (This->Name);

	} /* Rootstock_SetName() */

/************************************************************************/

char *Rootstock_SetColorName(struct Rootstock *This, char *NameStr)
	{
	short Len;

	if (This->ColorName)
		free_Memory(This->ColorName, This->ColorNameSize);
	Len = strlen(NameStr) + 1;
	if ((This->ColorName = (char *)get_Memory(Len, 0)))
		{
		This->ColorNameSize = Len;
		strcpy(This->ColorName, NameStr);
		} /* if */
	else
		This->ColorNameSize = 0;

	return (This->ColorName);

	} /* Rootstock_SetColorName() */

/**********************************************************************/

void Foliage_SetImgSize(struct Foliage *This, ULONG Flags, short ShtVal)
	{

	switch (Flags)
		{
		case WCS_ECOTYPE_IMGHEIGHT:
			{
			This->ImgHeight = ShtVal;
			break;
			}
		case WCS_ECOTYPE_IMGWIDTH:
			{
			This->ImgWidth = ShtVal;
			break;
			}
		case WCS_ECOTYPE_COLORIMAGE:
			{
			This->ColorImage = ShtVal;
			break;
			}
		} /* switch */

	} /* Foliage_SetImgSize() */

/***********************************************************************/

float Rootstock_GetFloatValue(struct Rootstock *This, ULONG Flags)
	{

	switch (Flags)
		{
		case WCS_ECOTYPE_HEIGHT:
			{
			return(This->Height);
			break;
			}
		case WCS_ECOTYPE_DENSITY:
			{
			return (This->Density);
			break;
			}
		default:
		    {
		    printf("Invalid Flags %lu in %s %d\n",Flags,__FILE__,__LINE__);
		    return 0;
		    }
		} /* switch */

	} /* Rootstock_GetFloatValue() */

/**********************************************************************/

short Rootstock_GetShortValue(struct Rootstock *This, ULONG Flags)
	{

	switch (Flags)
		{
		case WCS_ECOTYPE_USEIMGCOL:
			{
			return (This->UseImgCol);
			break;
			}
		case WCS_ECOTYPE_PALCOL:
			{
			return (This->PalCol);
			break;
			}
		case WCS_ECOTYPE_MAXIMGHT:
			{
			return (This->MaxImgHeight);
			break;
			}
        default:
            {
            printf("Invalid Flags %lu in %s %d\n",Flags,__FILE__,__LINE__);
            return 0;
            }


		} /* switch */

	} /* Rootstock_GetValue() */

/************************************************************************/

char *Rootstock_GetName(struct Rootstock *This)
	{

	return (This->Name);

	} /* Rootstock_SetName() */

/************************************************************************/

char *Rootstock_GetColorName(struct Rootstock *This)
	{

	return (This->ColorName);

	} /* Rootstock_SetColorName() */

/************************************************************************/

char **Rootstock_GetNameAddr(struct Rootstock *This)
	{

	return (&This->Name);

	} /* Rootstock_GetNameAddr() */

/**********************************************************************/

char **Foliage_GetImageNameAddr(struct Foliage *This)
	{
	short i;
	
	i = strlen(This->Root.Name) - 1;
	while (This->Root.Name[i] != ':' && This->Root.Name[i] != '/' && This->Root.Name[i] != '\\' && i >= 0)
		i --;

	This->NameAddr = &This->Root.Name[i + 1];
	return (&This->NameAddr);

	} /* Foliage_GetImageNameAddr() */

/**********************************************************************/

short Foliage_GetImgSize(struct Foliage *This, ULONG Flags)
	{

	switch (Flags)
		{
		case WCS_ECOTYPE_IMGHEIGHT:
			{
			return (This->ImgHeight);
			break;
			}
		case WCS_ECOTYPE_IMGWIDTH:
			{
			return (This->ImgWidth);
			break;
			}
		case WCS_ECOTYPE_COLORIMAGE:
			{
			return (This->ColorImage);
			break;
			}
        default:
            {
            printf("Invalid Flags %lu in %s %d\n",Flags,__FILE__,__LINE__);
            return 0;
            }

		} /* switch */

	} /* Foliage_SetImgSize() */

/***********************************************************************/

void Ecotype_SetDefaults(struct Ecotype *This, struct Ecosystem *Eco)
{

 Rootstock_SetName(&This->Root, Eco->Name);
 Rootstock_SetColorName(&This->Root, PAR_NAME_COLOR(Eco->Color));
 This->Root.Height = Eco->Height;
 This->Root.Density = Eco->Density / 100.0;
 This->Root.UseImgCol = 0;
 This->Root.PalCol = Eco->Color;
 This->Root.MaxImgHeight = 640;

} /* Ecotype_SetDefaults() */

/***********************************************************************/

void FoliageGroup_SetDefaults(struct FoliageGroup *This,
	struct Ecotype *Proto)
{
struct FoliageGroup *Another;
float Sum = 0.0;
long Groups = 0;

 Another = Proto->FolGp;
 while (Another && Another != This)
  {
  Sum += Another->Root.Density;
  Another = Another->Next;
  Groups ++;
  } /* while */

 This->Root.Height = 1.0;
 if (Groups)
  This->Root.Density = Sum / Groups;
 else
  This->Root.Density = 1.0;

 This->Root.UseImgCol = Proto->Root.UseImgCol;
 This->Root.PalCol = Proto->Root.PalCol;
 This->Root.MaxImgHeight = Proto->Root.MaxImgHeight;
 This->Root.ColorName = Rootstock_SetColorName(&This->Root,
			Proto->Root.ColorName);

} /* FoliageGroup_SetDefaults() */

/***********************************************************************/

void Foliage_SetDefaults(struct Foliage *This,
	struct FoliageGroup *Proto)
{
struct Foliage *Another;
float Sum = 0.0;
long Groups = 0;

 Another = Proto->Fol;
 while (Another && Another != This)
  {
  Sum += Another->Root.Density;
  Another = Another->Next;
  Groups ++;
  } /* while */

 This->Root.Height = 1.0;
 if (Groups)
  This->Root.Density = Sum / Groups;
 else
  This->Root.Density = 1.0;

 This->Root.UseImgCol = Proto->Root.UseImgCol;
 This->Root.PalCol = Proto->Root.PalCol;
 This->Root.MaxImgHeight = Proto->Root.MaxImgHeight;
 This->Root.ColorName = Rootstock_SetColorName(&This->Root,
			Proto->Root.ColorName);

} /* Foliage_SetDefaults() */

/***********************************************************************/

STATIC_FCN ULONG ReadBlock(FILE *ffile, char *Block, ULONG Flags) // used locally only -> static, AF 19.7.2021
{
ULONG SizeRead, SizeBlock;

 SizeBlock = Flags & 0x00ff;

 SizeRead = SizeBlock * fread(Block, SizeBlock, 1, ffile);

#ifdef WIN_32

/* byte swapping */
 if (SizeRead)
  {
  switch (Flags & 0xff00)
   {
   case WCS_BLOCKTYPE_SHORTINT:
    {
    break;
    }
   case WCS_BLOCKTYPE_LONGINT:
    {
    break;
    }
   case WCS_BLOCKTYPE_FLOAT:
    {
    break;
    }
   case WCS_BLOCKTYPE_DOUBLE:
    {
    break;
    }
   } /* switch */
  }/* if something read */

#endif /* WIN_32 */ 

 return (SizeRead);

} /* ReadBlock() */

/***********************************************************************/

STATIC_FCN ULONG WriteBlock(FILE *ffile, char *Block, ULONG Flags) // used locally only -> static, AF 19.7.2021
{
ULONG SizeWritten, SizeBlock;

 SizeBlock = Flags & 0x00ff;

#ifdef WIN_32

/* byte swapping */
 if (SizeBlock)
  {
  switch (Flags & 0xff00)
   {
   case WCS_BLOCKTYPE_SHORTINT:
    {
    break;
    }
   case WCS_BLOCKTYPE_LONGINT:
    {
    break;
    }
   case WCS_BLOCKTYPE_FLOAT:
    {
    break;
    }
   case WCS_BLOCKTYPE_DOUBLE:
    {
    break;
    }
   } /* switch */
  }/* if something to write */

#endif /* WIN_32 */ 

 SizeWritten = SizeBlock * fwrite(Block, SizeBlock, 1, ffile);

 return (SizeWritten);

} /* WriteBlock() */

/***********************************************************************/

STATIC_FCN ULONG PrepWriteBlock(FILE *ffile, ULONG ItemTag, ULONG SizeSize,
	ULONG SizeType, ULONG FieldSize, ULONG FieldType, char *FieldAddr) // used locally only -> static, AF 19.7.2021
{
UBYTE CharSize;
USHORT ShortSize;
ULONG LongSize, BytesWritten, TotalWritten = 0;


 ItemTag += SizeSize + SizeType;

 if ((BytesWritten = (WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG))) == 0)
  {
  goto EndSave;
  } /* if */
 TotalWritten += BytesWritten;

 switch (SizeSize)
  {
  case WCS_BLOCKSIZE_CHAR:
   {
   CharSize = FieldSize;
   if ((BytesWritten = WriteBlock(ffile, (char *)&CharSize,
	SizeSize + SizeType)) == 0)
    {
    goto EndSave;
    } /* if */
   break;
   }
  case WCS_BLOCKSIZE_SHORT:
   {
   ShortSize = FieldSize;
   if ((BytesWritten = WriteBlock(ffile, (char *)&ShortSize,
	SizeSize + SizeType)) == 0)
    {
    goto EndSave;
    } /* if */
   break;
   }
  case WCS_BLOCKSIZE_LONG:
   {
   LongSize = FieldSize;
   if ((BytesWritten = WriteBlock(ffile, (char *)&LongSize,
	SizeSize + SizeType)) == 0)
    {
    goto EndSave;
    } /* if */
   break;
   }
  } /* switch */
 TotalWritten += BytesWritten;

 if ((BytesWritten = WriteBlock(ffile, (char *)FieldAddr,
	FieldSize + FieldType)) == 0)
  {
  goto EndSave;
  } /* if */
 TotalWritten += BytesWritten;

 return (TotalWritten);

EndSave:

 return (0L);

} /* PrepWriteBlock() */

/***********************************************************************/

union MultiVal {
 char Char[4];
 short Short[2];
 long Long;
};

struct Ecotype *Ecotype_Load(FILE *ffile, ULONG ReadSize)
{
struct Ecotype *This;
struct FoliageGroup *FolGp, **FolGpPtr;
char TempStr[256];
short ShortVal;
float FloatVal;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

 if ((This = Ecotype_New()))
  {
  FolGpPtr = &This->FolGp;

  while (ItemTag != WCS_ECOTYPE_DONE)
   {
	/* read block descriptor tag from file */
   if ((BytesRead = ReadBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG)))
    {
    ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32U(ItemTag, &ItemTag);)   // AF: 4.Jan.2023, Endian correction for i386-aros
    TotalRead += BytesRead;
    if (ItemTag != WCS_ECOTYPE_DONE)
     {
	/* read block size from file */
     if ((BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff)))
      {
      TotalRead += BytesRead;
      BytesRead = 0;
      switch (ItemTag & 0xff)
       {
       case WCS_BLOCKSIZE_CHAR:
        {
        Size = MV.Char[0];
        break;
	}
       case WCS_BLOCKSIZE_SHORT:
        {
        ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(MV.Short[0], &MV.Short[0]);)   // AF: 4.Jan.2023, Endian correction for i386-aros
        Size = MV.Short[0];
        break;
	}
       case WCS_BLOCKSIZE_LONG:
        {
        ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32S(MV.Long, &MV.Long);)   // AF: 4.Jan.2023, Endian correction for i386-aros
        Size = MV.Long;
        break;
	}
       } /* switch */

      if (Size < 256 || (ItemTag & 0xffff0000) == WCS_ECOTYPE_FOLIAGEGRP)
       {
       switch (ItemTag & 0xffff0000)
        {
        case WCS_ECOTYPE_NAME:
         {
         BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size);
         TempStr[255] = 0;
         Rootstock_SetName(&This->Root, TempStr);
         break;
	 }
        case WCS_ECOTYPE_COLORNAME:
         {
         BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size);
         TempStr[255] = 0;
         Rootstock_SetColorName(&This->Root, TempStr);
         break;
	 }
        case WCS_ECOTYPE_HEIGHT:
         {
         BytesRead = ReadBlock(ffile, (char *)&FloatVal, WCS_BLOCKTYPE_FLOAT + Size);
         // AF: FloatVal is only a single float, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32F(FloatVal, &FloatVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_HEIGHT, FloatVal, 0);
         break;
	 }
        case WCS_ECOTYPE_DENSITY:
         {
         BytesRead = ReadBlock(ffile, (char *)&FloatVal, WCS_BLOCKTYPE_FLOAT + Size);
         // AF: FloatVal is only a single float, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32F(FloatVal, &FloatVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_DENSITY, FloatVal, 0);
         break;
	 }
        case WCS_ECOTYPE_USEIMGCOL:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_USEIMGCOL, (float)0.0, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_PALCOL:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_PALCOL, (float)0.0, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_MAXIMGHT:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_MAXIMGHT, (float)0.0, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_FOLIAGEGRP:
         {
         if ((*FolGpPtr = FoliageGroup_Load(ffile, Size)))
          {
          FolGp = *FolGpPtr;
          FolGpPtr = &FolGp->Next;
          BytesRead = Size;
	  } /* if foliage group read OK */
         break;
	 }
        default:
         {
         BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size);
         break;
	 } 
	} /* switch */

       TotalRead += BytesRead;
       } /* if block < 256 bytes */
      if (BytesRead != Size)
       break;
      } /* if size block read */
     else
      break;
     } /* if not done flag */
    } /* if tag block read */
   else
    break;
   } /* while */

  if (ReadSize > 0 && ReadSize != TotalRead)
   {
   Ecotype_Del(This);
   This = NULL;
   } /* if error reading */
  } /* if new ecotype */

 return (This);

} /* Ecotype_Load() */

/***********************************************************************/

struct FoliageGroup *FoliageGroup_Load(FILE *ffile, ULONG ReadSize)
{
struct FoliageGroup *This;
struct Foliage *Fol, **FolPtr;
char TempStr[256];
short ShortVal;
float FloatVal;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

 if ((This = FoliageGroup_New()))
  {
  FolPtr = &This->Fol;

	/* read block descriptor tag from file */
  while (ItemTag != WCS_ECOTYPE_DONE)
   {
   if ((BytesRead = ReadBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG)))
    {
    ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32U(ItemTag, &ItemTag);)   // AF: 4.Jan.2023, Endian correction for i386-aros
    TotalRead += BytesRead;
    if (ItemTag != WCS_ECOTYPE_DONE)
     {
     if ((BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff)))
      {
      TotalRead += BytesRead;
      BytesRead = 0;
      switch (ItemTag & 0xff)
       {
       case WCS_BLOCKSIZE_CHAR:
        {
        Size = MV.Char[0];
        break;
	}
       case WCS_BLOCKSIZE_SHORT:
        {
        ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(MV.Short[0], &MV.Short[0]);)   // AF: 4.Jan.2023, Endian correction for i386-aros
        Size = MV.Short[0];
        break;
	}
       case WCS_BLOCKSIZE_LONG:
        {
        ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32S(MV.Long, &MV.Long);)   // AF: 4.Jan.2023, Endian correction for i386-aros
        Size = MV.Long;
        break;
	}
       } /* switch */

      if (Size < 256 || (ItemTag & 0xffff0000) == WCS_ECOTYPE_FOLIAGE)
       {
       switch (ItemTag & 0xffff0000)
        {
        case WCS_ECOTYPE_NAME:
         {
         BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size);
         TempStr[255] = 0;
         Rootstock_SetName(&This->Root, TempStr);
         break;
	 }
        case WCS_ECOTYPE_COLORNAME:
         {
         BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size);
         TempStr[255] = 0;
         Rootstock_SetColorName(&This->Root, TempStr);
         break;
	 }
        case WCS_ECOTYPE_HEIGHT:
         {
         BytesRead = ReadBlock(ffile, (char *)&FloatVal, WCS_BLOCKTYPE_FLOAT + Size);
         // AF: FloatVal is only a single float, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32F(FloatVal, &FloatVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_HEIGHT, FloatVal, 0);
         break;
	 }
        case WCS_ECOTYPE_DENSITY:
         {
         BytesRead = ReadBlock(ffile, (char *)&FloatVal, WCS_BLOCKTYPE_FLOAT + Size);
         // AF: FloatVal is only a single float, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32F(FloatVal, &FloatVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_DENSITY, FloatVal, 0);
         break;
	 }
        case WCS_ECOTYPE_USEIMGCOL:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_USEIMGCOL, (float)0.0, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_PALCOL:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_PALCOL, (float)0.0, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_MAXIMGHT:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_MAXIMGHT, (float)0.0, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_FOLIAGE:
         {
         if ((*FolPtr = Foliage_Load(ffile, Size)))
          {
          Fol = *FolPtr;
          FolPtr = &Fol->Next;
          BytesRead = Size;
	  } /* if foliage read OK */
         break;
	 }
        default:
         {
         BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size);
         break;
	 } 
	} /* switch */
       TotalRead += BytesRead;
       } /* if block < 256 bytes */
      if (BytesRead != Size)
       break;
      } /* if size block read */
     else
      break;
     } /* if not done flag */
    } /* if tag block read */
   else
    break;
   } /* while */

  if (ReadSize > 0 && ReadSize != TotalRead)
   {
   FoliageGroup_Del(This, 1);
   This = NULL;
   } /* if error reading */
  } /* if new foliage group */

 return (This);

} /* FoliageGroup_Load() */

/***********************************************************************/

STATIC_FCN struct Foliage *Foliage_Load(FILE *ffile, ULONG ReadSize) // used locally only -> static, AF 24.7.2021
{
struct Foliage *This;
char TempStr[256];
short ShortVal;
float FloatVal;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

 if ((This = Foliage_New()))
  {
	/* read block descriptor tag from file */
  while (ItemTag != WCS_ECOTYPE_DONE)
   {
   if ((BytesRead = ReadBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG)))
    {
    ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32U(ItemTag, &ItemTag);)   // AF: 4.Jan.2023, Endian correction for i386-aros
    TotalRead += BytesRead;
    if (ItemTag != WCS_ECOTYPE_DONE)
     {
     if ((BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff)))
      {
      TotalRead += BytesRead;
      BytesRead = 0;
      switch (ItemTag & 0xff)
       {
       case WCS_BLOCKSIZE_CHAR:
        {
        Size = MV.Char[0];
        break;
	}
       case WCS_BLOCKSIZE_SHORT:
        {
        ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(MV.Short[0], &MV.Short[0]);)   // AF: 4.Jan.2023, Endian correction for i386-aros
        Size = MV.Short[0];
        break;
	}
       case WCS_BLOCKSIZE_LONG:
        {
        ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32S(MV.Long, &MV.Long);)   // AF: 4.Jan.2023, Endian correction for i386-aros
        Size = MV.Long;
        break;
	}
       } /* switch */

      if (Size < 256)
       {
       switch (ItemTag & 0xffff0000)
        {
        case WCS_ECOTYPE_NAME:
         {
         BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size);
         TempStr[255] = 0;
         Rootstock_SetName(&This->Root, TempStr);
         break;
	 }
        case WCS_ECOTYPE_COLORNAME:
         {
         BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size);
         TempStr[255] = 0;
         Rootstock_SetColorName(&This->Root, TempStr);
         break;
	 }
        case WCS_ECOTYPE_HEIGHT:
         {
         BytesRead = ReadBlock(ffile, (char *)&FloatVal, WCS_BLOCKTYPE_FLOAT + Size);
         // AF: FloatVal is only a single float, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32F(FloatVal, &FloatVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_HEIGHT, FloatVal, 0);
         break;
	 }
        case WCS_ECOTYPE_DENSITY:
         {
         BytesRead = ReadBlock(ffile, (char *)&FloatVal, WCS_BLOCKTYPE_FLOAT + Size);
         // AF: FloatVal is only a single float, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip32F(FloatVal, &FloatVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_DENSITY, FloatVal, 0);
         break;
	 }
        case WCS_ECOTYPE_USEIMGCOL:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_USEIMGCOL, (float)0.0, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_PALCOL:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_PALCOL, (float)0.0, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_MAXIMGHT:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Rootstock_SetValue(&This->Root, WCS_ECOTYPE_MAXIMGHT, (float)0.0, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_IMGHEIGHT:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Foliage_SetImgSize(This, WCS_ECOTYPE_IMGHEIGHT, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_IMGWIDTH:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Foliage_SetImgSize(This, WCS_ECOTYPE_IMGWIDTH, ShortVal);
         break;
	 }
        case WCS_ECOTYPE_COLORIMAGE:
         {
         BytesRead = ReadBlock(ffile, (char *)&ShortVal, WCS_BLOCKTYPE_SHORTINT + Size);
         // AF: ShortVal is only a single short, so Size must be one, no loop for endian correction
         ENDIAN_CHANGE_IF_NEEDED( SimpleEndianFlip16S(ShortVal, &ShortVal);)   // AF: 4.Jan.2023, Endian correction for i386-aros
         Foliage_SetImgSize(This, WCS_ECOTYPE_COLORIMAGE, ShortVal);
         break;
	 }
        default:
         {
         BytesRead = ReadBlock(ffile, (char *)TempStr, WCS_BLOCKTYPE_CHAR + Size);
         break;
	 } 
	} /* switch */
       TotalRead += BytesRead;
       } /* if block < 256 bytes */
      if (BytesRead != Size)
       break;
      } /* if size block read */
     else
      break;
     } /* if not done flag */
    } /* if tag block read */
   else
    break;
   } /* while */

  if (ReadSize > 0 && ReadSize != TotalRead)
   {
   Foliage_Del(This, 1);
   This = NULL;
   } /* if error reading */
  } /* if new foliage */

 return (This);

} /* FoliageLoad() */

/**********************************************************************/

struct Ecotype *Ecotype_Copy(struct Ecotype *This)
{
short error = 0;
struct Ecotype *NewEco = NULL;
struct FoliageGroup *FolGp, *NewGp, **NewGpAddr;
struct Foliage *Fol, *NewFol, **NewFolAddr;

 if (This)
  {
  if ((NewEco = Ecotype_New()))
   {
   memcpy(NewEco, This, sizeof (struct Ecotype));
   NewEco->Root.Name = NewEco->Root.ColorName = NULL;
   Rootstock_SetName(&NewEco->Root, This->Root.Name);
   Rootstock_SetColorName(&NewEco->Root, This->Root.ColorName);
   FolGp = This->FolGp;
   NewGpAddr = &NewEco->FolGp;
   while (FolGp)
    {
    if ((*NewGpAddr = FoliageGroup_New()))
     {
     NewGp = *NewGpAddr;
     memcpy(NewGp, FolGp, sizeof (struct FoliageGroup));
     NewGp->Root.Name = NewGp->Root.ColorName = NULL;
     Rootstock_SetName(&NewGp->Root, FolGp->Root.Name);
     Rootstock_SetColorName(&NewGp->Root, FolGp->Root.ColorName);
     Fol = FolGp->Fol;
     NewFolAddr = &NewGp->Fol;
     while (Fol)
      {
      if ((*NewFolAddr = Foliage_New()))
       {
       NewFol = *NewFolAddr;
       memcpy(NewFol, Fol, sizeof (struct Foliage));
       NewFol->Root.Name = NewFol->Root.ColorName = NULL;
       Rootstock_SetName(&NewFol->Root, Fol->Root.Name);
       Rootstock_SetColorName(&NewFol->Root, Fol->Root.ColorName);
       NewFolAddr = &NewFol->Next;
       } /* if new foliage */
      else
       {
       error = 1;
       break;
       } /* else */
      Fol = Fol->Next;
      } /* while */
     if (error)
      break;
     NewGpAddr = &NewGp->Next;
     } /* if new foliage group */
    else
     {
     error = 1;
     break;
     } /* else */
    FolGp = FolGp->Next;
    } /* while */
   } /* if */
  } /* if */

 if (error)
  {
  Ecotype_Del(NewEco);
  NewEco = NULL;
  } /* if */

 return (NewEco);

} /* Ecotype_Copy() */

/***********************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
long Rootstock_Save(struct Rootstock *This, FILE *ffile)
{

 return (0);

} /* Rootstock_Save() */
#endif
/***********************************************************************/

ULONG Ecotype_Save(struct Ecotype *This, FILE *ffile)
{
struct FoliageGroup *FolGp;
ULONG ItemTag, BytesWritten, TotalWritten = 0;
short ShortVal;
float FloatVal;

 if (This)
  {
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, strlen(This->Root.Name) + 1,
	WCS_BLOCKTYPE_CHAR, (char *)This->Root.Name)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_COLORNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, strlen(This->Root.ColorName) + 1,
	WCS_BLOCKTYPE_CHAR, (char *)This->Root.ColorName)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  FloatVal = This->Root.Height;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_HEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&FloatVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  FloatVal = This->Root.Density;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_DENSITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&FloatVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->Root.UseImgCol;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_USEIMGCOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->Root.PalCol;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_PALCOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->Root.MaxImgHeight;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_MAXIMGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  FolGp = This->FolGp;
  while (FolGp)
   {
   ItemTag = WCS_ECOTYPE_FOLIAGEGRP + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
   if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)))
    {
    TotalWritten += BytesWritten;

    ItemTag = 0;
    if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)))
     {
     TotalWritten += BytesWritten;

     if ((BytesWritten = FoliageGroup_Save(FolGp, ffile)))
      {
      TotalWritten += BytesWritten;
      fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
      if (WriteBlock(ffile, (char *)&BytesWritten,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
       {
       fseek(ffile, 0, SEEK_END);
       FolGp = FolGp->Next;
       } /* if wrote size of block */
      else
       goto WriteError;
      } /* if foliage group saved */
     else
      goto WriteError;
     } /* if size written */
    else
     goto WriteError;
    } /* if tag written */
   else
    goto WriteError;
   } /* while */

  ItemTag = WCS_ECOTYPE_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;
  } /* if This */

 return (TotalWritten);

WriteError:

 return (0L);

} /* Ecotype_Save() */

/***********************************************************************/

ULONG FoliageGroup_Save(struct FoliageGroup *This, FILE *ffile)
{
struct Foliage *Fol;
ULONG ItemTag, BytesWritten, TotalWritten = 0;
short ShortVal;
float FloatVal;

 if (This)
  {
  if ((BytesWritten = PrepWriteBlock(ffile, (ULONG)WCS_ECOTYPE_NAME, (ULONG)WCS_BLOCKSIZE_CHAR,
	(ULONG)WCS_BLOCKTYPE_CHAR, strlen(This->Root.Name) + 1,
	(ULONG)WCS_BLOCKTYPE_CHAR, (char *)This->Root.Name)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_COLORNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, strlen(This->Root.ColorName) + 1,
	WCS_BLOCKTYPE_CHAR, (char *)This->Root.ColorName)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  FloatVal = This->Root.Height;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_HEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&FloatVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  FloatVal = This->Root.Density;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_DENSITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&FloatVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->Root.UseImgCol;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_USEIMGCOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->Root.PalCol;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_PALCOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->Root.MaxImgHeight;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_MAXIMGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  Fol = This->Fol;
  while (Fol)
   {
   ItemTag = WCS_ECOTYPE_FOLIAGE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
   if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)))
    {
    TotalWritten += BytesWritten;

    ItemTag = 0;
    if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)))
     {
     TotalWritten += BytesWritten;

     if ((BytesWritten = Foliage_Save(Fol, ffile)))
      {
      TotalWritten += BytesWritten;
      fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
      if (WriteBlock(ffile, (char *)&BytesWritten,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
       {
       fseek(ffile, 0, SEEK_END);
       Fol = Fol->Next;
       } /* if wrote size of block */
      else
       goto WriteError;
      } /* if foliage group saved */
     else
      goto WriteError;
     } /* if size written */
    else
     goto WriteError;
    } /* if tag written */
   else
    goto WriteError;
   } /* while */

  ItemTag = WCS_ECOTYPE_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;
  } /* if This */

 return (TotalWritten);

WriteError:

 return (0L);

} /* FoliageGroup_Save() */

/***********************************************************************/

STATIC_FCN ULONG Foliage_Save(struct Foliage *This, FILE *ffile) // used locally only -> static, AF 24.7.2021
{
ULONG ItemTag, BytesWritten, TotalWritten = 0;
short ShortVal;
float FloatVal;

 if (This)
  {
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, strlen(This->Root.Name) + 1,
	WCS_BLOCKTYPE_CHAR, (char *)This->Root.Name)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_COLORNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, strlen(This->Root.ColorName) + 1,
	WCS_BLOCKTYPE_CHAR, (char *)This->Root.ColorName)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  FloatVal = This->Root.Height;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_HEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&FloatVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  FloatVal = This->Root.Density;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_DENSITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&FloatVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->Root.UseImgCol;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_USEIMGCOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->Root.PalCol;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_PALCOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->Root.MaxImgHeight;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_MAXIMGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->ImgWidth;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_IMGWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->ImgHeight;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_IMGHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ShortVal = This->ColorImage;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_COLORIMAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShortVal)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;

  ItemTag = WCS_ECOTYPE_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == 0)
   goto WriteError;
  TotalWritten += BytesWritten;
  } /* if This */
   
 return (TotalWritten);

WriteError:

 return (0L);

} /* Foliage_Save() */

/***********************************************************************/

#else /* C_PLUS_PLUS */

/***********************************************************************/
/* in Foliage.h

class Rootstock
	{
	protected:
		~Rootstock();

		void SetValue(ULONG Flags, float FlVal, short ShtVal);
		char *SetName(char *NameStr);
		char *SetColorName(char *NameStr);

		char *Name, ColorName;
		float Height, Density;
		short NameSize, ColorNameSize, UseImgCol, PalCol, MaxImgHeight;
	};

class Ecotype : private Rootstock
	{
	public:
		~Ecotype();
	
		long Load(FILE *ffile);
		long Save(FILE *ffile);

		class FoliageGroup *FolGp;
	};

class FoliageGroup : private Rootstock
	{
	public:
		~FoliageGroup();

		long Load(FILE *ffile);
		long Save(FILE *ffile);

		class FoligeGroup *Next;
		class Foliage *Fol;
	};

class Foliage : private Rootstock
	{
	public:
		~Foliage();

		long Load(FILE *ffile);
		long Save(FILE *ffile);
		void SetImgSize(ULONG Flags, short ShtVal);

		class Foliage *Next;
		short ImgWidth, ImgHeight;
	};
*/
/************************************************************************/

Rootstock::~Rootstock()
	{

	if (Name)
		Appmem_Free(Name, NameSize);
	if (ColorName)
		Appmem_Free(ColorName, ColorNameSize);

	} /* ~Rootstock() */

/************************************************************************/

Ecotype::~Ecotype()
	{

	if (FolGp)
		Delete FolGp;

	} /* ~Ecotype() */

/************************************************************************/

FoliageGroup::~FoliageGroup()
	{

	if (Next)
		Delete Next;
	if (Fol)
		Delete Fol;

	} /* ~FoliageGroup() */

/************************************************************************/

Foliage::~Foliage()
	{

	if (Next)
		Delete Next;

	} /* ~Foliage() */

/************************************************************************/

void Rootstock::SetValue(ULONG Flags, float FlVal, short ShtVal)
	{

	switch (Flags)
		{
		case WCS_ECOTYPE_HEIGHT:
			{
			Height = FlVal;
			break;
			}
		case WCS_ECOTYPE_DENSITY:
			{
			Density = FlVal;
			break;
			}
		case WCS_ECOTYPE_USEIMGCOL:
			{
			UseImgCol = ShtVal;
			break;
			}
		case WCS_ECOTYPE_PALCOL:
			{
			PalCol = ShtVal;
			break;
			}
		case WCS_ECOTYPE_MAXIMGHT:
			{
			MaxImgHeight = ShtVal;
			break;
			}
		} /* switch */

	} /* Rootstock::SetValue() */

/************************************************************************/

char *Rootstock::SetName(char *NameStr)
	{
	short Len;

	if (Name)
		AppMem_Free(Name, NameSize);
	Len = strlen(NameStr) + 1;
	if (Name = (char *)AppMem_Alloc(Len, 0))
		{
		NameSize = Len;
		strcpy(Name, NameStr);
		} /* if */
	else
		NameSize = 0;

	return (Name);

	} /* Rootstock::SetName() */

/************************************************************************/

char *Rootstock::SetColorName(char *NameStr)
	{
	short Len;

	if (ColorName)
		AppMem_Free(ColorName, ColorNameSize);
	Len = strlen(NameStr) + 1;
	if (ColorName = (char *)AppMem_Alloc(Len, 0))
		{
		ColorNameSize = Len;
		strcpy(ColorName, NameStr);
		} /* if */
	else
		ColorNameSize = 0;

	return (ColorName);

	} /* Rootstock::SetColorName() */

/**********************************************************************/

void Foliage::SetImgSize(ULONG Flags, short ShtVal)
	{

	switch (Flags)
		{
		case WCS_ECOTYPE_IMGHEIGHT:
			{
			ImgHeight = ShtVal;
			break;
			}
		case WCS_ECOTYPE_IMGWIDTH:
			{
			ImgWidth = ShtVal;
			break;
			}
		} /* switch */

	} /* Foliage::SetImgSize() */

#endif /* C_PLUS_PLUS */
