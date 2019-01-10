#include "GuiInstanceLoader.h"
#include "InstanceQuery/GuiInstanceQuery.h"
#include "GuiInstanceSharedScript.h"
#include "GuiInstanceAnimation.h"
#include "GuiInstanceLocalizedStrings.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"
#include "../Reflection/GuiInstanceCompiledWorkflow.h"
#include "../Resources/GuiParserManager.h"
#include "../Resources/GuiResourceManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace parsing;
		using namespace parsing::xml;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace workflow::emitter;
		using namespace workflow::runtime;
		using namespace reflection::description;
		using namespace collections;
		using namespace stream;

		using namespace controls;

		class WorkflowVirtualScriptPositionVisitor : public traverse_visitor::ModuleVisitor
		{
		public:
			GuiResourcePrecompileContext&						context;
			Ptr<types::ScriptPosition>							sp;

			WorkflowVirtualScriptPositionVisitor(GuiResourcePrecompileContext& _context)
				:context(_context)
			{
				sp = Workflow_GetScriptPosition(context);
			}

			void Visit(WfVirtualCfeExpression* node)override
			{
				traverse_visitor::ExpressionVisitor::Visit(node);
				vint index = sp->nodePositions.Keys().IndexOf(node);
				if (index != -1)
				{
					auto record = sp->nodePositions.Values()[index];
					Workflow_RecordScriptPosition(context, record.position, node->expandedExpression, record.availableAfter);
				}
			}

			void Visit(WfVirtualCseExpression* node)override
			{
				traverse_visitor::ExpressionVisitor::Visit(node);
				vint index = sp->nodePositions.Keys().IndexOf(node);
				if (index != -1)
				{
					auto record = sp->nodePositions.Values()[index];
					Workflow_RecordScriptPosition(context, record.position, node->expandedExpression, record.availableAfter);
				}
			}

			void Visit(WfVirtualCseStatement* node)override
			{
				traverse_visitor::StatementVisitor::Visit(node);
				vint index = sp->nodePositions.Keys().IndexOf(node);
				if (index != -1)
				{
					auto record = sp->nodePositions.Values()[index];
					Workflow_RecordScriptPosition(context, record.position, node->expandedStatement, record.availableAfter);
				}
			}

			void Visit(WfVirtualCfeDeclaration* node)override
			{
				traverse_visitor::DeclarationVisitor::Visit(node);
				vint index = sp->nodePositions.Keys().IndexOf(node);
				if (index != -1)
				{
					auto record = sp->nodePositions.Values()[index];
					FOREACH(Ptr<WfDeclaration>, decl, node->expandedDeclarations)
					{
						Workflow_RecordScriptPosition(context, record.position, decl, record.availableAfter);
					}
				}
			}

			void Visit(WfVirtualCseDeclaration* node)override
			{
				traverse_visitor::DeclarationVisitor::Visit(node);
				vint index = sp->nodePositions.Keys().IndexOf(node);
				if (index != -1)
				{
					auto record = sp->nodePositions.Values()[index];
					FOREACH(Ptr<WfDeclaration>, decl, node->expandedDeclarations)
					{
						Workflow_RecordScriptPosition(context, record.position, decl, record.availableAfter);
					}
				}
			}
		};

		Ptr<GuiInstanceCompiledWorkflow> Workflow_GetModule(GuiResourcePrecompileContext& context, const WString& path, Nullable<GuiInstanceCompiledWorkflow::AssemblyType> assemblyType)
		{
			auto compiled = context.targetFolder->GetValueByPath(path).Cast<GuiInstanceCompiledWorkflow>();
			if (assemblyType && !compiled)
			{
				compiled = new GuiInstanceCompiledWorkflow;
				compiled->type = assemblyType.Value();
				context.targetFolder->CreateValueByPath(path, L"Workflow", compiled);
			}
			return compiled;
		}

		void Workflow_AddModule(GuiResourcePrecompileContext& context, const WString& path, Ptr<WfModule> module, GuiInstanceCompiledWorkflow::AssemblyType assemblyType, GuiResourceTextPos tagPosition)
		{
			auto compiled = Workflow_GetModule(context, path, assemblyType);
			CHECK_ERROR(compiled->type == assemblyType, L"Workflow_AddModule(GuiResourcePrecompiledContext&, const WString&, GuiInstanceCompiledWorkflow::AssemblyType)#Unexpected assembly type.");

			GuiInstanceCompiledWorkflow::ModuleRecord record;
			record.module = module;
			record.position = tagPosition;
			record.shared = assemblyType == GuiInstanceCompiledWorkflow::Shared;
			compiled->modules.Add(record);
		}

		void Workflow_GenerateAssembly(GuiResourcePrecompileContext& context, const WString& path, GuiResourceError::List& errors, bool keepMetadata, IWfCompilerCallback* compilerCallback)
		{
			auto compiled = Workflow_GetModule(context, path, {});
			if (!compiled)
			{
				return;
			}

			if (!compiled->assembly)
			{
				List<WString> codes;
				auto manager = Workflow_GetSharedManager();
				manager->Clear(false, true);

				auto addCode = [&codes](TextReader& reader)
				{
					vint row = 0;
					WString code;
					while (!reader.IsEnd())
					{
						auto rowHeader = itow(++row);
						while (rowHeader.Length() < 6)
						{
							rowHeader = L" " + rowHeader;
						}
						code += rowHeader + L" : " + reader.ReadLine() + L"\r\n";
					}
					codes.Add(code);
				};

				for (vint i = 0; i < compiled->modules.Count(); i++)
				{
					manager->AddModule(compiled->modules[i].module);
				}

				if (manager->errors.Count() == 0)
				{
					manager->Rebuild(true, compilerCallback);
				}

				if (manager->errors.Count() == 0)
				{
					compiled->assembly = GenerateAssembly(manager, compilerCallback);
					WfAssemblyLoadErrors loadErrors;
					if (!compiled->Initialize(true, loadErrors))
					{
						manager->errors.Add(new ParsingError(L"Internal error happened during loading an assembly that just passed type verification."));
					}
				}
				else
				{
					for (vint i = 0; i < compiled->modules.Count(); i++)
					{
						auto module = compiled->modules[i];
						WorkflowVirtualScriptPositionVisitor visitor(context);
						visitor.VisitField(module.module.Obj());
						Workflow_RecordScriptPosition(context, module.position, module.module);
					}

					auto sp = Workflow_GetScriptPosition(context);
					for (vint i = 0; i < manager->errors.Count(); i++)
					{
						auto error = manager->errors[i];
						errors.Add({ sp->nodePositions[error->parsingTree].computedPosition, error->errorMessage });
					}
				}

				if (keepMetadata)
				{
					compiled->metadata = Workflow_TransferSharedManager();
				}
				else
				{
					manager->Clear(false, true);
				}
			}
		}

