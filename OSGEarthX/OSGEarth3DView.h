#ifndef _OSGEARTH3DVIEW_H_
#define _OSGEARTH3DVIEW_H_


#ifndef _OSGVIEW_EXPORTS
#define OSGVIEW  __declspec(dllimport) 
#pragma comment(lib, "OSGEarthX.lib")	
#else
#define OSGVIEW  __declspec(dllexport)
#endif	

#include <windows.h>
#include <vector>
using namespace std;


class OSGVIEW CDOT
{
public:
	CDOT()
	{
		x = y = z = w = 0;
	}
	double	x;
	double	y;
	double	z;
	double	w;
};
class OSGViewBase;
class OSGVIEW OSGView
{
public:
	OSGView(HWND hWnd);
	~OSGView();

	void	StartRender();
	double	metersToLongitudinalDegrees(double value, double lat_deg);

	int		DrawPlane(CDOT& centerPos,
		CDOT*centerDots,
		CDOT* colors, 
		int	xNum,
		int	yNum,
		double	  xWidth, double yWidth, double zWidth);

	int		DrawSurface(CDOT& centerPos, CDOT* centerDots,
		double* pHeight, int	xNum,
		int	yNum, double	  xWidth, double yWidth, double zWidth);


	bool		RemovePlane(int* id,int num);
	void		TestDraw(CDOT* pDots);

	void	Jump(double lon, double lat, double z, double heading, double pitch, double range);

	bool	GetAtt(double& att);

private:
	OSGViewBase* mpViewBaseObject;
	void* mpThread;
};





#endif