#include "pch.h"

#include "OSGView.h"

//int OSGCLR::OSGCLRDLL::InitDLL(IntPtr _hwnd2, bool is3D)
//{
//	HWND _hwnd = (HWND)_hwnd2.ToPointer();
//	if (is3D)
//	{
//		//ͨ������ľ����ʼ��cOSG����
//		mOSG3D = new OSGWrapper(_hwnd);
//		mOSG3D->InitOSG("Earth3D.earth");
//
//		//������Ⱦ�̲߳���ʼ��Ⱦ
//		m3DThreadHandle = new CRenderingThread(mOSG3D, false);
//		m3DThreadHandle->start();
//	}
//	else
//	{
//		//ͨ������ľ����ʼ��cOSG����
//		mOSG2D = new OSGWrapper(_hwnd);
//		mOSG2D->InitOSG("Map2D.earth");
//		double fov, ratio, near1, far1;
//		mOSG2D->getViewer()->getCamera()->getProjectionMatrixAsPerspective(fov, ratio, near1, far1);
//		mOSG2D->getViewer()->getCamera()->setProjectionMatrixAsPerspective(7.5, ratio, near1, far1);
//
//		//������Ⱦ�̲߳���ʼ��Ⱦ
//		m2DThreadHandle = new CRenderingThread(mOSG2D, true);
//		m2DThreadHandle->start();
//	}
//	return 0;
//}