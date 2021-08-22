#include "pch.h"
#include "OSGEarthView.h"

OSGWrapper::OSGWrapper(HWND hWnd) :
	m_hWnd(hWnd)
{
}

OSGWrapper::~OSGWrapper()
{
	viewer->setDone(true);
	Sleep(100);
	viewer->stopThreading();
	viewer.~ref_ptr();
	delete viewer;
}

void OSGWrapper::InitOSG(std::string modelname)
{
	// Store the name of the model to load
	m_ModelName = modelname;

	// Init different parts of OSG
	InitManipulators();
	InitSceneGraph();
	InitCameraConfig();
}

void OSGWrapper::InitManipulators(void)
{
}

void OSGWrapper::InitSceneGraph(void)
{
	// Init the main Root Node/Group
	mRoot = new osg::Group;

	int argc = 2;
	char* argv[2];

	argv[0] = _pgmptr;
	argv[1] = (char*)m_ModelName.c_str();
	osg::ArgumentParser arguments(&argc, argv);

	// Create the viewer for this window
	viewer = new osgViewer::Viewer();

	// Load the Model from the model name
	mModel = MapNodeHelper().load(arguments, viewer);
	if (!mModel) return;

	// Optimize the model
	osgUtil::Optimizer optimizer;
	optimizer.optimize(mModel);
	optimizer.reset();

	// Add the model to the scene
	mRoot->addChild(mModel);
}

void OSGWrapper::InitCameraConfig(void)
{
	// Local Variable to hold window size data
	RECT rect;

	// Add a Stats Handler to the viewer
	viewer->addEventHandler(new osgViewer::StatsHandler);

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
	osg::ref_ptr<osg::Camera> camera = viewer->getCamera();

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

	viewer->setCamera(camera.get());

	osgEarth::MapNode* mapNode = osgEarth::MapNode::findMapNode(mRoot);
	mMapNode = mapNode;
	osgEarth::Map* map = mapNode->getMap();
	mMap = map;
	double equatorRadius = map->getSRS()->getEllipsoid()->getRadiusEquator();
	viewer->getCamera()->addCullCallback(new osgEarth::Util::AutoClipPlaneCullCallback(mapNode));

	manip = new EarthManipulator();
	manip->setHomeViewpoint(osgEarth::Util::Viewpoint("±±¾©", 116.3, 39.9, 0, 0, -90, equatorRadius * 4));
	viewer->setCameraManipulator(manip);

	// Set the Scene Data
	viewer->setSceneData(mRoot);
	viewer->setThreadingModel(osgViewer::ViewerBase::ThreadingModel::SingleThreaded);

	viewer->addEventHandler(new osgViewer::StatsHandler);
	viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
	viewer->addEventHandler(new osgViewer::ThreadingHandler);
	viewer->addEventHandler(new osgViewer::RecordCameraPathHandler);
	viewer->addEventHandler(new osgViewer::LODScaleHandler);
	viewer->addEventHandler(new osgViewer::ScreenCaptureHandler);
	viewer->realize();
}

void OSGWrapper::PreFrameUpdate()
{
	// Due any preframe updates in this routine
}

void OSGWrapper::PostFrameUpdate()
{
	// Due any postframe updates in this routine
}

CRenderingThread::CRenderingThread(OSGWrapper* ptr)
	: OpenThreads::Thread(), _ptr(ptr), _done(false)
{
	_osgRuningFlag = true;
	_osgRuningState = true;
}
CRenderingThread::CRenderingThread(OSGWrapper* ptr, bool is3dThread)
	: OpenThreads::Thread(), _ptr(ptr), _done(false)
{
	_is3Dthread = is3dThread;
	_osgRuningFlag = true;
	_osgRuningState = true;
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
	osgViewer::Viewer* viewer = _ptr->getViewer();
	if (!testCancel() && !viewer->done() && !_done)
		return false;
	else
		return true;
}

void CRenderingThread::run()
{
	if (!_ptr)
	{
		_done = true;
		return;
	}

	osgViewer::Viewer* viewer = _ptr->getViewer();
	do
	{
		if (_osgRuningFlag)
		{
			_ptr->PreFrameUpdate();
			viewer->frame();
			_ptr->PostFrameUpdate();
			_osgRuningState = true;
		}
		else
		{
			_osgRuningState = false;
			Sleep(5);
		}
	} while (!testCancel() && !viewer->done() && !_done);
}