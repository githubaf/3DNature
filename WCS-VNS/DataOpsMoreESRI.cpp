// DataOpsMoreESRI.cpp
// Code to handle ESRI E00 & ADF files
// Code created on 11/13/01 by Frank Weed II
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "Project.h"
#include "AppMem.h"

//#define DEBUG_VERBOSE

short ImportWizGUI::LoadARCBinaryADFDEM(char *filename, float *Output, char TestOnly)
{
double PixelSizeX, PixelSizeY;
float *BlockBuffer = NULL, *BlockPtr = NULL, *DecodeBuffer = NULL, *Decoded = NULL, *ElevBase = NULL, *ElevPtr = NULL;
BusyWin *BWDO = NULL;
FILE *ftmp, *fdata = NULL, *fndx = NULL;
unsigned long BlockSize, BlocksPerColumn, BlocksPerRow, BlockX, BlockY, BlockXSize, BlockYSize, CellType, lastx, lasty, LastXBlock, LastYBlock, unpacked;
unsigned long offset, /* TileX, TileXSize, TileY, TileYSize, tmp, */ VirtualBlock, /* VirtualX, VirtualY, */ x, y;
long Min, lval;
unsigned short i, TileSize;
short error = 0;	// default to no error
short first, last;
char errmsg[512], ftmpname[256], scratch[80];
UBYTE BlockType, *info, MinSize;
#ifdef DEBUG
char debugstr[8192];
#endif // DEBUG

// Process the Header file
strcpy(ftmpname, Importing->InDir);
strcat(ftmpname, "hdr.adf");
if ((ftmp = PROJ_fopen(ftmpname, "rb")) == NULL)
	{
	error = 2;
	sprintf(errmsg, "Unable to open file '%s'", ftmpname);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
	goto Cleanup;
	} // if
fread(scratch, 1, 8, ftmp);
if (strcmp(scratch, "GRID1.2") != 0)
	{
	error = 36;
	sprintf(errmsg, "Not a grid file");
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
	goto Cleanup;
	} // if
fseek(ftmp, 16, SEEK_SET);
CellType = GetB32U(ftmp);
INVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;	// this routine will convert all integer types to floats
if ((CellType != 1) && (CellType != 2))
	{
	error = 36;
	sprintf(errmsg, "Unknown Grid Type");
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
	goto Cleanup;
	} // if
Importing->ByteOrder = DEM_DATA_BYTEORDER_HILO; 
INVALUE_SIZE = DEM_DATA_VALSIZE_LONG; 
fseek(ftmp, 256, SEEK_SET);
PixelSizeX = GetB64(ftmp);
PixelSizeY = GetB64(ftmp);
fseek(ftmp, 16, SEEK_CUR);
BlocksPerRow = GetB32U(ftmp);
BlocksPerColumn = GetB32U(ftmp);	//lint -e550
BlockXSize = GetB32U(ftmp);
fseek(ftmp, 4, SEEK_CUR);
BlockYSize = GetB32U(ftmp);
BlockSize = BlockXSize * BlockYSize;
fclose(ftmp);

if (TestOnly)
	{
	// Process the GeoRef Bounds file
	strcpy(ftmpname, Importing->InDir);
	strcat(ftmpname, "dblbnd.adf");
	if ((ftmp = PROJ_fopen(ftmpname, "rb")) == NULL)
		{
		error = 2;
		sprintf(errmsg, "Unable to open file '%s'", ftmpname);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
		goto Cleanup;
		} // if
	Importing->WBound = GetB64(ftmp);
	Importing->SBound = GetB64(ftmp);
	Importing->EBound = GetB64(ftmp);
	Importing->NBound = GetB64(ftmp);
	Importing->InCols = (long)(fabs(Importing->WBound - Importing->EBound) / PixelSizeX + 0.5);
	Importing->InRows = (long)((Importing->NBound - Importing->SBound) / PixelSizeY + 0.5);
	fclose(ftmp);
	} // if TestOnly

#ifdef WCS_BUILD_VNS
if (TestOnly)
	{
	// Process the Projection file if found
	strcpy(ftmpname, Importing->InDir);
	strcat(ftmpname, "prj.adf");
	if ((ftmp = PROJ_fopen(ftmpname, "rb")) != NULL)
		ReadArcPrj(ftmp);
	} // if TestOnly
#endif // WCS_BUILD_VNS

// the PRJ file may have set a different scale
if (TestOnly)
	{
	Importing->NBound *= Importing->HScale;
	Importing->SBound *= Importing->HScale;
	Importing->EBound *= Importing->HScale;
	Importing->WBound *= Importing->HScale;
	} // if TestOnly

if (TestOnly)
	{
	// get the min/max from the statistics file
	strcpy(ftmpname, Importing->InDir);
	strcat(ftmpname, "sta.adf");
	if ((ftmp = PROJ_fopen(ftmpname, "rb")) != NULL)
		{
		Importing->TestMin = GetB64(ftmp);
		Importing->TestMax = GetB64(ftmp);
		fclose(ftmp);
		} // if
	// grab the directory name to use for the database name
	strcpy(ftmpname, Importing->InDir);
	first = last = (short)(strlen(ftmpname) - 1); // point to the trailing slash
	ftmpname[last] = 0;	// make that the string terminator
	while ((first >= 0) && (ftmpname[first] != '\\') && (ftmpname[first] != ':'))	// scan forward to start of directory or assign
		first--;
	strcpy(Importing->NameBase, &ftmpname[first + 1]);	// now that we have the parent directory, make it the database name
	} // if TestOnly
else
	{
	// try to find the tile index
	strcpy(ftmpname, Importing->InDir);
	sprintf(scratch, "w001001x.adf");
	strcat(ftmpname, scratch);
	if ((fndx = PROJ_fopen(ftmpname, "rb")) == NULL)
		{
		error = 36;
		sprintf(errmsg, "Unable to open tile index file %s", ftmpname);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
		goto Cleanup;
		} // if
	// try to find the tile data
	strcpy(ftmpname, Importing->InDir);
	sprintf(scratch, "w001001.adf");
	strcat(ftmpname, scratch);
	if ((fdata = PROJ_fopen(ftmpname, "rb")) == NULL)
		{
		error = 36;
		sprintf(errmsg, "Unable to open grid data file %s", ftmpname);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
		goto Cleanup;
		} // if
	// now the real fun starts...
	if (CellType == 1)	// Integer cells
		{
		UBYTE ub_val;
		char b_val;
		short s_val;
		//unsigned short us_val;
		long l_val;
		float nodata = (float)(Importing->TestMin - 1.0);
		if ((BlockBuffer = (float *)AppMem_Alloc((unsigned int)(BlockSize * sizeof(float) + 8), 0L)) == NULL)
			{
			error = 36;	// error in data
			sprintf(errmsg, "Unable to allocate memory for BlockBuffer (size %lu)", BlockSize * sizeof(float) + 8);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
			goto Cleanup;
			} // if
		if ((DecodeBuffer = (float *)AppMem_Alloc((unsigned int)(BlockSize * sizeof(float)), 0L)) == NULL)
			{
			error = 36;	// error in data
			sprintf(errmsg, "Unable to allocate memory for DecodeBuffer (size %lu)", BlockSize * sizeof(float));
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
			goto Cleanup;
			} // if
		Importing->HasNulls = TRUE;
		Importing->NullMethod = IMWIZ_NULL_LE;
		Importing->NullVal = nodata;
		// I see no reason to loop through all the blocks, checking their size to see if there's any data stored in them.
		// It seems like it's easy enough just to compute what blocks will have data and then loop through those.
		LastXBlock = ((unsigned long)Importing->InCols) / BlockXSize;
		LastYBlock = ((unsigned long)Importing->InRows) / BlockYSize;
		BWDO = new BusyWin("Reading", LastYBlock, 'BWDO', 0);
		for (BlockY = 0; BlockY <= LastYBlock; BlockY++)
			{
			if (BlockY == LastYBlock)
				{
				if (Importing->InRows % (long)BlockYSize == 0)
					break;
				else
					lasty = ((unsigned long)Importing->InRows - 1) % BlockYSize;	// changed 09/10/04
					// lasty = ((unsigned long)Importing->InRows) % BlockYSize - 1;
				} // if
			else
				lasty = BlockYSize - 1;
			for (BlockX = 0; BlockX <= LastXBlock; BlockX++)
				{
				//if ((BlockY == 219) && (BlockX == 3))
				//	printf("foo");
				if (BlockX == LastXBlock)
					lastx = ((unsigned long)Importing->InCols - 1) % BlockXSize;	// changed 09/10/04
					//lastx = ((unsigned long)Importing->InCols) % BlockXSize - 1;
				else
					lastx = BlockXSize - 1;
				// seek to current block
				VirtualBlock = BlockY * BlocksPerRow + BlockX;
				fseek(fndx, 100 + VirtualBlock * 8, SEEK_SET);
				offset = 2 * GetB32U(fndx);
				TileSize = (unsigned short)GetB32U(fndx);
#ifdef DEBUG
				sprintf(debugstr, "\nVirtualBlock = %d (%dX, %dY)\n", VirtualBlock, BlockX, BlockY);
				DEBUGOUT(debugstr);
				sprintf(debugstr, "Tile seek = %d\n", offset);
				DEBUGOUT(debugstr);
				sprintf(debugstr, "Tile size = %d words (%d bytes)\n", TileSize, TileSize * 2);
				DEBUGOUT(debugstr);
#endif // DEBUG
				//sprintf(errmsg, "Index File: Virtual Block #%lu @%lu, Data File: Seek = %lu", VirtualBlock, ftell(fndx) - 4, offset);
				//GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
				// read block into blockbuffer
				fseek(fdata, offset, SEEK_SET);
				if (GetB16U(fdata) != TileSize)
					{
					error = 36;	// error in data
					sprintf(errmsg, "Encountered size mismatch in file");
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
					goto Cleanup;
					} // if
#ifdef DEBUG
				memset(BlockBuffer, 0, BlockSize * sizeof(float));	/*** F2 NOTE: For debugging only ***/
				memset(DecodeBuffer, 0, BlockSize * sizeof(float));	/*** F2 NOTE: For debugging only ***/
#endif // DEBUG
//				assert((BlockSize * sizeof(float) + 8) >= (TileSize * 2));
				fread(BlockBuffer, 1, TileSize * 2, fdata);
				info = (UBYTE *)BlockBuffer;
				BlockType = info[0];
				MinSize = info[1];
				info += 2;
				if (MinSize == 0)
					Min = 0;
				if (MinSize == 1)
					Min = b_val = *(char *)info++;
				else if (MinSize == 2)
					{
					s_val = *(short *)info;
					info += 2;
#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip16S(s_val, &s_val);
#endif // BYTEORDER_LITTLEENDIAN
					Min = s_val;
					} // else if
				else if (MinSize == 4)
					{
					Min = *(long *)info;
					info += 4;
#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip32S(Min, &Min);
#endif // BYTEORDER_LITTLEENDIAN
					} // else if
				Decoded = DecodeBuffer;
				unpacked = 0;
				switch (BlockType)
					{
					unsigned char Marker;
					case 0x00:	// constant block
						// F2 NOTE: verified correct 09/24/04
#ifdef DEBUG
						DEBUGOUT("\n00 Block\n");
						sprintf(debugstr, "Full tile (%d) of constant (%d)\n", BlockSize, Min);
						DEBUGOUT(debugstr);
#endif // DEBUG
						for (i = 0; i < BlockSize; i++)
							*Decoded++ = (float)Min;

						break;
					case 0x01:	// raw 1 bit data
#ifdef DEBUG
						DEBUGOUT("\n01 Block\n");
#endif // DEBUG
						UserMessageOK("Import Wizard", "Please contact 3D Nature.  This file can't be decoded properly at this time.");
						break;
					case 0x04:	// raw 4 bit data
						// F2 NOTE: Finally implemented 04/20/06 - seems to work
#ifdef DEBUG
						DEBUGOUT("\n04 Block\n");
#endif // DEBUG
						for (i = 0; i < BlockSize; i += 2)
							{
							ub_val = *info++;
							*Decoded++ = (float)(((ub_val & 0xf0) >> 4) + Min);
							*Decoded++ = (float)((ub_val & 0x0f) + Min);
							} // for
						break;
					case 0x08:	// raw 8 bit data
						// F2 NOTE: seems to work - checked again 04/20/06
#ifdef DEBUG
						DEBUGOUT("\n08 Block\n");
						sprintf(debugstr, "Full tile of bytes (%d)\n", BlockSize);
						DEBUGOUT(debugstr);
#endif // DEBUG
						for (i = 0; i < BlockSize; i++)
							{
							ub_val = *info++;
#ifdef DEBUG_VERBOSE
							sprintf(debugstr, "%d ", (long)(ub_val + Min));
							DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
							*Decoded++ = (float)(ub_val + Min);
							} // for
#ifdef DEBUG_VERBOSE
						DEBUGOUT("\n");
#endif // DEBUG_VERBOSE
						break;
					case 0x10:	// raw 16 bit data
						// F2 NOTE: seems to work - modified 04/19/06
#ifdef DEBUG
						DEBUGOUT("\n10 Block\n");
#endif // DEBUG
						for (i = 0; i < BlockSize; i++)
							{
							//us_val = *(unsigned short *)info;
							s_val = *(short *)info;
							info += 2;
							#ifdef BYTEORDER_LITTLEENDIAN
							//SimpleEndianFlip16U(us_val, &us_val);
							SimpleEndianFlip16S(s_val, &s_val);
							#endif // BYTEORDER_LITTLEENDIAN
							//*Decoded++ = (float)(us_val + Min);
							*Decoded++ = (float)(s_val + Min);
							} // for
						break;
					case 0x20:	// raw 32 bit data
						// F2 NOTE: Untested
#ifdef DEBUG
						DEBUGOUT("\n20 Block\n");
#endif // DEBUG
						for (i = 0; i < BlockSize; i++)
							{
							l_val = *(long *)info;
							info += 4;
							#ifdef BYTEORDER_LITTLEENDIAN
							SimpleEndianFlip32S(l_val, &l_val);
							#endif // BYTEORDER_LITTLEENDIAN
							*Decoded++ = (float)(l_val + Min);
							} // for
						break;
					case 0xCF:	// 16 bit literal runs/nodata runs
						// F2 NOTE: both marker types verified correct 09/13/04
#ifdef DEBUG
						DEBUGOUT("\nCF Block\n");
#endif // DEBUG
						Marker = *info++;
						while ((unpacked < BlockSize) && (Marker != 0))
							{
							if (Marker < 128)
								{
								UBYTE i;
#ifdef DEBUG_VERBOSE
								sprintf(debugstr, "Marker = %x (run of %d shorts)\n", Marker, Marker);
								DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
								for (i = 0; i < Marker; i++)
									{
									s_val = *(short *)info;
									info += 2;
									#ifdef BYTEORDER_LITTLEENDIAN
									SimpleEndianFlip16S(s_val, &s_val);
									#endif // BYTEORDER_LITTLEENDIAN
									//if ((lastx != 255) || (lasty != 3))
									//	printf("foo!");
//									if ((x < lastx) && (y < lasty))
//										{
#ifdef DEBUG_VERBOSE
										sprintf(debugstr, "%d ", (long)(s_val + Min));
										DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
										*Decoded++ = (float)(s_val + Min);
//										} // if
									} // for
								unpacked += Marker;
#ifdef DEBUG_VERBOSE
								DEBUGOUT("\n");
#endif // DEBUG_VERBOSE
								} // if
							else if (Marker > 127)
								{
#ifdef DEBUG_VERBOSE
								sprintf(debugstr, "Marker = %x (%d nulls)\n", Marker, (256 - Marker));
								DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
								for (i = 0; i < (256 - Marker); i++)
									{
#ifdef DEBUG_VERBOSE
									DEBUGOUT("Null ");
#endif // DEBUG_VERBOSE
									*Decoded++ = nodata;
									}
								unpacked += (256 - Marker);
#ifdef DEBUG_VERBOSE
								DEBUGOUT("\n");
#endif // DEBUG_VERBOSE
								} // else if
							Marker = *info++;
							} // while
						break;
					case 0xD7:	// literal runs/nodata runs
						// F2 NOTE: both marker types verified correct 09/13/04
#ifdef DEBUG
						DEBUGOUT("\nD7 Block\n");
#endif // DEBUG
						Marker = *info++;
						while ((unpacked < BlockSize) && (Marker != 0))
							{
							if (Marker < 128)
								{
#ifdef DEBUG_VERBOSE
								sprintf(debugstr, "Marker = %x (run of %d bytes)\n", Marker, Marker);
								DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
								for (i = 0; i < Marker; i++)
									{
#ifdef DEBUG_VERBOSE
									sprintf(debugstr, "%d ", (long)(*info + Min));
									DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
									*Decoded++ = (float)(*info++ + Min);
									}
								unpacked += Marker;
#ifdef DEBUG_VERBOSE
								DEBUGOUT("\n");
#endif // DEBUG_VERBOSE
								} // if
							else if (Marker > 127)
								{
#ifdef DEBUG_VERBOSE
								sprintf(debugstr, "Marker = %x (%d nulls)\n", Marker, (256 - Marker));
								DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
								for (i = 0; i < (256 - Marker); i++)
									{
#ifdef DEBUG_VERBOSE
									DEBUGOUT("<Null>");
#endif // DEBUG_VERBOSE
									*Decoded++ = nodata;
									}
								unpacked += (256 - Marker);
#ifdef DEBUG_VERBOSE
								DEBUGOUT("\n");
#endif // DEBUG_VERBOSE
								} // else if
							Marker = *info++;
							} // while
						break;
					case 0xDF:	// minimum value runs/nodata runs
						// F2 NOTE: modified 04/19/06 - seems to work
#ifdef DEBUG
						DEBUGOUT("\nDF Block\n");
#endif // DEBUG
						Marker = *info++;
						while ((unpacked < BlockSize) && (Marker != 0))
							{
							if (Marker < 128)
								{
								for (i = 0; i < Marker; i++)
									*Decoded++ = (float)(Min);
								unpacked += Marker;
								}
							else if (Marker > 127)
								{
								for (i = 0; i < (256 - Marker); i++)
									*Decoded++ = nodata;
								unpacked += 256 - Marker;
								}
							Marker = *info++;
							};
						break;
					case 0xE0:	// RLE 32 bit
						// F2 NOTE: modified 04/19/06 - seems to work
#ifdef DEBUG
						DEBUGOUT("\nE0 Block\n");
#endif // DEBUG
						Marker = *info++;
						while ((unpacked < BlockSize) && (Marker != 0))
							{
							lval = *((long *)info);
#ifdef BYTEORDER_LITTLEENDIAN
							SimpleEndianFlip32S(lval, &lval);
#endif // BYTEORDER_LITTLEENDIAN
							info += 4;
							for (i = 0; i < Marker; i++)
								*Decoded++ = (float)(lval + Min);
							unpacked += Marker;
							Marker = *info++;
							}
						break;
					case 0xF0:	// RLE 16 bit
						// F2 NOTE: seems to work - modified 04/19/06 - still seems to work
#ifdef DEBUG
						DEBUGOUT("\nF0 Block\n");
#endif // DEBUG
						Marker = *info++;
#ifdef DEBUG_VERBOSE
						sprintf(debugstr, "Marker = %x (replicate %d shorts)\n", Marker, Marker);
						DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
						while ((unpacked < BlockSize) && (Marker != 0))
							{
							s_val = *((short *)info);
#ifdef BYTEORDER_LITTLEENDIAN
							SimpleEndianFlip16S(s_val, &s_val);
#endif // BYTEORDER_LITTLEENDIAN
							info += 2;
							for (i = 0; i < Marker; i++)
								{
#ifdef DEBUG_VERBOSE
								sprintf(debugstr, "%d ", (long)(s_val + Min));
								DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
								*Decoded++ = (float)(s_val + Min);
								} // for
							unpacked += Marker;
							Marker = *info++;
							} // while
						break;
					case 0xF8:
					case 0xFC:	// RLE 8 bit
						// F2 NOTE: seems to work
#ifdef DEBUG
						DEBUGOUT("\nF8/FC Block\n");
#endif // DEBUG
						Marker = *info++;
						while ((unpacked < BlockSize) && (Marker != 0))
							{
							ub_val = *info++;
#ifdef DEBUG_VERBOSE
							sprintf(debugstr, "Marker = %x (run of %d bytes)\n", Marker, Marker);
							DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
							for (i = 0; i < Marker; i++)
								{
#ifdef DEBUG_VERBOSE
								sprintf(debugstr, "%d ", (long)(ub_val + Min));
								DEBUGOUT(debugstr);
#endif // DEBUG_VERBOSE
								*Decoded++ = (float)(ub_val + Min);
								} // for
							unpacked += Marker;
							Marker = *info++;
#ifdef DEBUG_VERBOSE
							DEBUGOUT("\n");
#endif // DEBUG_VERBOSE
							} // while
						break;
					case 0xFF:	// nobody's figured this one out yet
#ifdef DEBUG
						DEBUGOUT("\nFF Block\n");
#endif // DEBUG
						break;
					default:
						error = 36;
						sprintf(errmsg, "Unknown Tile Type (%ux)", BlockType);
						GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
						goto Cleanup;
						break;	//lint !e527
					} // switch BlockType
				// now transfer the decoded buffer to the output
				ElevBase = Output + BlockY * BlockYSize * Importing->InCols + BlockX * BlockXSize;
				for (y = 0; y < BlockYSize; y++)
					{
					ElevPtr = ElevBase + y * Importing->InCols;
					Decoded = DecodeBuffer + y * BlockXSize;
					for (x = 0; x < BlockXSize; x++)
						{
						if ((x <= lastx) && (y <= lasty))	// copy if we're in the valid region?
							{
							assert((ElevPtr >= Output) && (ElevPtr < (Output + Importing->InRows * Importing->InCols * sizeof(float))));
							assert((Decoded >= DecodeBuffer) && (Decoded < (DecodeBuffer + BlockSize * sizeof(float))));
							if (*Decoded < Importing->TestMin)
								*Decoded = nodata;
							*ElevPtr++ = *Decoded++;
							} // if
						else
							Decoded++;
						} // for x
					} // for y
				} // for BlockX
			if (BWDO)
				{
				if (BWDO->CheckAbort())
					{
					error = 50;
					goto Cleanup;
					} // if
				BWDO->Update(BlockY);
				} // if BWDO
			} // for BlockY
		} // if CellType 1
	else // CellType must be 2 (Float)
		{
		float gnumin;
		gnumin = float(Importing->TestMin - 1.0);
		if ((BlockBuffer = (float *)AppMem_Alloc((unsigned int)(BlockSize * sizeof(float) + 8), 0L)) == NULL)
			{
			error = 36;
			sprintf(errmsg, "Unable to allocate memory for BlockBuffer (size %lu)", BlockSize * sizeof(float) + 8);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
			goto Cleanup;
			} // if
		Importing->HasNulls = TRUE;
		Importing->NullVal = gnumin;
		// I see no reason to loop through all the blocks, checking their size to see if there's any data stored in them.
		// It seems like it's easy enough just to compute what blocks will have data and then loop through those.
		LastXBlock = ((unsigned long)Importing->InCols - 1) / BlockXSize;
		LastYBlock = ((unsigned long)Importing->InRows - 1) / BlockYSize;
		BWDO = new BusyWin("Reading", LastYBlock, 'BWDO', 0);
		for (BlockY = 0; BlockY <= LastYBlock; BlockY++)
			{
			if (BlockY == LastYBlock)
				lasty = ((unsigned long)Importing->InRows - 1) % BlockYSize;
			else
				lasty = BlockYSize - 1;
			for (BlockX = 0; BlockX <= LastXBlock; BlockX++)
				{
				if (BlockX == LastXBlock)
					lastx = ((unsigned long)Importing->InCols - 1) % BlockXSize;
				else
					lastx = BlockXSize - 1;
				// seek to current block
				VirtualBlock = BlockY * BlocksPerRow + BlockX;
				fseek(fndx, 100 + VirtualBlock * 8, SEEK_SET);
				offset = 2 * GetB32U(fndx);
				//sprintf(errmsg, "Index File: Virtual Block #%lu @%lu, Data File: Seek = %lu", VirtualBlock, ftell(fndx) - 4, offset);
				//GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
				// read block into blockbuffer
				fseek(fdata, offset, SEEK_SET);
				TileSize = GetB16U(fdata);
				if ((unsigned long)(TileSize * 2) != (BlockSize * sizeof(float)))
					{
					error = 36;
					sprintf(errmsg, "Block Size Mismatch");
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
					goto Cleanup;
					} // if
				fread(BlockBuffer, 1, BlockSize * sizeof(float), fdata);
				BlockPtr = BlockBuffer;
				ElevBase = Output + BlockY * BlockYSize * Importing->InCols + BlockX * BlockXSize;
				for (y = 0; y < BlockYSize; y++)
					{
					ElevPtr = ElevBase + y * Importing->InCols;
					for (x = 0; x < BlockXSize; x++)
						{
						if ((x <= lastx) && (y <= lasty))	// are we in the valid region?
							{
							assert(ElevPtr < (Output + (Importing->InCols * Importing->InRows * sizeof(float))));
							assert(BlockPtr < (BlockBuffer + BlockSize * sizeof(float)));
							#ifdef BYTEORDER_LITTLEENDIAN
							SimpleEndianFlip32F(BlockPtr, BlockPtr);
							#endif // BYTEORDER_LITTLEENDIAN
							if ((*BlockPtr < Importing->TestMin) || (*BlockPtr > Importing->TestMax))
								*BlockPtr = gnumin;
							*ElevPtr++ = *BlockPtr++;
							} // if
						else	// no, so just step over it
							{
							BlockPtr++;
							} // else
						} // for x
					} // for y
				} // for BlockX
			if (BWDO)
				{
				if (BWDO->CheckAbort())
					{
					error = 50;
					goto Cleanup;
					} // if
				BWDO->Update(BlockY);
				} // if BWDO
			} // for BlockY
		} // else CellType
	} // else TestOnly

Cleanup:

if (BWDO)
	delete BWDO;
if (DecodeBuffer)
	AppMem_Free(DecodeBuffer, (unsigned int)(BlockSize * sizeof(float)));
if (BlockBuffer)
	AppMem_Free(BlockBuffer, (unsigned int)(BlockSize * sizeof(float) + 8));
if (fdata)
	fclose(fdata);
if (fndx)
	fclose(fndx);

return error;

} // ImportWizGUI::LoadARCBinaryADFDEM

