#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

/***********************************************************************
GuiCommonDatePickerLookLoader
***********************************************************************/

			class GuiCommonDatePickerLookLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;
				GlobalStringKey					_BackgroundColor;
				GlobalStringKey					_PrimaryTextColor;
				GlobalStringKey					_SecondaryTextColor;

			public:
				GuiCommonDatePickerLookLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiCommonDatePickerLook>::content.typeName);
					_BackgroundColor = GlobalStringKey::Get(L"BackgroundColor");
					_PrimaryTextColor = GlobalStringKey::Get(L"PrimaryTextColor");
					_SecondaryTextColor = GlobalStringKey::Get(L"SecondaryTextColor");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetRequiredPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(_BackgroundColor);
						propertyNames.Add(_PrimaryTextColor);
						propertyNames.Add(_SecondaryTextColor);
					}
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					GetRequiredPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _BackgroundColor || propertyInfo.propertyName == _PrimaryTextColor || propertyInfo.propertyName == _SecondaryTextColor)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<Color>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool CanCreate(const TypeInfo& typeInfo)override
				{
					return typeInfo.typeName == typeName;
				}

				Ptr<workflow::WfStatement> CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
				{
					if (CanCreate(typeInfo))
					{
						vint indexBackgroundColor = arguments.Keys().IndexOf(_BackgroundColor);
						vint indexPrimaryTextColor = arguments.Keys().IndexOf(_PrimaryTextColor);
						vint indexSecondaryTextColor = arguments.Keys().IndexOf(_SecondaryTextColor);
						if (indexBackgroundColor != -1 && indexPrimaryTextColor != -1 && indexSecondaryTextColor != -1)
						{
							auto type = TypeInfoRetriver<GuiCommonDatePickerLook*>::CreateTypeInfo();
							auto createExpr = MakePtr<WfNewClassExpression>();
							createExpr->type = GetTypeFromTypeInfo(type.Obj());
							createExpr->arguments.Add(arguments.GetByIndex(indexBackgroundColor)[0].expression);
							createExpr->arguments.Add(arguments.GetByIndex(indexPrimaryTextColor)[0].expression);
							createExpr->arguments.Add(arguments.GetByIndex(indexSecondaryTextColor)[0].expression);

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
GuiCommonScrollViewLookLoader
***********************************************************************/

			class GuiCommonScrollViewLookLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;
				GlobalStringKey					_DefaultScrollSize;

			public:
				GuiCommonScrollViewLookLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiCommonScrollViewLook>::content.typeName);
					_DefaultScrollSize = GlobalStringKey::Get(L"DefaultScrollSize");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetRequiredPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(_DefaultScrollSize);
					}
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					GetRequiredPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _DefaultScrollSize)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<vint>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool CanCreate(const TypeInfo& typeInfo)override
				{
					return typeInfo.typeName == typeName;
				}

				Ptr<workflow::WfStatement> CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
				{
					if (CanCreate(typeInfo))
					{
						vint indexDefaultScrollSize = arguments.Keys().IndexOf(_DefaultScrollSize);
						if (indexDefaultScrollSize != -1)
						{
							auto type = TypeInfoRetriver<GuiCommonScrollViewLook*>::CreateTypeInfo();
							auto createExpr = MakePtr<WfNewClassExpression>();
							createExpr->type = GetTypeFromTypeInfo(type.Obj());
							createExpr->arguments.Add(arguments.GetByIndex(indexDefaultScrollSize)[0].expression);

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
Initialization
***********************************************************************/

			void LoadTemplates(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiCommonDatePickerLookLoader);
				manager->SetLoader(new GuiCommonScrollViewLookLoader);
			}
		}
	}
}

#endif
