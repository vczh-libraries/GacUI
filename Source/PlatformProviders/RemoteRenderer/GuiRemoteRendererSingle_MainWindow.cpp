#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	void GuiRemoteRendererSingle::RequestWindowGetBounds(vint id)
	{
		CHECK_FAIL(L"Not Implemented");
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetTitle(const WString& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetEnabled(const bool& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetTopMost(const bool& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetShowInTaskBar(const bool& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetBounds(const NativeRect& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetClientSize(const NativeSize& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetCustomFrameMode(const bool& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetMaximizedBox(const bool& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetMinimizedBox(const bool& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetBorder(const bool& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetSizeBox(const bool& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetIconVisible(const bool& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifySetTitleBar(const bool& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifyActivate()
	{
	}

	void GuiRemoteRendererSingle::RequestWindowNotifyShow(const remoteprotocol::WindowShowing& arguments)
	{
	}
}