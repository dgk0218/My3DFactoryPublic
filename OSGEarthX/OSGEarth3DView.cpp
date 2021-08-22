
#include "pch.h"
#include "OSGEarth3DView.h"

#include "OSGEarthView.h"
#include "OSGPlaneView.h"


OSGView::OSGView(HWND hWnd)
{
	mpViewBaseObject = new OSGEarthView(hWnd);
	if (mpViewBaseObject)
	{
		//mpViewObject->InitOSG("openstreetmap.earth");
	}
}
OSGView::~OSGView()
{

}
void OSGView::StartRender()
{
	OSGPlaneView* pPlaneView = dynamic_cast<OSGPlaneView*>(mpViewBaseObject);
	if ( pPlaneView )
	{
		mpViewBaseObject->InitOSG("cow.osg");
	}
	else
	{
		mpViewBaseObject->InitOSG("Map.earth");
	}
	mpThread = new CRenderingThread( mpViewBaseObject );
	((CRenderingThread*)mpThread)->start();
}

double OSGView::metersToLongitudinalDegrees(double value, double lat_deg)
{
	if (mpViewBaseObject)
	{
		return mpViewBaseObject->metersToLongitudinalDegrees(value, lat_deg);
	}
	return 0;
}

int OSGView::DrawPlane(CDOT& centerPos,
	CDOT* pcenterDots,
	CDOT* pcolors,
	int	xNum,
	int	yNum,
	double xWidth, double yWidth, double zWidth)
{
	if (mpViewBaseObject)
	{

		std::vector< std::vector<CDOT> >		centerDots;
		std::vector <vector< CDOT >>			colors;
		if (pcenterDots)
		{
			int		index = 0;
			for (int y =0;y < yNum;++y)
			{
				vector<CDOT>		linex;
				for (int x =0;x < xNum;++x)
				{
					linex.push_back( pcenterDots[index++] );
				}
				centerDots.push_back(linex);
			}
		}
		if (pcolors)
		{
			int		index = 0;
			for (int y = 0; y < yNum; ++y)
			{
				vector<CDOT>		linex;
				for (int x = 0; x < xNum; ++x)
				{
					linex.push_back(pcolors[index++]);
				}
				colors.push_back(linex);
			}
		}
		return mpViewBaseObject->DrawPlane(centerPos, centerDots, colors, xWidth, yWidth, zWidth);
	}
	return -1;
}

int OSGView::DrawSurface(CDOT& centerPos, CDOT* pcenterDots,
	double* pHeight, int	xNum,
	int	yNum,
	double xWidth, double yWidth, double zWidth)
{
	if (mpViewBaseObject)
	{

		std::vector< std::vector<CDOT> >		centerDots;
		std::vector <vector< double >>		heights;
		if (pcenterDots)
		{
			int		index = 0;
			for (int y = 0; y < yNum; ++y)
			{
				vector<CDOT>		linex;
				for (int x = 0; x < xNum; ++x)
				{
					linex.push_back(pcenterDots[index++]);
				}
				centerDots.push_back(linex);
			}
		}
		if (pHeight)
		{
			int		index = 0;
			for (int y = 0; y < yNum; ++y)
			{
				vector<double>		linex;
				for (int x = 0; x < xNum; ++x)
				{
					linex.push_back(pHeight[index++]);
				}
				heights.push_back(linex);
			}
		}
		return mpViewBaseObject->DrawSurface(centerPos, centerDots, heights, xWidth, yWidth, zWidth);
	}
	return -1;
}
bool OSGView::RemovePlane(int* id, int num)
{
	if (mpThread)
	{
		CRenderingThread* pThread = (CRenderingThread*)mpThread;
		if (pThread)
		{
			pThread->RemoveID(id,num);
		}
	}
	return false;
}

void OSGView::TestDraw(CDOT* pDots)
{
	int count = 0; //pDots.size();
}

void OSGView::Jump(double lon, double lat, double z, double heading, double pitch, double range)
{
	if (mpViewBaseObject)
	{
		mpViewBaseObject->Jump(lon, lat, z, heading, pitch, range);
	}
}

bool OSGView::GetAtt(double& att)
{
	if (mpViewBaseObject)
	{
		return	mpViewBaseObject->GetAtt(att);
	}
	return false;
}

