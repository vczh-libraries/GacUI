#include "SharedDocumentEditor.h"

using namespace vl;
using namespace vl::presentation;

namespace gacui_demo
{
	Ptr<DocumentModel> LoadDocument(const WString& fileName)
	{
#define ERROR_MESSAGE_PREFIX L"gacui_demo::LoadDocument()#"
		stream::FileStream file(fileName, stream::FileStream::ReadOnly);
		CHECK_ERROR(file.IsAvailable(), ERROR_MESSAGE_PREFIX L"Cannot open document.");
		auto model = LoadDocumentFromClipboardStream(file);
		CHECK_ERROR(model, ERROR_MESSAGE_PREFIX L"Invalid document.");
		file.Close();
		return model;
#undef ERROR_MESSAGE_PREFIX
	}

	void WriteDocument(stream::MemoryStream& content, const WString& fileName)
	{
#define ERROR_MESSAGE_PREFIX L"gacui_demo::WriteDocument()#"
		stream::FileStream file(fileName, stream::FileStream::WriteOnly);
		CHECK_ERROR(file.IsAvailable(), ERROR_MESSAGE_PREFIX L"Cannot open output.");
		content.SeekFromBegin(0);
		CHECK_ERROR(CopyStream(content, file) == content.Size(), ERROR_MESSAGE_PREFIX L"Incomplete document write.");
		file.Close();
#undef ERROR_MESSAGE_PREFIX
	}

	void SaveDocument(Ptr<DocumentModel> model, const WString& fileName)
	{
		model = model->CopyDocument();
		ModifyDocumentForClipboard(model);
		stream::MemoryStream content;
		SaveDocumentToClipboardStream(model, content);
		WriteDocument(content, fileName);
	}

	void ExportDocumentRtf(Ptr<DocumentModel> model, const WString& fileName)
	{
		AString rtf;
		SaveDocumentToRtf(model, rtf);
		stream::MemoryStream content;
		content.Write((void*)rtf.Buffer(), rtf.Length());
		WriteDocument(content, fileName);
	}

	void ExportDocumentHtml(Ptr<DocumentModel> model, const WString& fileName)
	{
		AString header, body, footer;
		SaveDocumentToHtmlUtf8(model, header, body, footer);
		stream::MemoryStream content;
		content.Write((void*)header.Buffer(), header.Length());
		content.Write((void*)body.Buffer(), body.Length());
		content.Write((void*)footer.Buffer(), footer.Length());
		WriteDocument(content, fileName);
	}
}
