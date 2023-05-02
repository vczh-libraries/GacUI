#include "GuiSharedCallbackService.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
SharedCallbackService
***********************************************************************/

		SharedCallbackService::SharedCallbackService()
		{
		}

		SharedCallbackService::~SharedCallbackService()
		{
		}

		bool SharedCallbackService::InstallListener(INativeControllerListener* listener)
		{
			if(listeners.Contains(listener))
			{
				return false;
			}
			else
			{
				listeners.Add(listener);
				return true;
			}
		}

		bool SharedCallbackService::UninstallListener(INativeControllerListener* listener)
		{
			if(listeners.Contains(listener))
			{
				listeners.Remove(listener);
				return true;
			}
			else
			{
				return false;
			}
		}

		INativeCallbackInvoker* SharedCallbackService::Invoker()
		{
			return this;
		}

		void SharedCallbackService::InvokeGlobalTimer()
		{
			// TODO: (enumerable) foreach
			for(vint i=0;i<listeners.Count();i++)
			{
				listeners[i]->GlobalTimer();
			}
		}

		void SharedCallbackService::InvokeClipboardUpdated()
		{
			// TODO: (enumerable) foreach
			for(vint i=0;i<listeners.Count();i++)
			{
				listeners[i]->ClipboardUpdated();
			}
		}

		void SharedCallbackService::InvokeGlobalShortcutKeyActivated(vint id)
		{
			// TODO: (enumerable) foreach
			for (vint i = 0; i < listeners.Count(); i++)
			{
				listeners[i]->GlobalShortcutKeyActivated(id);
			}
		}

		void SharedCallbackService::InvokeNativeWindowCreated(INativeWindow* window)
		{
			// TODO: (enumerable) foreach
			for(vint i=0;i<listeners.Count();i++)
			{
				listeners[i]->NativeWindowCreated(window);
			}
		}

		void SharedCallbackService::InvokeNativeWindowDestroying(INativeWindow* window)
		{
			// TODO: (enumerable) foreach
			for(vint i=0;i<listeners.Count();i++)
			{
				listeners[i]->NativeWindowDestroying(window);
			}
		}
	}
}