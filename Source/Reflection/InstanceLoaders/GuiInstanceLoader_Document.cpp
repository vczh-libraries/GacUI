#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

/***********************************************************************
GuiDocumentItemInstanceLoader
***********************************************************************/

			class GuiDocumentItemInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;
				GlobalStringKey					_Name;

			public:
				GuiDocumentItemInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiDocumentItem>()->GetTypeName());
					_Name = GlobalStringKey::Get(L"Name");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(_Name);
					}
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						auto info = GuiInstancePropertyInfo::Collection();
						info->acceptableTypes.Add(description::GetTypeDescriptor<GuiControl>());
						info->acceptableTypes.Add(description::GetTypeDescriptor<GuiGraphicsComposition>());
						return info;
					}
					else if (propertyInfo.propertyName == _Name)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						info->required = true;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				
				bool CanCreate(const TypeInfo& typeInfo)
				{
					return typeName == typeInfo.typeName;
				}

				Ptr<workflow::WfStatement> CreateInstance(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)
				{
					if (typeInfo.typeName == GetTypeName())
					{
						vint indexName = arguments.Keys().IndexOf(_Name);	
						if (indexName != -1)
						{
							auto type = TypeInfoRetriver<Ptr<GuiDocumentItem>>::CreateTypeInfo();
							auto createExpr = MakePtr<WfNewTypeExpression>();
							createExpr->type = GetTypeFromTypeInfo(type.Obj());
							createExpr->arguments.Add(arguments.GetByIndex(indexName)[0].expression);

							auto refVariable = MakePtr<WfReferenceExpression>();
							refVariable->name.value = variableName.ToString();

							auto assignExpr = MakePtr<WfBinaryExpression>();
							assignExpr->op = WfBinaryOperator::Assign;
							assignExpr->first = refVariable;
							assignExpr->second = createExpr;

							auto assignStat = MakePtr<WfExpressionStatement>();
							assignStat->expression = assignExpr;
							return assignStat;
						}
					}
					return nullptr;
				}

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						auto block = MakePtr<WfBlockStatement>();

						FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
						{
							const auto& values = arguments.GetByIndex(index);
							if (prop == GlobalStringKey::Empty)
							{
								auto value = values[0].expression;
								auto type = values[0].type;

								Ptr<WfExpression> compositionExpr;
								if (type->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
								{
									auto member = MakePtr<WfMemberExpression>();
									member->parent = value;
									member->name.value = L"BoundsComposition";
									compositionExpr = member;
								}
								else if (type->CanConvertTo(description::GetTypeDescriptor<GuiGraphicsComposition>()))
								{
									compositionExpr = value;
								}

								if (compositionExpr)
								{
									auto refItem = MakePtr<WfReferenceExpression>();
									refItem->name.value = variableName.ToString();

									auto refContainer = MakePtr<WfMemberExpression>();
									refContainer->parent = refItem;
									refContainer->name.value = L"Container";

									auto refAddChild = MakePtr<WfMemberExpression>();
									refAddChild->parent = refContainer;
									refAddChild->name.value = L"AddChild";

									auto call = MakePtr<WfCallExpression>();
									call->function = refAddChild;
									call->arguments.Add(compositionExpr);

									auto stat = MakePtr<WfExpressionStatement>();
									stat->expression = call;
									block->statements.Add(stat);
								}
							}
						}

						if (block->statements.Count() > 0)
						{
							return block;
						}
					}
					return nullptr;
				}
			};

/***********************************************************************
GuiDocumentViewerInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiDocumentViewer, GuiDocumentViewerTemplate_StyleProvider, GuiDocumentViewerTemplate>
			class GuiDocumentViewerInstanceLoader : public BASE_TYPE
			{
			public:
				GuiDocumentViewerInstanceLoader()
					:BASE_TYPE(description::GetTypeDescriptor<GuiDocumentViewer>()->GetTypeName(), L"CreateDocumentViewerStyle")
				{
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiDocumentItem>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						auto block = MakePtr<WfBlockStatement>();

						FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
						{
							const auto& values = arguments.GetByIndex(index);
							if (prop == GlobalStringKey::Empty)
							{
								auto type = values[0].type;

								auto refControl = MakePtr<WfReferenceExpression>();
								refControl->name.value = variableName.ToString();

								auto refAddDocumentItem = MakePtr<WfMemberExpression>();
								refAddDocumentItem->parent = refControl;
								refAddDocumentItem->name.value = L"AddDocumentItem";

								auto call = MakePtr<WfCallExpression>();
								call->function = refAddDocumentItem;
								call->arguments.Add(values[0].expression);

								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = call;
								block->statements.Add(stat);
							}
						}

						if (block->statements.Count() > 0)
						{
							return block;
						}
					}
					return nullptr;
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiDocumentLabelInstanceLoader
***********************************************************************/
		
#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiDocumentLabel, GuiDocumentLabelTemplate_StyleProvider, GuiDocumentLabelTemplate>
			class GuiDocumentLabelInstanceLoader : public BASE_TYPE
			{
			public:
				GuiDocumentLabelInstanceLoader()
					:BASE_TYPE(description::GetTypeDescriptor<GuiDocumentLabel>()->GetTypeName(), L"CreateDocumentLabelStyle")
				{
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiDocumentItem>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						auto block = MakePtr<WfBlockStatement>();

						FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
						{
							const auto& values = arguments.GetByIndex(index);
							if (prop == GlobalStringKey::Empty)
							{
								auto type = values[0].type;

								auto refControl = MakePtr<WfReferenceExpression>();
								refControl->name.value = variableName.ToString();

								auto refAddDocumentItem = MakePtr<WfMemberExpression>();
								refAddDocumentItem->parent = refControl;
								refAddDocumentItem->name.value = L"AddDocumentItem";

								auto call = MakePtr<WfCallExpression>();
								call->function = refAddDocumentItem;
								call->arguments.Add(values[0].expression);

								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = call;
								block->statements.Add(stat);
							}
						}

						if (block->statements.Count() > 0)
						{
							return block;
						}
					}
					return nullptr;
				}
			};
#undef BASE_TYPE

/***********************************************************************
Initialization
***********************************************************************/

			void LoadDocumentControls(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiDocumentItemInstanceLoader);
				manager->SetLoader(new GuiDocumentViewerInstanceLoader);
				manager->SetLoader(new GuiDocumentLabelInstanceLoader);
			}
		}
	}
}

#endif