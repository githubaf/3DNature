/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#ifndef OSGGA_PanoMANIPULATOR
#define OSGGA_PanoMANIPULATOR 1

#include <osgGA/MatrixManipulator>
#include <osg/Quat>
#include <osg/Timer>

/**
PanoManipulator is a MatrixManipulator that provides a basic and semi-intuitive navigation experience
*/

#define NVW_PANOMANIPULATOR_TOUR_DELAY_TIME_SECONDS		3
#define NVW_PANOMANIPULATOR_SLIDE_GIMBAL_EPSILON		.001
#define NVW_PANOMANIPULATOR_MOUSE_INACTIVE_TIME_SECONDS		1.5

class PanoManipulator : public osgGA::MatrixManipulator
{
    public:

		enum PM_ActionClass
			{
			PM_AC_HOME = 0, //
			PM_AC_HALT,
			PM_AC_MOVEFORWARD,
			PM_AC_MOVEBACKWARD,
			PM_AC_MOVELEFT,
			PM_AC_MOVERIGHT,
			PM_AC_MOVEUP,
			PM_AC_MOVEDOWN,
			PM_AC_TURNLEFT,
			PM_AC_TURNRIGHT,
			PM_AC_TILTUP,
			PM_AC_TILTDOWN,
			PM_AC_BANKCLOCKWISE,
			PM_AC_BANKCOUNTERCLOCKWISE,
			PM_AC_UNDO,
			PM_AC_STARTTOUR,
			PM_AC_ENDTOUR,
			PM_AC_MAXIMUM // to prevent comma propogation
			}; // PM_ActionClass


        PanoManipulator();

        virtual const char* className() const { return "Pano"; }

        /** set the position of the matrix manipulator using a 4x4 Matrix.*/
        virtual void setByMatrix(const osg::Matrix& matrix);

        /** set the position of the matrix manipulator using a 4x4 Matrix.*/
        virtual void setByInverseMatrix(const osg::Matrix& matrix) { setByMatrix(osg::Matrix::inverse(matrix)); }

        /** get the position of the manipulator as 4x4 Matrix.*/
        virtual osg::Matrix getMatrix() const;

        /** get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
        virtual osg::Matrix getInverseMatrix() const;


        virtual void setNode(osg::Node*);

        virtual const osg::Node* getNode() const;

        virtual osg::Node* getNode();

        virtual void home(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

        virtual void halt(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

        virtual void init(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

        virtual bool DoAction(PM_ActionClass ActionType, float Magnitude = 1.0f);
		virtual void Goto(float X, float Y);


        /** Get the keyboard and mouse usage of this manipulator.*/
        virtual void getUsage(osg::ApplicationUsage& usage) const;

        void setModelScale(float NewValue) {_modelScale = NewValue;};

		void MarkTransStartMoment(void);
		void SetTransStartFromCurrent(void);
		void SetTransEndFromCurrent(void);
		void SetTransEnd(double CX, double CY, double CZ, double TX, double TY, double TZ, double HFOV, double Bank);
		double GetCurrentMoment(void) {return(MomentTimer.delta_s(StartMoment, MomentTimer.tick()));};
		void ClearStartMoment(void) {StartMoment = MomentTimer.tick();};
		void MarkLastMouseMoveMoment(void) {LastMouseMoveMoment = MomentTimer.tick(); SetMouseInactiveOneShot(0);};
		double GetMouseInactiveTime(void) {return(MomentTimer.delta_s(LastMouseMoveMoment, MomentTimer.tick()));};
		void SetMouseInactiveOneShot(bool NewState) {MouseInactiveOneShot = NewState;};
		bool GetMouseInactiveOneShot(void) const {return(MouseInactiveOneShot);};

		void JumpTo(osg::Vec3 NewSpatialPos); // NewSpatialPos is lat/lon/elev or the like
		void JumpTo(double SpatialX, double SpatialY);  // SpatialPos is lat/lon, keeps current elev


    protected:

        virtual ~PanoManipulator();

        /** Reset the internal GUIEvent stack.*/
        void flushMouseEventStack();
        /** Add the current mouse GUIEvent to internal stack.*/
        void addMouseEvent(const osgGA::GUIEventAdapter& ea);

        void computePosition(const osg::Vec3& eye,const osg::Vec3& lv,const osg::Vec3& up);

        /** For the given mouse movement calculate the movement of the camera.
            Return true if camera has moved and a redraw is required.*/
        bool calcMovement(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

		// move by MX/MY/MZ 'steps' in a given direction. A step is not a world unit.
		bool PanoManipulator::ExecuteMovement(double MX, double MY, double MZ);

		// turn by RX/RY/RZ percent in a given direction. 100% is 360 degrees
		bool PanoManipulator::ExecuteRotate(double RX, double RY, double RZ);

		void InternalHome(void);
		void EnforceConform(void);
		void UpdateUndo();
		void RestoreFromUndo();

		// Orbit mode support
		osg::Vec3 _OrbitCenter;
		bool _OrbitCenterValid;
		
		// Goto and Orbit intersection support
		bool GetFirstTangibleHitLocation(float X, float Y, osg::Vec3 &IntersectPoint); // returns true if recorded a hit
		
		void ClearOrbitCenter(void) {_OrbitCenterValid = false;};
		void SetOrbitCenter(const osg::Vec3 NewOrbitCenter) {_OrbitCenter = NewOrbitCenter; _OrbitCenterValid = true;};
		bool GetOrbitCenterValid(void) const {return(_OrbitCenterValid);};
		bool GetOrbitCenter(osg::Vec3 &Destination) const {if(GetOrbitCenterValid()) Destination = _OrbitCenter; return(GetOrbitCenterValid());};

		// Viewpoint transition support
		double TransitionStartTime, TransitionDesiredLengthTime;
		double TransHFOV_S;
		osg::Vec3 TransCamPos_S;
		osg::Quat TransCamOrient_S;
		double TransHFOV_E, TransBank_E;
		double TransCamPos_E[3], TransTargPos_E[3];
		void InternalTrans(double Moment);
		void InternalTrans(void);

		void InternalAnim(void);
		void InternalAnim(double Moment);

        float TourPauseTime, TourPauseMoment;
		void StartTour(double NewTourPauseTime);
		void TourPauseHere(void);
		void InternalTour(void);
		void EndTour(void);
		

		// timing support
        osg::Timer      MomentTimer;
        osg::Timer_t    StartMoment;
        osg::Timer_t    LastMouseMoveMoment;
        bool MouseInactiveOneShot; // false if we have yet to fire a mouse-inactive event, true once it has been fired during this inactivity period


        // Internal event stack comprising last three mouse events.
        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t1;
        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t0;

        osg::ref_ptr<osg::Node>       _node;

        float _modelScale;
        float _velocity;

		float LastClickOriginX, LastClickOriginY;

       
        osg::Vec3   _eye, _eyeUndo;
        osg::Quat   _rotation, _rotationUndo;

};

#endif

