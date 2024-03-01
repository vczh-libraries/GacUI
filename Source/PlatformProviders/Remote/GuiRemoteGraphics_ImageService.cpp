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
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsImageFrame::SetCache(void* key, Ptr<INativeImageFrameCache> cache)
	{
		CHECK_FAIL(L"vl::presentation::GuiRemoteGraphicsImageFrame::SetCache(void*, Ptr<INativeImageFrameCache>)#This function should not be called.");
	}

	Ptr<INativeImageFrameCache> GuiRemoteGraphicsImageFrame::GetCache(void* key)
	{
		CHECK_FAIL(L"vl::presentation::GuiRemoteGraphicsImageFrame::GetCache(void*)#This function should not be called.");
	}

	Ptr<INativeImageFrameCache> GuiRemoteGraphicsImageFrame::RemoveCache(void* key)
	{
		CHECK_FAIL(L"vl::presentation:GuiRemoteGraphicsImageFrame::RemoveCache(void*)#This function should not be called.");
	}

/***********************************************************************
GuiRemoteGraphicsImage
***********************************************************************/

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
		CHECK_FAIL(L"Not Implemented!");
	}

	vint GuiRemoteGraphicsImage::GetFrameCount()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	INativeImageFrame* GuiRemoteGraphicsImage::GetFrame(vint index)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteGraphicsImage::SaveToStream(stream::IStream& imageStream, FormatType formatType)
	{
		CHECK_FAIL(L"Not Implemented!");
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