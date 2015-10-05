/* MarkovTable.c
** Builds markov trees from an input vocabulary, then writes out the
** trees in table form.
**
** The actual workings of the algorithm are close to pure structural voodoo.
** But dang is it slick.
**
** Warning: I'm not proud of the abyssmal memory managment this code
** does -- it doesn't do ANY. See Warning2 below.
**
** Warning2: This routine can consume big chunks of memory while crunching,
** and probably likes to have a godawful amount of stack. Make no
** mistake, this is not an application, this is a hack development tool.
**
** Copyright 1994 by Questar Productions
*/

/* Lots of things you can define to alter the program. Most are only
** for debugging purposes. */

/* #define MOREWORDS */
/* #define SPEW_LOTS */
/* #define PROGRESS_DEBUG */
/* #define WORD_VERBOSE */
#define NUMBERIZE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <exec/types.h> /* Just to get TRUE and FALSE...? */

int main(int Count, char *Vector[]);
int MarkOff(char *NewWord);
void NumberEntries(struct AlphaNode *Torch);
void OutputTreeSeg(struct AlphaNode *Torch, char *Base, FILE *Out);
int OutputTableSeg(struct AlphaNode *Torch, FILE *Out);
char *GetWord(char *Buf, int Size, FILE *In);
int FetchWord(char *Buf, int Size, FILE *In);
int Zgetc(FILE *In);
int RecogWord(char *Word, struct AlphaNode *Root);
int LeadingSpace(char *Data);
int ProcessSentance(char *DispatchFunc);
char *DeRefWord(int WordNum);
int DeRefTwo(struct AlphaNode *Walk, int SearchWord, char *Temp, char *BringForth);
void MAnGleCaSe(char *String);
char *stralloc(char *CopyStr);
int WordTableAdd(char *FuncName, struct WordEntry *Base, int Index);
int StageTwo(char *FuncName, struct WordEntry *This, int Index);
struct WordEntry *WordEntry_New(void);
void WordEntry_Del(struct WordEntry *This);
void WordEntry_NumberTable(struct WordEntry *Base);
void WordEntry_WriteTable(FILE *Out, struct WordEntry *Base);

/* Bit breakdown of table entry longword */
/* 31   30      29   28 27 26 25 24  23 22 21 20 19 18 ... 5 4 3 2 1 0
** |\   |       |     \     |    /    \   \    \  |  |  |  /   /    /
** | \  |       |      \    |   /      \   \    \ |  |  | /   /    /
** Term Last   Un-    5-bit symbol         24-bit index to entry
** Flag Entry  used       entry           corresponding to symbol
*/

#define VE_TERM_FLAG				(1 << 31)
#define VE_TERM_FLAG_EIGHT		(1 << 7)
#define VE_LAST_ENTRY			(1 << 30)
#define VE_LAST_ENTRY_EIGHT	(1 << 6)

#define RECOG_OPTIONAL			(1 << 31) /* word is optional */
#define RECOG_VALID				(1 << 30) /* even word 0 has to have a bit set */
#define RECOG_WILD				(1 << 29) /* Means word requires arg */
#define RECOG_ENDLEVEL			(1 << 28) /* End of this level of table */

/* Note: Don't make sentances that both do and do not posess a wildcard after
** the same word. A wildcard is really an argument to the preceeding word,
** and is therefore required. So don't do:

FOO BAR * BAZ
and
FOO BAR BLUE

** If you want BAR to take an arg, make sure it always does -- the code will
** not necessarily do it for you, and results may be unpredictable. Also, don't
** do stupid things like optional wildcards. It doesn't work. Double wilds
** don't make sense either. Entirly optional sentances: Dumb. This is a tool,
** not a mind-reader. */

/* Note: Current implementation only supports a single wild arg. This could
** be changed in the future, but is simpler now. */

/* Data structures for building the letter-sequence word-recognition table */

struct AlphaEntry
	{
	struct AlphaNode *AlphaChild;
	}; /* AlphaEntry */

struct AlphaNode
	{
	unsigned char Term;
#ifndef MOREWORDS
	unsigned short int EntryNum;
#endif /* MOREWORDS */
	struct AlphaEntry AlphaIndex[26];
	}; /* AlphaNode */

struct MarkovTable
	{
	int TotalNodes, TotalWords, UniqueWords, Duplicates, Rejects, Blanks, EntryCount;
	struct AlphaNode MarkovRoot;
	}; /* MarkovTable */

/* Ok, here's the data structure for the Word-sequence table */

struct WordEntry
	{
	struct WordEntry *Next, *Child;
	unsigned long int WordToken, EntryNum;
	char *FuncName;
	}; /* WordEntry */

struct MarkovTable MainTable;
struct WordEntry CommandRoot;
int FirstWarning, CurLine, CurChar, WordPlace, HoldPlace, WordEntries, MyIndex = 0;
char LineBuf[250], ErrorLine[250], ScratchStr[80];
unsigned long int HoldForward[12], Sentance[12];

