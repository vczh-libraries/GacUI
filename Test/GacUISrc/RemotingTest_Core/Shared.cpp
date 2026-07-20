#include "../../../Source/GacUI.h"
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
							asyncService->InvokeInMainThreadAndWait(mainWindow, [&]()
							{
								respondString = automationService->DumpControlTree();
							});
						}
					}
					else if (relativePath == L"/Dom")
					{
						if (automationService->CanDumpDomTree())
						{
							asyncService->InvokeInMainThreadAndWait(mainWindow, [&]()
							{
								respondString = automationService->DumpDomTree();
							});
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

void StartMiniHttpAutomationService(Ptr<IAsyncSocketServer> socketServer)
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
			L"StartMiniHttpAutomationService(Ptr<IAsyncSocketServer>)#The automation service should provide either the control tree or the DOM tree."
			);

		auto urlPrefix = WString::Unmanaged(
			canDumpControlTree
			? L"/Automation/RemotingTest_Core"
			: L"/Automation/RemotingTest_Rendering_Win32"
			);
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

void StopMiniHttpAutomationService()
{
	if (miniHttpAutomationService)
	{
		miniHttpAutomationService->Stop();
		delete miniHttpAutomationService;
		miniHttpAutomationService = nullptr;
	}
}
