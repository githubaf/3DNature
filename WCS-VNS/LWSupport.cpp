// LWSupport.cpp
// LightWave 3D support functions for WCS
// Written on 1/5/95, Modified 2/96 by Gary R. Huber.
// Motion import and export functions formerly in EdPar.c, written 1994.
// Converted to V2 on 3/21/96 by GRH

#include "stdafx.h"
#include "MathSupport.h"
#include "Requester.h"
#include "Render.h"
#include "DEM.h"
#include "Project.h"
#include "Log.h"
#include "Database.h"
#include "Joe.h"
#include "Useful.h"
#include "MathSupport.h"
#include "SceneExportGUI.h"
#include "SceneImportGUI.h"
#include "Points.h"
#include "EffectsLib.h"
#include "AppMem.h"
#include "GraphData.h"
#include "Toolbar.h"


#ifndef min
#define   min(a,b)    ((a) <= (b) ? (a) : (b))
#endif

long LWNullObj[] = {
#ifdef BYTEORDER_LITTLEENDIAN
	0x4D524F46, 0x28000000, 0x424F574C, 0x53544E50,
	0x0C000000, 0x00000000, 0x00000000, 0x00000000,
	0x53465253, 0x00000000, 0x534C4F50, 0x00000000
#else // BYTEORDER_BIGENDIAN
	0x464F524D, 0x00000028, 0x4C574F42, 0x504E5453,
	0x0000000C, 0x00000000, 0x00000000, 0x00000000,
	0x53524653, 0x00000000, 0x504F4C53, 0x00000000
#endif // BYTEORDER_BIGENDIAN
};

Matx3x3 RotMatx;

// set up reference point in cartesian space
void SceneExportGUI::SetupForExport(ImportInfo *LWInfo, RenderData *Rend)
{
VertexDEM Vert;

Vert.Elev = 0;
Vert.Lat = LWInfo->RefLat;
Vert.Lon = 0.0;
BuildRotationMatrix((LWInfo->RefLat - 90.0) * PiOver180, 0.0, 0.0, LWInfo->RotMatx);
Rend->DefCoords->DegToCart(&Vert);
LWInfo->RefPt[0] = Vert.XYZ[0];
LWInfo->RefPt[1] = Vert.XYZ[1];
LWInfo->RefPt[2] = Vert.XYZ[2];

} // SceneExportGUI::SetupForExport

/*===========================================================================*/

// also defined in ExportFormat.cpp
#define WCS_LW7_DATA_NUMCHANNELS	14

int SceneExportGUI::ExportWave(ImportInfo *LWInfo, FILE *Supplied, RenderData *Rend, int ExportAsVersion)
{
double Last[3];
long MaxFrames, LastKey, TotalKeys, LW7NumKeys, LW7Ct;
VertexData Vert;
LightWaveMotion *LWM = NULL;
FILE *fLWM = NULL;
GraphNode *GrNode;
float *LW7Data[WCS_LW7_DATA_NUMCHANNELS];
short Version, Channels, i, error = 0;
char filename[256], Ptrn[100];

Last[0] = Last[1] = Last[2] = 0.0;
LW7Data[0] = NULL;

if (! Supplied)
	{
	if (LWInfo->Path[0] == 0)
		{
		if (ProjHost->altobjectpath[0] == 0)
			strcpy(ProjHost->altobjectpath, "WCSProjects:");
		strcpy(LWInfo->Path, ProjHost->altobjectpath);
		} // if 
	strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("LWM"));
	strcpy(LWInfo->Name, ProjHost->projectname);
	if (strcmp(&LWInfo->Name[strlen(LWInfo->Name) - 5], ".proj"))
		LWInfo->Name[strlen(LWInfo->Name) - 5] = 0;
	strcat(LWInfo->Name, ".LWM");
	if (! GetFileNamePtrn(1, "Export Motion Path/File", LWInfo->Path, LWInfo->Name, Ptrn, 256))
		return (0);
	if (LWInfo->Name[0] == 0)
		return (0);
	strmfp(filename, LWInfo->Path, LWInfo->Name);
	// set these in case not called from scene saver where it is also calculated
	SetupForExport(LWInfo, Rend);
	} // if 

// allocate some essential resources 

if ((LWM = new LightWaveMotion()) == NULL)
	{
	error = 3;
	goto EndWave;
	} // if out of memory 

// fill in LightWaveMotion structure 
if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
	{
	Version = 3;
	Channels = 6;
	} // if
else
	{
	Version = 1;
	Channels = 9;
	} // else

if (! Supplied)
	{
	if ((fLWM = PROJ_fopen(filename, "w")) == NULL) //lint !e645
		{
		error = 4;
		goto EndWave;
		} // if open fail
	} // if no file supplied 
else
	fLWM = Supplied;

if (! Supplied)
	fprintf(fLWM, "%s\n%1d\n", "LWMO", Version);

Rend->Cam->InitToRender(Rend->Opt, NULL);

// different procedures if key frame interval is 0 - output only at camera, focus or bank key frames or at frame 0 if no keys
if (LWInfo->KeyFrameInt <= 0)
	{
	// are there key frames?
	TotalKeys = 0;
	LastKey = -2;
	while ((LastKey = Rend->Cam->GetNextMotionKeyFrame(LastKey, Rend->FrameRate)) >= 0)
		TotalKeys ++;

	LW7NumKeys = MaxFrames = (TotalKeys > 0) ? TotalKeys: 1;
	LW7Ct = 0;
	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{
		if (LW7Data[0] = (float *)AppMem_Alloc(WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float), APPMEM_CLEAR))
			{
			LW7Data[1] = &LW7Data[0][LW7NumKeys];
			LW7Data[2] = &LW7Data[0][LW7NumKeys * 2];
			LW7Data[3] = &LW7Data[0][LW7NumKeys * 3];
			LW7Data[4] = &LW7Data[0][LW7NumKeys * 4];
			LW7Data[5] = &LW7Data[0][LW7NumKeys * 5];
			LW7Data[6] = &LW7Data[0][LW7NumKeys * 6];
			LW7Data[7] = &LW7Data[0][LW7NumKeys * 7];
			LW7Data[8] = &LW7Data[0][LW7NumKeys * 8];
			LW7Data[9] = &LW7Data[0][LW7NumKeys * 9];
			LW7Data[10] = &LW7Data[0][LW7NumKeys * 10];
			LW7Data[11] = &LW7Data[0][LW7NumKeys * 11];
			LW7Data[12] = &LW7Data[0][LW7NumKeys * 12];
			LW7Data[13] = &LW7Data[0][LW7NumKeys * 13];
			} // if
		else
			{
			error = 3;
			goto EndWave;
			} // else
		fprintf(fLWM, "NumChannels %1d\n", Channels);
		} // if
	else
		{
		fprintf(fLWM, "%1d\n%1d\n", Channels, MaxFrames);
		} // else
	if (TotalKeys)
		{
		LastKey = -2;
		while ((LastKey = Rend->Cam->GetNextMotionKeyFrame(LastKey, Rend->FrameRate)) >= 0)
			{
			LWM->Frame = LastKey;
			// init camera to this frame position
			if (Rend->Cam->TargetObj)
				Rend->Cam->TargetObj->SetToTime(LWM->Frame / Rend->FrameRate);
			Rend->Cam->SetToTime(LWM->Frame / Rend->FrameRate);
			Rend->Cam->InitFrameToRender(EffectsHost, Rend);
			// determine HPB
			Set_LWM(LWM, LWInfo, LWM->Frame, Rend);
			// determine TCB - we'll need the nearest node for that
			if (GrNode = Rend->Cam->GetNearestMotionNode(LWM->Frame / Rend->FrameRate))
				{
				LWM->TCB[0] = GrNode->TCB[0];
				LWM->TCB[1] = GrNode->TCB[1];
				LWM->TCB[2] = GrNode->TCB[2];
				LWM->Linear = GrNode->Linear;
				} // if node found
			else
				{
				LWM->TCB[0] = 0.0;
				LWM->TCB[1] = 0.0;
				LWM->TCB[2] = 0.0;
				LWM->Linear = 0;
				} // else
			if (LastKey >= 0)
				{
				if (fabs(LWM->HPB[0] - Last[0]) > 180.0)
					{
					if (LWM->HPB[0] > Last[0])
						LWM->HPB[0] -= 360.0;
					else
						LWM->HPB[0] += 360.0;
					} // if
				if (fabs(LWM->HPB[1] - Last[1]) > 180.0)
					{
					if (LWM->HPB[1] > Last[1])
						LWM->HPB[1] -= 360.0;
					else
						LWM->HPB[1] += 360.0;
					} // if
				if (fabs(LWM->HPB[2] - Last[2]) > 180.0)
					{
					if (LWM->HPB[2] > Last[2])
						LWM->HPB[2] -= 360.0;
					else
						LWM->HPB[2] += 360.0;
					} // if
				} // if
			Last[0] = LWM->HPB[0];
			Last[1] = LWM->HPB[1];
			Last[2] = LWM->HPB[2];

			if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
				{
				LW7Data[0][LW7Ct] = (float)(LWM->XYZ[0] * LWInfo->UnitScale);
				LW7Data[1][LW7Ct] = (float)(LWM->XYZ[1] * LWInfo->UnitScale);
				LW7Data[2][LW7Ct] = (float)(LWM->XYZ[2] * LWInfo->UnitScale);
				LW7Data[3][LW7Ct] = (float)(LWM->HPB[0] * PiOver180);
				LW7Data[4][LW7Ct] = (float)(LWM->HPB[1] * PiOver180);
				LW7Data[5][LW7Ct] = (float)(LWM->HPB[2] * PiOver180);
				LW7Data[6][LW7Ct] = (float)LWM->SCL[0];
				LW7Data[7][LW7Ct] = (float)LWM->SCL[1];
				LW7Data[8][LW7Ct] = (float)LWM->SCL[2];
				LW7Data[9][LW7Ct] = (float)LWM->TCB[0];
				LW7Data[10][LW7Ct] = (float)LWM->TCB[1];
				LW7Data[11][LW7Ct] = (float)LWM->TCB[2];
				LW7Data[12][LW7Ct] = (float)LWM->Frame;
				LW7Data[13][LW7Ct] = (float)LWM->Linear;
				LW7Ct ++;
				} // if
			else
				{
				if (fprintf(fLWM, "%f %f %f %f %f %f %f %f %f\n",
					LWM->XYZ[0] * LWInfo->UnitScale, LWM->XYZ[1] * LWInfo->UnitScale, LWM->XYZ[2] * LWInfo->UnitScale,
					LWM->HPB[0], LWM->HPB[1], LWM->HPB[2],
					LWM->SCL[0], LWM->SCL[1], LWM->SCL[2]) < 0)
					{
					error = 5;
					break;
					} // if write error 
				if (fprintf(fLWM, "%1d %1d %f %f %f\n",
					LWM->Frame, LWM->Linear,
					LWM->TCB[0], LWM->TCB[1], LWM->TCB[2]) < 0)
					{
					error = 5;
					break;
					} // if write error 
				} // else
			} // while
		} // if camera motion keys 
	else
		{
		LWM->Frame = 0;
		Set_LWM(LWM, LWInfo, 0, Rend);
		if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
			{
			LW7Data[0][LW7Ct] = (float)(LWM->XYZ[0] * LWInfo->UnitScale);
			LW7Data[1][LW7Ct] = (float)(LWM->XYZ[1] * LWInfo->UnitScale);
			LW7Data[2][LW7Ct] = (float)(LWM->XYZ[2] * LWInfo->UnitScale);
			LW7Data[3][LW7Ct] = (float)(LWM->HPB[0] * PiOver180);
			LW7Data[4][LW7Ct] = (float)(LWM->HPB[1] * PiOver180);
			LW7Data[5][LW7Ct] = (float)(LWM->HPB[2] * PiOver180);
			LW7Data[6][LW7Ct] = (float)LWM->SCL[0];
			LW7Data[7][LW7Ct] = (float)LWM->SCL[1];
			LW7Data[8][LW7Ct] = (float)LWM->SCL[2];
			LW7Data[9][LW7Ct] = (float)LWM->TCB[0];
			LW7Data[10][LW7Ct] = (float)LWM->TCB[1];
			LW7Data[11][LW7Ct] = (float)LWM->TCB[2];
			LW7Data[12][LW7Ct] = (float)LWM->Frame;
			LW7Data[13][LW7Ct] = (float)LWM->Linear;
			LW7Ct ++;
			} // if
		else
			{
			if (fprintf(fLWM, "%f %f %f %f %f %f %f %f %f\n",
				LWM->XYZ[0] * LWInfo->UnitScale, LWM->XYZ[1] * LWInfo->UnitScale, LWM->XYZ[2] * LWInfo->UnitScale,
				LWM->HPB[0], LWM->HPB[1], LWM->HPB[2],
				LWM->SCL[0], LWM->SCL[1], LWM->SCL[2]) < 0)
				{
				error = 5;
				} // if write error 
			if (fprintf(fLWM, "%1d %1d %f %f %f\n",
				LWM->Frame, LWM->Linear,
				LWM->TCB[0], LWM->TCB[1], LWM->TCB[2]) < 0)
				{
				error = 5;
				} // if write error 
			} // else
		} // else no camera or bank key frames 
	} // if only output camera key frames (key frame int <= 0)

