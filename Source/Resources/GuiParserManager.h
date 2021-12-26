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
			using ErrorList = collections::List<glr::ParsingError>;
		public:
			virtual Ptr<T>							ParseInternal(const WString& text, ErrorList& errors) = 0;

			Ptr<T> Parse(GuiResourceLocation location, const WString& text, collections::List<GuiResourceError>& errors)
			{
				ErrorList parsingErrors;
				auto result = ParseInternal(text, parsingErrors);
				GuiResourceError::Transform(location, errors, parsingErrors);
				return result;
			}

			Ptr<T> Parse(GuiResourceLocation location, const WString& text, glr::ParsingTextPos position, collections::List<GuiResourceError>& errors)
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
		public:
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
		};

		/// <summary>Get the global <see cref="IGuiParserManager"/> object.</summary>
		/// <returns>The parser manager.</returns>
		extern IGuiParserManager*					GetParserManager();

/***********************************************************************
Parser Manager
***********************************************************************/

		template<typename T>
		Ptr<IGuiParser<T>> IGuiParserManager::GetParser(const WString& name)
		{
			return GetParser(name).Cast<IGuiParser<T>>();
		}
	}
}

#endif