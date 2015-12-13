#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "..\..\..\Source\GacUI.h"
#include "..\..\..\Source\Reflection\GuiInstanceLoader.h"
#include "..\..\..\Source\Reflection\TypeDescriptors\GuiReflectionEvents.h"
#include <Windows.h>

using namespace vl::regex;
using namespace vl::regex_internal;
using namespace vl::parsing;
using namespace vl::parsing::definitions;
using namespace vl::parsing::tabling;
using namespace vl::parsing::xml;
using namespace vl::stream;
using namespace vl::reflection;
using namespace vl::reflection::description;
using namespace vl::collections;
using namespace vl::filesystem;

//#define GUI_GRAPHICS_RENDERER_GDI
#define GUI_GRAPHICS_RENDERER_DIRECT2D

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
#ifdef GUI_GRAPHICS_RENDERER_GDI
	int result=SetupWindowsGDIRenderer();
#endif
#ifdef GUI_GRAPHICS_RENDERER_DIRECT2D
	int result = SetupWindowsDirect2DRenderer();
#endif

	ThreadLocalStorage::DisposeStorages();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

extern void UnitTestInGuiMain();

namespace autocomplete_grammar
{
	extern void InstallTextBox(GuiMultilineTextBox* textBox);
}

void GuiMain_GrammarIntellisense()
{
	GuiWindow window(GetCurrentTheme()->CreateWindowStyle());
	window.GetContainerComposition()->SetPreferredMinSize(Size(600, 480));
	{
		auto textBox = g::NewMultilineTextBox();
		autocomplete_grammar::InstallTextBox(textBox);
		textBox->GetBoundsComposition()->SetAlignmentToParent(Margin(5, 5, 5, 5));
		window.AddChild(textBox);
	}
	window.ForceCalculateSizeImmediately();
	window.MoveToScreenCenter();
	GetApplication()->Run(&window);
}

namespace demo
{
	class IViewModel : public virtual IDescriptable, public Description<IViewModel>
	{
	protected:
		vint x = 0;
		vint y = 0;
	public:
		vint GetX() { return x; }
		void SetX(vint value) {if (x != value){ x = value; XChanged();} }
		Event<void()> XChanged;
		
		vint GetY() { return y; }
		void SetY(vint value) {if (y != value){ y = value; YChanged();} }
		Event<void()> YChanged;
	};

	template<typename TImpl>
	class MainWindow_ : public GuiWindow, public GuiInstancePartialClass<GuiWindow>, public Description<TImpl>
	{
	protected:
		Ptr<IViewModel> ViewModel_;
		GuiSinglelineTextBox* text1;
		GuiSinglelineTextBox* text2;

		void InitializeComponents()
		{
			InitializeFromResource();
			GUI_INSTANCE_REFERENCE(text1);
			GUI_INSTANCE_REFERENCE(text2);
		}
	public:
		MainWindow_(Ptr<IViewModel> ViewModel)
			:GuiInstancePartialClass<GuiWindow>(L"demo::MainWindow")
			, GuiWindow(theme::GetCurrentTheme()->CreateWindowStyle())
		{
			ViewModel_ = ViewModel;
		}

		Ptr<IViewModel> GetViewModel()
		{
			return ViewModel_;
		}

		void buttonClear_Clicked(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
		{
			text1->SetText(L"0");
			text2->SetText(L"0");
		}
	};

	class MainWindow : public MainWindow_<MainWindow>
	{
	public:
		MainWindow(Ptr<IViewModel> ViewModel)
			:MainWindow_<MainWindow>(ViewModel)
		{
			InitializeComponents();
		}
	};
}

namespace vl
{
	namespace reflection
	{
		namespace description
		{
#define _ ,
			DECL_TYPE_INFO(demo::IViewModel)
			DECL_TYPE_INFO(demo::MainWindow)

			IMPL_CPP_TYPE_INFO(demo::IViewModel)
			IMPL_CPP_TYPE_INFO(demo::MainWindow)

			BEGIN_CLASS_MEMBER(demo::IViewModel)
				CLASS_MEMBER_BASE(IDescriptable)

				CLASS_MEMBER_EVENT(XChanged)
				CLASS_MEMBER_PROPERTY_EVENT_FAST(X, XChanged)
				
				CLASS_MEMBER_EVENT(YChanged)
				CLASS_MEMBER_PROPERTY_EVENT_FAST(Y, YChanged)
			END_CLASS_MEMBER(demo::IViewModel)

			BEGIN_CLASS_MEMBER(demo::MainWindow)
				CLASS_MEMBER_BASE(GuiWindow)
				CLASS_MEMBER_CONSTRUCTOR(demo::MainWindow*(Ptr<demo::IViewModel>), { L"ViewModel" })

				CLASS_MEMBER_PROPERTY_READONLY_FAST(ViewModel)
				CLASS_MEMBER_GUIEVENT_HANDLER(buttonClear_Clicked, GuiEventArgs)
			END_CLASS_MEMBER(demo::MainWindow)

			class ResourceLoader : public Object, public ITypeLoader
			{
			public:
				void Load(ITypeManager* manager)override
				{
					ADD_TYPE_INFO(demo::IViewModel)
					ADD_TYPE_INFO(demo::MainWindow)
				}

				void Unload(ITypeManager* manager)override
				{
				}
			};

			class ResourcePlugin : public Object, public IGuiPlugin
			{
			public:
				void Load()override
				{
					GetGlobalTypeManager()->AddTypeLoader(new ResourceLoader);
				}

				void AfterLoad()override
				{
				}

				void Unload()override
				{
				}
			};
			GUI_REGISTER_PLUGIN(ResourcePlugin)
		}
	}
}

using namespace demo;

void GuiMain_Resource()
{
	{
		List<WString> errors;
		auto resource = GuiResource::LoadFromXml(L"UI.xml", errors);
		resource->Precompile(errors);
		CHECK_ERROR(errors.Count() == 0, L"Error");
		{
			FileStream fileStream(L"UI.bin", FileStream::WriteOnly);
			resource->SavePrecompiledBinary(fileStream);
		}
		{
			FileStream fileStream(L"UI.bin", FileStream::ReadOnly);
			resource = GuiResource::LoadPrecompiledBinary(fileStream, errors);
		}
		GetInstanceLoaderManager()->SetResource(L"Resource", resource);

		{
			auto item = resource->GetValueByPath(L"Precompiled/Workflow/InstanceCtor/MainWindowResource");
			auto compiled = item.Cast<GuiInstanceCompiledWorkflow>();
			auto code = compiled->assembly->insAfterCodegen->moduleCodes[0];
			File(L"UI.txt").WriteAllText(code);
		}
	}
	MainWindow window(new IViewModel);
	window.ForceCalculateSizeImmediately();
	window.MoveToScreenCenter();
	GetApplication()->Run(&window);
}

void GuiMain()
{
	UnitTestInGuiMain();
	GuiMain_Resource();
}