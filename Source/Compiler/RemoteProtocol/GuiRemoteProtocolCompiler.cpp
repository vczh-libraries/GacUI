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
			if (!symbols->structDecls.Keys().Contains(node->name.value))
			{
				errors.Add({ node->name.codeRange,L"Struct \"" + node->name.value + L"\" not found." });
			}
		}

		void Visit(GuiRpArrayType* node) override
		{
			node->element->Accept(this);
		}

		// GuiRpDeclaration::IVisitor

		void Visit(GuiRpStructDecl* node) override
		{
			if (symbols->structDecls.Keys().Contains(node->name.value))
			{
				errors.Add({ node->name.codeRange,L"Struct \"" + node->name.value + L"\" already exists." });
				return;
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
					else
					{
						symbols->structCppMapping.Add(node->name.value, att->cppType.value);
					}
				}
				else
				{
					errors.Add({ att->name.codeRange,L"Unrecognized attribute: \"" + att->name.value + L"\"." });
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
				errors.Add({ att->name.codeRange,L"Unrecognized attribute: \"" + att->name.value + L"\"." });
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
				errors.Add({ att->name.codeRange,L"Unrecognized attribute: \"" + att->name.value + L"\"." });
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

		void Visit(GuiRpReferenceType* node) override
		{
			vint index = symbols->structCppMapping.Keys().IndexOf(node->name.value);
			if (index == -1)
			{
				writer.WriteString(config.cppNamespace + L"::" + node->name.value);
			}
			else
			{
				writer.WriteString(symbols->structCppMapping.Values()[index]);
			}
		}

		void Visit(GuiRpArrayType* node) override
		{
			writer.WriteString(L"::vl::Ptr<::vl::collections::List<");
			node->element->Accept(this);
			writer.WriteString(L">>");
		}
	};

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
		for (auto structDecl : From(schema->declarations).FindType<GuiRpStructDecl>())
		{
			if (!symbols->structCppMapping.Keys().Contains(structDecl->name.value))
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
			writer.WriteString(messageDecl->response ? L", RES" : L", NROES");
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
		writer.WriteLine(L"}");
	}
}