/***********************************************************************
Shared Script Type Resolver (Script)
***********************************************************************/

#define Path_Shared				L"Workflow/Shared"
#define Path_TemporaryClass		L"Workflow/TemporaryClass"
#define Path_InstanceClass		L"Workflow/InstanceClass"

		class GuiResourceSharedScriptTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Precompile
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"Script";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return false;
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			vint GetMaxPassIndex()override
			{
				return Workflow_Max;
			}

			PassSupport GetPassSupport(vint passIndex)override
			{
				switch (passIndex)
				{
				case Workflow_Collect:
					return PerResource;
				case Workflow_Compile:
					return PerPass;
				default:
					return NotSupported;
				}
			}

			void PerResourcePrecompile(Ptr<GuiResourceItem> resource, GuiResourcePrecompileContext& context, GuiResourceError::List& errors)override
			{
				switch (context.passIndex)
				{
				case Workflow_Collect:
					{
						if (auto obj = resource->GetContent().Cast<GuiInstanceSharedScript>())
						{
							if (obj->language == L"Workflow")
							{
								if (auto module = Workflow_ParseModule(context, obj->codePosition.originalLocation, obj->code, obj->codePosition, errors))
								{
									Workflow_AddModule(context, Path_Shared, module, GuiInstanceCompiledWorkflow::Shared, obj->codePosition);
								}
							}
						}
					}
					break;
				}
			}

			void PerPassPrecompile(GuiResourcePrecompileContext& context, GuiResourceError::List& errors)override
			{
				switch (context.passIndex)
				{
				case Workflow_Compile:
					Workflow_GenerateAssembly(context, Path_Shared, errors, false, context.compilerCallback);
					if (auto compiled = Workflow_GetModule(context, Path_Shared, {}))
					{
						for (vint i = 0; i < compiled->modules.Count(); i++)
						{
							auto& module = compiled->modules[i];
							if (module.module)
							{
								module.module = CopyModule(module.module, true);
							}
						}
					}
					break;
				}
			}

			IGuiResourceTypeResolver_Precompile* Precompile()override
			{
				return this;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content)override
			{
				if (auto obj = content.Cast<GuiInstanceSharedScript>())
				{
					return obj->SaveToXml();
				}
				return nullptr;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, Ptr<GuiResourcePathResolver> resolver, GuiResourceError::List& errors)override
			{
				if (auto xml = resource->GetContent().Cast<XmlDocument>())
				{
					auto schema = GuiInstanceSharedScript::LoadFromXml(resource, xml, errors);
					return schema;
				}
				return nullptr;
			}
		};

