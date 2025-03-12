#include "DarkSkin.h"
#include "MainWindow.h"

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::templates;
using namespace vl::presentation::controls;

void GuiMain()
{
	theme::RegisterTheme(Ptr(new darkskin::Theme));
	{
		demo::MainWindow window;
		window.ForceCalculateSizeImmediately();
		window.MoveToScreenCenter();
		GetApplication()->Run(&window);
	}
}