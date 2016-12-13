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

		Ptr<workflow::WfExpression> Workflow_ParseExpression(const WString& code, types::ErrorList& errors)
		{
			auto parser = GetParserManager()->GetParser<WfExpression>(L"WORKFLOW-EXPRESSION");
			return parser->TypedParse(code, errors);
		}

		Ptr<workflow::WfStatement> Workflow_ParseStatement(const WString& code, types::ErrorList& errors)
		{
			auto parser = GetParserManager()->GetParser<WfStatement>(L"WORKFLOW-STATEMENT");
			return parser->TypedParse(code, errors);
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

		Ptr<workflow::WfExpression> Workflow_ParseTextValue(description::ITypeDescriptor* typeDescriptor, const WString& textValue, types::ErrorList& errors)
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
				auto valueExpr = Workflow_ParseExpression(L"{" + textValue + L"}", errors);
				auto type = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal);

				auto infer = MakePtr<WfInferExpression>();
				infer->type = GetTypeFromTypeInfo(type.Obj());
				infer->expression = valueExpr;

				return infer;
			}
			else if ((typeDescriptor->GetTypeDescriptorFlags() & TypeDescriptorFlags::EnumType) != TypeDescriptorFlags::Undefined)
			{
				auto valueExpr = Workflow_ParseExpression(L"(" + textValue + L")", errors);
				auto type = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal);

				auto infer = MakePtr<WfInferExpression>();
				infer->type = GetTypeFromTypeInfo(type.Obj());
				infer->expression = valueExpr;

				return infer;
			}
			else
			{
				CHECK_FAIL(L"vl::presentation::Workflow_ParseTextValue(ITypeDescriptor*, const WString&, types::ErrorList&)#This is not a value type.");
			}
		}

		Ptr<workflow::WfExpression> Workflow_CreateValue(const description::Value& value, types::ErrorList& errors)
		{
			auto typeDescriptor = value.GetTypeDescriptor();
			if (typeDescriptor == nullptr)
			{
				auto nullExpr = MakePtr<WfLiteralExpression>();
				nullExpr->value = WfLiteralValue::Null;
				return nullExpr;
			}
			else if (typeDescriptor == description::GetTypeDescriptor<WString>())
			{
				auto str = MakePtr<WfStringExpression>();
				str->value.value = UnboxValue<WString>(value);
				return str;
			}
			else if (auto st = typeDescriptor->GetSerializableType())
			{
				auto str = MakePtr<WfStringExpression>();
				st->Serialize(value, str->value.value);

				auto type = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal);

				auto cast = MakePtr<WfTypeCastingExpression>();
				cast->type = GetTypeFromTypeInfo(type.Obj());
				cast->strategy = WfTypeCastingStrategy::Strong;
				cast->expression = str;

				return cast;
			}
			else if (typeDescriptor->GetTypeDescriptorFlags() == TypeDescriptorFlags::Struct)
			{
				auto ctorExpr = MakePtr<WfConstructorExpression>();
				vint count = typeDescriptor->GetPropertyCount();
				for (vint i = 0; i < count; i++)
				{
					auto prop = typeDescriptor->GetProperty(i);

					auto keyExpr = MakePtr<WfReferenceExpression>();
					keyExpr->name.value = prop->GetName();

					auto argument = MakePtr<WfConstructorArgument>();
					argument->key = keyExpr;
					argument->value = Workflow_CreateValue(prop->GetValue(value), errors);

					ctorExpr->arguments.Add(argument);
				}

				auto type = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal);

				auto infer = MakePtr<WfInferExpression>();
				infer->type = GetTypeFromTypeInfo(type.Obj());
				infer->expression = ctorExpr;

				return infer;
			}
			else if ((typeDescriptor->GetTypeDescriptorFlags() & TypeDescriptorFlags::EnumType) != TypeDescriptorFlags::Undefined)
			{
				auto valueExpr = MakePtr<WfIntegerExpression>();
				valueExpr->value.value = u64tow(typeDescriptor->GetEnumType()->FromEnum(value));

				auto type = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal);

				auto cast = MakePtr<WfTypeCastingExpression>();
				cast->type = GetTypeFromTypeInfo(type.Obj());
				cast->strategy = WfTypeCastingStrategy::Strong;
				cast->expression = valueExpr;

				return cast;
			}
			else
			{
				CHECK_FAIL(L"vl::presentation::Workflow_ParseTextValue(ITypeDescriptor*, const WString&, types::ErrorList&)#This is not a value type.");
			}
		}
	}
}