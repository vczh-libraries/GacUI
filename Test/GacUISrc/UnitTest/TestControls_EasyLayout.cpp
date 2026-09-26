#include "../../../Source/GacUI.h"
#include "../../../Source/Resources/GuiParserManager.h"
#include "../../../Source/Skins/TuiSkin/Config/TuiSkinConfig.h"
#include "../../../Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

using namespace vl;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;

using namespace vl::presentation::compositions::eazy_layout;

namespace tui_provider_tests
{
	extern void RunGuiTest(const Func<void()>& test);
}

namespace easy_layout_xml_tests
{
	template<typename T>
	void TestPayloadReplacement(T owner)
	{
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		GuiResourcePrecompileContext context;
		GuiResourceError::List errors;
		types::ResolvingResult result;
		auto ownerType = TypeInfoRetriver<T>::CreateTypeInfo();
		IGuiInstanceLoader::TypeInfo typeInfo(GlobalStringKey::Get(ownerType->GetTypeDescriptor()->GetTypeName()), ownerType);
		auto loader = GetInstanceLoaderManager()->GetLoader(typeInfo.typeName);
		auto module = Ptr(new WfModule);
		module->name.value = L"initial_payloads";
		module->moduleType = WfModuleType::Module;
		auto function = Ptr(new WfFunctionDeclaration);
		function->name.value = L"Insert";
		function->functionKind = WfFunctionKind::Normal;
		function->anonymity = WfFunctionAnonymity::Named;
		function->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());
		for (auto name : { L"owner",L"payload" })
		{
			auto argument = Ptr(new WfFunctionArgument);
			argument->name.value = name;
			argument->type = GetTypeFromTypeInfo((WString(name) == L"owner" ? ownerType : TypeInfoRetriver<GuiGraphicsComposition*>::CreateTypeInfo()).Obj());
			function->arguments.Add(argument);
		}
		auto block = Ptr(new WfBlockStatement);
		function->statement = block;
		module->declarations.Add(function);
		{
			auto value = Ptr(new WfReferenceExpression);
			value->name.value = L"payload";
			IGuiInstanceLoader::ArgumentInfo argument;
			argument.expression = value;
			argument.typeInfo = TypeInfoRetriver<GuiGraphicsComposition*>::CreateTypeInfo();
			IGuiInstanceLoader::ArgumentMap arguments;
			arguments.Add(GlobalStringKey::Empty, argument);
			block->statements.Add(loader->AssignParameters(context, result, typeInfo, GlobalStringKey::Get(L"owner"), arguments, {}, errors));
		}
		auto manager = Workflow_GetSharedManager(
#ifdef VCZH_64
			GuiResourceCpuArchitecture::x64
#else
			GuiResourceCpuArchitecture::x86
#endif
		);
		manager->Clear(false, true);
		manager->AddModule(module);
		manager->Rebuild(true);
		TEST_ASSERT(errors.Count() == 0 && manager->errors.Count() == 0);
		auto assembly = workflow::emitter::GenerateAssembly(manager);
		manager->Clear(false, true);
		auto global = Ptr(new WfRuntimeGlobalContext(assembly));
		LoadFunction<void()>(global, L"<initialize>")();
		auto insert = LoadFunction<void(T, GuiGraphicsComposition*)>(global, L"Insert");
		auto first = new GuiBoundsComposition;
		auto second = new GuiBoundsComposition;
		insert(owner, first);
		TEST_ASSERT(owner->GetComposition() == first && !first->GetParent() && !second->GetParent());
		// Generated assignments contain no runtime guard and retain normal setter semantics.
		insert(owner, second);
		TEST_ASSERT(owner->GetComposition() == second);
		owner->SetComposition(nullptr);
		TEST_ASSERT(owner->GetComposition() == nullptr);
	}

	WString Resource(const WString& content)
	{
		return L"<Resource>\n<Instance name=\"Main\">\n<Instance ref.Class=\"easy_test::MainWindow\">\n<Window ref.Name=\"self\" Text=\"Easy layout\" ClientSize=\"x:320 y:240\">\n"
			+ content + L"\n</Window>\n</Instance>\n</Instance>\n</Resource>";
	}

	void RunResourceTest(const WString& resourceText, const Func<void(GuiWindow*)>& test)
	{
		tui_provider_tests::RunGuiTest([&]()
		{
			GuiResourceError::List errors;
			auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
			auto xml = parser->Parse({}, resourceText, errors);
			TEST_ASSERT(xml && errors.Count() == 0);
			auto resource = GuiResource::LoadFromXml(xml, L"TestControls_EasyLayout.xml", L".", errors);
			TEST_ASSERT(resource && errors.Count() == 0);
			resource->Precompile(
#ifdef VCZH_64
				GuiResourceCpuArchitecture::x64,
#else
				GuiResourceCpuArchitecture::x86,
#endif
				nullptr, errors);
			TEST_ASSERT(errors.Count() == 0);
			GetResourceManager()->SetResource(resource, errors, GuiResourceUsage::InstanceClass);
			TEST_ASSERT(errors.Count() == 0);

			auto skin = Ptr(new tuiskin::TuiTheme);
			theme::RegisterTheme(skin);
			{
				auto value = Value::Create(L"easy_test::MainWindow");
				TEST_ASSERT(value.GetRawPtr());
				auto window = Ptr(value.GetRawPtr()->SafeAggregationCast<GuiWindow>());
				TEST_ASSERT(window);
				test(window.Obj());
			}
			theme::UnregisterTheme(skin->Name);
		});
	}

	void Click(GuiButton* button)
	{
		GuiEventArgs args(button->GetBoundsComposition());
		button->Clicked.Execute(args);
	}

	void Drag(GuiTableSplitterCompositionBase* splitter, vint x, vint y)
	{
		TEST_ASSERT(splitter);
		GuiMouseEventArgs args(splitter);
		args.button = NativeMouseButton::Left;
		args.left = true;
		args.x = args.y = 2;
		splitter->GetEventReceiver()->mouseDown.Execute(args);
		args.x += x;
		args.y += y;
		splitter->GetEventReceiver()->mouseMove.Execute(args);
		splitter->GetEventReceiver()->mouseUp.Execute(args);
	}
}