int main(int Count, char *Vector[])
{
FILE *InStream, *OutStream;
int KeepCrankin, Status, CopyChar, CopyIdx, Recognise, TableError = 0;
char WordBuf[80]; /* 80 chars is way overkill. */
char OutName[255];
char Drive[32], Path[255], Name[32], Ext[32];


CurLine = FirstWarning = 1;
WordEntries = CurChar = 0;
memset(&MainTable, 0, sizeof(struct MarkovTable));
memset(&HoldForward[0], 0, 12 * sizeof(unsigned long int));
memset(&Sentance[0], 0, 12 * sizeof(unsigned long int));
memset(&CommandRoot, 0, sizeof(struct WordEntry));

printf("MarkovTable v1.0. Copyright 1994 by Questar productions.\n");

if((Count == 2) && (Vector[1][0] != '?'))
	{
	if(InStream = fopen(Vector[1], "r"))
		{
		for(KeepCrankin = 1; KeepCrankin;)
			{
			if(GetWord(WordBuf, 75, InStream))
				{
/*				printf("WORD: \"%s\"\n", WordBuf); */
				if(MarkOff(WordBuf) == 0)
					{
					KeepCrankin = FALSE;
					} /* if */
				MainTable.TotalWords++;
				} /* if */
			else
				{
				KeepCrankin = FALSE;
				} /* else */
			} /* for */
		
		/* Ok, _now_ what do we do...? */
		
		printf("\nDEBUG: %d AlphaEntries linked. ", MainTable.TotalNodes);
		printf("%d total words added.\n", MainTable.UniqueWords);
		printf("DEBUG: This is %f Entries per Word.\n\n",
		 (((float)MainTable.TotalNodes) / ((float)MainTable.UniqueWords)));
		printf("DEBUG: %d input lines processed.\n", MainTable.TotalWords);
		printf("DEBUG: %d blank lines.\n", MainTable.Blanks);
		printf("DEBUG: %d duplicates.\n", MainTable.Duplicates);
		printf("DEBUG: %d rejected words.\n", MainTable.Rejects);
		printf("DEBUG: %d unique words.\n", MainTable.UniqueWords);
		printf("DEBUG: %d = %d + %d + %d + %d.\n\n", MainTable.TotalWords,
		 MainTable.Blanks, MainTable.Duplicates, MainTable.Rejects,
		 MainTable.UniqueWords);

#ifndef MOREWORDS
		
		strsfn(Vector[1], Drive, Path, Name, Ext);
		strmfn(OutName, Drive, Path, "VocabTable", "h");
/*		printf("DEBUG: OutName = '%s'.\n", OutName); */
		if (OutStream = fopen(OutName, "w"))
			{
			fprintf(OutStream, "/* VocabTable.h\n**\n** #defines for Markov search table.\n");
			fprintf(OutStream, "** Machine generated code, do NOT hand-alter.\n");
			fprintf(OutStream, "** Created from file \"%s\".\n*/\n\n", Vector[1]);
			fprintf(OutStream, "extern unsigned long int MarkovTable[];\n\n");
			NumberEntries(&MainTable.MarkovRoot);
			OutputTreeSeg(&MainTable.MarkovRoot, "", OutStream);
			fprintf(OutStream, "\n\n/* EOF */\n");
			fclose(OutStream);
			if(MainTable.TotalNodes < 65536)
				{
				strmfn(OutName, Drive, Path, "VocabTable", "c");
				if(OutStream = fopen(OutName, "w"))
					{
					fprintf(OutStream, "/* VocabTable.c\n**\n** C-Source for binary Markov search table.\n");
					fprintf(OutStream, "** Machine generated code, do NOT hand-alter.\n");
					fprintf(OutStream, "** Created from file \"%s\".\n*/\n\n", Vector[1]);
					fprintf(OutStream, "\n\nunsigned long int MarkovTable[] =\n{\n");
					OutputTableSeg(&MainTable.MarkovRoot, OutStream);
					fprintf(OutStream, "0x00000000 /* End-of-Table dummy marker */\n};\n\n\n/* EOF */\n\n");
					fclose(OutStream);
					} /* if */
				rewind(InStream); /* Play it again, Sam */
				WordPlace = HoldPlace = 0;
				while(Status = FetchWord(WordBuf, 75, InStream))
					{
					switch(Status)
						{
						/* case 0: Won't happen */
						case 1:
							{
							Recognise = RecogWord(WordBuf, &MainTable.MarkovRoot);
							if(Recognise != -1)
								{
								if(WordPlace > 8)
									{
									printf("ERROR: Sentance overflow. truncating.\n");
									} /* if */
								else
									{
									Sentance[WordPlace++] = (Recognise | RECOG_VALID);
									} /* else */
#ifdef PROGRESS_DEBUG
								printf("%s(%d) ", WordBuf, Recognise);
#endif /* PROGRESS_DEBUG */
								} /* if */
							break;
							} /* all clear */
						case 2:
							{
							Recognise = RecogWord(WordBuf, &MainTable.MarkovRoot);
							if(Recognise != -1)
								{
								if(WordPlace > 8)
									{
									printf("ERROR: Sentance overflow. truncating.\n");
									} /* if */
								else
									{
									Sentance[WordPlace++] = (Recognise | RECOG_VALID | RECOG_OPTIONAL);
									} /* else */
#ifdef PROGRESS_DEBUG
								printf("[%s(%d)] ", WordBuf, Recognise);
#endif /* PROGRESS_DEBUG */
								} /* if */
							break;
							} /* optional */
						case 3:
							{
							if(WordPlace > 8)
								{
								printf("ERROR: Sentance overflow. truncating.\n");
								} /* if */
							else
								{
								if(WordPlace > 0)
									{
									Sentance[WordPlace - 1] |= RECOG_WILD;
									} /* if */
								else
									{
									printf("ERROR: Sentances can't begin with wildcards.\n");
									} /* else */
								} /* else */
#ifdef PROGRESS_DEBUG
							printf("* ");
#endif /* PROGRESS_DEBUG */
							break;
							} /* wildcard */
						case 4:
							{
#ifdef PROGRESS_DEBUG
							printf("...\n");
#endif /* PROGRESS_DEBUG */
							for(CopyIdx = 0; CopyIdx < 10; CopyIdx++)
								{
								HoldForward[CopyIdx] = Sentance[CopyIdx];
								Sentance[CopyIdx] = NULL;
								HoldPlace = WordPlace;
								WordPlace = 0;
								} /* for */
							while((CopyChar = Zgetc(InStream)) != '\n'); /* chop to EOL */
							break;
							} /* extend */
						case 5:
							{
#ifdef PROGRESS_DEBUG
							printf("= ");
#endif /* PROGRESS_DEBUG */
							memset(WordBuf, 0, 79);
							CopyIdx = 0;
							while(CopyChar = Zgetc(InStream))
								{
								if(CopyChar == '\n')
									{
#ifdef PROGRESS_DEBUG
									printf("%s\n::::", stpblk(WordBuf));
#endif /* PROGRESS_DEBUG */
									TableError += ProcessSentance(stpblk(WordBuf));
									WordPlace = 0;
									break;
									} /* if */
								else
									{
									if(CopyIdx < 75)
										{
										WordBuf[CopyIdx++] = CopyChar;
										} /* if */
									else
										{
										printf("\nERROR: Line %d, Assign overrun.\n", CurLine);
										while((CopyChar = Zgetc(InStream)) != '\n');
										break;
										} /* else */
									} /* else */
								} /* while */
							WordPlace = 0;
							break;
							} /* assign */
						case 50:
							{
							/* do nothing at this point */
							break;
							} /* bad parse */
						default:
							{
							printf("\n\nERROR: Unexpected return from FetchWord().\n");
							break;
							} /*  */						
						} /* switch */
					if((Status == 4) || (Status == 5))
						{ /* done processing a line, should process leading blanks */
						LineBuf[0] = 'Z'; /* Ignore this, I will */
						if(fgets(&LineBuf[1], 200, InStream))
							{
							CurChar = 1;
							ErrorLine[0] = '\"';
							ErrorLine[1] = NULL;
							strcat(ErrorLine, &LineBuf[1]);
							ErrorLine[strlen(ErrorLine) - 1] = '\"';
							CurLine++;
							
							for(CopyIdx = 0; CopyIdx < 10; CopyIdx++)
								{
								Sentance[CopyIdx] = NULL;
								} /* for */
							
							CopyIdx = LeadingSpace(&LineBuf[1]);
							for(Recognise = 0; Recognise < CopyIdx; Recognise++)
								{
								Sentance[Recognise] = HoldForward[Recognise];
								} /* for */
							WordPlace = CopyIdx;
							CurChar += CopyIdx;
							} /* if */
						/* else it'll fail all in good time */
						} /* if */
					} /* while */
#ifdef PROGRESS_DEBUG
				printf("\n/* EOF */\n");
#endif /* PROGRESS_DEBUG */
				
				if(TableError)
					{
					printf("ERROR: Problems were encountered while building the Sentance\n");
					printf("       recognition table. Discontinuing further processing.\n");
					} /* if */
				else
					{ /* Lock on and Rock on! */
					printf("DEBUG: %d entries were added to the Sentance Table.\n",
					 WordEntries);
#ifdef NUMBERIZE
					WordEntries = 0;
					WordEntry_NumberTable(&CommandRoot);
					printf("DEBUG: %d entries in Sentance Table.\n", WordEntries);
					strmfn(OutName, Drive, Path, "GrammarTable", "c");
					if(OutStream = fopen(OutName, "w"))
						{
						fprintf(OutStream, "/* GrammarTable.c\n**\n** C-Source for binary Markov word-sentance table.\n");
						fprintf(OutStream, "** Machine generated code, do NOT hand-alter.\n");
						fprintf(OutStream, "** Created from file \"%s\".\n*/\n\n", Vector[1]);
						fprintf(OutStream, "#define EXT extern\n");
						fprintf(OutStream, "#include \"GrammarTable.h\"\n\n\n");
						fprintf(OutStream, "struct MWS_Entry SentLookUp[] =\n{\n");
						WordEntries = 0;
						WordEntry_WriteTable(OutStream, &CommandRoot);
						fprintf(OutStream, "{0, 0, 0} /* End of table dummy marker. */\n};\n\n/* EOF */\n");
						fclose(OutStream);
						} /* if */
					else
						{
						printf("ERROR: Could not open GrammarTable.c file for writing.\n");
						} /* else */
#endif /* NUMBERIZE */
					} /* else */
				
				} /* if */
			} /* if */
		else
			{
			printf("\nERROR: Could not open output file.\n");
			} /* else */
		
#endif /* MOREWORDS */

		fclose(InStream);
		} /* if */
	else
		{
		printf("Error opening input file.\n");
		} /* else */
	} /* if */
else
	{
	printf("Usage:\n%s <inputfilename>\n", Vector[0]);
	printf("If successful, the program will write a files named \"VocabTable.h\",\n");
	printf("\"VocabTable.c\" and \"GrammarTable.c\" in the same directory as\n");
	printf("the input file.\n");
	} /* else */

return(20);
} /* main() */

