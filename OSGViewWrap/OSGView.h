

#include "OSGViewInner.h"

using namespace System;
using namespace System::Runtime::InteropServices;
namespace OSGCLR 
{
	public ref class OSGViewWrap
	{
	public:
		OSGViewWrap(System::IntPtr viewHandle);
		~OSGViewWrap();

		void Render();
	private:
		OSGViewInner* mpView;
	};
}
