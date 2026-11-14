#ifndef VCZH_TUIDEMO_TUICONTROLTESTEVENTHANDLERS
#define VCZH_TUIDEMO_TUICONTROLTESTEVENTHANDLERS

#ifdef GACUI_SKIN_DEVELOPMENT
#include "../SharedDocumentEditor.h"
#else
#include "SharedDocumentEditor.h"
#endif

namespace tuidemo
{
	extern void InitializeTuiSkinItems(const vl::Func<void(const vl::WString&, const vl::Func<void()>&, bool)>& addItem);

	// Call once before showing the window. Image labels belong to the editor;
	// its saving model restores the retained images for every native format.
	template<typename TEditor, typename TSkinItem, typename TWindow>
	void AttachTuiEvents(TWindow* window)
	{
		InitializeTuiSkinItems([window](const vl::WString& name, const vl::Func<void()>& action, bool selected)
		{
			auto item = vl::Ptr(new TSkinItem(name, action));
			if (selected) window->SetSelectedSkinItem(item);
			window->GetSkinItems()->Add(vl::reflection::description::BoxValue(item));
		});
		auto editor = dynamic_cast<TEditor*>(window->GetNamedObject(L"editorToolstrip").GetRawPtr());
		auto viewer = dynamic_cast<vl::presentation::controls::GuiDocumentViewer*>(editor->GetNamedObject(L"document").GetRawPtr());
		auto savingModel = [editor, viewer]()
		{
			auto model = viewer->GetDocument()->CopyDocument();
			editor->PrepareDocumentForSaving(model);
			return model;
		};
		editor->LoadAsPrivateFormat.Add([editor](const vl::WString& fileName) { editor->LoadDocument(gacui_demo::LoadDocument(fileName)); });
		editor->SaveAsPrivateFormat.Add([savingModel](const vl::WString& fileName) { gacui_demo::SaveDocument(savingModel(), fileName); });
		editor->SaveAsRTF.Add([savingModel, viewer](const vl::WString& fileName)
		{
			viewer->SelectAll();
			gacui_demo::ExportDocumentRtf(savingModel(), fileName);
		});
		editor->SaveAsHTML.Add([savingModel, viewer](const vl::WString& fileName)
		{
			viewer->SelectAll();
			gacui_demo::ExportDocumentHtml(savingModel(), fileName);
		});
	}
}

#endif
