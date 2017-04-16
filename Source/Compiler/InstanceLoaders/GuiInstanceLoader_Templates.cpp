#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

/***********************************************************************
GuiTemplateInstanceLoader
***********************************************************************/

			class GuiTemplateInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;

			public:
				GuiTemplateInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiTemplate>::content.typeName);
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					List<ITypeDescriptor*> tds;
					tds.Add(typeInfo.typeInfo->GetTypeDescriptor());

					for (vint i = 0; i < tds.Count(); i++)
					{
						auto td = tds[i];
						vint propCount = td->GetPropertyCount();
						for (vint i = 0; i < propCount; i++)
						{
							auto prop = td->GetProperty(i);
							if (prop->IsWritable() && prop->GetReturn()->GetTypeDescriptor() == description::GetTypeDescriptor<GuiTemplate::IFactory>())
							{
								propertyNames.Add(GlobalStringKey::Get(prop->GetName()));
							}
						}

						vint baseCount = td->GetBaseTypeDescriptorCount();
						for (vint i = 0; i < baseCount; i++)
						{
							auto baseTd = td->GetBaseTypeDescriptor(i);
							if (!tds.Contains(baseTd))
							{
								tds.Add(baseTd);
							}
						}
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (auto prop = propertyInfo.typeInfo.typeInfo->GetTypeDescriptor()->GetPropertyByName(propertyInfo.propertyName.ToString(), true))
					{
						if (prop->IsWritable() && prop->GetReturn()->GetTypeDescriptor() == description::GetTypeDescriptor<GuiTemplate::IFactory>())
						{
							auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<WString>::CreateTypeInfo());
							info->usage = GuiInstancePropertyInfo::Property;

							if (prop->GetOwnerTypeDescriptor() != description::GetTypeDescriptor<GuiWindowTemplate>())
							{
								info->requirement = GuiInstancePropertyInfo::Required;
							}
							return info;
						}
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool CanCreate(const TypeInfo& typeInfo)override
				{
					if (typeInfo.typeInfo->GetTypeDescriptor()->CanConvertTo(description::GetTypeDescriptor<GuiTemplate>()))
					{
						auto group = typeInfo.typeInfo->GetTypeDescriptor()->GetConstructorGroup();
						if (group && group->GetMethodCount() == 1)
						{
							auto ctor = group->GetMethod(0);
							if (ctor->GetParameterCount() == 0)
							{
								return true;
							}
						}
					}
					return false;
				}

				Ptr<workflow::WfStatement> CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
				{
					if (!CanCreate(typeInfo))
					{
						return nullptr;
					}

					auto block = MakePtr<WfBlockStatement>();
					{
						auto createExpr = MakePtr<WfNewClassExpression>();
						createExpr->type = GetTypeFromTypeInfo(MakePtr<TypeDescriptorTypeInfo>(typeInfo.typeInfo->GetTypeDescriptor(), TypeInfoHint::Normal).Obj());

						auto refVariable = MakePtr<WfReferenceExpression>();
						refVariable->name.value = variableName.ToString();

						auto assignExpr = MakePtr<WfBinaryExpression>();
						assignExpr->op = WfBinaryOperator::Assign;
						assignExpr->first = refVariable;
						assignExpr->second = createExpr;

						auto assignStat = MakePtr<WfExpressionStatement>();
						assignStat->expression = assignExpr;
						block->statements.Add(assignStat);
					}
					return block;
				}
			};

/***********************************************************************
Initialization
***********************************************************************/

			void LoadTemplates(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiTemplateInstanceLoader);
			}
		}
	}
}

#endif
