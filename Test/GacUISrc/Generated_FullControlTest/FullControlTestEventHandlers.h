#ifndef VCZH_DEMO_FULLCONTROLTESTEVENTHANDLERS
#define VCZH_DEMO_FULLCONTROLTESTEVENTHANDLERS

#ifdef GACUI_SKIN_DEVELOPMENT
#include "../../../Source/GacUI.h"
#else
#include <GacUI.h>
#endif

namespace demo
{
	extern void OnPaletteSelected(vl::vint preset);
	extern void LoadFctDocument(vl::presentation::controls::GuiDocumentViewer* viewer, const vl::WString& fileName);
	extern void SaveFctDocument(vl::presentation::controls::GuiDocumentViewer* viewer, const vl::WString& fileName);
	extern void ExportFctDocumentRtf(vl::presentation::controls::GuiDocumentViewer* viewer, const vl::WString& fileName);
	extern void ExportFctDocumentHtml(vl::presentation::controls::GuiDocumentViewer* viewer, const vl::WString& fileName);

	// Call once after creating each main window. Each editor owns its viewer;
	// palette refresh replaces templates without reattaching these handlers.
	template<typename TEditor, typename TWindow>
	void AttachFctEvents(TWindow* window)
	{
		window->PaletteSelected.Add(&OnPaletteSelected);
		for (auto name : { L"editorRibbon", L"editorToolstrip" })
		{
			auto editor = dynamic_cast<TEditor*>(window->GetNamedObject(name).GetRawPtr());
			auto viewer = dynamic_cast<vl::presentation::controls::GuiDocumentViewer*>(editor->GetNamedObject(L"document").GetRawPtr());
			editor->LoadAsPrivateFormat.Add([viewer](const vl::WString& fileName) { LoadFctDocument(viewer, fileName); });
			editor->SaveAsPrivateFormat.Add([viewer](const vl::WString& fileName) { SaveFctDocument(viewer, fileName); });
			editor->SaveAsRTF.Add([viewer](const vl::WString& fileName) { ExportFctDocumentRtf(viewer, fileName); });
			editor->SaveAsHTML.Add([viewer](const vl::WString& fileName) { ExportFctDocumentHtml(viewer, fileName); });
		}
	}
}

#endif
