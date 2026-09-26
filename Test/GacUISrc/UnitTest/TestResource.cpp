#include "../../../Source/GacUI.h"
#include "../../../Source/Compiler/GuiInstanceRepresentation.h"
#include "../../../Source/Resources/GuiParserManager.h"
#include "../../../Source/Compiler/GuiCppGen.h"
#include "../../../Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::presentation;

namespace compiler_error_tests
{
	extern WString GetTestResourcePath();
	extern WString GetTestSharedBaselinePath();
	extern WString GetTestPlatformBaselinePath();
	extern WString GetTestOutputPath();
}
using namespace compiler_error_tests;

void WriteErrors(GuiResourceError::List& errors, const WString& resourceName)
{
	auto outputPath = FilePath(GetTestOutputPath()) / (resourceName + L".txt");
	auto baselineShared = FilePath(GetTestSharedBaselinePath()) / (resourceName + L".txt");
	auto baselinePlatform = FilePath(GetTestPlatformBaselinePath()) / (resourceName + L".txt");

	bool baselineSharedExists = File(baselineShared).Exists();
	bool baselinePlatformExists = File(baselinePlatform).Exists();
	TEST_ASSERT(baselineSharedExists ^ baselinePlatformExists);

	List<WString> output;
	GuiResourceError::SortAndLog(errors, output, GetTestResourcePath());
	File(outputPath).WriteAllLines(output, false, BomEncoder::Utf8);

	List<WString> baseline;
	File(baselineSharedExists ? baselineShared : baselinePlatform).ReadAllLinesByBom(baseline);
	if (baseline.Count() > 0 && baseline[baseline.Count() - 1] == L"")
	{
		baseline.RemoveAt(baseline.Count() - 1);
	}

	vint count = output.Count();
	if (count < baseline.Count())
	{
		count = baseline.Count();
	}

	for (vint i = 0; i < count; i++)
	{
		TEST_ASSERT(output.Count() > i);
		TEST_ASSERT(baseline.Count() > i);
#if defined VCZH_MSVC
		TEST_ASSERT(output[i] == baseline[i]);
#elif defined VCZH_GCC
		auto posRes = INVLOC.FindFirst(output[i], L"Resource.", Locale::Normalization::None);
		if (posRes.key == -1)
		{
			TEST_ASSERT(output[i] == baseline[i]);
		}
		else
		{
			auto fixedOutput = output[i].Remove(posRes.key, posRes.value).Insert(posRes.key, L".\\Resource.");
			TEST_ASSERT(output[i] == baseline[i] || fixedOutput == baseline[i]);
		}
#endif
	}
}

#define PRINT_ERROR\
	do\
	{\
		if (errors.Count() > 0)\
		{\
			WriteErrors(errors, resourceName);\
			return;\
		}\
	}while(0)\

Ptr<GuiResource> LoadResource(const WString& resourceName)
{
	Ptr<GuiResource> resource;
	TEST_CASE(L"Compare compiler output for: " + resourceName)
	{
		auto inputPath = FilePath(GetTestResourcePath()) / resourceName;
		GuiResourceError::List errors;

		resource = GuiResource::LoadFromXml(inputPath.GetFullPath(), errors);
		PRINT_ERROR;
		resource->Precompile(GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
		PRINT_ERROR;
		TEST_ASSERT(false);
	});
	return resource;
}

#undef PRINT_ERROR

namespace easy_layout_xml_tests
{
	extern WString Resource(const WString& content);
}

namespace initialization_tests
{
	class Loader : public Object, public IGuiInstanceLoader
	{
	public:
		GlobalStringKey			name;
		Ptr<List<WString>>		log;
		bool					emit;
		GlobalStringKey			expectedName = GlobalStringKey::Get(L"init_tests::Derived");
		description::ITypeDescriptor* expectedType = description::GetTypeDescriptor<templates::GuiControlTemplate>();

		Loader(GlobalStringKey _name, Ptr<List<WString>> _log, bool _emit)
			: name(_name), log(_log), emit(_emit)
		{
		}

		GlobalStringKey GetTypeName() override { return name; }

