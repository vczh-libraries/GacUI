#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../TypeDescriptors/GuiReflectionEvents.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace reflection::description;

/***********************************************************************
WorkflowGenerateCreatingVisitor
***********************************************************************/

		class WorkflowGenerateCreatingVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			Ptr<GuiInstanceContext>				context;
			types::ResolvingResult&				resolvingResult;
			description::ITypeDescriptor*		rootTypeDescriptor;
			Ptr<WfBlockStatement>				statements;
			types::ErrorList&					errors;
			
			WorkflowGenerateCreatingVisitor(Ptr<GuiInstanceContext> _context, types::ResolvingResult& _resolvingResult, description::ITypeDescriptor* _rootTypeDescriptor, Ptr<WfBlockStatement> _statements, types::ErrorList& _errors)
				:context(_context)
				, resolvingResult(_resolvingResult)
				, rootTypeDescriptor(_rootTypeDescriptor)
				, errors(_errors)
				, statements(_statements)
			{
			}

			Ptr<WfExpression> GetValueExpression(GuiValueRepr* repr)
			{
				ITypeDescriptor* td = nullptr;
				WString textValue;
				GuiConstructorRepr* ctor = nullptr;

				if (auto text = dynamic_cast<GuiTextRepr*>(repr))
				{
					td = resolvingResult.propertyResolvings[repr].info->acceptableTypes[0];
					textValue = text->text;
				}
				else if (ctor = dynamic_cast<GuiConstructorRepr*>(repr))
				{
					td = resolvingResult.typeInfos[ctor->instanceName].typeDescriptor;
					if (td->GetValueSerializer() != nullptr)
					{
						textValue = ctor->setters.Values()[0]->values[0].Cast<GuiTextRepr>()->text;
					}
					else
					{
						td = nullptr;
					}
				}

				if (td)
				{
					if (td == description::GetTypeDescriptor<WString>())
					{
						auto str = MakePtr<WfStringExpression>();
						str->value.value = textValue;
						return str;
					}
					else
					{
						auto str = MakePtr<WfStringExpression>();
						str->value.value = textValue;

						auto type = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
						type->SetTypeDescriptor(td);

						auto cast = MakePtr<WfTypeCastingExpression>();
						cast->type = GetTypeFromTypeInfo(type.Obj());
						cast->strategy = WfTypeCastingStrategy::Strong;
						cast->expression = str;

						return cast;
					}
				}
				else
				{
					repr->Accept(this);

					auto ref = MakePtr<WfReferenceExpression>();
					ref->name.value = ctor->instanceName.ToString();
					return ref;
				}
			}

			void Visit(GuiTextRepr* repr)override
			{
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				IGuiInstanceLoader::TypeInfo reprTypeInfo;
				if (repr->instanceName != GlobalStringKey::Empty)
				{
					reprTypeInfo = resolvingResult.typeInfos[repr->instanceName];
				}
				
				if (reprTypeInfo.typeDescriptor && reprTypeInfo.typeDescriptor->GetValueSerializer() == nullptr)
				{
					FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
					{
						GlobalStringKey propertyName = repr->setters.Keys()[index];
						Ptr<GuiInstancePropertyInfo> propertyInfo;

						{
							IGuiInstanceLoader::PropertyInfo info;
							info.typeInfo = reprTypeInfo;
							info.propertyName = propertyName;
							auto currentLoader = GetInstanceLoaderManager()->GetLoader(info.typeInfo.typeName);

							while (currentLoader && !propertyInfo)
							{
								propertyInfo = currentLoader->GetPropertyType(info);
								if (propertyInfo && propertyInfo->support == GuiInstancePropertyInfo::NotSupport)
								{
									propertyInfo = 0;
								}
								currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
							}
						}

						if (!propertyInfo)
						{
							errors.Add(L"Precompile: Cannot find property \"" + propertyName.ToString() + L"\" in type \"" + reprTypeInfo.typeName.ToString() + L"\".");
						}
						else if (setter->binding == GlobalStringKey::Empty)
						{
						}
						else if (setter->binding == GlobalStringKey::_Set)
						{
						}

						FOREACH(Ptr<GuiValueRepr>, value, setter->values)
						{
							value->Accept(this);
						}
					}
				}
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				Visit((GuiAttSetterRepr*)repr);
			}
		};

		void Workflow_GenerateCreating(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, description::ITypeDescriptor* rootTypeDescriptor, Ptr<WfBlockStatement> statements, types::ErrorList& errors)
		{
			WorkflowGenerateCreatingVisitor visitor(context, resolvingResult, rootTypeDescriptor, statements, errors);
			context->instance->Accept(&visitor);
		}
	}
}