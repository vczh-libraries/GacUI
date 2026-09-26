#include "../WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"
#include "../../GraphicsComposition/EazyLayout/GuiEasyLayout.h"
#include "../../Application/Controls/GuiBasicControls.h"

namespace vl::presentation::instance_loaders
{
	using namespace collections;
	using namespace reflection::description;
	using namespace compositions;
	using namespace compositions::eazy_layout;
	using namespace controls;
	using namespace workflow;

/***********************************************************************
GuiEasyInstanceLoader
***********************************************************************/

	class GuiEasyInstanceLoaderState : public Object
	{
	public:
		SortedList<GlobalStringKey>				compositions;
	};

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
			if (propertyInfo.propertyName == GlobalStringKey::Get(L"Composition"))
			{
				return GuiInstancePropertyInfo::Assign(TypeInfoRetriver<GuiGraphicsComposition*>::CreateTypeInfo());
			}
			return nullptr;
		}

		Ptr<WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors) override
		{
			vint stateIndex = resolvingResult.loaderStates.Keys().IndexOf(this);
			auto state = stateIndex == -1
				? Ptr(new GuiEasyInstanceLoaderState)
				: resolvingResult.loaderStates.Values()[stateIndex].Cast<GuiEasyInstanceLoaderState>();
			if (stateIndex == -1)
			{
				resolvingResult.loaderStates.Add(this, state);
			}
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
					if (state->compositions.Contains(variableName))
					{
						errors.Add(GuiResourceError({ resolvingResult.resource }, argument.valuePosition,
							L"Easy layout: initial content cannot contain more than one composition/control payload."));
						return nullptr;
					}
					state->compositions.Add(variableName);
					member->name.value = L"Composition";

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

		Ptr<WfStatement> InitializeInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, GuiResourceTextPos tagPosition, GuiResourceError::List& errors) override
		{
			if constexpr (std::is_same_v<T, GuiEasyLayoutComposition>)
			{
				auto target = Ptr(new WfReferenceExpression);
				target->name.value = variableName.ToString();
				auto method = Ptr(new WfMemberExpression);
				method->parent = target;
				method->name.value = L"BuildLayout";
				auto call = Ptr(new WfCallExpression);
				call->function = method;
				auto statement = Ptr(new WfExpressionStatement);
				statement->expression = call;
				return statement;
			}
			return nullptr;
		}
	};

	void LoadEasyLayouts(IGuiInstanceLoaderManager* manager)
	{
		manager->SetLoader(Ptr(new GuiEasyInstanceLoader<GuiEasyLayoutComposition>));
		manager->SetLoader(Ptr(new GuiEasyInstanceLoader<GuiEasyLayout>));
	}
}
