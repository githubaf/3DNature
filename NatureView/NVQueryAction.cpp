// NVQueryAction.cpp
// Code to handle actions and queries
// Created from scratch on 01/21/05 (Rhys' one-month-and-one-day birthday!) by CXH
// Copyright 2005

#include <windows.h>
#include <shellapi.h>

#include <sstream>
#include <osgUtil/IntersectVisitor>
#include <osgProducer/Viewer>
#include <osg/Fog> // for highlights
#include <osg/NodeVisitor> // for clearing highlights

#include "NVQueryAction.h"
#include "NVNodeMasks.h"
#include "Viewer.h"
#include "NVScene.h"
#include "NVFoliage.h" // for ActionedGeometry class
#include "ActionDB.h"
#include "Viewer.h" // for GlobalViewer for ShellExecute
#include "MediaSupport.h" // for sound support and such
#include "DataDlg.h"
#include "ToolTips.h"
#include "StringSupport.h" // for string trim
#include "HTMLDlg.h"

extern ToolTipSupport *GlobalTipSupport;

#ifdef NV_BUILD_PLUGINS
#include "PluginTest.h"

extern PluginManager PIM;
#endif // NV_BUILD_PLUGINS


extern NVScene MasterScene;
osg::Group *GetSceneRootNodeGlobal(void);


unsigned long int PerformQueryAction(float X, float Y, bool NonIntrusive)
{
unsigned long int Result = 0;
#ifdef NVW_SUPPORT_QUERYACTION
osg::Geometry *HighlightGeom = NULL;
osg::Node *HighlightNode = NULL;

osgUtil::IntersectVisitor::HitList hlist;
osgProducer::Viewer *_viewer;

//osg::Node::NodeMask traversalMask = ~(NVW_NODEMASK_INTANGIBLE | NVW_NODEMASK_TERRAIN | NVW_NODEMASK_SKY); // ignores terrain and sky
osg::Node::NodeMask traversalMask = ~(NVW_NODEMASK_INTANGIBLE | NVW_NODEMASK_SKY); // ignores sky

_viewer = GetGlobalViewer();

if(!NonIntrusive)
	{ // if they're deliberately trying a query, clear existing results
	ClearAllHighlights();
	GlobalTipSupport->HideBalloonTip();
	} // if

if (_viewer->computeIntersections(X, Y, hlist, traversalMask))
	{
    for(osgUtil::IntersectVisitor::HitList::iterator hitr=hlist.begin(); hitr!=hlist.end(); ++hitr)
	    {
	    signed long int ActionID = -1;
	    //std::string HitText, Desc0;
        // did we hit a non-Node entity?
        if (hitr->_drawable.valid())
			{
			// hunt for our ActionID on the geometry-derived class, if that's what it turns out to be
			ActionedGeometry *AG;
			osg::Geometry *AsG;
            //os<<"Object \""<<hitr->_drawable->className()<<"\""<<std::endl;
            //HitText = "Drawable"; // <<<>>> debug
            if(AsG = (hitr->_drawable->asGeometry()))
				{
				AG = dynamic_cast<ActionedGeometry *>(AsG);
				if(AG)
					{
					//std::stringstream NumConvert;
					ActionID = AG->GetActionID();
					if(ActionID != -1)
						{
						HighlightGeom = AsG;
						} // if
					//NumConvert << ActionID;
					//Desc0 = NumConvert.str();
					//HitText = "ActionID = ";
					//HitText += Desc0;
					} // if
				} // if
	        } // if
	    if(ActionID == -1) // still looking?
			{
			// need to walk up the scene graph any number of steps before we find our
			// ActionID
			std::istringstream ConvertBackToNum;
			for(osg::NodePath::reverse_iterator WalkUp = hitr->_nodePath.rbegin(); WalkUp != hitr->_nodePath.rend(); WalkUp++)
				{
				/*
				char TempDebug[1000];
				sprintf(TempDebug, "%s::%s\n", (*WalkUp)->libraryName(), (*WalkUp)->className());
				OutputDebugString(TempDebug);
				*/
				if((*WalkUp)->getNumDescriptions() > 0)
					{
					std::string Desc0;
					Desc0 = (*WalkUp)->getDescription(0);
					//HitText = "ActionID = ";
					//HitText += Desc0;
					ConvertBackToNum.str(Desc0);
					ConvertBackToNum >> ActionID;
					if(ActionID != -1)
						{
						HighlightNode = (*WalkUp);
						} // if
					break;
					} // if
				} // for
			//OutputDebugString("\n");
			} // else
        if(ActionID != -1)
			{
			std::vector<NVAction *> HitActions;
			if(FetchActionsFromID(ActionID, HitActions) > 0)
				{
				PerformActionsFromActionRecords(HitActions, X, Y, NonIntrusive, HighlightGeom, HighlightNode);
				} // if
	        } // if
        Result = 1;
		break; // only do first hit item
		} // for
	} // if

#endif // NVW_SUPPORT_QUERYACTION
return(Result);
} // PerformQueryAction

