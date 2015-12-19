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
			vint&								generatedNameCount;
			types::ErrorList&					errors;

			List<types::PropertyResolving>&		candidatePropertyTypeInfos;
			IGuiInstanceLoader::TypeInfo		resolvedTypeInfo;
			vint								selectedPropertyTypeInfo = -1;

			WorkflowReferenceNamesVisitor(Ptr<GuiInstanceContext> _context, types::ResolvingResult& _resolvingResult, List<types::PropertyResolving>& _candidatePropertyTypeInfos, vint& _generatedNameCount, types::ErrorList& _errors)
				:context(_context)
				, resolvingResult(_resolvingResult)
				, candidatePropertyTypeInfos(_candidatePropertyTypeInfos)
				, generatedNameCount(_generatedNameCount)
				, errors(_errors)
			{
			}

			void Visit(GuiTextRepr* repr)override
			{
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				if (resolvedTypeInfo.typeDescriptor == nullptr)
				{
					return;
				}

				if (repr->instanceName == GlobalStringKey::Empty)
				{
					if (resolvedTypeInfo.typeDescriptor->GetValueSerializer() == nullptr)
					{
						auto name = GlobalStringKey::Get(L"<precompile>" + itow(generatedNameCount++));
						repr->instanceName = name;
						resolvingResult.typeInfos.Add(name, resolvedTypeInfo);
					}
				}
				else if (resolvingResult.typeInfos.Keys().Contains(repr->instanceName))
				{
					errors.Add(L"Precompile: Referece name \"" + repr->instanceName.ToString() + L"\" conflict with an existing named object.");
				}
				else if (resolvedTypeInfo.typeDescriptor->GetValueSerializer())
				{
					errors.Add(L"Precompile: Reference name \"" + repr->instanceName.ToString() + L"\" cannot be added to a non-reference instance of type \"" + resolvedTypeInfo.typeName.ToString() + L"\".");
				}
				else
				{
					resolvingResult.typeInfos.Add(repr->instanceName, resolvedTypeInfo);
				}
			
				auto loader = GetInstanceLoaderManager()->GetLoader(resolvedTypeInfo.typeName);

				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
				{
					List<types::PropertyResolving> possibleInfos;
					IGuiInstanceLoader::TypeInfo propertyTypeInfo;

					if (setter->binding == GlobalStringKey::_Set)
					{
						IGuiInstanceLoader::PropertyInfo info(resolvedTypeInfo, repr->setters.Keys()[index]);
						auto currentLoader = loader;

						while (currentLoader)
						{
							if (auto propertyTypeInfo = currentLoader->GetPropertyType(info))
							{
								if (propertyTypeInfo->support == GuiInstancePropertyInfo::NotSupport)
								{
									errors.Add(L"Precompile: Property \"" + info.propertyName.ToString() + L"\" of type \"" + resolvedTypeInfo.typeName.ToString() + L"\" is not supported.");
									break;
								}
								else
								{
									types::PropertyResolving resolving;
									resolving.loader = currentLoader;
									resolving.info = propertyTypeInfo;
									possibleInfos.Add(resolving);
								}

								if (!propertyTypeInfo->tryParent)
								{
									break;
								}
							}
							currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
						}
					}
					else if (setter->binding != GlobalStringKey::Empty)
					{
						auto binder = GetInstanceLoaderManager()->GetInstanceBinder(setter->binding);
						if (!binder)
						{
							errors.Add(L"The appropriate IGuiInstanceBinder of binding \"" + setter->binding.ToString() + L"\" cannot be found.");
						}
					}

					FOREACH(Ptr<GuiValueRepr>, value, setter->values)
					{
						// check serializable values
						// pass to ctors
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
						resolvedTypeInfo.typeName = fullName;
						resolvedTypeInfo.typeDescriptor = td;
					}
				}

				if (!found)
				{
					auto source = FindInstanceLoadingSource(context, repr);
					resolvedTypeInfo.typeName = source.typeName;
					resolvedTypeInfo.typeDescriptor = GetInstanceLoaderManager()->GetTypeDescriptorForType(source.typeName);
				}

				if (resolvedTypeInfo.typeDescriptor)
				{
					// select property type info
					Visit((GuiAttSetterRepr*)repr);
				}
				else
				{
					errors.Add(
						L"Precompile: Failed to find type \"" +
						(repr->typeNamespace == GlobalStringKey::Empty
							? repr->typeName.ToString()
							: repr->typeNamespace.ToString() + L":" + repr->typeName.ToString()
							) +
						L"\".");
				}
			}
		};

		ITypeDescriptor* Workflow_CollectReferences(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, types::ErrorList& errors)
		{
			FOREACH(Ptr<GuiInstanceParameter>, parameter, context->parameters)
			{
				auto type = GetTypeDescriptor(parameter->className.ToString());
				if (!type)
				{
					errors.Add(L"Precompile: Cannot find type \"" + parameter->className.ToString() + L"\".");
				}
				else if (resolvingResult.typeInfos.Keys().Contains(parameter->name))
				{
					errors.Add(L"Precompile: Parameter \"" + parameter->name.ToString() + L"\" conflict with an existing named object.");
				}
				else
				{
					IGuiInstanceLoader::TypeInfo typeInfo;
					typeInfo.typeDescriptor = type;
					typeInfo.typeName = GlobalStringKey::Get(type->GetTypeName());
					resolvingResult.typeInfos.Add(parameter->name, typeInfo);
				}
			}
			
			List<types::PropertyResolving> infos;
			vint generatedNameCount = 0;
			WorkflowReferenceNamesVisitor visitor(context, resolvingResult, infos, generatedNameCount, errors);
			context->instance->Accept(&visitor);
			return visitor.resolvedTypeInfo.typeDescriptor;
		}
	}
}