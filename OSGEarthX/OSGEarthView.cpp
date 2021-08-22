#include "pch.h"
#include "OSGEarthView.h"

#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osg/Texture3D>
#include <osgDB/FileUtils>
#include <osg/Billboard>
#include <osg/TexGenNode>
#include <osg/ClipNode>
#include <osgGA/StateSetManipulator>

#include <osgEarth/MapNode>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ExampleResources>
#include <osgEarth/ImageOverlay>
#include <osgEarth/CircleNode>
#include <osgEarth/RectangleNode>
#include <osgEarth/EllipseNode>
#include <osgEarth/PlaceNode>
#include <osgEarth/LabelNode>
#include <osgEarth/LocalGeometryNode>
#include <osgEarth/FeatureNode>
#include <osgEarth/ModelNode>

#include <osgEarth/ImageOverlayEditor>
#include <osgEarth/GeometryFactory>
#include <osgEarth/Geometry>

#include <sstream>
using namespace std;
const SpatialReference* g_geoSRS = NULL;

class PickHandler : public osgGA::GUIEventHandler {
public:

	PickHandler(osgText::Text* updateText) :
		_updateText(updateText) {}

	~PickHandler() {}

	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	virtual void pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea);

	void setLabel(const std::string& name)
	{
		if (_updateText.get()) _updateText->setText(name);
	}

protected:

	osg::ref_ptr<osgText::Text>  _updateText;
};

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	switch (ea.getEventType())
	{
	case(osgGA::GUIEventAdapter::PUSH):
	{
		osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
		if (view) pick(view, ea);
		return false;
	}
	case(osgGA::GUIEventAdapter::KEYDOWN):
	{
		if (ea.getKey() == 'c')
		{
			osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
			osg::ref_ptr<osgGA::GUIEventAdapter> event = new osgGA::GUIEventAdapter(ea);
			event->setX((ea.getXmin() + ea.getXmax()) * 0.5);
			event->setY((ea.getYmin() + ea.getYmax()) * 0.5);
			if (view) pick(view, *event);
		}
		return false;
	}
	default:
		return false;
	}
}
//继承自ArrayVisitor的访问器
class PrintArrayVisitor : public osg::ArrayVisitor
{
public:
	PrintArrayVisitor() {}
	~PrintArrayVisitor() {}
	virtual void apply(osg::Vec3dArray& v) override;
	std::vector<osg::Vec3f>		data;
};
void PrintArrayVisitor::apply(osg::Vec3dArray& v)
{
	for (osg::Vec3Array::size_type i = 0; i < v.size(); ++i)
	{
		//std::cout << v.at(i).x() << " " << v.at(i).y() << " " << v.at(i).z() << std::endl;
		data.push_back(osg::Vec3f(v.at(i).x() , v.at(i).y(), v.at(i).z()));
	}
}
class Vec3fConvertVisitor : public osg::ValueVisitor
{
public:
	virtual void apply(const osg::Vec3& v) 
	{
		r = osg::Vec3f(v[0], v[1], v[2]);
	}

	const osg::Vec3f& getResult() const { return r; }

protected:
	osg::Vec3f	 r;
};
bool			gPick = false;
double		gAtt = 0;
double		gSpan = 1000;		//防止出现小于 0 的透明值
void PickHandler::pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea)
{
	osgUtil::LineSegmentIntersector::Intersections intersections;

	std::string gdlist = "";
	gPick = false;
	gAtt = 0;
	if (view->computeIntersections(ea, intersections))
	{
		for (osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
			hitr != intersections.end();
			++hitr)
		{
			std::ostringstream os;
			if (!hitr->nodePath.empty() && !(hitr->nodePath.back()->getName().empty()))
			{
				// the geodes are identified by name.
				//os << "Object \"" << hitr->nodePath.back()->getName() << "\"" << std::endl;
			}
			else if (hitr->drawable.valid())
			{
				//os << "Object \"" << hitr->drawable->className() << "\"" << std::endl;
			}
			osg::Vec3d	 interSect = hitr->getLocalIntersectPoint();			
			osg::Vec3d	 normal = hitr->getLocalIntersectNormal();
			osg::Vec3d	worldInter = hitr->getWorldIntersectPoint();
			osg::Vec3d	 worldnormal	 = hitr->getWorldIntersectNormal();
			osg::Vec3d	localPosition;
			double			localZ = 0;
			g_geoSRS->transformFromWorld(worldInter , localPosition, &localZ);

			os << "Longitude:" << localPosition.x() << " Latitude:" << localPosition.y() << " Height:" << localZ;// << std::endl;
			
			if (hitr->drawable)
			{
				osg::Geometry* pGeo = hitr->drawable->asGeometry();
				if (pGeo)
				{
					osg::Vec4Array* colors = (osg::Vec4Array*)pGeo->getColorArray();
					if (colors)
					{
						osg::Vec4f		color;
						const osgUtil::LineSegmentIntersector::Intersection::IndexList& vil = hitr->indexList;
						for (unsigned int i = 0; i < vil.size(); ++i)
						{
							int		index = vil[i];
							if (index >= colors->size())
							{
								continue;
							}
							color = colors->at(index);
							//os << " Color(" << color.x() << "," << color.y() << "," << color.z() << "," << color.w() << " )" << std::endl;
							gPick = true;
							gAtt = color.w() - gSpan;
							break;
						}
					}
				}
			}
			gdlist += os.str();
			break;
		}
	}
	setLabel(gdlist);
}

