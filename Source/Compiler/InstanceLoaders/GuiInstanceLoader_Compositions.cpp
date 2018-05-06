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
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiAxis>::content.typeName);
					_AxisDirection = GlobalStringKey::Get(L"AxisDirection");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetRequiredPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(_AxisDirection);
					}
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					GetRequiredPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _AxisDirection)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<AxisDirection>::CreateTypeInfo());
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
						vint indexAxisDirection = arguments.Keys().IndexOf(_AxisDirection);
						if (indexAxisDirection != -1)
						{
							auto createExpr = MakePtr<WfNewClassExpression>();
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
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiGraphicsComposition>::content.typeName);
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
						auto info = GuiInstancePropertyInfo::Collection(nullptr);
						info->acceptableTypes.Add(TypeInfoRetriver<GuiControl*>::CreateTypeInfo());
						info->acceptableTypes.Add(TypeInfoRetriver<GuiGraphicsComposition*>::CreateTypeInfo());
						info->acceptableTypes.Add(TypeInfoRetriver<Ptr<IGuiGraphicsElement>>::CreateTypeInfo());
						if (propertyInfo.typeInfo.typeInfo->GetTypeDescriptor()->CanConvertTo(description::GetTypeDescriptor<GuiInstanceRootObject>()))
						{
							info->acceptableTypes.Add(TypeInfoRetriver<GuiComponent*>::CreateTypeInfo());
						}
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
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

							Ptr<WfExpression> expr;
							if (td->CanConvertTo(description::GetTypeDescriptor<GuiComponent>()))
							{
								auto refControl = MakePtr<WfReferenceExpression>();
								refControl->name.value = variableName.ToString();

								auto refAddComponent = MakePtr<WfMemberExpression>();
								refAddComponent->parent = refControl;
								refAddComponent->name.value = L"AddComponent";

								auto call = MakePtr<WfCallExpression>();
								call->function = refAddComponent;
								call->arguments.Add(value);

								expr = call;
							}
							else if (td->CanConvertTo(description::GetTypeDescriptor<GuiControlHost>()))
							{
								auto refControl = MakePtr<WfReferenceExpression>();
								refControl->name.value = variableName.ToString();

								auto refAddControlHostComponent = MakePtr<WfMemberExpression>();
								refAddControlHostComponent->parent = refControl;
								refAddControlHostComponent->name.value = L"AddControlHostComponent";

								auto call = MakePtr<WfCallExpression>();
								call->function = refAddControlHostComponent;
								call->arguments.Add(value);

								expr = call;
							}
							else if (td->CanConvertTo(description::GetTypeDescriptor<IGuiGraphicsElement>()))
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
							else if (td->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
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
							else if (td->CanConvertTo(description::GetTypeDescriptor<GuiGraphicsComposition>()))
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
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiTableComposition>::content.typeName);
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
						return GuiInstancePropertyInfo::Array(TypeInfoRetriver<GuiCellOption>::CreateTypeInfo());
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
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
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiCellComposition>::content.typeName);
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
						return GuiInstancePropertyInfo::Assign(TypeInfoRetriver<SiteValue>::CreateTypeInfo());
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						if (prop == _Site)
						{
							SiteValue site;
							{
								auto siteExpr = arguments.GetByIndex(index)[0].expression;
								if (auto inferExpr = siteExpr.Cast<WfInferExpression>())
								{
									if (auto ctorExpr = inferExpr->expression.Cast<WfConstructorExpression>())
									{
										auto st = description::GetTypeDescriptor<vint>()->GetSerializableType();
										FOREACH(Ptr<WfConstructorArgument>, argument, ctorExpr->arguments)
										{
											if (auto keyExpr = argument->key.Cast<WfReferenceExpression>())
											{
												if (auto valueExpr = argument->value.Cast<WfIntegerExpression>())
												{
													Value value;
													if (st->Deserialize(valueExpr->value.value, value))
													{
														vint propValue = UnboxValue<vint>(value);
														if (keyExpr->name.value == L"row")
														{
															site.row = propValue;
														}
														else if (keyExpr->name.value == L"column")
														{
															site.column = propValue;
														}
														else if (keyExpr->name.value == L"rowSpan")
														{
															site.rowSpan = propValue;
														}
														else if (keyExpr->name.value == L"columnSpan")
														{
															site.columnSpan = propValue;
														}
														else
														{
															goto ILLEGAL_SITE_PROPERTY;
														}
														continue;
													}
												}
											}
											goto ILLEGAL_SITE_PROPERTY;
										}
										goto FINISH_SITE_PROPERTY;
									}
								}
							ILLEGAL_SITE_PROPERTY:
								errors.Add(GuiResourceError({ resolvingResult.resource }, attPosition,
									L"Precompile: The value of property \"Site\" of type \"" +
									typeInfo.typeName.ToString() +
									L"\" is not in a correct format: \"row:<integer> column:<integer> [rowSpan:<integer>] [columnSpan:<integer>]\"."));
								continue;
							}
						FINISH_SITE_PROPERTY:;

							{
								auto refComposition = MakePtr<WfReferenceExpression>();
								refComposition->name.value = variableName.ToString();

								auto refSetSite = MakePtr<WfMemberExpression>();
								refSetSite->parent = refComposition;
								refSetSite->name.value = L"SetSite";

								auto call = MakePtr<WfCallExpression>();
								call->function = refSetSite;

								auto GetValueText = [](const Value& value)
								{
									WString result;
									auto st = value.GetTypeDescriptor()->GetSerializableType();
									st->Serialize(value, result);
									return result;
								};

								{
									auto arg = MakePtr<WfIntegerExpression>();
									arg->value.value = itow(site.row);
									call->arguments.Add(arg);
								}
								{
									auto arg = MakePtr<WfIntegerExpression>();
									arg->value.value = itow(site.column);
									call->arguments.Add(arg);
								}
								{
									auto arg = MakePtr<WfIntegerExpression>();
									arg->value.value = itow(site.rowSpan);
									call->arguments.Add(arg);
								}
								{
									auto arg = MakePtr<WfIntegerExpression>();
									arg->value.value = itow(site.columnSpan);
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