using namespace easy_layout_xml_tests;

TEST_FILE
{
	TEST_CASE(L"Compiled payload assignments retain ordinary replacement for owners and descriptors")
	{
		tui_provider_tests::RunGuiTest([]()
		{
			auto owner = new GuiEasyLayoutComposition;
			TestPayloadReplacement(owner);
			owner->BuildLayout();
			SafeDeleteComposition(owner);
			auto descriptor = Ptr(new GuiEasyTopLayout);
			TestPayloadReplacement(descriptor);
		});
	});

	TEST_CASE(L"Ordinary struct expressions decode cell options in attributes and property elements")
	{
		for (vint element = 0; element < 2; element++)
		for (auto text : { L"composeType:Absolute absolute:(10+20)",L"composeType:presentation::compositions::GuiCellOption::ComposeType::Absolute; absolute:30;",L"absolute:30 composeType:Absolute percentage:(1.0+2.0)" })
		{
			auto property = element ? L"><att.CellOption>" + WString(text) + L"</att.CellOption>" : L" CellOption=\"" + WString(text) + L"\">";
			RunResourceTest(Resource(L"<ez:Layout><ez:Row ref.Name=\"row\"" + property + L"<ez:Column ref.Name=\"column\" CellOption=\"percentage:4\"><Bounds/></ez:Column></ez:Row></ez:Layout>"), [=](GuiWindow* window)
			{
				auto option = FindObjectByName<GuiEasyRowLayout>(window, L"row")->GetCellOption();
				TEST_ASSERT(option.composeType == GuiCellOption::Absolute && option.absolute == 30);
				TEST_ASSERT(option.percentage == (WString(text).Left(8) == L"absolute" ? 3 : 0));
				auto defaults = FindObjectByName<GuiEasyColumnLayout>(window, L"column")->GetCellOption();
				TEST_ASSERT(defaults.composeType == GuiCellOption::Absolute && defaults.absolute == 20 && defaults.percentage == 4);
			});
		}
	});

	TEST_CASE(L"Initial cell and fill bindings build once and later values require rebuilding")
	{
		const auto text = LR"GacUISrc(
<Resource>
  <Instance name="Main">
    <Instance ref.Class="easy_test::MainWindow">
      <ref.Ctor><![CDATA[{
        if (grid.Children.Count != 1 or fills.Children.Count != 1)
        {
          raise "Layouts were not built before ref.Ctor";
        }
        if (self.Text != "Signal")
        {
          raise "Composition event handlers were not ready before BuildLayout";
        }
        self.Text = "Built";
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="Initial">
        <ev.ChildCompositionUpdated-eval><![CDATA[{ self.Text = "Signal"; }]]></ev.ChildCompositionUpdated-eval>
        <CheckBox ref.Name="toggle" Selected="false"/>
        <ez:Layout ref.Name="grid">
          <ez:Row ref.Name="row" CellOption-bind="{composeType:Absolute absolute:(toggle.Selected ? 40 : 20)}">
            <ez:Column><Bounds/></ez:Column>
          </ez:Row>
        </ez:Layout>
        <ez:Layout ref.Name="fills">
          <ez:Fill ref.Name="fill"><att.Percentage-bind>toggle.Selected ? 3.0 : 2.0</att.Percentage-bind><Bounds/></ez:Fill>
          <ez:Fill Percentage-eval="1.0 + 0.0"><Bounds/></ez:Fill>
        </ez:Layout>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";
		RunResourceTest(text, [](GuiWindow* window)
		{
			TEST_ASSERT(window->GetText() == L"Built");
			auto grid = FindObjectByName<GuiEasyLayoutComposition>(window, L"grid");
			auto fills = FindObjectByName<GuiEasyLayoutComposition>(window, L"fills");
			auto row = FindObjectByName<GuiEasyRowLayout>(window, L"row");
			auto fill = FindObjectByName<GuiEasyFillLayout>(window, L"fill");
			auto gridTable = dynamic_cast<GuiTableComposition*>(grid->Children()[0]->Children()[0]);
			auto fillTable = dynamic_cast<GuiTableComposition*>(fills->Children()[0]->Children()[0]);
			TEST_ASSERT(gridTable->GetRowOption(0).absolute == 20);
			TEST_ASSERT(fillTable->GetColumnOption(0).percentage / fillTable->GetColumnOption(1).percentage == 2);
			FindObjectByName<GuiSelectableButton>(window, L"toggle")->SetSelected(true);
			TEST_ASSERT(row->GetCellOption().absolute == 40 && fill->GetPercentage() == 3);
			TEST_ASSERT(gridTable->GetRowOption(0).absolute == 20);
			TEST_ASSERT(fillTable->GetColumnOption(0).percentage / fillTable->GetColumnOption(1).percentage == 2);
			grid->BuildLayout();
			fills->BuildLayout();
			gridTable = dynamic_cast<GuiTableComposition*>(grid->Children()[0]->Children()[0]);
			fillTable = dynamic_cast<GuiTableComposition*>(fills->Children()[0]->Children()[0]);
			TEST_ASSERT(gridTable->GetRowOption(0).absolute == 40);
			TEST_ASSERT(fillTable->GetColumnOption(0).percentage / fillTable->GetColumnOption(1).percentage == 3);
		});
	});

	TEST_CASE(L"An owning layout rebuilds from its button and retains nested payload state")
	{
		const auto content = LR"GacUISrc(
<ez:Layout ref.Name="layout">
  <ez:Top><CheckBox ref.Name="vertical" Text="Vertical"/></ez:Top>
  <ez:Top>
    <Button ref.Name="rebuild" Text="Rebuild">
      <ev.Clicked-eval><![CDATA[{ layout.BuildLayout(); }]]></ev.Clicked-eval>
    </Button>
  </ez:Top>
  <ez:Fill>
    <ez:Fill Direction-bind="vertical.Selected ? Vertical : Horizontal">
      <SinglelineTextBox ref.Name="editor" Text="Initial"/>
    </ez:Fill>
    <ez:Fill Percentage="2"><Label ref.Name="label" Text-bind="editor.Text"/></ez:Fill>
  </ez:Fill>
  <ez:Bottom>
    <ez:Row CellOption="composeType:Absolute absolute:24">
      <ez:Column CellOption="composeType:Absolute absolute:120"><Label ref.Name="first" Text="First"/></ez:Column>
      <ez:Splitter/>
      <ez:Column CellOption="composeType:Percentage percentage:1"><Label ref.Name="second" Text="Second"/></ez:Column>
    </ez:Row>
  </ez:Bottom>
</ez:Layout>
)GacUISrc";
		RunResourceTest(Resource(content), [](GuiWindow* window)
		{
			// Ready
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto editor = FindObjectByName<GuiSinglelineTextBox>(window, L"editor");
				auto label = FindObjectByName<GuiLabel>(window, L"label");
				TEST_ASSERT(editor->GetBoundsComposition()->GetGlobalBounds().Top() == label->GetBoundsComposition()->GetGlobalBounds().Top());
				editor->SetText(L"Retained");
				auto vertical = FindObjectByName<GuiSelectableButton>(window, L"vertical");
				vertical->SetSelected(!vertical->GetSelected());
			}
			// Text edited and vertical arrangement stored
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto editor = FindObjectByName<GuiSinglelineTextBox>(window, L"editor");
				auto label = FindObjectByName<GuiLabel>(window, L"label");
				TEST_ASSERT(editor->GetText() == L"Retained" && label->GetText() == L"Retained");
				TEST_ASSERT(editor->GetBoundsComposition()->GetGlobalBounds().Top() == label->GetBoundsComposition()->GetGlobalBounds().Top());
				Click(FindObjectByName<GuiButton>(window, L"rebuild"));
			}
			// Owning layout rebuilt vertically
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto editor = FindObjectByName<GuiSinglelineTextBox>(window, L"editor");
				auto label = FindObjectByName<GuiLabel>(window, L"label");
				auto a = editor->GetBoundsComposition()->GetGlobalBounds();
				auto b = label->GetBoundsComposition()->GetGlobalBounds();
				TEST_ASSERT(a.Left() == b.Left() && a.Right() == b.Right() && b.Top() - a.Bottom() == 5);
				TEST_ASSERT(b.Height() >= 2 * a.Height() && b.Height() <= 2 * a.Height() + 2);
				TEST_ASSERT(editor->GetText() == L"Retained" && label->GetText() == L"Retained");
				auto first = FindObjectByName<GuiLabel>(window, L"first")->GetBoundsComposition()->GetGlobalBounds();
				auto second = FindObjectByName<GuiLabel>(window, L"second")->GetBoundsComposition()->GetGlobalBounds();
				TEST_ASSERT(first.Width() == 120 && first.Height() == 24 && first.Top() == second.Top() && second.Left() - first.Right() == 5);
				editor->SetText(editor->GetText() + L" again");
				auto vertical = FindObjectByName<GuiSelectableButton>(window, L"vertical");
				vertical->SetSelected(!vertical->GetSelected());
				Click(FindObjectByName<GuiButton>(window, L"rebuild"));
			}
			// Repeated rebuild restores horizontal arrangement
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto editor = FindObjectByName<GuiSinglelineTextBox>(window, L"editor");
				auto label = FindObjectByName<GuiLabel>(window, L"label");
				auto a = editor->GetBoundsComposition()->GetGlobalBounds();
				auto b = label->GetBoundsComposition()->GetGlobalBounds();
				TEST_ASSERT(a.Top() == b.Top() && a.Bottom() == b.Bottom() && b.Left() - a.Right() == 5);
				TEST_ASSERT(editor->GetText() == L"Retained again" && label->GetText() == L"Retained again");
			}
		});
	});

	TEST_CASE(L"XML shared splitters resize both axes and rebuilding retains bound controls")
	{
		const auto content = LR"GacUISrc(
<ez:Layout ref.Name="layout">
  <ez:Column CellOption="composeType:Absolute absolute:180">
    <ez:Row CellOption="composeType:Absolute absolute:80">
      <Button ref.Name="button" Text-bind="self.Text"><ev.Clicked-eval><![CDATA[{ self.Text = "Retained"; }]]></ev.Clicked-eval></Button>
    </ez:Row>
    <ez:Splitter ref.Name="horizontal"/>
    <ez:Row CellOption="composeType:Percentage percentage:1"><Label Text="Bottom left"/></ez:Row>
  </ez:Column>
  <ez:Splitter ref.Name="vertical"/>
  <ez:Column CellOption="composeType:Percentage percentage:1">
    <ez:Row><Label Text="Top right"/></ez:Row>
    <ez:Splitter/>
    <ez:Row><ez:Layout Border="false"><ez:Fill><Label Text="Nested"/></ez:Fill><ez:Splitter/><ez:Fill><Label Text="Fills"/></ez:Fill></ez:Layout></ez:Row>
  </ez:Column>
</ez:Layout>
)GacUISrc";
		RunResourceTest(Resource(content), [](GuiWindow* window)
		{
			// Shared splitter grid initialized
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				TEST_ASSERT(FindObjectByName<GuiEasySplitterLayout>(window, L"horizontal"));
				TEST_ASSERT(FindObjectByName<GuiEasySplitterLayout>(window, L"vertical"));
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->Children().Count() == 6);
				auto splitter = dynamic_cast<GuiColumnSplitterComposition*>(table->Children()[4]);
				Drag(splitter, 25, 0);
			}
			// Column enlarged by dragging
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetColumnOption(0).absolute == 205);
				auto splitter = dynamic_cast<GuiRowSplitterComposition*>(table->Children()[5]);
				Drag(splitter, 0, 15);
			}
			// Row enlarged by dragging
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetRowOption(0).absolute == 95);
				window->SetClientSize({ 400,300 });
			}
			// Window resizing retains dragged sizes
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetColumnOption(0).absolute == 205 && table->GetRowOption(0).absolute == 95);
				layout->BuildLayout();
			}
			// Rebuild restores configured sizes
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetColumnOption(0).absolute == 180 && table->GetRowOption(0).absolute == 80);
				Click(FindObjectByName<GuiButton>(window, L"button"));
			}
			// Retained control updates its binding
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				TEST_ASSERT(window->GetText() == L"Retained");
				TEST_ASSERT(FindObjectByName<GuiButton>(window, L"button")->GetText() == L"Retained");
			}
		});
	});

	TEST_CASE(L"XML instantiates every descriptor with defaults and both constant property syntaxes")
	{
		for (vint syntax = 0; syntax < 3; syntax++)
		for (vint customNamespace = 0; customNamespace < 2; customNamespace++)
		{
			auto field = [=](const WString& name, const WString& value)
			{
				return syntax == 2 ? L"<att." + name + L">" + value + L"</att." + name + L">" : WString::Empty;
			};
			auto attribute = [=](const WString& name, const WString& value)
			{
				return syntax == 1 ? L" " + name + L"=\"" + value + L"\"" : WString::Empty;
			};
			auto content = L"<ez:Layout ref.Name=\"vertical\"><ez:Bottom ref.Name=\"bottom\"/><ez:Top ref.Name=\"top\"/></ez:Layout>"
				L"<ez:Layout><ez:Right ref.Name=\"right\"/><ez:Left ref.Name=\"left\"/></ez:Layout>"
				L"<ez:Layout><ez:Fill ref.Name=\"fill\"" + attribute(L"Percentage", L"3") + attribute(L"Direction", L"Horizontal") + L">"
				+ field(L"Percentage", L"3") + field(L"Direction", L"Horizontal") + L"<Button Text=\"Payload\"/></ez:Fill></ez:Layout>"
				L"<ez:Layout><ez:Row ref.Name=\"row\"" + attribute(L"CellOption", L"composeType:Absolute absolute:12") + attribute(L"CellSpan", L"1") + L">"
				+ field(L"CellOption", L"composeType:Absolute absolute:12") + field(L"CellSpan", L"1")
				+ L"<ez:Column ref.Name=\"column\"" + attribute(L"CellOption", L"composeType:Percentage percentage:2") + attribute(L"CellSpan", L"1") + L">"
				+ field(L"CellOption", L"composeType:Percentage percentage:2") + field(L"CellSpan", L"1") + L"<Bounds/></ez:Column></ez:Row></ez:Layout>"
				L"<ez:Layout><ez:Column><ez:Row><Bounds/></ez:Row></ez:Column></ez:Layout>"
				L"<ez:Layout Padding=\"0\"><ez:Fill/><ez:Splitter ref.Name=\"splitter\"/><ez:Fill/></ez:Layout>";
			auto resource = easy_layout_xml_tests::Resource(content);
			if (customNamespace)
			{
				resource = L"<Resource><Instance name=\"Main\"><Instance ref.Class=\"easy_test::MainWindow\" "
					L"xmlns=\"presentation::controls::Gui*;presentation::compositions::Gui*Composition\" "
					L"xmlns:ez=\"presentation::compositions::eazy_layout::GuiEasy*Composition;presentation::compositions::eazy_layout::GuiEasy*Layout\">"
					L"<Window ref.Name=\"self\" Text=\"Easy layout\" ClientSize=\"x:320 y:240\">" + content + L"</Window></Instance></Instance></Resource>";
			}
			RunResourceTest(resource, [=](GuiWindow* window)
			{
				// Descriptors initialized
				{
					window->GetBoundsComposition()->ForceCalculateSizeImmediately();
					TEST_ASSERT(FindObjectByName<GuiEasyTopLayout>(window, L"top"));
					TEST_ASSERT(FindObjectByName<GuiEasyBottomLayout>(window, L"bottom"));
					TEST_ASSERT(FindObjectByName<GuiEasyLeftLayout>(window, L"left"));
					TEST_ASSERT(FindObjectByName<GuiEasyRightLayout>(window, L"right"));
					TEST_ASSERT(FindObjectByName<GuiEasySplitterLayout>(window, L"splitter"));
					auto fill = FindObjectByName<GuiEasyFillLayout>(window, L"fill");
					TEST_ASSERT(fill->GetPercentage() == (syntax ? 3 : 1));
					TEST_ASSERT(fill->GetDirection() == (syntax ? GuiEasyLayoutDirection::Horizontal : GuiEasyLayoutDirection::Inherited));
					auto row = FindObjectByName<GuiEasyRowLayout>(window, L"row");
					auto column = FindObjectByName<GuiEasyColumnLayout>(window, L"column");
					TEST_ASSERT(row->GetCellSpan() == 1 && column->GetCellSpan() == 1);
					TEST_ASSERT(row->GetCellOption().composeType == (syntax ? GuiCellOption::Absolute : GuiCellOption::MinSize));
					TEST_ASSERT(column->GetCellOption().composeType == (syntax ? GuiCellOption::Percentage : GuiCellOption::MinSize));
					if (syntax)
					{
						TEST_ASSERT(row->GetCellOption().absolute == 12);
						TEST_ASSERT(column->GetCellOption().percentage == 2);
					}
				}
			});
		}
	});

	TEST_CASE(L"XML builds empty and nested layouts after initial bindings and preserves payload events on rebuild")
	{
		const auto content = LR"GacUISrc(
<ez:Layout ref.Name="outer" Padding="2">
  <ez:Top><CheckBox ref.Name="toggle" Text="Change stored layout" Selected="false"/></ez:Top>
  <ez:Fill>
    <ez:Layout ref.Name="layout" Padding-bind="toggle.Selected ? 9 : 3" Border-bind="toggle.Selected">
      <ez:Fill ref.Name="first" Direction-bind="toggle.Selected ? Vertical : Horizontal">
        <Button ref.Name="button" Text-bind="self.Text"><ev.Clicked-eval><![CDATA[{ self.Text = "Clicked"; }]]></ev.Clicked-eval></Button>
      </ez:Fill>
      <ez:Fill><att.Percentage>2</att.Percentage><Bounds ref.Name="payload" PreferredMinSize="x:20 y:20"/></ez:Fill>
    </ez:Layout>
  </ez:Fill>
  <ez:Bottom>
    <ez:Layout ref.Name="grid" Border="false" Padding="0">
      <ez:Row ref.Name="row" CellSpan-bind="toggle.Selected ? 2 : 1" CellOption="composeType:Absolute absolute:(10+10)">
        <ez:Column CellOption="composeType:Percentage percentage:1"><Bounds PreferredMinSize="x:20 y:20"/></ez:Column>
      </ez:Row>
      <ez:Row><att.CellOption>composeType:Absolute absolute:20</att.CellOption><ez:Column><ez:Layout ref.Name="empty" Padding-eval="7"/></ez:Column></ez:Row>
    </ez:Layout>
  </ez:Bottom>
</ez:Layout>
)GacUISrc";
		RunResourceTest(Resource(content), [](GuiWindow* window)
		{
			// Initial bindings applied before build
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				auto empty = FindObjectByName<GuiEasyLayoutComposition>(window, L"empty");
				auto button = FindObjectByName<GuiButton>(window, L"button");
				TEST_ASSERT(layout->GetPadding() == 3 && !layout->GetBorder());
				TEST_ASSERT(layout->GetAlignmentToParent() == Margin(0, 0, 0, 0));
				TEST_ASSERT(empty->GetAlignmentToParent() == Margin(7, 7, 7, 7));
				TEST_ASSERT(button->GetBoundsComposition()->GetAlignmentToParent() == Margin(0, 0, 0, 0));
				TEST_ASSERT(button->GetText() == window->GetText());
				FindObjectByName<GuiSelectableButton>(window, L"toggle")->SetSelected(true);
			}
			// Bindings change stored values
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				TEST_ASSERT(layout->GetPadding() == 9 && layout->GetBorder());
				TEST_ASSERT(FindObjectByName<GuiEasyFillLayout>(window, L"first")->GetDirection() == GuiEasyLayoutDirection::Vertical);
				TEST_ASSERT(FindObjectByName<GuiEasyRowLayout>(window, L"row")->GetCellSpan() == 2);
				TEST_ASSERT(layout->GetAlignmentToParent() == Margin(0, 0, 0, 0));
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetCellPadding() == 3 && table->GetColumns() == 2);
				layout->BuildLayout();
				FindObjectByName<GuiEasyLayoutComposition>(window, L"grid")->BuildLayout();
			}
			// Rebuilt with new values
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				TEST_ASSERT(layout->GetAlignmentToParent() == Margin(9, 9, 9, 9));
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetCellPadding() == 9 && table->GetRows() == 2);
				Click(FindObjectByName<GuiButton>(window, L"button"));
			}
			// Payload event and binding survived
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				TEST_ASSERT(window->GetText() == L"Clicked");
				TEST_ASSERT(FindObjectByName<GuiButton>(window, L"button")->GetText() == L"Clicked");
			}
		});
	});

	TEST_CASE(L"Mixed docking payloads stretch to the row height through resizing and rebuilding")
	{
		const auto content = LR"GacUISrc(
<ez:Layout>
  <ez:Bottom>
    <ez:Layout Border="false">
      <ez:Row>
        <ez:Column CellSpan="2">
          <ez:Layout ref.Name="row" Border="false">
            <ez:Right>
              <ComboBox ref.Name="combo" SelectedIndex="0">
                <att.ListControl><TextList><att.Items><TextItem Text="One"/><TextItem Text="Two"/></att.Items></TextList></att.ListControl>
              </ComboBox>
            </ez:Right>
            <ez:Right><CheckBox ref.Name="check" Text="Check"/></ez:Right>
            <ez:Left><Button ref.Name="first" Text="A"/></ez:Left>
            <ez:Left><Button ref.Name="second" Text="B"/></ez:Left>
            <ez:Left><Button ref.Name="third" Text="C"/></ez:Left>
            <ez:Left><Bounds ref.Name="payload" PreferredMinSize="x:10 y:55"/></ez:Left>
          </ez:Layout>
        </ez:Column>
      </ez:Row>
      <ez:Row><ez:Column><Label Text="Left"/></ez:Column><ez:Column><Label Text="Right"/></ez:Column></ez:Row>
    </ez:Layout>
  </ez:Bottom>
</ez:Layout>
)GacUISrc";
		RunResourceTest(Resource(content), [](GuiWindow* window)
		{
			auto assertRow = [=]()
			{
				auto payload = FindObjectByName<GuiBoundsComposition>(window, L"payload");
				auto expected = payload->GetGlobalBounds();
				TEST_ASSERT(expected.Height() == 55);
				TEST_ASSERT(payload->GetAlignmentToParent() == Margin(0, 0, 0, 0));
				vint right = -1;
				for (auto name : { L"first",L"second",L"third",L"combo",L"check" })
				{
					auto bounds = FindObjectByName<GuiControl>(window, name)->GetBoundsComposition();
					auto actual = bounds->GetGlobalBounds();
					TEST_ASSERT(bounds->GetAlignmentToParent() == Margin(0, 0, 0, 0));
					TEST_ASSERT(actual.Top() == expected.Top() && actual.Bottom() == expected.Bottom());
					TEST_ASSERT(actual.Left() > right);
					right = actual.Right();
				}
				TEST_ASSERT(FindObjectByName<GuiEasyLayoutComposition>(window, L"row")->GetAlignmentToParent() == Margin(0, 0, 0, 0));
			};
			// Ready
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				assertRow();
				window->SetClientSize({ 500,300 });
			}
			// Row expanded
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				assertRow();
				FindObjectByName<GuiEasyLayoutComposition>(window, L"row")->BuildLayout();
				window->SetClientSize({ 320,240 });
			}
			// Row rebuilt and reduced
			{
				window->GetBoundsComposition()->ForceCalculateSizeImmediately();
				assertRow();
			}
		});
	});
}
