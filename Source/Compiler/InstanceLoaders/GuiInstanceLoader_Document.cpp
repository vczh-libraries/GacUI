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
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiDocumentItem>::content.typeName);
					_Name = GlobalStringKey::Get(L"Name");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetRequiredPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(_Name);
					}
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					GetRequiredPropertyNames(typeInfo, propertyNames);
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						auto info = GuiInstancePropertyInfo::Collection(nullptr);
						info->acceptableTypes.Add(TypeInfoRetriver<GuiControl*>::CreateTypeInfo());
						info->acceptableTypes.Add(TypeInfoRetriver<GuiGraphicsComposition*>::CreateTypeInfo());
						return info;
					}
					else if (propertyInfo.propertyName == _Name)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<WString>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				
				bool CanCreate(const TypeInfo& typeInfo)override
				{
					return typeName == typeInfo.typeName;
				}

				Ptr<workflow::WfStatement> CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
				{
					if (CanCreate(typeInfo))
					{
						vint indexName = arguments.Keys().IndexOf(_Name);	
						if (indexName != -1)
						{
							auto type = TypeInfoRetriver<Ptr<GuiDocumentItem>>::CreateTypeInfo();
							auto createExpr = MakePtr<WfNewClassExpression>();
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

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::Empty)
						{
							auto value = values[0].expression;
							auto td = values[0].typeInfo->GetTypeDescriptor();

							Ptr<WfExpression> compositionExpr;
							if (td->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
							{
								auto member = MakePtr<WfMemberExpression>();
								member->parent = value;
								member->name.value = L"BoundsComposition";
								compositionExpr = member;
							}
							else if (td->CanConvertTo(description::GetTypeDescriptor<GuiGraphicsComposition>()))
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
					return nullptr;
				}
			};

/***********************************************************************
GuiDocumentInstanceLoaderBase
***********************************************************************/

			template<typename TBaseType>
			class GuiDocumentInstanceLoaderBase : public TBaseType
			{
			public:
				using PropertyInfo = IGuiInstanceLoader::PropertyInfo;
				using ArgumentMap = IGuiInstanceLoader::ArgumentMap;

				GuiDocumentInstanceLoaderBase(const WString& _typeName, const WString& _styleMethod)
					:TBaseType(_typeName, _styleMethod)
				{
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
					TBaseType::GetPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(TypeInfoRetriver<Ptr<GuiDocumentItem>>::CreateTypeInfo());
					}
					return TBaseType::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::Empty)
						{
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
					return TBaseType::AssignParameters(precompileContext, resolvingResult, typeInfo, variableName, arguments, attPosition, errors);
				}
			};

/***********************************************************************
GuiDocumentViewerInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiDocumentViewer, GuiDocumentViewerTemplate_StyleProvider, GuiDocumentViewerTemplate>
			class GuiDocumentViewerInstanceLoader : public GuiDocumentInstanceLoaderBase<BASE_TYPE>
			{
			public:
				GuiDocumentViewerInstanceLoader()
					:GuiDocumentInstanceLoaderBase<BASE_TYPE>(description::TypeInfo<GuiDocumentViewer>::content.typeName, L"CreateDocumentViewerStyle")
				{
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiDocumentLabelInstanceLoader
***********************************************************************/
		
#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiDocumentLabel, GuiDocumentLabelTemplate_StyleProvider, GuiDocumentLabelTemplate>
			class GuiDocumentLabelInstanceLoader : public GuiDocumentInstanceLoaderBase<BASE_TYPE>
			{
			public:
				GuiDocumentLabelInstanceLoader()
					:GuiDocumentInstanceLoaderBase<BASE_TYPE>(description::TypeInfo<GuiDocumentLabel>::content.typeName, L"CreateDocumentLabelStyle")
				{
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
