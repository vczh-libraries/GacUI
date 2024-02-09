#include "GuiRemoteController.h"
#include "GuiRemoteGraphics_BasicElements.h"
#include "../Hosted/GuiHostedController.h"

namespace vl::presentation::elements
{
	using namespace collections;

/***********************************************************************
GuiRemoteGraphicsRenderTarget
***********************************************************************/

	void GuiRemoteGraphicsRenderTarget::StartRenderingOnNativeWindow()
	{
		canvasSize = remote->remoteWindow.GetClientSize();
		clipperValidArea = GetClipper();
		renderingBatchId++;

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
				renderer->SendUpdateElementMessages(false);
				if (renderer->NeedUpdateMinSizeFromCache())
				{
					if (!renderersAskingForCache.Contains(renderer))
					{
						renderersAskingForCache.Add(renderer);
					}
				}
				renderer->ResetUpdated();
			}
		}

		remote->remoteMessages.RequestRendererBeginRendering();
	}

	RenderTargetFailure GuiRemoteGraphicsRenderTarget::StopRenderingOnNativeWindow()
	{
		vint idRendering = remote->remoteMessages.RequestRendererEndRendering();
		remote->remoteMessages.Submit();
		auto measuring = remote->remoteMessages.RetrieveRendererEndRendering(idRendering);
		remote->remoteMessages.ClearResponses();

		bool minSizeChanged = false;

		if (measuring.fontHeights)
		{
			for (auto&& fontHeight : *measuring.fontHeights.Obj())
			{
				auto key = Tuple(fontHeight.fontFamily, fontHeight.fontSize);
				if (!fontHeights.Keys().Contains(key))
				{
					fontHeights.Add(key, fontHeight.height);
				}
			}

			// TODO: (enumerable) foreach:indexed(alterable(reversed))
			for (vint i = renderersAskingForCache.Count() - 1; i >= 0; i--)
			{
				auto renderer = renderersAskingForCache[i];
				auto oldMinSize = renderer->GetRenderer()->GetMinSize();
				renderer->TryFetchMinSizeFromCache();
				if (renderer->IsRenderedInLastBatch() && oldMinSize != renderer->GetRenderer()->GetMinSize())
				{
					minSizeChanged = true;
				}
				if (!renderer->NeedUpdateMinSizeFromCache())
				{
					renderersAskingForCache.RemoveAt(i);
				}
			}
		}

		if (measuring.minSizes)
		{
			for (auto&& minSize : *measuring.minSizes.Obj())
			{
				vint index = renderers.Keys().IndexOf(minSize.id);
				if (index != -1)
				{
					auto renderer = renderers.Values()[index];
					auto oldMinSize = renderer->GetRenderer()->GetMinSize();
					renderer->UpdateMinSize(minSize.minSize);
					if (renderer->IsRenderedInLastBatch() && oldMinSize != renderer->GetRenderer()->GetMinSize())
					{
						minSizeChanged = true;
					}
				}
			}
		}

		if (minSizeChanged)
		{
			hostedController->RequestRefresh();
		}

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
		clipperValidArea = validArea;
	}

	void GuiRemoteGraphicsRenderTarget::AfterPushedClipperAndBecameInvalid(Rect clipper)
	{
		clipperValidArea.Reset();
	}

	void GuiRemoteGraphicsRenderTarget::AfterPoppedClipperAndBecameValid(Rect validArea, bool clipperExists)
	{
		clipperValidArea = validArea;
	}

	void GuiRemoteGraphicsRenderTarget::AfterPoppedClipper(Rect validArea, bool clipperExists)
	{
		clipperValidArea = validArea;
	}

	GuiRemoteGraphicsRenderTarget::GuiRemoteGraphicsRenderTarget(GuiRemoteController* _remote, GuiHostedController* _hostedController)
		: remote(_remote)
		, hostedController(_hostedController)
	{
	}

	GuiRemoteGraphicsRenderTarget::~GuiRemoteGraphicsRenderTarget()
	{
	}

	void GuiRemoteGraphicsRenderTarget::OnControllerConnect()
	{
		fontHeights.Clear();
		renderersAskingForCache.Clear();

		if (renderers.Count() > 0)
		{
			{
				auto ids = Ptr(new List<remoteprotocol::RendererCreation>);
				for (auto renderer : renderers.Values())
				{
					ids->Add({ renderer->GetID(),renderer->GetRendererType() });
					renderer->NotifyMinSizeCacheInvalidated();
				}
				createdRenderers.Clear();
				remote->remoteMessages.RequestRendererCreated(ids);
			}

			for (auto renderer : renderers.Values())
			{
				renderer->SendUpdateElementMessages(true);
				if (renderer->NeedUpdateMinSizeFromCache())
				{
					renderersAskingForCache.Add(renderer);
				}
				renderer->ResetUpdated();
			}
		}
	}

	void GuiRemoteGraphicsRenderTarget::OnControllerDisconnect()
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
		renderersAskingForCache.Remove(renderer);
		{
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
				createdRenderers.RemoveAt(index);
			}
		}
	}

	Rect GuiRemoteGraphicsRenderTarget::GetClipperValidArea()
	{
		return clipperValidArea.Value();
	}

/***********************************************************************
GuiRemoteGraphicsResourceManager
***********************************************************************/

	GuiRemoteGraphicsResourceManager::GuiRemoteGraphicsResourceManager(GuiRemoteController* _remote, GuiHostedController* _hostedController)
		: remote(_remote)
		, renderTarget(_remote, _hostedController)
		, hostedController(_hostedController)
	{
		remote->resourceManager = this;
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

	void GuiRemoteGraphicsResourceManager::OnControllerConnect()
	{
		renderTarget.OnControllerConnect();
		hostedController->RequestRefresh();
	}

	void GuiRemoteGraphicsResourceManager::OnControllerDisconnect()
	{
		renderTarget.OnControllerDisconnect();
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