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

			bool RequirePrecompile()override
			{
				return false;
			}

			void GetExpectedValueTypes(collections::List<description::ITypeDescriptor*>& expectedTypes)override
			{
				expectedTypes.Add(stringTypeDescriptor);
			}

			Ptr<workflow::WfStatement> GenerateInstallStatement(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, const WString& code, collections::List<WString>& errors)override
			{
				return 0;
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
GuiEvalInstanceBinder
***********************************************************************/

		class GuiEvalInstanceBinder : public GuiTextInstanceBinderBase
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

			bool RequirePrecompile()override
			{
				return true;
			}

			bool SetPropertyValue(Ptr<GuiInstanceEnvironment> env, IGuiInstanceLoader* loader, GlobalStringKey instanceName, IGuiInstanceLoader::PropertyValue& propertyValue)override
			{
				CHECK_ERROR(env->context->precompiledScript, L"Instance with -eval property binder should be precompiled.");
				return true;
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, const WString& code, collections::List<WString>& errors)override
			{
				if (auto expression = Workflow_ParseExpression(code, errors))
				{
					return Workflow_InstallEvalProperty(variableName, propertyInfo, expression);
				}
				return 0;
			}
		};

/***********************************************************************
GuiBindInstanceBinder
***********************************************************************/

		class GuiBindInstanceBinder : public GuiTextInstanceBinderBase
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Bind;
			}

			bool RequirePrecompile()override
			{
				return true;
			}

			bool SetPropertyValue(Ptr<GuiInstanceEnvironment> env, IGuiInstanceLoader* loader, GlobalStringKey instanceName, IGuiInstanceLoader::PropertyValue& propertyValue)override
			{
				CHECK_ERROR(env->context->precompiledScript, L"Instance with -bind property binder should be precompiled.");
				return true;
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, const WString& code, collections::List<WString>& errors)override
			{
				if (auto expression = Workflow_ParseExpression(L"bind(" + code + L")", errors))
				{
					return Workflow_InstallEvalProperty(variableName, propertyInfo, expression);
				}
				return 0;
			}
		};

/***********************************************************************
GuiFormatInstanceBinder
***********************************************************************/

		class GuiFormatInstanceBinder : public GuiTextInstanceBinderBase
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Format;
			}

			bool RequirePrecompile()override
			{
				return true;
			}

			bool SetPropertyValue(Ptr<GuiInstanceEnvironment> env, IGuiInstanceLoader* loader, GlobalStringKey instanceName, IGuiInstanceLoader::PropertyValue& propertyValue)override
			{
				CHECK_ERROR(env->context->precompiledScript, L"Instance with -format property binder should be precompiled.");
				return true;
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, const WString& code, collections::List<WString>& errors)override
			{
				if (auto expression = Workflow_ParseExpression(L"bind($\"" + code + L"\")", errors))
				{
					return Workflow_InstallEvalProperty(variableName, propertyInfo, expression);
				}
				return 0;
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

			bool RequirePrecompile()override
			{
				return true;
			}

			bool AttachEvent(Ptr<GuiInstanceEnvironment> env, IGuiInstanceLoader* loader, GlobalStringKey instanceName, IGuiInstanceLoader::PropertyValue& propertyValue)
			{
				CHECK_ERROR(env->context->precompiledScript, L"Instance with -eval event binder should be precompiled.");
				return true;
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GlobalStringKey variableName, description::IEventInfo* eventInfo, const WString& code, collections::List<WString>& errors)
			{
				if (auto statement = Workflow_ParseStatement(code, errors))
				{
					return Workflow_InstallEvalEvent(variableName, eventInfo, statement);
				}
				return 0;
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