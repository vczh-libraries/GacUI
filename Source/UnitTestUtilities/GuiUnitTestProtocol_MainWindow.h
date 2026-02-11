/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_MAINWINDOW
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_MAINWINDOW

#include "GuiUnitTestProtocol_Shared.h"

namespace vl::presentation::unittest
{
	class UnitTestRemoteProtocol_MainWindow : public virtual UnitTestRemoteProtocolBase
	{
		using WindowSizingConfig = remoteprotocol::WindowSizingConfig;
		using WindowShowing = remoteprotocol::WindowShowing;
	public:
		WindowSizingConfig			sizingConfig;
		WindowStyleConfig			styleConfig;
		NativeRect					lastRestoredSize;
	
		UnitTestRemoteProtocol_MainWindow(const UnitTestScreenConfig& _globalConfig)
			: UnitTestRemoteProtocolBase(_globalConfig)
		{
			sizingConfig.bounds = { 0,0,0,0 };
			sizingConfig.clientBounds = { 0,0,0,0 };
			sizingConfig.customFramePadding = this->GetGlobalConfig().customFramePadding;
			sizingConfig.sizeState = INativeWindow::Restored;
		}

	protected:

/***********************************************************************
IGuiRemoteProtocolMessages (Controller)
***********************************************************************/
	
		void Impl_ControllerGetFontConfig(vint id)
		{
			this->GetEvents()->RespondControllerGetFontConfig(id, this->GetGlobalConfig().fontConfig);
		}
	
		void Impl_ControllerGetScreenConfig(vint id)
		{
			this->GetEvents()->RespondControllerGetScreenConfig(id, this->GetGlobalConfig().screenConfig);
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Window)
***********************************************************************/
	
		void Impl_WindowGetBounds(vint id)
		{
			this->GetEvents()->RespondWindowGetBounds(id, sizingConfig);
		}
	
		void Impl_WindowNotifySetTitle(const ::vl::WString& arguments)
		{
			styleConfig.title = arguments;
		}
	
		void Impl_WindowNotifySetEnabled(const bool& arguments)
		{
			styleConfig.enabled = arguments;
		}
	
		void Impl_WindowNotifySetTopMost(const bool& arguments)
		{
			styleConfig.topMost = arguments;
		}
	
		void Impl_WindowNotifySetShowInTaskBar(const bool& arguments)
		{
			styleConfig.showInTaskBar = arguments;
		}
	
		void OnBoundsUpdated()
		{
			sizingConfig.clientBounds = sizingConfig.bounds;
			if (sizingConfig.sizeState == INativeWindow::Restored)
			{
				lastRestoredSize = sizingConfig.bounds;
			}
			this->GetEvents()->OnWindowBoundsUpdated(sizingConfig);
		}
	
		void Impl_WindowNotifySetBounds(const NativeRect& arguments)
		{
			sizingConfig.bounds = arguments;
			OnBoundsUpdated();
		}
	
		void Impl_WindowNotifySetClientSize(const NativeSize& arguments)
		{
			sizingConfig.bounds = { sizingConfig.bounds.LeftTop(), arguments };
			OnBoundsUpdated();
		}
	
		void Impl_WindowNotifySetCustomFrameMode(const bool& arguments)	{ styleConfig.customFrameMode = arguments;	this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void Impl_WindowNotifySetMaximizedBox(const bool& arguments)	{ styleConfig.maximizedBox = arguments;		this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void Impl_WindowNotifySetMinimizedBox(const bool& arguments)	{ styleConfig.minimizedBox = arguments;		this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void Impl_WindowNotifySetBorder(const bool& arguments)			{ styleConfig.border = arguments;			this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void Impl_WindowNotifySetSizeBox(const bool& arguments)			{ styleConfig.sizeBox = arguments;			this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void Impl_WindowNotifySetIconVisible(const bool& arguments)		{ styleConfig.iconVisible = arguments;		this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void Impl_WindowNotifySetTitleBar(const bool& arguments)		{ styleConfig.titleBar = arguments;			this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void Impl_WindowNotifyActivate()								{ styleConfig.activated = true; }
		void Impl_WindowNotifyMinSize(const NativeSize& arguments)		{}
		void Impl_WindowNotifySetCaret(const NativePoint& arguments)	{}
	
		void Impl_WindowNotifyShow(const WindowShowing& arguments)
		{
			styleConfig.activated = arguments.activate;
			if (sizingConfig.sizeState != arguments.sizeState)
			{
				sizingConfig.sizeState = arguments.sizeState;
				switch (arguments.sizeState)
				{
				case INativeWindow::Maximized:
					sizingConfig.bounds = this->GetGlobalConfig().screenConfig.clientBounds;
					OnBoundsUpdated();
					break;
				case INativeWindow::Minimized:
					sizingConfig.bounds = NativeRect(
						{
							this->GetGlobalConfig().screenConfig.bounds.x2,
							this->GetGlobalConfig().screenConfig.bounds.y2
						},
						{ 1,1 }
					);
					OnBoundsUpdated();
					break;
				case INativeWindow::Restored:
					if (sizingConfig.bounds != lastRestoredSize)
					{
						sizingConfig.bounds = lastRestoredSize;
						OnBoundsUpdated();
					}
					else
					{
						this->GetEvents()->OnWindowBoundsUpdated(sizingConfig);
					}
					break;
				}
			}
		}
	};
}

#endif