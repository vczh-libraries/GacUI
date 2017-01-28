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

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					List<ITypeDescriptor*> tds;
					tds.Add(typeInfo.typeDescriptor);

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
					if (auto prop = propertyInfo.typeInfo.typeDescriptor->GetPropertyByName(propertyInfo.propertyName.ToString(), true))
					{
						if (prop->IsWritable() && prop->GetReturn()->GetTypeDescriptor() == description::GetTypeDescriptor<GuiTemplate::IFactory>())
						{
							auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
							info->scope = GuiInstancePropertyInfo::Constructor;
							info->bindable = false;

							if (prop->GetOwnerTypeDescriptor() != description::GetTypeDescriptor<GuiWindowTemplate>())
							{
								info->required = true;
							}
							return info;
						}
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool CanCreate(const TypeInfo& typeInfo)override
				{
					if (typeInfo.typeDescriptor->CanConvertTo(description::GetTypeDescriptor<GuiTemplate>()))
					{
						auto group = typeInfo.typeDescriptor->GetConstructorGroup();
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

				Ptr<workflow::WfStatement> CreateInstance(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					if (!CanCreate(typeInfo))
					{
						return nullptr;
					}

					auto block = MakePtr<WfBlockStatement>();
					{
						auto createExpr = MakePtr<WfNewClassExpression>();
						createExpr->type = GetTypeFromTypeInfo(MakePtr<TypeDescriptorTypeInfo>(typeInfo.typeDescriptor, TypeInfoHint::Normal).Obj());

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

					if (auto stat = InitializeRootInstance(typeInfo, variableName, arguments, errors))
					{
						CopyFrom(block->statements, stat.Cast<WfBlockStatement>()->statements, true);
					}
					return block;
				}

				Ptr<workflow::WfStatement> InitializeRootInstance(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)
				{
					if (arguments.Count() > 0)
					{
						auto block = MakePtr<WfBlockStatement>();
						using Helper = GuiTemplateControlInstanceLoader<Value, Value, GuiTemplate>;

						FOREACH_INDEXER(GlobalStringKey, propKey, index, arguments.Keys())
						{
							List<ITypeDescriptor*> tds;
							Helper::GetItemTemplateType(arguments.GetByIndex(index)[0].expression, tds, typeInfo, propKey.ToString(), errors);
							auto refFactory = Helper::CreateTemplateFactory(tds, errors);

							auto refVariable = MakePtr<WfReferenceExpression>();
							refVariable->name.value = variableName.ToString();

							auto member = MakePtr<WfMemberExpression>();
							member->parent = refVariable;
							member->name.value = propKey.ToString();

							auto assignExpr = MakePtr<WfBinaryExpression>();
							assignExpr->op = WfBinaryOperator::Assign;
							assignExpr->first = member;
							assignExpr->second = refFactory;

							auto assignStat = MakePtr<WfExpressionStatement>();
							assignStat->expression = assignExpr;
							block->statements.Add(assignStat);
						}
						return block;
					}
					else
					{
						return nullptr;
					}
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