int MarkOff(char *NewWord)
/* We make funny word pun, yes? */
{
int LetterWalk;
unsigned char LetterIndex;
struct AlphaNode *Traverse;

if(NewWord[0] == 0)
	{
	MainTable.Blanks++;
	return(1);
	} /* if */

Traverse = &MainTable.MarkovRoot;
strlwr(NewWord);

for(LetterWalk = 0; LetterWalk < strlen(NewWord); LetterWalk++)
	{
	if(isalpha(NewWord[LetterWalk]) == FALSE)
		{
		if(FirstWarning)
			{
			printf("\nERROR: Vocabulary words can only be composed of alphabetic characters.\n");
			FirstWarning = 0;
			} /* if */
		printf("LINE: %d. The word \"%s\" is unacceptable.\n", MainTable.TotalWords, NewWord);
		MainTable.Rejects++;
		return(1); /* We'll keep going. */
		} /* if */
	} /* if */

for(LetterWalk = 0; LetterWalk < strlen(NewWord); LetterWalk++)
	{
	LetterIndex = NewWord[LetterWalk] - 'a';
	if((LetterIndex < 0) || (LetterIndex > 25))
		{
		printf("\nERROR: Major unexpected condition.\n");
		return(0); /* Bail hard */
		} /* if */
	if(Traverse->AlphaIndex[LetterIndex].AlphaChild == NULL)
		{ /* end of the chain, we build on from here */
		if((Traverse->AlphaIndex[LetterIndex].AlphaChild = calloc(1, sizeof(struct AlphaNode))) == NULL)
			{
			printf("Panic allocating memory. Bailing.\n");
			return(0);
			} /* if */
		MainTable.TotalNodes++;
		} /* if */
	Traverse = Traverse->AlphaIndex[LetterIndex].AlphaChild;
	} /* for */	

if(LetterWalk)
	{
	if(Traverse->Term == FALSE)
		{
		Traverse->Term = TRUE;
		MainTable.UniqueWords++;
		} /* if */
	else
		{
		MainTable.Duplicates++;
		/* printf("Line %d, word '%s' already in vocabulary.\n", MainTable.TotalWords, NewWord); */
		} /* else */
	} /* if */

return(1); /* Looks good */
} /* MarkOff() */

