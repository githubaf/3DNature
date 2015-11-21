// NVScene.h

#ifndef NVW_NVSCENE_H
#define NVW_NVSCENE_H

#include "Defines.h"
#include "UsefulGeo.h"

#include "NVAnimObject.h"
#include "NVFoliage.h"
#include "NVOverlay.h"
#include "NVWatermark.h"
#include "SceneLOD.h"
#include "PartialVertexDEM.h"
#include "DEMCore.h"

typedef std::vector <NVAnimObject *> CameraList, OceanList, ObjectList, HazeList, SkyList, LightList;


class NVScene
	{
	public:
		enum HorizontalAlignment
			{
			Left,
			Center,
			Right,
			HA_MAX // never use this entry
			}; // enum HorizontalAlignment

	private:
		HorizontalAlignment LogoAlign;
		char SceneName[255], DEMName[255], DrapeImageNameA[255], DrapeImageNameB[255], FolDrapeImageName[255], DEMSig[255], DrapeImageSigA[255], DrapeImageSigB[255], FolDrapeImageSig[255], LogoImageName[255], LogoImageSig[255], FolFileName[255], FolFileSig[255], ObjFileName[255];
		char MetaName[255], MetaCopyright[255], MetaAuthor[255], MetaEMail[255], MetaUser1[255], MetaUser2[255], MetaUser3[255], MetaUser4[255], MetaUser5[255];
		std::string ObjectIndexFile, ObjectRecordsFile, ObjectRecordsSig, ActionIndexFile, ActionRecordsFile, ActionRecordsSig;
		double TerrainMaxElev, TerrainMinElev;
		bool TerrainIsPageable, ImageIsPageable;
		std::string NoticeFile, NoticeText, NoticeSig;
		bool FastActionCheckOk, NoticeModal;
		float Speed, MaxSpeed, ThrottleSpeed, HotNavFwdBackAmount, HotNavSideAmount, HotTurnHAmount, HotTurnVAmount, Acceleration, Friction, Inertia, FollowTerrainHeight, FollowTerrainHeightMax, SplashTime, NoticeTime;
		std::string SceneFileSig;
		std::string SceneFileWorkingPath;
		bool FollowTerrainEnabled, Constrain;
		unsigned char FolType;
		double ClipNear, ClipFar;
		float CameraHFOV;
		int CurrentCameraNum;
		double XMax, YMax, XMin, YMin; // in local
		double TerXMax, TerYMax, TerXMin, TerYMin; // terrain-only, in local
		double CoordXMax, CoordYMax, CoordXMin, CoordYMin; // in geographic for now

		std::vector <NVFolSpecies *> FolSpecies;

		// Projection variables
		double LonScaleFac, LatScaleFac, LocalOriginLat, LocalOriginLon;

		// The (currently only) DEM
		DEM *Terrain;

	public:
		NVOverlay Overlay;
		NVWatermark Watermark;
		NVLOD SceneLOD;

		std::vector <NVAnimObject *> Cameras, Oceans, Objects, Hazes, Skies, Lights;

		NVScene()
			{
			LogoAlign = Right; // by default
			SceneName[0] = DEMName[0] = DrapeImageNameA[0] = DrapeImageNameB[0] = FolDrapeImageName[0] = DEMSig[0] = DrapeImageSigA[0] = DrapeImageSigB[0] = FolDrapeImageSig[0] = LogoImageName[0] = LogoImageSig[0] = FolFileName[0] = FolFileSig[0] = ObjFileName[0] = 0;
			MetaName[0] = MetaCopyright[0] = MetaAuthor[0] = MetaEMail[0] = MetaUser1[0] = MetaUser2[0] = MetaUser3[0] = MetaUser4[0] = MetaUser5[0] = 0;
			Speed = 0.0; MaxSpeed = -FLT_MAX, ThrottleSpeed = 0.0; HotNavFwdBackAmount = HotNavSideAmount = HotTurnHAmount = HotTurnVAmount = 0.0f; Friction = 0.1f; Acceleration = 1.0; SetConstrain(false); SetInertia(1.0f); FollowTerrainHeight = 4.0; FollowTerrainHeightMax = -FLT_MAX; SplashTime = NVW_SPLASH_DISPLAY_SECONDS; NoticeTime = 0;
			CameraHFOV = 45.0f;
			TerrainMaxElev = FLT_MAX;
			TerrainMinElev = -FLT_MAX;
			TerrainIsPageable = false;
			ImageIsPageable = false;
			FollowTerrainEnabled = true;
			NoticeModal = false;
			FolType = 0; // Billboards
			XMax = YMax = 0; XMin = YMin = FLT_MAX;
			TerXMax = TerYMax = 0; TerXMin = TerYMin = FLT_MAX;
			CoordXMax = CoordYMax = -FLT_MAX; CoordXMin = CoordYMin = FLT_MAX;
			FastActionCheckOk = false;
			};

		const char *GetSceneName(void) {return(SceneName);};
		const char *GetDEMName(void) {return(DEMName);};
		const char *GetDrapeImageName(void) {return(DrapeImageNameA);};
		const char *GetDrapeImageNameA(void) {return(DrapeImageNameA);};
		const char *GetDrapeImageNameB(void) {return(DrapeImageNameB);};
		const char *GetFolDrapeImageName(void) {return(FolDrapeImageName);};
		const char *GetDEMSig(void) {return(DEMSig);};
		const char *GetDrapeImageSigA(void) {return(DrapeImageSigA);};
		const char *GetDrapeImageSigB(void) {return(DrapeImageSigB);};
		const char *GetFolDrapeImageSig(void) {return(FolDrapeImageSig);};
		const char *GetLogoImageName(void) {return(LogoImageName);};
		const char *GetLogoImageSig(void) {return(LogoImageSig);};
		HorizontalAlignment GetLogoAlign(void) {return(LogoAlign);};
		const char *GetFolFileName(void) {return(FolFileName);};
		const char *GetFolFileSig(void) {return(FolFileSig);};
		const char *GetObjFileName(void) {return(ObjFileName);};
		const char *GetSkyName(int ItemNum = 0) {return(Skies[ItemNum]->GetName());};
		const char *GetSkyFileName(int ItemNum = 0) {return(Skies[ItemNum]->GetStringTime(NVAnimObject::STRING_FILEA, 0.0f));};
		const char *GetSkySig(int ItemNum = 0) {return(Skies[ItemNum]->GetStringTime(NVAnimObject::STRING_SIGA, 0.0f));};
		const char *GetOceanName(int ItemNum = 0) {return(Oceans[ItemNum]->GetName());};
		const char *GetOceanFileName(int ItemNum = 0) {return(Oceans[ItemNum]->GetStringTime(NVAnimObject::STRING_FILEA, 0.0f));};
		const char *GetCameraName(int ItemNum = 0) {return(Cameras[ItemNum]->GetName());};
		const char *GetLightName(int ItemNum = 0) {return(Lights[ItemNum]->GetName());};

		double GetLightPosLat(int ItemNum = 0) {return(Lights[ItemNum]->GetChannelValueTime(NVAnimObject::CANONY, 0.0f));};
		double GetLightPosLon(int ItemNum = 0) {return(Lights[ItemNum]->GetChannelValueTime(NVAnimObject::CANONX, 0.0f));};
		double GetLightPosElev(int ItemNum = 0) {return(Lights[ItemNum]->GetChannelValueTime(NVAnimObject::CANONZ, 0.0f));};

		const char *GetMetaName(void) {return(MetaName);};
		const char *GetMetaAuthor(void) {return(MetaAuthor);};
		const char *GetMetaEMail(void) {return(MetaEMail);};
		const char *GetMetaCopyright(void) {return(MetaCopyright);};
		const char *GetMetaUser1(void) {return(MetaUser1);};
		const char *GetMetaUser2(void) {return(MetaUser2);};
		const char *GetMetaUser3(void) {return(MetaUser3);};
		const char *GetMetaUser4(void) {return(MetaUser4);};
		const char *GetMetaUser5(void) {return(MetaUser5);};
		
		double GetTerrainMaxElev(void) const {return(TerrainMaxElev);};
		double GetTerrainMinElev(void) const {return(TerrainMinElev);};
		bool CheckTerrainMaxElev(void) const {return(TerrainMaxElev != FLT_MAX);};
		bool CheckTerrainMinElev(void) const {return(TerrainMinElev != -FLT_MAX);};
		void SetTerrainMaxElev(double NewVal) {TerrainMaxElev = NewVal;};
		void SetTerrainMinElev(double NewVal) {TerrainMinElev = NewVal;};

		bool GetTerrainIsPageable(void) const {return(TerrainIsPageable);};
		void SetTerrainIsPageable(bool NewVal) {TerrainIsPageable = NewVal;};
		bool GetImageIsPageable(void) const {return(ImageIsPageable);};
		void SetImageIsPageable(bool NewVal) {ImageIsPageable = NewVal;};
		
		// this method caches the result after the first query
		bool CheckAllActionDataPresent(void) {return(FastActionCheckOk || (FastActionCheckOk = (!ObjectIndexFile.empty() && !ObjectRecordsFile.empty() && !ActionIndexFile.empty() && !ActionRecordsFile.empty() /* && !ObjectRecordsSig.empty() && !ActionRecordsSig.empty() */ )));};

		const std::string &GetObjectIndexFile(void) const {return(ObjectIndexFile);};
		const std::string &GetObjectRecordsFile(void) const {return(ObjectRecordsFile);};
		const std::string &GetActionIndexFile(void) const {return(ActionIndexFile);};
		const std::string &GetActionRecordsFile(void) const {return(ActionRecordsFile);};
		const std::string &GetObjectRecordsSig(void) const {return(ObjectRecordsSig);};
		const std::string &GetActionRecordsSig(void) const {return(ActionRecordsSig);};

		void SetObjectIndexFile(const std::string NewValue) {ObjectIndexFile = NewValue;};
		void SetObjectRecordsFile(const std::string NewValue) {ObjectRecordsFile = NewValue;};
		void SetActionIndexFile(const std::string NewValue) {ActionIndexFile = NewValue;};
		void SetActionRecordsFile(const std::string NewValue) {ActionRecordsFile = NewValue;};
		void SetObjectRecordsSig(const std::string NewValue) {ObjectRecordsSig = NewValue;};
		void SetActionRecordsSig(const std::string NewValue) {ActionRecordsSig = NewValue;};
		
		const std::string &GetSceneFileWorkingPath(void) const {return(SceneFileWorkingPath);};
		void SetSceneFileWorkingPath(const std::string NewValue) {SceneFileWorkingPath = NewValue;};

		// Notice
		const std::string &GetNoticeSig(void) const {return(NoticeSig);};
		const std::string &GetNoticeText(void) const {return(NoticeText);};
		const std::string &GetNoticeFile(void) const {return(NoticeFile);};
		const bool GetNoticeModal(void) const {return(NoticeModal);};

		void SetNoticeSig(const std::string NewValue) {NoticeSig = NewValue;};
		void SetNoticeText(const std::string NewValue) {NoticeText = NewValue;};
		void SetNoticeFile(const std::string NewValue) {NoticeFile = NewValue;};
		void SetNoticeModal(const bool NewValue) {NoticeModal = NewValue;};

		
		float GetSpeed(void) {return(Speed);};
		float GetMaxSpeed(void) {return(MaxSpeed);};
		float GetThrottleSpeed(void) {return(ThrottleSpeed);};
		float GetHotNavFwdBackAmount(void) {return(HotNavFwdBackAmount);};
		float GetHotNavSideAmount(void) {return(HotNavSideAmount);};
		float GetHotTurnHAmount(void) {return(HotTurnHAmount);};
		float GetHotTurnVAmount(void) {return(HotTurnVAmount);};
		float GetFriction(void) {return(Friction);};
		float GetInertia(void) {return(Inertia);};
		float GetAcceleration(void) {return(Acceleration);};
		float GetSplashTime(void) {return(SplashTime);};
		float GetNoticeTime(void) {return(NoticeTime);};
		bool  GetConstrain(void) {return(Constrain);};
		float GetFollowTerrainHeight(void) {return(FollowTerrainHeight);};
		float GetFollowTerrainHeightMax(void) {return(FollowTerrainHeightMax);};
		float GetOceanSpec(int ItemNum = 0) {return(Oceans[ItemNum]->GetChannelValueTime(NVAnimObject::CANONINTENSITYA, 0.0f));};
		float GetOceanElev(int ItemNum = 0) {return(Oceans[ItemNum]->GetChannelValueTime(NVAnimObject::CANONZ, 0.0f));};
		const char *GetSig(void) {return(SceneFileSig.c_str());};
		double GetXMax(void) {return(XMax);};
		double GetXMin(void) {return(XMin);};
		double GetYMax(void) {return(YMax);};
		double GetYMin(void) {return(YMin);};
		double GetXRange(void) {return(XMax - XMin);};
		double GetYRange(void) {return(YMax - YMin);};

		double GetTerXMax(void) {return(TerXMax);};
		double GetTerXMin(void) {return(TerXMin);};
		double GetTerYMax(void) {return(TerYMax);};
		double GetTerYMin(void) {return(TerYMin);};
		double GetTerXRange(void) {return(TerXMax - TerXMin);};
		double GetTerYRange(void) {return(TerYMax - TerYMin);};

		bool CheckCamera(void) {return(!Cameras.empty());}
		bool CheckOcean(void) {return(!Oceans.empty());}
		bool CheckObject(void) {return(!Objects.empty());}
		bool CheckHaze(void) {return(!Hazes.empty());}
		bool CheckSky(void) {return(!Skies.empty());}

		NVAnimObject *GetCamera(int ItemNum = 0) {if(Cameras.empty()) return(NULL); else return(Cameras[ItemNum]);}
		NVAnimObject *GetOcean(int ItemNum = 0) {if(Oceans.empty()) return(NULL); else return(Oceans[ItemNum]);}
		NVAnimObject *GetObject(int ItemNum = 0) {if(Objects.empty()) return(NULL); else return(Objects[ItemNum]);}
		NVAnimObject *GetHaze(int ItemNum = 0) {if(Hazes.empty()) return(NULL); else return(Hazes[ItemNum]);}
		NVAnimObject *GetSky(int ItemNum = 0) {if(Skies.empty()) return(NULL); else return(Skies[ItemNum]);}
		NVAnimObject *GetLight(int ItemNum = 0) {if(Lights.empty()) return(NULL); else return(Lights[ItemNum]);}

		int GetCurrentCameraNum(void) {return(CurrentCameraNum);};
		int GetNumCameras(void) {return(Cameras.size());};
		int SetCurrentCameraNum(int NewCamNum); // range checked, returns success if changed camera
		void SyncAnimFlag(void); // used at startup to switch on animation if needed
		int SelectNextCamera(int HowManyNext = 1);
		int SelectPrevCamera(int HowManyNext = 1);

		int GetNum3DOInstances(void) {return(Objects.size());};
		NVAnimObject *Get3DOInstance(unsigned long int InstNum) {return(Objects[InstNum]);};
		int GetNumLights(void) {return(Lights.size());};
		NVAnimObject *GetLight(unsigned long int InstNum) {return(Lights[InstNum]);};

		double GetCamPosLat(int ItemNum = 0) {return(Cameras[ItemNum]->GetChannelValueTime(NVAnimObject::CANONY, 0.0f));};
		double GetCamPosLon(int ItemNum = 0) {return(Cameras[ItemNum]->GetChannelValueTime(NVAnimObject::CANONX, 0.0f));};
		double GetCamPosElev(int ItemNum = 0) {return(Cameras[ItemNum]->GetChannelValueTime(NVAnimObject::CANONZ, 0.0f));};
		double GetCamHFOV(int ItemNum = 0) {return(Cameras[ItemNum]->GetChannelValueTime(NVAnimObject::CANONHANGLE, 0.0f));};
		double GetCamBank(int ItemNum = 0) {return(Cameras[ItemNum]->GetChannelValueTime(NVAnimObject::CANONB, 0.0f));};
		double GetTargPosLat(int ItemNum = 0) {return(Cameras[ItemNum]->GetChannelValueTime(NVAnimObject::CANONAUXY, 0.0f));};
		double GetTargPosLon(int ItemNum = 0) {return(Cameras[ItemNum]->GetChannelValueTime(NVAnimObject::CANONAUXX, 0.0f));};
		double GetTargPosElev(int ItemNum = 0)  {return(Cameras[ItemNum]->GetChannelValueTime(NVAnimObject::CANONAUXZ, 0.0f));};
		unsigned char GetFolType(void) {return(FolType);};

		double GetCoordXMax(void) {return(CoordXMax);};
		double GetCoordXMin(void) {return(CoordXMin);};
		double GetCoordYMax(void) {return(CoordYMax);};
		double GetCoordYMin(void) {return(CoordYMin);};
		double GetCoordXRange(void) {return(CoordXMax - CoordXMin);};
		double GetCoordYRange(void) {return(CoordYMax - CoordYMin);};

		bool CheckSceneName(void) {return(SceneName[0] ? 1 : 0);};
		bool CheckDEMName(void) {return(DEMName[0] ? 1 : 0);};
		bool CheckDrapeImageNameA(void) {return(DrapeImageNameA[0] ? 1 : 0);};
		bool CheckDrapeImageNameB(void) {return(DrapeImageNameB[0] ? 1 : 0);};
		bool CheckFolDrapeImageName(void) {return(FolDrapeImageName[0] ? 1 : 0);};
		bool CheckDEMSig(void) {return(DEMSig[0] ? 1 : 0);};
		bool CheckDrapeImageSigA(void) {return(DrapeImageSigA[0] ? 1 : 0);};
		bool CheckDrapeImageSigB(void) {return(DrapeImageSigB[0] ? 1 : 0);};
		bool CheckFolDrapeImageSig(void) {return(FolDrapeImageSig[0] ? 1 : 0);};
		bool CheckLogoImageName(void) {return(LogoImageName[0] ? 1 : 0);};
		bool CheckFolFileName(void) {return(FolFileName[0] ? 1 : 0);};
		bool CheckFolFileSig(void) {return(FolFileSig[0] ? 1 : 0);};
		bool CheckObjFileName(void) {return(ObjFileName[0] ? 1 : 0);};
		bool CheckSkyName(int ItemNum = 0) {return(Skies[ItemNum]->CheckName());};
		bool CheckSkyFileName(int ItemNum = 0) {return(Skies[ItemNum]->GetStringTime(NVAnimObject::STRING_FILEA, 0.0f)) ? 1 : 0;};
		bool CheckOceanName(int ItemNum = 0) {return(Oceans[ItemNum]->CheckName());};
		bool CheckOceanFileName(int ItemNum = 0) {return(Oceans[ItemNum]->GetStringTime(NVAnimObject::STRING_FILEA, 0.0f)) ? 1 : 0;};
		bool CheckAnyCameras(void) {return(!Cameras.empty());};
		bool CheckCameraName(int ItemNum = 0) {return(Cameras[ItemNum]->CheckName());};
		bool CheckSpeed(void) {return(Speed != 0.0f);};
		bool CheckMaxSpeed(void) {return(Speed >= 0.0f);}; // maxspeed of 0.0 is allowed
		bool CheckFollowTerrainEnabled(void) {return(FollowTerrainEnabled);};
		bool CheckFollowTerrainHeight(void) {return(FollowTerrainHeight != -FLT_MAX);};
		bool CheckFollowTerrainHeightMax(void) {return(FollowTerrainHeightMax != -FLT_MAX);};
		bool CheckSig(void) {return(!SceneFileSig.empty());};

		void SetSceneName(const char * NewName) {strcpy(SceneName, NewName);};
		void SetDEMName(const char * NewName) {strcpy(DEMName, NewName);};
		void SetDrapeImageName(const char * NewName) {strcpy(DrapeImageNameA, NewName);};
		void SetDrapeImageNameA(const char * NewName) {strcpy(DrapeImageNameA, NewName);};
		void SetDrapeImageNameB(const char * NewName) {strcpy(DrapeImageNameB, NewName);};
		void SetFolDrapeImageName(const char * NewName) {strcpy(FolDrapeImageName, NewName);};
		void SetDEMSig(const char * NewName) {strcpy(DEMSig, NewName);};
		void SetDrapeImageSigA(const char * NewName) {strcpy(DrapeImageSigA, NewName);};
		void SetDrapeImageSigB(const char * NewName) {strcpy(DrapeImageSigB, NewName);};
		void SetFolDrapeImageSig(const char * NewName) {strcpy(FolDrapeImageSig, NewName);};
		void SetLogoImageName(const char * NewName) {strcpy(LogoImageName, NewName);};
		void SetLogoImageSig(const char * NewName) {strcpy(LogoImageSig, NewName);};
		void SetLogoAlign(const HorizontalAlignment NewAlign) {LogoAlign = NewAlign;};
		void SetFolFileName(const char * NewName) {strcpy(FolFileName, NewName);};
		void SetFolFileSig(const char * NewName) {strcpy(FolFileSig, NewName);};
		void SetObjFileName(const char * NewName) {strcpy(ObjFileName, NewName);};
		void SetSpeed(float NewValue) {Speed = NewValue; if(MaxSpeed == -FLT_MAX) MaxSpeed = Speed;};
		void SetHotNavFwdBackAmount(float NewValue) {HotNavFwdBackAmount = NewValue;};
		void SetHotNavSideAmount(float NewValue) {HotNavSideAmount = NewValue;};
		void SetHotTurnHAmount(float NewValue) {HotTurnHAmount = NewValue;};
		void SetHotTurnVAmount(float NewValue) {HotTurnVAmount = NewValue;};
		void SetThrottleSpeed(float NewValue) {ThrottleSpeed = NewValue;};
		void IncreaseThrottleSpeed(float Amount = 1.10f) {ThrottleSpeed = max(min(MaxSpeed, ThrottleSpeed * Amount), MaxSpeed * 0.1f);};
		void DecreaseThrottleSpeed(float Amount = .90f) {ThrottleSpeed = max(0.0, ThrottleSpeed * Amount);};
		void SetMaxSpeed(float NewValue) {MaxSpeed = NewValue;};
		void SetFriction(float NewValue) {Friction = NewValue;};
		void SetInertia(float NewValue) {Inertia = NewValue;};
		void SetAcceleration(float NewValue) {Acceleration = NewValue;};
		void SetConstrain(bool NewValue) {Constrain = NewValue;};
		void SetSplashTime(float NewValue) {SplashTime = NewValue;};
		void SetNoticeTime(float NewValue) {NoticeTime = NewValue;};
		void SetFollowTerrainEnabled(bool NewState) {FollowTerrainEnabled = NewState;};
		void ToggleFollowTerrainEnabled(void) {FollowTerrainEnabled = !FollowTerrainEnabled;};
		void SetFollowTerrainHeight(float NewValue) {FollowTerrainHeight = NewValue;};
		void SetFollowTerrainHeightMax(float NewValue) {FollowTerrainHeightMax = NewValue;};
		void SetFolType(unsigned char NewFolType) {FolType = NewFolType;}

		void SetMetaName(const char * NewName) {strcpy(MetaName, NewName);};
		void SetMetaCopyright(const char * NewName) {strcpy(MetaCopyright, NewName);};
		void SetMetaAuthor(const char * NewName) {strcpy(MetaAuthor, NewName);};
		void SetMetaEMail(const char * NewName) {strcpy(MetaEMail, NewName);};
		void SetMetaUser1(const char * NewName) {strcpy(MetaUser1, NewName);};
		void SetMetaUser2(const char * NewName) {strcpy(MetaUser2, NewName);};
		void SetMetaUser3(const char * NewName) {strcpy(MetaUser3, NewName);};
		void SetMetaUser4(const char * NewName) {strcpy(MetaUser4, NewName);};
		void SetMetaUser5(const char * NewName) {strcpy(MetaUser5, NewName);};

		void AddFolSpecies(NVFolSpecies *NewSpecies);
		NVFolSpecies *GetFolSpecies(int SpeciesIndex) {return(FolSpecies[SpeciesIndex]);}

		void SetSig(const char *SigText) {SceneFileSig = SigText;};

		void ExpandXY(double CurrentX, double CurrentY) {
						if(CurrentX > XMax) XMax = CurrentX;
						if(CurrentX < XMin) XMin = CurrentX;
						if(CurrentY > YMax) YMax = CurrentY;
						if(CurrentY < YMin) YMin = CurrentY;
						}

		void ExpandTerrainXY(double CurrentX, double CurrentY) {
						if(CurrentX > TerXMax) TerXMax = CurrentX;
						if(CurrentX < TerXMin) TerXMin = CurrentX;
						if(CurrentY > TerYMax) TerYMax = CurrentY;
						if(CurrentY < TerYMin) TerYMin = CurrentY;
						}


		void ExpandCoords(double CurrentX, double CurrentY) {
						if(CurrentX > CoordXMax) CoordXMax = CurrentX;
						if(CurrentX < CoordXMin) CoordXMin = CurrentX;
						if(CurrentY > CoordYMax) CoordYMax = CurrentY;
						if(CurrentY < CoordYMin) CoordYMin = CurrentY;
						SetupForProjection();
						}

		void SetupForProjection(void) {
			LatScaleFac = EARTHLATSCALE_METERS;
			LonScaleFac = LonScale(EARTHRAD * 1000, (GetCoordYMin() + GetCoordYMax()) * .5);

			LocalOriginLat = ((GetCoordYMax() + GetCoordYMin()) / 2);
			LocalOriginLon = ((GetCoordXMax() + GetCoordXMin()) / 2);
			}; // SetupForProjection


		void DegToCart(PartialVertexDEM &Subject)
			{
			Subject.XYZ[0] = (LonScaleFac * ((Subject.Lon) - LocalOriginLon));
			Subject.XYZ[1] = -(LatScaleFac * ((Subject.Lat) - LocalOriginLat));
			// copy elevation
			Subject.XYZ[2] = (float)Subject.Elev; // Z=Elev=+Up in OSG
			};

		void CartToDeg(PartialVertexDEM &Subject)
			{
			Subject.Lon = ((Subject.XYZ[0] / LonScaleFac) + LocalOriginLon);
			Subject.Lat = ((-Subject.XYZ[1] / LatScaleFac) + LocalOriginLat);
			Subject.Elev = Subject.XYZ[2]; // Z=Elev=+Up in OSG
			};

		double GetLonScaleFac(void) {return(LonScaleFac);};
		double GetLatScaleFac(void) {return(LatScaleFac);};
		double GetLocalOriginLat(void) {return(LocalOriginLat);};
		double GetLocalOriginLon(void) {return(LocalOriginLon);};

		void UpdateHUD();

		DEM *GetTerrain(void) {return(Terrain);};
		void SetTerrain(DEM *NewTerrain) {Terrain = NewTerrain;};

	};

#endif // !NVW_NVSCENE_H
