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

	GuiRemoteGraphicsImage::GuiRemoteGraphicsImage(GuiRemoteController * _remote)
		: remote(_remote)
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

	GuiRemoteGraphicsImageService::GuiRemoteGraphicsImageService(GuiRemoteController* _remote)
		: remote(_remote)
	{
	}

	GuiRemoteGraphicsImageService::~GuiRemoteGraphicsImageService()
	{
	}

	Ptr<INativeImage> GuiRemoteGraphicsImageService::CreateImageFromFile(const WString& path)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Ptr<INativeImage> GuiRemoteGraphicsImageService::CreateImageFromMemory(void* buffer, vint length)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Ptr<INativeImage> GuiRemoteGraphicsImageService::CreateImageFromStream(stream::IStream& imageStream)
	{
		CHECK_FAIL(L"Not Implemented!");
	}
}