#ifndef MOREWORDS
void NumberEntries(struct AlphaNode *Torch)
{
int ABCloop, FlaggedOne = 0;

#ifdef BLORGZED
/* Take into account the special 'first case'... */
if(Torch->EntryNum == 0)
	{
	for(ABCloop = 0; ABCloop < 26; ABCloop++)
		{
		if(Torch->AlphaIndex[ABCloop].AlphaChild)
			MainTable.EntryCount++;
		} /* for */
	} /* if */
#else
/* Or maybe it's every case! */
for(ABCloop = 0; ABCloop < 26; ABCloop++)
	{
	if(Torch->AlphaIndex[ABCloop].AlphaChild)
		{
		MainTable.EntryCount++;
		FlaggedOne = 1;
		} /* if */
	} /* for */

if(FlaggedOne == 0)
	MainTable.EntryCount++;

#endif


#ifdef BLORGZED
/* Assign numbers, width first */
for(ABCloop = 0; ABCloop < 26; ABCloop++)
	{
	if(Torch->AlphaIndex[ABCloop].AlphaChild)
		Torch->AlphaIndex[ABCloop].AlphaChild->EntryNum = MainTable.EntryCount++;
	} /* for */

/* Recurse, by depth */
for(ABCloop = 0; ABCloop < 26; ABCloop++)
	{
	if(Torch->AlphaIndex[ABCloop].AlphaChild)
		NumberEntries(Torch->AlphaIndex[ABCloop].AlphaChild);
	} /* for */
#else
/* Assign numbers, width first */
for(ABCloop = 0; ABCloop < 26; ABCloop++)
	{
	if(Torch->AlphaIndex[ABCloop].AlphaChild)
		{
	 	Torch->AlphaIndex[ABCloop].AlphaChild->EntryNum = MainTable.EntryCount;
		NumberEntries(Torch->AlphaIndex[ABCloop].AlphaChild);
		} /* if */
	} /* for */

#endif

} /* NumberEntries() */
#endif /* MOREWORDS */


