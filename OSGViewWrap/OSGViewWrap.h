#pragma once

#include<OSGEarth3DView.h>
using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
//using namespace System::Windows::Forms;
using namespace System::Data;
//using namespace System::Drawing;
using namespace System::Runtime::CompilerServices;
using namespace System::ComponentModel::Design;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;

namespace OSGCLR
{
	public ref class  DOT
	{
	public:
		DOT()
		{
			x = y = z = w = 0;
		}
		double	x;
		double	y;
		double	z;
		double	w;
	};
	public ref class OSGViewWrap
	{
	public:
		OSGViewWrap(System::IntPtr viewHandle);
		~OSGViewWrap();
		//开始绘制
		void StartRender();	
		//距离 转经度间隔			lat_deg = 纬度值		 value = 距离 米单位			返回角度值
		double	metersToLongitudinalDegrees(double value, double lat_deg);
		//指定点的中小绘制曲面
		UInt32	DrawPlane(DOT^ startDot,
			List< List<DOT^>^ > ^  centerDots,
			List< List<DOT^>^ >^ colors , 
			double xWidth ,double yWidth,double zWidth);

		UInt32	DrawSurface(DOT^ startDot,
			List< List<DOT^>^ >^ centerDots,
			List< List<double>^ >^ heights,
			double xWidth, double yWidth, double zWidth);

		bool		RemovePlane(List<Int32>^   drawids);


		void Jump(double lon, double lat, double z, double heading, double pitch, double range);

		bool	GetAtt(double% att);

	private:
		OSGView* mpView;
	};
}