else
	{
	MaxFrames = GlobalApp->MCP->GetMaxFrame();
	MaxFrames = (MaxFrames > 0) ? MaxFrames: 1;
	LW7NumKeys = ((MaxFrames + LWInfo->KeyFrameInt) / LWInfo->KeyFrameInt) + ((MaxFrames + LWInfo->KeyFrameInt) % LWInfo->KeyFrameInt ? 1: 0);
	LW7Ct = 0;
	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{
		if (LW7Data[0] = (float *)AppMem_Alloc(WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float), APPMEM_CLEAR))
			{
			LW7Data[1] = &LW7Data[0][LW7NumKeys];
			LW7Data[2] = &LW7Data[0][LW7NumKeys * 2];
			LW7Data[3] = &LW7Data[0][LW7NumKeys * 3];
			LW7Data[4] = &LW7Data[0][LW7NumKeys * 4];
			LW7Data[5] = &LW7Data[0][LW7NumKeys * 5];
			LW7Data[6] = &LW7Data[0][LW7NumKeys * 6];
			LW7Data[7] = &LW7Data[0][LW7NumKeys * 7];
			LW7Data[8] = &LW7Data[0][LW7NumKeys * 8];
			LW7Data[9] = &LW7Data[0][LW7NumKeys * 9];
			LW7Data[10] = &LW7Data[0][LW7NumKeys * 10];
			LW7Data[11] = &LW7Data[0][LW7NumKeys * 11];
			LW7Data[12] = &LW7Data[0][LW7NumKeys * 12];
			LW7Data[13] = &LW7Data[0][LW7NumKeys * 13];
			} // if
		else
			{
			error = 3;
			goto EndWave;
			} // else
		fprintf(fLWM, "NumChannels %1d\n", Channels);
		} // if
	else
		{
		fprintf(fLWM, "%1d\n%1d\n", Channels, 1 + (MaxFrames % LWInfo->KeyFrameInt ? 1: 0) + MaxFrames / LWInfo->KeyFrameInt);
		} // else

	for (i = 0; i < MaxFrames + LWInfo->KeyFrameInt; i += LWInfo->KeyFrameInt)
		{
		if (i > MaxFrames)
			i = (short)MaxFrames;
		LWM->Frame = i;
		// init camera to this frame position
		if (Rend->Cam->TargetObj)
			Rend->Cam->TargetObj->SetToTime(LWM->Frame / Rend->FrameRate);
		Rend->Cam->SetToTime(LWM->Frame / Rend->FrameRate);
		Rend->Cam->InitFrameToRender(EffectsHost, Rend);
		// determine HPB
		Set_LWM(LWM, LWInfo, i, Rend);

		if (i != 0)
			{
			if (fabs(LWM->HPB[0] - Last[0]) > 180.0)
				{
				if (LWM->HPB[0] > Last[0])
					LWM->HPB[0] -= 360.0;
				else
					LWM->HPB[0] += 360.0;
				} // if
			if (fabs(LWM->HPB[1] - Last[1]) > 180.0)
				{
				if (LWM->HPB[1] > Last[1])
					LWM->HPB[1] -= 360.0;
				else
					LWM->HPB[1] += 360.0;
				} // if
			if (fabs(LWM->HPB[2] - Last[2]) > 180.0)
				{
				if (LWM->HPB[2] > Last[2])
					LWM->HPB[2] -= 360.0;
				else
					LWM->HPB[2] += 360.0;
				} // if
			} // if
		Last[0] = LWM->HPB[0];
		Last[1] = LWM->HPB[1];
		Last[2] = LWM->HPB[2];

		if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
			{
			LW7Data[0][LW7Ct] = (float)(LWM->XYZ[0] * LWInfo->UnitScale);
			LW7Data[1][LW7Ct] = (float)(LWM->XYZ[1] * LWInfo->UnitScale);
			LW7Data[2][LW7Ct] = (float)(LWM->XYZ[2] * LWInfo->UnitScale);
			LW7Data[3][LW7Ct] = (float)(LWM->HPB[0] * PiOver180);
			LW7Data[4][LW7Ct] = (float)(LWM->HPB[1] * PiOver180);
			LW7Data[5][LW7Ct] = (float)(LWM->HPB[2] * PiOver180);
			LW7Data[6][LW7Ct] = (float)LWM->SCL[0];
			LW7Data[7][LW7Ct] = (float)LWM->SCL[1];
			LW7Data[8][LW7Ct] = (float)LWM->SCL[2];
			LW7Data[9][LW7Ct] = (float)LWM->TCB[0];
			LW7Data[10][LW7Ct] = (float)LWM->TCB[1];
			LW7Data[11][LW7Ct] = (float)LWM->TCB[2];
			LW7Data[12][LW7Ct] = (float)LWM->Frame;
			LW7Data[13][LW7Ct] = (float)LWM->Linear;
			LW7Ct ++;
			} // if
		else
			{
			if (fprintf(fLWM, "%f %f %f %f %f %f %f %f %f\n",
				LWM->XYZ[0] * LWInfo->UnitScale, LWM->XYZ[1] * LWInfo->UnitScale, LWM->XYZ[2] * LWInfo->UnitScale,
				LWM->HPB[0], LWM->HPB[1], LWM->HPB[2],
				LWM->SCL[0], LWM->SCL[1], LWM->SCL[2]) < 0)
				{
				error = 5;
				break;
				} // if write error
			if (fprintf(fLWM, "%1d %1d %f %f %f\n",
				LWM->Frame, LWM->Linear,
				LWM->TCB[0], LWM->TCB[1], LWM->TCB[2]) < 0)
				{
				error = 5;
				break;
				} // if write error 
			} // else
		} // for i=0... 
	} // else

if(! error && ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7 && LW7Data[0])
	{
	// LW7-LWMO3 write 6 channels for cameras -- scale is a no-op
	for(int LWChan = 0; LWChan < Channels; LWChan++)
		{
		fprintf(fLWM, "Channel %d\n", LWChan);
		fprintf(fLWM, "{ Envelope\n");
		fprintf(fLWM, "  %d\n", LW7NumKeys);
		for (int LW7CtWrite = 0; LW7CtWrite < LW7Ct; LW7CtWrite ++)
			{
			// write out data considering the following definitions.
			// careful, these are all float values and may need casts.
			// as long as the frame and linear values are integers and within float precision limits 
			// it should be OK to store as float, otherwise do some fancy casting to store as long.
			//LW7Data[0][LW7Ct] = X
			//LW7Data[1][LW7Ct] = Y
			//LW7Data[2][LW7Ct] = Z
			//LW7Data[3][LW7Ct] = heading
			//LW7Data[4][LW7Ct] = pitch
			//LW7Data[5][LW7Ct] = bank
			//LW7Data[6][LW7Ct] = x scale
			//LW7Data[7][LW7Ct] = y scale
			//LW7Data[8][LW7Ct] = z scale
			//LW7Data[9][LW7Ct] = tension
			//LW7Data[10][LW7Ct] = continuity
			//LW7Data[11][LW7Ct] = bias
			//LW7Data[12][LW7Ct] = frame # / Rend->FrameRate
			//LW7Data[13][LW7Ct] = linear

			if(LW7Data[13][LW7CtWrite] != 0.0)
				{ // linear
				fprintf(fLWM, "  Key %g %g 3 0 0 0 0 0 0\n", LW7Data[LWChan][LW7CtWrite], LW7Data[12][LW7CtWrite] / Rend->FrameRate);
				} // if
			else
				{ // TCB spline
				fprintf(fLWM, "  Key %g %g 0 %g %g %g 0 0 0\n", LW7Data[LWChan][LW7CtWrite], LW7Data[12][LW7CtWrite] / Rend->FrameRate, LW7Data[9][LW7CtWrite], LW7Data[10][LW7CtWrite], LW7Data[11][LW7CtWrite]);
				} // else
			} // for
		fprintf(fLWM, "  Behaviors 1 1\n");
		fprintf(fLWM, "}\n");
		} // for
	} // if


EndWave:

if (fLWM && ! Supplied)
	fclose(fLWM);

if (LW7Data[0])
	AppMem_Free(LW7Data[0], WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float));

if (LWM)
	delete LWM;
if (! Supplied)
	{
	switch (error)
		{
		case 2:
			{
			UserMessageOK("LightWave Motion: Export",
				"No Key Frames to export!\nOperation terminated.");
			break;
			} // no key frames 
		case 3:
			{
			UserMessageOK("LightWave Motion: Export",
				"Out of memory!\nOperation terminated.");
			break;
			} // file write fail 
		case 4:
			{
			UserMessageOK("LightWave Motion: Export",
				"Error opening file for output!\nOperation terminated.");
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, LWInfo->Name);
			break;
			} // no memory 
		case 5:
			{
			UserMessageOK("LightWave Motion: Export",
				"Error writing to file!\nOperation terminated prematurely.");
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, LWInfo->Name);
			break;
			} // file open fail 
		} // switch 
	}// if file name not supplied - function called from LW scene file save 

return (error);

} // SceneExportGUI::ExportWave()

/*===========================================================================*/

int SceneExportGUI::Set_LWM(LightWaveMotion *LWM, ImportInfo *LWInfo,
	long Frame, RenderData *Rend)
{
double Heading, Pitch, Bank;
VertexDEM VP, FP, CV;

VP.CopyXYZ(Rend->Cam->CamPos);
FP.CopyXYZ(Rend->Cam->TargPos);
CV.CopyXYZ(Rend->Cam->CamVertical);
VP.RotateY(-LWInfo->RefLon);
FP.RotateY(-LWInfo->RefLon);
CV.RotateY(-LWInfo->RefLon);
FindPosVector(VP.XYZ, VP.XYZ, LWInfo->RefPt);
VP.RotateX(90.0 - LWInfo->RefLat);
FP.RotateX(90.0 - LWInfo->RefLat);
CV.RotateX(90.0 - LWInfo->RefLat);
//RotatePoint(VP.XYZ, LWInfo->RotMatx);
//RotatePoint(FP.XYZ, LWInfo->RotMatx);
//RotatePoint(CV.XYZ, LWInfo->RotMatx);

Heading = FP.FindAngleYfromZ();
FP.RotateY(-Heading);
Pitch = FP.FindAngleXfromZ();
CV.RotateY(-Heading);
CV.RotateX(-Pitch);
Bank = CV.FindAngleZfromY();

LWM->XYZ[0] = VP.XYZ[0];	/* X */
LWM->XYZ[1] = VP.XYZ[1];	/* Y */
LWM->XYZ[2] = VP.XYZ[2];	/* Z */

LWM->HPB[0] = Heading;				/* Heading */
LWM->HPB[1] = Pitch;				/* Pitch */
LWM->HPB[2] = Bank;					/* Bank */

LWM->SCL[0] = 1.0;			/* Scale X */
LWM->SCL[1] = 1.0;			/* Scale Y */
LWM->SCL[2] = 1.0;			/* Scale Z */

return (1);

} // SceneExportGUI::Set_LWM() 

/*===========================================================================*/

int SceneImportGUI::UnSet_LWM(ImportInfo *LWInfo, double *Value, VertexDEM *Vert, double &Heading, double &Pitch, double &Bank)
{
VertexDEM VP, FP, CV;

Vert->XYZ[0] = Value[0] * LWInfo->UnitScale;
Vert->XYZ[1] = Value[1] * LWInfo->UnitScale;
Vert->XYZ[2] = Value[2] * LWInfo->UnitScale;

RotatePoint(Vert->XYZ, LWInfo->RotMatx);
Vert->XYZ[0] += LWInfo->RefPt[0];
Vert->XYZ[1] += LWInfo->RefPt[1];
Vert->XYZ[2] += LWInfo->RefPt[2];

DefCoords->CartToDeg(Vert);

Vert->Lon += LWInfo->RefLon;

FP.SetDefaultViewVector();
CV.SetDefaultCamVerticalVector();

FP.RotateX(Value[4]);
FP.RotateY(Value[3]);
CV.RotateZ(Value[5]);
CV.RotateX(Value[4]);
CV.RotateY(Value[3]);

FP.RotateX(LWInfo->RefLat - 90.0);
FP.RotateY(LWInfo->RefLon);

FP.RotateY(-Vert->Lon);
FP.RotateX(90.0 - Vert->Lat);

Heading = FP.FindAngleYfromZ();
FP.RotateY(-Heading);
Pitch = FP.FindAngleXfromZ();

CV.RotateX(Vert->Lat - 90.0);
CV.RotateY(Vert->Lon);

CV.RotateY(-LWInfo->RefLon);
CV.RotateX(90.0 - LWInfo->RefLat);

CV.RotateY(-Heading);
CV.RotateX(-Pitch);
Bank = CV.FindAngleZfromY();

return (1);

} // SceneImportGUI::UnSet_LWM

/*===========================================================================*/

int SceneImportGUI::UnSet_LWMLight(ImportInfo *LWInfo, double *Value, VertexDEM *Vert)
{

Vert->XYZ[0] = Value[0] * LWInfo->UnitScale;
Vert->XYZ[1] = Value[1] * LWInfo->UnitScale;
Vert->XYZ[2] = Value[2] * LWInfo->UnitScale;

RotatePoint(Vert->XYZ, LWInfo->RotMatx);
Vert->XYZ[0] += LWInfo->RefPt[0];
Vert->XYZ[1] += LWInfo->RefPt[1];
Vert->XYZ[2] += LWInfo->RefPt[2];

DefCoords->CartToDeg(Vert);

Vert->Lon += LWInfo->RefLon;

return (1);

} // SceneImportGUI::UnSet_LWMLight

/*===========================================================================*/

int SceneExportGUI::LWOB_Export(Joe *JoeObj, char *OutputName, VertexDEM *PP,
	short SaveObject, double LonRot, double LatRot, ImportInfo *LWInfo, RenderData *Rend, int ExportAsVersion)
{
//double NClamp, SClamp, EClamp, WClamp;
char filename[256], DEMPath[256], DEMName[100], Ptrn[100], *ChunkTag;
FILE *fLWOB = NULL;
float *VertData = NULL;
short Done = 0, success = 0, ShortPad = 0;
long Polys, LWRows, LWCols, LWRowInt, LWColInt, FORMSize = 0,
	PNTSSize, SRFSSize, POLSSize, Size, zip, pzip, LWr, LWc, Lr, Lc, CurVtx,
	NumVertData, NumPolyData, PtCt;
unsigned long int ExportMaxVertices, Vertices, *PolyData = NULL, FormSeek = 0;
long SURFSize;
unsigned long SubChunkData;
unsigned short SizeS;
long i;
#ifdef BYTEORDER_LITTLEENDIAN
#endif // BYTEORDER_LITTLEENDIAN
double PivotPt[3], ptelev, DefSeaLevel;
LakeEffect *DefOcean;
VertexDEM Vert;
DEM Topo;

#ifdef WCS_VIEW_TERRAFFECTORS
VertexData VTD;
PolygonData PGD;
#endif // WCS_VIEW_TERRAFFECTORS


#ifdef WCS_VIEW_TERRAFFECTORS
// Setup for terraffector eval
//Rend.PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
//Rend.EarthLatScaleMeters = LatScale(Rend.PlanetRad);
//Rend.EffectsBase = GlobalApp->AppEffects;
//Rend.ElevDatum = VC->Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
//Rend.Exageration = VC->Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
//Rend.DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
Rend->TexRefLon = Rend->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
Rend->TexRefLat = Rend->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
Rend->TexRefElev = Rend->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z);
Rend->RefLonScaleMeters = Rend->EarthLatScaleMeters * cos(Rend->TexRefLat * PiOver180);
Rend->TexData.MetersPerDegLat = Rend->EarthLatScaleMeters;
Rend->TexData.Datum = Rend->ElevDatum;
Rend->TexData.Exageration = Rend->Exageration;
Rend->ExagerateElevLines = ((PlanetOpt *)Rend->EffectsBase->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))->EcoExageration;
//Rend.DBase = LocalDB;
#endif // WCS_VIEW_TERRAFFECTORS

// select DEM for export 
if (! JoeObj)
	{
	strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("elev"));
	strcpy(DEMPath, ProjHost->dirname);
	DEMName[0] = 0;
	if (! GetFileNamePtrn(0, "DEM file to export", DEMPath, DEMName, Ptrn, 100))
		{
		return (0);
		} /* if */
	strmfp(filename, DEMPath, DEMName);
	if(!Topo.LoadDEM(filename, 0, NULL))
		{
		UserMessageOK(DEMName,
			"Error loading DEM Object!\nOperation terminated.");
		goto EndExport;
		} // if open/read fails 
	BuildRotationMatrix(LatRot * PiOver180, 0.0, 0.0, LWInfo->RotMatx);
	} // if no object name supplied 
else
	{
	strcpy(DEMName, JoeObj->FileName());

	if (! Topo.AttemptLoadDEM(JoeObj, 0, ProjHost))
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_OPEN_FAIL, DEMName);
		UserMessageOK("DEMName",
			"Error loading DEM Object!\nObject not saved.");
		goto EndExport;
		} // if 
	} // else find the object and read it 

// I believe TransferToVerticesLatLon is safe to do in non-Coordinate-System-aware builds
// Ensure that the point remains within the DEM after whatever precision errors occur during coord transforms
// so that VertexDataPoint will find a valid elevation
Topo.TransferToVerticesLatLon(TRUE);

// prepare object for export 
LWRows = Topo.LonEntries();
LWCols = Topo.LatEntries();
Vertices = LWRows * LWCols;
Polys = (LWRows - 1) * (LWCols - 1) * 2;
LWColInt = LWRowInt = 1;

if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
	{
	ExportMaxVertices = WCS_EXPORT_LWO2_MAX_VERTICES;
	} // if
else
	{
	ExportMaxVertices = WCS_EXPORT_LWOB_MAX_VERTICES;
	} // else

if (Vertices > ExportMaxVertices || Vertices >= (unsigned)LWInfo->MaxVerts || Polys >= LWInfo->MaxPolys)
	Done = 0;
else
	Done = 1;

while (! Done && LWRows > 3 && LWCols > 3)
	{
	if (LWRows > LWCols)
		{
		LWRows = LWRows / 2 + LWRows % 2;
		LWRowInt *= 2;
		} // if 
	else
		{
		LWCols = LWCols / 2 + LWCols % 2;
		LWColInt *= 2;
		} // else  
	Vertices = LWRows * LWCols;
	Polys = (LWRows - 1) * (LWCols - 1) * 2;
	if (Vertices < ExportMaxVertices && Vertices <= (unsigned)LWInfo->MaxVerts && Polys <= LWInfo->MaxPolys)
		Done = 1;
	} // while need to decimate DEM points 

