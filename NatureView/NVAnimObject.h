// NVAnimObject.h

#ifndef NVW_NVANIMOBJECT_H
#define NVW_NVANIMOBJECT_H

#include <string>
#include <vector>

// overkill, but no need to skimp
#define NV_KEYFRAME_MAX_CHANNELS	32
#define NV_KEYFRAME_MAX_STRINGS		6
class MiniKeyFrame
	{
	public:
		float TimeValue;
		double ChannelValues[NV_KEYFRAME_MAX_CHANNELS];
		bool ChannelKeyPresent[NV_KEYFRAME_MAX_CHANNELS];

		std::string Strings[NV_KEYFRAME_MAX_STRINGS];

		MiniKeyFrame() {TimeValue = 0.0f; for(int i=0; i < NV_KEYFRAME_MAX_CHANNELS; i++) ChannelKeyPresent[i] = false;};
		void SetTimeValue(float NewValue) {TimeValue = NewValue;};
		float GetTimeValue(void) {return(TimeValue);}

		bool SetChannelValue(int Channel, double NewValue);
		bool GetChannelKeyValid(int Channel) {if(Channel < NV_KEYFRAME_MAX_CHANNELS) return(ChannelKeyPresent[Channel]); else return(false);};
		double GetChannelValue(int Channel) {if(GetChannelKeyValid(Channel)) return(ChannelValues[Channel]); else return(0.0);};

		bool GetStringValid(int StringID) {if(StringID < NV_KEYFRAME_MAX_STRINGS) return(!Strings[StringID].empty()); else return(false);}; 
		void SetString(int StringID, char *StringData) {if(StringID < NV_KEYFRAME_MAX_STRINGS) Strings[StringID] = StringData;};
		const char *GetString(int StringID) {if((StringID < NV_KEYFRAME_MAX_STRINGS) && GetStringValid(StringID)) return(Strings[StringID].c_str()); else return(NULL);}; 
	}; // MiniKeyFrame


