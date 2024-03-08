#include "GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace glr::xml;
		using namespace glr::json;
		using namespace regex;

/***********************************************************************
IGuiParserManager
***********************************************************************/

		IGuiParserManager* parserManager=0;

		IGuiParserManager* GetParserManager()
		{
			return parserManager;
		}

		class GuiParser_Xml : public IGuiParser<XmlDocument>
		{
		protected:
			glr::xml::Parser							parser;

		public:
			Ptr<XmlDocument> ParseInternal(const WString& text, List<glr::ParsingError>& errors) override
			{
				auto handler = glr::InstallDefaultErrorMessageGenerator(parser, errors);
				auto ast = XmlParseDocument(text, parser);
				parser.OnError.Remove(handler);
				return ast;
			}
		};

		class GuiParser_Json : public IGuiParser<JsonNode>
		{
		protected:
			glr::json::Parser							parser;

		public:
			Ptr<JsonNode> ParseInternal(const WString& text, List<glr::ParsingError>& errors) override
			{
				auto handler = glr::InstallDefaultErrorMessageGenerator(parser, errors);
				auto ast = JsonParse(text, parser);
				parser.OnError.Remove(handler);
				return ast;
			}
		};

		class GuiParserManager : public Object, public IGuiParserManager, public IGuiPlugin
		{
		protected:
			SpinLock									lock;

			Dictionary<WString, Ptr<IGuiGeneralParser>>	parsers;
		public:
			
			GUI_PLUGIN_NAME(GacUI_Parser)
			{
			}

			void Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
			{
				if (controllerUnrelatedPlugins)
				{
					parserManager = this;
					SetParser(L"XML", Ptr(new GuiParser_Xml()));
					SetParser(L"JSON", Ptr(new GuiParser_Json()));
				}
			}

			void Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
			{
				if (controllerUnrelatedPlugins)
				{
					parserManager = nullptr;
				}
			}

			Ptr<IGuiGeneralParser> GetParser(const WString& name)override
			{
				vint index=parsers.Keys().IndexOf(name);
				return index == -1 ? nullptr : parsers.Values()[index];
			}

			bool SetParser(const WString& name, Ptr<IGuiGeneralParser> parser)override
			{
				if(parsers.Keys().Contains(name)) return false;
				parsers.Add(name, parser);
				return true;
			}
		};
		GUI_REGISTER_PLUGIN(GuiParserManager)
	}
}