osg::Node* createHUD(osgText::Text* updateText)
{

	// create the hud. derived from osgHud.cpp
	// adds a set of quads, each in a separate Geode - which can be picked individually
	// eg to be used as a menuing/help system!
	// Can pick texts too!

	osg::Camera* hudCamera = new osg::Camera;
	hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	hudCamera->setProjectionMatrixAsOrtho2D(0, 1280, 0, 1024);
	hudCamera->setViewMatrix(osg::Matrix::identity());
	hudCamera->setRenderOrder(osg::Camera::POST_RENDER);
	hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);

	std::string timesFont("fonts/times.ttf");

	// turn lighting off for the text and disable depth test to ensure its always ontop.
	osg::Vec3 position(0, 20, 0.0f);
	osg::Vec3 delta(0.0f, -60.0f, 0.0f);

	//{
	//	osg::Geode* geode = new osg::Geode();
	//	osg::StateSet* stateset = geode->getOrCreateStateSet();
	//	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	//	stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	//	geode->setName("simple");
	//	hudCamera->addChild(geode);

	//	osgText::Text* text = new  osgText::Text;
	//	geode->addDrawable(text);

	//	text->setFont(timesFont);
	//	text->setText("Picking in Head Up Displays is simple!");
	//	text->setPosition(position);

	//	position += delta;
	//}
	//for (int i = 0; i < 5; i++) {
	//	osg::Vec3 dy(0.0f, -30.0f, 0.0f);
	//	osg::Vec3 dx(120.0f, 0.0f, 0.0f);
	//	osg::Geode* geode = new osg::Geode();
	//	osg::StateSet* stateset = geode->getOrCreateStateSet();
	//	const char* opts[] = { "One", "Two", "Three", "January", "Feb", "2003" };
	//	osg::Geometry* quad = new osg::Geometry;
	//	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	//	stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	//	std::string name = "subOption";
	//	name += " ";
	//	name += std::string(opts[i]);
	//	geode->setName(name);
	//	osg::Vec3Array* vertices = new osg::Vec3Array(4); // 1 quad
	//	osg::Vec4Array* colors = new osg::Vec4Array;
	//	colors = new osg::Vec4Array;
	//	colors->push_back(osg::Vec4(0.8 - 0.1 * i, 0.1 * i, 0.2 * i, 1.0));
	//	quad->setColorArray(colors, osg::Array::BIND_OVERALL);
	//	(*vertices)[0] = position;
	//	(*vertices)[1] = position + dx;
	//	(*vertices)[2] = position + dx + dy;
	//	(*vertices)[3] = position + dy;
	//	quad->setVertexArray(vertices);
	//	quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
	//	geode->addDrawable(quad);
	//	hudCamera->addChild(geode);

	//	position += delta;
	//}
	{ // this displays what has been selected
		osg::Geode* geode = new osg::Geode();
		osg::StateSet* stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		geode->setName("The text label");
		geode->addDrawable(updateText);
		hudCamera->addChild(geode);
		updateText->setCharacterSize(20.0f);
		updateText->setFont(timesFont);
		updateText->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 0.8f));
		updateText->setText("");
		updateText->setPosition(position);
		updateText->setDataVariance(osg::Object::DYNAMIC);
		position += delta;
	}

	return hudCamera;

}

OSGEarthView::OSGEarthView(HWND hWnd)
	:OSGViewBase(hWnd)
{
}

OSGEarthView::~OSGEarthView()
{
	if (mViewer)
	{

	}
}

void OSGEarthView::InitOSG(std::string modelname)
{
	// Store the name of the model to load
	m_ModelName = modelname;

	// Init different parts of OSG
	InitManipulators();
	InitSceneGraph();
	InitCameraConfig();
}

