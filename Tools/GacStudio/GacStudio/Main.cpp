
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "UI\_Source\GacStudioUI.h"
#include "Studio\StudioModel.h"
#include <Windows.h>

using namespace vl::collections;
using namespace vl::stream;
using namespace ui;
using namespace vm;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	int result =  SetupWindowsDirect2DRenderer();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

#define GACSTUDIO_LOADBINARYRESOURCE

void GuiMain()
{
	List<WString> errors;
#ifdef GACSTUDIO_LOADBINARYRESOURCE
	Ptr<GuiResource> resource;
	{
		FileStream fileStream(L"..\\GacStudio\\UI\\_UIRes\\Resources.bin", FileStream::ReadOnly);
		LzwDecoder decoder;
		DecoderStream decoderStream(fileStream, decoder);
		resource = GuiResource::LoadPrecompiledBinary(decoderStream, errors);
	}
#else
	auto resource = GuiResource::LoadFromXml(L"..\\GacStudio\\UI\\_UIRes\\Resources.xml", errors);
#endif
	GetInstanceLoaderManager()->SetResource(L"GacStudioUI", resource);
	MainWindow window(new StudioModel);
	window.ForceCalculateSizeImmediately();
	window.MoveToScreenCenter();
	GetApplication()->Run(&window);
}