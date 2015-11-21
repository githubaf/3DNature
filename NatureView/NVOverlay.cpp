
#include <osg/PositionAttitudeTransform>
#include "UsefulUnit.h"
#include "UsefulTime.h"
#include "NVOverlay.h"
#include "CameraSupport.h"
#include "NVScene.h"
#include "NVTerrain.h"

extern NVScene MasterScene;

extern double LastMovementMoment;
extern unsigned long int DebugVarTotalTreeCountFile;
extern unsigned long int DebugVarTotalTreeCountLoaded;


extern InfoDlg *GlobalInfoWindow;



void NVOverlay::SetOverlayText(std::string NewText)
{
OverlayText = NewText;
if(GlobalInfoWindow)
	{
	GlobalInfoWindow->SetNewText(OverlayText.c_str());
	} // if
} // NVOverlay::SetOverlayText



void NVOverlay::UpdateMapPoint(double WorldXCoord, double WorldYCoord, float Heading) // heading is in radians
{
if(MapPointerTransform.valid())
	{
	if(WorldXCoord >= MasterScene.GetCoordXMin() && WorldXCoord <= MasterScene.GetCoordXMax() && WorldYCoord >= MasterScene.GetCoordYMin() && WorldYCoord <= MasterScene.GetCoordYMax()
	 && MasterScene.GetCoordXRange() != 0.0 && MasterScene.GetCoordYRange() != 0.0)
		{
		float XFrac, YFrac;
		unsigned long int XPix, YPix;

		XFrac = (WorldXCoord - MasterScene.GetCoordXMin()) / MasterScene.GetCoordXRange();
		if(1) // if geographic <<<>>>
			{
			XFrac = 1.0 - XFrac; // flip L/R for WCS pos=west geographic notation
			} // if
		YFrac = (WorldYCoord - MasterScene.GetCoordYMin()) / MasterScene.GetCoordYRange();
		XPix = XFrac * (MapRight - MapLeft);
		YPix = YFrac * (MapTop - MapBot);
		MapPointerTransform->setPosition(osg::Vec3( MapLeft + XPix, MapBot + YPix, 0.0));
		MapPointerTransform->setAttitude(osg::Quat(Heading, osg::Vec3(0, 0, 1)));
		} // if
	else
		{ // move indicator out of sight offscreen
		MapPointerTransform->setPosition(osg::Vec3( -10000, -10000, 0.0));
		} // else
	} // if
} // NVOverlay::UpdateMapPoint

#include "PanoManipulator.h"
extern osg::ref_ptr<PanoManipulator> PM; // for JumpTo


bool NVOverlay::HandlePotentialMapClick(float RawMouseX, float RawMouseY)
{
float FracMouseX, FracMouseY;

FracMouseX = (RawMouseX + 1.0f) * .5f;
FracMouseY = (RawMouseY + 1.0f) * .5f;

if(GetMapOn())
	{
	// these coords increase towards the right and up
	unsigned long int ClickPseudoCoordX, ClickPseudoCoordY;
	ClickPseudoCoordX = (unsigned long int)(FracMouseX * (float)NVW_NVOVERLAY_PSEUDO_SCREENSIZE_X);
	ClickPseudoCoordY = (unsigned long int)(FracMouseY * (float)NVW_NVOVERLAY_PSEUDO_SCREENSIZE_Y);
	
	if(ClickPseudoCoordX > MapLeft && ClickPseudoCoordX < MapRight && ClickPseudoCoordY > MapBot && ClickPseudoCoordY < MapTop)
		{
		// translate into image-fraction units
		float ImageFracX, ImageFracY;
		// these units also increase to the right and up
		ImageFracX = ((float)ClickPseudoCoordX - (float)MapLeft) / (float)(MapRight - MapLeft);
		ImageFracY = ((float)ClickPseudoCoordY - (float)MapBot) / (float)(MapTop - MapBot);
		
		// figure out the new spatial position represented by these map fractional coordinates
		double SpatialX, SpatialY;
		
		SpatialX = ((1.0f - ImageFracX) * MasterScene.GetCoordXRange()) + MasterScene.GetCoordXMin(); // Lon increases to left so must be inverted by 1.0 -
		SpatialY = (ImageFracY * MasterScene.GetCoordYRange()) + MasterScene.GetCoordYMin();
		PM->JumpTo(SpatialX, SpatialY);
		return(true);
		} // if
	} // if
return(false);
} // NVOverlay::HandlePotentialMapClick



