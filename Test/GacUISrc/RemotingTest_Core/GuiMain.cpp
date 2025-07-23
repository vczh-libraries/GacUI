#include "DarkSkin.h"
#include "MainWindow.h"
#include "CoreChannel.h"
#include "Shared/NamedPipeShared.h"
#include "Shared/HttpServer.h"

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::templates;
using namespace vl::presentation::controls;

CoreChannel* coreChannel = nullptr;

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
			coreChannel->WriteErrorThreadUnsafe(e.Message());
		}
		catch (const Error& e)
		{
			coreChannel->WriteErrorThreadUnsafe(WString::Unmanaged(e.Description()));
		}
	}
}

template<typename TServer>
void StartServer(TServer& server)
{
	CoreChannel namedPipeCoreChannel(&server);

	auto jsonParser = Ptr(new glr::json::Parser);
	GuiRemoteJsonChannelStringSerializer channelJsonSerializer(&namedPipeCoreChannel, jsonParser);

	GuiRemoteProtocolAsyncJsonChannelSerializer asyncChannelSender;
	asyncChannelSender.Start(
		&channelJsonSerializer,
		[&namedPipeCoreChannel](GuiRemoteProtocolAsyncJsonChannelSerializer* channel)
		{
			GuiRemoteProtocolFromJsonChannel channelSender(channel);
			GuiRemoteProtocolFilter filteredProtocol(&channelSender);
			GuiRemoteProtocolDomDiffConverter diffConverterProtocol(&filteredProtocol);
			coreChannel = &namedPipeCoreChannel;
			SetupRemoteNativeController(&diffConverterProtocol);
			coreChannel = nullptr;
		},
		[&namedPipeCoreChannel](
			GuiRemoteProtocolAsyncJsonChannelSerializer::TChannelThreadProc channelThreadProc,
			GuiRemoteProtocolAsyncJsonChannelSerializer::TUIThreadProc uiThreadProc
			)
		{
			Thread::CreateAndStart(channelThreadProc);
			Thread::CreateAndStart(uiThreadProc);
		});

	Console::WriteLine(L"> Waiting for a renderer ...");
	server.WaitForClient();
	namedPipeCoreChannel.RendererConnectedThreadUnsafe(&asyncChannelSender);
	asyncChannelSender.WaitForStopped();
	server.Stop();
	namedPipeCoreChannel.WaitForDisconnected();
}

int StartNamedPipeServer()
{
	NamedPipeServer namedPipeServer;
	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(NamedPipeId));
	StartServer(namedPipeServer);
	return 0;
}

int StartHttpServer()
{
	HttpServer httpServer;
	Console::WriteLine(L"> HTTP server created, waiting on: " + WString::Unmanaged(HttpServerUrl));
	StartServer(httpServer);
	return 0;
}