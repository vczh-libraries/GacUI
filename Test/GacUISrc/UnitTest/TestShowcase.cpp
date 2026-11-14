#include "TestControls.h"

#if defined VCZH_MSVC || defined VCZH_GCC
#include "../../../Source/Skins/TuiSkin/Config/TuiSkinConfig.h"
#include "../../../Source/Utilities/FakeServices/TuiDialogs/Source/TuiFakeDialogServiceUIReflection.h"

using namespace vl::collections;

namespace vl::reflection::description
{
	extern bool LoadTuiSkinTypes();
}

namespace tui_provider_tests
{
	extern void RunGuiTest(const Func<void()>& test, Size size);
}

namespace showcase_tests
{
	class RefreshCounter : public GuiCustomControl
	{
	protected:
		void AfterControlTemplateInstalled(bool initialize) override
		{
			GuiCustomControl::AfterControlTemplateInstalled(initialize);
			if (!initialize) refreshes++;
		}
	public:
		vint refreshes = 0;
		RefreshCounter() : GuiCustomControl(theme::ThemeName::CustomControl) {}
	};

	class MemoryFrame : public Object, public INativeImageFrame
	{
	public:
		INativeImage* image;
		MemoryFrame(INativeImage* _image) : image(_image) {}
		INativeImage* GetImage() override { return image; }
		Size GetSize() override { return Size(123, 456); }
		bool SetCache(void*, Ptr<INativeImageFrameCache>) override { CHECK_FAIL(L"Image labels do not render image frames."); }
		Ptr<INativeImageFrameCache> GetCache(void*) override { return nullptr; }
		Ptr<INativeImageFrameCache> RemoveCache(void*) override { return nullptr; }
	};

	class MemoryImage : public Object, public INativeImage
	{
	public:
		MemoryFrame frame{ this };
		INativeImageService* GetImageService() override { return nullptr; }
		FormatType GetFormat() override { return Png; }
		vint GetFrameCount() override { return 2; }
		INativeImageFrame* GetFrame(vint index) override { TEST_ASSERT(index == 1); return &frame; }
		void SaveToStream(stream::IStream&, FormatType) override { CHECK_FAIL(L"This test inspects image identity without encoding."); }
	};

	void VerifyImages(const Value& editor)
	{
		auto root = editor.GetRawPtr()->SafeAggregationCast<GuiInstanceRootObject>();
		auto viewer = FindObjectByName<GuiDocumentViewer>(root, L"document");
		auto model = Ptr(new DocumentModel);
		auto paragraph = Ptr(new DocumentParagraphRun);
		paragraph->alignment = Alignment::Right;
		auto image = Ptr(new DocumentImageRun);
		image->image = Ptr<INativeImage>(new MemoryImage);
		image->frameIndex = 1;
		image->sizeOverride = Size(12, 15);
		image->baseline = 8;
		paragraph->runs.Add(image);
		model->paragraphs.Add(paragraph);
		auto unused = Ptr(new DocumentStyle);
		unused->styles = Ptr(new DocumentStyleProperties);
		unused->styles->color = Color(12, 34, 56);
		model->styles.Add(L"Unused image style", unused);
		for (vint pass = 0; pass < 3; pass++)
		{
			editor.Invoke(L"LoadDocument", (Value_xs(), BoxValue(model->CopyDocument())));
			TEST_ASSERT(viewer->GetDocumentItems().Count() == 1);
			auto item = viewer->GetDocumentItems().Values()[0];
			auto label = dynamic_cast<GuiLabel*>(item->GetContainer()->Children()[0]->GetAssociatedControl());
			TEST_ASSERT(label && label->GetText() == L"[Image, 123 x 456]");
			TEST_ASSERT(viewer->GetDocument()->paragraphs[0]->runs[0].Cast<DocumentEmbeddedObjectRun>());
			TEST_ASSERT(!viewer->GetModified());
			GetApplication()->RefreshThemes();
			TEST_ASSERT(label->GetText() == L"[Image, 123 x 456]");
			auto saving = viewer->GetDocument()->CopyDocument();
			editor.Invoke(L"PrepareDocumentForSaving", (Value_xs(), BoxValue(saving)));
			auto restored = saving->paragraphs[0]->runs[0].Cast<DocumentImageRun>();
			TEST_ASSERT(restored && restored->image == image->image && restored != image);
			TEST_ASSERT(restored->frameIndex == 1 && restored->sizeOverride.Value() == Size(12, 15) && restored->baseline == 8);
			TEST_ASSERT(restored->source.Length() == 0);
			TEST_ASSERT(saving->paragraphs[0]->alignment.Value() == Alignment::Right);
			TEST_ASSERT(saving->styles[L"Unused image style"]->styles->color.Value() == Color(12, 34, 56));
			TEST_ASSERT(viewer->GetDocument()->paragraphs[0]->runs[0].Cast<DocumentEmbeddedObjectRun>());
			model = saving;
		}
	}
}

