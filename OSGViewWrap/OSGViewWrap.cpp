#include "pch.h"
#include "OSGViewWrap.h"
OSGCLR::OSGViewWrap::OSGViewWrap(System::IntPtr viewHandle)
{
	HWND		hwnd = (HWND)(viewHandle.ToPointer());
	mpView = new OSGView(hwnd);


}
OSGCLR::OSGViewWrap::~OSGViewWrap()
{

}

void OSGCLR::OSGViewWrap::StartRender()
{
	if (mpView)
	{
		mpView->StartRender();
	}
}

double OSGCLR::OSGViewWrap::metersToLongitudinalDegrees(double value, double lat_deg)
{
	if (mpView)
	{
		return mpView->metersToLongitudinalDegrees(value, lat_deg);
	}
}

System::UInt32 OSGCLR::OSGViewWrap::DrawPlane(DOT^ startDot, List< List<DOT^>^ >^ centerDots, List< List<DOT^>^ >^ colors, double xWidth, double yWidth, double zWidth)
{
	if (centerDots == nullptr || colors == nullptr || startDot == nullptr )
	{
		return -1;
	}
	if (mpView)
	{
		CDOT centerPosTem;
		//
		std::vector<CDOT>		line;
		std::vector<CDOT>		lineColor;
		int xNum = 2;
		int yNum = 2;
		int countXLine = centerDots->Count;

		yNum = countXLine;
		for ( int y = 0;y < countXLine;++y )
		{
			xNum = centerDots[y]->Count;
			for ( int x =0 ; x < centerDots[y]->Count;++x )
			{
				CDOT		temdot;
				DOT^		dot = centerDots[y][x];
				DOT^ cv = colors[y][x];
				temdot.x = dot->x;
				temdot.y = dot->y;
				temdot.z = dot->z;
				temdot.w = dot->w;
				line.push_back(temdot);

				CDOT		fcolor;
				fcolor.x = cv->x;
				fcolor.y = cv->y;
				fcolor.z = cv->z;
				fcolor.w = cv->w;
				lineColor.push_back(fcolor);
			}
		}
		if (startDot != nullptr)
		{
			centerPosTem.x = startDot->x;
			centerPosTem.y = startDot->y;
			centerPosTem.z = startDot->z;
		}
		else
		{
			centerPosTem.x = line[0].x;
			centerPosTem.x = line[0].y;
			centerPosTem.x = line[0].z;
		}
		return mpView->DrawPlane(centerPosTem, &line[0], &lineColor[0], xNum, yNum, xWidth, yWidth, zWidth);
	}
	return -1;
}

System::UInt32 OSGCLR::OSGViewWrap::DrawSurface(DOT^ startDot, 
	List< List<DOT^>^ >^ centerDots, 
	List< List<double>^ >^ heights, double xWidth, double yWidth, double zWidth)
{
	if (centerDots == nullptr || heights == nullptr || startDot == nullptr)
	{
		return -1;
	}
	if (mpView)
	{
		CDOT centerPosTem;
		//
		std::vector<CDOT>		line;
		std::vector<double>		lineHeight;
		int xNum = 2;
		int yNum = 2;
		int countXLine = centerDots->Count;

		yNum = countXLine;
		for (int y = 0; y < countXLine; ++y)
		{
			xNum = centerDots[y]->Count;
			for (int x = 0; x < centerDots[y]->Count; ++x)
			{
				CDOT		temdot;
				DOT^ dot = centerDots[y][x];
				temdot.x = dot->x;
				temdot.y = dot->y;
				temdot.z = dot->z;
				temdot.w = dot->w;
				line.push_back(temdot);
				lineHeight.push_back( heights[y][x] );
			}
		}
		if (startDot != nullptr)
		{
			centerPosTem.x = startDot->x;
			centerPosTem.y = startDot->y;
			centerPosTem.z = startDot->z;
		}
		else
		{
			centerPosTem.x = line[0].x;
			centerPosTem.x = line[0].y;
			centerPosTem.x = line[0].z;
		}
		return mpView->DrawSurface(centerPosTem, &line[0], &lineHeight[0], xNum, yNum, xWidth, yWidth, zWidth);
	}
	return -1;
}

bool OSGCLR::OSGViewWrap::RemovePlane(List<Int32>^ drawids)
{
	if (mpView && drawids != nullptr)
	{
		vector<int>		ids;
		for (int k =0; k < drawids->Count;++k)
		{
			ids.push_back(drawids[k]);
		}
		if (ids.size() > 0)
		{
			return mpView->RemovePlane(&ids[0], ids.size());
		}
		return false;
	}
	return false;
}

void OSGCLR::OSGViewWrap::Jump(double lon, double lat, double z, double heading, double pitch, double range)
{
	if (mpView)
	{
		mpView->Jump(lon, lat, z, heading, pitch, range);
	}
}

bool OSGCLR::OSGViewWrap::GetAtt(double% att)
{
	if (mpView)
	{
		double		outAtt;
		bool			res =	mpView->GetAtt(outAtt);
		att = outAtt;
		return res;
	}
	return false;
}