// allocate vertex array 
NumVertData = Vertices * 3;
NumPolyData = Polys * 5;
PNTSSize = NumVertData * 4;
POLSSize = NumPolyData * 2;
SRFSSize = 8;

SURFSize = 52;
FORMSize = PNTSSize + SRFSSize + POLSSize +SURFSize + 4 + 8 + 8 + 8;


VertData = (float *)AppMem_Alloc(PNTSSize, 0);
PolyData = (unsigned long *)AppMem_Alloc(POLSSize * 2, 0);

if (! VertData || ! PolyData)
	{
	goto EndExport;
	} /* if */

if (DefOcean = (LakeEffect *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_LAKE, 0, NULL))
	{
	DefSeaLevel = DefOcean->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].CurValue;
	if (Rend->ExagerateElevLines)
		DefSeaLevel = (DefSeaLevel - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
	} // if
else
	DefSeaLevel = - FLT_MAX;

ptelev = Topo.SampleRaw(0) * Topo.ElScale() / ELSCALE_METERS;
ptelev = (ptelev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
if (! LWInfo->Bathymetry && ptelev <= DefSeaLevel)
	ptelev = DefSeaLevel;
Vert.Elev = ptelev;
Vert.Lat = Topo.Vertices[0].Lat;
Vert.Lon = Topo.Vertices[0].Lon + LonRot;
Rend->DefCoords->DegToCart(&Vert);
PivotPt[0] = Vert.XYZ[0];
PivotPt[1] = Vert.XYZ[1];
PivotPt[2] = Vert.XYZ[2];
if (PP)
	{
	PP->CopyXYZ(&Vert);
	} // if VertexDEM provided

if (! SaveObject)
	{
	success = 1;
	goto EndExport;
	} // if we just needed the pivot point coords 

// precision errors cause FindAndLoadDEM to flush too often
// This problem is believed fixed by ensuring the vertices fall within the DEM bounds in TransferToVerticesLatLon()
//NClamp = JoeObj->NWLat;
//SClamp = JoeObj->SELat;
//EClamp = JoeObj->SELon;
//WClamp = JoeObj->NWLon;

for (zip = pzip = LWr = Lr = 0; LWr < LWRows; Lr += LWRowInt, LWr ++)
	{
	PtCt = Lr * Topo.LatEntries();
	for (LWc = Lc = 0; LWc < LWCols; Lc += LWColInt, LWc ++, PtCt += LWColInt)
		{
		Vert.Lat = Topo.Vertices[PtCt].Lat;
		Vert.Lon = Topo.Vertices[PtCt].Lon + LonRot;
		//Vert.Lat = Topo.Southest() + Lc * Topo.LatStep();
		//Vert.Lon = Topo.Westest() - Lr * Topo.LonStep() + LonRot;

		#ifdef WCS_VIEW_TERRAFFECTORS
		VTD.Lat = Vert.Lat;
		VTD.Lon = Topo.Vertices[PtCt].Lon;
		//if (VTD.Lat < SClamp)
		//	VTD.Lat = SClamp;
		//if (VTD.Lat > NClamp)
		//	VTD.Lat = NClamp;
		//if (VTD.Lon < EClamp)
		//	VTD.Lon = EClamp;
		//if (VTD.Lon > WClamp)
		//	VTD.Lon = WClamp;
		//VTD.Lon = Topo.Westest() - Lr * Topo.LonStep();
		GlobalApp->MainProj->Interactive->VertexDataPoint(Rend, &VTD, &PGD,
		 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
		if(VTD.Elev > 1000000.0) // sanity check
			{
			VTD.Elev = 0;
			} // if
		ptelev = VTD.Elev;
		#else // !WCS_VIEW_TERRAFFECTORS
		ptelev = Topo.Vertices[PtCt].Elev * Topo.ElScale() / ELSCALE_METERS;
		ptelev = (ptelev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
		#endif // !WCS_VIEW_TERRAFFECTORS


		if (! LWInfo->Bathymetry && ptelev <= DefSeaLevel)
			ptelev = DefSeaLevel;
		Vert.Elev = ptelev;
		Rend->DefCoords->DegToCart(&Vert);

		FindPosVector(Vert.XYZ, Vert.XYZ, PivotPt);
		RotatePoint(Vert.XYZ, LWInfo->RotMatx);

		VertData[zip] = (float)(Vert.XYZ[0] * LWInfo->UnitScale);
		zip ++;
		VertData[zip] = (float)(Vert.XYZ[1] * LWInfo->UnitScale);
		zip ++;
		VertData[zip] = (float)(Vert.XYZ[2] * LWInfo->UnitScale);
		zip ++;

		if (LWr < LWRows - 1 && LWc < LWCols - 1)
			{
			CurVtx = LWr * LWCols + LWc;
			PolyData[pzip] = 3;
			pzip ++;
			PolyData[pzip] = (CurVtx);
			pzip ++;
			PolyData[pzip] = (CurVtx + 1);
			pzip ++;
			PolyData[pzip] = (CurVtx + LWCols);
			pzip ++;
			PolyData[pzip] = 1;
			pzip ++;
	
			PolyData[pzip] = 3;
			pzip ++;
			PolyData[pzip] = (CurVtx + 1);
			pzip ++;
			PolyData[pzip] = (CurVtx + 1 + LWCols);
			pzip ++;
			PolyData[pzip] = (CurVtx + LWCols);
			pzip ++;
			PolyData[pzip] = 1;
			pzip ++;
			} // if 
		} // for LWc=0... 
	} // for LWr=0... 

// get output file name 
if (! OutputName)
	{
	if (! strcmp(&DEMName[strlen(DEMName) - 5], ".elev"))
		DEMName[strlen(DEMName) - 5] = 0;
	strcat(DEMName, ".LWO");
	if (ProjHost->altobjectpath[0] == 0)
		strcpy(ProjHost->altobjectpath, "WCSProjects:");
	strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("LWO"));
	if (! GetFileNamePtrn(1, "LW Object path/file", ProjHost->altobjectpath, DEMName, Ptrn, 100))
		{
		goto EndExport;
		} // if 
	strmfp(filename, ProjHost->altobjectpath, DEMName);
	} // if no output name supplied 
else
	strcpy(filename, OutputName);

// open file 
if ((fLWOB = PROJ_fopen(filename, "wb")) == NULL)
	{
	goto EndExport;
	} // if 


// write "FORM" and temporary size 
ChunkTag = "FORM";
fwrite((char *)ChunkTag, 4, 1, fLWOB);
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32S(FORMSize, &Size);
#else
Size = FORMSize;
#endif // BYTEORDER_LITTLEENDIAN
FormSeek = ftell(fLWOB); // in case we need to come back later to rewrite FORMSize (as in LWO2)
fwrite((char *)&Size, 4, 1, fLWOB);

if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
	{
	ChunkTag = "LWO2";
	} // if
else
	{
	ChunkTag = "LWOB";
	} // else

fwrite((char *)ChunkTag, 4, 1, fLWOB);


if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
	{
	int TAGSSize;
	float FPad = 0.0f;
	char *TAGSWCSDEM = "WCSDEM\0\0"; // only tag (surface) we write (with NULL and extra pad NULL)
	// write TAGS (WCSDEM) and LAYR
	ChunkTag = "TAGS";
	fwrite((char *)ChunkTag, 4, 1, fLWOB);

	TAGSSize = (int)(strlen(TAGSWCSDEM) + 1 + 1); // for two NULLs

	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(TAGSSize, &Size);
	#else
	Size = TAGSSize;
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&Size, 4, 1, fLWOB);

	ChunkTag = TAGSWCSDEM;
	fwrite((char *)ChunkTag, 8, 1, fLWOB);


	// Layer info
	ChunkTag = "LAYR";
	fwrite((char *)ChunkTag, 4, 1, fLWOB);

	TAGSSize = 18; // Layer size, actually
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(TAGSSize, &Size);
	#else
	Size = TAGSSize;
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&Size, 4, 1, fLWOB);

	// LayerNum = 0
	fwrite((char *)&ShortPad, 2, 1, fLWOB);
	// Flags = 0
	fwrite((char *)&ShortPad, 2, 1, fLWOB);
	// Pivot (4 floats, not endian-flipped because they're all 0)
	fwrite((char *)&FPad, 4, 1, fLWOB);
	fwrite((char *)&FPad, 4, 1, fLWOB);
	fwrite((char *)&FPad, 4, 1, fLWOB);

	// Name ([S0]NULL, plus NULL pad byte
	fwrite((char *)&ShortPad, 2, 1, fLWOB);
	// No parent field
	} // if

ChunkTag = "PNTS";
fwrite((char *)ChunkTag, 4, 1, fLWOB);

// PNTS is unchanged, except that you can have more than 2^16 points
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32S(PNTSSize, &Size);
#else
Size = PNTSSize;
#endif // BYTEORDER_LITTLEENDIAN
fwrite((char *)&Size, 4, 1, fLWOB);

// flip VertexData
#ifdef BYTEORDER_LITTLEENDIAN
for (i=0; i<NumVertData; i++)
	{
	SimpleEndianFlip32F(&VertData[i], &VertData[i]);
	} // for
#endif // BYTEORDER_LITTLEENDIAN
// write the points array to the file 
fwrite((char *)VertData, PNTSSize, 1, fLWOB);

// we'll skip BBOX, as it is optional

// we'll write LW7 surface info (as PTAGs) later
if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_3)
	{
	// write "SRFS" 
	ChunkTag = "SRFS";
	fwrite((char *)ChunkTag, 4, 1, fLWOB);
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(SRFSSize, &Size);
	#else
	Size = SRFSSize;
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&Size, 4, 1, fLWOB);

	// write "WCSDEM" + 0 + 0 
	ChunkTag = "WCSDEM";
	fwrite((char *)ChunkTag, 6, 1, fLWOB);
	fwrite((char *)&ShortPad, 2, 1, fLWOB);
	} // if


if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
	{ // more complex LW7/VX polygon writing code here
	unsigned long LWO2POLSSize = 0, polyzip;
	// write "POLS" 
	ChunkTag = "POLS";
	fwrite((char *)ChunkTag, 4, 1, fLWOB);
	// size is trickier now
	LWO2POLSSize += 4; // type field: FACE

	for (polyzip = i = 0; i<Polys; i++)
		{
		LWO2POLSSize += 2; // numvert/flags
		polyzip++; // numvert/flags

		// three vertices
		// v1
		if(PolyData[polyzip] < 0xff00)
			{
			LWO2POLSSize += 2;
			} // if
		else
			{
			LWO2POLSSize += 4;
			} // else
		polyzip++;

		// v2
		if(PolyData[polyzip] < 0xff00)
			{
			LWO2POLSSize += 2;
			} // if
		else
			{
			LWO2POLSSize += 4;
			} // else
		polyzip++;

		// v3
		if(PolyData[polyzip] < 0xff00)
			{
			LWO2POLSSize += 2;
			} // if
		else
			{
			LWO2POLSSize += 4;
			} // else
		polyzip++;

		polyzip++; // skip LWOB surface ID

		} // for

	// write POLS size field
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(LWO2POLSSize, &Size);
	#else
	Size = LWO2POLSSize;
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&Size, 4, 1, fLWOB);

	// write "FACE" 
	ChunkTag = "FACE";
	fwrite((char *)ChunkTag, 4, 1, fLWOB);

	for (polyzip = i = 0; i<Polys; i++)
		{
		unsigned long PolyWrite;
		unsigned short PolyWrite16;

		PolyWrite16 = (unsigned short)PolyData[polyzip];
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
		#endif // !BYTEORDER_LITTLEENDIAN
	
		// write out numvert/flags
		if (fwrite((char *)&PolyWrite16, 2, 1, fLWOB) == 1)
			success = 1;
		polyzip++; // numvert/flags

		// three vertices
		// v1
		if(PolyData[polyzip] < 0xff00)
			{
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U((unsigned short)PolyData[polyzip], &PolyWrite16);
			#else // !BYTEORDER_LITTLEENDIAN
			PolyWrite16 = (unsigned short)PolyData[polyzip];
			#endif // !BYTEORDER_LITTLEENDIAN
		
			// write out numvert/flags
			if (fwrite((char *)&PolyWrite16, 2, 1, fLWOB) == 1)
				success = 1;
			} // if
		else
			{
			unsigned long int PolyTemp;
			PolyTemp = PolyData[polyzip];
			PolyTemp |= 0xff000000; // set high byte to ff to signal 4-byte entity
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip32U(PolyTemp, &PolyWrite);
			#else // !BYTEORDER_LITTLEENDIAN
			PolyWrite = PolyTemp;
			#endif // !BYTEORDER_LITTLEENDIAN
		
			// write out numvert/flags
			if (fwrite((char *)&PolyWrite, 4, 1, fLWOB) == 1)
				success = 1;
			} // else
		polyzip++;

		// v2
		if(PolyData[polyzip] < 0xff00)
			{
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U((unsigned short)PolyData[polyzip], &PolyWrite16);
			#else // !BYTEORDER_LITTLEENDIAN
			PolyWrite16 = (unsigned short)PolyData[polyzip];
			#endif // !BYTEORDER_LITTLEENDIAN
		
			// write out numvert/flags
			if (fwrite((char *)&PolyWrite16, 2, 1, fLWOB) == 1)
				success = 1;
			} // if
		else
			{
			unsigned long int PolyTemp;
			PolyTemp = PolyData[polyzip];
			PolyTemp |= 0xff000000; // set high byte to ff to signal 4-byte entity
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip32U(PolyTemp, &PolyWrite);
			#else // !BYTEORDER_LITTLEENDIAN
			PolyWrite = PolyTemp;
			#endif // !BYTEORDER_LITTLEENDIAN
		
			// write out numvert/flags
			if (fwrite((char *)&PolyWrite, 4, 1, fLWOB) == 1)
				success = 1;
			} // else
		polyzip++;

		// v3
		if(PolyData[polyzip] < 0xff00)
			{
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U((unsigned short)PolyData[polyzip], &PolyWrite16);
			#else // !BYTEORDER_LITTLEENDIAN
			PolyWrite16 = (unsigned short)PolyData[polyzip];
			#endif // !BYTEORDER_LITTLEENDIAN
		
			// write out numvert/flags
			if (fwrite((char *)&PolyWrite16, 2, 1, fLWOB) == 1)
				success = 1;
			} // if
		else
			{
			unsigned long int PolyTemp;
			PolyTemp = PolyData[polyzip];
			PolyTemp |= 0xff000000; // set high byte to ff to signal 4-byte entity
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip32U(PolyTemp, &PolyWrite);
			#else // !BYTEORDER_LITTLEENDIAN
			PolyWrite = PolyTemp;
			#endif // !BYTEORDER_LITTLEENDIAN
		
			// write out numvert/flags
			if (fwrite((char *)&PolyWrite, 4, 1, fLWOB) == 1)
				success = 1;
			} // else
		polyzip++;

		polyzip++; // skip LWOB surface ID
		} // for

	// write PTAG data
	ChunkTag = "PTAG";
	fwrite((char *)ChunkTag, 4, 1, fLWOB);

	Size = 4; // SURF + ...
	// add up ptag VXes
	for (i = 0; i<Polys; i++)
		{
		if(i < 0xff00)
			{
			Size += 4; // VX[2] + U2
			} // if
		else
			{
			Size += 6; // VX[4] + U2
			} // else
		} // if
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(Size, &Size);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&Size, 4, 1, fLWOB);

	// write SURF
	ChunkTag = "SURF";
	fwrite((char *)ChunkTag, 4, 1, fLWOB);

	// write ptags
	for (i = 0; i<Polys; i++)
		{
		unsigned long PolyWrite;
		unsigned short PolyWrite16;
		if(i < 0xff00)
			{
			PolyWrite16 = (unsigned short)i;
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite((char *)&PolyWrite16, 2, 1, fLWOB);
			} // if
		else
			{
			PolyWrite = i;
			PolyWrite |= 0xff000000;
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip32U(PolyWrite, &PolyWrite);
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite((char *)&PolyWrite, 4, 1, fLWOB);
			} // else
		// write tag value (0 always for surface 0)
		fwrite((char *)&ShortPad, 2, 1, fLWOB); // 16-bit zero
		} // if

	} // if
