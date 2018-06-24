/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Resource

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_RESOURCES_GUIDOCUMENTCLIPBOARD
#define VCZH_PRESENTATION_RESOURCES_GUIDOCUMENTCLIPBOARD

#include "GuiDocument.h"

namespace vl
{
	namespace presentation
	{
		extern void					ModifyDocumentForClipboard(Ptr<DocumentModel> model);
		extern Ptr<DocumentModel>	LoadDocumentFromClipboardStream(stream::IStream& stream);
		extern void					SaveDocumentToClipboardStream(Ptr<DocumentModel> model, stream::IStream& stream);

		extern void					SaveDocumentToHtml(Ptr<DocumentModel> model, WString& header, WString& content, WString& footer);
		extern void					SaveDocumentToHtmlClipboardStream(Ptr<DocumentModel> model, stream::IStream& stream);
	}
}

#endif