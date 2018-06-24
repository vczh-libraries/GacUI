#include "GuiDocumentClipboard.h"

namespace vl
{
	namespace presentation
	{
		using namespace stream;

		void SaveDocumentToHtmlUtf8(Ptr<DocumentModel> model, AString& header, AString& content, AString& footer)
		{

		}

		void SaveDocumentToHtmlClipboardStream(Ptr<DocumentModel> model, stream::IStream& stream)
		{
			AString header, content, footer;
			SaveDocumentToHtmlUtf8(model, header, content, footer);

			char clipboardHeader[] =
				"StartHTML:-1\r\n"
				"EndHTML:-1\r\n"
				"StartFragment:0000000000\r\n"
				"EndFragment:0000000000\r\n"
				;
			char commentStart[] = "<!--StartFragment-->";
			char commentEnd[] = "<!--EndFragment-->";
			vint offsetStart = sizeof(clipboardHeader) - 1 + header.Length() + sizeof(commentStart) - 1;
			vint offsetEnd = offsetStart + content.Length();

			AString offsetStartString = itoa(offsetStart);
			AString offsetEndString = itoa(offsetEnd);
			memcpy(strstr(clipboardHeader, "EndFragment:") - offsetStartString.Length() - 2, offsetStartString.Buffer(), offsetStartString.Length());
			memcpy(clipboardHeader + sizeof(clipboardHeader) - 1 - offsetEndString.Length() - 2, offsetEndString.Buffer(), offsetEndString.Length());

			stream.Write(clipboardHeader, sizeof(clipboardHeader) - 1);
			if (header.Length() > 0) stream.Write((void*)header.Buffer(), header.Length());
			stream.Write(commentStart, sizeof(commentStart) - 1);
			if (content.Length() > 0) stream.Write((void*)content.Buffer(), content.Length());
			stream.Write(commentEnd, sizeof(commentEnd) - 1);
			if (footer.Length() > 0) stream.Write((void*)footer.Buffer(), footer.Length());
		}
	}
}