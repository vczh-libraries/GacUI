#include "GuiRemoteProtocolCompiler.h"

namespace vl::presentation
{
	using namespace collections;
	using namespace remoteprotocol;

/***********************************************************************
CheckRemoteProtocolSchema
***********************************************************************/

	class GuiRpCheckSchemaVisitor
		: public Object
		, public GuiRpType::IVisitor
		, public GuiRpDeclaration::IVisitor
	{
	protected:
		Ptr<GuiRpSymbols>			symbols;
		List<GuiRpError>&			errors;

	public:
		GuiRpCheckSchemaVisitor(Ptr<GuiRpSymbols> _symbols, List<GuiRpError>& _errors)
			: symbols(_symbols)
			, errors(_errors)
		{
		}

		// GuiRpType::IVisitor

		void Visit(GuiRpPrimitiveType* node) override
		{
		}

		void Visit(GuiRpReferenceType* node) override
		{
			if (!symbols->enumDecls.Keys().Contains(node->name.value) && !symbols->structDecls.Keys().Contains(node->name.value))
			{
				errors.Add({ node->name.codeRange,L"Custom type \"" + node->name.value + L"\" not found." });
			}
		}

		void Visit(GuiRpArrayType* node) override
		{
			node->element->Accept(this);
		}

		// GuiRpDeclaration::IVisitor

		bool EnsureTypeUndefined(const glr::ParsingToken& name)
		{
			if (symbols->enumDecls.Keys().Contains(name.value))
			{
				errors.Add({ name.codeRange,L"Enum \"" + name.value + L"\" already defined." });
				return false;
			}
			if (symbols->structDecls.Keys().Contains(name.value))
			{
				errors.Add({ name.codeRange,L"Struct \"" + name.value + L"\" already defined." });
				return false;
			}
			return true;
		}

		void Visit(GuiRpEnumDecl* node) override
		{
			if (!EnsureTypeUndefined(node->name)) return;

			SortedList<WString> memberNames;
			for (auto member : node->members)
			{
				if (memberNames.Contains(member->name.value))
				{
					errors.Add({ member->name.codeRange,L"Enum member \"" + node->name.value + L"::" + member->name.value + L"\" already exists." });
				}
				else
				{
					memberNames.Add(member->name.value);
				}
			}

			for (auto att : node->attributes)
			{
				if (att->name.value == L"@Cpp")
				{
					if (!att->cppType)
					{
						errors.Add({ att->name.codeRange,L"Missing parameter for attribute: \"" + att->name.value + L"\"." });
					}
					else
					{
						symbols->cppMapping.Add(node->name.value, att->cppType.value);
					}
				}
				else
				{
					errors.Add({ att->name.codeRange,L"Unsupported attribute: \"" + att->name.value + L"\" on enum \"" + node->name.value + L"\"." });
				}
			}
			symbols->enumDecls.Add(node->name.value, node);
		}

		void Visit(GuiRpStructDecl* node) override
		{
			if (!EnsureTypeUndefined(node->name)) return;

			SortedList<WString> memberNames;
			for (auto member : node->members)
			{
				if (memberNames.Contains(member->name.value))
				{
					errors.Add({ member->name.codeRange,L"Struct member \"" + node->name.value + L"::" + member->name.value + L"\" already exists." });
				}
				else
				{
					memberNames.Add(member->name.value);
				}
				member->type->Accept(this);
			}

			for (auto att : node->attributes)
			{
				if (att->name.value == L"@Cpp")
				{
					if (!att->cppType)
					{
						errors.Add({ att->name.codeRange,L"Missing parameter for attribute: \"" + att->name.value + L"\"." });
					}
					else
					{
						symbols->cppMapping.Add(node->name.value, att->cppType.value);
					}
				}
				else
				{
					errors.Add({ att->name.codeRange,L"Unsupported attribute: \"" + att->name.value + L"\" on struct \"" + node->name.value + L"\"." });
				}
			}
			symbols->structDecls.Add(node->name.value, node);
		}

		void Visit(GuiRpMessageDecl* node) override
		{
			if (symbols->messageDecls.Keys().Contains(node->name.value))
			{
				errors.Add({ node->name.codeRange,L"Message \"" + node->name.value + L"\" already exists." });
				return;
			}

			if (node->request)
			{
				node->request->type->Accept(this);
			}

			if (node->response)
			{
				node->response->type->Accept(this);
			}

			for (auto att : node->attributes)
			{
				errors.Add({ att->name.codeRange,L"Unsupported attribute: \"" + att->name.value + L"\" on message \"" + node->name.value + L"\"." });
			}
			symbols->messageDecls.Add(node->name.value, node);
		}

		void Visit(GuiRpEventDecl* node) override
		{
			if (symbols->eventDecls.Keys().Contains(node->name.value))
			{
				errors.Add({ node->name.codeRange,L"Event \"" + node->name.value + L"\" already exists." });
				return;
			}

			if (node->request)
			{
				node->request->type->Accept(this);
			}

			for (auto att : node->attributes)
			{
				errors.Add({ att->name.codeRange,L"Unsupported attribute: \"" + att->name.value + L"\" on event \"" + node->name.value + L"\"." });
			}
			symbols->eventDecls.Add(node->name.value, node);
		}
	};

