/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Header Files and Common Namespaces

Resource:
	vl::reflection::description::					GetGlobalTypeManager
	vl::presentation::								GetParserManager
	vl::presentation::controls::					GetPluginManager
	vl::presentation::								GetResourceResolverManager
	vl::presentation::								GetResourceManager

Platform:
	vl::presentation::								GetCurrentController
	vl::presentation::								GetNativeServiceSubstitution
	vl::presentation::elements::					GetGuiGraphicsResourceManager
	vl::presentation::IGuiHostedApplication::		GetHostedApplication

GacUI:
	vl::presentation::controls::					GetApplication
	vl::presentation::theme::						GetCurrentTheme
	vl::presentation::								GetInstanceLoaderManager
	vl::presentation::								Workflow_GetSharedManager

Windows:
	vl::presentation::windows::						GetD3D11Device
	vl::presentation::elements_windows_d2d::		GetWindowsDirect2DResourceManager
	vl::presentation::elements_windows_d2d::		GetWindowsDirect2DObjectProvider
	{
		vl::presentation::windows::					GetDirect2DFactory
		vl::presentation::windows::					GetDirectWriteFactory
	}
	vl::presentation::elements_windows_gdi::		GetWindowsGDIResourceManager
	vl::presentation::elements_windows_gdi::		GetWindowsGDIObjectProvider
	vl::presentation::windows::						GetWindowsNativeController

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
extern int SetupRawWindowsGDIRenderer();
extern int SetupRawWindowsDirect2DRenderer();

// Gtk
extern int SetupGtkRenderer();

// macOS
extern int SetupOSXCoreGraphicsRenderer();

#endif