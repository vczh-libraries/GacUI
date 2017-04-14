#include "GuiInstanceLoader.h"
#include "../Controls/GuiApplication.h"
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
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedInstanceDeserializersPlugin)
	}
}
