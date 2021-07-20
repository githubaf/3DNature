/* Foliage.h
** prototypes for foliage stuff
** For WCS V2 by GRH 12/95.
** Copyright 1995 by Questar Productions
*/

#ifndef FOLIAGE_H
#define FOLIAGE_H

#define WCS_ECOTYPE_DONE	0xffff0000

/* strings */
#define WCS_ECOTYPE_NAME	0x00010000
#define WCS_ECOTYPE_COLORNAME	0x00020000
/* floats */
#define WCS_ECOTYPE_HEIGHT	0x00080000
#define WCS_ECOTYPE_DENSITY	0x00090000
/* shorts */
#define WCS_ECOTYPE_IMGHEIGHT	0x00100000
#define WCS_ECOTYPE_IMGWIDTH	0x00200000
#define WCS_ECOTYPE_USEIMGCOL	0x00300000
#define WCS_ECOTYPE_PALCOL	0x00400000
#define WCS_ECOTYPE_MAXIMGHT	0x00500000
#define WCS_ECOTYPE_COLORIMAGE	0x00600000
/* groups */
#define WCS_ECOTYPE_ECOSYS	0x01000000
#define WCS_ECOTYPE_ECOTYPE	0x02000000
#define WCS_ECOTYPE_FOLIAGEGRP	0x03000000
#define WCS_ECOTYPE_FOLIAGE	0x04000000
/* variable sizes */
#define WCS_BLOCKSIZE_CHAR	0x01
#define WCS_BLOCKSIZE_SHORT	0x02
#define WCS_BLOCKSIZE_LONG	0x04
#define WCS_BLOCKSIZE_DOUBLE	0x08
/* variable types */
#define WCS_BLOCKTYPE_CHAR	0x0100
#define WCS_BLOCKTYPE_SHORTINT	0x0200
#define WCS_BLOCKTYPE_LONGINT	0x0300
#define WCS_BLOCKTYPE_FLOAT	0x0400
#define WCS_BLOCKTYPE_DOUBLE	0x0500


#ifndef C_PLUS_PLUS

struct Rootstock {
	char  *Name, *ColorName;
	float  Height, Density;
	short  UseImgCol, PalCol, MaxImgHeight, NameSize, ColorNameSize;
}; /* 26 bytes */

struct Ecotype {
	struct FoliageGroup *FolGp;
	struct Rootstock Root;
}; /* 30 bytes */

struct FoliageGroup {
	struct FoliageGroup *Next;
	struct Foliage *Fol;
	struct Rootstock Root;
}; /* 34 bytes */

struct Foliage {
	struct Foliage *Next;
	char  *NameAddr;
	short  ImgWidth, ImgHeight, ColorImage;
	struct Rootstock Root;
}; /* 38 bytes */


struct Rootstock *Rootstock_New(void);
struct Ecotype *Ecotype_New(void);
struct FoliageGroup *FoliageGroup_New(void);
struct Foliage *Foliage_New(void);
void Rootstock_Del(struct Rootstock *This);
void Ecotype_Del(struct Ecotype *This);
void FoliageGroup_Del(struct FoliageGroup *This, short DelAll);
void Foliage_Del(struct Foliage *This, short DelAll);
void Rootstock_SetValue(struct Rootstock *This,
	ULONG Flags, float FlVal, short ShtVal);
char *Rootstock_SetName(struct Rootstock *This, char *NameStr);
char *Rootstock_SetColorName(struct Rootstock *This, char *NameStr);
void Foliage_SetImgSize(struct Foliage *This, ULONG Flags, short ShtVal);
float Rootstock_GetFloatValue(struct Rootstock *This, ULONG Flags);
short Rootstock_GetShortValue(struct Rootstock *This, ULONG Flags);
char *Rootstock_GetName(struct Rootstock *This);
char **Rootstock_GetNameAddr(struct Rootstock *This);
char **Foliage_GetImageNameAddr(struct Foliage *This);
char *Rootstock_GetColorName(struct Rootstock *This);
short Foliage_GetImgSize(struct Foliage *This, ULONG Flags);
void Ecotype_SetDefaults(struct Ecotype *This, struct Ecosystem *Eco);
void FoliageGroup_SetDefaults(struct FoliageGroup *This,
	struct Ecotype *Proto);
void Foliage_SetDefaults(struct Foliage *This,
	struct FoliageGroup *Proto);
void DisposeEcotypes(void);
struct Ecotype *Ecotype_Load(FILE *ffile, ULONG ReadSize);
struct FoliageGroup *FoliageGroup_Load(FILE *ffile, ULONG ReadSize);
struct Foliage *Foliage_Load(FILE *ffile, ULONG ReadSize);
struct Ecotype *Ecotype_Copy(struct Ecotype *This);
ULONG Ecotype_Save(struct Ecotype *This, FILE *ffile);
ULONG FoliageGroup_Save(struct FoliageGroup *This, FILE *ffile);
ULONG Foliage_Save(struct Foliage *This, FILE *ffile);
//ULONG ReadBlock(FILE *ffile, char *Block, ULONG Flags);  // used locally only -> static, AF 19.7.2021
//ULONG WriteBlock(FILE *ffile, char *Block, ULONG Flags); // used locally only -> static, AF 19.7.2021
/*ULONG PrepWriteBlock(FILE *ffile, ULONG ItemTag, ULONG SizeSize,
	ULONG SizeType, ULONG FieldSize, ULONG FieldType, char *FieldAddr);*/ // used locally only -> static, AF 19.7.2021

#else /* C_PLUS_PLUS */


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
		short ImgWidth, ImgHeight, ColorImage;
	};

#endif /* C_PLUS_PLUS */

#endif /* FOLIAGE_H */
