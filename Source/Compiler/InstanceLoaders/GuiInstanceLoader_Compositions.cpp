#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

/***********************************************************************
GuiAxisInstanceLoader
***********************************************************************/

			class GuiAxisInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;
				GlobalStringKey					_AxisDirection;

			public:
				GuiAxisInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiAxis>::TypeName);
					_AxisDirection = GlobalStringKey::Get(L"AxisDirection");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(_AxisDirection);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _AxisDirection)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<AxisDirection>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						info->required = true;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool CanCreate(const TypeInfo& typeInfo)override
				{
					return typeName == typeInfo.typeName;
				}

				Ptr<workflow::WfStatement> CreateInstance(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					if (typeName == typeInfo.typeName)
					{
						vint indexAxisDirection = arguments.Keys().IndexOf(_AxisDirection);
						if (indexAxisDirection)
						{
							auto createExpr = MakePtr<WfNewTypeExpression>();
							createExpr->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<GuiAxis>>::CreateTypeInfo().Obj());
							createExpr->arguments.Add(arguments.GetByIndex(indexAxisDirection)[0].expression);

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
			};

/***********************************************************************
GuiCompositionInstanceLoader
***********************************************************************/

			class GuiCompositionInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;

			public:
				GuiCompositionInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiGraphicsComposition>::TypeName);
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
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
						info->acceptableTypes.Add(description::GetTypeDescriptor<IGuiGraphicsElement>());
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::Empty)
						{
							auto value = values[0].expression;
							auto type = values[0].type;

							Ptr<WfExpression> expr;
							if (type->CanConvertTo(description::GetTypeDescriptor<IGuiGraphicsElement>()))
							{
								auto refComposition = MakePtr<WfReferenceExpression>();
								refComposition->name.value = variableName.ToString();

								auto refOwnedElement = MakePtr<WfMemberExpression>();
								refOwnedElement->parent = refComposition;
								refOwnedElement->name.value = L"OwnedElement";

								auto assign = MakePtr<WfBinaryExpression>();
								assign->op = WfBinaryOperator::Assign;
								assign->first = refOwnedElement;
								assign->second = value;

								expr = assign;
							}
							else if (type->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
							{
								auto refBoundsComposition = MakePtr<WfMemberExpression>();
								refBoundsComposition->parent = value;
								refBoundsComposition->name.value = L"BoundsComposition";

								auto refComposition = MakePtr<WfReferenceExpression>();
								refComposition->name.value = variableName.ToString();

								auto refAddChild = MakePtr<WfMemberExpression>();
								refAddChild->parent = refComposition;
								refAddChild->name.value = L"AddChild";

								auto call = MakePtr<WfCallExpression>();
								call->function = refAddChild;
								call->arguments.Add(refBoundsComposition);

								expr = call;
							}
							else if (type->CanConvertTo(description::GetTypeDescriptor<GuiGraphicsComposition>()))
							{
								auto refComposition = MakePtr<WfReferenceExpression>();
								refComposition->name.value = variableName.ToString();

								auto refAddChild = MakePtr<WfMemberExpression>();
								refAddChild->parent = refComposition;
								refAddChild->name.value = L"AddChild";

								auto call = MakePtr<WfCallExpression>();
								call->function = refAddChild;
								call->arguments.Add(value);

								expr = call;
							}

							if (expr)
							{
								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = expr;
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
GuiTableCompositionInstanceLoader
***********************************************************************/

			class GuiTableCompositionInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;
				GlobalStringKey					_Rows, _Columns;

			public:
				GuiTableCompositionInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiTableComposition>::TypeName);
					_Rows = GlobalStringKey::Get(L"Rows");
					_Columns = GlobalStringKey::Get(L"Columns");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_Rows);
					propertyNames.Add(_Columns);
				}

				void GetPairedProperties(const PropertyInfo& propertyInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (propertyInfo.propertyName == _Rows || propertyInfo.propertyName == _Columns)
					{
						propertyNames.Add(_Rows);
						propertyNames.Add(_Columns);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _Rows || propertyInfo.propertyName == _Columns)
					{
						return GuiInstancePropertyInfo::Array(description::GetTypeDescriptor<GuiCellOption>());
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						if (prop == _Rows)
						{
							auto indexColumns = arguments.Keys().IndexOf(_Columns);
							if (indexColumns != -1)
							{
								auto& rows = arguments.GetByIndex(index);
								auto& columns = arguments.GetByIndex(indexColumns);

								{
									auto refComposition = MakePtr<WfReferenceExpression>();
									refComposition->name.value = variableName.ToString();

									auto refSetRowsAndColumns = MakePtr<WfMemberExpression>();
									refSetRowsAndColumns->parent = refComposition;
									refSetRowsAndColumns->name.value = L"SetRowsAndColumns";

									auto rowsExpr = MakePtr<WfIntegerExpression>();
									rowsExpr->value.value = itow(rows.Count());

									auto columnsExpr = MakePtr<WfIntegerExpression>();
									columnsExpr->value.value = itow(columns.Count());

									auto call = MakePtr<WfCallExpression>();
									call->function = refSetRowsAndColumns;
									call->arguments.Add(rowsExpr);
									call->arguments.Add(columnsExpr);

									auto stat = MakePtr<WfExpressionStatement>();
									stat->expression = call;
									block->statements.Add(stat);
								}

								for (vint i = 0; i < rows.Count(); i++)
								{
									auto refComposition = MakePtr<WfReferenceExpression>();
									refComposition->name.value = variableName.ToString();

									auto refSetRowOption = MakePtr<WfMemberExpression>();
									refSetRowOption->parent = refComposition;
									refSetRowOption->name.value = L"SetRowOption";

									auto indexExpr = MakePtr<WfIntegerExpression>();
									indexExpr->value.value = itow(i);

									auto call = MakePtr<WfCallExpression>();
									call->function = refSetRowOption;
									call->arguments.Add(indexExpr);
									call->arguments.Add(rows[i].expression);

									auto stat = MakePtr<WfExpressionStatement>();
									stat->expression = call;
									block->statements.Add(stat);
								}

								for (vint i = 0; i < columns.Count(); i++)
								{
									auto refComposition = MakePtr<WfReferenceExpression>();
									refComposition->name.value = variableName.ToString();

									auto refSetColumnOption = MakePtr<WfMemberExpression>();
									refSetColumnOption->parent = refComposition;
									refSetColumnOption->name.value = L"SetColumnOption";

									auto indexExpr = MakePtr<WfIntegerExpression>();
									indexExpr->value.value = itow(i);

									auto call = MakePtr<WfCallExpression>();
									call->function = refSetColumnOption;
									call->arguments.Add(indexExpr);
									call->arguments.Add(columns[i].expression);

									auto stat = MakePtr<WfExpressionStatement>();
									stat->expression = call;
									block->statements.Add(stat);
								}
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
GuiCellCompositionInstanceLoader
***********************************************************************/

			class GuiCellCompositionInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;
				GlobalStringKey					_Site;

			public:
				GuiCellCompositionInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiCellComposition>::TypeName);
					_Site = GlobalStringKey::Get(L"Site");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_Site);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _Site)
					{
						return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<SiteValue>());
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						if (prop == _Site)
						{
							auto value = arguments.GetByIndex(index)[0].expression;
							Value siteValue = ParseConstantArgument<SiteValue>(value, typeInfo, L"Site", L"row:<integer> column:<integer> rowSpan:<integer> columnSpan:<integer>", errors);
							if (siteValue.IsNull())
							{
								continue;
							}

							{
								auto refComposition = MakePtr<WfReferenceExpression>();
								refComposition->name.value = variableName.ToString();

								auto refSetSite = MakePtr<WfMemberExpression>();
								refSetSite->parent = refComposition;
								refSetSite->name.value = L"SetSite";

								auto call = MakePtr<WfCallExpression>();
								call->function = refSetSite;
								{
									auto arg = MakePtr<WfIntegerExpression>();
									arg->value.value = siteValue.GetProperty(L"row").GetText();
									call->arguments.Add(arg);
								}
								{
									auto arg = MakePtr<WfIntegerExpression>();
									arg->value.value = siteValue.GetProperty(L"column").GetText();
									call->arguments.Add(arg);
								}
								{
									auto arg = MakePtr<WfIntegerExpression>();
									arg->value.value = siteValue.GetProperty(L"rowSpan").GetText();
									call->arguments.Add(arg);
								}
								{
									auto arg = MakePtr<WfIntegerExpression>();
									arg->value.value = siteValue.GetProperty(L"columnSpan").GetText();
									call->arguments.Add(arg);
								}

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
Initialization
***********************************************************************/

			void LoadCompositions(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiAxisInstanceLoader);
				manager->SetLoader(new GuiCompositionInstanceLoader);
				manager->SetLoader(new GuiTableCompositionInstanceLoader);
				manager->SetLoader(new GuiCellCompositionInstanceLoader);
			}
		}
	}
}

#endif
