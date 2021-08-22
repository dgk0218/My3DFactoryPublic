pragma once
#include <windows.h>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/api/win32/GraphicsWindowWin32>
#include <osgGA/TrackballManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgDB/DatabasePager>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <string>
#include <osgGA/StateSetManipulator>
#include <osgEarth/EarthManipulator>
#include <osgEarth/Map>
#include <osgEarth/MapNode>
//#include <osgEarthDrivers/tms/TMSOptions>

//#include <osgEarthDrivers/model_feature_geom/FeatureGeomModelOptions>
//#include <osgEarthDrivers/feature_ogr/OGRFeatureOptions>
#include <osgEarthDrivers/cache_filesystem/FileSystemCache>
#include <osgEarth/EarthManipulator>
#include <osgEarth/GeodeticGraticule>
#include <osgEarth/LatLongFormatter>
#include <osgEarth/Controls>
#include <osgEarth/MouseCoordsTool>
#include <osgEarth/AutoClipPlaneHandler>
#include <osg/PositionAttitudeTransform>
#include <osg/Group>
#include <osg/Node>
#include <osgDB/ReadFile>

#include <osgEarth/ImageLayer>
#include <osgEarth/Notify>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ExampleResources>
#include <osgEarth/MapNode>
//#include <osgEarth/ThreadingUtils>
#include <osgEarth/Metrics>



using namespace osgEarth::Util;
class OSGWrapper
{
public:
	OSGWrapper(HWND hWnd);
	~OSGWrapper();

	void InitOSG(std::string filename);
	void InitManipulators(void);
	void InitSceneGraph(void);
	void InitCameraConfig(void);
	void PreFrameUpdate(void);
	void PostFrameUpdate(void);
	void Done(bool value) { mDone = value; }
	bool Done(void) { return mDone; }

	osg::ref_ptr<osgViewer::Viewer> getViewer() { return viewer; }
	osg::ref_ptr<EarthManipulator> getEm() { return manip; }
	osg::ref_ptr<osg::Group> getRoot() { return mRoot; }
	osg::ref_ptr<osgEarth::Map> getMap() { return mMap; }
	osg::ref_ptr<osgEarth::MapNode> getMapNode() { return mMapNode; }

private:
	bool mDone;
	std::string m_ModelName;
	HWND m_hWnd;
	osg::ref_ptr<osgViewer::Viewer> viewer;
	osg::ref_ptr<osg::Group> mRoot;
	osg::ref_ptr<osg::Node> mModel;
	osg::ref_ptr<osgEarth::Map> mMap;
	osg::ref_ptr<osgEarth::MapNode> mMapNode;

	osg::ref_ptr < EarthManipulator> manip;
};

class CRenderingThread : public OpenThreads::Thread
{
public:
	CRenderingThread(OSGWrapper* ptr);
	CRenderingThread(OSGWrapper* ptr, bool is3dThread);
	virtual ~CRenderingThread();
	void Done(bool);
	bool isEnd();
	void setOsgRuningFlag(bool);
	bool getOsgRuningState();

	virtual void run();

protected:
	OSGWrapper* _ptr;
	bool _done;
	bool _is3Dthread;
	bool _osgRuningFlag;
	bool _osgRuningState;
};