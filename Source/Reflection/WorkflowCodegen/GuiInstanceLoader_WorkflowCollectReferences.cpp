#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../TypeDescriptors/GuiReflectionEvents.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace reflection::description;
		using namespace collections;

/***********************************************************************
WorkflowReferenceNamesVisitor
***********************************************************************/

		class WorkflowReferenceNamesVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			Ptr<GuiInstanceContext>				context;
			types::ResolvingResult&				resolvingResult;
			types::ErrorList&					errors;

			IGuiInstanceLoader::TypeInfo		bindingTargetTypeInfo;
			vint								generatedNameCount;
			ITypeDescriptor*					rootTypeDescriptor;

			WorkflowReferenceNamesVisitor(Ptr<GuiInstanceContext> _context, types::ResolvingResult& _resolvingResult, types::ErrorList& _errors)
				:context(_context)
				, resolvingResult(_resolvingResult)
				, errors(_errors)
				, generatedNameCount(0)
				, rootTypeDescriptor(0)
			{
			}

			void Visit(GuiTextRepr* repr)override
			{
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				auto reprTypeInfo = bindingTargetTypeInfo;
				auto loader = GetInstanceLoaderManager()->GetLoader(reprTypeInfo.typeName);

				if (repr->instanceName != GlobalStringKey::Empty && reprTypeInfo.typeDescriptor)
				{
					if (resolvingResult.typeInfos.Keys().Contains(repr->instanceName))
					{
						errors.Add(L"Precompile: Parameter \"" + repr->instanceName.ToString() + L"\" conflict with an existing named object.");
					}
					else
					{
						resolvingResult.typeInfos.Add(repr->instanceName, reprTypeInfo);
					}
				}
				
				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
				{
					IGuiInstanceLoader::TypeInfo propertyTypeInfo;

					if (setter->binding != GlobalStringKey::Empty && setter->binding != GlobalStringKey::_Set)
					{
						auto binder = GetInstanceLoaderManager()->GetInstanceBinder(setter->binding);
						if (!binder)
						{
							errors.Add(L"The appropriate IGuiInstanceBinder of binding \"" + setter->binding.ToString() + L"\" cannot be found.");
						}
						else if (repr->instanceName == GlobalStringKey::Empty && reprTypeInfo.typeDescriptor)
						{
							auto name = GlobalStringKey::Get(L"<precompile>" + itow(generatedNameCount++));
							repr->instanceName = name;
							resolvingResult.typeInfos.Add(name, reprTypeInfo);
						}
					}

					if (setter->binding == GlobalStringKey::_Set)
					{
						IGuiInstanceLoader::PropertyInfo info;
						info.typeInfo = reprTypeInfo;
						info.propertyName = repr->setters.Keys()[index];
						auto currentLoader = loader;

						while (currentLoader)
						{
							auto typeInfo = currentLoader->GetPropertyType(info);
							if (typeInfo && typeInfo->support != GuiInstancePropertyInfo::NotSupport)
							{
								propertyTypeInfo.typeDescriptor = typeInfo->acceptableTypes[0];
								propertyTypeInfo.typeName = GlobalStringKey::Get(typeInfo->acceptableTypes[0]->GetTypeName());
								break;
							}
							currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
						}
					}

					FOREACH(Ptr<GuiValueRepr>, value, setter->values)
					{
						bindingTargetTypeInfo = propertyTypeInfo;
						value->Accept(this);
					}
				}

				FOREACH(Ptr<GuiAttSetterRepr::EventValue>, handler, repr->eventHandlers.Values())
				{
					if (handler->binding != GlobalStringKey::Empty)
					{
						auto binder = GetInstanceLoaderManager()->GetInstanceEventBinder(handler->binding);
						if (!binder)
						{
							errors.Add(L"The appropriate IGuiInstanceEventBinder of binding \"" + handler->binding.ToString() + L"\" cannot be found.");
						}
					}

					if (repr->instanceName == GlobalStringKey::Empty && reprTypeInfo.typeDescriptor)
					{
						auto name = GlobalStringKey::Get(L"<precompile>" + itow(generatedNameCount++));
						repr->instanceName = name;
						resolvingResult.typeInfos.Add(name, reprTypeInfo);
					}
				}
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				bool found = false;

				if (repr == context->instance.Obj())
				{
					auto fullName = GlobalStringKey::Get(context->className);
					auto td = GetInstanceLoaderManager()->GetTypeDescriptorForType(fullName);
					if (td)
					{
						found = true;
						bindingTargetTypeInfo.typeName = fullName;
						bindingTargetTypeInfo.typeDescriptor = td;
					}
				}

				if (!found)
				{
					auto source = FindInstanceLoadingSource(context, repr);
					bindingTargetTypeInfo.typeName = source.typeName;
					bindingTargetTypeInfo.typeDescriptor = GetInstanceLoaderManager()->GetTypeDescriptorForType(source.typeName);
				}

				if (!bindingTargetTypeInfo.typeDescriptor)
				{
					errors.Add(
						L"Precompile: Failed to find type \"" +
						(repr->typeNamespace == GlobalStringKey::Empty
							? repr->typeName.ToString()
							: repr->typeNamespace.ToString() + L":" + repr->typeName.ToString()
							) +
						L"\".");
				}

				if (context->instance.Obj() == repr)
				{
					rootTypeDescriptor = bindingTargetTypeInfo.typeDescriptor;
				}
				Visit((GuiAttSetterRepr*)repr);
			}
		};

		ITypeDescriptor* Workflow_CollectReferences(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, types::ErrorList& errors)
		{
			WorkflowReferenceNamesVisitor visitor(context, resolvingResult, errors);
			context->instance->Accept(&visitor);
			return visitor.rootTypeDescriptor;
		}
	}
}