#include "GuiRemoteController.h"
#include "GuiRemoteGraphics_BasicElements.h"

namespace vl::presentation::elements
{
	using namespace collections;

/***********************************************************************
GuiRemoteGraphicsRenderTarget
***********************************************************************/

	void GuiRemoteGraphicsRenderTarget::StartRenderingOnNativeWindow()
	{
		canvasSize = remote->remoteWindow.GetClientSize();

		if (destroyedRenderers.Count() > 0)
		{
			auto ids = Ptr(new List<vint>);
			CopyFrom(*ids.Obj(), destroyedRenderers);
			destroyedRenderers.Clear();
			remote->remoteMessages.RequestRendererDestroyed(ids);
		}

		if (createdRenderers.Count() > 0)
		{
			auto ids = Ptr(new List<remoteprotocol::RendererCreation>);
			for (auto id : createdRenderers)
			{
				ids->Add({ id,renderers[id]->GetRendererType() });
			}
			createdRenderers.Clear();
			remote->remoteMessages.RequestRendererCreated(ids);
		}

		for (auto [id, renderer] : renderers)
		{
			if (renderer->IsUpdated())
			{
				renderer->SendUpdateElementMessages();
				renderer->ResetUpdated();
			}
		}

		vint idRendering = remote->remoteMessages.RequestRendererBeginRendering();
		remote->remoteMessages.Submit();
		auto measuring = remote->remoteMessages.RetrieveRendererBeginRendering(idRendering);
		CHECK_ERROR(!measuring.solidLabelMinSizes, L"Not Implemented!");
		remote->remoteMessages.ClearResponses();
	}

	RenderTargetFailure GuiRemoteGraphicsRenderTarget::StopRenderingOnNativeWindow()
	{
		vint idRendering = remote->remoteMessages.RequestRendererEndRendering();
		remote->remoteMessages.Submit();
		auto measuring = remote->remoteMessages.RetrieveRendererEndRendering(idRendering);
		CHECK_ERROR(!measuring.solidLabelMinSizes, L"Not Implemented!");
		remote->remoteMessages.ClearResponses();

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

	GuiRemoteMessages& GuiRemoteGraphicsRenderTarget::GetRemoteMessages()
	{
		return remote->remoteMessages;
	}

	vint GuiRemoteGraphicsRenderTarget::AllocateNewElementId()
	{
		return ++usedElementIds;
	}

	void GuiRemoteGraphicsRenderTarget::RegisterRenderer(elements_remoteprotocol::IGuiRemoteProtocolElementRender* renderer)
	{
		vint id = renderer->GetID();
		if (!createdRenderers.Contains(id))
		{
			renderers.Add(id, renderer);
			createdRenderers.Add(id);
		}
	}

	void GuiRemoteGraphicsRenderTarget::UnregisterRenderer(elements_remoteprotocol::IGuiRemoteProtocolElementRender* renderer)
	{
		vint id = renderer->GetID();
		renderers.Remove(id);
		vint index = createdRenderers.IndexOf(id);
		if (index == -1)
		{
			if (!destroyedRenderers.Contains(id))
			{
				destroyedRenderers.Add(id);
			}
		}
		else
		{
			createdRenderers.RemoveAt(id);
		}
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