void OutputTreeSeg(struct AlphaNode *Torch, char *Base, FILE *Out)
{
int ABCloop, Insert;
char Forward[80];

strcpy(Forward, Base);
strcat(Forward, " "); /* leaves room to insert letters */
Insert = strlen(Forward) - 1; /* The place to insert */

if(Torch->Term)
	{
	fprintf(Out, "#define VE_% -20s % 4d", Base, Torch->EntryNum);
	fprintf(Out, " /* %s */\n", DeRefWord(Torch->EntryNum));
	} /* if */

for(ABCloop = 0; ABCloop < 26; ABCloop++)
	{
	Forward[Insert] = (ABCloop + 'A');
	if(Torch->AlphaIndex[ABCloop].AlphaChild)
		OutputTreeSeg(Torch->AlphaIndex[ABCloop].AlphaChild, Forward, Out);
	} /* for */

} /* OutputTreeSeg() */

int OutputTableSeg(struct AlphaNode *Torch, FILE *Out)
{
int ABCloop, TopLetter, HasKids = 0;
unsigned long int IndexNum, FinalFour;
unsigned char SymbolNum;

/* First determine which entry is the last at this level */
for(TopLetter = ABCloop = 0; ABCloop < 26; ABCloop++)
	{
	if(Torch->AlphaIndex[ABCloop].AlphaChild)
		{
		TopLetter = ABCloop;
		HasKids = 1;
		} /* if */
	} /* for */

/* #ifdef WRITE_LEAF_NODES */
if(HasKids == 0)
	{ /* We are a leaf node */
	SymbolNum = 0xff; /* Christmas tree entry */
	IndexNum = Torch->EntryNum;
	FinalFour  = (SymbolNum << 24);
	FinalFour |= (IndexNum & 0x00FFFFFF); /* Lowest 24 bits, please */
	MyIndex++;
	if(fprintf(Out, "0x%08lX, ", FinalFour) < 0)
		{
		return(0);
		} /* if */
	else
		{
		if(fprintf(Out, "/* % 3d: No Children. */\n\n", Torch->EntryNum) < 0)
			return(0);
		} /* else */
	} /* if */
/* #endif */ /* WRITE_LEAF_NODES */

/* Next write out entries for this level */
for(ABCloop = 0; ABCloop < TopLetter + 1; ABCloop++)
	{
	SymbolNum = ABCloop;
	FinalFour = IndexNum = 0;
	if(ABCloop == TopLetter)
		SymbolNum |= VE_LAST_ENTRY_EIGHT;
	if(Torch->AlphaIndex[ABCloop].AlphaChild)
		{
		IndexNum = Torch->AlphaIndex[ABCloop].AlphaChild->EntryNum;
		if(Torch->AlphaIndex[ABCloop].AlphaChild->Term)
			{
			SymbolNum |= VE_TERM_FLAG_EIGHT;
			} /* if */
		FinalFour  = (SymbolNum << 24);
		FinalFour |= (IndexNum & 0x00FFFFFF); /* Lowest 24 bits, please */
#ifdef SPEW_LOTS
		printf("%d: '%c' -> %d", Torch->EntryNum, 'A' + ABCloop, IndexNum);
		if(SymbolNum & VE_TERM_FLAG_EIGHT)
			printf("!");
		if(SymbolNum & VE_LAST_ENTRY_EIGHT)
			printf("L");
		printf("\n");
#endif /* SPEW_LOTS */
/*		if(fwrite((void *)&FinalFour, 1, 4, Out) != 4)
			return(0); */
		if(fprintf(Out, "0x%08lX, ", FinalFour) < 0)
			return(0);
		if(fprintf(Out, "/* % 3d: '%c' = %- 4d (Term = %d) */\n", MyIndex++,
		 (ABCloop + 'A'), (IndexNum & 0x00FFFFFF),
		 ((SymbolNum & VE_TERM_FLAG_EIGHT) != 0 )) < 0)
			return(0);
		if(SymbolNum & VE_LAST_ENTRY_EIGHT)
			if(fprintf(Out, "\n") < 0)
				return(0);
		} /* if */
	} /* for */

/* Now recurse */
for(ABCloop = 0; ABCloop < 26; ABCloop++)
	{
	if(Torch->AlphaIndex[ABCloop].AlphaChild)
		{
		if(OutputTableSeg(Torch->AlphaIndex[ABCloop].AlphaChild, Out) == 0)
			return(0);
		} /* if */
	} /* for */

return(1); /* Hunky-Dory */

} /* OutputTableSeg() */

char *GetWord(char *Buf, int Size, FILE *In)
{
int Count;
unsigned char Current;

for(Count = 0; Count < (Size - 1);)
	{
	Current = fgetc(In);
	if(Current == EOF)
		return(NULL);
	if(isalpha(Current))
		{
		Buf[Count++] = Current;
		} /* if */
	else
		{
		if(Current == '=') /* skip to EOL */
			{
			while(Current != '\n')
				{
				Current = fgetc(In);
				if((Current == EOF) && (Count == 0))
					return(NULL); /* we're finished */
				} /* while */
			} /* if */
		if(Count)
			{
			Buf[Count] = NULL;
			return(Buf);
			}
		} /* else */
	} /* for */

Buf[Size - 1] = NULL;
return(Buf);

} /* GetWord() */


