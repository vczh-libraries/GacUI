/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteController

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTEGRAPHICS_IMAGESERVICE
#define VCZH_PRESENTATION_GUIREMOTEGRAPHICS_IMAGESERVICE

#include "Protocol/Generated/GuiRemoteProtocolSchema.h"

namespace vl::presentation
{
	class GuiRemoteController;

/***********************************************************************
GuiRemoteGraphicsImage
***********************************************************************/

	class GuiRemoteGraphicsImage;
	class GuiRemoteGraphicsImageService;

	class GuiRemoteGraphicsImageFrame : public Object, public virtual INativeImageFrame
	{
		friend class GuiRemoteGraphicsImage;
	protected:
		GuiRemoteGraphicsImage*				image;
		Size								size;

	public:
		GuiRemoteGraphicsImageFrame(GuiRemoteGraphicsImage* _image);
		~GuiRemoteGraphicsImageFrame();

		INativeImage*						GetImage() override;
		Size								GetSize() override;
		bool								SetCache(void* key, Ptr<INativeImageFrameCache> cache) override;
		Ptr<INativeImageFrameCache>			GetCache(void* key) override;
		Ptr<INativeImageFrameCache>			RemoveCache(void* key) override;
	};

	class GuiRemoteGraphicsImage : public Object, public virtual INativeImage
	{
		friend class GuiRemoteGraphicsImageFrame;
		friend class GuiRemoteGraphicsImageService;
		using ImageFrameList = collections::List<Ptr<GuiRemoteGraphicsImageFrame>>;
	protected:
		enum class MetadataStatus
		{
			Uninitialized,
			Requested,
			Retrived,
		};

		GuiRemoteController*				remote;
		vint								id = -1;
		Ptr<stream::MemoryStream>			binary;
		INativeImage::FormatType			format = INativeImage::Unknown;
		ImageFrameList						frames;
		MetadataStatus						status = MetadataStatus::Uninitialized;

	public:
		GuiRemoteGraphicsImage(GuiRemoteController* _remote, vint _id, Ptr<stream::MemoryStream> _binary);
		~GuiRemoteGraphicsImage();

		INativeImageService*				GetImageService() override;
		FormatType							GetFormat() override;
		vint								GetFrameCount() override;
		INativeImageFrame*					GetFrame(vint index) override;
		void								SaveToStream(stream::IStream& imageStream, FormatType formatType) override;
	};

/***********************************************************************
GuiRemoteGraphicsImageService
***********************************************************************/

	class GuiRemoteGraphicsImageService : public Object, public virtual INativeImageService
	{
		using ImageMap = collections::Dictionary<vint, Ptr<GuiRemoteGraphicsImage>>;
	protected:
		GuiRemoteController*				remote;
		vint								usedImageIds = 0;
		ImageMap							images;

		Ptr<GuiRemoteGraphicsImage>			CreateImage(Ptr<stream::MemoryStream> binary);
	public:
		GuiRemoteGraphicsImageService(GuiRemoteController* _remote);
		~GuiRemoteGraphicsImageService();

		Ptr<INativeImage>					CreateImageFromFile(const WString& path) override;
		Ptr<INativeImage>					CreateImageFromMemory(void* buffer, vint length) override;
		Ptr<INativeImage>					CreateImageFromStream(stream::IStream& imageStream) override;
	};
}

#endif