#include "GuiInstanceLoader.h"
#include "GuiInstanceLoader_WorkflowCompiler.h"
#include "../Controls/GuiApplication.h"
#include "../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace reflection::description;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		using namespace controls;

/***********************************************************************
GuiTextInstanceBinderBase
***********************************************************************/

		class GuiTextInstanceBinderBase : public Object, public IGuiInstanceBinder
		{
		protected:
			ITypeDescriptor*				stringTypeDescriptor;
		public:
			GuiTextInstanceBinderBase()
				:stringTypeDescriptor(description::GetTypeDescriptor<WString>())
			{
			}

			bool ApplicableToConstructorArgument()override
			{
				return false;
			}

			bool RequireInstanceName()override
			{
				return false;
			}

			void GetRequiredContexts(collections::List<GlobalStringKey>& contextNames)override
			{
			}

			void GetExpectedValueTypes(collections::List<description::ITypeDescriptor*>& expectedTypes)override
			{
				expectedTypes.Add(stringTypeDescriptor);
			}

			description::Value GetValue(Ptr<GuiInstanceEnvironment> env, const description::Value& propertyValue)override
			{
				return Value();
			}
		};

/***********************************************************************
GuiResourceInstanceBinder
***********************************************************************/

		class GuiResourceInstanceBinder : public GuiTextInstanceBinderBase
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Uri;
			}

			bool SetPropertyValue(Ptr<GuiInstanceEnvironment> env, IGuiInstanceLoader* loader, GlobalStringKey instanceName, IGuiInstanceLoader::PropertyValue& propertyValue)override
			{
				if (propertyValue.propertyValue.GetValueType() == Value::Text)
				{
					WString protocol, path;
					if (IsResourceUrl(propertyValue.propertyValue.GetText(), protocol, path))
					{
						if(Ptr<DescriptableObject> resource=env->resolver->ResolveResource(protocol, path))
						{
							Value value;
							if(Ptr<GuiTextData> text=resource.Cast<GuiTextData>())
							{
								value=Value::From(text->GetText(), stringTypeDescriptor);
							}
							else if(Ptr<DescriptableObject> obj=resource.Cast<DescriptableObject>())
							{
								if (auto image = obj.Cast<GuiImageData>())
								{
									auto td = propertyValue.typeInfo.typeDescriptor;
									if (auto prop = td->GetPropertyByName(propertyValue.propertyName.ToString(), true))
									{
										if (prop->GetReturn() && prop->GetReturn()->GetTypeDescriptor()->GetTypeName() == L"presentation::INativeImage")
										{
											obj = image->GetImage();
										}
									}
								}
								value = Value::From(obj);
							}

							if(!value.IsNull())
							{
								IGuiInstanceLoader::PropertyValue newValue = propertyValue;
								newValue.propertyValue = value;
								return loader->SetPropertyValue(newValue);
							}
						}
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiReferenceInstanceBinder
***********************************************************************/

		class GuiReferenceInstanceBinder : public GuiTextInstanceBinderBase
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Ref;
			}

			bool SetPropertyValue(Ptr<GuiInstanceEnvironment> env, IGuiInstanceLoader* loader, GlobalStringKey instanceName, IGuiInstanceLoader::PropertyValue& propertyValue)override
			{
				if (propertyValue.propertyValue.GetValueType() == Value::Text)
				{
					GlobalStringKey name = GlobalStringKey::Get(propertyValue.propertyValue.GetText());
					vint index = env->scope->referenceValues.Keys().IndexOf(name);
					if (index != -1)
					{
						IGuiInstanceLoader::PropertyValue newValue = propertyValue;
						newValue.propertyValue = env->scope->referenceValues.Values()[index];
						if (!newValue.propertyValue.IsNull())
						{
							return loader->SetPropertyValue(newValue);
						}
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiWorkflowGlobalContext
***********************************************************************/

		class GuiWorkflowGlobalContext : public Object, public IGuiInstanceBindingContext
		{
		public:
			List<WorkflowDataBinding>		dataBindings;
			Ptr<WfRuntimeGlobalContext>		globalContext;

			GuiWorkflowGlobalContext()
			{
			}

			GlobalStringKey GetContextName()override
			{
				return GuiWorkflowCache::CacheContextName;
			}

			void Initialize(Ptr<GuiInstanceEnvironment> env)override
			{
				Ptr<WfAssembly> assembly;
				vint cacheIndex = env->context->precompiledCaches.Keys().IndexOf(GetContextName());
				if (cacheIndex != -1)
				{
					assembly = env->context->precompiledCaches.Values()[cacheIndex].Cast<GuiWorkflowCache>()->assembly;
				}
				else
				{
					types::VariableTypeMap types;
					ITypeDescriptor* thisType = env->scope->rootInstance.GetTypeDescriptor();
					Workflow_GetVariableTypes(env, types);
					assembly = Workflow_CompileDataBinding(env->context, types, thisType, env->scope->errors, dataBindings);
					env->context->precompiledCaches.Add(GetContextName(), new GuiWorkflowCache(assembly));
				}

				if (assembly)
				{
					globalContext = new WfRuntimeGlobalContext(assembly);
				
					try
					{
						LoadFunction<void()>(globalContext, L"<initialize>")();
					}
					catch (const TypeDescriptorException& ex)
					{
						env->scope->errors.Add(L"Workflow Script Exception: " + ex.Message());
					}

					Workflow_SetVariablesForReferenceValues(globalContext, env);
					{
						vint index = assembly->variableNames.IndexOf(L"<this>");
						globalContext->globalVariables->variables[index] = env->scope->rootInstance;
					}

					try
					{
						LoadFunction<void()>(globalContext, L"<initialize-data-binding>")();
					}
					catch (const TypeDescriptorException& ex)
					{
						env->scope->errors.Add(L"Workflow Script Exception: " + ex.Message());
					}
				}
			}
		};

/***********************************************************************
GuiScriptInstanceBinder
***********************************************************************/

		class GuiScriptInstanceBinder : public GuiTextInstanceBinderBase
		{
		public:
			virtual WString TranslateExpression(const WString& input) = 0;

			bool RequireInstanceName()override
			{
				return true;
			}

			void GetRequiredContexts(collections::List<GlobalStringKey>& contextNames)override
			{
				contextNames.Add(GuiWorkflowCache::CacheContextName);
			}

			bool SetPropertyValue(Ptr<GuiInstanceEnvironment> env, IGuiInstanceLoader* loader, GlobalStringKey instanceName, IGuiInstanceLoader::PropertyValue& propertyValue)override
			{
				auto context = env->scope->bindingContexts[GuiWorkflowCache::CacheContextName].Cast<GuiWorkflowGlobalContext>();
				WorkflowDataBinding dataBinding;
				dataBinding.variableName = instanceName;

				if (env->context->precompiledCaches.Keys().Contains(GuiWorkflowCache::CacheContextName))
				{
					goto SUCCESS;
				}
				if (propertyValue.propertyValue.GetValueType() == Value::Text)
				{
					WString expressionCode = TranslateExpression(propertyValue.propertyValue.GetText());
					Ptr<WfExpression> expression;
					types::VariableTypeMap types;
					Workflow_GetVariableTypes(env, types);
					if (Workflow_ValidateExpression(env->context, types, env->scope->errors, propertyValue, expressionCode, expression))
					{
						auto expr = expression;
						if (auto bind = expr.Cast<WfBindExpression>())
						{
							bind->expandedExpression = 0;
							expr = bind->expression;
						}
						if (auto format = expr.Cast<WfFormatExpression>())
						{
							format->expandedExpression = 0;
						}
						
						auto td = propertyValue.typeInfo.typeDescriptor;
						auto propertyInfo = td->GetPropertyByName(propertyValue.propertyName.ToString(), true);
						dataBinding.propertyInfo = propertyInfo;
						dataBinding.bindExpression = expression;
						goto SUCCESS;
					}
					else
					{
						goto FAILED;
					}
				}

			FAILED:
				context->dataBindings.Add(dataBinding);
				return false;
			SUCCESS:
				context->dataBindings.Add(dataBinding);
				return true;
			}
		};

/***********************************************************************
GuiEvalInstanceBinder
***********************************************************************/

		class GuiEvalInstanceBinder : public GuiScriptInstanceBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Eval;
			}

			bool ApplicableToConstructorArgument()override
			{
				return true;
			}

			description::Value GetValue(Ptr<GuiInstanceEnvironment> env, const description::Value& propertyValue)override
			{
				if (propertyValue.GetValueType() == Value::Text)
				{
					Ptr<WfAssembly> assembly;
					WString expressionCode = TranslateExpression(propertyValue.GetText());
					GlobalStringKey cacheKey = GlobalStringKey::Get(L"<att.eval>" + expressionCode);
					vint cacheIndex = env->context->precompiledCaches.Keys().IndexOf(cacheKey);
					if (cacheIndex != -1)
					{
						assembly = env->context->precompiledCaches.Values()[cacheIndex].Cast<GuiWorkflowCache>()->assembly;
					}
					else
					{
						types::VariableTypeMap types;
						Workflow_GetVariableTypes(env, types);
						assembly = Workflow_CompileExpression(env->context, types, env->scope->errors, expressionCode);
						env->context->precompiledCaches.Add(cacheKey, new GuiWorkflowCache(assembly));
					}

					if (assembly)
					{
						auto globalContext = MakePtr<WfRuntimeGlobalContext>(assembly);
				
						try
						{
							LoadFunction<void()>(globalContext, L"<initialize>")();
						}
						catch (const TypeDescriptorException& ex)
						{
							env->scope->errors.Add(L"Workflow Script Exception: " + ex.Message());
						}

						Workflow_SetVariablesForReferenceValues(globalContext, env);
						vint variableIndex = assembly->variableNames.IndexOf(L"<initialize-data-binding>");
						auto variable = globalContext->globalVariables->variables[variableIndex];
						auto proxy = UnboxValue<Ptr<IValueFunctionProxy>>(variable);

						Value translated;
						try
						{
							translated = proxy->Invoke(IValueList::Create());
						}
						catch (const TypeDescriptorException& ex)
						{
							env->scope->errors.Add(L"Workflow Script Exception: " + ex.Message());
						}

						// the global context contains a closure variable <initialize-data-binding> which captured the context
						// clear all variables to break the circle references
						globalContext->globalVariables = 0;
						return translated;
					}
				}
				return Value();
			}

			WString TranslateExpression(const WString& input)override
			{
				return input;
			}
		};

/***********************************************************************
GuiEvalInstanceEventBinder
***********************************************************************/

		class GuiEvalInstanceEventBinder : public Object, public IGuiInstanceEventBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Eval;
			}

			bool RequireInstanceName()override
			{
				return true;
			}

			void GetRequiredContexts(collections::List<GlobalStringKey>& contextNames)override
			{
			}

			bool AttachEvent(Ptr<GuiInstanceEnvironment> env, IGuiInstanceLoader* loader, GlobalStringKey instanceName, IGuiInstanceLoader::PropertyValue& propertyValue)
			{
				auto handler = propertyValue.propertyValue;
				if (handler.GetValueType() == Value::Text)
				{
					Ptr<WfAssembly> assembly;
					WString statementCode = handler.GetText();
					GlobalStringKey cacheKey = GlobalStringKey::Get(L"<ev.eval><" + instanceName.ToString() + L"><" + propertyValue.propertyName.ToString() + L">" + statementCode);
					vint cacheIndex = env->context->precompiledCaches.Keys().IndexOf(cacheKey);
					if (cacheIndex != -1)
					{
						assembly = env->context->precompiledCaches.Values()[cacheIndex].Cast<GuiWorkflowCache>()->assembly;
					}
					else
					{
						types::VariableTypeMap types;
						Workflow_GetVariableTypes(env, types);
						assembly = Workflow_CompileEventHandler(env->context, types, env->scope->errors, propertyValue, statementCode);
						env->context->precompiledCaches.Add(cacheKey, new GuiWorkflowCache(assembly));
					}

					if (assembly)
					{
						auto globalContext = MakePtr<WfRuntimeGlobalContext>(assembly);
				
						try
						{
							LoadFunction<void()>(globalContext, L"<initialize>")();
						}
						catch (const TypeDescriptorException& ex)
						{
							env->scope->errors.Add(L"Workflow Script Exception: " + ex.Message());
						}

						Workflow_SetVariablesForReferenceValues(globalContext, env);
						auto eventHandler = LoadFunction(globalContext, L"<event-handler>");
						handler = BoxValue(eventHandler);

						propertyValue.propertyValue = handler;
						return loader->SetEventValue(propertyValue);
					}
				}
				return false;
			}
		};

/***********************************************************************
GuiBindInstanceBinder
***********************************************************************/

		class GuiBindInstanceBinder : public GuiScriptInstanceBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Bind;
			}

			WString TranslateExpression(const WString& input)override
			{
				return L"bind(" + input + L")";
			}
		};