int FetchWord(char *Buf, int Size, FILE *In)
{
int Count, Advance, Optional = 0;
int Current;
/* returncodes:
** 0 - complete failure, stop processing file
** 1 - normal exit, process word normally
** 2 - normal exit, process word as optional
** 3 - normal exit, process wildcard (*)
** 4 - normal exit, process as extend
** 5 - normal exit, process as assign
** 50 - bad word, but continue processing (Syntax will most likely be wrong.)
*/

memset(Buf, 0, Size);
for(Count = 0; Count < (Size - 1);)
	{
	Advance = 0;
	Current = Zgetc(In);
	if(Current == EOF)
		return(0);
	if(isalpha(Current))
		{
		Buf[Count++] = Current; /* enter WORD state */
		} /* if */
	else
		{
		if (Optional)
			{ /* in OPTIONAL state */
			switch(Current)
				{
				case ']':
					{
					Buf[Count] = NULL;
					return(2); /* EXIT as OPTIONAL */
					/* break; */ /* more anti-compiler-complain measures */
					} /* close-bracket */
				case '[':
					{
					printf("\nERROR: Line %d, No nested brackets allowed.\n%s\n\n",
					 CurLine, ErrorLine);
					return(50); /* Keep going after error */
					/* break; */
					} /* open-bracket */
				default:
					{
					printf("\nERROR: Line %d, Symbol \"%c\" (0x%02X) not allowed inside optional keyword.\n%s\n\n",
					 CurLine, Current, Current, ErrorLine);
					return(50); /* keep parsing */
					/* break; */
					} /* default */
				} /* switch */
			} /* else */
		else if(Count)
			{ /* in WORD state */
			if(Current == ' ')
				{ /* End of word, exit */
				return(1);
				} /* if */
			else
				{
				printf("\nERROR: Line %d, Symbol \"%c\" (0x%02X) not allowed inside word.\n%s\n\n",
				 CurLine, Current, Current, ErrorLine);
				return(50); /* we'll be back */
				/* break; */
				} /* else */
			} /* if */
		else
			{ /* in WHITESPACE state */
			switch(Current)
				{
				case '[':
					{
					Optional = TRUE;
					break;
					} /* openbracket */
				case '.':
					{
					return(4); /* EXTEND */
					/* break; */ /* Why? It'll just make the compiler complain. */
					} /* period */
				case '=':
					{
					return(5); /* ASSIGN */
					/* break; */ /* Why? It'll just make the compiler complain. */
					} /* equals */
				case '*':
					{
					return(3); /* WILDCARD */
					} /* asterisk */
				case ' ':
				case '\n':
					{
					/* do nothing */
					break;
					} /* space */
				default:
					{
					printf("\nERROR: Line %d, Bogus symbol '%c' (0x%02X).\n%s\n\n",
					 CurLine, Current, Current, ErrorLine);
					return(50); /* Bad, but keep going */
					/* break; */
					} /* default */
				} /* switch */
			} /* else */
		} /* else */
	} /* for */

Buf[Size - 1] = NULL;
printf("\nERROR: Line %d, Line overrun.\n", CurLine);
return(50);
} /* FetchWord() */

int Zgetc(FILE *In)
{

if(In == NULL)
	return(EOF); /* just to be paranoid */

/* Latent code lurking in main() will probably obsolete this next if.
** I suspect it'll never actually get run. But just in case. */

if(CurChar == 0)
	{
	if(fgets(LineBuf, 200, In) == NULL)
		return(EOF);
	ErrorLine[0] = '\"';
	ErrorLine[1] = NULL;
	strcat(ErrorLine, LineBuf);
	ErrorLine[strlen(ErrorLine) - 1] = '\"';
	CurLine++;
	} /* if */

if(LineBuf[CurChar] == '\n')
	{
	CurChar = 0;
	return('\n');
	} /* if */

return(LineBuf[CurChar++]);
} /* Zgetc() */

int RecogWord(char *Word, struct AlphaNode *Root)
{
int Scan;
struct AlphaNode *Walk, *OnlyOne;
unsigned char Index;

for(Scan = 0; Scan < strlen(Word); Scan++)
	{
	if(!isalpha(Word[Scan]))
		{
		printf("ERROR: Invalid input character in RecogWord(\"%s\").\n", Word);
		return(-1); /* You lose, pal */
		} /* if */
	else
		{
		Word[Scan] = tolower(Word[Scan]);
		} /* else */
	} /* for */

Walk = Root;

for(Scan = 0; Scan < strlen(Word) + 1; Scan++)
	{
	if(Word[Scan] == NULL)
		{ /* come to end of input word, should have recognised at this point */
		if(Walk->Term)
			{
/*			printf("(%d)", Walk->EntryNum); */
			return(Walk->EntryNum); /* Tres simple */
			} /* if */
		else
			{
			/* If you want to require exact word-matching, just return an error
			** at this point. Otherwise, we'll walk out towards the end, looking
			** to see if there is only one path. If there is, we'll accept that.
			** If there are two ways to go at any route, we'll return an error. */
			while(Walk)
				{
				OnlyOne = NULL;
				for(Index = 0; Index < 26; Index++)
					{
					if(Walk->AlphaIndex[Index].AlphaChild)
						{
						if(OnlyOne)
							{
							printf("ERROR: Unable to resolve unique match in RecogWord(\"%s\").\n", Word);
							return(-1);
							} /* if */
						else
							{
							OnlyOne = Walk->AlphaIndex[Index].AlphaChild;
							} /* else */
						} /* if */
					} /* for */
				if(OnlyOne)
					{ /* move further into the table */
					Walk = OnlyOne;
					} /* if */
				else
					{ /* no further to go, we must have a unique match */
					if(Walk->Term)
						{
						return(Walk->EntryNum);
						} /* if */
					else
						{ /* Uh, oh, this shouldn't happen. */
						printf("ERROR: Reached end without Termination in RecogWord(\"%s\").", Word);
						return(-1);
						} /* else */
					} /* if */
				} /* while */
			} /* if */
		} /* if */
	else
		{
		Index = Word[Scan] - 'a';
		if(Walk->AlphaIndex[Index].AlphaChild)
			{
			Walk = Walk->AlphaIndex[Index].AlphaChild;
			} /* if */
		else
			{ /* we have found a word that is not in the table */
			printf("ERROR: Word not recognised by RecogWord().\n");
			return(-1);
			} /* else */
		} /* else */
	} /* for */

return(-1); /* To be safe. I was burned once. */
} /* RecogWord() */