		Ptr<workflow::WfStatement> InitializeInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, GuiResourceTextPos tagPosition, GuiResourceError::List& errors) override
		{
			TEST_ASSERT(!CanCreate(typeInfo));
			TEST_ASSERT(typeInfo.typeName == expectedName);
			TEST_ASSERT(typeInfo.typeInfo->GetTypeDescriptor() == expectedType);
			TEST_ASSERT(tagPosition.row == 12 && tagPosition.column == 3);
			log->Add(variableName.ToString() + L":" + name.ToString());
			if (!emit) return nullptr;
			auto value = Ptr(new workflow::WfStringExpression);
			value->value.value = log->Get(log->Count() - 1);
			auto statement = Ptr(new workflow::WfExpressionStatement);
			statement->expression = value;
			return statement;
		}
	};
}

extern void SetGuiMainProxy(const Func<void()>& proxy);

TEST_FILE
{
	SetGuiMainProxy([]()
	{
		TEST_CASE(L"Final initialization visits resolved references in postorder and every loader in order")
		{
			auto manager = GetInstanceLoaderManager();
			auto log = Ptr(new List<WString>);
			auto base = GlobalStringKey::Get(L"init_tests::Base");
			auto middle = GlobalStringKey::Get(L"init_tests::Middle");
			auto derived = GlobalStringKey::Get(L"init_tests::Derived");
			TEST_ASSERT(manager->CreateVirtualType(GlobalStringKey::Get(description::TypeInfo<templates::GuiControlTemplate>::content.typeName), Ptr(new initialization_tests::Loader(base, log, true))));
			TEST_ASSERT(manager->CreateVirtualType(base, Ptr(new initialization_tests::Loader(middle, log, false))));
			TEST_ASSERT(manager->CreateVirtualType(middle, Ptr(new initialization_tests::Loader(derived, log, true))));

			GuiResourcePrecompileContext context;
			GuiResourceError::List errors;
			types::ResolvingResult result;
			result.context = Ptr(new GuiInstanceContext);
			auto ns = Ptr(new GuiInstanceContext::NamespaceInfo);
			auto mapping = Ptr(new GuiInstanceNamespace);
			mapping->prefix = L"init_tests::";
			ns->namespaces.Add(mapping);
			result.context->namespaces.Add(GlobalStringKey::Empty, ns);
			auto root = Ptr(new GuiConstructorRepr);
			root->typeName = GlobalStringKey::Get(L"Derived");
			result.context->instance = root;
			auto child = Ptr(new GuiConstructorRepr);
			auto existing = Ptr(new GuiAttSetterRepr);
			auto grandchild = Ptr(new GuiConstructorRepr);
			auto serialized = Ptr(new GuiConstructorRepr);
			auto add = [](Ptr<GuiAttSetterRepr> parent, const wchar_t* property, Ptr<GuiValueRepr> value)
			{
				auto setter = Ptr(new GuiAttSetterRepr::SetterValue);
				setter->values.Add(value);
				parent->setters.Add(GlobalStringKey::Get(property), setter);
			};
			add(root, L"First", child);
			add(root, L"Existing", existing);
			root->setters[GlobalStringKey::Get(L"Existing")]->binding = GlobalStringKey::_Set;
			add(existing, L"Child", grandchild);
			add(root, L"Value", serialized);
			add(root, L"Text", Ptr(new GuiTextRepr));
			vint index = 0;
			for (auto node : { Ptr<GuiAttSetterRepr>(root),Ptr<GuiAttSetterRepr>(child),existing,Ptr<GuiAttSetterRepr>(grandchild) })
			{
				node->instanceName = GlobalStringKey::Get(L"node" + itow(index++));
				node->tagPosition = { {},{12,3} };
				result.typeInfos.Add(node->instanceName, { derived,manager->GetTypeInfoForType(derived) });
			}
			// The root reference can have a generated class type; its hooks still receive the XML source type.
			result.typeInfos.Set(root->instanceName, { GlobalStringKey::Get(L"generated::Root"),description::TypeInfoRetriver<controls::GuiWindow*>::CreateTypeInfo() });
			serialized->instanceName = GlobalStringKey::Get(L"serialized");
			result.typeInfos.Add(serialized->instanceName, { derived,description::TypeInfoRetriver<compositions::GuiCellOption>::CreateTypeInfo() });
			auto block = Ptr(new workflow::WfBlockStatement);
			Workflow_GenerateInitialization(context, result, block, errors);
			TEST_ASSERT(errors.Count() == 0 && result.rootCtorArguments.Count() == 0);
			TEST_ASSERT(log->Count() == 12 && block->statements.Count() == 8);
			vint position = 0;
			for (auto node : { L"node1",L"node3",L"node2",L"node0" })
			for (auto loader : { L"init_tests::Derived",L"init_tests::Middle",L"init_tests::Base" })
			{
				TEST_ASSERT(log->Get(position++) == WString(node) + L":" + loader);
			}
			for (auto statement : block->statements)
			{
				auto recorded = Workflow_GetScriptPosition(context)->nodePositions[statement.Obj()];
				TEST_ASSERT(recorded.position.row == 12 && recorded.position.column == 3);
			}
			auto last = manager->GetLoader(derived);
			while (auto parent = manager->GetParentLoader(last)) last = parent;
			TEST_ASSERT(last->GetTypeName() == GlobalStringKey::Empty);
			TEST_ASSERT(!last->InitializeInstance(context, result, result.typeInfos[child->instanceName], child->instanceName, child->tagPosition, errors));

			// An existing property target need not have any applicable constructor.
			auto noCtorName = GlobalStringKey::Get(L"init_tests::Existing");
			auto noCtor = Ptr(new initialization_tests::Loader(noCtorName, log, false));
			noCtor->expectedName = noCtorName;
			noCtor->expectedType = description::GetTypeDescriptor<compositions::GuiGraphicsComposition>();
			TEST_ASSERT(manager->CreateVirtualType(GlobalStringKey::Get(noCtor->expectedType->GetTypeName()), noCtor));
			result.typeInfos.Set(existing->instanceName, { noCtorName,manager->GetTypeInfoForType(noCtorName) });
			for (auto loader = manager->GetLoader(noCtorName); loader; loader = manager->GetParentLoader(loader))
			{
				TEST_ASSERT(!loader->CanCreate(result.typeInfos[existing->instanceName]));
			}
			log->Clear();
			block->statements.Clear();
			Workflow_GenerateInitialization(context, result, block, errors);
			TEST_ASSERT(log->Count() == 10 && block->statements.Count() == 6);
			TEST_ASSERT(log->Get(6) == L"node2:init_tests::Existing");

			// Ordinary objects contribute no statement, and the owning layout contributes exactly one.
			mapping->prefix = L"presentation::controls::";
			root->typeName = GlobalStringKey::Get(L"GuiWindow");
			root->setters.Clear();
			block->statements.Clear();
			auto marker = Ptr(new workflow::WfExpressionStatement);
			block->statements.Add(marker);
			Workflow_GenerateInitialization(context, result, block, errors);
			TEST_ASSERT(block->statements.Count() == 1 && block->statements[0] == marker);
			auto ownerName = GlobalStringKey::Get(description::TypeInfo<compositions::eazy_layout::GuiEasyLayoutComposition>::content.typeName);
			result.typeInfos.Set(child->instanceName, { ownerName,manager->GetTypeInfoForType(ownerName) });
			TEST_ASSERT(!manager->GetLoader(ownerName)->CanCreate(result.typeInfos[child->instanceName]));
			TEST_ASSERT(last->CanCreate(result.typeInfos[child->instanceName]));
			add(root, L"Content", child);
			Workflow_GenerateInitialization(context, result, block, errors);
			TEST_ASSERT(errors.Count() == 0 && block->statements.Count() == 2 && block->statements[0] == marker);
			auto call = block->statements[1].Cast<workflow::WfExpressionStatement>()->expression.Cast<workflow::WfCallExpression>();
			auto method = call->function.Cast<workflow::WfMemberExpression>();
			TEST_ASSERT(method->name.value == L"BuildLayout");
			TEST_ASSERT(method->parent.Cast<workflow::WfReferenceExpression>()->name.value == child->instanceName.ToString());
		});

		TEST_CASE(L"Easy layout namespace is predefined independently of the default namespace")
		{
			GuiResourceError::List errors;
			auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
			for (vint customized = 0; customized < 2; customized++)
			for (vint explicitEz = 0; explicitEz < 2; explicitEz++)
			{
				auto xml = parser->Parse({}, L"<Instance "
					+ WString(customized ? L"xmlns=\"presentation::controls::Gui*\" " : L"")
					+ WString(explicitEz ? L"xmlns:ez=\"presentation::compositions::Gui*Composition\" " : L"")
					+ L"><Window/></Instance>", errors);
				auto context = GuiInstanceContext::LoadFromXml(nullptr, xml, errors);
				for (vint round = 0; round < 2; round++)
				{
					TEST_ASSERT(errors.Count() == 0);
					TEST_ASSERT(context->namespaces.Keys().Contains(GlobalStringKey::Get(L"ez")));
					auto mapping = context->namespaces[GlobalStringKey::Get(L"ez")];
					TEST_ASSERT(mapping->namespaces.Count() == (explicitEz ? 1 : 2));
					TEST_ASSERT(mapping->namespaces[0]->prefix == (explicitEz ? L"presentation::compositions::Gui" : L"presentation::compositions::eazy_layout::GuiEasy"));
					context = GuiInstanceContext::LoadFromXml(nullptr, context->SaveToXml(), errors);
				}
			}
		});

		TEST_CASE(L"Compiler accepts easy splitter descriptors between fill tracks")
		{
			GuiResourceError::List errors;
			auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
			auto xml = parser->Parse({}, easy_layout_xml_tests::Resource(L"<ez:Layout><ez:Fill/><ez:Splitter/><ez:Fill/></ez:Layout>"), errors);
			TEST_ASSERT(xml && errors.Count() == 0);
			auto resource = GuiResource::LoadFromXml(xml, L"TestControls_EasyLayout.xml", L".", errors);
			TEST_ASSERT(resource);
			if (errors.Count() == 0) PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
			for (auto error : errors) TEST_PRINT(error.message);
			TEST_ASSERT(errors.Count() == 0);
		});

		TEST_CASE(L"Compiler accepts easy layout runtime property expressions")
		{
			GuiResourceError::List errors;
			auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
			auto xml = parser->Parse({}, easy_layout_xml_tests::Resource(L"<ez:Layout><ez:Fill Percentage-eval=\"1.0 + 2.0\"/></ez:Layout>"), errors);
			TEST_ASSERT(xml && errors.Count() == 0);
			auto resource = GuiResource::LoadFromXml(xml, L"TestControls_EasyLayout.xml", L".", errors);
			TEST_ASSERT(resource);
			if (errors.Count() == 0) PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
			for (auto error : errors) TEST_PRINT(error.message);
			TEST_ASSERT(errors.Count() == 0);
		});

		TEST_CASE(L"Compiler retains ordinary syntax, type and binder errors for easy layout properties")
		{
			collections::List<WString> cases;
			cases.Add(L"<ez:Layout><ez:Fill Percentage-eval=\"1.0 +\"/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill><att.Percentage-eval>\"wrong type\"</att.Percentage-eval></ez:Fill></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Row CellOption=\"composeType:Unknown\"><ez:Column/></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Row><att.CellOption>absolute:true</att.CellOption><ez:Column/></ez:Row></ez:Layout>");
			for (auto binder : { L"ref",L"uri",L"set" })
			for (vint element = 0; element < 2; element++)
			{
				auto percentage = element
					? L"<ez:Fill><att.Percentage-" + WString(binder) + L">1.0</att.Percentage-" + binder + L"></ez:Fill>"
					: L"<ez:Fill Percentage-" + WString(binder) + L"=\"1.0\"/>";
				auto option = element
					? L"<ez:Row><att.CellOption-" + WString(binder) + L">{composeType:MinSize}</att.CellOption-" + binder + L"><ez:Column/></ez:Row>"
					: L"<ez:Row CellOption-" + WString(binder) + L"=\"{composeType:MinSize}\"><ez:Column/></ez:Row>";
				cases.Add(L"<ez:Layout>" + percentage + L"</ez:Layout>");
				cases.Add(L"<ez:Layout>" + option + L"</ez:Layout>");
			}
			for (auto content : cases)
			{
				GuiResourceError::List errors;
				auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
				auto xml = parser->Parse({}, easy_layout_xml_tests::Resource(content), errors);
				TEST_ASSERT(xml && errors.Count() == 0);
				auto resource = GuiResource::LoadFromXml(xml, L"TestControls_EasyLayout.xml", L".", errors);
				TEST_ASSERT(resource);
				if (errors.Count() == 0) PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
				TEST_ASSERT(errors.Count() > 0);
				bool positioned = false;
				for (auto error : errors)
				{
					TEST_PRINT(error.message);
					positioned |= error.position.row >= 0 && error.position.column >= 0;
				}
				TEST_ASSERT(positioned);
			}
		});

		TEST_CASE(L"Compiler rejects duplicate initial payloads for easy layout owners and descriptors")
		{
			for (auto tag : { L"ez:Layout",L"ez:Top" })
			for (auto children : {
				L"<Button/><Button/>",
				L"<Bounds/><Bounds/>",
				L"<Button/><Bounds/>",
				L"<Bounds/><Button/>",
				L"<ez:Top/><Button/><ez:Bottom/><Bounds/>",
				L"<Bounds/><ez:Top/><Button/><ez:Bottom/>",
				L"<Button/><ez:Top/><Button/>",
				L"<Bounds/><ez:Top/><Bounds/>",
				})
			{
				auto content = L"<" + WString(tag) + L">" + children + L"</" + tag + L">";
				if (WString(tag) != L"ez:Layout") content = L"<ez:Layout>" + content + L"</ez:Layout>";
				GuiResourceError::List errors;
				auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
				auto xml = parser->Parse({}, easy_layout_xml_tests::Resource(content), errors);
				TEST_ASSERT(xml && errors.Count() == 0);
				auto resource = GuiResource::LoadFromXml(xml, L"TestControls_EasyLayout.xml", L".", errors);
				TEST_ASSERT(resource && errors.Count() == 0);
				PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
				TEST_ASSERT(errors.Count() == 1);
				TEST_ASSERT(errors[0].message == L"Easy layout: initial content cannot contain more than one composition/control payload.");
				TEST_ASSERT(errors[0].position.row >= 0 && errors[0].position.column >= 0);
			}
		});

		TEST_CASE(L"Compiler rejects named content properties for easy layout owners and descriptors")
		{
			for (auto tag : { L"ez:Layout",L"ez:Top" })
			for (auto property : { L"Composition",L"Layouts" })
			{
				auto name = WString(property);
				auto child = name == L"Composition" ? L"<Bounds/>" : L"<ez:Fill/>";
				collections::List<WString> contents;
				contents.Add(L"><att." + name + L">" + child + L"</att." + name + L">");
				contents.Add(L"><Bounds/><att." + name + L">" + child + L"</att." + name + L">");
				contents.Add(L"><att." + name + L"-set/>");
				contents.Add(L" " + name + L"=\"null\">");
				for (auto binder : { L"eval",L"bind",L"uri" })
				{
					contents.Add(L"><att." + name + L"-" + binder + L">null</att." + name + L"-" + binder + L">");
					contents.Add(L" " + name + L"-" + binder + L"=\"null\">");
				}
				for (auto properties : contents)
				{
					auto content = L"<" + WString(tag) + properties + L"</" + tag + L">";
					if (WString(tag) != L"ez:Layout") content = L"<ez:Layout>" + content + L"</ez:Layout>";
					GuiResourceError::List errors;
					auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
					auto xml = parser->Parse({}, easy_layout_xml_tests::Resource(content), errors);
					TEST_ASSERT(xml && errors.Count() == 0);
					auto resource = GuiResource::LoadFromXml(xml, L"TestControls_EasyLayout.xml", L".", errors);
					TEST_ASSERT(resource && errors.Count() == 0);
					PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
					TEST_ASSERT(errors.Count() == 1);
					TEST_ASSERT(INVLOC.FindFirst(errors[0].message, L"Property \"" + name + L"\"", Locale::Normalization::None).key != -1);
					TEST_ASSERT(INVLOC.FindFirst(errors[0].message, L" is not supported.", Locale::Normalization::None).key != -1);
					TEST_ASSERT(errors[0].position.row >= 0 && errors[0].position.column >= 0);
				}
			}
		});

		TEST_CASE(L"Compiler accepts runtime struct fields and defers easy layout grammar to initialization")
		{
			collections::List<WString> cases;
			cases.Add(L"<ez:Layout><ez:Row CellOption=\"composeType:Absolute absolute:self.ClientSize.x\"><ez:Column/></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Row><ez:Column><att.CellOption>composeType:Percentage percentage:(cast double self.ClientSize.x)</att.CellOption></ez:Column></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Top/><Button/></ez:Layout>");
			cases.Add(L"<ez:Layout><Bounds/></ez:Layout><ez:Layout><Button/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Top><Bounds/></ez:Top><ez:Bottom><Bounds/></ez:Bottom></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Top/><ez:Left/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill/><ez:Row><ez:Column/></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Row><Button/></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Column><ez:Top/></ez:Column></ez:Layout>");
			cases.Add(L"<ez:Layout><Cell/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill><StackItem/></ez:Fill></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Splitter/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Splitter/><ez:Fill/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill/><ez:Splitter/><ez:Splitter/><ez:Fill/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill/><ez:Splitter><Bounds/></ez:Splitter><ez:Fill/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill/><ez:Splitter><ez:Fill/></ez:Splitter><ez:Fill/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Row><ez:Splitter/><ez:Column/></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Column><ez:Row/><ez:Splitter/><ez:Splitter/><ez:Row/></ez:Column></ez:Layout>");
			for (auto content : cases)
			{
				GuiResourceError::List errors;
				auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
				auto xml = parser->Parse({}, easy_layout_xml_tests::Resource(content), errors);
				TEST_ASSERT(xml && errors.Count() == 0);
				auto resource = GuiResource::LoadFromXml(xml, L"TestControls_EasyLayout.xml", L".", errors);
				TEST_ASSERT(resource);
				if (errors.Count() == 0) PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
				for (auto error : errors) TEST_PRINT(error.message);
				TEST_ASSERT(errors.Count() == 0);
			}
		});

		LoadResource(L"Resource.NotExists.xml");
		LoadResource(L"Resource.WrongSyntax.xml");
		LoadResource(L"Resource.WrongSyntax2.xml");
		LoadResource(L"Resource.WrongDoc.xml");
		LoadResource(L"Resource.WrongInstanceStyle.xml");
		LoadResource(L"Resource.WrongInstance.xml");
		LoadResource(L"Resource.FailedInstance.Ctor1.xml");
		LoadResource(L"Resource.FailedInstance.Ctor2.xml");
		LoadResource(L"Resource.FailedInstance.Ctor2_r.xml");
		LoadResource(L"Resource.FailedInstance.Ctor3.xml");
		LoadResource(L"Resource.FailedInstance.Ctor4.xml");
		LoadResource(L"Resource.FailedInstance.Ctor5.xml");
		LoadResource(L"Resource.FailedInstance.Control.xml");
		LoadResource(L"Resource.FailedInstance.Inheriting1.xml");
		LoadResource(L"Resource.FailedInstance.Inheriting2.xml");
		LoadResource(L"Resource.FailedScript.Workflow.xml");
		LoadResource(L"Resource.FailedScript.Properties.xml");
		LoadResource(L"Resource.FailedScript.Animations.xml");
		LoadResource(L"Resource.FailedScript.Animations2.xml");
		LoadResource(L"Resource.FailedScript.Strings.xml");
		LoadResource(L"Resource.FailedScript.Strings2.xml");
		LoadResource(L"Resource.FailedScript.Strings3.xml");
		LoadResource(L"Resource.FailedScript.Strings4.xml");
	});
	SetupGacGenNativeController();
	SetGuiMainProxy({});
}
