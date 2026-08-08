#if defined __linux__ && __has_include(<GacUI.h>) && __has_include("../WGac/Renderers/WGacRenderer.h")
#include <GacUI.h>
#include <Test.RemotingHelpers.h>
#include <Skins/DarkSkin/DarkSkin.h>
#include "../WGac/Renderers/WGacRenderer.h"
#elif defined __APPLE__ && __has_include(<GacUI.h>)
#include <GacUI.h>
#include <Test.RemotingHelpers.h>
#include <Skins/DarkSkin/DarkSkin.h>
#include "../Mac/NativeWindow/OSX/CoreGraphics/CoreGraphicsApp.h"
#else
#include "DarkSkin.h"
#include "../Generated_RemoteViewModelTest/RemoteViewModelTestInitialize.h"
#include "../../RemotingHelpers/Rvmt/ViewModelHostServer.h"
#include "../../../Source/Utilities/AutomationService/MiniHttpAutomationService.h"
#include "../../../Source/Utilities/AutomationService/Windows/WindowsAutomationService.Windows.h"
#include "resource.h"
#endif
#if defined __linux__ || defined __APPLE__
#include "RemoteViewModelTest.h"
#include "RemoteViewModelTestInitialize.h"
#else
#include "RemoteViewModelTestIncludes.h"
#endif
#include <VlppOS.h>
#if defined VCZH_MSVC
#include <VlppOS.Windows.h>
#endif

using namespace vl;
using namespace vl::inter_process;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoting;
using namespace vl::presentation::remote_view_model_test;

#if defined VCZH_MSVC
constexpr const wchar_t* RemotingNamedPipeName = L"GacUIRemoteProtocolNamedPipe";
#endif
constexpr const wchar_t* RemotingHttpBaseUrl = L"/GacUIRemoteProtocolHttp";
constexpr vint RemotingHttpPort = 8888;

struct RvmGuiContext
{
	Ptr<rvmt::IViewModel>							viewModel;
	Ptr<async_tcp_socket::IAsyncSocketServer>		miniHttpSocketServer;
};

RvmGuiContext* currentGuiContext = nullptr;

void GuiMain()
{
	CHECK_ERROR(currentGuiContext, L"GuiMain()#The RVM GUI context is null.");
	CHECK_ERROR(currentGuiContext->viewModel, L"GuiMain()#The rvmt::IViewModel proxy is null.");

	theme::RegisterTheme(Ptr(new darkskin::Theme));
	rvmt::MainWindow window(currentGuiContext->viewModel);
	window.ForceCalculateSizeImmediately();
	window.MoveToScreenCenter();
#if defined VCZH_MSVC
	windows::SetWindowDefaultIcon(MAINICON);
	windows::WindowsAutomationServiceHosted automationService;
	GetNativeServiceSubstitution()->Substitute(&automationService, false);
	if (currentGuiContext->miniHttpSocketServer)
	{
		StartMiniHttpAutomationService(
			currentGuiContext->miniHttpSocketServer,
			WString::Unmanaged(L"CppTest_Rvm")
			);
	}
	else
	{
		windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/CppTest_Rvm"), RemotingHttpPort);
	}
#else
	StartMiniHttpAutomationService(
		currentGuiContext->miniHttpSocketServer,
		WString::Unmanaged(L"CppTest_Rvm")
		);
#endif

	GetApplication()->Run(&window);
#if defined VCZH_MSVC
	if (currentGuiContext->miniHttpSocketServer)
	{
		StopMiniHttpAutomationService();
	}
	else
	{
		windows::StopWindowsHttpAutomationService();
	}
	automationService.Stop();
	GetNativeServiceSubstitution()->Unsubstitute(&automationService);
#else
	StopMiniHttpAutomationService();
#endif
}

template<typename TServerBase>
int StartServer(
	RemoteViewModelChannelServer<TServerBase>& server,
	Ptr<async_tcp_socket::IAsyncSocketServer> miniHttpSocketServer
	)
{
	server.Start();
	collections::List<WString> requiredServiceNames;
	requiredServiceNames.Add(L"rvmt::IViewModel");
	auto requesterClientId = server.Connect(requiredServiceNames);
	RemoteViewModelTestInitialize::InitializeRpc(server.GetDispatcher(), requesterClientId);
	auto viewModel = server.RequestService(L"rvmt::IViewModel").template Cast<rvmt::IViewModel>();
	auto secondViewModel = server.RequestService(L"rvmt::IViewModel").template Cast<rvmt::IViewModel>();
	CHECK_ERROR(viewModel->Translate(L"First") == L"Hello, First!", L"StartServer(...)#The first rvmt::IViewModel proxy returned an unexpected response.");
	CHECK_ERROR(secondViewModel->Translate(L"Second") == L"Hello, Second!", L"StartServer(...)#The second rvmt::IViewModel proxy returned an unexpected response.");
	RvmGuiContext context{ viewModel, miniHttpSocketServer };
	CHECK_ERROR(!currentGuiContext, L"StartServer(...)#The GUI context has already been bound.");
	currentGuiContext = &context;
#if defined VCZH_MSVC
	auto result = SetupHostedWindowsDirect2DRenderer();
#elif defined VCZH_GCC && !defined VCZH_APPLE
	auto result = elements::wgac::SetupWGacHostedRenderer();
#else
	auto result = SetupOSXHostedCoreGraphicsRenderer();
#endif
	currentGuiContext = nullptr;
	server.Stop();
	return result;
}

#if defined VCZH_MSVC
int StartNamedPipeServer()
{
	auto parser = Ptr(new glr::json::Parser);
	RemoteViewModelChannelServer<named_pipe::NamedPipeServer> server(
		parser,
		false,
		WString::Unmanaged(RemotingNamedPipeName)
		);
	return StartServer(server, nullptr);
}

int StartHttpServer()
{
	auto parser = Ptr(new glr::json::Parser);
	RemoteViewModelChannelServer<windows_http::HttpServer> server(
		parser,
		false,
		WString::Unmanaged(RemotingHttpBaseUrl),
		RemotingHttpPort
		);
	return StartServer(server, nullptr);
}
#endif

int StartMiniHttpServer()
{
	auto parser = Ptr(new glr::json::Parser);
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	RemoteViewModelChannelServer<async_tcp_socket::SocketHttpServer> server(
		parser,
		false,
		socketServer,
		WString::Unmanaged(RemotingHttpBaseUrl)
		);
	return StartServer(server, socketServer);
}
