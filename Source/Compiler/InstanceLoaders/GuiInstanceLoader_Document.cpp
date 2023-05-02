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

				void GetRequiredPropertyNames(GuiResourcePrecompileContext& precompileContext, const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(_Name);
					}
				}

				void GetPropertyNames(GuiResourcePrecompileContext& precompileContext, const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					GetRequiredPropertyNames(precompileContext, typeInfo, propertyNames);
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(GuiResourcePrecompileContext& precompileContext, const PropertyInfo& propertyInfo)override
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
					return IGuiInstanceLoader::GetPropertyType(precompileContext, propertyInfo);
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
							auto createExpr = Ptr(new WfNewClassExpression);
							createExpr->type = GetTypeFromTypeInfo(type.Obj());
							createExpr->arguments.Add(arguments.GetByIndex(indexName)[0].expression);

							auto refVariable = Ptr(new WfReferenceExpression);
							refVariable->name.value = variableName.ToString();

							auto assignExpr = Ptr(new WfBinaryExpression);
							assignExpr->op = WfBinaryOperator::Assign;
							assignExpr->first = refVariable;
							assignExpr->second = createExpr;

							auto assignStat = Ptr(new WfExpressionStatement);
							assignStat->expression = assignExpr;
							return assignStat;
						}
					}
					return nullptr;
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = Ptr(new WfBlockStatement);

					// TODO: (enumerable) foreach on group
					for (auto [prop, index] : indexed(arguments.Keys()))
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::Empty)
						{
							auto value = values[0].expression;
							auto td = values[0].typeInfo->GetTypeDescriptor();

							Ptr<WfExpression> compositionExpr;
							if (td->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
							{
								auto member = Ptr(new WfMemberExpression);
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
								auto refItem = Ptr(new WfReferenceExpression);
								refItem->name.value = variableName.ToString();

								auto refContainer = Ptr(new WfMemberExpression);
								refContainer->parent = refItem;
								refContainer->name.value = L"Container";

								auto refAddChild = Ptr(new WfMemberExpression);
								refAddChild->parent = refContainer;
								refAddChild->name.value = L"AddChild";

								auto call = Ptr(new WfCallExpression);
								call->function = refAddChild;
								call->arguments.Add(compositionExpr);

								auto stat = Ptr(new WfExpressionStatement);
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
			private:
				using TypeInfo = typename TBaseType::TypeInfo;

			public:
				using PropertyInfo = IGuiInstanceLoader::PropertyInfo;
				using ArgumentMap = IGuiInstanceLoader::ArgumentMap;

				GuiDocumentInstanceLoaderBase(const WString& _typeName, theme::ThemeName themeName)
					:TBaseType(_typeName, themeName)
				{
				}

				void GetPropertyNames(GuiResourcePrecompileContext& precompileContext, const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
					TBaseType::GetPropertyNames(precompileContext, typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(GuiResourcePrecompileContext& precompileContext, const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(TypeInfoRetriver<Ptr<GuiDocumentItem>>::CreateTypeInfo());
					}
					return TBaseType::GetPropertyType(precompileContext, propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = Ptr(new WfBlockStatement);

					// TODO: (enumerable) foreach on group
					for (auto [prop, index] : indexed(arguments.Keys()))
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::Empty)
						{
							auto refControl = Ptr(new WfReferenceExpression);
							refControl->name.value = variableName.ToString();

							auto refAddDocumentItem = Ptr(new WfMemberExpression);
							refAddDocumentItem->parent = refControl;
							refAddDocumentItem->name.value = L"AddDocumentItem";

							auto call = Ptr(new WfCallExpression);
							call->function = refAddDocumentItem;
							call->arguments.Add(values[0].expression);

							auto stat = Ptr(new WfExpressionStatement);
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

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiDocumentViewer>
			class GuiDocumentViewerInstanceLoader : public GuiDocumentInstanceLoaderBase<BASE_TYPE>
			{
			public:
				GuiDocumentViewerInstanceLoader()
					:GuiDocumentInstanceLoaderBase<BASE_TYPE>(description::TypeInfo<GuiDocumentViewer>::content.typeName, theme::ThemeName::DocumentViewer)
				{
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiDocumentLabelInstanceLoader
***********************************************************************/
		
#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiDocumentLabel>
			class GuiDocumentLabelInstanceLoader : public GuiDocumentInstanceLoaderBase<BASE_TYPE>
			{
			public:
				GuiDocumentLabelInstanceLoader()
					:GuiDocumentInstanceLoaderBase<BASE_TYPE>(description::TypeInfo<GuiDocumentLabel>::content.typeName, theme::ThemeName::DocumentLabel)
				{
				}
			};
#undef BASE_TYPE

/***********************************************************************
Initialization
***********************************************************************/

			void LoadDocumentControls(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(Ptr(new GuiDocumentItemInstanceLoader));
				manager->SetLoader(Ptr(new GuiDocumentViewerInstanceLoader));
				manager->SetLoader(Ptr(new GuiDocumentLabelInstanceLoader));
			}
		}
	}
}

#endif