void OSGEarthView::InitManipulators(void)
{
}
#define VSIZE 128
//从raw文件中创建3D纹理
osg::Texture3D* Create3DTextureFromRaw()
{
	osg::Texture3D* t3d = new osg::Texture3D();
	osg::Image* image = new osg::Image;
	image->allocateImage(VSIZE, VSIZE, VSIZE, GL_RGBA, GL_UNSIGNED_BYTE);
	t3d->setImage(image);

	osgDB::ifstream fin("buckball.raw", osgDB::ifstream::binary);

	for (int s = 0; s < VSIZE; s++)
	{
		for (int t = 0; t < VSIZE; t++)
		{
			for (int r = 0; r < VSIZE; r++)
			{
				unsigned char* datePtr = image->data(s, t, r);
				unsigned char dataTemp;
				fin.read((char*)(&dataTemp), 1);

				if (dataTemp < 10)
				{
					datePtr[0] = 0;
					datePtr[1] = 255;
					datePtr[2] = dataTemp;
					datePtr[3] = 0;
				}
				else if (dataTemp < 30)
				{
					datePtr[0] = dataTemp;
					datePtr[1] = 255;
					datePtr[2] = dataTemp;
					datePtr[3] = 10;
				}
				else if (dataTemp >= 150)
				{
					datePtr[0] = 255;
					datePtr[1] = 255;
					datePtr[2] = dataTemp;
					datePtr[3] = 100;
				}
				else
				{
					datePtr[0] = 0;
					datePtr[1] = 0;
					datePtr[2] = 0;
					datePtr[3] = 0;
				}


			}
		}
	}

	fin.close();

	t3d->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
	t3d->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
	t3d->setWrap(osg::Texture::WRAP_R, osg::Texture3D::CLAMP_TO_EDGE);
	t3d->setWrap(osg::Texture::WRAP_T, osg::Texture3D::CLAMP_TO_EDGE);
	t3d->setWrap(osg::Texture::WRAP_S, osg::Texture3D::CLAMP_TO_EDGE);

	return t3d;
}

//创建切片
osg::Node* CreateCube2(const SpatialReference* pGeos ,
	CDOT& centerPos, std::vector< std::vector<CDOT> >& centerDots, std::vector <vector< CDOT >>& Incolors, double xWidth, double yWidth, double zWidth)
{
	if (pGeos == NULL)
	{
		return NULL;
	}
	if ( centerDots.size() < 1 )
	{
		return NULL;
	}
	if (centerDots.size()  != Incolors.size() )
	{
		return NULL;
	}
	if (centerDots[0].size() < 1)
	{
		return NULL;
	}
	osg::Geode* gnode = new osg::Geode;
	osg::Geometry* geom = new osg::Geometry;
	gnode->addDrawable(geom);
	osg::Vec3Array* vertex = new osg::Vec3Array;
	osg::Vec4Array* colors = new osg::Vec4Array;
	geom->setVertexArray(vertex);
	geom->setColorArray(colors, osg::Array::BIND_PER_VERTEX);

	//经纬度 
	const Ellipsoid   ellipsoid = pGeos->getEllipsoid();
	Vec2d		dot1;
	Vec2d		dot2;

	dot1._v[0] = 114.31;
	dot1._v[1] = 30.52;
	
	dot2._v[0] = 114.41;
	dot2._v[1] = 30.52;
	double	distance =	ellipsoid.geodesicDistance( dot1 , dot2 );
	//
	double		xStep = 100;
	double		yStep = 100;
	
	vector< CDOT >		lineDots;
	int			countY = centerDots.size();
	int			countX = 0;
	int			iY = 0;
	int			iX = 0;
	dot1._v[0] = 114.31;
	dot1._v[1] = 30.52;

	dot2._v[0] = 114.41;
	dot2._v[1] = 30.52;

	if ( centerDots.size() > 1)
	{
		if ( centerDots[0].size() > 1)
		{
			dot1._v[0] = centerDots[0][0].x;
			dot1._v[1] = centerDots[0][0].y;
			dot2._v[0] = centerDots[0][1].x;
			dot2._v[1] = centerDots[0][1].y;
			xStep = ellipsoid.geodesicDistance(dot1, dot2);
		}
		if (centerDots[1].size() > 0)
		{
			dot2._v[0] = centerDots[1][0].x;
			dot2._v[1] = centerDots[1][0].y;
			yStep = ellipsoid.geodesicDistance(dot1, dot2);
		}
	}
	Vec3f		halfSize[4];
	halfSize[0]._v[0] = -xStep * 0.5;
	halfSize[0]._v[1] = -yStep * 0.5;
	halfSize[0]._v[2] = 0;
	halfSize[1]._v[0] = xStep * 0.5;
	halfSize[1]._v[1] = -yStep * 0.5;
	halfSize[1]._v[2] = 0;
	halfSize[2]._v[0] = xStep * 0.5;
	halfSize[2]._v[1] = yStep * 0.5;
	halfSize[2]._v[2] = 0;
	halfSize[3]._v[0] = -xStep * 0.5;
	halfSize[3]._v[1] = yStep * 0.5;
	halfSize[3]._v[2] = 0;
	
	osg::Vec3f		center;
	center._v[0] = 0;
	center._v[1] = 0;// +0.5 * yStep;
	center._v[2] = 0;// centerDots[0][0].z;
	iY = 0;
	iX = 0;
	for (iY =0 ; iY < countY;++iY )
	{
		lineDots = centerDots[iY];
		countX = lineDots.size();
		center._v[0] = 0;// +0.5 * xStep;
		center._v[1] += yStep;
		for (iX =0; iX < countX;++iX)
		{
			center._v[0] += xStep;
			{
				osg::Vec3f		center0 = center + halfSize[0];
				vertex->push_back(center0);
				osg::Vec3f		center1 = center + halfSize[1];
				vertex->push_back(center1);
				osg::Vec3f		center2 = center + halfSize[2];
				vertex->push_back(center2);
				osg::Vec3f		center3 = center + halfSize[3];
				vertex->push_back(center3);
			}
			{
				osg::Vec4f  temColor;
				temColor._v[0] = Incolors[iY][iX].x;
				temColor._v[1] = Incolors[iY][iX].y;
				temColor._v[2] = Incolors[iY][iX].z;
				temColor._v[3] = Incolors[iY][iX].w + gSpan;
				colors->push_back(temColor);
				colors->push_back(temColor);
				colors->push_back(temColor);
				colors->push_back(temColor);
			}
		}
	}
	//  y   朝北方向    z竖直向上
	//for (int y = 128 - 1; y >= 0; y--)
	//{
	//	//压入顶点
	//	vertex->push_back(osg::Vec3f(-distance, -distance, 1000 * y * 1.0 / VSIZE));
	//	vertex->push_back(osg::Vec3f(-distance, distance, 1000 * y * 1.0 / VSIZE));
	//	vertex->push_back(osg::Vec3f(distance, distance, 1000 * y * 1.0 / VSIZE));
	//	vertex->push_back(osg::Vec3f(distance, -distance, 1000 * y * 1.0 / VSIZE));

	//	colors->push_back(osg::Vec3f(y * 1.0 / 128, 1 - y * 1.0 / 128, 0));
	//	colors->push_back(osg::Vec3f(y * 1.0 / 128, 1 - y * 1.0 / 128, 0));
	//	colors->push_back(osg::Vec3f(y * 1.0 / 128, 1 - y * 1.0 / 128, 0));
	//	colors->push_back(osg::Vec3f(y * 1.0 / 128, 1 - y * 1.0 / 128, 0));
	//}
	geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, vertex->size()));

	return gnode;
}

