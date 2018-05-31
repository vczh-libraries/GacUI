#include "GuiInstanceLoader.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace reflection::description;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace controls;
		using namespace controls::list;
		using namespace templates;

/***********************************************************************
GuiTemplatePropertyDeserializer
***********************************************************************/

		class GuiTemplatePropertyDeserializer : public Object, public IGuiInstanceDeserializer
		{
		protected:
			Ptr<ITypeInfo>						stringType;

			bool IsTemplatePropertyType(ITypeInfo* propType)
			{
				if (propType->GetDecorator() == ITypeInfo::SharedPtr)
				{
					auto genericType = propType->GetElementType();
					if (genericType->GetDecorator() == ITypeInfo::Generic && genericType->GetGenericArgumentCount() == 2)
					{
						if (genericType->GetElementType()->GetTypeDescriptor() == description::GetTypeDescriptor<IValueFunctionProxy>())
						{
							if (genericType->GetGenericArgument(1)->GetTypeDescriptor() == description::GetTypeDescriptor<Value>())
							{
								auto returnType = genericType->GetGenericArgument(0);
								if (returnType->GetDecorator() == ITypeInfo::RawPtr)
								{
									if (returnType->GetElementType()->GetTypeDescriptor()->CanConvertTo(description::GetTypeDescriptor<GuiTemplate>()))
									{
										return true;
									}
								}
							}
						}
					}
				}
				return false;
			}

			bool IsDataVisualizerFactoryType(ITypeInfo* propType)
			{
				return propType->GetDecorator() == ITypeInfo::SharedPtr && propType->GetTypeDescriptor() == description::GetTypeDescriptor<list::IDataVisualizerFactory>();
			}

			bool IsDataEditorFactoryType(ITypeInfo* propType)
			{
				return propType->GetDecorator() == ITypeInfo::SharedPtr && propType->GetTypeDescriptor() == description::GetTypeDescriptor<list::IDataEditorFactory>();
			}

		public:
			GuiTemplatePropertyDeserializer()
			{
				stringType = TypeInfoRetriver<WString>::CreateTypeInfo();
			}

			bool CanDeserialize(const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo)override
			{
				return IsTemplatePropertyType(typeInfo) || IsDataVisualizerFactoryType(typeInfo) || IsDataEditorFactoryType(typeInfo);
			}

			description::ITypeInfo* DeserializeAs(const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo)override
			{
				return stringType.Obj();
			}

			static void GetItemTemplateType(
				types::ResolvingResult& resolvingResult,
				WString typeNamesString,
				List<ITypeDescriptor*>& tds,
				GuiResourceTextPos tagPosition,
				GuiResourceError::List& errors
				)
			{
				List<WString> typeNames;
				SplitBySemicolon(typeNamesString, typeNames);
				if (typeNames.Count() == 0)
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, tagPosition,
						L"Precompile: Template list \"" +
						typeNamesString +
						L"\" cannot be empty."));
				}

				FOREACH(WString, controlTemplateName, typeNames)
				{
					auto controlTemplateTd = description::GetTypeDescriptor(controlTemplateName);
					if (!controlTemplateTd)
					{
						auto index = INVLOC.FindFirst(controlTemplateName, L":", Locale::None);
						GlobalStringKey namespaceName;
						auto typeName = controlTemplateName;
						if (index.key != -1)
						{
							namespaceName = GlobalStringKey::Get(controlTemplateName.Left(index.key));
							typeName = controlTemplateName.Right(controlTemplateName.Length() - index.key - index.value);
						}

						auto source = FindInstanceLoadingSource(resolvingResult.context, namespaceName, typeName);
						if (auto typeInfo = GetInstanceLoaderManager()->GetTypeInfoForType(source.typeName))
						{
							controlTemplateTd = typeInfo->GetTypeDescriptor();
						}
					}
					if (controlTemplateTd)
					{
						tds.Add(controlTemplateTd);
					}
					else
					{
						errors.Add(GuiResourceError({ resolvingResult.resource }, tagPosition,
							L"Precompile: Type \"" +
							controlTemplateName +
							L"\" does not exist."));
						continue;
					}
				}
			}

			static Ptr<WfExpression> CreateTemplateFactory(
				types::ResolvingResult& resolvingResult,
				List<ITypeDescriptor*>& controlTemplateTds,
				ITypeInfo* returnTemplateType,
				ITypeInfo* expectedTemplateType,
				GuiResourceTextPos tagPosition,
				GuiResourceError::List& errors
				)
			{
				auto funcCreateTemplate = MakePtr<WfFunctionDeclaration>();
				funcCreateTemplate->anonymity = WfFunctionAnonymity::Anonymous;
				funcCreateTemplate->returnType = GetTypeFromTypeInfo(returnTemplateType);

				auto argViewModel = MakePtr<WfFunctionArgument>();
				argViewModel->type = GetTypeFromTypeInfo(TypeInfoRetriver<Value>::CreateTypeInfo().Obj());
				argViewModel->name.value = L"<viewModel>";
				funcCreateTemplate->arguments.Add(argViewModel);

				auto block = MakePtr<WfBlockStatement>();
				funcCreateTemplate->statement = block;

				ITypeDescriptor* stopControlTemplateTd = nullptr;
				FOREACH(ITypeDescriptor*, controlTemplateTd, controlTemplateTds)
				{
					if (!controlTemplateTd->CanConvertTo(expectedTemplateType->GetTypeDescriptor()))
					{
						errors.Add(GuiResourceError({ resolvingResult.resource }, tagPosition,
							L"Precompile: Type \"" +
							controlTemplateTd->GetTypeName() +
							L"\" cannot be used here because it requires \"" +
							expectedTemplateType->GetTypeDescriptor()->GetTypeName() +
							L"\" or its derived classes."));
					}

					if (stopControlTemplateTd)
					{
						errors.Add(GuiResourceError({ resolvingResult.resource }, tagPosition,
							L"Precompile: Type \"" +
							controlTemplateTd->GetTypeName() +
							L"\" will never be tried, because \"" +
							stopControlTemplateTd->GetTypeName() +
							L"\", which is listed before, has a default constructor. So whatever the view model is, it will be the last choice."));
						continue;
					}

					ITypeInfo* viewModelType = nullptr;
					{
						auto ctors = controlTemplateTd->GetConstructorGroup();
						if (!ctors || ctors->GetMethodCount() != 1)
						{
							errors.Add(GuiResourceError({ resolvingResult.resource }, tagPosition,
								L"Precompile: To use type \"" +
								controlTemplateTd->GetTypeName() +
								L"\" as a control template or item template, it should have exactly one constructor."));
							continue;
						}

						auto ctor = ctors->GetMethod(0);
						if (ctor->GetParameterCount() > 1)
						{
							errors.Add(GuiResourceError({ resolvingResult.resource }, tagPosition,
								L"Precompile: To use type \"" +
								controlTemplateTd->GetTypeName() +
								L"\" as a control template or item template, its constructor cannot have more than one parameter."));
							continue;
						}

						if (ctor->GetParameterCount() != 0)
						{
							viewModelType = ctor->GetParameter(0)->GetType();
						}
					}

					if (!viewModelType)
					{
						stopControlTemplateTd = controlTemplateTd;
					}

					auto subBlock = MakePtr<WfBlockStatement>();
					block->statements.Add(subBlock);

					Ptr<ITypeInfo> controlTemplateType;
					{
						auto elementType = MakePtr<TypeDescriptorTypeInfo>(controlTemplateTd, TypeInfoHint::Normal);
						auto pointerType = MakePtr<RawPtrTypeInfo>(elementType);

						controlTemplateType = pointerType;
					}

					Ptr<WfBlockStatement> returnStatBlock;
					if (viewModelType)
					{
						auto refViewModel = MakePtr<WfReferenceExpression>();
						refViewModel->name.value = L"<viewModel>";

						auto condition = MakePtr<WfTypeTestingExpression>();
						condition->test = WfTypeTesting::IsType;
						condition->expression = refViewModel;
						condition->type = GetTypeFromTypeInfo(viewModelType);

						auto ifStat = MakePtr<WfIfStatement>();
						subBlock->statements.Add(ifStat);
						ifStat->expression = condition;

						returnStatBlock = MakePtr<WfBlockStatement>();
						ifStat->trueBranch = returnStatBlock;
					}
					else
					{
						returnStatBlock = subBlock;
					}

					{
						auto createControlTemplate = MakePtr<WfNewClassExpression>();
						createControlTemplate->type = GetTypeFromTypeInfo(controlTemplateType.Obj());
						if (viewModelType)
						{
							auto refViewModel = MakePtr<WfReferenceExpression>();
							refViewModel->name.value = L"<viewModel>";

							auto cast = MakePtr<WfTypeCastingExpression>();
							cast->strategy = WfTypeCastingStrategy::Strong;
							cast->expression = refViewModel;
							cast->type = GetTypeFromTypeInfo(viewModelType);
							createControlTemplate->arguments.Add(cast);
						}

						auto returnStat = MakePtr<WfReturnStatement>();
						returnStat->expression = createControlTemplate;
						returnStatBlock->statements.Add(returnStat);
					}
				}

				if (!stopControlTemplateTd)
				{
					auto value = MakePtr<WfStringExpression>();
					value->value.value = L"Cannot find a matched control template to create.";

					auto raiseStat = MakePtr<WfRaiseExceptionStatement>();
					raiseStat->expression = value;

					block->statements.Add(raiseStat);
				}

				auto expr = MakePtr<WfFunctionExpression>();
				expr->function = funcCreateTemplate;
				return expr;
			}

			static Ptr<WfExpression> CreateDataVisualizerFactory(
				types::ResolvingResult& resolvingResult,
				List<ITypeDescriptor*>& controlTemplateTds,
				GuiResourceTextPos tagPosition,
				GuiResourceError::List& errors
			)
			{
				auto templateType = TypeInfoRetriver<GuiGridVisualizerTemplate*>::CreateTypeInfo();
				Ptr<WfExpression> previousFactory;
				FOREACH_INDEXER(ITypeDescriptor*, controlTemplateTd, index, controlTemplateTds)
				{
					List<ITypeDescriptor*> tds;
					tds.Add(controlTemplateTd);
					auto refFactory = CreateTemplateFactory(resolvingResult, tds, templateType.Obj(), templateType.Obj(), tagPosition, errors);
					auto createStyle = MakePtr<WfNewClassExpression>();
					createStyle->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<list::DataVisualizerFactory>>::CreateTypeInfo().Obj());
					createStyle->arguments.Add(refFactory);

					if (index > 0)
					{
						createStyle->arguments.Add(previousFactory);
					}
					else
					{
						auto nullExpr = MakePtr<WfLiteralExpression>();
						nullExpr->value = WfLiteralValue::Null;
						createStyle->arguments.Add(nullExpr);
					}
					previousFactory = createStyle;
				}
				return previousFactory;
			}

			static Ptr<WfExpression> CreateDataEditorFactory(
				types::ResolvingResult& resolvingResult,
				List<ITypeDescriptor*>& controlTemplateTds,
				GuiResourceTextPos tagPosition,
				GuiResourceError::List& errors
			)
			{
				auto templateType = TypeInfoRetriver<GuiGridEditorTemplate*>::CreateTypeInfo();
				auto refFactory = CreateTemplateFactory(resolvingResult, controlTemplateTds, templateType.Obj(), templateType.Obj(), tagPosition, errors);
				auto createStyle = MakePtr<WfNewClassExpression>();
				createStyle->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<list::DataEditorFactory>>::CreateTypeInfo().Obj());
				createStyle->arguments.Add(refFactory);
				return createStyle;
			}

			Ptr<workflow::WfExpression> Deserialize(
				GuiResourcePrecompileContext& precompileContext,
				types::ResolvingResult& resolvingResult,
				const IGuiInstanceLoader::PropertyInfo& propertyInfo,
				description::ITypeInfo* typeInfo,
				Ptr<workflow::WfExpression> valueExpression,
				GuiResourceTextPos tagPosition,
				GuiResourceError::List& errors
				)override
			{
				auto stringExpr = valueExpression.Cast<WfStringExpression>();

				List<ITypeDescriptor*> tds;
				GetItemTemplateType(resolvingResult, stringExpr->value.value, tds, tagPosition, errors);

				if (IsDataVisualizerFactoryType(typeInfo))
				{
					return CreateDataVisualizerFactory(resolvingResult, tds, tagPosition, errors);
				}
				else if (IsDataEditorFactoryType(typeInfo))
				{
					return CreateDataEditorFactory(resolvingResult, tds, tagPosition, errors);
				}
				else
				{
					auto returnTemplateType = typeInfo->GetElementType()->GetGenericArgument(0);
					auto expectedTemplateType = returnTemplateType;
					if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
					{
						auto td = propertyInfo.typeInfo.typeInfo->GetTypeDescriptor();
						if (td != nullptr && td->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
						{
							auto methodGroup = td->GetMethodGroupByName(L"GetControlTemplateObject", true);
							vint count = methodGroup->GetMethodCount();
							for (vint i = 0; i < count; i++)
							{
								auto methodInfo = methodGroup->GetMethod(i);
								if (methodInfo->GetParameterCount() == 1)
								{
									auto returnType = methodInfo->GetReturn();
									if (returnType->GetDecorator() == ITypeInfo::RawPtr)
									{
										if (returnType->GetTypeDescriptor()->CanConvertTo(description::GetTypeDescriptor<GuiControlTemplate>()))
										{
											expectedTemplateType = returnType;
										}
									}
								}
							}
						}
					}
					return CreateTemplateFactory(resolvingResult, tds, returnTemplateType, expectedTemplateType, tagPosition, errors);
				}
			}
		};