else
	{
	unsigned short PolyWrite;
	// write "POLS" 
	ChunkTag = "POLS";
	fwrite((char *)ChunkTag, 4, 1, fLWOB);
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(POLSSize, &Size);
	#else
	Size = POLSSize;
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&Size, 4, 1, fLWOB);

	#ifdef BYTEORDER_LITTLEENDIAN
	for (i=0; i<NumPolyData; i++)
		{
		SimpleEndianFlip16U((unsigned short)PolyData[i], &PolyWrite);
		// write out polygon data (not blocked)
		if (fwrite((char *)&PolyWrite, 2, 1, fLWOB) == 1)
			success = 1;
		} // for
	#else // !BYTEORDER_LITTLEENDIAN
	for (i=0; i<NumPolyData; i++)
		{
		PolyWrite = (unsigned short)PolyData[i];
		// write out polygon data (not blocked)
		if (fwrite((char *)&PolyWrite, 2, 1, fLWOB) == 1)
			success = 1;
		} // for
	#endif // !BYTEORDER_LITTLEENDIAN
	} // if


// write "SURF" 
ChunkTag = "SURF";
fwrite((char *)ChunkTag, 4, 1, fLWOB);

if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
	{
	SURFSize = 42; // size without SURF and Size, 50 with
	} // if
else
	{
	SURFSize = 44; // size without SURF and Size, 52 with
	} // else
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32S(SURFSize, &Size);
#else
Size = SURFSize;
#endif // BYTEORDER_LITTLEENDIAN
fwrite((char *)&Size, 4, 1, fLWOB);

// write SURFace name
ChunkTag = "WCSDEM";
fwrite((char *)ChunkTag, 6, 1, fLWOB);
fwrite((char *)&ShortPad, 2, 1, fLWOB); // ending NULL, plus pad NULL

if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
	{ // write surface source (null) name
	fwrite((char *)&ShortPad, 2, 1, fLWOB); // ending NULL, plus pad NULL
	} // if


// Subchunk COLR
ChunkTag = "COLR";
fwrite((char *)ChunkTag, 4, 1, fLWOB);
// Subchunk 16-bit size
if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
	{
	float SubChunkFloat;
	SizeS = 14;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(SizeS, &SizeS);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SizeS, 2, 1, fLWOB);
	// Color Data
	SubChunkFloat = (float)(0xc8) / 255.0f;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32F(&SubChunkFloat, (float *)&SubChunkData);
	#endif // BYTEORDER_LITTLEENDIAN
	// write three identical color channels
	fwrite((char *)&SubChunkData, 4, 1, fLWOB);
	fwrite((char *)&SubChunkData, 4, 1, fLWOB);
	fwrite((char *)&SubChunkData, 4, 1, fLWOB);
	// write a VX envelope ID of 0 (with a pad byte)
	fwrite((char *)&ShortPad, 2, 1, fLWOB);
	} // if
else
	{
	SizeS = 4;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(SizeS, &SizeS);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SizeS, 2, 1, fLWOB);
	// Color Data
	SubChunkData = 0xC8C8C800;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32U(SubChunkData, &SubChunkData);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SubChunkData, 4, 1, fLWOB);

	// Subchunk FLAG
	ChunkTag = "FLAG";
	fwrite((char *)ChunkTag, 4, 1, fLWOB);
	// Subchunk 16-bit size
	SizeS = 2;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(SizeS, &SizeS);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SizeS, 2, 1, fLWOB);
	// Flag Data
	fwrite((char *)&ShortPad, 2, 1, fLWOB);
	} // else


// Subchunk DIFF
ChunkTag = "DIFF";
fwrite((char *)ChunkTag, 4, 1, fLWOB);

if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
	{ // LWO2 does it in FP
	float SubChunkFloat;
	// Subchunk 16-bit size
	SizeS = 6;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(SizeS, &SizeS);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SizeS, 2, 1, fLWOB);
	// DIFFuse Data

	SubChunkFloat = 1.0f;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32F(&SubChunkFloat, (float *)&SubChunkData);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SubChunkData, 4, 1, fLWOB);
	// write a VX envelope ID of 0 (with a pad byte)
	fwrite((char *)&ShortPad, 2, 1, fLWOB);
	} // if
else
	{
	// Subchunk 16-bit size
	SizeS = 2;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(SizeS, &SizeS);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SizeS, 2, 1, fLWOB);
	// DIFFuse Data

	SizeS = 0x0100;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(SizeS, &SizeS);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SizeS, 2, 1, fLWOB);
	} // else

// LWO2 does not have VDIF chunk
if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_3)
	{
	// Subchunk VDIF
	ChunkTag = "VDIF";
	fwrite((char *)ChunkTag, 4, 1, fLWOB);
	// Subchunk 16-bit size
	SizeS = 4;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(SizeS, &SizeS);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SizeS, 2, 1, fLWOB);
	// VDIF Data

	SubChunkData = 0x3F800000;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32U(SubChunkData, &SubChunkData);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SubChunkData, 4, 1, fLWOB);
	} // if


if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
	{
	FORMSize = ftell(fLWOB);
	FORMSize -= 8; // subtract FORM and FormSize
	fseek(fLWOB, FormSeek, SEEK_SET);
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(FORMSize, &Size);
	#else
	Size = FORMSize;
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&Size, 4, 1, fLWOB);
	} // if


EndExport:
if (fLWOB)
	fclose(fLWOB);
Topo.FreeRawElevs();		// don't need to free map.map
if (VertData)
	AppMem_Free(VertData, PNTSSize);
if (PolyData)
	AppMem_Free(PolyData, POLSSize * 2);

return (success);

} // SceneExportGUI::LWOB_Export() 

/*===========================================================================*/

int CheckTCBNormalRange(double T, double C, double B)
{
int RangeViolations = 0;

if((T > 1.0) || (T < -1.0))
	RangeViolations++;
if((C > 1.0) || (C < -1.0))
	RangeViolations++;
if((B > 1.0) || (B < -1.0))
	RangeViolations++;

return(RangeViolations);
} // CheckTCBNormalRange

void SceneExportGUI::LWScene_Export(ImportInfo *LWInfo, RenderData *Rend, int ExportAsVersion)
{
char filename[256], ScenePath[256], SceneFile[100], DEMPath[256], DEMName[100], Ptrn[100];
unsigned char Red, Green, Blue;
double DRed, DGreen, DBlue;
int LightTCBRangeWarn = 0, CamTCBRangeWarn = 0, HazeTCBRangeWarn = 0;
FILE *fScene;
long i, StartFile, KeyFrames, length, error = 0, /* MaxHt, */ NotFound = 0, LastKeyFrame, NextKeyFrame, LWChan;
double value;
VertexDEM Vert;
Joe *MyTurn;
VectorPoint *Point;
GraphNode *GrNode;
Atmosphere *DefAtmo;
Sky *DefSky;
Light *CurLight;
RasterAnimHostProperties Prop;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
float *LW7Data[WCS_LW7_DATA_NUMCHANNELS];
long LW7NumKeys, LW7Ct;

// init to NULL
LW7Data[0] = NULL;

// check center X and Center Y 

if (Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].CurValue != .5 ||
	Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].CurValue != .5)
	{
	if (! UserMessageOKCAN("Scene Export", "Center X and/or Center Y Camera Parameters\nare not in the center of the image!\nThis offset will not be reflected in the exported scene.", 1))
		{
		return;
		} // if cancel
	} // if

// scene file 

if (LWInfo->Path[0] == 0)
	{
	if (ProjHost->altobjectpath[0] == 0)
		strcpy(ProjHost->altobjectpath, "WCSProjects:");
	strcpy(ScenePath, ProjHost->altobjectpath);
	} // if 
else
	{
	strcpy(ScenePath, LWInfo->Path);
	} // else
strcpy(SceneFile, ProjHost->projectname);
if (! strcmp(&SceneFile[strlen(SceneFile) - 5], ".proj"))
	SceneFile[strlen(SceneFile) - 5] = 0;
strcat(SceneFile, ".LWS");
strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("LWS"));
if (! GetFileNamePtrn(1, "Scene Path/File", ScenePath, SceneFile, Ptrn, 100))
	{
	return;
	} // if 
strmfp(filename, ScenePath, SceneFile);

StartFile = 0;
length = (long)(strlen(ScenePath) - 6);
while (StartFile <= length)
	{
	if (strnicmp(&ScenePath[StartFile], "Scenes", 6))
		StartFile ++;
	else
		break;
	} // while
if (StartFile <= length)
	{
	ScenePath[StartFile] = 0;
	if (ScenePath[StartFile - 1] == '/' || ScenePath[StartFile - 1] == '\\')
		ScenePath[StartFile - 1] = 0;
	}
strcpy(LWInfo->Path, ScenePath);


// save objects 

// dem path 
strcpy(DEMPath, ScenePath);
#ifdef _WIN32
strcat(DEMPath, "\\Objects");
#else // !_WIN32: Unix
strcat(DEMPath, "/Objects");
#endif // !_WIN32: Unix

FileReq FR;
FR.SetDefPath(DEMPath);
FR.SetTitle("LightWave DEM Object Path");
if (FR.Request(WCS_REQUESTER_FILE_DIRONLY | WCS_REQUESTER_FILE_NOMUNGE))
	{
	DEMPath[0] = 0;
	DEMName[0] = 0;
	BreakFileName((char *)FR.GetNextName(), DEMPath, 255, DEMName, 100);
	} // if
else
	{
	return;
	} // else

length = (long)strlen(ProjHost->altobjectpath);
if (length > 0 && ! strnicmp(ProjHost->altobjectpath, DEMPath, length))
	{
	StartFile = length;
	while (DEMPath[StartFile] == ':' || DEMPath[StartFile] == '\\' || DEMPath[StartFile] == '/')
		StartFile ++;
	} // if

// figure out where the reference point is

SetupForExport(LWInfo, Rend);

// save objects 

if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_3)
	{
	LWInfo->MaxPolys = min(LWInfo->MaxPolys, WCS_EXPORT_LWOB_MAX_POLYGONS);
	LWInfo->MaxVerts = min(LWInfo->MaxVerts, WCS_EXPORT_LWOB_MAX_VERTICES);
	} // if
else
	{ // WCS_EXPORT_FORMAT_LIGHTWAVE_7: LWO2
	LWInfo->MaxPolys = min(LWInfo->MaxPolys, WCS_EXPORT_LWO2_MAX_POLYGONS);
	LWInfo->MaxVerts = min(LWInfo->MaxVerts, WCS_EXPORT_LWO2_MAX_VERTICES);
	} // else

if (LWInfo->MaxPolys < 2 || LWInfo->MaxVerts < 4)
	return;

if (fScene = PROJ_fopen(filename, "w"))
	{
	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_3)
		{
		fprintf(fScene, "%s\n%d\n\n", "LWSC", 1);
		} // if
	else
		{ // WCS_EXPORT_FORMAT_LIGHTWAVE_7
		fprintf(fScene, "%s\n%d\n\n", "LWSC", 3);
		} // else

	fprintf(fScene, "%s %d\n", "FirstFrame", (long)(Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue * Rend->FrameRate + .5));
	fprintf(fScene, "%s %d\n", "LastFrame", (long)(Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue * Rend->FrameRate + .5));
	fprintf(fScene, "%s %d\n\n", "FrameStep", Rend->Opt->FrameStep);
	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{
		fprintf(fScene, "%s %d\n", "PreviewFirstFrame", (long)(Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue * Rend->FrameRate + .5));
		fprintf(fScene, "%s %d\n", "PreviewLastFrame", (long)(Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue * Rend->FrameRate + .5));
		fprintf(fScene, "%s %d\n\n", "PreviewFrameStep", Rend->Opt->FrameStep);
		fprintf(fScene, "%s %d\n\n", "CurrentFrame", 0);
		} // if

	fprintf(fScene, "%s %f\n\n", "FramesPerSecond", Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue);

//###################### Parent Null

	fprintf(fScene, "AddNullObject WCSMasterNull\n");

	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{
		fprintf(fScene, "ShowObject 6 3\n"); // gunmetal blue
		fprintf(fScene, "%s\nNumChannels %d\n", "ObjectMotion", 9);

		// write 9 non-animated channels for master null
		for(LWChan = 0; LWChan < 9; LWChan++)
			{
			fprintf(fScene, "Channel %d\n", LWChan);
			fprintf(fScene, "{ Envelope\n");
			fprintf(fScene, "  1\n");
			fprintf(fScene, "  Key %d 0 0 0 0 0 0 0 0\n", LWChan > 5 ? 1 : 0); // channels 0-5: 0, channels 6-8 (scaling): 1
			fprintf(fScene, "  Behaviors 1 1\n");
			fprintf(fScene, "}\n");
			} // for
		} // if
	else
		{ // WCS_EXPORT_FORMAT_LIGHTWAVE_3
		fprintf(fScene, "%s\n%d\n%d\n", "ObjectMotion (unnamed)", 9, 1);
		fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
			0.0, 0.0, 0.0, 0.0,
			0.0,
			0.0, 1.0, 1.0, 1.0);
		fprintf(fScene, "%d %d %1.1f %1.1f %1.1f\n", 0, 0, 0.0, 0.0, 0.0);
		fprintf(fScene, "%s %d\n", "EndBehavior", 1);
		} // else
	fprintf(fScene, "%s %d\n\n", "ShadowOptions", 7);