osg::Node* CreateCubeHeight(const SpatialReference* pGeos,
	CDOT& centerPos, std::vector< std::vector<CDOT> >& centerDots, std::vector <std::vector<double>>& heights, double xWidth, double yWidth, double zWidth)
{
	if (pGeos == NULL)
	{
		return NULL;
	}
	if (centerDots.size() < 1)
	{
		return NULL;
	}
	if (centerDots.size() != heights.size())
	{
		return NULL;
	}
	if (centerDots[0].size() < 1)
	{
		return NULL;
	}
	osg::Geode* gnode = new osg::Geode;
	osg::Geometry* geom = new osg::Geometry;
	gnode->addDrawable(geom);
	osg::Vec3Array* vertex = new osg::Vec3Array;
	osg::Vec4Array* colors = new osg::Vec4Array;
	geom->setVertexArray(vertex);
	geom->setColorArray(colors, osg::Array::BIND_PER_VERTEX);

	//经纬度 
	const Ellipsoid   ellipsoid = pGeos->getEllipsoid();
	Vec2d		dot1;
	Vec2d		dot2;

	dot1._v[0] = 114.31;
	dot1._v[1] = 30.52;

	dot2._v[0] = 114.41;
	dot2._v[1] = 30.52;
	double	distance = ellipsoid.geodesicDistance(dot1, dot2);
	//
	double		xStep = 100;
	double		yStep = 100;

	vector< CDOT >		lineDots;
	int			countY = centerDots.size();
	int			countX = 0;
	int			iY = 0;
	int			iX = 0;
	if (centerDots.size() > 1)
	{
		if (centerDots[0].size() > 1)
		{
			dot1._v[0] = centerDots[0][0].x;
			dot1._v[1] = centerDots[0][0].y;
			dot2._v[0] = centerDots[0][1].x;
			dot2._v[1] = centerDots[0][1].y;
			xStep = ellipsoid.geodesicDistance(dot1, dot2);
		}
		if (centerDots[1].size() > 0)
		{
			dot2._v[0] = centerDots[1][0].x;
			dot2._v[1] = centerDots[1][0].y;
			yStep = ellipsoid.geodesicDistance(dot1, dot2);
		}
	}
	Vec3f		halfSize[4];
	halfSize[0]._v[0] = -xStep * 0.5;
	halfSize[0]._v[1] = -yStep * 0.5;
	halfSize[0]._v[2] = 0;
	halfSize[1]._v[0] = xStep * 0.5;
	halfSize[1]._v[1] = -yStep * 0.5;
	halfSize[1]._v[2] = 0;
	halfSize[2]._v[0] = xStep * 0.5;
	halfSize[2]._v[1] = yStep * 0.5;
	halfSize[2]._v[2] = 0;
	halfSize[3]._v[0] = -xStep * 0.5;
	halfSize[3]._v[1] = yStep * 0.5;
	halfSize[3]._v[2] = 0;

	osg::Vec3f		center;
	center._v[0] = 0;
	center._v[1] = 0;
	center._v[2] = centerDots[0][0].z;
	iY = 0;
	iX = 0;

	double		minH = 1e+38;
	double	    maxH = -1e+30;
	for (int j = 0; j < heights.size();++j)
	{
		for (int i =0; i < heights[j].size();++i )
		{
			minH = min(heights[j][i], minH);
			maxH = max(heights[j][i], maxH);
		}
	}
	double		hSpan = maxH - minH;
	double		scale = 20;
	for (iY = 0; iY < countY- 1; ++iY)
	{
		lineDots = centerDots[iY];
		countX = lineDots.size();
		center._v[0] = 0;
		center._v[1] += yStep;
		for (iX = 0; iX < countX - 1; ++iX)
		{
			center._v[0] += xStep;
			{
				//012
				osg::Vec3f		center0 = center + halfSize[0];
				center0._v[2] = scale * heights[iY][iX];
				vertex->push_back(center0);

				osg::Vec3f		center1 = center + halfSize[1];
				center1._v[2] = scale * heights[iY][iX + 1];
				vertex->push_back(center1);

				osg::Vec3f		center2 = center + halfSize[2];
				center2._v[2] = scale * heights[iY + 1][iX + 1];
				vertex->push_back(center2);

				//023 
				vertex->push_back(center0);
				vertex->push_back(center2);
				osg::Vec3f		center3 = center + halfSize[3];
				center3._v[2] = scale * heights[iY + 1][iX];
				vertex->push_back(center3);
			}
			{
				double		h0 = heights[iY][iX];
				double		h1 = heights[iY][iX + 1];
				double		h2 = heights[iY + 1][iX + 1];
				double		h3 = heights[iY + 1][iX];

				double		hh0 = (h0 - minH) / hSpan;
				double		hh1 = (h1 - minH) / hSpan;
				double		hh2 = (h2 - minH) / hSpan;
				double		hh3 = (h3 - minH) / hSpan;
				osg::Vec4f  temColor;
				temColor._v[0] = hh0;
				temColor._v[1] = hh0;
				temColor._v[2] = hh0;
				temColor._v[3] = h0 + gSpan;
				colors->push_back(temColor);
				temColor._v[0] = hh1;
				temColor._v[1] = hh1;
				temColor._v[2] = hh1;
				temColor._v[3] = h1 + gSpan;
				colors->push_back(temColor);
				temColor._v[0] = hh2;
				temColor._v[1] = hh2;
				temColor._v[2] = hh2;
				temColor._v[3] = h2 + gSpan;
				colors->push_back(temColor);
				temColor._v[0] = hh0;
				temColor._v[1] = hh0;
				temColor._v[2] = hh0;
				temColor._v[3] = h0 + gSpan;
				colors->push_back(temColor);
				temColor._v[0] = hh2;
				temColor._v[1] = hh2;
				temColor._v[2] = hh2;
				temColor._v[3] = h2 + gSpan;
				colors->push_back(temColor);
				temColor._v[0] = hh3;
				temColor._v[1] = hh3;
				temColor._v[2] = hh3;
				temColor._v[3] = h3 + gSpan;
				colors->push_back(temColor);
			}
		}
	}
	//  y   朝北方向    z竖直向上
	//for (int y = 128 - 1; y >= 0; y--)
	//{
	//	//压入顶点
	//	vertex->push_back(osg::Vec3f(-distance, -distance, 1000 * y * 1.0 / VSIZE));
	//	vertex->push_back(osg::Vec3f(-distance, distance, 1000 * y * 1.0 / VSIZE));
	//	vertex->push_back(osg::Vec3f(distance, distance, 1000 * y * 1.0 / VSIZE));
	//	vertex->push_back(osg::Vec3f(distance, -distance, 1000 * y * 1.0 / VSIZE));

	//	colors->push_back(osg::Vec3f(y * 1.0 / 128, 1 - y * 1.0 / 128, 0));
	//	colors->push_back(osg::Vec3f(y * 1.0 / 128, 1 - y * 1.0 / 128, 0));
	//	colors->push_back(osg::Vec3f(y * 1.0 / 128, 1 - y * 1.0 / 128, 0));
	//	colors->push_back(osg::Vec3f(y * 1.0 / 128, 1 - y * 1.0 / 128, 0));
	//}
	geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES/*GL_QUADS*/, 0, vertex->size()));

	return gnode;
}