TEST_FILE
{
	TEST_CASE(L"TUI showcase repeats arbitrary skin items and retains image labels at document boundaries")
	{
		auto manager = GetGlobalTypeManager();
		SortedList<WString> originalTypes;
		for (vint i = 0; i < manager->GetTypeDescriptorCount(); i++) originalTypes.Add(manager->GetTypeDescriptor(i)->GetTypeName());
		TEST_ASSERT(LoadTuiSkinTypes());
		LoadTuiFakeDialogServiceUITypes();
		for (auto size : { Size(80, 25), Size(120, 40) })
		{
			tui_provider_tests::RunGuiTest([&]()
			{
#ifdef VCZH_64
				auto resourcePath = GetUnitTestFrameworkConfig().resourceFolder / L"../../GacUISrc/Generated_TuiControlTest/Resource_x64/TuiControlTest.UI.bin";
#else
				auto resourcePath = GetUnitTestFrameworkConfig().resourceFolder / L"../../GacUISrc/Generated_TuiControlTest/Resource_x86/TuiControlTest.UI.bin";
#endif
				FileStream resource(resourcePath.GetFullPath(), FileStream::ReadOnly);
				TEST_ASSERT(resource.IsAvailable());
				GetResourceManager()->LoadResourceOrPending(resource, GuiResourceUsage::InstanceClass);
				auto skin = Ptr(new tuiskin::TuiTheme);
				theme::RegisterTheme(skin);
				{
					auto windowValue = Value::Create(L"tuidemo::TuiMainWindow");
					auto window = UnboxValue<GuiWindow*>(windowValue);
					auto items = UnboxValue<Ptr<IValueObservableList>>(windowValue.GetProperty(L"SkinItems"));
					TEST_ASSERT(!windowValue.GetTypeDescriptor()->GetPropertyByName(L"SkinItems", true)->IsWritable());
					auto stack = FindObjectByName<GuiRepeatStackComposition>(window, L"skinItemsStack");
					auto button = [&](vint index)
					{
						auto itemTemplate = dynamic_cast<templates::GuiTemplate*>(stack->GetStackItems()[index]->Children()[0]);
						return UnboxValue<GuiSelectableButton*>(BoxValue(itemTemplate).GetProperty(L"Button"));
					};
					vint calls[3] = {};
					Array<Value> skinItems(3);
					for (vint i = 0; i < 3; i++)
					{
						Array<Value> arguments(2);
						arguments[0] = BoxValue(L"Ordinary item " + itow(i));
						arguments[1] = BoxParameter(Func<void()>([&, i]() { calls[i]++; }));
						skinItems[i] = Value::Create(L"tuidemo::TuiSkinItem", arguments);
						if (i == 0) windowValue.SetProperty(L"SelectedSkinItem", skinItems[i]);
						items->Add(skinItems[i]);
					}
					TEST_ASSERT(stack->GetStackItems().Count() == 3 && stack->GetPadding() == 0);
					TEST_ASSERT(button(0)->GetSelected() && !button(1)->GetSelected() && !button(2)->GetSelected());
					showcase_tests::VerifyImages(window->GetNamedObject(L"editorToolstrip"));
					auto counter = new showcase_tests::RefreshCounter;
					window->GetContainerComposition()->AddChild(counter->GetBoundsComposition());
					counter->refreshes = 0;
					items->RemoveAt(1);
					TEST_ASSERT(stack->GetStackItems().Count() == 2 && button(1)->GetText() == L"Ordinary item 2");
					button(1)->SetSelected(true);
					TEST_ASSERT(calls[0] == 0 && calls[1] == 0 && calls[2] == 0);
					// Retire the radio before its queued action. Only the item is needed by that action.
					items->RemoveAt(1);
					GetApplication()->InvokeInMainThread(window, [&]()
					{
						TEST_ASSERT(calls[0] == 0 && calls[1] == 0 && calls[2] == 1 && counter->refreshes == 1);
						TEST_ASSERT(UnboxValue<Ptr<IValueObservableList>>(windowValue.GetProperty(L"SkinItems")) == items);
						items->Add(skinItems[2]);
						TEST_ASSERT(button(1)->GetSelected() && !button(0)->GetSelected());
						TEST_ASSERT(windowValue.GetProperty(L"SelectedSkinItem") == skinItems[2]);
						// Reconstruct every item without changing selected identity or scheduling another action.
						items->Clear();
						for (auto item : skinItems) items->Add(item);
						TEST_ASSERT(button(2)->GetSelected() && !button(0)->GetSelected() && !button(1)->GetSelected());
						GetApplication()->InvokeInMainThread(window, [&]()
						{
							TEST_ASSERT(calls[0] == 0 && calls[1] == 0 && calls[2] == 1 && counter->refreshes == 1);
							window->Hide();
						});
					});
					GetNativeController()->InputService()->StartTimer();
					GetApplication()->Run(window);
					delete window;
				}
				theme::UnregisterTheme(skin->Name);
			}, size);
		}
		for (vint i = manager->GetTypeDescriptorCount() - 1; i >= 0; i--)
		{
			auto name = manager->GetTypeDescriptor(i)->GetTypeName();
			if (!originalTypes.Contains(name)) TEST_ASSERT(manager->SetTypeDescriptor(name, nullptr));
		}
	});
}
#endif
