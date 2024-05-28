#include "GuiRemoteController.h"
#include "GuiRemoteGraphics_BasicElements.h"
#include "../Hosted/GuiHostedController.h"
#include "../../Application/GraphicsHost/GuiGraphicsHost.h"

namespace vl::presentation::elements
{
	using namespace collections;
	using namespace compositions;

/***********************************************************************
GuiRemoteGraphicsRenderTarget
***********************************************************************/

	GuiRemoteGraphicsRenderTarget::HitTestResult GuiRemoteGraphicsRenderTarget::GetHitTestResultFromGenerator(reflection::DescriptableObject* generator)
	{
		if (auto composition = dynamic_cast<GuiGraphicsComposition*>(generator))
		{
			auto hitTestResult = composition->GetAssociatedHitTestResult();
			if (hitTestResult != INativeWindowListener::NoDecision)
			{
				if (auto graphicsHost = composition->GetRelatedGraphicsHost())
				{
					if (auto nativeWindow = graphicsHost->GetNativeWindow())
					{
						if (nativeWindow == GetCurrentController()->WindowService()->GetMainWindow())
						{
							return hitTestResult;
						}
					}
				}
			}
		}
		return INativeWindowListener::NoDecision;
	}

	Nullable<GuiRemoteGraphicsRenderTarget::SystemCursorType> GuiRemoteGraphicsRenderTarget::GetCursorFromGenerator(reflection::DescriptableObject* generator)
	{
		if (auto composition = dynamic_cast<GuiGraphicsComposition*>(generator))
		{
			if (auto cursor = composition->GetAssociatedCursor())
			{
				if (auto graphicsHost = composition->GetRelatedGraphicsHost())
				{
					if (auto nativeWindow = graphicsHost->GetNativeWindow())
					{
						if (nativeWindow == GetCurrentController()->WindowService()->GetMainWindow())
						{
							return cursor->GetSystemCursorType();
						}
					}
				}
			}
		}
		return {};
	}

	void GuiRemoteGraphicsRenderTarget::StartRenderingOnNativeWindow()
	{
		CHECK_ERROR(hitTestResults.Count() == 0, L"vl::presentation::elements::GuiRemoteGraphicsRenderTarget::StartRenderingOnNativeWindow()#Internal error: hit test result stack is not cleared.");
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

		{
			remoteprotocol::ElementBeginRendering arguments;
			arguments.frameId = ++usedFrameIds;
			remote->remoteMessages.RequestRendererBeginRendering(arguments);
		}
	}

	RenderTargetFailure GuiRemoteGraphicsRenderTarget::StopRenderingOnNativeWindow()
	{
		CHECK_ERROR(hitTestResults.Count() == 0, L"vl::presentation::elements::GuiRemoteGraphicsRenderTarget::StartRenderingOnNativeWindow()#Internal error: hit test result stack is not cleared.");
		vint idRendering = remote->remoteMessages.RequestRendererEndRendering();
		remote->remoteMessages.Submit();
		auto measuring = remote->remoteMessages.RetrieveRendererEndRendering(idRendering);

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

		if (measuring.createdImages)
		{
			for (auto&& imageMetadata : *measuring.createdImages.Obj())
			{
				auto image = remote->imageService.GetImage(imageMetadata.id);
				image->UpdateFromImageMetadata(imageMetadata);
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

	void GuiRemoteGraphicsRenderTarget::AfterPushedClipper(Rect clipper, Rect validArea, reflection::DescriptableObject* generator)
	{
		clipperValidArea = validArea;
		auto hitTestResult = GetHitTestResultFromGenerator(generator);
		auto cursor = GetCursorFromGenerator(generator);

		remoteprotocol::ElementBoundary arguments;
		if (hitTestResult != INativeWindowListener::NoDecision)
		{
			if (hitTestResults.Count() == 0 || hitTestResults[hitTestResults.Count() - 1] != hitTestResult)
			{
				arguments.hitTestResult = hitTestResult;
			}
			hitTestResults.Add(hitTestResult);
		}
		if (cursor)
		{
			if (cursors.Count() == 0 || cursors[cursors.Count() - 1] != cursor.Value())
			{
				arguments.cursor = cursor.Value();
			}
			cursors.Add(cursor.Value());
		}

		if (arguments.hitTestResult || arguments.cursor)
		{
			arguments.bounds = clipper;
			arguments.areaClippedBySelf = validArea;
			remote->remoteMessages.RequestRendererBeginBoundary(arguments);
		}
	}

	void GuiRemoteGraphicsRenderTarget::AfterPushedClipperAndBecameInvalid(Rect clipper, reflection::DescriptableObject* generator)
	{
		clipperValidArea.Reset();
	}

	void GuiRemoteGraphicsRenderTarget::AfterPoppedClipperAndBecameValid(Rect validArea, bool clipperExists, reflection::DescriptableObject* generator)
	{
		clipperValidArea = validArea;
	}

	void GuiRemoteGraphicsRenderTarget::AfterPoppedClipper(Rect validArea, bool clipperExists, reflection::DescriptableObject* generator)
	{
		clipperValidArea = validArea;
		auto hitTestResult = GetHitTestResultFromGenerator(generator);
		auto cursor = GetCursorFromGenerator(generator);
		bool needEndBoundary = false;

		if (hitTestResult != INativeWindowListener::NoDecision)
		{
			hitTestResults.RemoveAt(hitTestResults.Count() - 1);
			if (hitTestResults.Count() == 0 || hitTestResults[hitTestResults.Count() - 1] != hitTestResult)
			{
				needEndBoundary = true;
			}
		}
		if (cursor)
		{
			cursors.RemoveAt(cursors.Count() - 1);
			if (cursors.Count() == 0 || cursors[cursors.Count() - 1] != cursor.Value())
			{
				needEndBoundary = true;
			}
		}

		if (needEndBoundary)
		{
			remote->remoteMessages.RequestRendererEndBoundary();
		}
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