int LeadingSpace(char *Data)
{
int Loop;

for(Loop = 0;;Loop++)
	{
	if(Data[Loop] != ' ')
		return(Loop);
	} /* for */
} /* LeadingSpace() */

int ProcessSentance(char *DispatchFunc)
{
int Loop;

for(Loop = 0; Loop < 10; Loop++)
	{
	if(Sentance[Loop])
		{
#ifdef WORD_VERBOSE
		if(Sentance[Loop] & RECOG_WILD)
			printf("* ");
		if(Sentance[Loop] & RECOG_OPTIONAL)
			printf("[%s] ", DeRefWord(Sentance[Loop]));
		else
			printf("%s ", DeRefWord(Sentance[Loop]));
#endif /* WORD_VERBOSE */
		} /* if */
	else
		{
#ifdef WORD_VERBOSE
		printf("= %s\n", DispatchFunc);
#endif /* WORD_VERBOSE */
		return(WordTableAdd(DispatchFunc, &CommandRoot, 0));
		} /* else */
	} /* for */


} /* ProcessSentance() */

char *DeRefWord(int WordNum)
{

/* Note: The Markov table is designed for exactly the _opposite_ type of
** search from the kind we're about to do. Thus, this is a brute force
** search method, and is gonna be slow. Fortunately, this is only done
** during the table compiliation process, and never during actual use. */

static char FinalBuf[80], TempBuf[80];
memset(FinalBuf, 0, 79);
memset(TempBuf, 0, 79);

if (DeRefTwo(&MainTable.MarkovRoot, (WordNum & 0x00ffffff), TempBuf, FinalBuf))
	{
	MAnGleCaSe(FinalBuf);
	return(FinalBuf); /* Can do this cause it's static. */
	} /* if */
else
	{
	printf("ERROR: Didn't find word %d in table. How'd you do that?\n", WordNum);
	FinalBuf[0] = 0;
	return(FinalBuf);
	} /* else */

} /* DeRefWord */

int DeRefTwo(struct AlphaNode *Walk, int SearchWord, char *Temp, char *BringForth)
{
int Scan, Unique, ThrowForward = 0;
char Moron;

/* Moron is used to avoid a particularly annoying compiler warning about a
** perfectly valid condition. Yes, Mommy. */

for(Unique = Scan = 0; Scan < 26; Scan++)
	{
	if(Walk->AlphaIndex[Scan].AlphaChild)
		{
		Unique++;
		} /* if */
	} /* for */

if(Walk->Term)
	{
	if(Walk->EntryNum == SearchWord)
		{
		if(Unique > 0)
			{
			Moron = Temp[strlen(Temp) - 1];
			Temp[strlen(Temp) - 1] = toupper(Moron);
			} /* if */
		strcpy(BringForth, Temp);
		return(1);
		} /* if */
	else
		{
		ThrowForward = TRUE;
		} /* else */
	} /* if */

for(Scan = 0; Scan < 26; Scan++)
	{
	if(Walk->AlphaIndex[Scan].AlphaChild)
		{
		if(ThrowForward || (Unique > 1))
			{
			Temp[strlen(Temp)] = (Scan + 'A');
			} /* if */
		else
			{
			Temp[strlen(Temp)] = (Scan + 'a');
			} /* if */
		if(DeRefTwo(Walk->AlphaIndex[Scan].AlphaChild, SearchWord, Temp, BringForth))
			return(1);
		Temp[strlen(Temp) - 1] = NULL;
		} /* if */
	} /* for */

return(0);
} /* DeRefTwo() */

void MAnGleCaSe(char *String)
{
int Zip, Zap; /* Always use descriptive variable names, right Gary? */

for(Zap = Zip = 0; Zip < strlen(String); Zip++)
	{
	if(isupper(String[Zip]))
		Zap = Zip;
	} /* for */

for(Zip = 0; Zip < Zap; Zip++)
	{
	String[Zip] = toupper(String[Zip]);
	} /* for */

} /* MAnGleCaSe() */

char *stralloc(char *CopyStr)
{
char *temp;

if(temp = malloc(strlen(CopyStr) + 1))
	{
	strcpy(temp, CopyStr);
	} /* if */
return(temp);

} /* stralloc() */