/***********************************************************************
GuiItemPropertyDeserializer
***********************************************************************/

		class GuiItemPropertyDeserializer : public Object, public IGuiInstanceDeserializer
		{
		protected:
			Ptr<ITypeInfo>						stringType;

			bool IsItemPropertyType(ITypeInfo* propType)
			{
				if (propType->GetDecorator() == ITypeInfo::SharedPtr)
				{
					auto genericType = propType->GetElementType();
					if (genericType->GetDecorator() == ITypeInfo::Generic && genericType->GetGenericArgumentCount() == 2)
					{
						if (genericType->GetElementType()->GetTypeDescriptor() == description::GetTypeDescriptor<IValueFunctionProxy>())
						{
							if (genericType->GetGenericArgument(1)->GetTypeDescriptor() == description::GetTypeDescriptor<Value>())
							{
								return true;
							}
						}
					}
				}
				return false;
			}

			bool IsWritableItemPropertyType(ITypeInfo* propType)
			{
				if (propType->GetDecorator() == ITypeInfo::SharedPtr)
				{
					auto genericType = propType->GetElementType();
					if (genericType->GetDecorator() == ITypeInfo::Generic && genericType->GetGenericArgumentCount() == 4)
					{
						if (genericType->GetElementType()->GetTypeDescriptor() == description::GetTypeDescriptor<IValueFunctionProxy>())
						{
							if (genericType->GetGenericArgument(1)->GetTypeDescriptor() == description::GetTypeDescriptor<Value>()
								&& genericType->GetGenericArgument(3)->GetTypeDescriptor() == description::GetTypeDescriptor<bool>())
							{
								if (IsSameType(genericType->GetGenericArgument(0), genericType->GetGenericArgument(2)))
								{
									return true;
								}
							}
						}
					}
				}
				return false;
			}

		public:
			GuiItemPropertyDeserializer()
			{
				stringType = TypeInfoRetriver<WString>::CreateTypeInfo();
			}

			bool CanDeserialize(const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo)override
			{
				return IsItemPropertyType(typeInfo) || IsWritableItemPropertyType(typeInfo);
			}

			description::ITypeInfo* DeserializeAs(const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo)override
			{
				return stringType.Obj();
			}

			Ptr<workflow::WfExpression> Deserialize(
				GuiResourcePrecompileContext& precompileContext,
				types::ResolvingResult& resolvingResult,
				const IGuiInstanceLoader::PropertyInfo& propertyInfo,
				description::ITypeInfo* typeInfo,
				Ptr<workflow::WfExpression> valueExpression,
				GuiResourceTextPos tagPosition,
				GuiResourceError::List& errors
				)override
			{
				auto stringExpr = valueExpression.Cast<WfStringExpression>();
				Ptr<WfExpression> propertyExpression;
				{
					propertyExpression = Workflow_ParseExpression(precompileContext, { resolvingResult.resource }, stringExpr->value.value, tagPosition, errors);
					if (!propertyExpression)
					{
						return nullptr;
					}
				};

				vint indexItemType = resolvingResult.envVars.Keys().IndexOf(GlobalStringKey::Get(L"ItemType"));
				if (indexItemType == -1)
				{
					auto error
						= L"Precompile: env.ItemType must be specified before deserializing \""
						+ stringExpr->value.value
						+ L"\" to value of type \""
						+ typeInfo->GetTypeFriendlyName()
						+ L"\".";
					errors.Add(GuiResourceError({ resolvingResult.resource }, tagPosition, error));
					return nullptr;
				}

				Ptr<WfType> itemType;
				{
					const auto& values = resolvingResult.envVars.GetByIndex(indexItemType);
					auto itemTypeValue = values[values.Count() - 1];

					itemType = Workflow_ParseType(precompileContext, { resolvingResult.resource }, itemTypeValue->value, itemTypeValue->valuePosition, errors);
					if (!itemType)
					{
						return nullptr;
					}
				};

				vint indexItemName = resolvingResult.envVars.Keys().IndexOf(GlobalStringKey::Get(L"ItemName"));
				WString itemName(L"item", false);
				if (indexItemName != -1)
				{
					const auto& values = resolvingResult.envVars.GetByIndex(indexItemName);
					itemName = values[values.Count() - 1]->value;
				}

				if (auto refExpr = propertyExpression.Cast<WfReferenceExpression>())
				{
					if (refExpr->name.value != itemName)
					{
						auto refItem = MakePtr<WfReferenceExpression>();
						refItem->name.value = itemName;

						auto member = MakePtr<WfMemberExpression>();
						member->parent = refItem;
						member->name.value = refExpr->name.value;

						propertyExpression = member;
					}
				}

				bool isWritableItemProperty = IsWritableItemPropertyType(typeInfo);

				auto funcDecl = MakePtr<WfFunctionDeclaration>();
				ITypeInfo* acceptValueType = nullptr;
				funcDecl->anonymity = WfFunctionAnonymity::Anonymous;
				{
					auto genericType = typeInfo->GetElementType();
					funcDecl->returnType = GetTypeFromTypeInfo(genericType->GetGenericArgument(0));
					{
						auto argument = MakePtr<WfFunctionArgument>();
						argument->name.value = L"<item>";
						argument->type = GetTypeFromTypeInfo(genericType->GetGenericArgument(1));
						funcDecl->arguments.Add(argument);
					}

					if (isWritableItemProperty)
					{
						{
							auto argument = MakePtr<WfFunctionArgument>();
							argument->name.value = L"<value>";
							argument->type = GetTypeFromTypeInfo((acceptValueType = genericType->GetGenericArgument(2)));
							funcDecl->arguments.Add(argument);
						}
						{
							auto argument = MakePtr<WfFunctionArgument>();
							argument->name.value = L"<update>";
							argument->type = GetTypeFromTypeInfo(genericType->GetGenericArgument(3));
							funcDecl->arguments.Add(argument);
						}
					}
				}

				auto funcBlock = MakePtr<WfBlockStatement>();
				funcDecl->statement = funcBlock;

				{
					auto refItem = MakePtr<WfReferenceExpression>();
					refItem->name.value = L"<item>";

					auto refCast = MakePtr<WfTypeCastingExpression>();
					refCast->strategy = WfTypeCastingStrategy::Strong;
					refCast->type = itemType;
					refCast->expression = refItem;

					auto varDecl = MakePtr<WfVariableDeclaration>();
					varDecl->name.value = itemName;
					varDecl->expression = refCast;

					auto varStat = MakePtr<WfVariableStatement>();
					varStat->variable = varDecl;
					funcBlock->statements.Add(varStat);
				}

				Ptr<WfReturnStatement> returnStat;
				{
					returnStat = MakePtr<WfReturnStatement>();
					returnStat->expression = propertyExpression;
				}

				if (isWritableItemProperty)
				{
					auto ifStat = MakePtr<WfIfStatement>();
					funcBlock->statements.Add(ifStat);
					{
						auto refUpdate = MakePtr<WfReferenceExpression>();
						refUpdate->name.value = L"<update>";

						ifStat->expression = refUpdate;
					}
					{
						auto block = MakePtr<WfBlockStatement>();
						ifStat->trueBranch = block;

						{
							auto refValue = MakePtr<WfReferenceExpression>();
							refValue->name.value = L"<value>";

							auto assignExpr = MakePtr<WfBinaryExpression>();
							assignExpr->op = WfBinaryOperator::Assign;
							assignExpr->first = CopyExpression(propertyExpression, true);

							if (acceptValueType->GetTypeDescriptor()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Object)
							{
								auto castExpr = MakePtr<WfExpectedTypeCastExpression>();
								castExpr->strategy = WfTypeCastingStrategy::Strong;
								castExpr->expression = refValue;
								assignExpr->second = castExpr;
							}
							else
							{
								assignExpr->second = refValue;
							}

							auto stat = MakePtr<WfExpressionStatement>();
							stat->expression = assignExpr;
							block->statements.Add(stat);
						}
						{
							auto returnStat = MakePtr<WfReturnStatement>();
							block->statements.Add(returnStat);

							auto returnType = typeInfo->GetElementType()->GetGenericArgument(0);
							returnStat->expression = CreateDefaultValue(returnType);
						}
					}
					{
						auto block = MakePtr<WfBlockStatement>();
						ifStat->falseBranch = block;

						block->statements.Add(returnStat);
					}
				}
				else
				{
					funcBlock->statements.Add(returnStat);
				}

				auto funcExpr = MakePtr<WfFunctionExpression>();
				funcExpr->function = funcDecl;
				return funcExpr;
			}
		};

