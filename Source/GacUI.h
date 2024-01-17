/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Header Files and Common Namespaces

Global Objects:
	vl::reflection::description::					GetGlobalTypeManager
	vl::presentation::								GetParserManager
	vl::presentation::								GetResourceResolverManager
	vl::presentation::								GetCurrentController
	vl::presentation::								GetInstanceLoaderManager
	vl::presentation::elements::					GetGuiGraphicsResourceManager
	vl::presentation::controls::					GetApplication
	vl::presentation::controls::					GetPluginManager
	vl::presentation::theme::						GetCurrentTheme

	vl::presentation::windows::						GetDirect2DFactory
	vl::presentation::windows::						GetDirectWriteFactory
	vl::presentation::elements_windows_gdi::		GetWindowsGDIResourceManager
	vl::presentation::elements_windows_gdi::		GetWindowsGDIObjectProvider
	vl::presentation::elements_windows_d2d::		GetWindowsDirect2DResourceManager
	vl::presentation::elements_windows_d2d::		GetWindowsDirect2DObjectProvider
***********************************************************************/

#ifndef VCZH_PRESENTATION_GACUI
#define VCZH_PRESENTATION_GACUI

#include "GacUIReflectionHelper.h"

#ifdef GAC_HEADER_USE_NAMESPACE

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::theme;
using namespace vl::presentation::templates;

#endif

// GacUI Compiler
extern int SetupGacGenNativeController();

// Remote
namespace vl::presentation
{
	class IGuiRemoteProtocol;
}
extern int SetupRemoteNativeController(vl::presentation::IGuiRemoteProtocol* protocol);

// Windows
extern int SetupWindowsGDIRenderer();
extern int SetupWindowsDirect2DRenderer();
extern int SetupHostedWindowsGDIRenderer();
extern int SetupHostedWindowsDirect2DRenderer();

// Gtk
extern int SetupGtkRenderer();

// macOS
extern int SetupOSXCoreGraphicsRenderer();

#endif