class NVAnimObject
	{
	public:
		std::string Name, OverallSig;
		std::vector <MiniKeyFrame *> KeyFrames;
		std::vector <int> GenericIntValues;
		bool Enabled;

		NVAnimObject() {Enabled = 0;};

		void AddKeyFrame(class MiniKeyFrame *NewKey);
		void SetName(char *NewName) {Name = NewName;};
		void SetOverallSig(char *NewName) {OverallSig = NewName;};
		const char *GetName(void) {return(Name.c_str());};
		bool CheckName(void) {return(!Name.empty());};

		double GetTimeOfEndKey(void) {if(KeyFrames.empty()) return(0.0); else return(KeyFrames.back()->GetTimeValue());};
		bool GetIsAnimated(void) {return(KeyFrames.size() > 1);};

		const char *GetOverallSig(void) {return(OverallSig.c_str());};
		bool CheckOverallSig(void) {return(!OverallSig.empty());};

		MiniKeyFrame *GetKeyFrame(float Time) {return(KeyFrames[0]);};  // <<<>>> Ignores Time since we don't have anim support yet
		bool CheckKeyFrame(float Time) {return(!KeyFrames.empty());};  // <<<>>> Ignores Time since we don't have anim support yet
		double GetChannelValueTime(int Channel, float Time) {return(KeyFrames[0]->GetChannelValue(Channel));}; // <<<>>> Ignores Time since we don't have anim support yet
		//MiniKeyFrame *GetPriorKeyFrame(float Time);
		//MiniKeyFrame *GetFollowingKeyFrame(float Time);
		int GetBracketingKeyFrames(float Time, MiniKeyFrame *&PriorKey, MiniKeyFrame *&FollowingKey, bool UseCached);

		const char *GetStringTime(int StringID, float Time) {return(KeyFrames[0]->GetString(StringID));}; // <<<>>> Ignores Time since we don't have anim support yet

		bool GetEnabled(void) {return(Enabled);};
		void SetEnabled(bool NewState) {Enabled = NewState;};

		enum HazeTypeGenericIntDefs
			{
			HAZETYPE = 0, // add others later
			HAZEDEFSMAXIMUM // avoid comma propogation
			}; // Haze anim Key subtags

		enum HazeTypeGenericInt
			{
			HAZELINEAR = 0, // add others later
			HAZEMAXIMUM // avoid comma propogation
			}; // Haze anim Key subtags

		enum SkyTypeGenericIntDefs
			{
			SKYTYPE = 0, // add others later
			SKYDEFSMAXIMUM // avoid comma propogation
			}; // Sky anim Key subtags

		enum SkyTypeGenericInt
			{
			SKYDOME = 0, // add others later
			SKYMAXIMUM // avoid comma propogation
			}; // Sky anim Key subtags

		enum LightTypeGenericIntDefs
			{
			LIGHTTYPE = 0, // add others later
			LIGHTDEFSMAXIMUM // avoid comma propogation
			}; // Light anim Key subtags

		enum LightTypeGenericInt
			{ // PARALLEL, SPOT, OMNI, AMBIENT SKY, AMBIENT GROUND
			LIGHTPARALLEL = 0, // add others later
			LIGHTSPOT,
			LIGHTOMNI,
			LIGHTAMBIENTSKY,
			LIGHTAMBIENTGROUND,
			LIGHTMAXIMUM // avoid comma propogation
			}; // Light anim Key subtags

		// These could conflict with other types (above)
		enum ActionIDTypeGenericIntDefs
			{
			ACTIONID = 0, // we won't have any others, AFAIK
			ACTIONIDDEFSMAXIMUM // avoid comma propogation
			}; // ActionID subtags


		void SetGenericIntValue(unsigned long int ValueSubscript, int NewValue)
			{
			GenericIntValues.resize(ValueSubscript + 1);
			GenericIntValues[ValueSubscript] = NewValue;
			};

		int GetGenericIntValue(unsigned long int ValueSubscript) {return(GenericIntValues[ValueSubscript]);};

		int CheckGenericIntValuePresent(unsigned long int ValueSubscript) {return(GenericIntValues.size() > ValueSubscript);};

		enum AnimObjectType
			{
			CAMERA = 0,
			OBJINSTANCE,
			OCEAN,
			HAZE,
			SKY,
			LIGHT,
			TYPEMAXIMUM // avoid comma propogation
			}; // Object Type tags

		AnimObjectType Type;

		// used as subscripts into the MiniKeyFrame arrays
		enum // when used, these need a NVAnimObject:: prefix
			{
			CANON_NONE = -1,
			CANONX = 0,
			CANONY,
			CANONZ,
			CANONAUXX, // often scale
			CANONAUXY, // often scale
			CANONAUXZ, // often scale
			CANONH,
			CANONP,
			CANONB, // also used as angle in axis-angle notation
			CANONQI,
			CANONQJ,
			CANONQK,
			CANONQW,
			CANONAXISX,
			CANONAXISY,
			CANONAXISZ,
			CANONHANGLE,
			CANONINTENSITYA,
			CANONINTENSITYB,
			CANONDISTANCEA,
			CANONDISTANCEB,
			CANONCOLORR,
			CANONCOLORG,
			CANONCOLORB,
			CANONMAXIMUM // avoid comma propogation
			}; // Camera anim Key subtags

		// used as subscripts into the MiniKeyFrame strings
		enum // when used, these need a NVAnimObject:: prefix
			{
			STRING_NONE = -1,
			STRING_FILEA = 0,
			STRING_FILEB,
			STRING_FILEC,
			STRING_SIGA,
			STRING_SIGB,
			STRING_SIGC,
			STRING_MAXIMUM // avoid comma propogation
			}; // Camera anim Key subtags


	}; // NVAnimObj

#endif // NVW_NVANIMOBJECT_H
