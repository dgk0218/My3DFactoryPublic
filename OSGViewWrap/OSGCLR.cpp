#include "pch.h"

#include "OSGView.h"

//int OSGCLR::OSGCLRDLL::InitDLL(IntPtr _hwnd2, bool is3D)
//{
//	HWND _hwnd = (HWND)_hwnd2.ToPointer();
//	if (is3D)
//	{
//		//通过传入的句柄初始化cOSG对象
//		mOSG3D = new OSGWrapper(_hwnd);
//		mOSG3D->InitOSG("Earth3D.earth");
//
//		//创建渲染线程并开始渲染
//		m3DThreadHandle = new CRenderingThread(mOSG3D, false);
//		m3DThreadHandle->start();
//	}
//	else
//	{
//		//通过传入的句柄初始化cOSG对象
//		mOSG2D = new OSGWrapper(_hwnd);
//		mOSG2D->InitOSG("Map2D.earth");
//		double fov, ratio, near1, far1;
//		mOSG2D->getViewer()->getCamera()->getProjectionMatrixAsPerspective(fov, ratio, near1, far1);
//		mOSG2D->getViewer()->getCamera()->setProjectionMatrixAsPerspective(7.5, ratio, near1, far1);
//
//		//创建渲染线程并开始渲染
//		m2DThreadHandle = new CRenderingThread(mOSG2D, true);
//		m2DThreadHandle->start();
//	}
//	return 0;
//}