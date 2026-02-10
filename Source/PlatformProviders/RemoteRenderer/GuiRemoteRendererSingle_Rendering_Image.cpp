#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace collections;
	using namespace elements;
	using namespace remoteprotocol;

/***********************************************************************
* Rendering (Elements -- Image)
***********************************************************************/

	remoteprotocol::ImageMetadata GuiRemoteRendererSingle::CreateImageMetadata(vint id, INativeImage* image)
	{
		ImageMetadata response;
		response.id = id;
		response.format = image->GetFormat();
		response.frames = Ptr(new List<ImageFrameMetadata>);
		for (vint i = 0; i < image->GetFrameCount(); i++)
		{
			auto frame = image->GetFrame(i);
			response.frames->Add({ frame->GetSize() });
		}

		return response;
	}

	remoteprotocol::ImageMetadata GuiRemoteRendererSingle::CreateImage(const remoteprotocol::ImageCreation& arguments)
	{
		arguments.imageData->SeekFromBegin(0);
		auto image = GetCurrentController()->ImageService()->CreateImageFromStream(*arguments.imageData.Obj());
		if (availableImages.Keys().Contains(arguments.id))
		{
			availableImages.Set(arguments.id, image);
		}
		else
		{
			availableImages.Add(arguments.id, image);
		}
		return CreateImageMetadata(arguments.id, image.Obj());
	}
	
	void GuiRemoteRendererSingle::RequestImageCreated(vint id, const remoteprotocol::ImageCreation& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestImageCreated(const ImageCreation&)#"
		CHECK_ERROR(!arguments.imageDataOmitted && arguments.imageData, ERROR_MESSAGE_PREFIX L"Binary content of the image is missing.");

		events->RespondImageCreated(id, CreateImage(arguments));
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteRendererSingle::RequestImageDestroyed(const vint& arguments)
	{
		availableImages.Remove(arguments);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_ImageFrame(const remoteprotocol::ElementDesc_ImageFrame& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererUpdateElement_ImageFrame(const arguments&)#"

		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiImageFrameElement>();
		if (!element) return;

		element->SetAlignments(GetAlignment(arguments.horizontalAlignment), GetAlignment(arguments.verticalAlignment));
		element->SetStretch(arguments.stretch);
		element->SetEnabled(arguments.enabled);

		if (arguments.imageId && arguments.imageCreation)
		{
			CHECK_ERROR(arguments.imageId.Value() == arguments.imageCreation.Value().id, ERROR_MESSAGE_PREFIX L"imageId and imageCreation.id must be identical.");
		}

		if (arguments.imageId)
		{
			if (arguments.imageCreation && !elementMeasurings.createdImages)
			{
				elementMeasurings.createdImages = Ptr(new List<ImageMetadata>);
			}

			vint index = availableImages.Keys().IndexOf(arguments.imageId.Value());
			if (index == -1)
			{
				CHECK_ERROR(arguments.imageCreation && !arguments.imageCreation.Value().imageDataOmitted && arguments.imageCreation.Value().imageData, ERROR_MESSAGE_PREFIX L"Binary content of the image is missing.");

				auto response = CreateImage(arguments.imageCreation.Value());
				element->SetImage(availableImages[response.id], arguments.imageFrame);
				elementMeasurings.createdImages->Add(response);
			}
			else
			{
				auto image = availableImages.Values()[index];
				element->SetImage(image, arguments.imageFrame);
				if (arguments.imageCreation)
				{
					elementMeasurings.createdImages->Add(CreateImageMetadata(arguments.imageId.Value(), image.Obj()));
				}
			}
		}
#undef ERROR_MESSAGE_PREFIX
	}
}