#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace stream;
		using namespace reflection::description;
		using namespace workflow;
		using namespace workflow::analyzer;

/***********************************************************************
Parser
***********************************************************************/

		template<typename T>
		Ptr<T> Workflow_Parse(GuiResourcePrecompileContext& precompileContext, const WString& parserName, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
		{
			vint errorCount = errors.Count();
			auto parser = GetParserManager()->GetParser<T>(parserName);
			auto result = parser->Parse(location, code, position, errors);

			if (availableAfter.row != 0 || availableAfter.column != 0)
			{
				for (vint i = errorCount; i < errors.Count(); i++)
				{
					auto& error = errors[i];
					if (error.position.row > position.row)
					{
						error.position.row -= availableAfter.row;
					}
					else if (error.position.row == position.row && error.position.column >= position.column)
					{
						error.position.column -= availableAfter.column;
					}
				}
			}

			if (result)
			{
				Workflow_RecordScriptPosition(precompileContext, position, result, availableAfter);
			}
			return result;
		}

		Ptr<workflow::WfType> Workflow_ParseType(GuiResourcePrecompileContext& precompileContext, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
		{
			return Workflow_Parse<WfType>(precompileContext, L"WORKFLOW-TYPE", location, code, position, errors, availableAfter);
		}

		Ptr<workflow::WfExpression> Workflow_ParseExpression(GuiResourcePrecompileContext& precompileContext, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
		{
			return Workflow_Parse<WfExpression>(precompileContext, L"WORKFLOW-EXPRESSION", location, code, position, errors, availableAfter);
		}

		Ptr<workflow::WfStatement> Workflow_ParseStatement(GuiResourcePrecompileContext& precompileContext, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
		{
			return Workflow_Parse<WfStatement>(precompileContext, L"WORKFLOW-STATEMENT", location, code, position, errors, availableAfter);
		}

		Ptr<workflow::WfStatement> Workflow_ParseCoProviderStatement(GuiResourcePrecompileContext& precompileContext, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
		{
			return Workflow_Parse<WfStatement>(precompileContext, L"WORKFLOW-COPROVIDER-STATEMENT", location, code, position, errors, availableAfter);
		}

		Ptr<workflow::WfModule> Workflow_ParseModule(GuiResourcePrecompileContext& precompileContext, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
		{
			return Workflow_Parse<WfModule>(precompileContext, L"WORKFLOW-MODULE", location, code, position, errors, availableAfter);
		}

/***********************************************************************
Workflow_ModuleToString
***********************************************************************/

		WString Workflow_ModuleToString(Ptr<workflow::WfModule> module)
		{
			return GenerateToStream([&](StreamWriter& writer)
			{
				WfPrint(module, L"", writer);
			});
		}

/***********************************************************************
Converter
***********************************************************************/

		Ptr<workflow::WfExpression> Workflow_ParseTextValue(GuiResourcePrecompileContext& precompileContext, description::ITypeDescriptor* typeDescriptor, GuiResourceLocation location, const WString& textValue, GuiResourceTextPos position, collections::List<GuiResourceError>& errors)
		{
			if (typeDescriptor == description::GetTypeDescriptor<WString>())
			{
				auto valueExpr = MakePtr<WfStringExpression>();
				valueExpr->value.value = textValue;
				return valueExpr;
			}
			else if (typeDescriptor == description::GetTypeDescriptor<bool>())
			{
				bool value = false;
				if (!TypedValueSerializerProvider<bool>::Deserialize(textValue, value)) return nullptr;
				auto valueExpr = MakePtr<WfLiteralExpression>();
				valueExpr->value = value ? WfLiteralValue::True : WfLiteralValue::False;
				return valueExpr;
			}
#define INTEGER_BRANCH(TYPE) \
			else if (typeDescriptor == description::GetTypeDescriptor<TYPE>()) \
			{ \
				auto valueExpr = MakePtr<WfIntegerExpression>(); \
				valueExpr->value.value = textValue; \
				auto type = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal); \
				auto infer = MakePtr<WfInferExpression>(); \
				infer->type = GetTypeFromTypeInfo(type.Obj()); \
				infer->expression = valueExpr; \
				return infer; \
			}
			INTEGER_BRANCH(vint8_t)
			INTEGER_BRANCH(vint16_t)
			INTEGER_BRANCH(vint32_t)
			INTEGER_BRANCH(vint64_t)
			INTEGER_BRANCH(vuint8_t)
			INTEGER_BRANCH(vuint16_t)
			INTEGER_BRANCH(vuint32_t)
			INTEGER_BRANCH(vuint64_t)
#undef INTEGER_BRANCH
				
#define FLOATING_BRANCH(TYPE) \
			else if (typeDescriptor == description::GetTypeDescriptor<TYPE>()) \
			{ \
				auto valueExpr = MakePtr<WfFloatingExpression>(); \
				valueExpr->value.value = textValue; \
				auto type = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal); \
				auto infer = MakePtr<WfInferExpression>(); \
				infer->type = GetTypeFromTypeInfo(type.Obj()); \
				infer->expression = valueExpr; \
				return infer; \
			}
			FLOATING_BRANCH(float)
			FLOATING_BRANCH(double)
#undef FLOATING_BRANCH

			else if (typeDescriptor->GetSerializableType())
			{
				auto str = MakePtr<WfStringExpression>();
				str->value.value = textValue;

				auto type = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal);

				auto cast = MakePtr<WfTypeCastingExpression>();
				cast->type = GetTypeFromTypeInfo(type.Obj());
				cast->strategy = WfTypeCastingStrategy::Strong;
				cast->expression = str;

				return cast;
			}
			else if (typeDescriptor->GetTypeDescriptorFlags() == TypeDescriptorFlags::Struct)
			{
				if (auto valueExpr = Workflow_ParseExpression(precompileContext, location, L"{" + textValue + L"}", position, errors, { 0,1 })) // {
				{
					auto type = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal);

					auto infer = MakePtr<WfInferExpression>();
					infer->type = GetTypeFromTypeInfo(type.Obj());
					infer->expression = valueExpr;

					return infer;
				}
				return nullptr;
			}
			else if ((typeDescriptor->GetTypeDescriptorFlags() & TypeDescriptorFlags::EnumType) != TypeDescriptorFlags::Undefined)
			{
				if (auto valueExpr = Workflow_ParseExpression(precompileContext, location, L"(" + textValue + L")", position, errors, { 0,1 })) // {
				{
					auto type = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal);

					auto infer = MakePtr<WfInferExpression>();
					infer->type = GetTypeFromTypeInfo(type.Obj());
					infer->expression = valueExpr;

					return infer;
				}
				return nullptr;
			}
			else
			{
				CHECK_FAIL(L"vl::presentation::Workflow_ParseTextValue(ITypeDescriptor*, const WString&, GuiResourceError::List&)#This is not a value type.");
			}
		}
	}
}