/***********************************************************************
Instance Type Resolver (Instance)
***********************************************************************/

		class GuiResourceInstanceTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Precompile
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"Instance";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return false;
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			vint GetMaxPassIndex()override
			{
				return Instance_Max;
			}

			PassSupport GetPassSupport(vint passIndex)override
			{
				switch (passIndex)
				{
				case Workflow_Collect:
				case Instance_CollectInstanceTypes:
				case Instance_CollectEventHandlers:
				case Instance_GenerateInstanceClass:
					return PerResource;
				case Instance_CompileInstanceTypes:
				case Instance_CompileEventHandlers:
				case Instance_CompileInstanceClass:
					return PerPass;
				default:
					return NotSupported;
				}
			}

#define ENSURE_ASSEMBLY_EXISTS(PATH, ASSEMBLY_TYPE)\
			if (auto compiled = Workflow_GetModule(context, PATH, GuiInstanceCompiledWorkflow::ASSEMBLY_TYPE))\
			{\
				if (!compiled->assembly)\
				{\
					break;\
				}\
			}\
			else\
			{\
				break;\
			}\

#define UNLOAD_ASSEMBLY(PATH)\
			if (auto compiled = Workflow_GetModule(context, PATH, {}))\
			{\
				compiled->context = nullptr;\
			}\

