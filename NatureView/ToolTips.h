// ToolTips.h
// Support code to enable and handle ToolTips

#ifndef NVW_TOOLTIPS_H
#define NVW_TOOLTIPS_H

#include <string>
#include "Types.h"
#include <osg/Timer>


class ToolTipSupport
	{
	private:
		NativeAnyWin TTWidget;
		NativeAnyWin BalloonWidget;

		std::string BalloonText;
		osg::Timer BalloonTimer;
		osg::Timer_t StartMoment;
		double DisplayLength;

	public:
		ToolTipSupport();
		~ToolTipSupport();

		int AddTool(NativeAnyWin Control, unsigned long int StringID);
		int AddCallbackWin(NativeAnyWin Control);
		int RemoveTool(NativeAnyWin Control);

		void SetEnabled(bool NewEnabled); // no get functionality provided
		
		int ShowBalloonTip(unsigned long int X, unsigned long int Y, const char *TipText, double DisplayTime = 10.0);
		int HideBalloonTip(void);
		int HandleBalloonTipTimeout(void);
	}; // ToolTipSupport


#endif // NVW_TOOLTIPS_H