// command strings:
// &C = camera, followed by N, F, H, P, B, X, Y, or Z (name, fov, heading, pitch, bank, longitude, latitude, elevation)
//   &CC -- camera compass heading, &CT -- camera Tilt
// &I = italic mode toggle {NeHe font switch} (defaults to off each scan line)
// &F = frame, followed by N or S (frame number, or SMPTE format {N option is followed by a digit ['1'..'9']})
// &J = justify, followed by C, L, N, or R (center, left, none, right)
// &PN = project name
// &RD = date and time of render
// &RN = render opts name
// &T = target, followed by X, Y, or Z (longitude, latitude, elevation)
// &TD -- distance to target
// &UN = user name
// &UE = user email
//    note: numeric fields are followed by %n where n is '0' to '9' {# digits after decimal}
// CR/LF pair expected to separate lines of text
// use && to print '&', all other chars printed
// ===
// added units interpretation for Camera Elevation, Target Elevation, and Target Distance
// unit suffixes are as found in UsefulUnitTextSuffixes and must follow the digits field
// ie: &CZ%2ft will display the camera altitude in feet to two decimal places

std::string NVOverlay::GetProcessedText(void)
{
return(GetProcessedText(OverlayTextTemplate));
} // NVOverlay::GetProcessedText


std::string NVOverlay::GetProcessedText(std::string InputText)
{ // do string substitution on all the special embedded characters
// lots of code taken from WCS/VNS's PostProcessEvent.cpp
char MesgText[2048];
char tmp[256], tmp2[256];
std::string OutString;
char *ch, n;
long i = 0;
double dval;
int unitid;
unsigned long unitchar;
char units[8];
long FrameNum = 0;
float XTemp = 0.0, YTemp = 0.0, ZTemp = 0.0;
double ElevAGL;


strncpy(MesgText, InputText.c_str(), 2046);
MesgText[2047] = 0;

ch = &MesgText[0];
memset(tmp, 0, sizeof(tmp));
//italics = FALSE;
while (*ch != 0)
	{
	switch (*ch)
		{
		case 13:	// expecting a CR/LF pair
			ch++;
			if (*ch != 10)
				break;
			ch++;
			tmp[i] = 0;
			if (tmp[0])
				{
				//FI->PrintText(x, y, tmp, 210, 255, 21, justify, WCS_FONTIMAGE_DRAWMODE_NORMAL, leading, kerning, edging, this);
				OutString.append(tmp);
				OutString.append("\n");
				} // if
			memset(tmp, 0, sizeof(tmp));
			i = 0;
			//x = 0;
			//y += fontysize + leading;
			//italics = FALSE;
			break;
		case '&':	// command string mode
			ch++;
			if (*ch == '&')
				{
				*ch++;
				break;
				} // if
			else if ((*ch != 'C') && (*ch != 'F') && (*ch != 'I') && (*ch != 'J') && (*ch != 'T') && (*ch != 'R') && (*ch != 'P') && (*ch != 'U') && (*ch != '*') && (*ch != '#'))
				break;
			n = '2';	// default # decimal digits to print
			switch (*ch++)
				{
				case 'C':
					if ((*ch != 'B') && (*ch != 'E') && (*ch != 'F') && (*ch != 'H') && (*ch != 'N') && (*ch != 'P') && (*ch != 'X') && (*ch != 'Y') && (*ch != 'Z') && (*ch != 'C') && (*ch != 'T'))
						break;
					switch (*ch++)
						{
						case 'B':	// bank
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = MasterScene.GetCamBank(MasterScene.GetCurrentCameraNum());
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'E':	// Elevation AGL
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = 0.0f;
							GetCurCameraCurXYZ(XTemp, YTemp, ZTemp);
							if(FetchTerrainHeight(NULL, XTemp, YTemp, ElevAGL))
								{
								dval = (ZTemp - ElevAGL);
								} // if
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'F':	// fov
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							GetCurCameraCurFov(XTemp, YTemp);
							dval = XTemp;
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'N':	// name
							strcpy(tmp2, MasterScene.GetCameraName(MasterScene.GetCurrentCameraNum()));
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'X':	// longitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							GetCurCameraCurXYZ(XTemp, YTemp, ZTemp);
							dval = XTemp;
							sprintf(tmp2, "%c%.*f", n - '0', (dval < 0.0 ? 'S' : 'N'), fabs(dval));
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'Y':	// latitude
							{
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							GetCurCameraCurXYZ(XTemp, YTemp, ZTemp);
							dval = YTemp;
							sprintf(tmp2, "%c%.*f", n - '0', (dval < 0.0 ? 'E' : 'W'), fabs(dval));
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
							} // Y/latitude
						case 'Z':	// elevation
							unitid = WCS_USEFUL_UNIT_METER;
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								// look for unit suffix
								unitchar = 0;
								while ((*ch != 0) && (*ch != 13) && (*ch != ' ') && (*ch != '&') && (unitchar < 7))
									{
									units[unitchar++] = *ch++;
									} // while
								units[unitchar] = 0;
								unitid = MatchUnitSuffix(units);
								if (unitid < 0)
									unitid = WCS_USEFUL_UNIT_METER;
								} // if
							GetCurCameraCurXYZ(XTemp, YTemp, ZTemp);
							dval = ZTemp;
							if (unitid != WCS_USEFUL_UNIT_METER)
								{
								dval = ConvertFromMeters(dval, unitid);
								} // if
							sprintf(tmp2, "%.*f%s", n - '0', dval, GetUnitSuffix(unitid));
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						// these are basically equivalent within NVE
						case 'C':	// compass heading
						case 'H':	// heading
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							GetCurCameraCurHeadingRadians(XTemp);
							dval = osg::RadiansToDegrees(-XTemp);
							if(dval < 0) dval += 360.0f;
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
/*
						// these are basically equivalent within NVE
						case 'T':	// tilt
						case 'P':	// pitch
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							//dval = Rend->Cam->CamPitch;
							dval = 123.4; // <<<>>>
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
*/
						} // switch camera subchar
					break;
/*
				case 'F':	// frame number options
					if ((*ch != 'N') && (*ch != 'S'))
						break;
					switch (*ch++)
						{
						case 'N':	// frame number
							if ((*ch < '1') || (*ch > '9'))
								break;
							n = *ch++;
							sprintf(tmp2, "%0*u", n - '0', FrameNum);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'S':	// SMPTE type timecode
							unsigned long frate, h, m, s, f, t;
							//frate = (unsigned long)(Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + 0.5);
							frate = 30.0; // <<<>>>
							if (frate != 0)
								{
								f = (unsigned long)FrameNum % frate;
								t = (unsigned long)FrameNum / frate;	// this frames time in seconds at this frame rate
								h = t / 3600;
								m = (t % 3600) / 60;
								s = t % 60;
								sprintf(tmp2, "%02u:%02u:%02u:%02u", h, m, s, f);
								}
							else
								sprintf(tmp2, "00:00:00:00");
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						default:
							break;
						}
					break;
*/
				case 'P':	// project
					if (*ch != 'N')
						break;
					ch++;
					strcpy(tmp2, MasterScene.GetMetaName());
					strcat(tmp, tmp2);
					i = strlen(tmp);
					break;
				case 'R':	// render opt name (actually Export options name in User1)
					if ((*ch != 'N') && (*ch != 'D'))
						break;
					switch (*ch++)
						{
						case 'N':
							strcpy(tmp2, MasterScene.GetMetaUser1());
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'D':
							time_t ltime;
							struct tm *now;
							time(&ltime);
							now = localtime(&ltime);
							strftime(tmp2, sizeof(tmp2), "%c", now);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						default:
							break;
						}
					break;
				/*
				case 'T':	// target options
					if ((*ch != 'X') && (*ch != 'Y') && (*ch != 'Z') && (*ch != 'D'))
						break;
					switch (*ch++)
						{
						case 'X':	// longitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							//dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetCurValue();
							dval = 123.4; // <<<>>>
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'Y':	// latitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							//dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetCurValue();
							dval = 123.4; // <<<>>>
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'Z':	// elevation
							unitid = WCS_USEFUL_UNIT_METER;
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								// look for unit suffix
								unitchar = 0;
								while ((*ch != 0) && (*ch != 13) && (*ch != ' ') && (*ch != '&') && (unitchar < 7))
									{
									units[unitchar++] = *ch++;
									} // while
								units[unitchar] = 0;
								unitid = MatchUnitSuffix(units);
								if (unitid < 0)
									unitid = WCS_USEFUL_UNIT_METER;
								} // if
							//dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetCurValue();
							dval = 123.4; // <<<>>>
							if (unitid != WCS_USEFUL_UNIT_METER)
								dval = ConvertFromMeters(dval, unitid);
							sprintf(tmp2, "%.*f%s", n - '0', dval, GetUnitSuffix(unitid));
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'D':	// distance
							unitid = WCS_USEFUL_UNIT_METER;
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								// look for unit suffix
								unitchar = 0;
								while ((*ch != 0) && (*ch != 13) && (*ch != '&') && (unitchar < 7))
									{
									units[unitchar++] = *ch++;
									} // while
								units[unitchar] = 0;
								unitid = MatchUnitSuffix(units);
								if (unitid < 0)
									unitid = WCS_USEFUL_UNIT_METER;
								} // if
							//dval = TargDist;
							dval = 123.4; // <<<>>>
							if (unitid != WCS_USEFUL_UNIT_METER)
								dval = ConvertFromMeters(dval, unitid);
							sprintf(tmp2, "%.*f%s", n - '0', dval, GetUnitSuffix(unitid));
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						default:
							break;
						}
					break;
*/
				case 'U':	// user
					if ((*ch != 'N') && (*ch != 'E') && (*ch != 'C') && (*ch != '1') && (*ch != '2') && (*ch != '3') && (*ch != '4') && (*ch != '5'))
						break;
					switch (*ch++)
						{
						case 'N':	// Name
							strcpy(tmp2, MasterScene.GetMetaAuthor());
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'E':	// EMail
							strcpy(tmp2, MasterScene.GetMetaEMail());
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case '1':	// User1
							strcpy(tmp2, MasterScene.GetMetaUser1());
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case '2':	// User2
							strcpy(tmp2, MasterScene.GetMetaUser2());
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case '3':	// User3
							strcpy(tmp2, MasterScene.GetMetaUser3());
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case '4':	// User4
							strcpy(tmp2, MasterScene.GetMetaUser4());
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case '5':	// User5
							strcpy(tmp2, MasterScene.GetMetaUser5());
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'C':	// Copyright
							strcpy(tmp2, MasterScene.GetMetaCopyright());
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						default:
							break;
						}
					break;
				case '*':	// newline
						strcat(tmp, "\n");
						i = strlen(tmp);
					break;
				case '#':	// internal testing numeric variables
					if ((*ch != 'P') && (*ch != 'F') && (*ch != 'S') && (*ch != 'T'))
						break;
					char VarSym = *ch++; // stash for later parsing
					if (*ch == '%')
						{
						ch++;
						if ((*ch < '0') || (*ch > '9'))
							break;
						n = *ch++;
						}
					switch (VarSym)
						{
						case 'P':	// MinFeatureSizePixels
							dval = MasterScene.SceneLOD.GetMinFeatureSizePixels();
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'F':	// Total stems of foliage in file
							dval = DebugVarTotalTreeCountFile;
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'S':	// Total stems of foliage in scene
							dval = DebugVarTotalTreeCountLoaded;
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						case 'T':	// OptimizeMove Settle Time
							dval = GetSystemTimeFP() - LastMovementMoment;
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = strlen(tmp);
							break;
						default:
							break;
						} // switch VarSym
					break;
				} // switch
			break;
		default:
			tmp[i++] = *ch++;
			break;
		} // switch
	} // while

if (tmp[0])
	{
	//FI->PrintText(x, y, tmp, 210, 255, 21, justify, WCS_FONTIMAGE_DRAWMODE_NORMAL, leading, kerning, edging, this);
	OutString.append(tmp);
	//y += fontysize + leading;
	}

return(OutString);
} // NVOverlay::GetProcessedText