	Ptr<GuiRpSymbols> CheckRemoteProtocolSchema(Ptr<GuiRpSchema> schema, collections::List<GuiRpError>& errors)
	{
		auto symbols = Ptr(new GuiRpSymbols);
		GuiRpCheckSchemaVisitor visitor(symbols, errors);
		for (auto decl : schema->declarations)
		{
			decl->Accept(&visitor);
		}
		return symbols;
	}

/***********************************************************************
GenerateRemoteProtocolHeaderFile
***********************************************************************/

	class GuiRpPrintTypeVisitor : public Object, public GuiRpType::IVisitor
	{
	protected:
		Ptr<GuiRpSymbols>			symbols;
		GuiRpCppConfig&				config;
		stream::TextWriter&			writer;

	public:
		GuiRpPrintTypeVisitor(Ptr<GuiRpSymbols> _symbols, GuiRpCppConfig& _config, stream::TextWriter& _writer)
			: symbols(_symbols)
			, config(_config)
			, writer(_writer)
		{
		}

		// GuiRpType::IVisitor

		void Visit(GuiRpPrimitiveType* node) override
		{
			switch (node->type)
			{
			case GuiRpPrimitiveTypes::Boolean:
				writer.WriteString(L"bool");
				break;
			case GuiRpPrimitiveTypes::Integer:
				writer.WriteString(L"::vl::vint");
				break;
			case GuiRpPrimitiveTypes::Float:
				writer.WriteString(L"float");
				break;
			case GuiRpPrimitiveTypes::Double:
				writer.WriteString(L"double");
				break;
			case GuiRpPrimitiveTypes::String:
				writer.WriteString(L"::vl::WString");
				break;
			default:
				CHECK_FAIL(L"Unrecognized type");
			}
		}

		static WString GetCppType(const WString& type, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config)
		{
			vint index = symbols->cppMapping.Keys().IndexOf(type);
			if (index == -1)
			{
				return config.cppNamespace + L"::" + type;
			}
			else
			{
				return symbols->cppMapping.Values()[index];
			}
		}

		void Visit(GuiRpReferenceType* node) override
		{
			writer.WriteString(GetCppType(node->name.value, symbols, config));
		}

		void Visit(GuiRpArrayType* node) override
		{
			writer.WriteString(L"::vl::Ptr<::vl::collections::List<");
			node->element->Accept(this);
			writer.WriteString(L">>");
		}
	};

	void GenerateSerializerFunctionHeader(const WString& type, bool semicolon, stream::TextWriter& writer)
	{
		writer.WriteString(L"vl::Ptr<vl::glr::json::JsonNode> ConvertCustomTypeToJson(const " + type + L"& value)");
		writer.WriteLine(semicolon ? L";" : L"");
	}