void OSGEarthView::InitSceneGraph(void)
{
	// Init the main Root Node/Group
	mRoot = new osg::Group;
	int argc = 2;
	char* argv[2];
	
	TCHAR szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	
	//(_tcsrchr(szFilePath, _T('\\')))[1] = 0;				// 删除文件名，只获得路径字串
	CString str_url = szFilePath;
	argv[0] = str_url.GetBuffer();// "I:\\OSG\\osgearth\\build64\\bin\\Release\\Demo.exe";//_pgmptr;

	argv[1] = (char*)m_ModelName.c_str();
	osg::ArgumentParser arguments(&argc, argv);

	// Create the mViewer for this window
	mViewer = new osgViewer::Viewer();

	// Load the Model from the model name
	mModel = MapNodeHelper().load(arguments, mViewer);
	if (!mModel) return;

	// Optimize the model
	osgUtil::Optimizer optimizer;
	optimizer.optimize(mModel);
	optimizer.reset();

	// Add the model to the scene
	mRoot->addChild(mModel);

	mpMapNode = MapNode::findMapNode(mModel);
	if (mpMapNode == NULL)
	{
		return;
	}	
	const SpatialReference* geoSRS = mpMapNode->getMapSRS()->getGeographicSRS();
	g_geoSRS = geoSRS;
	mpGeoSRS = geoSRS;

	mpModelGroupNode = new osg::Group();
	mpTextGroupNode = new osg::Group();

	MapNode::get(mModel)->addChild(mpModelGroupNode);
	MapNode::get(mModel)->addChild(mpTextGroupNode);

	if (mpGeoSRS)
	{
		//mEllipsoid = mpGeoSRS->getEllipsoid();
	}
	//double		stepDeg = mEllipsoid.metersToLongitudinalDegrees(10, 30);
	{
		//Style rectStyle;
		//rectStyle.getOrCreate<PolygonSymbol>()->fill()->color() = Color(Color::Green, 0.5);
		//rectStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_NONE/*CLAMP_TO_TERRAIN*/;
		//rectStyle.getOrCreate<AltitudeSymbol>()->technique() = AltitudeSymbol::TECHNIQUE_DRAPE;
		//int		x = 0;
		//int		y = 0;
		//int		xNum = 100;
		//int		yNum = 100;
		//osg::Group* rectsGroupNode = new osg::Group();
		//double	height = 100;
		//for (x = 0; x < xNum;++x)
		//{
		//	//for (y = 0;y < yNum;++y)
		//	{
		//		RectangleNode* rect = new RectangleNode(
		//			GeoPoint(geoSRS, 114 , 30 , height, AltitudeMode::ALTMODE_ABSOLUTE),
		//			Distance(300, Units::KILOMETERS),
		//			Distance(600, Units::KILOMETERS),
		//			rectStyle);
		//		height += 200.0;
		//		rectsGroupNode->addChild(rect);
		//	}
		//}
		//RectGroupNode->addChild(rectsGroupNode);
	}
	osg::ref_ptr<osgText::Text> updateText = new osgText::Text;
	mViewer->addEventHandler(new PickHandler(updateText.get()));
	mpTextGroupNode->addChild(createHUD(updateText.get()));
}

