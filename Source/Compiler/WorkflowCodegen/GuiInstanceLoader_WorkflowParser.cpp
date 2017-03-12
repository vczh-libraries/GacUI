#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace reflection::description;
		using namespace workflow;
		using namespace workflow::analyzer;

/***********************************************************************
Parser
***********************************************************************/

		template<typename T>
		Ptr<T> Workflow_Parse(const WString& parserName, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
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

			return result;
		}

		Ptr<workflow::WfType> Workflow_ParseType(GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
		{
			return Workflow_Parse<WfType>(L"WORKFLOW-TYPE", location, code, position, errors, availableAfter);
		}

		Ptr<workflow::WfExpression> Workflow_ParseExpression(GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
		{
			return Workflow_Parse<WfExpression>(L"WORKFLOW-EXPRESSION", location, code, position, errors, availableAfter);
		}

		Ptr<workflow::WfStatement> Workflow_ParseStatement(GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
		{
			return Workflow_Parse<WfStatement>(L"WORKFLOW-STATEMENT", location, code, position, errors, availableAfter);
		}

		Ptr<workflow::WfModule> Workflow_ParseModule(GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter)
		{
			return Workflow_Parse<WfModule>(L"WORKFLOW-MODULE", location, code, position, errors, availableAfter);
		}

/***********************************************************************
Workflow_ModuleToString
***********************************************************************/

		WString Workflow_ModuleToString(Ptr<workflow::WfModule> module)
		{
			stream::MemoryStream stream;
			{
				stream::StreamWriter writer(stream);
				WfPrint(module, L"", writer);
			}
			stream.SeekFromBegin(0);
			stream::StreamReader reader(stream);
			return reader.ReadToEnd();
		}

/***********************************************************************
Converter
***********************************************************************/

		Ptr<workflow::WfExpression> Workflow_ParseTextValue(description::ITypeDescriptor* typeDescriptor, GuiResourceLocation location, const WString& textValue, GuiResourceTextPos position, collections::List<GuiResourceError>& errors)
		{
			if (typeDescriptor == description::GetTypeDescriptor<WString>())
			{
				auto str = MakePtr<WfStringExpression>();
				str->value.value = textValue;
				return str;
			}
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
				if (auto valueExpr = Workflow_ParseExpression(location, L"{" + textValue + L"}", position, errors, { 0,1 })) // {
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
				if (auto valueExpr = Workflow_ParseExpression(location, L"(" + textValue + L")", position, errors, { 0,1 })) // {
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