//###################### Focus Null

	if (Rend->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
		{
		fprintf(fScene, "AddNullObject WCSCameraTargetNull\n");

		if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
			{
			fprintf(fScene, "ShowObject 6 9\n"); // camera targets are light green
			} // if


		if (Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetNumNodes(0) > 0
			|| Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetNumNodes(0) > 0
			|| Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetNumNodes(0) > 0)
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
			Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;
			LastKeyFrame = -2;
			KeyFrames = 0;

			while (1)	//lint !e716
				{
				NextKeyFrame = -1;
				Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
				Prop.NewKeyNodeRange[0] = -DBL_MAX;
				Prop.NewKeyNodeRange[1] = DBL_MAX;
				Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetRAHostProperties(&Prop);
				if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
					NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
				if (NextKeyFrame > LastKeyFrame)
					{
					KeyFrames ++;
					LastKeyFrame = NextKeyFrame;
					} // if
				else
					break;
				} // while

			LastKeyFrame = -2;
			if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
				{
				LW7NumKeys = (KeyFrames > 0) ? KeyFrames: 1;
				LW7Ct = 0;
				if (LW7Data[0] = (float *)AppMem_Alloc(WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float), APPMEM_CLEAR))
					{
					LW7Data[1] = &LW7Data[0][LW7NumKeys];
					LW7Data[2] = &LW7Data[0][LW7NumKeys * 2];
					LW7Data[3] = &LW7Data[0][LW7NumKeys * 3];
					LW7Data[4] = &LW7Data[0][LW7NumKeys * 4];
					LW7Data[5] = &LW7Data[0][LW7NumKeys * 5];
					LW7Data[6] = &LW7Data[0][LW7NumKeys * 6];
					LW7Data[7] = &LW7Data[0][LW7NumKeys * 7];
					LW7Data[8] = &LW7Data[0][LW7NumKeys * 8];
					LW7Data[9] = &LW7Data[0][LW7NumKeys * 9];
					LW7Data[10] = &LW7Data[0][LW7NumKeys * 10];
					LW7Data[11] = &LW7Data[0][LW7NumKeys * 11];
					LW7Data[12] = &LW7Data[0][LW7NumKeys * 12];
					LW7Data[13] = &LW7Data[0][LW7NumKeys * 13];
					} // if
				fprintf(fScene, "%s\nNumChannels %d\n", "ObjectMotion", 9);

				if (LW7Data[0])
					{
					while (1)	//lint !e716
						{
						NextKeyFrame = -1;
						Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
						Prop.NewKeyNodeRange[0] = -DBL_MAX;
						Prop.NewKeyNodeRange[1] = DBL_MAX;
						Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetRAHostProperties(&Prop);
						if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
							NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
						if (NextKeyFrame > LastKeyFrame)
							{
							if (GrNode = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
								{
								LW7Data[0][LW7Ct] = (float)(Vert.XYZ[0] * LWInfo->UnitScale);
								LW7Data[1][LW7Ct] = (float)(Vert.XYZ[1] * LWInfo->UnitScale);
								LW7Data[2][LW7Ct] = (float)(Vert.XYZ[2] * LWInfo->UnitScale);
								LW7Data[3][LW7Ct] = 0.0f;
								LW7Data[4][LW7Ct] = 0.0f;
								LW7Data[5][LW7Ct] = 0.0f;
								LW7Data[6][LW7Ct] = 1.0f;
								LW7Data[7][LW7Ct] = 1.0f;
								LW7Data[8][LW7Ct] = 1.0f;
								LW7Data[9][LW7Ct] = (float)GrNode->TCB[0];
								LW7Data[10][LW7Ct] = (float)GrNode->TCB[1];
								LW7Data[11][LW7Ct] = (float)GrNode->TCB[2];
								LW7Data[12][LW7Ct] = (float)NextKeyFrame;
								LW7Data[13][LW7Ct] = (float)GrNode->Linear;
								LW7Ct ++;
								CamTCBRangeWarn += CheckTCBNormalRange(GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);

								LastKeyFrame = NextKeyFrame;
								} // if
							else
								break;
							} // if
						else
							break;
						} // while

					// write channels
					for(LWChan = 0; LWChan < 9; LWChan++)
						{
						fprintf(fScene, "Channel %d\n", LWChan);
						fprintf(fScene, "{ Envelope\n");
						fprintf(fScene, "  %d\n", LW7NumKeys);
						for (int LW7CtWrite = 0; LW7CtWrite < LW7Ct; LW7CtWrite ++)
							{
							if(LW7Data[13][LW7CtWrite] != 0.0)
								{ // linear
								fprintf(fScene, "  Key %g %g 3 0 0 0 0 0 0\n", LW7Data[LWChan][LW7CtWrite], LW7Data[12][LW7CtWrite] / Rend->FrameRate);
								} // if
							else
								{ // TCB spline
								fprintf(fScene, "  Key %g %g 0 %g %g %g 0 0 0\n", LW7Data[LWChan][LW7CtWrite], LW7Data[12][LW7CtWrite] / Rend->FrameRate, LW7Data[9][LW7CtWrite], LW7Data[10][LW7CtWrite], LW7Data[11][LW7CtWrite]);
								} // else
							} // for
						fprintf(fScene, "  Behaviors 1 1\n");
						fprintf(fScene, "}\n");
						} // for

					// now free our array
					AppMem_Free(LW7Data[0], WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float));
					LW7Data[0] = NULL;
					LW7Ct = LW7NumKeys = 0;
					} // if

				} // if
			else
				{
				fprintf(fScene, "%s\n%d\n%d\n", "ObjectMotion (unnamed)", 9, KeyFrames);
				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame)
						{
						if (GrNode = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
							{
							fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
								Vert.XYZ[0] * LWInfo->UnitScale, Vert.XYZ[1] * LWInfo->UnitScale, Vert.XYZ[2] * LWInfo->UnitScale, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
							fprintf(fScene, "%d %d %f %f %f\n",
								NextKeyFrame, GrNode->Linear,
								GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
							CamTCBRangeWarn += CheckTCBNormalRange(GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);

							LastKeyFrame = NextKeyFrame;
							} // if
						else
							break;
						} // if
					else
						break;
					} // while
				} // if
			} // if target key frames
		else
			{
			Vert.Lat = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue;
			Vert.Lon = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue;
			Vert.Elev = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue;
			Vert.Lon -= LWInfo->RefLon;
			Rend->DefCoords->DegToCart(&Vert);
			FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
			RotatePoint(Vert.XYZ, LWInfo->RotMatx);

			if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
				{
				fprintf(fScene, "%s\nNumChannels %d\n", "ObjectMotion", 9);
				// write 9 non-animated channels for target NULL
				for(LWChan = 0; LWChan < 9; LWChan++)
					{
					fprintf(fScene, "Channel %d\n", LWChan);
					fprintf(fScene, "{ Envelope\n");
					fprintf(fScene, "  1\n");
					switch(LWChan)
						{
						case 0: fprintf(fScene, "  Key %g 0 0 0 0 0 0 0 0\n", Vert.XYZ[0] * LWInfo->UnitScale); break; // pos X
						case 1: fprintf(fScene, "  Key %g 0 0 0 0 0 0 0 0\n", Vert.XYZ[1] * LWInfo->UnitScale); break; // pos Y
						case 2: fprintf(fScene, "  Key %g 0 0 0 0 0 0 0 0\n", Vert.XYZ[2] * LWInfo->UnitScale); break; // pos Z

						case 3: fprintf(fScene, "  Key 0 0 0 0 0 0 0 0 0\n"); break; // rot H
						case 4: fprintf(fScene, "  Key 0 0 0 0 0 0 0 0 0\n"); break; // rot P
						case 5: fprintf(fScene, "  Key 0 0 0 0 0 0 0 0 0\n"); break; // rot B
						
						case 6: fprintf(fScene, "  Key 1 0 0 0 0 0 0 0 0\n"); break; // scale X
						case 7: fprintf(fScene, "  Key 1 0 0 0 0 0 0 0 0\n"); break; // scale Y
						case 8: fprintf(fScene, "  Key 1 0 0 0 0 0 0 0 0\n"); break; // scale Z
						} // switch
					fprintf(fScene, "  Behaviors 1 1\n");
					fprintf(fScene, "}\n");
					} // for
				} // if
			else
				{
				fprintf(fScene, "%s\n%d\n%d\n", "ObjectMotion (unnamed)", 9, 1);
				fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
					Vert.XYZ[0] * LWInfo->UnitScale, Vert.XYZ[1] * LWInfo->UnitScale, Vert.XYZ[2] * LWInfo->UnitScale, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
				fprintf(fScene, "%d %d %f %f %f\n", 0, 0, 0.0, 0.0, 0.0);
				} // else
			} // else

		if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
			{
			// Behaviour was already written for each channel, only write parent info
			fprintf(fScene, "%s 1%07d\n\n", "ParentItem", 0); // Object, item 0 (first item)
			fprintf(fScene, "%s %d\n", "ShadowOptions", 7);
			} // if
		else
			{
			fprintf(fScene, "%s %d\n", "EndBehavior", 1);
			fprintf(fScene, "%s %d\n\n", "ParentObject", 1);
			} // else
		} // if targeted camera

//###################### DEMs and vectors

	DBHost->ResetGeoClip();
	for(MyTurn = DBHost->GetFirst(); MyTurn; MyTurn = DBHost->GetNext(MyTurn))
 		{
		if (MyTurn->TestFlags(WCS_JOEFLAG_ACTIVATED))
			{
			if(MyTurn->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				strcpy(DEMName, MyTurn->FileName());
				strcat(DEMName, ".LWO");
				strmfp(filename, DEMPath, DEMName);
				if (! LWOB_Export(MyTurn, filename, &Vert,
					LWInfo->SaveDEMs, -LWInfo->RefLon, LWInfo->RefLat - 90.0, LWInfo, Rend, ExportAsVersion))
					{
					error = 1;
					break;
					} // if object save error 
				FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
				RotatePoint(Vert.XYZ, LWInfo->RotMatx);
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s %s\n", "LoadObjectLayer 1", &filename[StartFile]);
					fprintf(fScene, "ShowObject 6 6\n"); // DEMs are dark brown
					fprintf(fScene, "%s\nNumChannels %d\n", "ObjectMotion", 9);

					// write 9 non-animated channels for target NULL
					for(LWChan = 0; LWChan < 9; LWChan++)
						{
						fprintf(fScene, "Channel %d\n", LWChan);
						fprintf(fScene, "{ Envelope\n");
						fprintf(fScene, "  1\n");
						switch(LWChan)
							{
							case 0: fprintf(fScene, "  Key %g 0 0 0 0 0 0 0 0\n", Vert.XYZ[0] * LWInfo->UnitScale); break; // pos X
							case 1: fprintf(fScene, "  Key %g 0 0 0 0 0 0 0 0\n", Vert.XYZ[1] * LWInfo->UnitScale); break; // pos Y
							case 2: fprintf(fScene, "  Key %g 0 0 0 0 0 0 0 0\n", Vert.XYZ[2] * LWInfo->UnitScale); break; // pos Z

							case 3: fprintf(fScene, "  Key 0 0 0 0 0 0 0 0 0\n"); break; // rot H
							case 4: fprintf(fScene, "  Key 0 0 0 0 0 0 0 0 0\n"); break; // rot P
							case 5: fprintf(fScene, "  Key 0 0 0 0 0 0 0 0 0\n"); break; // rot B
							
							case 6: fprintf(fScene, "  Key 1 0 0 0 0 0 0 0 0\n"); break; // scale X
							case 7: fprintf(fScene, "  Key 1 0 0 0 0 0 0 0 0\n"); break; // scale Y
							case 8: fprintf(fScene, "  Key 1 0 0 0 0 0 0 0 0\n"); break; // scale Z
							} // switch
						fprintf(fScene, "  Behaviors 1 1\n");
						fprintf(fScene, "}\n");
						} // for
					// Behaviour was already written for each channel, only write parent info
					fprintf(fScene, "%s 1%07d\n\n", "ParentItem", 0); // Object, item 0 (first item)
					fprintf(fScene, "%s %d\n", "ShadowOptions", 7);
					} // if
				else
					{
					fprintf(fScene, "%s %s\n", "LoadObject", &filename[StartFile]);
					fprintf(fScene, "%s\n%d\n%d\n", "ObjectMotion (unnamed)", 9, 1);
					fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
						Vert.XYZ[0] * LWInfo->UnitScale, Vert.XYZ[1] * LWInfo->UnitScale, Vert.XYZ[2] * LWInfo->UnitScale, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
					fprintf(fScene, "%d %d %1.1f %1.1f %1.1f\n", 0, 0, 0.0, 0.0, 0.0);
					fprintf(fScene, "%s %d\n", "EndBehavior", 1);
					fprintf(fScene, "%s %d\n", "ParentObject", 1);
					fprintf(fScene, "%s %d\n\n", "ShadowOptions", 7);
					} // else
				} // if
			else if (MyTurn->GetFirstRealPoint() && MyTurn->GetNumRealPoints() > 0)
				{
				strcpy(DEMName, "WCSNull");
				strcat(DEMName, ".LWO");
				strmfp(filename, DEMPath, DEMName);

				fprintf(fScene, "AddNullObject %s\n", MyTurn->GetBestName());

				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s\nNumChannels %d\n", "ObjectMotion", 9);
					} // if
				else
					{
					fprintf(fScene, "%s\n%d\n%d\n", "ObjectMotion (unnamed)", 9, MyTurn->GetNumRealPoints());
					} // else
				// identify if there is a coordsys attached to this object
				if (MyAttr = (JoeCoordSys *)MyTurn->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					MyCoords = MyAttr->Coord;
				else
					MyCoords = NULL;
				LW7NumKeys = 0;
				for (Point = MyTurn->GetFirstRealPoint(), i = 0; Point; Point = Point->Next, i ++)
					{
					LW7NumKeys++;
					} // for
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					// allocate channel demultiplex buffer
					if (LW7Data[0] = (float *)AppMem_Alloc(WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float), APPMEM_CLEAR))
						{
						LW7Data[1] = &LW7Data[0][LW7NumKeys];
						LW7Data[2] = &LW7Data[0][LW7NumKeys * 2];
						LW7Data[3] = &LW7Data[0][LW7NumKeys * 3];
						LW7Data[4] = &LW7Data[0][LW7NumKeys * 4];
						LW7Data[5] = &LW7Data[0][LW7NumKeys * 5];
						LW7Data[6] = &LW7Data[0][LW7NumKeys * 6];
						LW7Data[7] = &LW7Data[0][LW7NumKeys * 7];
						LW7Data[8] = &LW7Data[0][LW7NumKeys * 8];
						LW7Data[9] = &LW7Data[0][LW7NumKeys * 9];
						LW7Data[10] = &LW7Data[0][LW7NumKeys * 10];
						LW7Data[11] = &LW7Data[0][LW7NumKeys * 11];
						LW7Data[12] = &LW7Data[0][LW7NumKeys * 12];
						LW7Data[13] = &LW7Data[0][LW7NumKeys * 13];
						} // if
					} // if
				LW7Ct = 0;
				for (Point = MyTurn->GetFirstRealPoint(), i = 0; Point; Point = Point->Next, i ++)
					{
					// Convert to Lat/Lon using coordsys
					Point->ProjToDefDeg(MyCoords, &Vert);
					Vert.Elev = (Vert.Elev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
					Vert.Lon -= LWInfo->RefLon;
					Rend->DefCoords->DegToCart(&Vert);
					FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
					RotatePoint(Vert.XYZ, LWInfo->RotMatx);
					if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
						{
						if (LW7Data[0])
							{
							LW7Data[0][LW7Ct] = (float)(Vert.XYZ[0] * LWInfo->UnitScale);
							LW7Data[1][LW7Ct] = (float)(Vert.XYZ[1] * LWInfo->UnitScale);
							LW7Data[2][LW7Ct] = (float)(Vert.XYZ[2] * LWInfo->UnitScale);
							LW7Data[3][LW7Ct] = 0.0f;
							LW7Data[4][LW7Ct] = 0.0f;
							LW7Data[5][LW7Ct] = 0.0f;
							LW7Data[6][LW7Ct] = 1.0f;
							LW7Data[7][LW7Ct] = 1.0f;
							LW7Data[8][LW7Ct] = 1.0f;
							LW7Data[9][LW7Ct] = 0.0f;
							LW7Data[10][LW7Ct] = 0.0f;
							LW7Data[11][LW7Ct] = 0.0f;
							LW7Data[12][LW7Ct] = (float)i;
							LW7Data[13][LW7Ct] = 0.0f;
							LW7Ct ++;
							} // if
						} // if
					else
						{
						fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
							Vert.XYZ[0] * LWInfo->UnitScale, Vert.XYZ[1] * LWInfo->UnitScale, Vert.XYZ[2] * LWInfo->UnitScale, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
						fprintf(fScene, "%d %d %1.1f %1.1f %1.1f\n", i, 0, 0.0, 0.0, 0.0);
						} // else
					} // for


				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					// write 9 animated channels for vector NULL
					if(LW7Data[0])
						{
						for(LWChan = 0; LWChan < 9; LWChan++)
							{
							fprintf(fScene, "Channel %d\n", LWChan);
							fprintf(fScene, "{ Envelope\n");
							fprintf(fScene, "  %d\n", LW7NumKeys);
							for (int LW7CtWrite = 0; LW7CtWrite < LW7Ct; LW7CtWrite ++)
								{
								if(LW7Data[13][LW7CtWrite] != 0.0)
									{ // linear
									fprintf(fScene, "  Key %g %g 3 0 0 0 0 0 0\n", LW7Data[LWChan][LW7CtWrite], LW7Data[12][LW7CtWrite] / Rend->FrameRate);
									} // if
								else
									{ // TCB spline
									fprintf(fScene, "  Key %g %g 0 %g %g %g 0 0 0\n", LW7Data[LWChan][LW7CtWrite], LW7Data[12][LW7CtWrite] / Rend->FrameRate, LW7Data[9][LW7CtWrite], LW7Data[10][LW7CtWrite], LW7Data[11][LW7CtWrite]);
									} // else
								} // for
							fprintf(fScene, "  Behaviors 1 1\n");
							fprintf(fScene, "}\n");
							} // for
						
						// now free our array
						AppMem_Free(LW7Data[0], WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float));
						LW7Data[0] = NULL;
						LW7Ct = LW7NumKeys = 0;
						} // if

					// Behaviour was already written for each channel, only write parent info
					fprintf(fScene, "%s 1%07d\n\n", "ParentItem", 0); // Object, item 0 (first item)
					fprintf(fScene, "%s %d\n", "ShadowOptions", 7);
					} // if
				else
					{
					fprintf(fScene, "%s %d\n", "EndBehavior", 1);
					fprintf(fScene, "%s %d\n\n", "ParentObject", 1);
					} // else
				} // if
			} // if
		} // for

	if (error)
		goto EndExport;

