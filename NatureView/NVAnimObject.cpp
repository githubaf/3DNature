#include "NVAnimObject.h"

bool MiniKeyFrame::SetChannelValue(int Channel, double NewValue)
{
bool Success = false;

if(Channel < NV_KEYFRAME_MAX_CHANNELS)
	{
	ChannelValues[Channel] = NewValue;
	ChannelKeyPresent[Channel] = true;
	Success = true;
	} // if

return(Success);
} // MiniKeyFrame::SetChannelValue

/*
MiniKeyFrame *NVAnimObject::GetPriorKeyFrame(float Time)
{
return(NULL);
} // NVAnimObject::GetPriorKeyFrame

MiniKeyFrame *NVAnimObject::GetFollowingKeyFrame(float Time);
{
return(NULL);
} // NVAnimObject::GetFollowingKeyFrame
*/

int NVAnimObject::GetBracketingKeyFrames(float Time, MiniKeyFrame *&PriorKey, MiniKeyFrame *&FollowingKey, bool UseCached)
{


if(!KeyFrames.empty())
	{
	/*
	if(UseCached && PriorKey)
		{
		if(PriorKey->GetTimeValue() <= Time && FollowingKey->GetTimeValue() <= Time)
			{
			// we still have a winnner!
			return(2);
			} // if
		} // if
	*/
	PriorKey = NULL; // prep for search

	std::vector <MiniKeyFrame *>::iterator CurKF;
	for(CurKF = KeyFrames.begin(); CurKF != KeyFrames.end(); CurKF++)
		{
		if((*CurKF)->GetTimeValue() == Time)
			{
			FollowingKey = PriorKey = *CurKF;
			return(1); // only one of them is unique, no need to tween
			} // if
		else if((*CurKF)->GetTimeValue() < Time)
			{
			PriorKey = *CurKF;
			if(CurKF + 1 == KeyFrames.end())
				{ // desired TimeValue is past end of all keys, return last key in both tweening pairs
				FollowingKey = PriorKey;
				return(1); // only one of them is unique, no need to tween
				} // if
			else
				{ // examine next key to see what to do
				MiniKeyFrame *NextKey;
				NextKey = *(CurKF + 1);
				if(NextKey->GetTimeValue() > Time)
					{
					FollowingKey = NextKey;
					return(2); // got two bracketing, need to tween
					} // if
				else
					{
					// move on to next key
					} // else
				} // else
			} // else if
		else
			{
			// move on to next key
			} // else
		} // if
	} // if

return(0);
} // NVAnimObject::GetBracketingKeyFrames


void NVAnimObject::AddKeyFrame(class MiniKeyFrame *NewKey)
{
if(NewKey)
	{
	if(KeyFrames.empty())
		{ // don't worry about order
		KeyFrames.push_back(NewKey);
		} // if
	else
		{ // see if it's a simple append
		MiniKeyFrame *LastKF;
		if(LastKF = KeyFrames.back())
			{
			if(LastKF->TimeValue < NewKey->GetTimeValue())
				{
				KeyFrames.push_back(NewKey); // simple append
				return;
				} // if
			else if(LastKF->TimeValue == NewKey->GetTimeValue())
				{
				KeyFrames.pop_back(); // remove it
				// replace existing last keyframe
				KeyFrames.push_back(NewKey); // simple append
				// free LastKF
				delete LastKF;
				LastKF = NULL;
				return;
				} // else if
			else
				{
				// search for where it needs to go...
				std::vector <MiniKeyFrame *>::iterator CurKF;
				for(CurKF = KeyFrames.begin(); CurKF != KeyFrames.end(); CurKF++)
					{
					if((*CurKF)->GetTimeValue() > NewKey->GetTimeValue())
						{
						// insert before this element
						KeyFrames.insert(CurKF, NewKey);
						return;
						} // if
					else if((*CurKF)->GetTimeValue() == NewKey->GetTimeValue())
						{
						MiniKeyFrame *OldKey;
						// replace this element
						OldKey = *CurKF;
						*CurKF = NewKey;
						delete OldKey;
						OldKey = NULL;
						return;
						} // if
					else
						{
						// keep looking...
						} // else
					} // for
				// if we get here, we ran off the end without finding the right place to insert before
				// we should have hit the simple append trivial case!
				return;
				} // else
			} // if
		else
			{
			// uhh...
			return;
			} // else
		} // else
	} // if
} // NVAnimObject::AddKeyFrame