	void GenerateDeserializerFunctionHeader(const WString& type, bool semicolon, stream::TextWriter& writer)
	{
		writer.WriteString(L"void ConvertJsonToCustomType(vl::Ptr<vl::glr::json::JsonNode> node, " + type + L"& value)");
		writer.WriteLine(semicolon ? L";" : L"");
	}

	void GenerateRemoteProtocolHeaderFile(Ptr<GuiRpSchema> schema, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config, stream::TextWriter& writer)
	{
		writer.WriteLine(L"/***********************************************************************");
		writer.WriteLine(L"This file is generated by : Vczh GacUI Remote Protocol Generator");
		writer.WriteLine(L"Licensed under https ://github.com/vczh-libraries/License");
		writer.WriteLine(L"***********************************************************************/");
		writer.WriteLine(L"");
		writer.WriteLine(L"#ifndef " + config.headerGuard);
		writer.WriteLine(L"#define " + config.headerGuard);
		writer.WriteLine(L"");
		writer.WriteLine(L"#include \"" + config.headerInclude + L"\"");
		writer.WriteLine(L"");
		writer.WriteLine(L"namespace " + config.cppNamespace);
		writer.WriteLine(L"{");

		GuiRpPrintTypeVisitor printTypeVisitor(symbols, config, writer);

		for (auto enumDecl : From(schema->declarations).FindType<GuiRpEnumDecl>())
		{
			if (!symbols->cppMapping.Keys().Contains(enumDecl->name.value))
			{
				writer.WriteLine(L"\tenum class " + enumDecl->name.value);
				writer.WriteLine(L"\t{");
				for (auto member : enumDecl->members)
				{
					writer.WriteLine(L"\t\t" + member->name.value + L",");
				}
				writer.WriteLine(L"\t};");
				writer.WriteLine(L"");
			}
		}

		for (auto structDecl : From(schema->declarations).FindType<GuiRpStructDecl>())
		{
			if (!symbols->cppMapping.Keys().Contains(structDecl->name.value))
			{
				writer.WriteLine(L"\tstruct " + structDecl->name.value);
				writer.WriteLine(L"\t{");
				for (auto member : structDecl->members)
				{
					writer.WriteString(L"\t\t");
					member->type->Accept(&printTypeVisitor);
					writer.WriteLine(L" " + member->name.value + L";");
				}
				writer.WriteLine(L"\t};");
				writer.WriteLine(L"");
			}
		}

		for (auto enumDecl : From(schema->declarations).FindType<GuiRpEnumDecl>())
		{
			writer.WriteString(L"\textern ");
			GenerateSerializerFunctionHeader(GuiRpPrintTypeVisitor::GetCppType(enumDecl->name.value, symbols, config), true, writer);
		}
		for (auto structDecl : From(schema->declarations).FindType<GuiRpStructDecl>())
		{
			writer.WriteString(L"\textern ");
			GenerateSerializerFunctionHeader(GuiRpPrintTypeVisitor::GetCppType(structDecl->name.value, symbols, config), true, writer);
		}
		writer.WriteLine(L"");

		for (auto enumDecl : From(schema->declarations).FindType<GuiRpEnumDecl>())
		{
			writer.WriteString(L"\textern ");
			GenerateDeserializerFunctionHeader(GuiRpPrintTypeVisitor::GetCppType(enumDecl->name.value, symbols, config), true, writer);
		}
		for (auto structDecl : From(schema->declarations).FindType<GuiRpStructDecl>())
		{
			writer.WriteString(L"\textern ");
			GenerateDeserializerFunctionHeader(GuiRpPrintTypeVisitor::GetCppType(structDecl->name.value, symbols, config), true, writer);
		}
		writer.WriteLine(L"");

		writer.WriteLine(L"#define " + config.builderMacroPrefix + L"_MESSAGES(HANDLER)\\");
		for (auto messageDecl : From(schema->declarations).FindType<GuiRpMessageDecl>())
		{
			writer.WriteString(L"\tHANDLER(" + messageDecl->name.value);

			writer.WriteString(L", ");
			if (messageDecl->request)
			{
				messageDecl->request->type->Accept(&printTypeVisitor);
			}
			else
			{
				writer.WriteString(L"void");
			}

			writer.WriteString(L", ");
			if (messageDecl->response)
			{
				messageDecl->response->type->Accept(&printTypeVisitor);
			}
			else
			{
				writer.WriteString(L"void");
			}

			writer.WriteString(messageDecl->request ? L", REQ" : L", NOREQ");
			writer.WriteString(messageDecl->response ? L", RES" : L", NORES");
			writer.WriteLine(L")\\");
		}
		writer.WriteLine(L"");

		writer.WriteLine(L"#define " + config.builderMacroPrefix + L"_EVENTS(HANDLER)\\");
		for (auto eventDecl : From(schema->declarations).FindType<GuiRpEventDecl>())
		{
			writer.WriteString(L"\tHANDLER(" + eventDecl->name.value);

			writer.WriteString(L", ");
			if (eventDecl->request)
			{
				eventDecl->request->type->Accept(&printTypeVisitor);
			}
			else
			{
				writer.WriteString(L"void");
			}

			writer.WriteString(eventDecl->request ? L", REQ" : L", NOREQ");
			writer.WriteLine(L")\\");
		}
		writer.WriteLine(L"");

		writer.WriteLine(L"}");
		writer.WriteLine(L"");
		writer.WriteLine(L"#endif");
	}

/***********************************************************************
GenerateRemoteProtocolCppFile
***********************************************************************/