//###################### Ambient Light

	if (DefAtmo = (Atmosphere *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 0, NULL))
		{
		if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
			{
			fprintf(fScene, "%s %g %g %g\n", "AmbientColor",
				(DefAtmo->TopAmbientColor.GetCurValue(0)),
				(DefAtmo->TopAmbientColor.GetCurValue(1)),
				(DefAtmo->TopAmbientColor.GetCurValue(2)));
			fprintf(fScene, "%s %g\n\n", "AmbientIntensity", DefAtmo->TopAmbientColor.Intensity.CurValue);
			} // if
		else
			{
			fprintf(fScene, "%s %d %d %d\n", "AmbientColor",
				(long)(DefAtmo->TopAmbientColor.GetCurValue(0) * 255),
				(long)(DefAtmo->TopAmbientColor.GetCurValue(1) * 255),
				(long)(DefAtmo->TopAmbientColor.GetCurValue(2) * 255));
			fprintf(fScene, "%s %f\n\n", "AmbIntensity", DefAtmo->TopAmbientColor.Intensity.CurValue);
			} // else
		} // if

//###################### Sun Light

	for (CurLight = (Light *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT); CurLight; CurLight = (Light *)CurLight->Next)
		{
		if (CurLight->Enabled)
			{
			fprintf(fScene, "%s\n", "AddLight");
			fprintf(fScene, "%s %s\n", "LightName", CurLight->GetName());
			if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
				{
				fprintf(fScene, "%s\n", "ShowLight 1 13"); // lights are, uh, gold/yellow
				} // if

			if (CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetNumNodes(0) > 0
				|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetNumNodes(0) > 0
				|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetNumNodes(0) > 0)
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
				Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;
				LastKeyFrame = -2;
				KeyFrames = 0;

				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame)
						{
						KeyFrames ++;
						LastKeyFrame = NextKeyFrame;
						} // if
					else
						break;
					} // while

				LastKeyFrame = -2;
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					LW7NumKeys = (KeyFrames > 0) ? KeyFrames: 1;
					LW7Ct = 0;
					if (LW7Data[0] = (float *)AppMem_Alloc(WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float), APPMEM_CLEAR))
						{
						LW7Data[1] = &LW7Data[0][LW7NumKeys];
						LW7Data[2] = &LW7Data[0][LW7NumKeys * 2];
						LW7Data[3] = &LW7Data[0][LW7NumKeys * 3];
						LW7Data[4] = &LW7Data[0][LW7NumKeys * 4];
						LW7Data[5] = &LW7Data[0][LW7NumKeys * 5];
						LW7Data[6] = &LW7Data[0][LW7NumKeys * 6];
						LW7Data[7] = &LW7Data[0][LW7NumKeys * 7];
						LW7Data[8] = &LW7Data[0][LW7NumKeys * 8];
						LW7Data[9] = &LW7Data[0][LW7NumKeys * 9];
						LW7Data[10] = &LW7Data[0][LW7NumKeys * 10];
						LW7Data[11] = &LW7Data[0][LW7NumKeys * 11];
						LW7Data[12] = &LW7Data[0][LW7NumKeys * 12];
						LW7Data[13] = &LW7Data[0][LW7NumKeys * 13];
						} // if

					fprintf(fScene, "%s\nNumChannels %d\n", "LightMotion", 9);

					if (LW7Data[0])
						{
						while (1)	//lint !e716
							{
							NextKeyFrame = -1;
							Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
							Prop.NewKeyNodeRange[0] = -DBL_MAX;
							Prop.NewKeyNodeRange[1] = DBL_MAX;
							CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetRAHostProperties(&Prop);
							if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
								NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
							if (NextKeyFrame > LastKeyFrame)
								{
								if (GrNode = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
									{
									CurLight->SetToTime(NextKeyFrame / Rend->FrameRate);
									Vert.Lat = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
									Vert.Lon = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
									Vert.Elev = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;
									Vert.Lon -= LWInfo->RefLon;
									Rend->DefCoords->DegToCart(&Vert);
									FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
									RotatePoint(Vert.XYZ, LWInfo->RotMatx);
									
									LW7Data[0][LW7Ct] = (float)(Vert.XYZ[0] * LWInfo->UnitScale);
									LW7Data[1][LW7Ct] = (float)(Vert.XYZ[1] * LWInfo->UnitScale);
									LW7Data[2][LW7Ct] = (float)(Vert.XYZ[2] * LWInfo->UnitScale);
									LW7Data[3][LW7Ct] = 0.0f;
									LW7Data[4][LW7Ct] = 0.0f;
									LW7Data[5][LW7Ct] = 0.0f;
									LW7Data[6][LW7Ct] = 1.0f;
									LW7Data[7][LW7Ct] = 1.0f;
									LW7Data[8][LW7Ct] = 1.0f;
									LW7Data[9][LW7Ct] = (float)GrNode->TCB[0];
									LW7Data[10][LW7Ct] = (float)GrNode->TCB[1];
									LW7Data[11][LW7Ct] = (float)GrNode->TCB[2];
									LW7Data[12][LW7Ct] = (float)NextKeyFrame;
									LW7Data[13][LW7Ct] = (float)GrNode->Linear;
									LW7Ct ++;
									
									LightTCBRangeWarn += CheckTCBNormalRange(GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);

									LastKeyFrame = NextKeyFrame;
									} // if
								else
									break;
								} // if
							else
								break;
							} // while

						// write channels
						for(LWChan = 0; LWChan < 9; LWChan++)
							{
							fprintf(fScene, "Channel %d\n", LWChan);
							fprintf(fScene, "{ Envelope\n");
							fprintf(fScene, "  %d\n", LW7NumKeys);
							for (int LW7CtWrite = 0; LW7CtWrite < LW7Ct; LW7CtWrite ++)
								{
								if(LW7Data[13][LW7CtWrite] != 0.0)
									{ // linear
									fprintf(fScene, "  Key %g %g 3 0 0 0 0 0 0\n", LW7Data[LWChan][LW7CtWrite], LW7Data[12][LW7CtWrite] / Rend->FrameRate);
									} // if
								else
									{ // TCB spline
									fprintf(fScene, "  Key %g %g 0 %g %g %g 0 0 0\n", LW7Data[LWChan][LW7CtWrite], LW7Data[12][LW7CtWrite] / Rend->FrameRate, LW7Data[9][LW7CtWrite], LW7Data[10][LW7CtWrite], LW7Data[11][LW7CtWrite]);
									} // else
								} // for
							fprintf(fScene, "  Behaviors 1 1\n");
							fprintf(fScene, "}\n");
							} // for

						// now free our array
						AppMem_Free(LW7Data[0], WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float));
						LW7Data[0] = NULL;
						LW7Ct = LW7NumKeys = 0;
						} // if
					} // if
				else
					{
					fprintf(fScene, "%s\n%d\n%d\n", "LightMotion (unnamed)", 9, KeyFrames);
					while (1)	//lint !e716
						{
						NextKeyFrame = -1;
						Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
						Prop.NewKeyNodeRange[0] = -DBL_MAX;
						Prop.NewKeyNodeRange[1] = DBL_MAX;
						CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetRAHostProperties(&Prop);
						if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
							NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
						if (NextKeyFrame > LastKeyFrame)
							{
							if (GrNode = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
								{
								CurLight->SetToTime(NextKeyFrame / Rend->FrameRate);
								Vert.Lat = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
								Vert.Lon = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
								Vert.Elev = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;
								Vert.Lon -= LWInfo->RefLon;
								Rend->DefCoords->DegToCart(&Vert);
								FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
								RotatePoint(Vert.XYZ, LWInfo->RotMatx);
								fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
									Vert.XYZ[0] * LWInfo->UnitScale, Vert.XYZ[1] * LWInfo->UnitScale, Vert.XYZ[2] * LWInfo->UnitScale, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
								fprintf(fScene, "%d %d %f %f %f\n",
									NextKeyFrame, GrNode->Linear,
									GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
								LightTCBRangeWarn += CheckTCBNormalRange(GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);

								LastKeyFrame = NextKeyFrame;
								} // if
							else
								break;
							} // if
						else
							break;
						} // while
					} // else
				} // if light key frames
			else
				{
				Vert.Lat = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
				Vert.Lon = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
				Vert.Elev = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;
				Vert.Lon -= LWInfo->RefLon;
				Rend->DefCoords->DegToCart(&Vert);
				FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
				RotatePoint(Vert.XYZ, LWInfo->RotMatx);
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s\nNumChannels %d\n", "LightMotion", 9);

					// write 9 non-animated channels for Light
					for(LWChan = 0; LWChan < 9; LWChan++)
						{
						fprintf(fScene, "Channel %d\n", LWChan);
						fprintf(fScene, "{ Envelope\n");
						fprintf(fScene, "  1\n");
						switch(LWChan)
							{
							case 0: fprintf(fScene, "  Key %g 0 0 0 0 0 0 0 0\n", Vert.XYZ[0] * LWInfo->UnitScale); break; // pos X
							case 1: fprintf(fScene, "  Key %g 0 0 0 0 0 0 0 0\n", Vert.XYZ[1] * LWInfo->UnitScale); break; // pos Y
							case 2: fprintf(fScene, "  Key %g 0 0 0 0 0 0 0 0\n", Vert.XYZ[2] * LWInfo->UnitScale); break; // pos Z

							case 3: fprintf(fScene, "  Key 0 0 0 0 0 0 0 0 0\n"); break; // rot H
							case 4: fprintf(fScene, "  Key 0 0 0 0 0 0 0 0 0\n"); break; // rot P
							case 5: fprintf(fScene, "  Key 0 0 0 0 0 0 0 0 0\n"); break; // rot B
							
							case 6: fprintf(fScene, "  Key 1 0 0 0 0 0 0 0 0\n"); break; // scale X
							case 7: fprintf(fScene, "  Key 1 0 0 0 0 0 0 0 0\n"); break; // scale Y
							case 8: fprintf(fScene, "  Key 1 0 0 0 0 0 0 0 0\n"); break; // scale Z
							} // switch
						fprintf(fScene, "  Behaviors 1 1\n");
						fprintf(fScene, "}\n");
						} // for
					} // if
				else
					{
					fprintf(fScene, "%s\n%d\n%d\n", "LightMotion (unnamed)", 9, 1);
					fprintf(fScene, "%f %f %f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
						Vert.XYZ[0] * LWInfo->UnitScale, Vert.XYZ[1] * LWInfo->UnitScale, Vert.XYZ[2] * LWInfo->UnitScale, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
					fprintf(fScene, "%d %d %f %f %f\n", 0, 0, 0.0, 0.0, 0.0);
					} // else
				} // else

			if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
				{
				// Behaviour was already written for each channel, only write parent info
				fprintf(fScene, "%s 1%07d\n\n", "ParentItem", 0); // Object, item 0 (first item)

				// light color
				fprintf(fScene, "%s %g %g %g\n", "LightColor",
					(CurLight->Color.GetCurValue(0)),
					(CurLight->Color.GetCurValue(1)),
					(CurLight->Color.GetCurValue(2)));

				fprintf(fScene, "%s %g\n\n", "LightIntensity", CurLight->Color.Intensity.CurValue);
				fprintf(fScene, "%s %d\n", "AffectCaustics", 1);
				fprintf(fScene, "%s %d\n", "LightType", CurLight->LightType <= 1 ? 1: CurLight->LightType); // our light type enums match LW's except that their distant type is directional.
				// falloff seems missing in LW7 files. Hmm
				//fprintf(fScene, "%s %f\n", "Falloff", 0.0);
				// lensflare also missing
				//fprintf(fScene, "%s %d\n", "LensFlare", 0);
				fprintf(fScene, "%s %d\n\n", "ShadowType", CurLight->CastShadows); // 0=none, 1=raytrace, 2=shadowmap
				fprintf(fScene, "%s 0 0 0\n\n", "ShadowColor");
				} // if
			else
				{
				fprintf(fScene, "%s %d\n", "EndBehavior", 1);
				fprintf(fScene, "%s %d\n\n", "ParentObject", 1);
				fprintf(fScene, "%s %d %d %d\n", "LightColor",
					(long)(CurLight->Color.GetClampedCompleteValue(0) * 255),
					(long)(CurLight->Color.GetClampedCompleteValue(1) * 255),
					(long)(CurLight->Color.GetClampedCompleteValue(2) * 255));
				fprintf(fScene, "%s %d\n", "LightType", CurLight->LightType <= 1 ? 1: CurLight->LightType); // our light type enums match LW's
				fprintf(fScene, "%s %f\n", "Falloff", 0.0);
				fprintf(fScene, "%s %d\n", "LensFlare", 0);
				fprintf(fScene, "%s %d\n\n", "ShadowCasting", CurLight->CastShadows);
				} // else

			} // if light enabled
		} // for each light

//###################### Camera Motion

	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{
		fprintf(fScene, "%s\n", "AddCamera");
		fprintf(fScene, "%s %s\n", "CameraName", Rend->Cam->Name);
		fprintf(fScene, "%s %d %d\n", "ShowCamera", 1, 11); // cameras are red
		fprintf(fScene, "%s\n", "CameraMotion");
		} // if
	else
		{
		fprintf(fScene, "%s\n", "CameraMotion (unnamed)");
		} // else
	if (ExportWave(LWInfo, fScene, Rend, ExportAsVersion))
		{
		error = 1;
		goto EndExport;
		} // if motion file write failed 
	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{
		fprintf(fScene, "%s 1%07d\n\n", "ParentItem", 0); // Object, item 0 (first item)
		} // if
	else
		{
		fprintf(fScene, "%s %d\n", "EndBehavior", 1);
		fprintf(fScene, "%s %d\n", "ParentObject", 1);
		} // else

