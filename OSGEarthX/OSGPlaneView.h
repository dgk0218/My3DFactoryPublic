#ifndef _OSGVIEWINNER_H_
#define _OSGVIEWINNER_H_

#include "OSGEarth3DView.h"
#include "OSGViewBase.h"

class  OSGPlaneView	:public OSGViewBase
{
public:
	OSGPlaneView( HWND viewHandle);
	~OSGPlaneView();

	void InitOSG(std::string filename);
	void InitManipulators(void);
	void InitSceneGraph(void);
	void InitCameraConfig(void);

	void PreFrameUpdate(void);
	void PostFrameUpdate(void);

	double	metersToLongitudinalDegrees(double value, double lat_deg);

private:

	std::string m_ModelName;
	osg::ref_ptr<osg::Group>		mRoot;
	osg::ref_ptr<osg::Node>		mModel;
	osg::ref_ptr<osgGA::TrackballManipulator> trackball;
	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator;
};

#endif 