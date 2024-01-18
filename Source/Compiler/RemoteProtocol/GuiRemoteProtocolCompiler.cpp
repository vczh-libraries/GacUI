#include "GuiRemoteProtocolCompiler.h"

namespace vl::presentation
{
	using namespace collections;
	using namespace remoteprotocol;

	class GuiRpCheckSchemaVisitor
		: public Object
		, public remoteprotocol::GuiRpType::IVisitor
		, public remoteprotocol::GuiRpDeclaration::IVisitor
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

			for (auto att : node->attributes)
			{
				errors.Add({ att->name.codeRange,L"Unrecognized attribute: \"" + att->name.value + L"\"." });
			}
			symbols->eventDecls.Add(node->name.value, node);
		}
	};

	Ptr<GuiRpSymbols> CheckRemoteProtocolSchema(Ptr<remoteprotocol::GuiRpSchema> schema, collections::List<GuiRpError>& errors)
	{
		auto symbols = Ptr(new GuiRpSymbols);
		GuiRpCheckSchemaVisitor visitor(symbols, errors);
		for (auto decl : schema->declarations)
		{
			decl->Accept(&visitor);
		}
		return symbols;
	}
}
