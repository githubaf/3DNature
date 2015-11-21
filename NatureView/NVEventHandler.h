
#ifndef NVW_NVEVENTHANDLER_H
#define NVW_NVEVENTHANDLER_H

#include <osgGA/GUIEventHandler>
#include <osgProducer/Viewer>

class NVEventHandler : public osgGA::GUIEventHandler
{
    public:
    
        NVEventHandler(osgProducer::Viewer* viewer):_viewer(viewer) {}
    
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
		// static so it can be called from event handlers of floating windows
		static bool HandleKey(int Key, bool ControlQual, bool ShiftQual, bool AltQual);
		// A few meys want to fire on keyup, not down
		static bool HandleKeyUp(int Key, bool ControlQual, bool ShiftQual, bool AltQual);

        virtual void accept(osgGA::GUIEventHandlerVisitor& v)
        {
            v.visit(*this);
        }
      
        osg::Group* rootNode() { return dynamic_cast<osg::Group*>(_viewer->getSceneData()); }
        osgProducer::Viewer*                    _viewer;
};

#endif // !NVW_NVEVENTHANDLER_H

