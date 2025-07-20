#include "DarkSkin.h"
#include "MainWindow.h"
#include "CoreChannel.h"

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::templates;
using namespace vl::presentation::controls;

CoreChannel* coreChannel = nullptr;

void WriteErrorToRendererChannel(const WString& message)
{
	coreChannel->WriteErrorThreadUnsafe(message);
}

void GuiMain()
{
	theme::RegisterTheme(Ptr(new darkskin::Theme));
	{
		demo::MainWindow window;
		window.ForceCalculateSizeImmediately();
		window.MoveToScreenCenter();
		try
		{
			GetApplication()->Run(&window);
		}
		catch (const Exception& e)
		{
			WriteErrorToRendererChannel(e.Message());
		}
		catch (const Error& e)
		{
			WriteErrorToRendererChannel(WString::Unmanaged(e.Description()));
		}
	}
}