void OSGEarthView::InitCameraConfig(void)
{
	// Local Variable to hold window size data
	RECT rect;

	// Add a Stats Handler to the mViewer
	mViewer->addEventHandler(new osgViewer::StatsHandler);

	// Get the current window size
	::GetWindowRect(m_hWnd, &rect);

	// Init the GraphicsContext Traits
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

	// Init the Windata Variable that holds the handle for the Window to display OSG in.
	osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowWin32::WindowData(m_hWnd);

	// Setup the traits parameters
	traits->x = 0;
	traits->y = 0;
	traits->width = rect.right - rect.left;
	traits->height = rect.bottom - rect.top;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;

	traits->sharedContext = 0;

	traits->setInheritedWindowPixelFormat = true;
	traits->inheritedWindowData = windata;

	// Create the Graphics Context
	osg::GraphicsContext* gc = osg::GraphicsContext::createGraphicsContext(traits.get());

	// Init Master Camera for this View
	osg::ref_ptr<osg::Camera> camera = mViewer->getCamera();

	// Assign Graphics Context to the Camera
	camera->setGraphicsContext(gc);

	// Set the viewport for the Camera
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

	// set the draw and read buffers up for a double buffered window with rendering going to back buffer
	camera->setDrawBuffer(GL_BACK);
	camera->setReadBuffer(GL_BACK);

	// Set projection matrix and camera attribtues
	camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	camera->setClearColor(osg::Vec4f(0.0f, 0.0f, 0.0f, 1.0f));
	camera->setProjectionMatrixAsPerspective(
		30.0f, static_cast<double>(traits->width) / static_cast<double>(traits->height), 1.0, 1000.0);

	mViewer->setCamera(camera.get());

	osgEarth::MapNode* mapNode = osgEarth::MapNode::findMapNode(mRoot);
	mMapNode = mapNode;
	osgEarth::Map* map = mapNode->getMap();
	mMap = map;
	
	const SpatialReference* SpRef = map->getSRS();
	double equatorRadius = SpRef->getEllipsoid().getRadiusEquator();
	mViewer->getCamera()->addCullCallback(new osgEarth::Util::AutoClipPlaneCullCallback(mapNode));

	mpManip = new EarthManipulator();
	mpManip->setHomeViewpoint(osgEarth::Util::Viewpoint("北京", 116.3, 39.9, 0, 0, -90, equatorRadius * 4));
	mViewer->setCameraManipulator(mpManip);

	// Set the Scene Data
	mViewer->setSceneData(mRoot);
	mViewer->setThreadingModel(osgViewer::ViewerBase::ThreadingModel::SingleThreaded);

	mViewer->addEventHandler(new osgViewer::StatsHandler);
	mViewer->addEventHandler(new osgGA::StateSetManipulator(mViewer->getCamera()->getOrCreateStateSet()));
	mViewer->addEventHandler(new osgViewer::ThreadingHandler);
	mViewer->addEventHandler(new osgViewer::RecordCameraPathHandler);
	mViewer->addEventHandler(new osgViewer::LODScaleHandler);
	mViewer->addEventHandler(new osgViewer::ScreenCaptureHandler);
	mViewer->realize();
}

