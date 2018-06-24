#include "GuiDocumentClipboard.h"

namespace vl
{
	namespace presentation
	{
		using namespace stream;

		void SaveDocumentToHtml(Ptr<DocumentModel> model, WString& header, WString& content, WString& footer)
		{

		}

		void SaveDocumentToHtmlClipboardStream(Ptr<DocumentModel> model, stream::IStream& stream)
		{
			WString header, content, footer;
			SaveDocumentToHtml(model, header, content, footer);

			stream::MemoryStream headerStream, contentStream, footerStream;
#define CONVERT_TO_UTF8(NAME)\
			{ \
				Utf8Encoder encoder; \
				EncoderStream encoderStream(NAME##Stream, encoder); \
				StreamWriter writer(encoderStream); \
				writer.WriteString(NAME); \
				NAME##Stream.SeekFromBegin(0); \
			}
			CONVERT_TO_UTF8(header);
			CONVERT_TO_UTF8(content);
			CONVERT_TO_UTF8(footer);
#undef CONVERT_TO_UTF8

			char clipboardHeader[] =
				"StartHTML:-1\r\n"
				"EndHTML:-1\r\n"
				"StartFragment:0000000000\r\n"
				"EndFragment:0000000000\r\n"
				;
			char commentStart[] = "<!--StartFragment-->";
			char commentEnd[] = "<!--EndFragment-->";
			vint offsetStart = sizeof(clipboardHeader) - 1 + (vint)headerStream.Size() + sizeof(commentStart) - 1;
			vint offsetEnd = offsetStart + (vint)contentStream.Size();

			AString offsetStartString = itoa(offsetStart);
			AString offsetEndString = itoa(offsetEnd);
			memcpy(strstr(clipboardHeader, "EndFragment:") - offsetStartString.Length() - 2, offsetStartString.Buffer(), offsetStartString.Length());
			memcpy(clipboardHeader + sizeof(clipboardHeader) - 1 - offsetEndString.Length() - 2, offsetEndString.Buffer(), offsetEndString.Length());

			stream.Write(clipboardHeader, sizeof(clipboardHeader) - 1);
			if (headerStream.Size() > 0) stream.Write(headerStream.GetInternalBuffer(), (vint)headerStream.Size());
			stream.Write(commentStart, sizeof(commentStart) - 1);
			if (contentStream.Size() > 0) stream.Write(contentStream.GetInternalBuffer(), (vint)contentStream.Size());
			stream.Write(commentEnd, sizeof(commentEnd) - 1);
			if (footerStream.Size() > 0) stream.Write(footerStream.GetInternalBuffer(), (vint)footerStream.Size());
		}
	}
}