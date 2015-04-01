#include "GuiParserManager.h"
#include "../Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace controls;
		using namespace parsing::tabling;
		using namespace parsing::xml;
		using namespace parsing::json;
		using namespace regex;

/***********************************************************************
IGuiParserManager
***********************************************************************/

		IGuiParserManager* parserManager=0;

		IGuiParserManager* GetParserManager()
		{
			return parserManager;
		}

		class GuiParserManager : public Object, public IGuiParserManager, public IGuiPlugin
		{
		protected:
			Dictionary<WString, Ptr<Table>>				tables;
			Dictionary<WString, Func<Ptr<Table>()>>		loaders;
			SpinLock									lock;

			Dictionary<WString, Ptr<IGuiGeneralParser>>	parsers;
		public:
			GuiParserManager()
			{
			}

			~GuiParserManager()
			{
			}

			void Load()override
			{
				parserManager=this;
				SetParsingTable(L"XML", &XmlLoadTable);
				SetParsingTable(L"JSON", &JsonLoadTable);
				SetTableParser(L"XML", L"XML", &XmlParseDocument);
				SetTableParser(L"JSON", L"JSON", &JsonParse);
			}

			void AfterLoad()override
			{
			}

			void Unload()override
			{
				parserManager=0;
			}

			Ptr<Table> GetParsingTable(const WString& name)override
			{
				SPIN_LOCK(lock)
				{
					vint index=tables.Keys().IndexOf(name);
					if(index!=-1)
					{
						return tables.Values()[index];
					}

					index=loaders.Keys().IndexOf(name);
					if(index!=-1)
					{
						Ptr<Table> table=loaders.Values()[index]();
						tables.Add(name, table);
						return table;
					}
				}
				return 0;
			}

			bool SetParsingTable(const WString& name, Func<Ptr<Table>()> loader)override
			{
				if(loaders.Keys().Contains(name)) return false;
				loaders.Add(name, loader);
				return true;
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