void OSGEarthView::PreFrameUpdate()
{
	// Due any preframe updates in this routine
}

void OSGEarthView::PostFrameUpdate()
{
	// Due any postframe updates in this routine
}

double OSGEarthView::metersToLongitudinalDegrees(double value, double lat_deg)
{
	if (mpGeoSRS)
	{
		const Ellipsoid	tem = mpGeoSRS->getEllipsoid();
		return tem.metersToLongitudinalDegrees(value, lat_deg);
	}
	return 0;
}

int OSGEarthView::DrawPlane(CDOT& centerPos, std::vector< std::vector<CDOT> >& centerDots, std::vector <vector< CDOT >>& colors, 
	double xWidth, double yWidth, double zWidth)
{
	if (mpModelGroupNode == NULL)
	{
		return -1;
	}
	++guid;
	if (mMapNode)
	{
		osg::Node*					myModel = CreateCube2(
			mpGeoSRS,
			centerPos , centerDots , colors , xWidth , yWidth , zWidth
		);
		// osgDB::readNodeFile("test.osg");
		const SpatialReference* latLong = mpGeoSRS;
		Style style;
		style.getOrCreate<ModelSymbol>()->setModel(myModel);
		ModelNode* modelNode = new ModelNode(mpMapNode, style);
		modelNode->setPosition(GeoPoint(latLong, centerPos.x, centerPos.y, centerPos.z, AltitudeMode::ALTMODE_ABSOLUTE));
		mpModelGroupNode->addChild(modelNode);
		mDrawNodes[guid] = modelNode;

//		mViewer->setCameraManipulator(earthManip);		//必须在setViewpoint之前
		{
			//osgEarth::Geometry* utah = new osgEarth::Polygon();
			//utah->push_back(115.31, 30.52);
			//utah->push_back(115.51, 30.52);
			//utah->push_back(115.51, 32);
			//utah->push_back(115.31, 32);
			//utah->push_back(115.31, 30.52);
			//Style utahStyle;
			//utahStyle.getOrCreate<ExtrusionSymbol>()->height() = 1000; // meters MSL
			//utahStyle.getOrCreate<PolygonSymbol>()->fill()->color() = Color(Color::Red, 0.8);
			//Feature* utahFeature = new Feature(utah, mpGeoSRS);
			//FeatureNode* featureNode = new FeatureNode(utahFeature, utahStyle);

			//mpModelGroupNode->addChild(featureNode);
		}
		{
			//Style rectStyle;
			//rectStyle.getOrCreate<PolygonSymbol>()->fill()->color() = Color(Color::Green, 0.5);
			//rectStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_TO_TERRAIN;
			//rectStyle.getOrCreate<AltitudeSymbol>()->technique() = AltitudeSymbol::TECHNIQUE_DRAPE;
			//RectangleNode* rect = new RectangleNode(
			//	GeoPoint(mpGeoSRS, 114.31, 31.52),
			//	Distance(300, Units::METERS),
			//	Distance(600, Units::METERS),
			//	rectStyle);
			//mpModelGroupNode->addChild(rect);
		}
		{
			//osgEarth::Geometry* geom1 = new osgEarth::Polygon();
			//geom1->push_back(-160., -30.);
			//geom1->push_back(150., -20.);
			//geom1->push_back(160., -45.);
			//geom1->push_back(-150., -40.);
			//Style geomStyle;
			//Feature* feature = new Feature(geom1, mpGeoSRS);
			//feature->geoInterp() = GEOINTERP_RHUMB_LINE;
			//geomStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color::Lime;
			//geomStyle.getOrCreate<LineSymbol>()->stroke()->width() = 3.0f;
			//geomStyle.getOrCreate<LineSymbol>()->tessellationSize()->set(75000, Units::METERS);
			//geomStyle.getOrCreate<RenderSymbol>()->depthOffset()->enabled() = true;
			//FeatureNode* gnode = new FeatureNode(feature, geomStyle);
			//mpModelGroupNode->addChild(gnode);
		}


		//LabelNode* label = new LabelNode("Antimeridian polygon", labelStyle);
		//label->setPosition(GeoPoint(geoSRS, -175, -35));
		//labelGroup->addChild(label);


	}
	return guid;
}