	void GenerateEnumSerializerFunctionImpl(Ptr<GuiRpEnumDecl> enumDecl, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config, stream::TextWriter& writer)
	{
		WString cppName = GuiRpPrintTypeVisitor::GetCppType(enumDecl->name.value, symbols, config);
		writer.WriteString(L"\t");
		GenerateSerializerFunctionHeader(cppName, false, writer);
		writer.WriteLine(L"\t{");
		writer.WriteLine(L"\t#define ERROR_MESSAGE_PREFIX L\"vl::presentation::remoteprotocol::ConvertCustomTypeToJson(const " + cppName + L"&)#\"");
		writer.WriteLine(L"\t\tauto node = Ptr(new glr::json::JsonString);");
		writer.WriteLine(L"\t\tswitch (value)");
		writer.WriteLine(L"\t\t{");
		for (auto member : enumDecl->members)
		{
			writer.WriteLine(L"\t\tcase " + cppName + L"::" + member->name.value + L": node->content.value = L\"" + member->name.value + L"\"; break;");
		}
		writer.WriteLine(L"\t\tdefault: CHECK_FAIL(ERROR_MESSAGE_PREFIX L\"Unsupported enum value.\");");
		writer.WriteLine(L"\t\t}");
		writer.WriteLine(L"\t\treturn node;");
		writer.WriteLine(L"\t#undef ERROR_MESSAGE_PREFIX");
		writer.WriteLine(L"\t}");
		writer.WriteLine(L"");
	}

	void GenerateStructSerializerFunctionImpl(Ptr<GuiRpStructDecl> structDecl, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config, stream::TextWriter& writer)
	{
		WString cppName = GuiRpPrintTypeVisitor::GetCppType(structDecl->name.value, symbols, config);
		writer.WriteString(L"\t");
		GenerateSerializerFunctionHeader(cppName, false, writer);
		writer.WriteLine(L"\t{");
		writer.WriteLine(L"\t\tauto node = Ptr(new glr::json::JsonObject);");
		for (auto member : structDecl->members)
		{
			writer.WriteLine(L"\t\t{");
			writer.WriteLine(L"\t\t\tauto field = Ptr(new glr::json::JsonObjectField);");
			writer.WriteLine(L"\t\t\tfield->name.value = L\"" + member->name.value + L"\"; ");
			writer.WriteLine(L"\t\t\tfield->value = ConvertCustomTypeToJson(value." + member->name.value + L");");
			writer.WriteLine(L"\t\t\tnode->fields.Add(field);");
			writer.WriteLine(L"\t\t}");
		}
		writer.WriteLine(L"\t\treturn node;");
		writer.WriteLine(L"\t}");
		writer.WriteLine(L"");
	}

