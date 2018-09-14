/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Resource

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_RESOURCES_GUIPARSERMANAGER
#define VCZH_PRESENTATION_RESOURCES_GUIPARSERMANAGER

#include "GuiResource.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
Parser
***********************************************************************/

		/// <summary>Represents a parser.</summary>
		class IGuiGeneralParser : public IDescriptable, public Description<IGuiGeneralParser>
		{
		public:
		};

		template<typename T>
		class IGuiParser : public IGuiGeneralParser
		{
			using ErrorList = collections::List<Ptr<parsing::ParsingError>>;
		public:
			virtual Ptr<T>							ParseInternal(const WString& text, ErrorList& errors) = 0;

			Ptr<T> Parse(GuiResourceLocation location, const WString& text, collections::List<GuiResourceError>& errors)
			{
				ErrorList parsingErrors;
				auto result = ParseInternal(text, parsingErrors);
				GuiResourceError::Transform(location, errors, parsingErrors);
				return result;
			}

			Ptr<T> Parse(GuiResourceLocation location, const WString& text, parsing::ParsingTextPos position, collections::List<GuiResourceError>& errors)
			{
				ErrorList parsingErrors;
				auto result = ParseInternal(text, parsingErrors);
				GuiResourceError::Transform(location, errors, parsingErrors, position);
				return result;
			}

			Ptr<T> Parse(GuiResourceLocation location, const WString& text, GuiResourceTextPos position, collections::List<GuiResourceError>& errors)
			{
				ErrorList parsingErrors;
				auto result = ParseInternal(text, parsingErrors);
				GuiResourceError::Transform(location, errors, parsingErrors, position);
				return result;
			}
		};

/***********************************************************************
Parser Manager
***********************************************************************/

		/// <summary>Parser manager for caching parsing table globally.</summary>
		class IGuiParserManager : public IDescriptable, public Description<IGuiParserManager>
		{
		protected:
			typedef parsing::tabling::ParsingTable			Table;

		public:
			/// <summary>Get a parsing table by name.</summary>
			/// <returns>The parsing table.</returns>
			/// <param name="name">The name.</param>
			virtual Ptr<Table>						GetParsingTable(const WString& name)=0;
			/// <summary>Set a parsing table loader by name.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="name">The name.</param>
			/// <param name="loader">The parsing table loader.</param>
			virtual bool							SetParsingTable(const WString& name, Func<Ptr<Table>()> loader)=0;
			/// <summary>Get a parser.</summary>
			/// <returns>The parser.</returns>
			/// <param name="name">The name.</param>
			virtual Ptr<IGuiGeneralParser>			GetParser(const WString& name)=0;
			/// <summary>Set a parser by name.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="name">The name.</param>
			/// <param name="parser">The parser.</param>
			virtual bool							SetParser(const WString& name, Ptr<IGuiGeneralParser> parser)=0;

			template<typename T>
			Ptr<IGuiParser<T>>						GetParser(const WString& name);

			template<typename T>
			bool									SetTableParser(const WString& tableName, const WString& parserName, Ptr<T>(*function)(const WString&, Ptr<Table>, collections::List<Ptr<parsing::ParsingError>>&, vint));
		};

		/// <summary>Get the global <see cref="IGuiParserManager"/> object.</summary>
		/// <returns>The parser manager.</returns>
		extern IGuiParserManager*					GetParserManager();

/***********************************************************************
Strong Typed Table Parser
***********************************************************************/

		template<typename T>
		class GuiStrongTypedTableParser : public Object, public IGuiParser<T>
		{
		protected:
			typedef parsing::tabling::ParsingTable				Table;
			typedef Ptr<T>(ParserFunction)(const WString&, Ptr<Table>, collections::List<Ptr<parsing::ParsingError>>&, vint);
		protected:
			WString									name;
			Ptr<Table>								table;
			Func<ParserFunction>					function;

		public:
			GuiStrongTypedTableParser(const WString& _name, ParserFunction* _function)
				:name(_name)
				,function(_function)
			{
			}

			Ptr<T> ParseInternal(const WString& text, collections::List<Ptr<parsing::ParsingError>>& errors)override
			{
				if (!table)
				{
					table = GetParserManager()->GetParsingTable(name);
				}
				if (table)
				{
					collections::List<Ptr<parsing::ParsingError>> parsingErrors;
					auto result = function(text, table, parsingErrors, -1);
					if (parsingErrors.Count() > 0)
					{
						errors.Add(parsingErrors[0]);
					}
					return result;
				}
				return nullptr;
			}
		};

/***********************************************************************
Parser Manager
***********************************************************************/

		template<typename T>
		Ptr<IGuiParser<T>> IGuiParserManager::GetParser(const WString& name)
		{
			return GetParser(name).Cast<IGuiParser<T>>();
		}

		template<typename T>
		bool IGuiParserManager::SetTableParser(const WString& tableName, const WString& parserName, Ptr<T>(*function)(const WString&, Ptr<Table>, collections::List<Ptr<parsing::ParsingError>>&, vint))
		{
			Ptr<IGuiParser<T>> parser=new GuiStrongTypedTableParser<T>(tableName, function);
			return SetParser(parserName, parser);
		}
	}
}

#endif