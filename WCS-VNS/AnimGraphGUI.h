// AnimGraphGUI.h
// The one, the only, TimeLine editor!
// Built from bits of V1 TimeLine code on 5/30/96 by CXH

#ifndef WCS_ANIMGRAPHGUI_H
#define WCS_ANIMGRAPHGUI_H

class SetCritter;
class RasterAnimHost;

#include "Application.h"
#include "Fenetre.h"
#include "WCSWidgets.h"
#include "GraphWidget.h"
#include "Notify.h"
#include "GraphData.h"

class AnimGraphGUI : public WCSModule, public GUIFenetre, public GraphDialog
	{
	private:
		#ifdef _WIN32
		HWND TopControls, BotControls, SideControls, GraphWidget, StupidWin;
		int WidX, WidY;
		#endif // _WIN32
		AnimCritter *HostCritter, *RestoreBackup, *UndoBackup;
		NotifyEx *NotifyExHost;
		short IgnoreNotify;
		char MyGraphID;
		double Distance, Value, FrameRate;
		NotifyTag NotifyEvents[5];
		AnimDoubleTime MaxValueADT, MinValueADT, DistanceADT, ValueADT;

		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void HScroll(NativeGUIWin Panel, short CommandCode, short ButtonID, short Notify);
		void VScroll(NativeGUIWin Panel, short CommandCode, short ButtonID, short Notify);
		void Keep(void);
		void Cancel(void);
		int CreateBackups(void);
		void UpdateUndo(void);
		void Undo(void);
		void Restore(void);
		void NextNode(void);
		void PrevNode(void);
		void AddNode(void);
		void RemoveNode(void);
		void SelectAllNodes(void);
		void ClearSelectNodes(void);
		void ToggleSelectNodes(void);
		void NewValue(void);
		void NewDistance(void);
		void NewLinear(void);
		void NewTCB(char Channel);
		void NewPriority(void);
		void NewRoughness(void);
		void NewEcoMixing(void);
		void NewEcosystem(void);
		void RefreshGraph(void);
		void SelectNewEco(void);
		void FreeEcosystem(void);

	public:
		int ConstructError;

		struct GRData WidgetLocal;

		AnimGraphGUI(AnimCritter *AnimSource, unsigned char IDSource, NotifyEx *NotifyExSource);
		~AnimGraphGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Proj);
		
		long HandleEvent(void);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandleReSized(int ReSizeType, long NewWidth, long NewHeight);
		unsigned long QueryHelpTopic(void);
		virtual void NewActiveNode(GraphNode *NewActive);
		unsigned char GetNotifyClass(void);
		unsigned char GetNotifySubclass(void);
		AnimCritter *GetActive(void) {return (HostCritter);};

	}; // class AnimGraphGUI

#endif // WCS_ANIMGRAPHGUI_H
