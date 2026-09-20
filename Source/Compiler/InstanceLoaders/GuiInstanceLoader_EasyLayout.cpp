#include "GuiInstanceLoader_EasyLayout.h"

namespace vl::presentation
{
	using namespace collections;
	using namespace reflection::description;
	using namespace compositions;
	using namespace compositions::eazy_layout;
	using namespace controls;
	using namespace workflow;
	using namespace workflow::analyzer;

	bool IsEasyLayoutConstantProperty(const IGuiInstanceLoader::PropertyInfo& propertyInfo)
	{
		if (!propertyInfo.typeInfo.typeInfo) return false;
		auto td = propertyInfo.typeInfo.typeInfo->GetTypeDescriptor();
		return (propertyInfo.propertyName == GlobalStringKey::Get(L"CellOption") && td->CanConvertTo(description::GetTypeDescriptor<GuiEasyCellLayout>()))
			|| (propertyInfo.propertyName == GlobalStringKey::Get(L"Percentage") && td->CanConvertTo(description::GetTypeDescriptor<GuiEasyFillLayout>()));
	}

/***********************************************************************
Static grammar and final initialization
***********************************************************************/

	enum class EasyKind { Other, Root, Top, Bottom, Left, Right, Row, Column, Fill, Splitter };

	EasyKind GetEasyKind(ITypeDescriptor* td)
	{
		if (!td) return EasyKind::Other;
#define EASY_KIND(TYPE, KIND) if (td->CanConvertTo(description::GetTypeDescriptor<TYPE>())) return EasyKind::KIND;
		EASY_KIND(GuiEasyLayoutComposition, Root)
		EASY_KIND(GuiEasyTopLayout, Top)
		EASY_KIND(GuiEasyBottomLayout, Bottom)
		EASY_KIND(GuiEasyLeftLayout, Left)
		EASY_KIND(GuiEasyRightLayout, Right)
		EASY_KIND(GuiEasyRowLayout, Row)
		EASY_KIND(GuiEasyColumnLayout, Column)
		EASY_KIND(GuiEasyFillLayout, Fill)
		EASY_KIND(GuiEasySplitterLayout, Splitter)
#undef EASY_KIND
		return EasyKind::Other;
	}

	ITypeDescriptor* GetEasyReprType(types::ResolvingResult& result, GuiAttSetterRepr* repr)
	{
		if (!repr || !result.typeInfos.Keys().Contains(repr->instanceName)) return nullptr;
		return result.typeInfos[repr->instanceName].typeInfo->GetTypeDescriptor();
	}