/*===========================================================================*/

short ImportWizGUI::LoadARCExportDEM(char *filename, float *Output, char TestOnly)
{
BusyWin *BWDO = NULL;
FILE *input = NULL;
float *ptr;
//long val;
unsigned short cols, i, last_col, last_reads, rows;
short error = 0;
char buffer[256], errmsg[80];
int GridType;

if ((input = PROJ_fopen(filename, "r")) == NULL)
	{
	error = 2;
	sprintf(errmsg, "Unable to open file '%s'", filename);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
	goto Cleanup;
	} // if

fgetline(buffer, 255, input);
if (strncmp(buffer, "EXP  0", 6) != 0)
	{
	error = 36;
	sprintf(errmsg, "File doesn't appear to be an Arc Export file");
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
	goto Cleanup;
	} // if
fgetline(buffer, 255, input);
if (strncmp(buffer, "GRD  2", 6) != 0)
	{
	error = 36;
	sprintf(errmsg, "File doesn't appear to be an Arc Export Grid");
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
	goto Cleanup;
	} // if

fgetline(buffer, 255, input);
sscanf(buffer, "%d%d%d%lf", &Importing->InCols, &Importing->InRows, &GridType, &Importing->NullVal);
INVALUE_SIZE = DEM_DATA_VALSIZE_LONG; 
INVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;
if (TestOnly)
	Importing->HasNulls = TRUE;
// hack to get around precision limits due to the way they store the reference NULL, and the NULL's in the file

// cell width & height
fgetline(buffer, 255, input);

// minimum x & y
fgetline(buffer, 255, input);
if (TestOnly)
	sscanf(buffer, "%lf%lf", &Importing->WBound, &Importing->SBound);

// maximum x & y
fgetline(buffer, 255, input);
if (TestOnly)
	sscanf(buffer, "%lf%lf", &Importing->EBound, &Importing->NBound);

if (TestOnly)
	{
	BOOL haveprj = FALSE, havestats = FALSE, wantmore = TRUE;
	fgetline(buffer, 255, input);
	do
		{
		while (!feof(input) && (strncmp(buffer, "PRJ  2", 6) != 0) && (strncmp(buffer, "STDV ", 5) != 0))
			fgetline(buffer, 255, input);
		if (strncmp(buffer, "PRJ  2", 6) == 0)
			{
#ifdef WCS_BUILD_VNS
			ReadArcPrj(input);
			haveprj = TRUE;
#endif // WCS_BUILD_VNS
			fgetline(buffer, 255, input);
			} // if
		else if (strncmp(buffer, "STDV ", 5) == 0)
			{
			fgetline(buffer, 255, input);
			sscanf(buffer, "%lf%lf", &Importing->TestMin, &Importing->TestMax);
			havestats = TRUE;
			fgetline(buffer, 255, input);
			} // if
		if (haveprj && havestats)
			wantmore = FALSE;
		} while (wantmore && !feof(input));
	if (! haveprj)
		Importing->HasNulls = FALSE;
	} // if TestOnly
else	// TestOnly
	{
	BWDO = new BusyWin("Reading", Importing->InRows, 'BWDO', 0);
	ptr = Output;
	for (rows = 0; rows < Importing->InRows; rows++)
		{
		last_col = (unsigned short)((Importing->InCols + 4) / 5);
		last_reads = (unsigned short)(int)((Importing->InCols - 1) % 5);
		for (cols = 0; cols < last_col; cols++)	// 5 values per line
			{
			fgetline(buffer, 255, input);
			if (GridType == 1)
				{
				int ivals[5];
				if (cols == (last_col - 1))	// short read
					{
					sscanf(buffer, "%d%d%d%d%d", &ivals[0], &ivals[1], &ivals[2], &ivals[3], &ivals[4]);
					for (i = 0; i <= last_reads; i++)
						*ptr++ = (float)ivals[i];
					} // if
				else
					{
					sscanf(buffer, "%d%d%d%d%d", &ivals[0], &ivals[1], &ivals[2], &ivals[3], &ivals[4]);
					*ptr++ = (float)ivals[0];
					*ptr++ = (float)ivals[1];
					*ptr++ = (float)ivals[2];
					*ptr++ = (float)ivals[3];
					*ptr++ = (float)ivals[4];
					} // else
				} // GridType 1 (integer)
			else
				{
				float fvals[5];
				if (cols == (last_col - 1))	// short read
					{
					sscanf(buffer, "%f%f%f%f%f", &fvals[0], &fvals[1], &fvals[2], &fvals[3], &fvals[4]);
					for (i = 0; i <= last_reads; i++)
						*ptr++ = fvals[i];
					} // if
				else
					{
					sscanf(buffer, "%f%f%f%f%f", ptr, ptr + 1, ptr + 2, ptr + 3, ptr + 4);
					ptr += 5;
					} // else
				} // hopefully GridType 2 (floats)
			} // for cols
		if (BWDO)
			{
			if (BWDO->CheckAbort())
				{
				error = 50;
				goto Cleanup;
				} // if
			BWDO->Update((ULONG)rows);
			} // if BWDO
		} // for rows
	fgetline(buffer, 255, input);
	if (strncmp(buffer, "EOG", 3) != 0)
		{
		error = 36;
		sprintf(errmsg, "Sync error with End Of Grid marker");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
		goto Cleanup;
		} // if
	if (GridType == 1)	// hack the data, since there's issues involved with the way they're storing the NULL values
		{
		float gnumin;
		ptr = Output;
		gnumin = float(Importing->TestMin - 1.0);
		for (rows = 0; rows < Importing->InRows; rows++)
			for (cols = 0; cols < Importing->InCols; cols++, ptr++)
				if ((*ptr < Importing->TestMin) || (*ptr > Importing->TestMax))
					*ptr = float(gnumin);
		Importing->NullVal = gnumin;
		} // if
	else if (GridType == 2)	// hack the data, since there's issues involved with the way they're storing the NULL values
		{
		double gnumin;
		ptr = Output;
		// Set NULL data to minimum minus 1% of elevation range
		gnumin = Importing->TestMin - (Importing->TestMax - Importing->TestMin) / 100.0;
		for (rows = 0; rows < Importing->InRows; rows++)
			for (cols = 0; cols < Importing->InCols; cols++, ptr++)
				if ((*ptr < Importing->TestMin) || (*ptr > Importing->TestMax))
					*ptr = float(gnumin);
		Importing->NullVal = gnumin;
		} // else if
	} // else TestOnly

Cleanup:

if (BWDO)
	delete BWDO;
if (input)
	fclose(input);

return error;

} // ImportWizGUI::LoadARCExportDEM

/*===========================================================================*/