//###################### Camera Field of View

	if (Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetNumNodes(0) > 0)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
		Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
		LastKeyFrame = -2;
		KeyFrames = 0;

		while (1)	//lint !e716
			{
			NextKeyFrame = -1;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
				NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
			if (NextKeyFrame > LastKeyFrame)
				{
				KeyFrames ++;
				LastKeyFrame = NextKeyFrame;
				} // if
			else
				break;
			} // while
		if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
			{
			fprintf(fScene, "%s\n{ Envelope\n  %d\n", "ZoomFactor (envelope)", KeyFrames);
			} // if
		else
			{
			fprintf(fScene, "%s\n%d\n%d\n", "ZoomFactor (envelope)", 1, KeyFrames);
			} // else
		LastKeyFrame = -2;
		while (1)	//lint !e716
			{
			NextKeyFrame = -1;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
				NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
			if (NextKeyFrame > LastKeyFrame)
				{
				if (GrNode = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
					{
					Rend->Cam->SetToTime(NextKeyFrame / Rend->FrameRate);
					value = (Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * 
						Rend->Opt->OutputImageWidth / Rend->Opt->OutputImageHeight) / 
						tan((Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * 0.5) * PiOver180);
					if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
						{
						fprintf(fScene, "  Key %g %g %d %g %g %g 0 0 0\n", value, NextKeyFrame * Rend->FrameRate,
						 GrNode->Linear ? 3 : 0, GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
						} // if
					else
						{
						fprintf(fScene, "%f\n", value);
						fprintf(fScene, "%d %d %f %f %f\n",
							NextKeyFrame, GrNode->Linear,
							GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
						} // else
					CamTCBRangeWarn += CheckTCBNormalRange(GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);

					LastKeyFrame = NextKeyFrame;
					} // if
				else
					break;
				} // if
			else
				break;
			} // while
		if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
			{
			fprintf(fScene, "%s %d %d\n}\n", "  Behaviors", 1, 1);
			} // if
		else
			{
			fprintf(fScene, "%s %d\n", "EndBehavior", 1);
			} // else
		} // if target key frames
	else
		{
		value = (Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * 
			Rend->Opt->OutputImageWidth / Rend->Opt->OutputImageHeight) / 
			tan((Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * 0.5) * PiOver180);
		fprintf(fScene, "%s %f\n", "ZoomFactor", value);
		} // else

	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_3)
		{ // written later in LW7
		fprintf(fScene, "%s %d\n", "RenderMode", 2);
		fprintf(fScene, "%s %d\n", "RayTraceEffects", 0);
		} // if

//###################### Resolution

	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{
		fprintf(fScene, "ResolutionMultiplier 1.0\n");
		fprintf(fScene, "%s %d %d\n", "FrameSize", Rend->Opt->OutputImageWidth, Rend->Opt->OutputImageHeight);
		fprintf(fScene, "%s %g\n", "PixelAspect", Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue);
		fprintf(fScene, "%s 0 0 %d %d\n", "MaskPosition", Rend->Opt->OutputImageWidth, Rend->Opt->OutputImageHeight);
		fprintf(fScene, "%s %d\n", "MotionBlur", 0);
		fprintf(fScene, "%s %d\n", "FieldRendering", Rend->Cam->FieldRender);
		fprintf(fScene, "\n");

		fprintf(fScene, "%s %g\n", "ApertureHeight", 0.015); // In Meters, replaces FilmSize
		
		// these two are written later for LW7
		fprintf(fScene, "%s %d\n", "Antialiasing", 0);
		fprintf(fScene, "%s %d\n", "AdaptiveSampling", 1);

		fprintf(fScene, "%s %g\n", "AdaptiveThreshold", .03137255); // Used to be 8, now in units of n/255
		fprintf(fScene, "%s %d\n", "FilmSize", 2);
		fprintf(fScene, "%s %d\n\n", "DepthOfField", 0);
		} // if
	else
		{
		fprintf(fScene, "Resolution 1\n");
		fprintf(fScene, "%s %d %d\n", "CustomSize", Rend->Opt->OutputImageWidth, Rend->Opt->OutputImageHeight);
		fprintf(fScene, "PixelAspectRatio -1\n");
		fprintf(fScene, "%s %f\n", "CustomPixelRatio", Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue);


		fprintf(fScene, "%s %d\n", "Antialiasing", 0);
		fprintf(fScene, "%s %d\n", "AdaptiveSampling", 1);
		fprintf(fScene, "%s %d\n", "AdaptiveThreshold", 8);
		fprintf(fScene, "%s %d\n", "FilmSize", 2);
		fprintf(fScene, "%s %d\n", "FieldRendering", Rend->Cam->FieldRender);
		fprintf(fScene, "%s %d\n", "MotionBlur", 0);
		fprintf(fScene, "%s %d\n\n", "DepthOfField", 0);
		} // else

	fprintf(fScene, "%s %d\n", "SolidBackdrop", 0);
	fprintf(fScene, "%s %d %d %d\n", "BackdropColor", 0, 0, 0);

	if (DefSky = (Sky *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_SKY, 0, NULL))
		{
		if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
			{
			DefSky->SkyGrad.GetBasicColor(DRed, DGreen, DBlue, 0.0);
			fprintf(fScene, "%s %g %g %g\n", "ZenithColor", DRed, DGreen, DBlue);

			DefSky->SkyGrad.GetBasicColor(DRed, DGreen, DBlue, .5);
			fprintf(fScene, "%s %g %g %g\n", "SkyColor", DRed, DGreen, DBlue);

			fprintf(fScene, "%s %g %g %g\n", "GroundColor", .1960784, .1568628, .1176471);

			DefSky->SkyGrad.GetBasicColor(DRed, DGreen, DBlue, 1.0);
			fprintf(fScene, "%s %g %g %g\n", "NadirColor", DRed, DGreen, DBlue);
			} // if
		else
			{
			DefSky->SkyGrad.GetBasicColor(Red, Green, Blue, 0.0);
			fprintf(fScene, "%s %d %d %d\n", "ZenithColor", Red, Green, Blue);

			DefSky->SkyGrad.GetBasicColor(Red, Green, Blue, .5);
			fprintf(fScene, "%s %d %d %d\n", "SkyColor", Red, Green, Blue);

			fprintf(fScene, "%s %d %d %d\n", "GroundColor", 50, 40, 30);

			DefSky->SkyGrad.GetBasicColor(Red, Green, Blue, 1.0);
			fprintf(fScene, "%s %d %d %d\n", "NadirColor", Red, Green, Blue);
			} // if
		} // if

//###################### Haze

	if (DefAtmo)
		{
		fprintf(fScene, "%s %d\n", "FogType", 1);

		if (DefAtmo->HazeEnabled)
			{
			if (DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].GetNumNodes(0) > 0)
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
				Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
				LastKeyFrame = -2;
				KeyFrames = 0;

				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame)
						{
						KeyFrames ++;
						LastKeyFrame = NextKeyFrame;
						} // if
					else
						break;
					} // while
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s\n{ Envelope\n  %d\n", "FogMinDist (envelope)", KeyFrames);
					} // if
				else
					{
					fprintf(fScene, "%s\n%d\n%d\n", "FogMinDist (envelope)", 1, KeyFrames);
					} // else
				LastKeyFrame = -2;
				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame)
						{
						if (GrNode = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
							{
							DefAtmo->SetToTime(NextKeyFrame / Rend->FrameRate);
							value = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue;
							if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
								{
								fprintf(fScene, "Key %g %g %d %g %g %g 0 0 0\n", value, NextKeyFrame * Rend->FrameRate,
								 GrNode->Linear ? 3 : 0, GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
								} // if
							else
								{
								fprintf(fScene, "%f\n", value);
								fprintf(fScene, "%d %d %f %f %f\n",
									NextKeyFrame, GrNode->Linear,
									GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
								} // else
							HazeTCBRangeWarn += CheckTCBNormalRange(GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);

							LastKeyFrame = NextKeyFrame;
							} // if
						else
							break;
						} // if
					else
						break;
					} // while
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s %d %d\n}\n", "  Behaviors", 1, 1);
					} // if
				else
					{
					fprintf(fScene, "%s %d\n", "EndBehavior", 1);
					} // else
				} // if haze key frames
			else
				{
				value = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue;
				fprintf(fScene, "%s %f\n", "FogMinDist", value);
				} // else


			if (DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].GetNumNodes(0) > 0
				|| DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].GetNumNodes(0) > 0)
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
				Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;
				LastKeyFrame = -2;
				KeyFrames = 0;

				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame)
						{
						KeyFrames ++;
						LastKeyFrame = NextKeyFrame;
						} // if
					else
						break;
					} // while
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s\n{ Envelope\n  %d\n", "FogMaxDist (envelope)", KeyFrames);
					} // if
				else
					{
					fprintf(fScene, "%s\n%d\n%d\n", "FogMaxDist (envelope)", 1, KeyFrames);
					} // else
				LastKeyFrame = -2;
				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame)
						{
						if (GrNode = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
							{
							DefAtmo->SetToTime(NextKeyFrame / Rend->FrameRate);
							value = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue +
								DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue;
							if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
								{
								fprintf(fScene, "Key %g %g %d %g %g %g 0 0 0\n", value, NextKeyFrame * Rend->FrameRate,
								 GrNode->Linear ? 3 : 0, GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
								} // if
							else
								{
								fprintf(fScene, "%f\n", value);
								fprintf(fScene, "%d %d %f %f %f\n",
									NextKeyFrame, GrNode->Linear,
									GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
								} // else
							HazeTCBRangeWarn += CheckTCBNormalRange(GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);

							LastKeyFrame = NextKeyFrame;
							} // if
						else
							break;
						} // if
					else
						break;
					} // while
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s %d %d\n}\n", "  Behaviors", 1, 1);
					} // if
				else
					{
					fprintf(fScene, "%s %d\n", "EndBehavior", 1);
					} // else
				} // if haze end key frames
			else
				{
				value = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue +
					DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue;
				fprintf(fScene, "%s %f\n", "FogMaxDist", value);
				} // else

			// FogMinAmount
			if (DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].GetNumNodes(0) > 0)
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
				Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
				LastKeyFrame = -2;
				KeyFrames = 0;

				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame)
						{
						KeyFrames ++;
						LastKeyFrame = NextKeyFrame;
						} // if
					else
						break;
					} // while
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s\n{ Envelope\n  %d\n", "FogMinAmount (envelope)", KeyFrames);
					} // if
				else
					{
					fprintf(fScene, "%s\n%d\n%d\n", "FogMinAmount (envelope)", 1, KeyFrames);
					} // else
				LastKeyFrame = -2;
				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame)
						{
						if (GrNode = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
							{
							DefAtmo->SetToTime(NextKeyFrame / Rend->FrameRate);
							value = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].CurValue;
							if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
								{
								fprintf(fScene, "Key %g %g %d %g %g %g 0 0 0\n", value, NextKeyFrame * Rend->FrameRate,
								 GrNode->Linear ? 3 : 0, GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
								} // if
							else
								{
								fprintf(fScene, "%f\n", value);
								fprintf(fScene, "%d %d %f %f %f\n",
									NextKeyFrame, GrNode->Linear,
									GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
								} // else
							HazeTCBRangeWarn += CheckTCBNormalRange(GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);

							LastKeyFrame = NextKeyFrame;
							} // if
						else
							break;
						} // if
					else
						break;
					} // while
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s %d %d\n}\n", "  Behaviors", 1, 1);
					} // if
				else
					{
					fprintf(fScene, "%s %d\n", "EndBehavior", 1);
					} // else
				} // if haze key frames
			else
				{
				value = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].CurValue;
				fprintf(fScene, "%s %f\n", "FogMinAmount", value);
				} // else

			// FogMaxAmount
			if (DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].GetNumNodes(0) > 0)
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
				Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
				LastKeyFrame = -2;
				KeyFrames = 0;

				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame)
						{
						KeyFrames ++;
						LastKeyFrame = NextKeyFrame;
						} // if
					else
						break;
					} // while
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s\n{ Envelope\n  %d\n", "FogMaxAmount (envelope)", KeyFrames);
					} // if
				else
					{
					fprintf(fScene, "%s\n%d\n%d\n", "FogMaxAmount (envelope)", 1, KeyFrames);
					} // else
				LastKeyFrame = -2;
				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame)
						{
						if (GrNode = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
							{
							DefAtmo->SetToTime(NextKeyFrame / Rend->FrameRate);
							value = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].CurValue;
							if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
								{
								fprintf(fScene, "Key %g %g %d %g %g %g 0 0 0\n", value, NextKeyFrame * Rend->FrameRate,
								 GrNode->Linear ? 3 : 0, GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
								} // if
							else
								{
								fprintf(fScene, "%f\n", value);
								fprintf(fScene, "%d %d %f %f %f\n",
									NextKeyFrame, GrNode->Linear,
									GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);
								} // else
							HazeTCBRangeWarn += CheckTCBNormalRange(GrNode->TCB[0], GrNode->TCB[1], GrNode->TCB[2]);

							LastKeyFrame = NextKeyFrame;
							} // if
						else
							break;
						} // if
					else
						break;
					} // while
				if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
					{
					fprintf(fScene, "%s %d %d\n}\n", "  Behaviors", 1, 1);
					} // if
				else
					{
					fprintf(fScene, "%s %d\n", "EndBehavior", 1);
					} // else
				} // if haze key frames
			else
				{
				value = DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].CurValue;
				fprintf(fScene, "%s %f\n", "FogMaxAmount", value);
				} // else

			if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
				{
				fprintf(fScene, "%s %g %g %g\n", "FogColor",
					(DefAtmo->HazeColor.GetClampedCompleteValue(0)),
					(DefAtmo->HazeColor.GetClampedCompleteValue(1)),
					(DefAtmo->HazeColor.GetClampedCompleteValue(2)));
				fprintf(fScene, "%s %d\n", "BackdropFog", 0);
				} // if
			else
				{
				fprintf(fScene, "%s %d %d %d\n", "FogColor",
					(long)(DefAtmo->HazeColor.GetClampedCompleteValue(0) * 255),
					(long)(DefAtmo->HazeColor.GetClampedCompleteValue(1) * 255),
					(long)(DefAtmo->HazeColor.GetClampedCompleteValue(2) * 255));
				} // else

			} // if haze enabled
		} // if

//###################### Miscellaneous

	fprintf(fScene, "%s %d\n", "DitherIntensity", 1);
	fprintf(fScene, "%s %d\n\n", "AnimatedDither", 0);

	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{ // written earlier in LW3.x
		fprintf(fScene, "%s %d\n", "RenderMode", 2);
		fprintf(fScene, "%s %d\n", "RayTraceEffects", 0);

		// new fields for LW7
		fprintf(fScene, "%s %d\n", "RayRecursionLimit", 16);
		fprintf(fScene, "%s %d\n", "RayTraceOptimization", 1);
		fprintf(fScene, "%s\n", "DataOverlayLabel  ");
		} // if

//###################### File Output
	strmfp(filename, ProjHost->framepath, ProjHost->framefile);
	strcat(filename, "LW");
	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{
		fprintf(fScene, "%s %d\n", "OutputFilenameFormat", 1);
		fprintf(fScene, "%s %d\n", "SaveRGB", 0);
		} // if
	fprintf(fScene, "%s %s\n\n", "SaveRGBImagesPrefix", filename);
	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{
		fprintf(fScene, "%s %s\n", "RGBImageSaver", "_ILBM");
		fprintf(fScene, "%s %d\n", "SaveAlpha", 0);
		} // if

//###################### View stuff
	if(ExportAsVersion == WCS_EXPORT_FORMAT_LIGHTWAVE_7)
		{
		fprintf(fScene, "%s %d\n", "ViewConfiguration", 0);
		fprintf(fScene, "%s %d\n", "DefineView", 0);
		fprintf(fScene, "%s %d\n", "ViewType", 9);
		fprintf(fScene, "%s %f %f %f\n", "ViewAimPoint", 0.0, 0.0, 0.0);
		fprintf(fScene, "%s %f\n\n", "ViewZoomFactor", 3.2);

		// grid stuff
		fprintf(fScene, "%s %d\n", "GridNumber", 10);
		fprintf(fScene, "%s %f\n", "GridSize", 500.0);

		// BG stuff
		fprintf(fScene, "%s %d\n", "CameraViewBG", 0);
		
		// show stuff
		fprintf(fScene, "%s %d\n", "ShowMotionPath", 1);
		fprintf(fScene, "%s %d\n", "ShowFogRadius", 1);
		fprintf(fScene, "%s %d\n", "ShowFogEffect", 1);
		fprintf(fScene, "%s %d\n", "ShowSafeAreas", 0);
		fprintf(fScene, "%s %d\n", "ShowFieldChart", 0);

		// Current settings
		fprintf(fScene, "%s %d\n", "CurrentObject", 0);
		fprintf(fScene, "%s %d\n", "CurrentLight", 0);
		fprintf(fScene, "%s %d\n", "CurrentCamera", 0);
		} // if
	else
		{
		fprintf(fScene, "%s %d\n", "ViewMode", 5);
		fprintf(fScene, "%s %f %f %f\n", "ViewAimPoint", 0.0, 0.0, 0.0);
		fprintf(fScene, "%s %f %f %f\n", "ViewDirection", 0.0, 0.0, 0.0);
		fprintf(fScene, "%s %f\n", "ViewZoomFactor", 3.2);

		// grid stuff
		fprintf(fScene, "%s %d\n", "LayoutGrid", 3);
		fprintf(fScene, "%s %f\n", "GridSize", 500.0);
		
		// show stuff
		fprintf(fScene, "%s %d\n", "ShowObjects", 1);
		fprintf(fScene, "%s %d\n", "ShowBones", 1);
		fprintf(fScene, "%s %d\n", "ShowLights", 0);
		fprintf(fScene, "%s %d\n", "ShowCamera", 1);
		fprintf(fScene, "%s %d\n", "ShowMotionPath", 1);
		fprintf(fScene, "%s %d\n", "ShowSafeAreas", 0);
		// BG stuff
		fprintf(fScene, "%s %d\n", "ShowBGImage", 0);
		// more show stuff
		fprintf(fScene, "%s %d\n", "ShowFogRadius", 1);
		fprintf(fScene, "%s %d\n", "ShowRedraw", 0);
		} // else

	strcpy(ProjHost->altobjectpath, ScenePath);