/* Third time's a charm... */
int WordTableAdd(char *FuncName, struct WordEntry *This, int Index)
{ /* returns 1 if bad, 0 if good */
struct WordEntry *New, **Scan;

/* Catch stupid errors, maybe */
if(Sentance[Index])
	{
	if(New = WordEntry_New())
		{
		New->WordToken = (Sentance[Index] & 0x3fffffff); /* all but top two bits */
		if(This->Child)
			{
			Scan = &This->Child;
			while(*Scan) /* If I'm wrong, we'll get enforcer hits here. But I'm not. */
				{
				if((*Scan)->WordToken == New->WordToken)
					{
					WordEntry_Del(New); /* Didn't need it anyway. */
					New = *Scan;
					return(StageTwo(FuncName, New, Index));
					} /* if */
				
				if((*Scan)->WordToken > New->WordToken)
					{
					New->Next = *Scan;
					*Scan = New; /* Veddy important */
					WordEntries++;
					return(StageTwo(FuncName, New, Index));
					} /* if */
				else
					{
					if((*Scan)->Next)
						{ /* move on to next one */
						/* Hah. Let's see our competitors figure this one out... */
						Scan = &((*Scan)->Next);
						/* Oh, you mean we have to understand it too...? */
						} /* if */
					else
						{ /* End of linked list */
						(*Scan)->Next = New;
						WordEntries++;
						return(StageTwo(FuncName, New, Index));
						} /* else */
					} /* else */
				} /* while */
			} /* if */
		else
			{
			This->Child = New;
			WordEntries++; /* Debugging counter */
			return(StageTwo(FuncName, New, Index));
			} /* else */
		} /* if */
	else
		{
		printf("ERROR: Couldn't WordEntry_New().\n");
		return(1);
		} /* else */
	} /* if */

return(0); /* could happen if optional word at end of sentance */
} /* WordTableAdd() */

int StageTwo(char *FuncName, struct WordEntry *This, int Index)
{ /* Really just a continuation of WordTableAdd() */

if(Sentance[Index + 1])
	{ /* carry on with WordTableAdd() */
	if(Sentance[Index + 1] & RECOG_OPTIONAL)
		{
		if(WordTableAdd(FuncName, This, Index + 2)) /* Plus 2 skips optional word */
			{
			return(1);
			} /* if */
		} /* if */
	return(WordTableAdd(FuncName, This, Index + 1));
	} /* if */
else
	{ /* end of the sentance */
	if(This->FuncName = stralloc(FuncName))
		return(0);
	else
		return(1);
	} /* else */

} /* StageTwo() */

struct WordEntry *WordEntry_New(void)
{
return(calloc(1, sizeof(struct WordEntry)));
} /* WordEntry_New() */

void WordEntry_Del(struct WordEntry *This)
{
free(This);
} /* WordTable_Del() */

void WordEntry_NumberTable(struct WordEntry *Base)
{
struct WordEntry *Walk;

Walk = Base->Child;
while(Walk)
	{
	Walk->EntryNum = WordEntries++;
	Walk = Walk->Next;
	} /* while */

Walk = Base->Child;
while(Walk)
	{
	if(Walk->Child)
		WordEntry_NumberTable(Walk);
	Walk = Walk->Next;
	} /* while */

} /* WordEntry_NumberTable() */

void WordEntry_WriteTable(FILE *Out, struct WordEntry *Base)
{
struct WordEntry *Walk;

Walk = Base->Child;
while(Walk)
	{
	if(Walk->Next)
		{
		fprintf(Out, "{0x%08x, ", Walk->WordToken);
		} /* if */
	else
		{
		fprintf(Out, "{0x%08x, ", (Walk->WordToken | RECOG_ENDLEVEL));
		} /* else */
	if(Walk->Child)
		{
		fprintf(Out, "0x%08x, ", Walk->Child->EntryNum);
		} /* if */
	else
		{
		fprintf(Out, "0,          ");
		} /* else */
	if(Walk->FuncName)
		{
		sprintf(ScratchStr, "%s},", Walk->FuncName);
		} /* if */
	else
		{ /* { Extra brace to help TTX's bracket-match */
		sprintf(ScratchStr, "0},");
		} /* else */
	fprintf(Out, "% -15s", ScratchStr);
	fprintf(Out, " /* % 3d: %s ", WordEntries++, DeRefWord(Walk->WordToken));
	if(Walk->WordToken & RECOG_WILD)
		{
		fprintf(Out, "[arg] ");
		} /* if */
	if(Walk->Child)
		{
		fprintf(Out, "goes to %ld. ", Walk->Child->EntryNum);
		} /* if */
	if(Walk->FuncName)
		{
		fprintf(Out, "(%s) ", Walk->FuncName);
		} /* if */
	fprintf(Out, "*/\n");
	if(Walk->Next == NULL)
	fprintf(Out, "\n");
	Walk = Walk->Next;
	} /* while */

Walk = Base->Child;
while(Walk)
	{
	if(Walk->Child)
		WordEntry_WriteTable(Out, Walk);
	Walk = Walk->Next;
	} /* while */

return;
} /* WordEntry_WriteTable() */
