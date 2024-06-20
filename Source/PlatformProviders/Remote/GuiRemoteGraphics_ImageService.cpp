#include "GuiRemoteController.h"

namespace vl::presentation
{
/***********************************************************************
GuiRemoteGraphicsImageFrame
***********************************************************************/

	GuiRemoteGraphicsImageFrame::GuiRemoteGraphicsImageFrame(GuiRemoteGraphicsImage* _image)
		: image(_image)
	{
	}

	GuiRemoteGraphicsImageFrame::~GuiRemoteGraphicsImageFrame()
	{
	}

	INativeImage* GuiRemoteGraphicsImageFrame::GetImage()
	{
		return image;
	}

	Size GuiRemoteGraphicsImageFrame::GetSize()
	{
		return size;
	}

/***********************************************************************
GuiRemoteGraphicsImage
***********************************************************************/

	void GuiRemoteGraphicsImage::EnsureMetadata()
	{
		if (status == MetadataStatus::Retrived) return;
		auto arguments = GenerateImageCreation();

		vint idImageCreated = remote->remoteMessages.RequestImageCreated(arguments);
		remote->remoteMessages.Submit();
		auto imageMetadata = remote->remoteMessages.RetrieveImageCreated(idImageCreated);
		UpdateFromImageMetadata(imageMetadata);
	}

	GuiRemoteGraphicsImage::GuiRemoteGraphicsImage(GuiRemoteController* _remote, vint _id, Ptr<stream::MemoryStream> _binary)
		: remote(_remote)
		, id(_id)
		, binary(_binary)
	{
		remote->imageService.images.Add(id, this);
	}

	GuiRemoteGraphicsImage::~GuiRemoteGraphicsImage()
	{
		if (remote)
		{
			if (status == MetadataStatus::Retrived)
			{
				remote->remoteMessages.RequestImageDestroyed(id);
			}
			remote->imageService.images.Remove(id);
		}
	}

	stream::IStream& GuiRemoteGraphicsImage::GetBinaryData()
	{
		return *binary.Obj();
	}

	remoteprotocol::ImageCreation GuiRemoteGraphicsImage::GenerateImageCreation()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteGraphicsImage::GenerateImageCreation()#"
		CHECK_ERROR(status != MetadataStatus::Retrived, L"Cannot call this function when status is Retrived.");

		remoteprotocol::ImageCreation arguments;
		arguments.id = id;
		if (status == MetadataStatus::Uninitialized)
		{
			arguments.imageData = binary;
			arguments.imageDataOmitted = false;
			status = MetadataStatus::Requested;
		}
		else
		{
			arguments.imageDataOmitted = true;
		}

		return arguments;
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteGraphicsImage::UpdateFromImageMetadata(const remoteprotocol::ImageMetadata& imageMetadata)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteGraphicsImage::UpdateFromImageMetadata(const remoteprotocol::ImageMetadata&)#"

		CHECK_ERROR(id == imageMetadata.id, L"Wrong image metadata id specified.");

		format = imageMetadata.format;
		if (imageMetadata.frames)
		{
			if (frames.Count() > 0)
			{
				CHECK_ERROR(frames.Count() == imageMetadata.frames, L"New metadata should be identical to the last one.");
				for (auto [imageFrameMetadata, index] : indexed(*imageMetadata.frames.Obj()))
				{
					CHECK_ERROR(frames[index]->size == imageFrameMetadata.size, L"New metadata should be identical to the last one.");
				}
			}
			else
			{
				for (auto imageFrameMetadata : *imageMetadata.frames.Obj())
				{
					auto frame = Ptr(new GuiRemoteGraphicsImageFrame(this));
					frame->size = imageFrameMetadata.size;
					frames.Add(frame);
				}
			}
		}
		else
		{
			CHECK_ERROR(frames.Count() == 0, L"New metadata should be identical to the last one.");
		}