/***********************************************************************
GuiDataProcessorDeserializer
***********************************************************************/

		class GuiDataProcessorDeserializer : public Object, public IGuiInstanceDeserializer
		{
		protected:
			Ptr<ITypeInfo>						stringType;

		public:
			GuiDataProcessorDeserializer()
			{
				stringType = TypeInfoRetriver<WString>::CreateTypeInfo();
			}

			bool CanDeserialize(const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo)override
			{
				return typeInfo->GetTypeDescriptor() == description::GetTypeDescriptor<IDataFilter>()
					|| typeInfo->GetTypeDescriptor() == description::GetTypeDescriptor<IDataSorter>();
			}

			description::ITypeInfo* DeserializeAs(const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo)override
			{
				return stringType.Obj();
			}

			Ptr<workflow::WfExpression> Deserialize(
				GuiResourcePrecompileContext& precompileContext,
				types::ResolvingResult& resolvingResult,
				const IGuiInstanceLoader::PropertyInfo& propertyInfo,
				description::ITypeInfo* typeInfo,
				Ptr<workflow::WfExpression> valueExpression,
				GuiResourceTextPos tagPosition,
				GuiResourceError::List& errors
				)override
			{
				auto stringExpr = valueExpression.Cast<WfStringExpression>();
				Ptr<WfExpression> propertyExpression;
				{
					propertyExpression = Workflow_ParseExpression(precompileContext, { resolvingResult.resource }, stringExpr->value.value, tagPosition, errors);
					if (!propertyExpression)
					{
						return nullptr;
					}
				};

				vint indexItemType = resolvingResult.envVars.Keys().IndexOf(GlobalStringKey::Get(L"ItemType"));
				if (indexItemType == -1)
				{
					auto error
						= L"Precompile: env.ItemType must be specified before deserializing \""
						+ stringExpr->value.value
						+ L"\" to value of type \""
						+ typeInfo->GetTypeFriendlyName()
						+ L"\".";
					errors.Add(GuiResourceError({ resolvingResult.resource }, tagPosition, error));
					return nullptr;
				}

				Ptr<WfType> itemType;
				{
					const auto& values = resolvingResult.envVars.GetByIndex(indexItemType);
					auto itemTypeValue = values[values.Count() - 1];

					itemType = Workflow_ParseType(precompileContext, { resolvingResult.resource }, itemTypeValue->value, itemTypeValue->valuePosition, errors);
					if (!itemType)
					{
						return nullptr;
					}
				};

				auto newExpr = MakePtr<WfNewInterfaceExpression>();
				newExpr->type = GetTypeFromTypeInfo(typeInfo);
				{
					auto decl = MakePtr<WfFunctionDeclaration>();
					newExpr->declarations.Add(decl);
					decl->classMember = MakePtr<WfClassMember>();
					decl->classMember->kind = WfClassMemberKind::Override;
					decl->name.value = L"SetCallback";
					decl->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());
					{
						auto argument = MakePtr<WfFunctionArgument>();
						argument->type = GetTypeFromTypeInfo(TypeInfoRetriver<IDataProcessorCallback*>::CreateTypeInfo().Obj());
						argument->name.value = L"value";
						decl->arguments.Add(argument);
					}

					auto block = MakePtr<WfBlockStatement>();
					decl->statement = block;
				}
				{
					auto decl = MakePtr<WfFunctionDeclaration>();
					newExpr->declarations.Add(decl);
					decl->classMember = MakePtr<WfClassMember>();
					decl->classMember->kind = WfClassMemberKind::Override;

					List<WString> argumentNames;
					if (typeInfo->GetTypeDescriptor() == description::GetTypeDescriptor<IDataFilter>())
					{
						decl->name.value = L"Filter";
						decl->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<bool>::CreateTypeInfo().Obj());
						argumentNames.Add(L"<row>");
					}
					else
					{
						decl->name.value = L"Compare";
						decl->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<vint>::CreateTypeInfo().Obj());
						argumentNames.Add(L"<row1>");
						argumentNames.Add(L"<row2>");
					}

					FOREACH(WString, name, argumentNames)
					{
						auto argument = MakePtr<WfFunctionArgument>();
						argument->type = GetTypeFromTypeInfo(TypeInfoRetriver<Value>::CreateTypeInfo().Obj());
						argument->name.value = name;
						decl->arguments.Add(argument);
					}

					auto block = MakePtr<WfBlockStatement>();
					decl->statement = block;

					auto inferExpr = MakePtr<WfInferExpression>();
					inferExpr->expression = propertyExpression;
					{
						auto funcType = MakePtr<WfFunctionType>();
						inferExpr->type = funcType;

						funcType->result = CopyType(decl->returnType);
						for (vint i = 0; i < decl->arguments.Count(); i++)
						{
							funcType->arguments.Add(CopyType(itemType));
						}
					}

					auto callExpr = MakePtr<WfCallExpression>();
					callExpr->function = inferExpr;
					FOREACH_INDEXER(WString, name, index, argumentNames)
					{
						auto refExpr = MakePtr<WfReferenceExpression>();
						refExpr->name.value = name;

						auto castExpr = MakePtr<WfTypeCastingExpression>();
						castExpr->strategy = WfTypeCastingStrategy::Strong;
						castExpr->type = (index == 0 ? itemType : CopyType(itemType));
						castExpr->expression = refExpr;

						callExpr->arguments.Add(castExpr);
					}

					auto stat = MakePtr<WfReturnStatement>();
					stat->expression = callExpr;
					block->statements.Add(stat);
				}
				return newExpr;
			}
		};

/***********************************************************************
GuiPredefinedInstanceDeserializersPlugin
***********************************************************************/

		class GuiPredefinedInstanceDeserializersPlugin : public Object, public IGuiPlugin
		{
		public:

			GUI_PLUGIN_NAME(GacUI_Instance_Deserializers)
			{
				GUI_PLUGIN_DEPEND(GacUI_Instance);
			}

			void Load()override
			{
				IGuiInstanceLoaderManager* manager = GetInstanceLoaderManager();
				manager->AddInstanceDeserializer(new GuiTemplatePropertyDeserializer);
				manager->AddInstanceDeserializer(new GuiItemPropertyDeserializer);
				manager->AddInstanceDeserializer(new GuiDataProcessorDeserializer);
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedInstanceDeserializersPlugin)
	}
}
