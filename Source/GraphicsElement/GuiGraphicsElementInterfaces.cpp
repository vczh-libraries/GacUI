#include "GuiGraphicsElementInterfaces.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{
			using namespace collections;

/***********************************************************************
GuiGraphicsRenderTarget
***********************************************************************/

			bool GuiGraphicsRenderTarget::IsInHostedRendering()
			{
				return hostedRendering;
			}

			void GuiGraphicsRenderTarget::StartHostedRendering()
			{
				CHECK_ERROR(!hostedRendering && !rendering, L"vl::presentation::elements::GuiGraphicsRenderTarget::StartHostedRendering()#Wrong timing to call this function.");
				hostedRendering = true;
				StartRenderingOnNativeWindow();
			}

			RenderTargetFailure GuiGraphicsRenderTarget::StopHostedRendering()
			{
				CHECK_ERROR(hostedRendering && !rendering, L"vl::presentation::elements::GuiGraphicsRenderTarget::StopHostedRendering()#Wrong timing to call this function.");
				hostedRendering = false;
				return StopRenderingOnNativeWindow();
			}

			void GuiGraphicsRenderTarget::StartRendering()
			{
				CHECK_ERROR(!rendering, L"vl::presentation::elements::GuiGraphicsRenderTarget::StartRendering()#Wrong timing to call this function.");
				rendering = true;
				if (!hostedRendering)
				{
					StartRenderingOnNativeWindow();
				}
			}

			RenderTargetFailure GuiGraphicsRenderTarget::StopRendering()
			{
				CHECK_ERROR(rendering, L"vl::presentation::elements::GuiGraphicsRenderTarget::StopRendering()#Wrong timing to call this function.");
				rendering = false;
				if (!hostedRendering)
				{
					return StopRenderingOnNativeWindow();
				}
				return RenderTargetFailure::None;
			}

			void GuiGraphicsRenderTarget::PushClipper(Rect clipper, reflection::DescriptableObject* generator)
			{
				if (clipperCoverWholeTargetCounter > 0)
				{
					clipperCoverWholeTargetCounter++;
				}
				else
				{
					Rect previousClipper = GetClipper();
					Rect currentClipper;

					currentClipper.x1 = (previousClipper.x1 > clipper.x1 ? previousClipper.x1 : clipper.x1);
					currentClipper.y1 = (previousClipper.y1 > clipper.y1 ? previousClipper.y1 : clipper.y1);
					currentClipper.x2 = (previousClipper.x2 < clipper.x2 ? previousClipper.x2 : clipper.x2);
					currentClipper.y2 = (previousClipper.y2 < clipper.y2 ? previousClipper.y2 : clipper.y2);

					if (currentClipper.x1 < currentClipper.x2 && currentClipper.y1 < currentClipper.y2)
					{
						clippers.Add(currentClipper);
						AfterPushedClipper(clipper, currentClipper, generator);
					}
					else
					{
						clipperCoverWholeTargetCounter++;
						AfterPushedClipperAndBecameInvalid(clipper, generator);
					}
				}
			}

			void GuiGraphicsRenderTarget::PopClipper(reflection::DescriptableObject* generator)
			{
				if (clipperCoverWholeTargetCounter > 0)
				{
					clipperCoverWholeTargetCounter--;
					if (clipperCoverWholeTargetCounter == 0)
					{
						AfterPoppedClipperAndBecameValid(GetClipper(), clippers.Count() > 0, generator);
					}
				}
				else if (clippers.Count() > 0)
				{
					clippers.RemoveAt(clippers.Count() - 1);
					AfterPoppedClipper(GetClipper(), clippers.Count() > 0, generator);
				}
			}

			Rect GuiGraphicsRenderTarget::GetClipper()
			{
				if (clipperCoverWholeTargetCounter > 0)
				{
					return { {-1,-1},{0,0} };
				}
				else if (clippers.Count() == 0)
				{
					return Rect(Point(0, 0), GetCanvasSize());
				}
				else
				{
					return clippers[clippers.Count() - 1];
				}
			}

			bool GuiGraphicsRenderTarget::IsClipperCoverWholeTarget()
			{
				return clipperCoverWholeTargetCounter > 0;
			}
		}
	}
}