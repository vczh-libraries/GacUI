#include "GuiUnitTestUtilities.h"

using namespace vl;

namespace vl::presentation::unittest
{
}

Func<void()> guiMainProxy;

void SetGuiMainProxy(const Func<void()>& proxy)
{
	guiMainProxy = proxy;
}

void GuiMain()
{
	guiMainProxy();
}