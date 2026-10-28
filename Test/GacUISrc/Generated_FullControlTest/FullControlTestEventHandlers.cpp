#include "FullControlTestEventHandlers.h"
#ifdef GACUI_SKIN_DEVELOPMENT
#include "../../../Source/Skins/DarkSkin/Config/DarkSkinConfig.h"
#else
#include <Skins/DarkSkin/DarkSkin.h>
#endif

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;

namespace demo
{
	void LoadFctDocument(GuiDocumentViewer* viewer, const WString& fileName)
	{
#define ERROR_MESSAGE_PREFIX L"demo::LoadFctDocument()#"
		stream::FileStream file(fileName, stream::FileStream::ReadOnly);
		CHECK_ERROR(file.IsAvailable(), ERROR_MESSAGE_PREFIX L"Cannot open document.");
		auto model = LoadDocumentFromClipboardStream(file);
		CHECK_ERROR(model, ERROR_MESSAGE_PREFIX L"Invalid document.");
		file.Close();
		viewer->SetDocument(model);
#undef ERROR_MESSAGE_PREFIX
	}

	void WriteFctDocument(stream::MemoryStream& content, const WString& fileName)
	{
		stream::FileStream file(fileName, stream::FileStream::WriteOnly);
		CHECK_ERROR(file.IsAvailable(), L"demo::WriteFctDocument()#Cannot open output.");
		content.SeekFromBegin(0);
		CHECK_ERROR(CopyStream(content, file) == content.Size(), L"demo::WriteFctDocument()#Incomplete document write.");
		file.Close();
	}

	void SaveFctDocument(GuiDocumentViewer* viewer, const WString& fileName)
	{
		viewer->SelectAll();
		auto model = viewer->GetSelectionModel();
		ModifyDocumentForClipboard(model);
		stream::MemoryStream content;
		SaveDocumentToClipboardStream(model, content);
		WriteFctDocument(content, fileName);
	}

	void ExportFctDocumentRtf(GuiDocumentViewer* viewer, const WString& fileName)
	{
		viewer->SelectAll();
		auto model = viewer->GetSelectionModel();
		AString rtf;
		SaveDocumentToRtf(model, rtf);
		stream::MemoryStream content;
		content.Write((void*)rtf.Buffer(), rtf.Length());
		WriteFctDocument(content, fileName);
	}

	void ExportFctDocumentHtml(GuiDocumentViewer* viewer, const WString& fileName)
	{
		viewer->SelectAll();
		auto model = viewer->GetSelectionModel();
		AString header, body, footer;
		SaveDocumentToHtmlUtf8(model, header, body, footer);
		stream::MemoryStream content;
		content.Write((void*)header.Buffer(), header.Length());
		content.Write((void*)body.Buffer(), body.Length());
		content.Write((void*)footer.Buffer(), footer.Length());
		WriteFctDocument(content, fileName);
	}

	void OnPaletteSelected(vint preset)
	{
		GetApplication()->InvokeInMainThread(GetApplication()->GetMainWindow(), [preset]()
		{
#define ERROR_MESSAGE_PREFIX L"demo::OnPaletteSelected()#"
			darkskin::ColorPackage colors;
			switch (preset)
			{
			case 0: colors = darkskin::CreateDefaultColorPackage(); break;
			case 1: colors = darkskin::CreateAuroraColorPackage(); break;
			case 2: colors = darkskin::CreateEmberColorPackage(); break;
			case 3: colors = darkskin::CreateMoonstoneColorPackage(); break;
			case 4: colors = darkskin::CreateLagoonColorPackage(); break;
			case 5: colors = darkskin::CreateRosewoodColorPackage(); break;
			default: CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unknown palette.");
			}
			darkskin::SetColorPackage(colors);
			GetApplication()->RefreshThemes();
#undef ERROR_MESSAGE_PREFIX
		});
	}
}
