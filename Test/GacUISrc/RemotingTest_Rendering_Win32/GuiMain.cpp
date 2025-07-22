#include "../../../Source/GacUI.h"
#include "RendererChannel.h"

using namespace vl::presentation;

RendererChannel* rendererChannel = nullptr;
GuiRemoteRendererSingle* renderer = nullptr;

void GuiMain()
{
	auto mainWindow = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
	mainWindow->SetTitle(L"Connecting ...");
	{
		auto size = mainWindow->Convert(Size(320, 240));
		auto screen = GetCurrentController()->ScreenService()->GetScreen((vint)0);
		auto client = screen->GetClientBounds();
		auto x = client.Left() + (client.Width() - size.x) / 2;
		auto y = client.Top() + (client.Height() - size.y) / 2;
		mainWindow->SetBounds({ {x,y},size });
	}
	rendererChannel->RegisterMainWindow(mainWindow);
	renderer->RegisterMainWindow(mainWindow);
	GetCurrentController()->WindowService()->Run(mainWindow);
	renderer->UnregisterMainWindow();
	rendererChannel->UnregisterMainWindow();
}