#define DELETE_ASSEMBLY(PATH)\
			if (auto compiled = Workflow_GetModule(context, PATH, {}))\
			{\
				compiled->context = nullptr;\
				compiled->assembly = nullptr;\
			}\

			void PerResourcePrecompile(Ptr<GuiResourceItem> resource, GuiResourcePrecompileContext& context, GuiResourceError::List& errors)override
			{
				switch (context.passIndex)
				{
				case Workflow_Collect:
					{
						if (auto obj = resource->GetContent().Cast<GuiInstanceContext>())
						{
							auto record = context.targetFolder->GetValueByPath(L"ClassNameRecord").Cast<GuiResourceClassNameRecord>();
							if (!record)
							{
								record = MakePtr<GuiResourceClassNameRecord>();
								context.targetFolder->CreateValueByPath(L"ClassNameRecord", L"ClassNameRecord", record);
							}

							if (!record->classResources.Keys().Contains(obj->className))
							{
								record->classNames.Add(obj->className);
								record->classResources.Add(obj->className, resource);
							}
						}
					}
					break;
				case Instance_CollectEventHandlers:
					ENSURE_ASSEMBLY_EXISTS(Path_TemporaryClass, TemporaryClass)
				case Instance_CollectInstanceTypes:
					{
						if (auto obj = resource->GetContent().Cast<GuiInstanceContext>())
						{
							if (obj->className == L"")
							{
								errors.Add(GuiResourceError({ resource }, obj->tagPosition,
									L"Precompile: Instance \"" +
									(obj->instance->typeNamespace == GlobalStringKey::Empty
										? obj->instance->typeName.ToString()
										: obj->instance->typeNamespace.ToString() + L":" + obj->instance->typeName.ToString()
										) +
									L"\" should have the class name specified in the ref.Class attribute."));
							}

							FOREACH_INDEXER(Ptr<GuiInstanceLocalized>, localized, index,
								From(obj->localizeds).Where([](Ptr<GuiInstanceLocalized> ls) {return ls->defaultStrings; })
								)
							{
								if (index > 0)
								{
									errors.Add(GuiResourceError({ resource }, localized->tagPosition, L"Precompile: Only one <ref.LocalizedStrings> can be the default one."));
								}
							}

							obj->ApplyStyles(resource, context.resolver, errors);

							types::ResolvingResult resolvingResult;
							resolvingResult.resource = resource;
							resolvingResult.context = obj;
							if (auto module = Workflow_GenerateInstanceClass(context, L"<instance>" + obj->className, resolvingResult, errors, context.passIndex))
							{
								Workflow_AddModule(context, Path_TemporaryClass, module, GuiInstanceCompiledWorkflow::TemporaryClass, obj->tagPosition);
							}
						}
					}
					break;
				case Instance_GenerateInstanceClass:
					{
						ENSURE_ASSEMBLY_EXISTS(Path_TemporaryClass, TemporaryClass)
						if (auto obj = resource->GetContent().Cast<GuiInstanceContext>())
						{
							vint previousErrorCount = errors.Count();

							types::ResolvingResult resolvingResult;
							resolvingResult.resource = resource;
							resolvingResult.context = obj;
							resolvingResult.rootTypeInfo = Workflow_CollectReferences(context, resolvingResult, errors);

							if (errors.Count() == previousErrorCount)
							{
								if (auto ctorModule = Workflow_PrecompileInstanceContext(context, L"<constructor>" + obj->className, resolvingResult, errors))
								{
									if (auto instanceModule = Workflow_GenerateInstanceClass(context, L"<instance>" + obj->className, resolvingResult, errors, context.passIndex))
									{
										Workflow_AddModule(context, Path_InstanceClass, ctorModule, GuiInstanceCompiledWorkflow::InstanceClass, obj->tagPosition);
										Workflow_AddModule(context, Path_InstanceClass, instanceModule, GuiInstanceCompiledWorkflow::InstanceClass, obj->tagPosition);
									}
								}
							}
						}
					}
					break;
				}
			}

			void PerPassPrecompile(GuiResourcePrecompileContext& context, GuiResourceError::List& errors)override
			{
				WString path;
				GuiInstanceCompiledWorkflow::AssemblyType assemblyType;
				switch (context.passIndex)
				{
				case Instance_CompileInstanceTypes:
					DELETE_ASSEMBLY(Path_Shared)
					path = Path_TemporaryClass;
					assemblyType = GuiInstanceCompiledWorkflow::TemporaryClass;
					break;
				case Instance_CompileEventHandlers:
					DELETE_ASSEMBLY(Path_TemporaryClass)
					path = Path_TemporaryClass;
					assemblyType = GuiInstanceCompiledWorkflow::TemporaryClass;
					break;
				case Instance_CompileInstanceClass:
					UNLOAD_ASSEMBLY(Path_TemporaryClass)
					path = Path_InstanceClass;
					assemblyType = GuiInstanceCompiledWorkflow::InstanceClass;
					break;
				default:
					return;
				}

				auto sharedCompiled = Workflow_GetModule(context, Path_Shared, {});
				auto compiled = Workflow_GetModule(context, path, assemblyType);
				if (sharedCompiled && compiled)
				{
					CopyFrom(
						compiled->modules,
						From(sharedCompiled->modules)
							.Where([](const GuiInstanceCompiledWorkflow::ModuleRecord& module)
							{
								return module.shared;
							}),
						true
					);
				}

				switch (context.passIndex)
				{
				case Instance_CompileInstanceTypes:
					Workflow_GenerateAssembly(context, path, errors, false, context.compilerCallback);
					compiled->modules.Clear();
					break;
				case Instance_CompileEventHandlers:
					Workflow_GenerateAssembly(context, path, errors, false, context.compilerCallback);
					break;
				case Instance_CompileInstanceClass:
					Workflow_GenerateAssembly(context, path, errors, true, context.compilerCallback);
					break;
				default:;
				}
				Workflow_ClearScriptPosition(context);
				GetInstanceLoaderManager()->ClearReflectionCache();
			}

