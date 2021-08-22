// MFC_OSG.cpp : implementation of the cOSG class
//
#include "pch.h"
#include "OSGView.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace OSGCLR;
char* ManagedString2UnmanagedStringA(System::String^ strIn)
{
	if (strIn == nullptr)
		return NULL;

	IntPtr ip = Marshal::StringToHGlobalAnsi(strIn);
	const char* pTemp = static_cast<const char*>(ip.ToPointer());
	if (pTemp == NULL) return NULL;
	char* pOut = new char[strlen(pTemp) + 1];
	strcpy(pOut, pTemp);
	Marshal::FreeHGlobal(ip);
	return pOut;
}

wchar_t* ManagedString2UnmanagedStringW(System::String^ strIn)
{
	if (strIn == nullptr)
		return NULL;

	IntPtr ip = Marshal::StringToHGlobalUni(strIn);
	const wchar_t* pTemp = static_cast<const wchar_t*>(ip.ToPointer());
	if (pTemp == NULL) return NULL;
	wchar_t* pOut = new wchar_t[wcslen(pTemp) + 1];
	wcscpy(pOut, pTemp);
	Marshal::FreeHGlobal(ip);
	return pOut;
}

OSGViewInner::OSGViewInner(HWND viewHandle)
{
    m_hWnd = viewHandle;

	osgViewer::Viewer* viewer = new osgViewer::Viewer;
	osg::Group* group = new osg::Group;
   
    char* path = "cow.osg";
    System::String^ strIn = gcnew System::String(path);

    path = ManagedString2UnmanagedStringA(strIn);

	group->addChild(osgDB::readNodeFile( path ));

	viewer->setSceneData(group);
    viewer->run();
}

OSGViewInner::~OSGViewInner()
{
    mViewer->setDone(true);
    Sleep(1000);
    mViewer->stopThreading();

    delete mViewer;
}

void OSGViewInner::InitOSG(std::string filename)
{

    // Store the name of the model to load

    filename = "robot.osg";

    m_ModelName = filename;

    // Init different parts of OSG
    InitManipulators();
    InitSceneGraph();
    InitCameraConfig();
}

void OSGViewInner::InitManipulators(void)
{
    // Create a trackball manipulator
    trackball = new osgGA::TrackballManipulator();

    // Create a Manipulator Switcher
    keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;
    if (keyswitchManipulator)
    {
		// Add our trackball manipulator to the switcher
		//keyswitchManipulator->addMatrixManipulator('1', "Trackball", trackball.get());

		// Init the switcher to the first manipulator (in this case the only manipulator)
		//keyswitchManipulator->selectMatrixManipulator(0);  // Zero based index Value
    }

}


void OSGViewInner::InitSceneGraph(void)
{
    // Init the main Root Node/Group
    mRoot = new osg::Group;

    // Load the Model from the model name
    mModel = osgDB::readNodeFile(m_ModelName);
    if (!mModel) return;

    // Optimize the model
    osgUtil::Optimizer optimizer;
    optimizer.optimize(mModel.get());
    optimizer.reset();

    // Add the model to the scene
    mRoot->addChild(mModel.get());
}

void OSGViewInner::InitCameraConfig(void)
{
    // Local Variable to hold window size data
    RECT rect;

    // Create the viewer for this window
    mViewer = new osgViewer::Viewer();

    // Add a Stats Handler to the viewer
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
    camera->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.4f, 1.0f));
    camera->setProjectionMatrixAsPerspective(
        30.0f, static_cast<double>(traits->width) / static_cast<double>(traits->height), 1.0, 1000.0);

    // Add the Camera to the Viewer
    //mViewer->addSlave(camera.get());
    mViewer->setCamera(camera.get());

    // Add the Camera Manipulator to the Viewer
    mViewer->setCameraManipulator(keyswitchManipulator.get());

    // Set the Scene Data
    mViewer->setSceneData(mRoot.get());

    // Realize the Viewer
    mViewer->realize();

    // Correct aspect ratio
    /*double fovy,aspectRatio,z1,z2;
    mViewer->getCamera()->getProjectionMatrixAsPerspective(fovy,aspectRatio,z1,z2);
    aspectRatio=double(traits->width)/double(traits->height);
    mViewer->getCamera()->setProjectionMatrixAsPerspective(fovy,aspectRatio,z1,z2);*/
}

void OSGViewInner::PreFrameUpdate()
{
    // Due any preframe updates in this routine
}

void OSGViewInner::PostFrameUpdate()
{
    // Due any postframe updates in this routine
}

/*void cOSG::Render(void* ptr)
{
    cOSG* osg = (cOSG*)ptr;

    osgViewer::Viewer* viewer = osg->getViewer();

    // You have two options for the main viewer loop
    //      viewer->run()   or
    //      while(!viewer->done()) { viewer->frame(); }

    //viewer->run();
    while(!viewer->done())
    {
        osg->PreFrameUpdate();
        viewer->frame();
        osg->PostFrameUpdate();
        //Sleep(10);         // Use this command if you need to allow other processes to have cpu time
    }

    // For some reason this has to be here to avoid issue:
    // if you have multiple OSG windows up
    // and you exit one then all stop rendering
    AfxMessageBox("Exit Rendering Thread");

    _endthread();
}*/

CRenderingThread::CRenderingThread(OSGViewInner* ptr)
    : OpenThreads::Thread(), _ptr(ptr), _done(false)
{
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
        _ptr->PreFrameUpdate();
        viewer->frame();
        _ptr->PostFrameUpdate();
    } while (!testCancel() && !viewer->done() && !_done);
}

OSGViewWrap::OSGViewWrap(System::IntPtr viewHandle)
{
    mpView = NULL;
    if (viewHandle != IntPtr::Zero )
    {
		HWND        hWnd = (HWND)(viewHandle.ToPointer());
		mpView = new OSGViewInner(hWnd);
    }
}
OSGViewWrap::~OSGViewWrap()
{

}

void OSGViewWrap::Render()
{
    if (mpView == NULL)
    {
        return;
    }
    //const char* szName = ManagedString2UnmanagedStringA(filename);
	mpView->InitOSG( "");
	// Start the thread to do OSG Rendering
	//mThreadHandle = (HANDLE)_beginthread(&cOSG::Render, 0, mOSG); 
    CRenderingThread* pRenderThread = new CRenderingThread( mpView );
    pRenderThread->start();
}