// de-allocates the NVAction records in the passed container
unsigned long int PerformActionsFromActionRecords(std::vector<NVAction *> ActionRecords, float X, float Y, bool NonIntrusive, osg::Geometry *HighlightGeom, osg::Node *HighlightNode)
{
unsigned long int ActionsPerformed = 0;
std::ostringstream FormatText;
POINT CurPos;

GetCursorPos(&CurPos);


for(std::vector<NVAction *>::iterator HAWalk = ActionRecords.begin(); HAWalk != ActionRecords.end(); HAWalk++)
	{
	std::string LocalActionStr, TipString;
	LocalActionStr = (*HAWalk)->GetActionString();
	signed char Type = (*HAWalk)->GetActionType();
		
	switch(Type)
		{
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYLABEL:
			{
			//GlobalTipSupport->ShowBalloonTip(CurPos.x, CurPos.y, LocalActionStr.c_str());
			TipString += LocalActionStr;
			// accumulate and show when all other actions are done
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYDATATABLE:
			{
			// this will automatically show the window
			if(!NonIntrusive)
				{
				DataDlg::SetDataText(LocalActionStr.c_str());
				} // if
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_VIEWTEXTFILE:
			{
			// this will automatically show the window
			if(!NonIntrusive)
				{
				DataDlg::SetDataTextFromFile(LocalActionStr.c_str());
				} // if
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGE:
			{
			if(!NonIntrusive)
				{
				// there probably ought to be some more safety-checking done here...
				TrimRight(LocalActionStr); // trim rightmost whitespace
				std::string SceneWorkPath = MasterScene.GetSceneFileWorkingPath();
				if(strlen(LocalActionStr.c_str()) > 4000)
					{
					LocalActionStr.resize(4000); // prevent buffer-overflow exploit known to be in older Win2k
					} // if
				if(TestFileKnownImage(LocalActionStr.c_str()) && !TestFileExecutable(LocalActionStr.c_str()))
					{
					if((int)ShellExecute(GetGlobalViewerHWND(), "open", LocalActionStr.c_str(), NULL, SceneWorkPath.c_str(), SW_SHOWNORMAL) > 32)
						{
						} // if
					} // if
				} // if
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGEINTERNAL:
			{
			if(!NonIntrusive)
				{
				TrimRight(LocalActionStr); // trim rightmost whitespace
				if(TestFileKnownImage(LocalActionStr.c_str()) && !TestFileExecutable(LocalActionStr.c_str()))
					{
					// Use internal web browser? Use OSG? Use media player?
					// <<<>>> Use internal web browser for stop-gap
					OpenImageInternally(LocalActionStr.c_str());
					} // if
				} // if
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGE:
			{
			if(!NonIntrusive)
				{
				// there probably ought to be some more safety-checking done here...
				TrimRight(LocalActionStr); // trim rightmost whitespace
				// this will make sure it starts with http: or https:
				OpenURLExternally(LocalActionStr.c_str());
				// below is for testing only...
				//OpenURLInternally(LocalActionStr.c_str());
				} // if
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGEINTERNAL:
			{
			if(!NonIntrusive)
				{
				// there probably ought to be some more safety-checking done here...
				TrimRight(LocalActionStr); // trim rightmost whitespace
				// this will make sure it starts with http: or https:
				OpenURLInternally(LocalActionStr.c_str());
				} // if
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILE:
			{
			if(!NonIntrusive)
				{
				// <<<>>> there probably ought to be some MORE safety-checking done here...
				TrimRight(LocalActionStr); // trim rightmost whitespace
				if(strlen(LocalActionStr.c_str()) > 4000)
					{
					LocalActionStr.resize(4000); // prevent buffer-overflow exploit known to be in older Win2k
					} // if
				bool Safe = true;
				// is the file extension unsafe?
				if(LocalActionStr.length() > 4)
					{
					if(!LocalActionStr.compare(strlen(LocalActionStr.c_str()) - 4, 4, ".exe"))
						{
						Safe = false;
						} // if
					if(!LocalActionStr.compare(strlen(LocalActionStr.c_str()) - 4, 4, ".com"))
						{
						Safe = false;
						} // if
					if(!LocalActionStr.compare(strlen(LocalActionStr.c_str()) - 4, 4, ".pif"))
						{
						Safe = false;
						} // if
					if(!LocalActionStr.compare(strlen(LocalActionStr.c_str()) - 4, 4, ".scr"))
						{
						Safe = false;
						} // if
					} // if
				if(Safe && TestFileKnownImage(LocalActionStr.c_str()) && !TestFileExecutable(LocalActionStr.c_str()))
					{
					std::string SceneWorkPath = MasterScene.GetSceneFileWorkingPath();
					HINSTANCE ReturnCode;
					char ResultStr[MAX_PATH];
					ReturnCode = FindExecutable(LocalActionStr.c_str(), SceneWorkPath.c_str(), ResultStr);
					if((int)ReturnCode > 32)
						{
						if((int)ShellExecute(GetGlobalViewerHWND(), "open", LocalActionStr.c_str(), NULL, SceneWorkPath.c_str(), SW_SHOWNORMAL) > 32)
							{
							} // if
						} // if
					} // if
				} // if
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_HIGHLIGHTOBJECTSET:
			{
			HighlightObject(HighlightGeom, HighlightNode);
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_LOADNEWSCENE:
			{
			if(!NonIntrusive)
				{
				TrimRight(LocalActionStr); // trim rightmost whitespace
				// there probably ought to be some MORE safety-checking done here...
				if(strlen(LocalActionStr.c_str()) > 4000)
					{
					LocalActionStr.resize(4000); // prevent buffer-overflow exploit known to be in older Win2k
					} // if
				if(!TestFileExecutable(LocalActionStr.c_str()))
					{
					// is the file extension .nvw?
					if(LocalActionStr.length() > 4)
						{
						if(!LocalActionStr.compare(strlen(LocalActionStr.c_str()) - 4, 4, ".nvw"))
							{
							std::string SceneWorkPath = MasterScene.GetSceneFileWorkingPath();
							if((int)ShellExecute(GetGlobalViewerHWND(), "open", LocalActionStr.c_str(), NULL, SceneWorkPath.c_str(), SW_SHOWNORMAL) > 32)
								{ // new viewer is loading, bail out quick
								SignalApplicationExit();
								} // if
							} // if
						} // if
					} // if
				} // if
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILEINTERNAL:
			{
			if(!NonIntrusive)
				{
				TrimRight(LocalActionStr); // trim rightmost whitespace
				if(TestFileKnownMedia(LocalActionStr.c_str()) && !TestFileExecutable(LocalActionStr.c_str()))
					{
					// Use internal web browser? Use OSG? Use media player?
					// <<<>>> Use internal web browser for stop-gap
					OpenMediaInternally(LocalActionStr.c_str());
					} // if
				} // if
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_PLAYSOUNDINTERNAL:
			{
			if(!NonIntrusive)
				{
				TrimRight(LocalActionStr); // trim rightmost whitespace
				if(TestFileKnownSound(LocalActionStr.c_str()) && !TestFileExecutable(LocalActionStr.c_str()))
					{
					//PlaySound(LocalActionStr.c_str(), NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT | SND_NOWAIT);
					if(PlaySoundAsync(LocalActionStr.c_str()))
						{
						} // if
					} // if
				} // if
			break;
			} // 
		case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_RUNSHELLCOMMAND:
			{
			if(!NonIntrusive)
				{
	#ifdef NV_BUILD_NVX
				TrimRight(LocalActionStr); // trim rightmost whitespace
				if(strlen(LocalActionStr.c_str()) > 4000)
					{
					LocalActionStr.resize(4000); // prevent buffer-overflow exploit known to be in older Win2k
					} // if
				std::string SceneWorkPath = MasterScene.GetSceneFileWorkingPath();
				if((int)ShellExecute(GetGlobalViewerHWND(), "open", LocalActionStr.c_str(), NULL, SceneWorkPath.c_str(), SW_SHOWNORMAL) > 32)
					{
					} // if
	#endif // NV_BUILD_NVX
				break;
				} // 
			case NVAction::WCS_SXQUERYACTION_ACTIONTYPE_LAUNCHPLUGIN:
				{
	#ifdef NV_BUILD_PLUGINS
				if(PIM.QueryPluginsLoaded())
					{
					static char TextBuf[1000];
					PIM.SetTime(0.0);
					PIM.SetHeight(1.0);
					PIM.SetX(2.0);
					PIM.SetY(3.0);
					PIM.SetZ(4.0);
					PIM.QueryText(TextBuf);
					FormatText << TextBuf << "\n";
					} // if
	#endif // NV_BUILD_PLUGINS
				} // if
			break;
			} // 
		default:
			{
			// dunno what to do:
			} // default
		} // switch

	if(TipString.length())
		{
		GlobalTipSupport->ShowBalloonTip(CurPos.x, CurPos.y, TipString.c_str());
		} // if
	// destroy NVAction
	delete (*HAWalk);
	*HAWalk = NULL;
	ActionsPerformed++;
	} // for

return(ActionsPerformed);
} // PerformActionFromActionRecords

static osg::ref_ptr<osg::StateSet> HighlightViaFog;
static osg::ref_ptr<osg::Fog> fog;

void HighlightObject(osg::Geometry *HighlightGeom, osg::Node *HighlightNode)
{
ClearAllHighlights();

if(!HighlightViaFog.valid())
	{
	HighlightViaFog = new osg::StateSet();
	} // if
if(!fog.valid())
	{
	fog = new osg::Fog(); 
	} // if
fog->setMode(osg::Fog::LINEAR);
fog->setColor(osg::Vec4d(1.0,1.0,0.0,0.5)); 
fog->setStart(0.0f);
fog->setEnd(0.01f);
HighlightViaFog->setAttribute(fog.get(), osg::StateAttribute::ON); 
HighlightViaFog->setMode(GL_FOG, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE); 

if(HighlightGeom)
	{
	HighlightGeom->setStateSet(HighlightViaFog.get());
	} // if
if(HighlightNode)
	{
	HighlightNode->setStateSet(HighlightViaFog.get());
	} // if

} // HighlightObject


class ClearHighlightVisitor : public osg::NodeVisitor 
	{
	public :
		ClearHighlightVisitor () : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ) { };
		virtual void apply( osg::Node &node )
			{
			osg::Node *NodeToClear = dynamic_cast<osg::Node *>( &node);
			osg::Geode *GeodeToClear = dynamic_cast<osg::Geode *>( &node);
			if (NodeToClear != NULL)
				{
				if(NodeToClear->getStateSet() == HighlightViaFog.get())
					{
					NodeToClear->setStateSet(NULL);
					} // if
				} // if
			if(GeodeToClear != NULL)
				{ // try drawables that are children of this geode
				for(unsigned long int DrawableNum = 0; DrawableNum < GeodeToClear->getNumDrawables(); DrawableNum++)
					{
					osg::Drawable* Current = GeodeToClear->getDrawable(DrawableNum);
					if(Current)
						{
						osg::Geometry *GeomToClear;
						GeomToClear = dynamic_cast<osg::Geometry *>(Current);
						if(GeomToClear)
							{
							if(GeomToClear->getStateSet() == HighlightViaFog.get())
								{
								GeomToClear->setStateSet(NULL);
								} // if
							} // if
						} // if
					} // for
				} // if
			traverse(node);
			} // func apply

} ; // class  ClearHighlightVisitor


void ClearAllHighlights()
{
if(HighlightViaFog.valid() && GetSceneRootNodeGlobal())
	{
	ClearHighlightVisitor CleanSweep;
	CleanSweep.setTraversalMask(NVW_NODEMASK_FOLIAGE|NVW_NODEMASK_STRUCT|NVW_NODEMASK_LABEL|NVW_NODEMASK_VECTOR); // not terrain, sky, ocean
	GetSceneRootNodeGlobal()->accept(CleanSweep);
	} // if
} // ClearAllHighlights