#undef DELETE_ASSEMBLY
#undef UNLOAD_ASSEMBLY
#undef ENSURE_ASSEMBLY_EXISTS

			IGuiResourceTypeResolver_Precompile* Precompile()override
			{
				return this;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content)override
			{
				if (auto obj = content.Cast<GuiInstanceContext>())
				{
					return obj->SaveToXml();
				}
				return nullptr;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, Ptr<GuiResourcePathResolver> resolver, GuiResourceError::List& errors)override
			{
				if (auto xml = resource->GetContent().Cast<XmlDocument>())
				{
					Ptr<GuiInstanceContext> context = GuiInstanceContext::LoadFromXml(resource, xml, errors);
					return context;
				}
				return nullptr;
			}
		};

/***********************************************************************
Instance Style Type Resolver (InstanceStyle)
***********************************************************************/

		class GuiResourceInstanceStyleTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"InstanceStyle";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return false;
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content)override
			{
				if (auto obj = content.Cast<GuiInstanceStyleContext>())
				{
					return obj->SaveToXml();
				}
				return nullptr;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, Ptr<GuiResourcePathResolver> resolver, GuiResourceError::List& errors)override
			{
				if (auto xml = resource->GetContent().Cast<XmlDocument>())
				{
					auto context = GuiInstanceStyleContext::LoadFromXml(resource, xml, errors);
					return context;
				}
				return nullptr;
			}
		};

/***********************************************************************
Animation Type Resolver (Animation)
***********************************************************************/

		class GuiResourceAnimationTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Precompile
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"Animation";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return false;
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			vint GetMaxPassIndex()override
			{
				return Instance_Max;
			}

			PassSupport GetPassSupport(vint passIndex)override
			{
				switch (passIndex)
				{
				case Instance_CollectInstanceTypes:
				case Instance_CollectEventHandlers:
				case Instance_GenerateInstanceClass:
					return PerResource;
				default:
					return NotSupported;
				}
			}

			void PerResourcePrecompile(Ptr<GuiResourceItem> resource, GuiResourcePrecompileContext& context, GuiResourceError::List& errors)override
			{
				bool generateImpl = true;
				auto path = Path_InstanceClass;
				auto assemblyType = GuiInstanceCompiledWorkflow::InstanceClass;

				switch (context.passIndex)
				{
				case Instance_CollectEventHandlers:
				case Instance_CollectInstanceTypes:
					generateImpl = false;
					path = Path_TemporaryClass;
					assemblyType = GuiInstanceCompiledWorkflow::TemporaryClass;
				case Instance_GenerateInstanceClass:
					{
						if (auto obj = resource->GetContent().Cast<GuiInstanceGradientAnimation>())
						{
							if (auto module = obj->Compile(context, L"<animation>" + obj->className, generateImpl, errors))
							{
								Workflow_AddModule(context, path, module, assemblyType, obj->tagPosition);
							}
						}
					}
					break;
				}
			}

			void PerPassPrecompile(GuiResourcePrecompileContext& context, GuiResourceError::List& errors)override
			{
				CHECK_FAIL(L"GuiResourceAnimationTypeResolver::PerPassPrecompile(GuiResourcePrecompileContext&, GuiResourceError::List&)#This function should not be called.");
			}

			IGuiResourceTypeResolver_Precompile* Precompile()override
			{
				return this;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content)override
			{
				if (auto obj = content.Cast<GuiInstanceGradientAnimation>())
				{
					return obj->SaveToXml();
				}
				return nullptr;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, Ptr<GuiResourcePathResolver> resolver, GuiResourceError::List& errors)override
			{
				if (auto xml = resource->GetContent().Cast<XmlDocument>())
				{
					if (xml->rootElement->name.value == L"Gradient")
					{
						return GuiInstanceGradientAnimation::LoadFromXml(resource, xml, errors);
					}
					else
					{
						errors.Add({
							{ {resource }, xml->rootElement->codeRange.start },
							L"Precompile: Unknown animation type: \"" + xml->rootElement->name.value + L"\"."
							});
					}
				}
				return nullptr;
			}
		};