int OSGEarthView::DrawSurface(CDOT& centerPos, std::vector< std::vector<CDOT> >& centerDots, std::vector <vector< double >>& heights, double xWidth, double yWidth, double zWidth)
{
	if (mpModelGroupNode == NULL)
	{
		return -1;
	}
	++guid;
	if (mMapNode)
	{
		osg::Node* myModel = CreateCubeHeight(
			mpGeoSRS,
			centerPos, centerDots, heights, xWidth, yWidth, zWidth
		);
		const SpatialReference* latLong = mpGeoSRS;
		Style style;
		style.getOrCreate<ModelSymbol>()->setModel(myModel);
		ModelNode* modelNode = new ModelNode(mpMapNode, style);

		modelNode->setPosition(GeoPoint(latLong, centerPos.x, centerPos.y , centerPos.z, AltitudeMode::ALTMODE_ABSOLUTE));
		mpModelGroupNode->addChild(modelNode);

		mDrawNodes[guid] = modelNode;
	}
	return guid;
}

bool OSGEarthView::RemovePlane(int id)
{
	MapIDNode::iterator		findID;
	findID = mDrawNodes.find(id);

	if (findID != mDrawNodes.end())
	{
		ModelNode* modelNode = (ModelNode*)findID->second;
		if (modelNode && mpModelGroupNode)
		{
			mpModelGroupNode->removeChild(modelNode);
		}
		//Style style = modelNode->getStyle();

		//osg::ref_ptr<const ModelSymbol> sym = style.get<ModelSymbol>();
		//if (sym.valid())
		//{
		//	if ((sym->url().isSet()) || (sym->getModel() != NULL))
		//	{
		//		// Try to get a model from symbol
		//		osg::ref_ptr<osg::Node> node = sym->getModel();
		//		if ( node.valid() )
		//		{
		//			node.release();
		//		}
		//	}
		//}
		mDrawNodes.erase(findID);
		return true;
	}
	return false;
}

void OSGEarthView::Jump(double lon, double lat, double z, double heading, double pitch, double range)
{
	if (mpManip)
	{
		osgEarth::Viewpoint vp("", lon, lat, z, heading , pitch , range);
		mpManip->setViewpoint(vp, 0);
	}
}

bool OSGEarthView::GetAtt(double& att)
{
	att = gAtt;
	return gPick;
}

CRenderingThread::CRenderingThread(OSGViewBase* ptr)
	: OpenThreads::Thread(), _ptr(ptr), _done(false)
{
	_osgRuningFlag = true;
	_osgRuningState = true;
	_RemoveID = NULL;
	removeIDNum = 0;
}
CRenderingThread::CRenderingThread(OSGViewBase* ptr, bool is3dThread)
	: OpenThreads::Thread(), _ptr(ptr), _done(false)
{
	_is3Dthread = is3dThread;
	_osgRuningFlag = true;
	_osgRuningState = true;
	_RemoveID = NULL;
	removeIDNum = 0;
}

CRenderingThread::~CRenderingThread()
{
	_done = true;
	if (isRunning())
	{
		cancel();
		join();
	}
}

void CRenderingThread::Done(bool flag)
{
	_done = flag;
}

void  CRenderingThread::setOsgRuningFlag(bool b)
{
	_osgRuningFlag = b;
}

bool  CRenderingThread::getOsgRuningState()
{
	return _osgRuningState;
}
bool CRenderingThread::isEnd()
{
	osgViewer::Viewer* mViewer = _ptr->getViewer();
	if (!testCancel() && !mViewer->done() && !_done)
		return false;
	else
		return true;
}

void CRenderingThread::RemoveID(int* id, int num)
{
	if (_RemoveID)
	{
		delete[]_RemoveID;
		_RemoveID = NULL;
	}
	if (_RemoveID == NULL)
	{
		_RemoveID = new int[num];
		memcpy(_RemoveID, id, num * sizeof(int));
	}
	removeIDNum = num;
}
void CRenderingThread::run()
{
	if (!_ptr)
	{
		_done = true;
		return;
	}

	osgViewer::Viewer* mViewer = _ptr->getViewer();
	do
	{
		if (_osgRuningFlag)
		{
			_ptr->PreFrameUpdate();
			mViewer->frame();
			_ptr->PostFrameUpdate();
			_osgRuningState = true;
		}
		else
		{
			_osgRuningState = false;
			Sleep(5);
		}
		if  ( removeIDNum > 0)
		{
			OSGViewBase* pBase = _ptr;
			if (pBase && _RemoveID)
			{
				for (int k =0; k < removeIDNum;++k)
				{
					pBase->RemovePlane(_RemoveID[k]);
				}
			}
			removeIDNum = 0;
		}
	} while (!testCancel() && !mViewer->done() && !_done);
}