/***********************************************************************
GuiFormatInstanceBinder
***********************************************************************/

		class GuiFormatInstanceBinder : public GuiScriptInstanceBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Format;
			}

			WString TranslateExpression(const WString& input)override
			{
				return L"bind($\"" + input + L"\")";
			}
		};

/***********************************************************************
GuiPredefinedInstanceBindersPlugin
***********************************************************************/

		class GuiPredefinedInstanceBindersPlugin : public Object, public IGuiPlugin
		{
		public:
			GuiPredefinedInstanceBindersPlugin()
			{
			}

			void Load()override
			{
				WfLoadTypes();
				GuiIqLoadTypes();
			}

			void AfterLoad()override
			{
				{
					IGuiParserManager* manager = GetParserManager();
					manager->SetParsingTable(L"WORKFLOW", &WfLoadTable);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-EXPRESSION", &WfParseExpression);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-STATEMENT", &WfParseStatement);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-MODULE", &WfParseModule);
					manager->SetParsingTable(L"INSTANCE-QUERY", &GuiIqLoadTable);
					manager->SetTableParser(L"INSTANCE-QUERY", L"INSTANCE-QUERY", &GuiIqParse);
				}
				{
					IGuiInstanceLoaderManager* manager=GetInstanceLoaderManager();

					manager->AddInstanceBindingContextFactory(new GuiInstanceBindingContextFactory<GuiWorkflowGlobalContext>(GuiWorkflowCache::CacheContextName));

					manager->AddInstanceBinder(new GuiResourceInstanceBinder);
					manager->AddInstanceBinder(new GuiReferenceInstanceBinder);
					manager->AddInstanceBinder(new GuiEvalInstanceBinder);
					manager->AddInstanceEventBinder(new GuiEvalInstanceEventBinder);
					manager->AddInstanceBinder(new GuiBindInstanceBinder);
					manager->AddInstanceBinder(new GuiFormatInstanceBinder);
				}
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedInstanceBindersPlugin)
	}
}