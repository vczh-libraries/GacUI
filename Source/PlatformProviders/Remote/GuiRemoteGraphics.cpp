#include "GuiRemoteController.h"
#include "GuiRemoteGraphics_BasicElements.h"

namespace vl::presentation::elements
{

/***********************************************************************
GuiRemoteGraphicsRenderTarget
***********************************************************************/

	void GuiRemoteGraphicsRenderTarget::StartRenderingOnNativeWindow()
	{
		canvasSize = remote->remoteWindow.GetClientSize();
	}

	RenderTargetFailure GuiRemoteGraphicsRenderTarget::StopRenderingOnNativeWindow()
	{
		if (canvasSize == remote->remoteWindow.GetClientSize())
		{
			return RenderTargetFailure::None;
		}
		else
		{
			return RenderTargetFailure::ResizeWhileRendering;
		}
	}

	Size GuiRemoteGraphicsRenderTarget::GetCanvasSize()
	{
		return remote->remoteWindow.Convert(canvasSize);
	}

	void GuiRemoteGraphicsRenderTarget::AfterPushedClipper(Rect clipper, Rect validArea)
	{
	}

	void GuiRemoteGraphicsRenderTarget::AfterPushedClipperAndBecameInvalid(Rect clipper)
	{
	}

	void GuiRemoteGraphicsRenderTarget::AfterPoppedClipperAndBecameValid(Rect validArea, bool clipperExists)
	{
	}

	void GuiRemoteGraphicsRenderTarget::AfterPoppedClipper(Rect validArea, bool clipperExists)
	{
	}

	GuiRemoteGraphicsRenderTarget::GuiRemoteGraphicsRenderTarget(GuiRemoteController* _remote)
		: remote(_remote)
	{
	}

	GuiRemoteGraphicsRenderTarget::~GuiRemoteGraphicsRenderTarget()
	{
	}

/***********************************************************************
GuiRemoteGraphicsResourceManager
***********************************************************************/

	GuiRemoteGraphicsResourceManager::GuiRemoteGraphicsResourceManager(GuiRemoteController* _remote)
		: remote(_remote)
		, renderTarget(_remote)
	{
	}

	GuiRemoteGraphicsResourceManager::~GuiRemoteGraphicsResourceManager()
	{
	}

	void GuiRemoteGraphicsResourceManager::Initialize()
	{
		elements_remoteprotocol::GuiFocusRectangleElementRenderer::Register();
		elements_remoteprotocol::GuiSolidBorderElementRenderer::Register();
		elements_remoteprotocol::Gui3DBorderElementRenderer::Register();
		elements_remoteprotocol::Gui3DSplitterElementRenderer::Register();
		elements_remoteprotocol::GuiSolidBackgroundElementRenderer::Register();
		elements_remoteprotocol::GuiGradientBackgroundElementRenderer::Register();
		elements_remoteprotocol::GuiInnerShadowElementRenderer::Register();
		elements_remoteprotocol::GuiSolidLabelElementRenderer::Register();
		elements_remoteprotocol::GuiImageFrameElementRenderer::Register();
		elements_remoteprotocol::GuiPolygonElementRenderer::Register();
		elements_remoteprotocol::GuiColorizedTextElementRenderer::Register();
		elements::GuiDocumentElement::GuiDocumentElementRenderer::Register();
	}

	void GuiRemoteGraphicsResourceManager::Finalize()
	{
	}

	IGuiGraphicsRenderTarget* GuiRemoteGraphicsResourceManager::GetRenderTarget(INativeWindow* window)
	{
		CHECK_ERROR(window == &remote->remoteWindow, L"vl::presentation::elements::GuiRemoteGraphicsResourceManager::GetRenderTarget(INativeWindow*)#GuiHostedController should call this function with the native window.");
		return &renderTarget;
	}

	void GuiRemoteGraphicsResourceManager::RecreateRenderTarget(INativeWindow* window)
	{
	}

	void GuiRemoteGraphicsResourceManager::ResizeRenderTarget(INativeWindow* window)
	{
	}

	IGuiGraphicsLayoutProvider* GuiRemoteGraphicsResourceManager::GetLayoutProvider()
	{
		CHECK_FAIL(L"Not Implemented!");
	}
}