		status = MetadataStatus::Retrived;
#undef ERROR_MESSAGE_PREFIX
	}

	INativeImageService* GuiRemoteGraphicsImage::GetImageService()
	{
		return remote->ImageService();
	}

	INativeImage::FormatType GuiRemoteGraphicsImage::GetFormat()
	{
		EnsureMetadata();
		return format;
	}

	vint GuiRemoteGraphicsImage::GetFrameCount()
	{
		EnsureMetadata();
		return frames.Count();
	}

	INativeImageFrame* GuiRemoteGraphicsImage::GetFrame(vint index)
	{
		EnsureMetadata();
		return frames[index].Obj();
	}

	void GuiRemoteGraphicsImage::SaveToStream(stream::IStream& imageStream, FormatType formatType)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteGraphicsImage::SaveToStream(IStream&, INativeImage::FormatType)#"
		CHECK_FAIL(ERROR_MESSAGE_PREFIX L"This function should not be called.");
#undef ERROR_MESSAGE_PREFIX
	}

/***********************************************************************
GuiRemoteGraphicsImageService
***********************************************************************/

	Ptr<GuiRemoteGraphicsImage> GuiRemoteGraphicsImageService::CreateImage(Ptr<stream::MemoryStream> binary)
	{
		return Ptr(new GuiRemoteGraphicsImage(remote, ++usedImageIds, binary));
	}

	GuiRemoteGraphicsImageService::GuiRemoteGraphicsImageService(GuiRemoteController* _remote)
		: remote(_remote)
	{
	}

	GuiRemoteGraphicsImageService::~GuiRemoteGraphicsImageService()
	{
	}

	void GuiRemoteGraphicsImageService::OnControllerConnect()
	{
		for (auto image : images.Values())
		{
			image->status = GuiRemoteGraphicsImage::MetadataStatus::Uninitialized;
		}
	}

	void GuiRemoteGraphicsImageService::OnControllerDisconnect()
	{
	}

	void GuiRemoteGraphicsImageService::Initialize()
	{
	}

	void GuiRemoteGraphicsImageService::Finalize()
	{
		// TODO: (enumerable) foreach:reversed
		for (vint i = images.Count() - 1; i >= 0; i--)
		{
			images.Values()[i]->remote = nullptr;
		}
		images.Clear();
	}

	GuiRemoteGraphicsImage* GuiRemoteGraphicsImageService::GetImage(vint id)
	{
		return images[id];
	}

	Ptr<INativeImage> GuiRemoteGraphicsImageService::CreateImageFromFile(const WString& path)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteGraphicsImageService::CreateImageFromFile(const WString&)#"
		CHECK_ERROR(remote, ERROR_MESSAGE_PREFIX L"This function cannot be called when GuiRemoteController is shut down.");
		stream::FileStream fileStream(path, stream::FileStream::ReadOnly);
		CHECK_ERROR(fileStream.IsAvailable(), ERROR_MESSAGE_PREFIX L"Unable to open file.");

		auto memoryStream = Ptr(new stream::MemoryStream((vint)fileStream.Size()));
		CopyStream(fileStream, *memoryStream.Obj());
		return CreateImage(memoryStream);
#undef ERROR_MESSAGE_PREFIX
	}

	Ptr<INativeImage> GuiRemoteGraphicsImageService::CreateImageFromMemory(void* buffer, vint length)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteGraphicsImageService::CreateImageFromMemory(void*, vint)#"
		CHECK_ERROR(remote, ERROR_MESSAGE_PREFIX L"This function cannot be called when GuiRemoteController is shut down.");
		auto memoryStream = Ptr(new stream::MemoryStream(length));
		memoryStream->Write(buffer, length);
		return CreateImage(memoryStream);
#undef ERROR_MESSAGE_PREFIX
	}

	Ptr<INativeImage> GuiRemoteGraphicsImageService::CreateImageFromStream(stream::IStream& imageStream)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteGraphicsImageService::CreateImageFromStream(IStream&)#"
		CHECK_ERROR(remote, ERROR_MESSAGE_PREFIX L"This function cannot be called when GuiRemoteController is shut down.");
		auto memoryStream = Ptr(new stream::MemoryStream(imageStream.IsLimited() ? (vint)imageStream.Size() : 65536));
		imageStream.SeekFromBegin(0);
		CopyStream(imageStream, *memoryStream.Obj());
		return CreateImage(memoryStream);
#undef ERROR_MESSAGE_PREFIX
	}
}