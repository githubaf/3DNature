// ProjectIODetail.h
// Objects and code for working with ProjectIODetail
// Needed for DEMCore to work stand-alone

struct ProjectIODetail
	{
	struct ProjectIODetail *Next;
	struct ChunkIODetail *Detail;
	unsigned long         Flags;
	char                  ChunkID[9];
	}; // ProjectIODetail

struct ProjectIODetail *ProjectIODetailSearchStandAlone(struct ProjectIODetail *This, char *Search);
