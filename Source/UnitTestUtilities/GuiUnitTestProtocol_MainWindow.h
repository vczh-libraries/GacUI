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
	template<typename TProtocol>
	class UnitTestRemoteProtocol_MainWindow : public TProtocol
	{
		using WindowSizingConfig = remoteprotocol::WindowSizingConfig;
		using WindowShowing = remoteprotocol::WindowShowing;
	public:
		WindowSizingConfig			sizingConfig;
		WindowStyleConfig			styleConfig;
		NativeRect					lastRestoredSize;
	
		template<typename ...TArgs>
		UnitTestRemoteProtocol_MainWindow(TArgs&& ...args)
			: TProtocol(std::forward<TArgs&&>(args)...)
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
	
		void RequestControllerGetFontConfig(vint id) override
		{
			this->GetEvents()->RespondControllerGetFontConfig(id, this->GetGlobalConfig().fontConfig);
		}
	
		void RequestControllerGetScreenConfig(vint id) override
		{
			this->GetEvents()->RespondControllerGetScreenConfig(id, this->GetGlobalConfig().screenConfig);
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Window)
***********************************************************************/
	
		void RequestWindowGetBounds(vint id) override
		{
			this->GetEvents()->RespondWindowGetBounds(id, sizingConfig);
		}
	
		void RequestWindowNotifySetTitle(const ::vl::WString& arguments) override
		{
			styleConfig.title = arguments;
		}
	
		void RequestWindowNotifySetEnabled(const bool& arguments) override
		{
			styleConfig.enabled = arguments;
		}
	
		void RequestWindowNotifySetTopMost(const bool& arguments) override
		{
			styleConfig.topMost = arguments;
		}
	
		void RequestWindowNotifySetShowInTaskBar(const bool& arguments) override
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
	
		void RequestWindowNotifySetBounds(const NativeRect& arguments) override
		{
			sizingConfig.bounds = arguments;
			OnBoundsUpdated();
		}
	
		void RequestWindowNotifySetClientSize(const NativeSize& arguments) override
		{
			sizingConfig.bounds = { sizingConfig.bounds.LeftTop(), arguments };
			OnBoundsUpdated();
		}
	
		void RequestWindowNotifySetCustomFrameMode(const bool& arguments) override	{ styleConfig.customFrameMode = arguments;	this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetMaximizedBox(const bool& arguments) override		{ styleConfig.maximizedBox = arguments;		this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetMinimizedBox(const bool& arguments) override		{ styleConfig.minimizedBox = arguments;		this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetBorder(const bool& arguments) override			{ styleConfig.border = arguments;			this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetSizeBox(const bool& arguments) override			{ styleConfig.sizeBox = arguments;			this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetIconVisible(const bool& arguments) override		{ styleConfig.iconVisible = arguments;		this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetTitleBar(const bool& arguments) override			{ styleConfig.titleBar = arguments;			this->GetEvents()->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifyActivate() override									{ styleConfig.activated = true; }
	
		void RequestWindowNotifyShow(const WindowShowing& arguments) override
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