


#ifndef _OSGVIEWBASE_H_
#define _OSGVIEWBASE_H_
#include <windows.h>

#include "OSGEarth3DView.h"
#include <osg/Config>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/api/win32/GraphicsWindowWin32>
#include <osgGA/TrackballManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StandardManipulator>
#include <osgGA/StateSetManipulator>
#include <osgDB/DatabasePager>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <string>
#include <osg/PositionAttitudeTransform>
#include <osg/Group>
#include <osg/Node>
#include <osgDB/ReadFile>

#include <osgEarthDrivers/cache_filesystem/FileSystemCache>
#include <osgEarth/EarthManipulator>
#include <osgEarth/GeodeticGraticule>
#include <osgEarth/LatLongFormatter>
#include <osgEarth/Controls>
#include <osgEarth/MouseCoordsTool>
#include <osgEarth/AutoClipPlaneHandler>
#include <osgEarth/ImageLayer>
#include <osgEarth/Notify>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ExampleResources>
#include <osgEarth/MapNode>
//#include <osgEarth/ThreadingUtils>
#include <osgEarth/Metrics>
#include <osgEarth/SpatialReference>


using namespace osg;
using namespace osgDB;
using namespace osgGA;
using namespace osgViewer;
using namespace osgUtil;
using namespace osgEarth;



static	int										guid = 0;
typedef std::map<int, Node*>			MapIDNode;

class OSGViewBase
{
public:
	OSGViewBase(HWND hwnd) 
	{
		m_hWnd = hwnd;
		mpGeoSRS = NULL;
	}
	virtual ~OSGViewBase()
	{
		if (mViewer)
		{
			mViewer->setDone(true);
			Sleep(1000);
			mViewer->stopThreading();
			delete mViewer;
		}
	}
	virtual	void InitOSG(std::string filename) = 0;
	osgViewer::Viewer* getViewer() { return mViewer; }
	virtual	void PreFrameUpdate(void) {}
	virtual	void PostFrameUpdate(void) {}
	virtual	void Done(bool value) { mDone = value; }
	virtual	bool Done(void) { return mDone; }
	
	virtual	double	metersToLongitudinalDegrees(double value, double lat_deg) { return 0; }
	virtual	double longitudinalDegreesToMeters(double value, double lat_deg) { return 0; }
	virtual	int		DrawPlane(CDOT& centerPos, std::vector< std::vector<CDOT> >& centerDots, std::vector <vector< CDOT >>& colors, double	  xWidth, double yWidth, double zWidth) { return -1; };
	virtual	int		DrawSurface(CDOT& centerPos, std::vector< std::vector<CDOT> >& centerDots, std::vector <vector< double >>& heights, double	  xWidth, double yWidth, double zWidth) { return -1; }
	virtual	bool		RemovePlane(int id) { return false; };

	virtual	void		Jump(double lon, double lat, double z, double heading, double pitch, double range) {};

	virtual	bool	GetAtt(double& att) { att = 0; return false; }

protected:
	bool							mDone;
	HWND						m_hWnd;
	osgViewer::Viewer*   mViewer;
	const SpatialReference*		mpGeoSRS;
	const Ellipsoid						mEllipsoid;
};

//double OSGViewBase::metersToLongitudinalDegrees(double value, double lat_deg)
//{
//	if ( mpGeoSRS )
//	{
//		const Ellipsoid	ellipsoid = mpGeoSRS->getEllipsoid();
//		return ellipsoid.metersToLongitudinalDegrees(value, lat_deg);
//	}
//}
#include <mutex>
class CRenderingThread : public OpenThreads::Thread
{
public:
	CRenderingThread(OSGViewBase* ptr);
	CRenderingThread(OSGViewBase* ptr, bool is3dThread);
	virtual ~CRenderingThread();
	void Done(bool);
	bool isEnd();
	void setOsgRuningFlag(bool);
	bool getOsgRuningState();

	virtual void run();
	void	RemoveID(int* id , int num);
protected:
	OSGViewBase* _ptr;
	bool _done;
	bool _is3Dthread;
	bool _osgRuningFlag;
	bool _osgRuningState;
	int	*_RemoveID;
	int		removeIDNum;
	std::mutex			removeMutex;
};




#endif
