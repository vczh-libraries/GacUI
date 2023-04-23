#include "GuiFakeClipboardService.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
FakeClipboardReader
***********************************************************************/

		class FakeClipboardReader : public Object, public INativeClipboardReader
		{
		public:
			Nullable<WString>			text;
			Ptr<DocumentModel>			document;
			Ptr<INativeImage>			image;

			bool ContainsText() override
			{
				return text;
			}

			WString GetText() override
			{
				return text ? text.Value() : WString();
			}

			bool ContainsDocument() override
			{
				return document;
			}

			Ptr<DocumentModel> GetDocument() override
			{
				return document;
			}

			bool ContainsImage() override
			{
				return image;
			}

			Ptr<INativeImage> GetImage() override
			{
				return image;
			}
		};

/***********************************************************************
FakeClipboardWriter
***********************************************************************/

		class FakeClipboardWriter : public Object, public INativeClipboardWriter
		{
		public:
			FakeClipboardService*		service;
			Ptr<FakeClipboardReader>	reader;

			FakeClipboardWriter(FakeClipboardService* _service)
				: service(_service)
			{
			}

			void SetText(const WString& value) override
			{
				if (reader) reader->text = value;
			}

			void SetDocument(Ptr<DocumentModel> value) override
			{
				if (reader) reader->document = value;
			}

			void SetImage(Ptr<INativeImage> value) override
			{
				if (reader) reader->image = value;
			}

			bool Submit() override
			{
				if (!reader) return false;
				service->reader = reader;
				reader = nullptr;
				service = nullptr;
				GetCurrentController()->CallbackService()->Invoker()->InvokeClipboardUpdated();
				return true;
			}
		};

/***********************************************************************
FakeClipboardService
***********************************************************************/

		FakeClipboardService::FakeClipboardService()
			: reader(Ptr(new FakeClipboardReader))
		{
		}

		FakeClipboardService::~FakeClipboardService()
		{
		}

		Ptr<INativeClipboardReader> FakeClipboardService::ReadClipboard()
		{
			return reader;
		}

		Ptr<INativeClipboardWriter> FakeClipboardService::WriteClipboard()
		{
			return Ptr(new FakeClipboardWriter(this));
		}
	}
}