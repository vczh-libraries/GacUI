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

		vint idImageCreated = remote->remoteMessages.RequestImageCreated(arguments);
		remote->remoteMessages.Submit();
		auto imageMetadata = remote->remoteMessages.RetrieveImageCreated(idImageCreated);
		remote->remoteMessages.ClearResponses();

		format = imageMetadata.format;
		if (imageMetadata.frames)
		{
			for (auto imageFrameMetadata : *imageMetadata.frames.Obj())
			{
				auto frame = Ptr(new GuiRemoteGraphicsImageFrame(this));
				frame->size = imageFrameMetadata.size;
				frames.Add(frame);
			}
		}

		status = MetadataStatus::Retrived;
	}

	GuiRemoteGraphicsImage::GuiRemoteGraphicsImage(GuiRemoteController * _remote, vint _id, Ptr<stream::MemoryStream> _binary)
		: remote(_remote)
		, id(_id)
		, binary(_binary)
	{
	}

	GuiRemoteGraphicsImage::~GuiRemoteGraphicsImage()
	{
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
		auto image = Ptr(new GuiRemoteGraphicsImage(remote, usedImageIds++, binary));
		images.Add(image->id, image);
		return image;
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

	Ptr<INativeImage> GuiRemoteGraphicsImageService::CreateImageFromFile(const WString& path)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteGraphicsImageService::CreateImageFromFile(const WString&)#"
		stream::FileStream fileStream(path, stream::FileStream::ReadOnly);
		CHECK_ERROR(fileStream.IsAvailable(), ERROR_MESSAGE_PREFIX L"Unable to open file.");

		auto memoryStream = Ptr(new stream::MemoryStream((vint)fileStream.Size()));
		CopyStream(fileStream, *memoryStream.Obj());
		return CreateImage(memoryStream);
#undef ERROR_MESSAGE_PREFIX
	}

	Ptr<INativeImage> GuiRemoteGraphicsImageService::CreateImageFromMemory(void* buffer, vint length)
	{
		auto memoryStream = Ptr(new stream::MemoryStream(length));
		memoryStream->Write(buffer, length);
		return CreateImage(memoryStream);
	}

	Ptr<INativeImage> GuiRemoteGraphicsImageService::CreateImageFromStream(stream::IStream& imageStream)
	{
		auto memoryStream = Ptr(new stream::MemoryStream(imageStream.IsLimited() ? (vint)imageStream.Size() : 65536));
		CopyStream(imageStream, *memoryStream.Obj());
		return CreateImage(memoryStream);
	}
}