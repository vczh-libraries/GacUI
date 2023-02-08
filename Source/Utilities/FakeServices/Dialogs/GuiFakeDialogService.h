/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_FAKESERVICES_FAKEDIALOGSERVICE
#define VCZH_PRESENTATION_UTILITIES_FAKESERVICES_FAKEDIALOGSERVICE

#include "../GuiFakeDialogServiceBase.h"

namespace vl
{
	namespace presentation
	{
		class FakeDialogService : public FakeDialogServiceBase
		{
		public:
			FakeDialogService();
			~FakeDialogService();
		};
	}
}

#endif