EndExport:
	fclose(fScene);
	} /* if file opened */

if (error)
	UserMessageOK("LW Scene Export", "A problem occurred saving the LW scene.\n\
If a file was created it will not be complete and may not load properly into LightWave.");

/*
if (NotFound)
	UserMessageOK("LW Scene Export", "The output image size is not a standard\
 LightWave image size. The zoom factor and image dimensions may not be\
 portrayed correctly in the scene file just created.");
*/
if (CamTCBRangeWarn)
	UserMessageOK("LW Scene Export", "Tension, Continuity or Bias values exceeding\
 Lightwave's normal +-1.0 range were found while exporting the\
 Camera animation information. Lightwave may clip these\
 parameters, and the animations may not match.");

if (LightTCBRangeWarn)
	UserMessageOK("LW Scene Export", "Tension, Continuity or Bias values exceeding\
 Lightwave's normal +-1.0 range were found while exporting the\
 Light animation information. Lightwave may clip these\
 parameters, and the animations may not match.");

if (HazeTCBRangeWarn)
	UserMessageOK("LW Scene Export", "Tension, Continuity or Bias values exceeding\
 Lightwave's normal +-1.0 range were found while exporting the\
 Haze animation information. Lightwave may clip these\
 parameters, and the animations may not match.");


} // SceneExportGUI::LWScene_Export() 

/*===========================================================================*/

/* Sample LW Scene files
LWSC
1

FirstFrame 600
LastFrame 600
FrameStep 1

LoadObject wcsprojects:Topos.object/40105.6.LWOBb
ObjectMotion (unnamed)
  9
  1
  -4678872.000000 4115472.000000 1319578.000000 0.0 0.0 0.0 1.0 1.0 1.0
  0 0 0.0 0.0 0.0
EndBehavior 1
ShadowOptions 7

AmbientColor 255 255 255
AmbIntensity 0.250000

AddLight
LightName Light
LightMotion (unnamed)
  9
  1
  0.0 0.0 0.0 59.999996 29.999998 0.0 1.0 1.0 1.0
  0 0 0.0 0.0 0.0
EndBehavior 1
LightColor 255 255 255
LgtIntensity 1.000000
LightType 0
LensFlare 0
ShadowCasting 1

CameraMotion (unnamed)
  9
  2
  -4677688.000000 4120507.000000 1305151.750000 -31.196329 22.619295 32.774567 0.0 0.0 0.0
  0 0 0.0 0.0 0.0
  -4677299.500000 4120908.750000 1305275.750000 -43.875767 31.254276 23.428293 0.0 0.0 0.0
  600 0 0.0 0.0 0.0
EndBehavior 1
ZoomFactor 3.200000
RenderMode 2
RayTraceEffects 0
Resolution 1
Overscan 1
Antialiasing 0
AdaptiveSampling 1
AdaptiveThreshold 8
FilmSize 2
FieldRendering 0
MotionBlur 0
DepthOfField 0

SolidBackdrop 1
BackdropColor 0 0 0
ZenithColor 0 40 80
SkyColor 120 180 240
GroundColor 50 40 30
NadirColor 100 80 60
FogType 0
DitherIntensity 1
AnimatedDither 0

SaveRGBImagesPrefix wcsframes:LWRMNP

ViewMode 5
ViewAimpoint 0.000000 0.000000 0.000000
ViewDirection 0.000000 -0.174533 0.000000
ViewZoomFactor 3.200000
LayoutGrid 3
GridSize 20000.000000
ShowObjects 1
ShowBones 1
ShowLights 0
ShowCamera 1
ShowMotionPath 1
ShowSafeAreas 0
ShowBGImage 0
ShowFogRadius 0
ShowRedraw 0


LWSC
1

FirstFrame 0
LastFrame 0
FrameStep 1

LoadObject WCSProjects:Colorado/RMNP.object/40105.6   .LWOB
ObjectMotion (unnamed)
  9
  1
  -4678872.000000 4115472.000000 1319578.375000 0.0 0.0 0.0 1.0 1.0 1.0
  0 0 0.0 0.0 0.0
EndBehavior 1
ShadowOptions 7

AmbientColor 0 0 10
AmbIntensity 1.000000

AddLight
LightName Sun
LightMotion (unnamed)
  9
  1
  -126057988096.000000 30863413248.000000 -73210175488.000000 0.0 0.0 0.0 1.0 1.0 1.0
  0 0 0.0 0.0 0.0
EndBehavior 1
LightColor 255 255 255
LgtIntensity (envelope)
  1
  2
  1.0
  0 0 0.0 0.0 0.0
  0.570000
  37 0 0.0 0.0 0.0
EndBehavior 1
LightType 1
Falloff 0.000000
LensFlare 0
ShadowCasting 1

CameraMotion (unnamed)
  9
  3
  -4677688.000000 4120507.000000 1305151.750000 -31.196329 22.619295 32.774567 0.0 0.0 0.0
  0 0 0.0 0.0 0.0
  -2700000.000000 4120507.000000 1305151.750000 -31.196329 22.619295 32.774567 0.0 0.0 0.0
  310 1 0.0 0.0 0.0
  -4677299.500000 4120908.750000 1305275.750000 -43.875767 31.254276 23.428293 0.0 0.0 0.0
  600 0 0.0 0.0 0.0
EndBehavior 1
ZoomFactor (envelope)
  1
  3
  2.976381
  0 1 1.0 0.0 0.0
  4.500000
  31 0 0.0 0.0 0.0
  1.0
  59 0 1.0 0.0 0.0
EndBehavior 1
RenderMode 2
RayTraceEffects 0
Resolution 3
Overscan 1
Letterbox 1
PixelAspectRatio 2
Antialiasing 0
AdaptiveSampling 1
AdaptiveThreshold 8
FilmSize 2
FieldRendering 1
MotionBlur 0
DepthOfField 0

BGImage WCSFrames:Molten001 (sequence)
ImageSequenceInfo 0 0 30
SolidBackdrop 1
BackdropColor 0 0 0
ZenithColor 80 130 255
SkyColor 255 255 255
GroundColor 50 40 30
NadirColor 80 130 255
FogType 1
FogMinDist 0.011100
FogMaxDist (envelope)
  1
  2
  19999.000000
  0 0 0.0 0.0 0.0
  8000.000000
  31 0 0.0 0.0 0.0
EndBehavior 1
FogColor 129 21 180
BackdropFog 0
DitherIntensity 1
AnimatedDither 0

SaveRGBImagesPrefix WCSFrames:EcoEditor

ViewMode 5
ViewAimpoint 0.000000 0.000000 0.000000
ViewDirection 0.000000 0.000000 0.000000
ViewZoomFactor 3.200000
LayoutGrid 3
GridSize 20000.000000
ShowObjects 1
ShowBones 1
ShowLights 1
ShowCamera 1
ShowMotionPath 1
ShowSafeAreas 0
ShowBGImage 0
ShowFogRadius 1
ShowRedraw 0

*/


// sample LWS (version3-5) file written by WCS6
/*
LWSC
1

FirstFrame 0
LastFrame 0
FrameStep 1

FramesPerSecond 30.000000

AddNullObject WCSMasterNull
ObjectMotion (unnamed)
9
1
0.000000 0.000000 0.000000 0.0 0.0 0.0 1.0 1.0 1.0
0 0 0.0 0.0 0.0
EndBehavior 1
ShadowOptions 7

AddNullObject WCSCameraTargetNull
ObjectMotion (unnamed)
9
1
-4056.650122 3572.040575 -4607.006787 0.0 0.0 0.0 1.0 1.0 1.0
0 0 0.000000 0.000000 0.000000
EndBehavior 1
ParentObject 1

LoadObject Objects\Spires.A.LWO
ObjectMotion (unnamed)
9
1
-22503.757188 1984.991293 -13921.504119 0.0 0.0 0.0 1.0 1.0 1.0
0 0 0.0 0.0 0.0
EndBehavior 1
ParentObject 1
ShadowOptions 7

LoadObject Objects\Spires.B.LWO
ObjectMotion (unnamed)
9
1
-36.339628 2024.711206 -13950.495554 0.0 0.0 0.0 1.0 1.0 1.0
0 0 0.0 0.0 0.0
EndBehavior 1
ParentObject 1
ShadowOptions 7

AmbientColor 131 170 255
AmbIntensity 0.129412

AddLight
LightName BlueMesa Sun
LightMotion (unnamed)
9
1
-62061648795.461792 92608158658.310471 -99761553312.473694 0.0 0.0 0.0 1.0 1.0 1.0
0 0 0.000000 0.000000 0.000000
EndBehavior 1
ParentObject 1
LightColor 255 255 232
LightType 1
Falloff 0.000000
LensFlare 0
ShadowCasting 1

CameraMotion (unnamed)
9
1
10570.113429 2445.075981 -3822.055855 -93.071854 -4.399522 0.039565 1.000000 1.000000 1.000000
0 0 0.000000 0.000000 0.000000
EndBehavior 1
ParentObject 1
ZoomFactor 4.422229
RenderMode 2
RayTraceEffects 0
Resolution 1
CustomSize 720 480
PixelAspectRatio -1
CustomPixelRatio 1.000000
Antialiasing 0
AdaptiveSampling 1
AdaptiveThreshold 8
FilmSize 2
FieldRendering 0
MotionBlur 0
DepthOfField 0

SolidBackdrop 1
BackdropColor 0 0 0
ZenithColor 89 118 178
SkyColor 138 177 255
GroundColor 50 40 30
NadirColor 138 177 255
FogType 1
FogMinDist 7524.337339
FogMaxDist 36100.000000
FogMinAmount 0.000000
FogMaxAmount 1.000000
FogColor 98 119 204
DitherIntensity 1
AnimatedDither 0

SaveRGBImagesPrefix WCSFrames:BlueMesaLW

ViewMode 5
ViewAimPoint 0.000000 0.000000 0.000000
ViewDirection 0.000000 0.000000 0.000000
ViewZoomFactor 3.200000
LayoutGrid 3
GridSize 500.000000
ShowObjects 1
ShowBones 1
ShowLights 0
ShowCamera 1
ShowMotionPath 1
ShowSafeAreas 0
ShowBGImage 0
ShowFogRadius 1
ShowRedraw 0
*/


// Above file rewritten by LW 7.5
// see also http://www.lightwave3d.com/developer/75lwsdk/docs/filefmts/lwsc.html
/*
LWSC
3

FirstFrame 0
LastFrame 0
FrameStep 1
PreviewFirstFrame 0
PreviewLastFrame 1
PreviewFrameStep 1
CurrentFrame 0
FramesPerSecond 30

AddNullObject WCSMasterNull
ShowObject 6 3
ObjectMotion
NumChannels 9
Channel 0
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 1
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 2
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 3
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 4
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 5
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 6
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 7
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 8
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}

ShadowOptions 7

AddNullObject WCSCameraTargetNull
ShowObject 6 3
ObjectMotion
NumChannels 9
Channel 0
{ Envelope
  1
  Key -4056.6501 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 1
{ Envelope
  1
  Key 3572.0405 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 2
{ Envelope
  1
  Key -4607.0068 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 3
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 4
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 5
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 6
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 7
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 8
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}

ParentItem 10000000
ShadowOptions 7

LoadObjectLayer 1 Objects/Spires.A.LWO
ShowObject 6 3
ObjectMotion
NumChannels 9
Channel 0
{ Envelope
  1
  Key -22503.758 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 1
{ Envelope
  1
  Key 1984.9913 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 2
{ Envelope
  1
  Key -13921.504 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 3
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 4
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 5
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 6
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 7
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 8
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}

ParentItem 10000000
ShadowOptions 7

LoadObjectLayer 1 Objects/Spires.B.LWO
ShowObject 6 3
ObjectMotion
NumChannels 9
Channel 0
{ Envelope
  1
  Key -36.339626 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 1
{ Envelope
  1
  Key 2024.7112 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 2
{ Envelope
  1
  Key -13950.495 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 3
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 4
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 5
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 6
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 7
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 8
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}

ParentItem 10000000
ShadowOptions 7

AmbientColor 0.5137255 0.6666667 1
AmbientIntensity 0.129412

AddLight
LightName BlueMesa Sun
ShowLight 1 5
LightMotion
NumChannels 9
Channel 0
{ Envelope
  1
  Key -6.206165e+010 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 1
{ Envelope
  1
  Key 9.260816e+010 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 2
{ Envelope
  1
  Key -9.9761553e+010 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 3
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 4
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 5
{ Envelope
  1
  Key 0 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 6
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 7
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 8
{ Envelope
  1
  Key 1 0 0 0 0 0 0 0 0
  Behaviors 1 1
}

ParentItem 10000000
LightColor 1 1 0.9098039
LightIntensity 1
AffectCaustics 1
LightType 1
ShadowType 1
ShadowColor 0 0 0

AddCamera
CameraName Camera
ShowCamera 1 0
CameraMotion
NumChannels 6
Channel 0
{ Envelope
  1
  Key 10570.113 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 1
{ Envelope
  1
  Key 2445.0759 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 2
{ Envelope
  1
  Key -3822.0559 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 3
{ Envelope
  1
  Key -1.6244103 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 4
{ Envelope
  1
  Key -0.076786138 0 0 0 0 0 0 0 0
  Behaviors 1 1
}
Channel 5
{ Envelope
  1
  Key 0.00069053948 0 0 0 0 0 0 0 0
  Behaviors 1 1
}

ParentItem 10000000
ZoomFactor 4.422229
ResolutionMultiplier 1.0
FrameSize 720 480
PixelAspect 1
MaskPosition 0 0 640 480
MotionBlur 0
FieldRendering 0

ApertureHeight 0.015
Antialiasing 0
AdaptiveSampling 1
AdaptiveThreshold 0.03137255

SolidBackdrop 1
BackdropColor 0 0 0
ZenithColor 0.3490196 0.4627451 0.6980392
SkyColor 0.5411765 0.6941177 1
GroundColor 0.1960784 0.1568628 0.1176471
NadirColor 0.5411765 0.6941177 1
FogType 1
FogMinDistance 7524.337
FogMaxDistance 36100
FogMinAmount 0
FogMaxAmount 1
FogColor 0.3843137 0.4666667 0.8
BackdropFog 0
DitherIntensity 1
AnimatedDither 0

RenderMode 2
RayTraceEffects 0
RayRecursionLimit 16
RayTraceOptimization 1
DataOverlayLabel  
OutputFilenameFormat 1
SaveRGB 0
SaveRGBImagesPrefix WCSFrames:BlueMesaLW
RGBImageSaver _ILBM
SaveAlpha 0

ViewConfiguration 0
DefineView 0
ViewType 9
ViewAimpoint 0 0 0
ViewZoomFactor 3.2

GridNumber 10
GridSize 500
CameraViewBG 0
ShowMotionPath 1
ShowFogRadius 1
ShowFogEffect 1
ShowSafeAreas 0
ShowFieldChart 0

CurrentObject 0
CurrentLight 0
CurrentCamera 0
GraphEditorData
{ GraphEd_Favorites
}
{ GE_Expression_Lib
  1
}

*/