	void GenerateEnumDeserializerFunctionImpl(Ptr<GuiRpEnumDecl> enumDecl, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config, stream::TextWriter& writer)
	{
		WString cppName = GuiRpPrintTypeVisitor::GetCppType(enumDecl->name.value, symbols, config);
		writer.WriteString(L"\t");
		GenerateDeserializerFunctionHeader(cppName, false, writer);
		writer.WriteLine(L"\t{");
		writer.WriteLine(L"\t#define ERROR_MESSAGE_PREFIX L\"vl::presentation::remoteprotocol::ConvertJsonToCustomType(Ptr<JsonNode>, " + cppName + L"&)#\"");
		writer.WriteLine(L"\t\tauto jsonNode = node.Cast<glr::json::JsonString>();");
		writer.WriteLine(L"\t\tCHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L\"Json node does not match the expected type.\");");
		for (auto member : enumDecl->members)
		{
			writer.WriteLine(L"\t\tif (jsonNode->content.value == L\"" + member->name.value + L"\") value = " + cppName + L"::" + member->name.value + L"; else");
		}
		writer.WriteLine(L"\t\tCHECK_FAIL(ERROR_MESSAGE_PREFIX L\"Unsupported enum value.\");");
		writer.WriteLine(L"\t#undef ERROR_MESSAGE_PREFIX");
		writer.WriteLine(L"\t}");
		writer.WriteLine(L"");
	}

	void GenerateStructDeserializerFunctionImpl(Ptr<GuiRpStructDecl> structDecl, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config, stream::TextWriter& writer)
	{
		WString cppName = GuiRpPrintTypeVisitor::GetCppType(structDecl->name.value, symbols, config);
		writer.WriteString(L"\t");
		GenerateDeserializerFunctionHeader(cppName, false, writer);
		writer.WriteLine(L"\t{");
		writer.WriteLine(L"\t\tCHECK_FAIL(L\"Not Implemented!\");");
		writer.WriteLine(L"\t}");
		writer.WriteLine(L"");
	}

	void GenerateRemoteProtocolCppFile(Ptr<GuiRpSchema> schema, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config, stream::TextWriter& writer)
	{
		writer.WriteLine(L"/***********************************************************************");
		writer.WriteLine(L"This file is generated by : Vczh GacUI Remote Protocol Generator");
		writer.WriteLine(L"Licensed under https ://github.com/vczh-libraries/License");
		writer.WriteLine(L"***********************************************************************/");
		writer.WriteLine(L"");
		writer.WriteLine(L"#include \"" + config.headerFileName + L"\"");
		writer.WriteLine(L"");
		writer.WriteLine(L"namespace " + config.cppNamespace);
		writer.WriteLine(L"{");

		for (auto enumDecl : From(schema->declarations).FindType<GuiRpEnumDecl>())
		{
			GenerateEnumSerializerFunctionImpl(enumDecl, symbols, config, writer);
		}
		for (auto structDecl : From(schema->declarations).FindType<GuiRpStructDecl>())
		{
			GenerateStructSerializerFunctionImpl(structDecl, symbols, config, writer);
		}

		for (auto enumDecl : From(schema->declarations).FindType<GuiRpEnumDecl>())
		{
			GenerateEnumDeserializerFunctionImpl(enumDecl, symbols, config, writer);
		}
		for (auto structDecl : From(schema->declarations).FindType<GuiRpStructDecl>())
		{
			GenerateStructDeserializerFunctionImpl(structDecl, symbols, config, writer);
		}
		writer.WriteLine(L"}");
	}
}
