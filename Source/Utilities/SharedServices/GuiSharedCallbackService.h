/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDCALLBACKSERVICE
#define VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDCALLBACKSERVICE

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		/// <summary>
		/// A general <see cref="INativeCallbackService/> implementation.
		/// </summary>
		class SharedCallbackService
			: public Object
			, public INativeCallbackService
			, public INativeCallbackInvoker
		{
		protected:
			collections::List<INativeControllerListener*>	listeners;

		public:
			SharedCallbackService();
			~SharedCallbackService();

			bool											InstallListener(INativeControllerListener* listener) override;
			bool											UninstallListener(INativeControllerListener* listener) override;
			INativeCallbackInvoker*							Invoker() override;

			void											InvokeGlobalTimer() override;
			void											InvokeClipboardUpdated() override;
			void											InvokeGlobalShortcutKeyActivated(vint id) override;
			void											InvokeNativeWindowCreated(INativeWindow* window) override;
			void											InvokeNativeWindowDestroying(INativeWindow* window) override;
		};
	}
}

#endif