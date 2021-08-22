
#ifndef _OSGEARTHVIEW_H_
#define _OSGEARTHVIEW_H_
#include <windows.h>
#include "OSGViewBase.h"

using namespace osgEarth::Util;

class OSGEarthView:public OSGViewBase
{
public:
	OSGEarthView(HWND hWnd);
	~OSGEarthView();

	void InitOSG(std::string filename);
	void InitManipulators(void);
	void InitSceneGraph(void);
	void InitCameraConfig(void);
	void PreFrameUpdate(void);
	void PostFrameUpdate(void);


	osg::ref_ptr<EarthManipulator> getEm() { return mpManip; }
	osg::ref_ptr<osg::Group> getRoot() { return mRoot; }
	osg::ref_ptr<osgEarth::Map> getMap() { return mMap; }
	osg::ref_ptr<osgEarth::MapNode> getMapNode() { return mMapNode; }
	
	double	metersToLongitudinalDegrees(double value, double lat_deg);

	int		DrawPlane(CDOT& centerPos, std::vector< std::vector<CDOT> >& centerDots, std::vector <vector< CDOT >>& colors, double	  xWidth, double yWidth, double zWidth);

	int		DrawSurface(CDOT& centerPos, std::vector< std::vector<CDOT> >& centerDots, std::vector <vector< double >>& heights, double	  xWidth, double yWidth, double zWidth);

	bool		RemovePlane(int id);


	void		Jump(double lon, double lat, double z, double heading, double pitch, double range);


	bool	GetAtt(double& att);

private:
	std::string m_ModelName;
	osg::ref_ptr<osg::Group>				mRoot;
	osg::ref_ptr<osg::Node>				mModel;
	osg::ref_ptr<osgEarth::Map> mMap;
	osg::ref_ptr<osgEarth::MapNode> mMapNode;
	osg::ref_ptr < EarthManipulator> mpManip;
	MapNode* mpMapNode;
	osg::Group* mpModelGroupNode;// = new osg::Group();
	osg::Group* mpTextGroupNode;// = new osg::Group();

	MapIDNode									mDrawNodes;
};

#endif