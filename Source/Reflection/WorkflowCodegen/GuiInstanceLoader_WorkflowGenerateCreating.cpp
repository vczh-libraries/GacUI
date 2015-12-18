#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../TypeDescriptors/GuiReflectionEvents.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace workflow;
		using namespace collections;

/***********************************************************************
WorkflowGenerateCreatingVisitor
***********************************************************************/

		class WorkflowGenerateCreatingVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			Ptr<GuiInstanceContext>				context;
			types::VariableTypeInfoMap&			typeInfos;
			description::ITypeDescriptor*		rootTypeDescriptor;
			Ptr<WfBlockStatement>				statements;
			types::ErrorList&					errors;
			
			WorkflowGenerateCreatingVisitor(Ptr<GuiInstanceContext> _context, types::VariableTypeInfoMap& _typeInfos, description::ITypeDescriptor* _rootTypeDescriptor, Ptr<WfBlockStatement> _statements, types::ErrorList& _errors)
				:context(_context)
				, typeInfos(_typeInfos)
				, rootTypeDescriptor(_rootTypeDescriptor)
				, errors(_errors)
				, statements(_statements)
			{
			}

			void Visit(GuiTextRepr* repr)override
			{
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				IGuiInstanceLoader::TypeInfo reprTypeInfo;
				if (repr->instanceName != GlobalStringKey::Empty)
				{
					reprTypeInfo = typeInfos[repr->instanceName];
				}

				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
				{
					if (reprTypeInfo.typeDescriptor)
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
					}

					FOREACH(Ptr<GuiValueRepr>, value, setter->values)
					{
						value->Accept(this);
					}
				}
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				Visit((GuiAttSetterRepr*)repr);
			}
		};

		void Workflow_GenerateCreating(Ptr<GuiInstanceContext> context, types::VariableTypeInfoMap& typeInfos, description::ITypeDescriptor* rootTypeDescriptor, Ptr<WfBlockStatement> statements, types::ErrorList& errors)
		{
			WorkflowGenerateCreatingVisitor visitor(context, typeInfos, rootTypeDescriptor, statements, errors);
			context->instance->Accept(&visitor);
		}
	}
}