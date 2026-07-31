#include "MiniHttpAutomationService.h"

namespace vl::presentation::remoting
{
	using namespace inter_process::async_tcp_socket;

	namespace
	{
		constexpr const wchar_t* HttpNetworkProtocolContentType = L"application/json; charset=utf8";

		bool IsValidWindowId(const WString& windowId)
		{
			if (windowId.Length() == 0)
			{
				return false;
			}
			for (vint i = 0; i < windowId.Length(); i++)
			{
				if (windowId[i] < L'0' || windowId[i] > L'9')
				{
					return false;
				}
			}
			return true;
		}

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
					}
					catch (...)
					{
						std::terminate();
					}
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

			void ProcessHttpRequest(Ptr<SocketHttpRequestContext> context)
			{
				auto mainWindow = GetCurrentController()->WindowService()->GetMainWindow();
				auto asyncService = GetCurrentController()->AsyncService();
				auto automationService = GetCurrentController()->AutomationService();
				auto request = context->GetRequest();
				auto relativePath = context->GetRelativePath();
				if (request->method == L"GET")
				{
					if (relativePath == L"/Controls" && automationService->CanDumpControlTree())
					{
						DumpTreeAsync(context, mainWindow, asyncService, automationService, true);
						return;
					}
					if (relativePath == L"/Dom" && automationService->CanDumpDomTree())
					{
						DumpTreeAsync(context, mainWindow, asyncService, automationService, false);
						return;
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
						auto id = relativePath.Right(relativePath.Length() - 4);
						if (!IsValidWindowId(id))
						{
							context->RespondStatus(404, WString::Unmanaged(L"Not Found"));
							return;
						}
						windowId = id;
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
							context->RespondUtf8(
								200,
								WString::Unmanaged(L"OK"),
								WString::Unmanaged(HttpNetworkProtocolContentType),
								automationService->RunIOCommand(windowId, body)
								);
							return;
						}
					}
				}
				context->RespondStatus(404, WString::Unmanaged(L"Not Found"));
			}

		protected:
			void OnHttpRequestReceived(Ptr<SocketHttpRequestContext> context) override
			{
				try
				{
					ProcessHttpRequest(context);
				}
				catch (...)
				{
					std::terminate();
				}
			}

		public:
			MiniHttpAutomationService(Ptr<IAsyncSocketServer> socketServer, const WString& urlPrefix)
				: SocketHttpServerApi(socketServer, urlPrefix)
			{
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
		CHECK_ERROR(!miniHttpAutomationService, L"vl::presentation::remoting::StartMiniHttpAutomationService(...)#The MiniHTTP automation service has already been started.");

		auto canDumpControlTree = automationService->CanDumpControlTree();
		auto canDumpDomTree = automationService->CanDumpDomTree();
		CHECK_ERROR(
			canDumpControlTree != canDumpDomTree,
			L"vl::presentation::remoting::StartMiniHttpAutomationService(...)#The automation service should provide either the control tree or the DOM tree."
			);

		auto urlPrefix = WString::Unmanaged(L"/Automation/") + applicationName;
		auto service = new MiniHttpAutomationService(socketServer, urlPrefix);
		service->Start();
		miniHttpAutomationService = service;
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
		CHECK_ERROR(miniHttpAutomationService, L"vl::presentation::remoting::StopMiniHttpAutomationService()#The MiniHTTP automation service has not been started.");
		miniHttpAutomationService->Stop();
		delete miniHttpAutomationService;
		miniHttpAutomationService = nullptr;
	}

	MiniHttpAutomationServiceScope::MiniHttpAutomationServiceScope(
		Ptr<IAsyncSocketServer> socketServer,
		const WString& applicationName
		)
	{
		StartMiniHttpAutomationService(socketServer, applicationName);
	}

	MiniHttpAutomationServiceScope::~MiniHttpAutomationServiceScope()
	{
		StopMiniHttpAutomationService();
	}
}