	void VisitEasyLayouts(types::ResolvingResult& result, GuiAttSetterRepr* repr, EasyKind parentKind, GuiResourceError::List* errors, Ptr<WfBlockStatement> statements)
	{
		auto kind = GetEasyKind(GetEasyReprType(result, repr));
		bool groupingRow = kind == EasyKind::Row && parentKind != EasyKind::Column;
		bool groupingColumn = kind == EasyKind::Column && parentKind != EasyKind::Row;
		vint payloadCount = 0, layoutCount = 0;
		bool vertical = false, horizontal = false, fills = false, tracks = false;
		for (auto [setter, index] : indexed(repr->setters.Values()))
		{
			auto property = repr->setters.Keys()[index];
			bool content = property == GlobalStringKey::Empty || property == GlobalStringKey::Get(L"Layouts") || property == GlobalStringKey::Get(L"Composition");
			EasyKind previous = EasyKind::Other;
			for (auto value : setter->values)
			{
				auto child = value.Cast<GuiAttSetterRepr>();
				if (!child) continue;
				auto td = GetEasyReprType(result, child.Obj());
				auto childKind = GetEasyKind(td);
				if (errors && kind != EasyKind::Other && content)
				{
					bool descriptor = childKind != EasyKind::Other && childKind != EasyKind::Root;
					if (descriptor) layoutCount++; else payloadCount++;
					if (kind == EasyKind::Splitter)
					{
						errors->Add(GuiResourceError({ result.resource }, child->tagPosition, L"Easy layout: splitters cannot contain descriptors or a payload."));
					}
					if (childKind == EasyKind::Splitter && (previous == EasyKind::Other || previous == EasyKind::Root || previous == EasyKind::Splitter))
					{
						errors->Add(GuiResourceError({ result.resource }, child->tagPosition, L"Easy layout: a splitter must immediately follow an ordinary descriptor."));
					}
					previous = childKind;
					if (childKind != EasyKind::Splitter && ((groupingRow && childKind != EasyKind::Column) || (groupingColumn && childKind != EasyKind::Row)))
					{
						errors->Add(GuiResourceError({ result.resource }, child->tagPosition, L"Easy layout: outer rows/columns only accept opposite-axis track descriptors."));
					}
					if (!descriptor && td && !td->CanConvertTo(description::GetTypeDescriptor<GuiBoundsComposition>()) && !td->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
					{
						errors->Add(GuiResourceError({ result.resource }, child->tagPosition, L"Easy layout: a payload must be a control or bounds composition; keep a cell/stack item's required parent."));
					}
					vertical |= childKind == EasyKind::Top || childKind == EasyKind::Bottom || childKind == EasyKind::Row;
					horizontal |= childKind == EasyKind::Left || childKind == EasyKind::Right || childKind == EasyKind::Column;
					fills |= childKind == EasyKind::Fill;
					tracks |= childKind == EasyKind::Row || childKind == EasyKind::Column;
				}
				VisitEasyLayouts(result, child.Obj(), content ? kind : EasyKind::Other, errors, statements);
			}
		}
		if (errors && kind != EasyKind::Other)
		{
			if (payloadCount > 1 || (payloadCount && layoutCount))
			{
				errors->Add(GuiResourceError({ result.resource }, repr->tagPosition, L"Easy layout: use descriptor children or one composition/control payload, without mixing them."));
			}
			if (!groupingRow && !groupingColumn && ((vertical && horizontal) || (fills && tracks)))
			{
				errors->Add(GuiResourceError({ result.resource }, repr->tagPosition, L"Easy layout: incompatible sibling descriptor kinds."));
			}
		}
		if (statements && kind == EasyKind::Root)
		{
			auto target = Ptr(new WfReferenceExpression);
			target->name.value = repr->instanceName.ToString();
			auto method = Ptr(new WfMemberExpression);
			method->parent = target;
			method->name.value = L"BuildLayout";
			auto call = Ptr(new WfCallExpression);
			call->function = method;
			auto statement = Ptr(new WfExpressionStatement);
			statement->expression = call;
			statements->statements.Add(statement);
		}
	}

	void Workflow_ValidateEasyLayouts(types::ResolvingResult& resolvingResult, GuiResourceError::List& errors)
	{
		VisitEasyLayouts(resolvingResult, resolvingResult.context->instance.Obj(), EasyKind::Other, &errors, nullptr);
	}

	void Workflow_BuildEasyLayouts(types::ResolvingResult& resolvingResult, Ptr<WfBlockStatement> statements)
	{
		VisitEasyLayouts(resolvingResult, resolvingResult.context->instance.Obj(), EasyKind::Other, nullptr, statements);
	}

	namespace instance_loaders
	{
/***********************************************************************
Descriptor and root child loaders
***********************************************************************/

		template<typename T>
		class GuiEasyInstanceLoader : public Object, public IGuiInstanceLoader
		{
		public:
			GlobalStringKey GetTypeName() override
			{
				return GlobalStringKey::Get(description::TypeInfo<T>::content.typeName);
			}

			void GetPropertyNames(GuiResourcePrecompileContext& precompileContext, const TypeInfo& typeInfo, List<GlobalStringKey>& propertyNames) override
			{
				propertyNames.Add(GlobalStringKey::Empty);
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(GuiResourcePrecompileContext& precompileContext, const PropertyInfo& propertyInfo) override
			{
				if (propertyInfo.propertyName == GlobalStringKey::Empty)
				{
					auto info = GuiInstancePropertyInfo::Collection(nullptr);
					info->acceptableTypes.Add(TypeInfoRetriver<Ptr<GuiEasyLayout>>::CreateTypeInfo());
					info->acceptableTypes.Add(TypeInfoRetriver<GuiControl*>::CreateTypeInfo());
					info->acceptableTypes.Add(TypeInfoRetriver<GuiGraphicsComposition*>::CreateTypeInfo());
					return info;
				}
				if (IsEasyLayoutConstantProperty(propertyInfo))
				{
					return GuiInstancePropertyInfo::Assign(propertyInfo.propertyName == GlobalStringKey::Get(L"CellOption")
						? TypeInfoRetriver<GuiCellOption>::CreateTypeInfo() : TypeInfoRetriver<double>::CreateTypeInfo());
				}
				return nullptr;
			}

			Ptr<WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors) override
			{
				auto block = Ptr(new WfBlockStatement);
				for (auto [property, index] : indexed(arguments.Keys()))
				for (auto argument : arguments.GetByIndex(index))
				{
					auto target = Ptr(new WfReferenceExpression);
					target->name.value = variableName.ToString();
					auto member = Ptr(new WfMemberExpression);
					member->parent = target;
					Ptr<WfExpression> expression;
					if (property == GlobalStringKey::Empty && argument.typeInfo->GetTypeDescriptor()->CanConvertTo(description::GetTypeDescriptor<GuiEasyLayout>()))
					{
						member->name.value = L"Layouts";
						auto add = Ptr(new WfMemberExpression);
						add->parent = member;
						add->name.value = L"Add";
						auto call = Ptr(new WfCallExpression);
						call->function = add;
						call->arguments.Add(argument.expression);
						expression = call;
					}
					else
					{
						member->name.value = property == GlobalStringKey::Empty ? L"Composition" : property.ToString();
						auto assign = Ptr(new WfBinaryExpression);
						assign->op = WfBinaryOperator::Assign;
						assign->first = member;
						assign->second = argument.expression;
						if (property == GlobalStringKey::Empty && argument.typeInfo->GetTypeDescriptor()->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
						{
							auto bounds = Ptr(new WfMemberExpression);
							bounds->parent = argument.expression;
							bounds->name.value = L"BoundsComposition";
							assign->second = bounds;
						}
						expression = assign;
					}
					auto statement = Ptr(new WfExpressionStatement);
					statement->expression = expression;
					block->statements.Add(statement);
				}
				return block;
			}
		};

/***********************************************************************
CellOption constant expressions
***********************************************************************/

		bool IsEasyNumericConstant(Ptr<WfExpression> expression)
		{
			if (expression.Cast<WfIntegerExpression>() || expression.Cast<WfFloatingExpression>()) return true;
			if (auto unary = expression.Cast<WfUnaryExpression>()) return IsEasyNumericConstant(unary->operand);
			if (auto binary = expression.Cast<WfBinaryExpression>())
			{
				return binary->op != WfBinaryOperator::Assign && binary->op != WfBinaryOperator::Index && binary->op != WfBinaryOperator::FailedThen
					&& IsEasyNumericConstant(binary->first) && IsEasyNumericConstant(binary->second);
			}
			return false;
		}

		WString EasyQualifiedName(Ptr<WfExpression> expression)
		{
			if (auto reference = expression.Cast<WfReferenceExpression>()) return reference->name.value;
			if (auto child = expression.Cast<WfChildExpression>()) return EasyQualifiedName(child->parent) + L"::" + child->name.value;
			return L"";
		}

		class GuiEasyCellOptionDeserializer : public Object, public IGuiInstanceDeserializer
		{
		public:
			bool CanDeserialize(const IGuiInstanceLoader::PropertyInfo& propertyInfo, ITypeInfo* typeInfo) override
			{
				return IsEasyLayoutConstantProperty(propertyInfo) && typeInfo->GetTypeDescriptor() == description::GetTypeDescriptor<GuiCellOption>();
			}

			ITypeInfo* DeserializeAs(const IGuiInstanceLoader::PropertyInfo& propertyInfo, ITypeInfo* typeInfo) override
			{
				return typeInfo;
			}

			Ptr<WfExpression> Deserialize(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& result, const IGuiInstanceLoader::PropertyInfo& propertyInfo, ITypeInfo* typeInfo, Ptr<WfExpression> expression, GuiResourceTextPos position, GuiResourceError::List& errors) override
			{
				auto infer = expression.Cast<WfInferExpression>();
				auto fields = infer ? infer->expression.Cast<WfConstructorExpression>() : nullptr;
				bool valid = fields != nullptr;
				if (fields)
				for (auto field : fields->arguments)
				{
					auto key = field->key.Cast<WfReferenceExpression>();
					bool fieldValid = false;
					if (key && key->name.value == L"composeType")
					{
						auto name = EasyQualifiedName(field->value);
						for (auto item : { L"MinSize",L"Absolute",L"Percentage" })
						{
							if (name == item || name == L"presentation::compositions::GuiCellOption::ComposeType::" + WString(item))
							{
								auto value = Ptr(new WfChildExpression);
								value->parent = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<GuiCellOption::ComposeType>());
								value->name.value = item;
								field->value = value;
								fieldValid = true;
							}
						}
					}
					else if (key && (key->name.value == L"absolute" || key->name.value == L"percentage"))
					{
						fieldValid = IsEasyNumericConstant(field->value);
					}
					valid &= fieldValid;
				}
				if (!valid)
				{
					errors.Add(GuiResourceError({ result.resource }, position, L"Easy layout: CellOption fields require numeric constant expressions and a literal ComposeType enum value; runtime expressions are not allowed."));
					return nullptr;
				}
				return expression;
			}
		};

		void LoadEasyLayouts(IGuiInstanceLoaderManager* manager)
		{
			manager->SetLoader(Ptr(new GuiEasyInstanceLoader<GuiEasyLayoutComposition>));
			manager->SetLoader(Ptr(new GuiEasyInstanceLoader<GuiEasyLayout>));
			manager->AddInstanceDeserializer(Ptr(new GuiEasyCellOptionDeserializer));
		}
	}
}
