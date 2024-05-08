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
			if (symbols->enumDecls.Keys().Contains(node->name.value)) return;
			if (symbols->unionDecls.Keys().Contains(node->name.value)) return;
			if (symbols->structDecls.Keys().Contains(node->name.value)) return;
			errors.Add({ node->name.codeRange,L"Custom type \"" + node->name.value + L"\" not found." });
		}

		void Visit(GuiRpOptionalType* node) override
		{
			node->element->Accept(this);
		}

		void Visit(GuiRpArrayType* node) override
		{
			node->element->Accept(this);
		}

		void Visit(GuiRpArrayMapType* node) override
		{
			vint index = symbols->structDecls.Keys().IndexOf(node->element.value);
			if (index == -1)
			{
				errors.Add({ node->element.codeRange,L"Struct \"" + node->element.value + L"\" not found in a map." });
				return;
			}

			auto structDecl = symbols->structDecls.Values()[index];
			auto fieldDecl = From(structDecl->members)
				.Where([&](auto&& member) { return member->name.value == node->keyField.value; })
				.First({});
			if (!fieldDecl)
			{
				errors.Add({ node->keyField.codeRange,L"Struct \"" + node->element.value + L"\" does not contain field " + node->keyField.value + L"\" required in a map." });
				return;
			}
		}

		void Visit(GuiRpMapType* node) override
		{
			node->element->Accept(this);
			node->keyType->Accept(this);
		}

		// GuiRpDeclaration::IVisitor

		bool EnsureTypeUndefined(const glr::ParsingToken& name)
		{
			if (symbols->enumDecls.Keys().Contains(name.value))
			{
				errors.Add({ name.codeRange,L"Enum \"" + name.value + L"\" already defined." });
				return false;
			}
			if (symbols->unionDecls.Keys().Contains(name.value))
			{
				errors.Add({ name.codeRange,L"Union \"" + name.value + L"\" already defined." });
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
					else if (symbols->cppMapping.Keys().Contains(node->name.value))
					{
						errors.Add({ att->name.codeRange,L"Too many attributes: \"" + att->name.value + L"\"." });
					}
					else
					{
						symbols->cppMapping.Add(node->name.value, att->cppType.value);
					}
				}
				else if (att->name.value == L"@CppNamespace")
				{
					if (!att->cppType)
					{
						errors.Add({ att->name.codeRange,L"Missing parameter for attribute: \"" + att->name.value + L"\"." });
					}
					else if (symbols->cppNamespaces.Keys().Contains(node->name.value))
					{
						errors.Add({ att->name.codeRange,L"Too many attributes: \"" + att->name.value + L"\"." });
					}
					else
					{
						symbols->cppNamespaces.Add(node->name.value, att->cppType.value);
					}
				}
				else
				{
					errors.Add({ att->name.codeRange,L"Unsupported attribute: \"" + att->name.value + L"\" on enum \"" + node->name.value + L"\"." });
				}
			}
			symbols->enumDecls.Add(node->name.value, node);
		}

		void Visit(GuiRpUnionDecl* node) override
		{
			if (!EnsureTypeUndefined(node->name)) return;

			SortedList<WString> memberNames;
			for (auto member : node->members)
			{
				if (memberNames.Contains(member->name.value))
				{
					errors.Add({ member->name.codeRange,L"Union member \"" + node->name.value + L"::" + member->name.value + L"\" already exists." });
				}
				else
				{
					memberNames.Add(member->name.value);

					vint index = symbols->structDecls.Keys().IndexOf(member->name.value);
					if (index == -1)
					{
						errors.Add({ member->name.codeRange,L"Struct \"" + member->name.value + L"\" not found in union " + node->name.value + L"\"." });
						return;
					}

					auto structDecl = symbols->structDecls.Values()[index];
					if (structDecl->type != GuiRpStructType::Struct)
					{
						errors.Add({ member->name.codeRange,L"Struct \"" + member->name.value + L"\" in union " + node->name.value + L"\" should not be a class." });
						return;
					}
				}
			}

			for (auto att : node->attributes)
			{
				errors.Add({ att->name.codeRange,L"Unsupported attribute: \"" + att->name.value + L"\" on union \"" + node->name.value + L"\"." });
			}
			symbols->unionDecls.Add(node->name.value, node);
		}

		void Visit(GuiRpStructDecl* node) override
		{
			if (!EnsureTypeUndefined(node->name)) return;
			if (node->type == GuiRpStructType::Class)
			{
				symbols->structDecls.Add(node->name.value, node);
			}

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
					else if (symbols->cppMapping.Keys().Contains(node->name.value))
					{
						errors.Add({ att->name.codeRange,L"Too many attributes: \"" + att->name.value + L"\"." });
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

			if (node->type == GuiRpStructType::Struct)
			{
				symbols->structDecls.Add(node->name.value, node);
			}
		}

		void VisitDropAttribute(Ptr<remoteprotocol::GuiRpAttribute> att, const WString& name, SortedList<WString>& names)
		{
			if (names.Contains(name))
			{
				errors.Add({ att->name.codeRange,L"Too many attributes: \"" + att->name.value + L"\"." });
			}
			else
			{
				names.Add(name);
			}
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
				if (att->name.value == L"@DropRepeat")
				{
					if (node->response)
					{
						errors.Add({ node->name.codeRange,L"@DropRepeat cannot be used on message \"" + node->name.value + L"\" since it has response." });
					}
					VisitDropAttribute(att, node->name.value, symbols->dropRepeatDeclNames);
				}
				else
				{
					errors.Add({ att->name.codeRange,L"Unsupported attribute: \"" + att->name.value + L"\" on message \"" + node->name.value + L"\"." });
				}
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
				if (att->name.value == L"@DropRepeat")
				{
					VisitDropAttribute(att, node->name.value, symbols->dropRepeatDeclNames);
				}
				else if (att->name.value == L"@DropConsecutive")
				{
					VisitDropAttribute(att, node->name.value, symbols->dropConsecutiveDeclNames);
				}
				else
				{
					errors.Add({ att->name.codeRange,L"Unsupported attribute: \"" + att->name.value + L"\" on event \"" + node->name.value + L"\"." });
				}
			}

			if (symbols->dropRepeatDeclNames.Contains(node->name.value) && symbols->dropConsecutiveDeclNames.Contains(node->name.value))
			{
				errors.Add({ node->name.codeRange,L"@DropRepeat and @DropConsecutive cannot be used together on event \"" + node->name.value + L"\"." });
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
			case GuiRpPrimitiveTypes::Char:
				writer.WriteString(L"wchar_t");
				break;
			case GuiRpPrimitiveTypes::Key:
				writer.WriteString(L"::vl::presentation::VKEY");
				break;
			case GuiRpPrimitiveTypes::Color:
				writer.WriteString(L"::vl::presentation::Color");
				break;
			case GuiRpPrimitiveTypes::Binary:
				writer.WriteString(L"::vl::Ptr<::vl::stream::MemoryStream>");
				break;
			default:
				CHECK_FAIL(L"Unrecognized type");
			}
		}

		static WString GetCppType(const WString& type, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config, bool forJson = false)
		{
			bool ptr = false;
			if (!forJson)
			{
				vint index = symbols->structDecls.Keys().IndexOf(type);
				if (index != -1)
				{
					auto structDecl = symbols->structDecls.Values()[index];
					ptr = structDecl->type == GuiRpStructType::Class;
				}
			}

			WString result;
			{
				vint index = symbols->cppMapping.Keys().IndexOf(type);
				if (index == -1)
				{
					result = L"::" + config.cppNamespace + L"::" + type;
				}
				else
				{
					result = symbols->cppMapping.Values()[index];
				}
			}

			if (ptr)
			{
				return L"::vl::Ptr<" + result + L">";
			}
			else
			{
				return result;
			}
		}

		static WString GetCppTypeForJson(const WString& type, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config)
		{
			return GetCppType(type, symbols, config, true);
		}

		static WString GetCppNamespace(const WString& type, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config)
		{
			vint index = symbols->cppNamespaces.Keys().IndexOf(type);
			if (index == -1)
			{
				return GetCppType(type, symbols, config);
			}
			else
			{
				return symbols->cppNamespaces.Values()[index];
			}
		}

		void Visit(GuiRpReferenceType* node) override
		{
			writer.WriteString(GetCppType(node->name.value, symbols, config));
		}

		void Visit(GuiRpOptionalType* node) override
		{
			writer.WriteString(L"::vl::Nullable<");
			node->element->Accept(this);
			writer.WriteString(L">");
		}

		void Visit(GuiRpArrayType* node) override
		{
			writer.WriteString(L"::vl::Ptr<::vl::collections::List<");
			node->element->Accept(this);
			writer.WriteString(L">>");
		}

		void Visit(GuiRpArrayMapType* node) override
		{
			auto cppName = GetCppType(node->element.value, symbols, config);
			auto structDecl = symbols->structDecls[node->element.value];
			auto fieldDecl = From(structDecl->members)
				.Where([&](auto&& member) { return member->name.value == node->keyField.value; })
				.First();
			writer.WriteString(L"::vl::Ptr<::vl::presentation::remoteprotocol::ArrayMap<");
			fieldDecl->type->Accept(this);
			writer.WriteString(L", ");
			writer.WriteString(cppName);
			writer.WriteString(L", &");
			writer.WriteString(cppName);
			writer.WriteString(L"::");
			writer.WriteString(node->keyField.value);
			writer.WriteString(L">>");
		}

		void Visit(GuiRpMapType* node) override
		{
			writer.WriteString(L"::vl::Ptr<::vl::collections::Dictionary<");
			node->keyType->Accept(this);
			writer.WriteString(L", ");
			node->element->Accept(this);
			writer.WriteString(L">>");
		}
	};

	void GenerateSerializerFunctionHeader(const WString& type, bool semicolon, stream::TextWriter& writer)
	{
		writer.WriteString(L"\ttemplate<> vl::Ptr<vl::glr::json::JsonNode> ConvertCustomTypeToJson<" + type + L">(const " + type + L" & value)");
		writer.WriteLine(semicolon ? L";" : L"");
	}

	void GenerateDeserializerFunctionHeader(const WString& type, bool semicolon, stream::TextWriter& writer)
	{
		writer.WriteString(L"\ttemplate<> void ConvertJsonToCustomType<" + type + L">(vl::Ptr<vl::glr::json::JsonNode> node, " + type + L"& value)");
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

		auto structDecls = From(schema->declarations).FindType<GuiRpStructDecl>();

		writer.WriteLine(L"namespace " + config.cppNamespace);
		writer.WriteLine(L"{");
		for (auto structDecl : structDecls)
		{
			if (!symbols->cppMapping.Keys().Contains(structDecl->name.value))
			{
				writer.WriteLine(L"\tstruct " + structDecl->name.value + L";");
			}
		}
		writer.WriteLine(L"}");
		
		writer.WriteLine(L"namespace vl::presentation::remoteprotocol");
		writer.WriteLine(L"{");
		for (auto structDecl : structDecls)
		{
			WString cppName = GuiRpPrintTypeVisitor::GetCppType(structDecl->name.value, symbols, config);
			writer.WriteLine(L"\ttemplate<> struct JsonNameHelper<" + cppName + L"> { static constexpr const wchar_t* Name = L\"" + structDecl->name.value + L"\"; };");
		}
		writer.WriteLine(L"}");

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

		for (auto unionDecl : From(schema->declarations).FindType<GuiRpUnionDecl>())
		{
			if (!symbols->cppMapping.Keys().Contains(unionDecl->name.value))
			{
				writer.WriteLine(L"\tusing " + unionDecl->name.value + L" = ::vl::Variant<");
				for (auto [member, index] : indexed(unionDecl->members))
				{
					writer.WriteString(L"\t\t" + GuiRpPrintTypeVisitor::GetCppType(member->name.value, symbols, config));
					if (index < unionDecl->members.Count() - 1)
					{
						writer.WriteLine(L",");
					}
					else
					{
						writer.WriteLine(L"");
					}
				}
				writer.WriteLine(L"\t>;");
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
			GenerateSerializerFunctionHeader(GuiRpPrintTypeVisitor::GetCppType(enumDecl->name.value, symbols, config), true, writer);
		}
		for (auto structDecl : From(schema->declarations).FindType<GuiRpStructDecl>())
		{
			GenerateSerializerFunctionHeader(GuiRpPrintTypeVisitor::GetCppTypeForJson(structDecl->name.value, symbols, config), true, writer);
		}
		writer.WriteLine(L"");

		for (auto enumDecl : From(schema->declarations).FindType<GuiRpEnumDecl>())
		{
			GenerateDeserializerFunctionHeader(GuiRpPrintTypeVisitor::GetCppType(enumDecl->name.value, symbols, config), true, writer);
		}
		for (auto structDecl : From(schema->declarations).FindType<GuiRpStructDecl>())
		{
			GenerateDeserializerFunctionHeader(GuiRpPrintTypeVisitor::GetCppTypeForJson(structDecl->name.value, symbols, config), true, writer);
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
			writer.WriteString(symbols->dropRepeatDeclNames.Contains(messageDecl->name.value) ? L", DROPREP" : L", NODROP");
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
			writer.WriteString(
				symbols->dropRepeatDeclNames.Contains(eventDecl->name.value) ? L", DROPREP" :
				symbols->dropConsecutiveDeclNames.Contains(eventDecl->name.value) ? L", DROPCON" :
				L", NODROP");
			writer.WriteLine(L")\\");
		}
		writer.WriteLine(L"");

		SortedList<WString> requestTypes, responseTypes, eventTypes;
		{
			for (auto messageDecl : From(schema->declarations).FindType<GuiRpMessageDecl>())
			{
				if (messageDecl->request)
				{
					auto type = stream::GenerateToStream([&](stream::TextWriter& writer)
					{
						GuiRpPrintTypeVisitor visitor(symbols, config, writer);
						messageDecl->request->type->Accept(&visitor);
					});
					if (!requestTypes.Contains(type))
					{
						requestTypes.Add(type);
					}
				}

				if (messageDecl->response)
				{
					auto type = stream::GenerateToStream([&](stream::TextWriter& writer)
					{
						GuiRpPrintTypeVisitor visitor(symbols, config, writer);
						messageDecl->response->type->Accept(&visitor);
					});
					if (!responseTypes.Contains(type))
					{
						responseTypes.Add(type);
					}
				}
			}

			for (auto eventDecl : From(schema->declarations).FindType<GuiRpEventDecl>())
			{
				if (eventDecl->request)
				{
					auto type = stream::GenerateToStream([&](stream::TextWriter& writer)
					{
						GuiRpPrintTypeVisitor visitor(symbols, config, writer);
					eventDecl->request->type->Accept(&visitor);
					});
					if (!eventTypes.Contains(type))
					{
						eventTypes.Add(type);
					}
				}
			}
		}

		writer.WriteLine(L"#define GACUI_REMOTEPROTOCOL_MESSAGE_REQUEST_TYPES(HANDLER)\\");
		for (auto type : requestTypes) writer.WriteLine(L"\tHANDLER(" + type + L")\\");
		writer.WriteLine(L"");

		writer.WriteLine(L"#define GACUI_REMOTEPROTOCOL_MESSAGE_RESPONSE_TYPES(HANDLER)\\");
		for (auto type : responseTypes) writer.WriteLine(L"\tHANDLER(" + type + L")\\");
		writer.WriteLine(L"");

		writer.WriteLine(L"#define GACUI_REMOTEPROTOCOL_EVENT_REQUEST_TYPES(HANDLER)\\");
		for (auto type : eventTypes) writer.WriteLine(L"\tHANDLER(" + type + L")\\");
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
		WString cppNss = GuiRpPrintTypeVisitor::GetCppNamespace(enumDecl->name.value, symbols, config);
		GenerateSerializerFunctionHeader(cppName, false, writer);
		writer.WriteLine(L"\t{");
		writer.WriteLine(L"#define ERROR_MESSAGE_PREFIX L\"vl::presentation::remoteprotocol::ConvertCustomTypeToJson<" + cppName + L">(const " + cppName + L"&)#\"");
		writer.WriteLine(L"\t\tauto node = Ptr(new glr::json::JsonString);");
		writer.WriteLine(L"\t\tswitch (value)");
		writer.WriteLine(L"\t\t{");
		for (auto member : enumDecl->members)
		{
			writer.WriteLine(L"\t\tcase " + cppNss + L"::" + member->name.value + L": node->content.value = WString::Unmanaged(L\"" + member->name.value + L"\"); break;");
		}
		writer.WriteLine(L"\t\tdefault: CHECK_FAIL(ERROR_MESSAGE_PREFIX L\"Unsupported enum value.\");");
		writer.WriteLine(L"\t\t}");
		writer.WriteLine(L"\t\treturn node;");
		writer.WriteLine(L"#undef ERROR_MESSAGE_PREFIX");
		writer.WriteLine(L"\t}");
		writer.WriteLine(L"");
	}

	void GenerateStructSerializerFunctionImpl(Ptr<GuiRpStructDecl> structDecl, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config, stream::TextWriter& writer)
	{
		WString cppName = GuiRpPrintTypeVisitor::GetCppTypeForJson(structDecl->name.value, symbols, config);
		GenerateSerializerFunctionHeader(cppName, false, writer);
		writer.WriteLine(L"\t{");
		writer.WriteLine(L"\t\tauto node = Ptr(new glr::json::JsonObject);");
		for (auto member : structDecl->members)
		{
			writer.WriteLine(L"\t\tConvertCustomTypeToJsonField(node, L\"" + member->name.value + L"\", value." + member->name.value + L");");
		}
		writer.WriteLine(L"\t\treturn node;");
		writer.WriteLine(L"\t}");
		writer.WriteLine(L"");
	}

	void GenerateEnumDeserializerFunctionImpl(Ptr<GuiRpEnumDecl> enumDecl, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config, stream::TextWriter& writer)
	{
		WString cppName = GuiRpPrintTypeVisitor::GetCppType(enumDecl->name.value, symbols, config);
		WString cppNss = GuiRpPrintTypeVisitor::GetCppNamespace(enumDecl->name.value, symbols, config);
		GenerateDeserializerFunctionHeader(cppName, false, writer);
		writer.WriteLine(L"\t{");
		writer.WriteLine(L"#define ERROR_MESSAGE_PREFIX L\"vl::presentation::remoteprotocol::ConvertJsonToCustomType<" + cppName + L">(Ptr<JsonNode>, " + cppName + L"&)#\"");
		writer.WriteLine(L"\t\tauto jsonNode = node.Cast<glr::json::JsonString>();");
		writer.WriteLine(L"\t\tCHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L\"Json node does not match the expected type.\");");
		for (auto member : enumDecl->members)
		{
			writer.WriteLine(L"\t\tif (jsonNode->content.value == L\"" + member->name.value + L"\") value = " + cppNss + L"::" + member->name.value + L"; else");
		}
		writer.WriteLine(L"\t\tCHECK_FAIL(ERROR_MESSAGE_PREFIX L\"Unsupported enum value.\");");
		writer.WriteLine(L"#undef ERROR_MESSAGE_PREFIX");
		writer.WriteLine(L"\t}");
		writer.WriteLine(L"");
	}

	void GenerateStructDeserializerFunctionImpl(Ptr<GuiRpStructDecl> structDecl, Ptr<GuiRpSymbols> symbols, GuiRpCppConfig& config, stream::TextWriter& writer)
	{
		WString cppName = GuiRpPrintTypeVisitor::GetCppTypeForJson(structDecl->name.value, symbols, config);
		GenerateDeserializerFunctionHeader(cppName, false, writer);
		writer.WriteLine(L"\t{");
		writer.WriteLine(L"#define ERROR_MESSAGE_PREFIX L\"vl::presentation::remoteprotocol::ConvertJsonToCustomType<" + cppName + L">(Ptr<JsonNode>, " + cppName + L"&)#\"");
		writer.WriteLine(L"\t\tauto jsonNode = node.Cast<glr::json::JsonObject>();");
		writer.WriteLine(L"\t\tCHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L\"Json node does not match the expected type.\");");
		writer.WriteLine(L"\t\tfor (auto field : jsonNode->fields)");
		writer.WriteLine(L"\t\t{");
		for (auto member : structDecl->members)
		{
			writer.WriteLine(L"\t\t\tif (field->name.value == L\"" + member->name.value + L"\") ConvertJsonToCustomType(field->value, value." + member->name.value + L"); else");
		}
		writer.WriteLine(L"\t\t\tCHECK_FAIL(ERROR_MESSAGE_PREFIX L\"Unsupported struct member.\");");
		writer.WriteLine(L"\t\t}");
		writer.WriteLine(L"#undef ERROR_MESSAGE_PREFIX");
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
