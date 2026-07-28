#if defined __linux__ && __has_include(<GacUI.h>) && __has_include("../WGac/Services/WGacAutomationService.h")
#include <GacUI.h>
#elif defined __APPLE__ && __has_include(<GacUI.h>)
#include <GacUI.h>
#else
#include "../../../Source/GacUI.h"
#endif
#include <VlppOS.h>

using namespace vl;
using namespace vl::inter_process;
using namespace vl::inter_process::async_tcp_socket;
using namespace vl::presentation;

namespace
{
	class MiniHttpAutomationService : public SocketHttpServerApi
	{
	private:
		void DumpTreeAsync(
			Ptr<SocketHttpRequestContext> context,
			INativeWindow* mainWindow,
			INativeAsyncService* asyncService,
			INativeAutomationService* automationService,
			bool dumpControlTree
			)
		{
			asyncService->InvokeInMainThread(mainWindow, [context, automationService, dumpControlTree]()
			{
				try
				{
					auto response = dumpControlTree
						? automationService->DumpControlTree()
						: automationService->DumpDomTree();
					context->RespondUtf8(
						200,
						WString::Unmanaged(L"OK"),
						WString::Unmanaged(HttpNetworkProtocolContentType),
						response
						);
					return;
				}
				catch (const Error&)
				{
				}
				catch (const Exception&)
				{
				}
				context->RespondStatus(404, WString::Unmanaged(L"Not Found"));
			});
		}

		bool TryGetBodyUtf8(Ptr<SocketHttpRequestContext> context, WString& body)
		{
			auto request = context->GetRequest();
			auto contentType = FindHttpField(request->headers, WString::Unmanaged(L"content-type"));
			if (
				CountHttpFields(request->headers, WString::Unmanaged(L"content-type")) != 1 ||
				!contentType ||
				!HttpFieldValueEqualsAscii(contentType->value, WString::Unmanaged(HttpNetworkProtocolContentType))
				)
			{
				return false;
			}
			return context->TryGetBodyUtf8(body) && body.Length() > 0;
		}

	protected:
		void OnHttpRequestReceived(Ptr<SocketHttpRequestContext> context) override
		{
			auto mainWindow = GetCurrentController()->WindowService()->GetMainWindow();
			auto asyncService = GetCurrentController()->AsyncService();
			auto automationService = GetCurrentController()->AutomationService();

			try
			{
				Nullable<WString> respondString;
				auto request = context->GetRequest();
				auto relativePath = context->GetRelativePath();
				if (request->method == L"GET")
				{
					if (relativePath == L"/Controls")
					{
						if (automationService->CanDumpControlTree())
						{
							DumpTreeAsync(context, mainWindow, asyncService, automationService, true);
							return;
						}
					}
					else if (relativePath == L"/Dom")
					{
						if (automationService->CanDumpDomTree())
						{
							DumpTreeAsync(context, mainWindow, asyncService, automationService, false);
							return;
						}
					}
				}
				else if (request->method == L"POST")
				{
					Nullable<WString> windowId;
					if (relativePath == L"/IO")
					{
					}
					else if (relativePath.Length() >= 4 && relativePath.Left(4) == L"/IO/")
					{
						windowId = relativePath.Right(relativePath.Length() - 4);
					}
					else
					{
						context->RespondStatus(404, WString::Unmanaged(L"Not Found"));
						return;
					}

					if (automationService->CanRunIOCommands() != INativeAutomationService::IOCommandAvailability::Disabled)
					{
						WString body;
						if (TryGetBodyUtf8(context, body))
						{
							respondString = automationService->RunIOCommand(windowId, body);
						}
					}
				}

				if (respondString)
				{
					context->RespondUtf8(
						200,
						WString::Unmanaged(L"OK"),
						WString::Unmanaged(HttpNetworkProtocolContentType),
						respondString.Value()
						);
					return;
				}
			}
			catch (const Error&)
			{
			}
			catch (const Exception&)
			{
			}
			context->RespondStatus(404, WString::Unmanaged(L"Not Found"));
		}

	public:
		MiniHttpAutomationService(Ptr<IAsyncSocketServer> socketServer, const WString& urlPrefix)
			: SocketHttpServerApi(socketServer, urlPrefix)
		{
		}

		~MiniHttpAutomationService()
		{
			Stop();
		}
	};

	MiniHttpAutomationService* miniHttpAutomationService = nullptr;
}

void StartMiniHttpAutomationService(Ptr<IAsyncSocketServer> socketServer, const WString& applicationName)
{
	auto automationService = GetCurrentController()->AutomationService();
	if (!automationService->Available())
	{
		return;
	}
	if (!miniHttpAutomationService)
	{
		bool canDumpControlTree = automationService->CanDumpControlTree();
		bool canDumpDomTree = automationService->CanDumpDomTree();
		CHECK_ERROR(
			canDumpControlTree != canDumpDomTree,
			L"StartMiniHttpAutomationService(Ptr<IAsyncSocketServer>, const WString&)#The automation service should provide either the control tree or the DOM tree."
			);

		auto urlPrefix = WString::Unmanaged(L"/Automation/") + applicationName;
		auto service = new MiniHttpAutomationService(socketServer, urlPrefix);
		try
		{
			service->Start();
		}
		catch (...)
		{
			delete service;
			throw;
		}
		miniHttpAutomationService = service;
	}
}

void StartMiniHttpAutomationService(Ptr<IAsyncSocketServer> socketServer)
{
	auto automationService = GetCurrentController()->AutomationService();
	StartMiniHttpAutomationService(
		socketServer,
		WString::Unmanaged(
			automationService->CanDumpControlTree()
			? L"RemotingTest_Core"
			: L"RemotingTest_Rendering_Win32"
			)
		);
}

void StopMiniHttpAutomationService()
{
	if (miniHttpAutomationService)
	{
		miniHttpAutomationService->Stop();
		delete miniHttpAutomationService;
		miniHttpAutomationService = nullptr;
	}
}
