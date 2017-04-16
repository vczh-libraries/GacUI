#include "GuiInstanceLoader.h"
#include "../Reflection/TypeDescriptors/GuiReflectionTemplates.h"
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
		using namespace templates;

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

			bool CanDeserialize(description::ITypeInfo* typeInfo)override
			{
				return IsItemPropertyType(typeInfo) || IsWritableItemPropertyType(typeInfo);
			}

			description::ITypeInfo* DeserializeAs(description::ITypeInfo* typeInfo)override
			{
				return stringType.Obj();
			}

			Ptr<workflow::WfExpression> Deserialize(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, description::ITypeInfo* typeInfo, Ptr<workflow::WfExpression> valueExpression, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
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
							argument->type = GetTypeFromTypeInfo(genericType->GetGenericArgument(2));
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
							assignExpr->first = CopyExpression(propertyExpression);
							assignExpr->second = refValue;

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

		public:
			GuiTemplatePropertyDeserializer()
			{
				stringType = TypeInfoRetriver<WString>::CreateTypeInfo();
			}

			bool CanDeserialize(description::ITypeInfo* typeInfo)override
			{
				return IsTemplatePropertyType(typeInfo);
			}

			description::ITypeInfo* DeserializeAs(description::ITypeInfo* typeInfo)override
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

				FOREACH(WString, controlTemplateName, typeNames)
				{
					auto controlTemplateTd = description::GetTypeDescriptor(controlTemplateName);
					if (!controlTemplateTd)
					{
						errors.Add(GuiResourceError({ resolvingResult.resource }, tagPosition,
							L"Precompile: Type \"" +
							typeNamesString +
							L"\" does not exist."));
						continue;
					}
					tds.Add(controlTemplateTd);
				}
			}

			static Ptr<WfExpression> CreateTemplateFactory(
				types::ResolvingResult& resolvingResult,
				List<ITypeDescriptor*>& controlTemplateTds,
				ITypeInfo* templateType,
				GuiResourceTextPos tagPosition,
				GuiResourceError::List& errors
				)
			{
				auto funcCreateTemplate = MakePtr<WfFunctionDeclaration>();
				funcCreateTemplate->anonymity = WfFunctionAnonymity::Anonymous;
				funcCreateTemplate->returnType = GetTypeFromTypeInfo(templateType);

				auto argViewModel = MakePtr<WfFunctionArgument>();
				argViewModel->type = GetTypeFromTypeInfo(TypeInfoRetriver<Value>::CreateTypeInfo().Obj());
				argViewModel->name.value = L"<viewModel>";
				funcCreateTemplate->arguments.Add(argViewModel);

				auto block = MakePtr<WfBlockStatement>();
				funcCreateTemplate->statement = block;

				ITypeDescriptor* stopControlTemplateTd = nullptr;
				FOREACH(ITypeDescriptor*, controlTemplateTd, controlTemplateTds)
				{
					if (!controlTemplateTd->CanConvertTo(templateType->GetTypeDescriptor()))
					{
						errors.Add(GuiResourceError({ resolvingResult.resource }, tagPosition,
							L"Precompile: Type \"" +
							controlTemplateTd->GetTypeName() +
							L"\" cannot be used here because it requires \"" +
							templateType->GetTypeDescriptor()->GetTypeName() +
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
						if (ctors->GetMethodCount() != 1)
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

			Ptr<workflow::WfExpression> Deserialize(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, description::ITypeInfo* typeInfo, Ptr<workflow::WfExpression> valueExpression, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
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

				List<ITypeDescriptor*> tds;
				GetItemTemplateType(resolvingResult, stringExpr->value.value, tds, tagPosition, errors);

				auto templateType = typeInfo->GetElementType()->GetGenericArgument(0);
				return CreateTemplateFactory(resolvingResult, tds, templateType, tagPosition, errors);
			}
		};

/***********************************************************************
GuiPredefinedInstanceDeserializersPlugin
***********************************************************************/

		class GuiPredefinedInstanceDeserializersPlugin : public Object, public IGuiPlugin
		{
		public:
			GuiPredefinedInstanceDeserializersPlugin()
			{
			}

			void Load()override
			{
			}

			void AfterLoad()override
			{
				IGuiInstanceLoaderManager* manager = GetInstanceLoaderManager();
				manager->AddInstanceDeserializer(new GuiItemPropertyDeserializer);
				manager->AddInstanceDeserializer(new GuiTemplatePropertyDeserializer);
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedInstanceDeserializersPlugin)
	}
}