/***********************************************************************
Localized Strings Type Resolver (LocalizedStrings)
***********************************************************************/

		class GuiResourceLocalizedStringsTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Precompile
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"LocalizedStrings";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return false;
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			vint GetMaxPassIndex()override
			{
				return Workflow_Collect + 1;
			}

			PassSupport GetPassSupport(vint passIndex)override
			{
				switch (passIndex)
				{
				case Workflow_Collect:
					return PerResource;
				default:
					return NotSupported;
				}
			}

			void PerResourcePrecompile(Ptr<GuiResourceItem> resource, GuiResourcePrecompileContext& context, GuiResourceError::List& errors)override
			{
				switch (context.passIndex)
				{
				case Workflow_Collect:
					{
						if (auto obj = resource->GetContent().Cast<GuiInstanceLocalizedStrings>())
						{
							if (auto module = obj->Compile(context, L"<localized-strings>" + obj->className, errors))
							{
								Workflow_AddModule(context, Path_Shared, module, GuiInstanceCompiledWorkflow::Shared, obj->tagPosition);
							}
						}
					}
					break;
				}
			}

			void PerPassPrecompile(GuiResourcePrecompileContext& context, GuiResourceError::List& errors)override
			{
				CHECK_FAIL(L"GuiResourceLocalizedStringsTypeResolver::PerPassPrecompile(GuiResourcePrecompileContext&, GuiResourceError::List&)#This function should not be called.");
			}

			IGuiResourceTypeResolver_Precompile* Precompile()override
			{
				return this;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content)override
			{
				if (auto obj = content.Cast<GuiInstanceLocalizedStrings>())
				{
					return obj->SaveToXml();
				}
				return nullptr;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, Ptr<GuiResourcePathResolver> resolver, GuiResourceError::List& errors)override
			{
				if (auto xml = resource->GetContent().Cast<XmlDocument>())
				{
					return GuiInstanceLocalizedStrings::LoadFromXml(resource, xml, errors);
				}
				return nullptr;
			}
		};

#undef Path_Shared
#undef Path_TemporaryClass
#undef Path_InstanceClass

/***********************************************************************
Plugin
***********************************************************************/

		class GuiCompilerTypeResolversPlugin : public Object, public IGuiPlugin
		{
		public:

			GUI_PLUGIN_NAME(GacUI_Compiler_InstanceTypeResolvers)
			{
				GUI_PLUGIN_DEPEND(GacUI_Res_ResourceResolver);
			}

			void Load()override
			{
				IGuiResourceResolverManager* manager = GetResourceResolverManager();
				manager->SetTypeResolver(new GuiResourceSharedScriptTypeResolver);
				manager->SetTypeResolver(new GuiResourceInstanceTypeResolver);
				manager->SetTypeResolver(new GuiResourceInstanceStyleTypeResolver);
				manager->SetTypeResolver(new GuiResourceAnimationTypeResolver);
				manager->SetTypeResolver(new GuiResourceLocalizedStringsTypeResolver);
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiCompilerTypeResolversPlugin)
	}
}
