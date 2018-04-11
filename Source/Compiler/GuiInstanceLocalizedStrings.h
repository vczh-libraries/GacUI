/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Shared Script

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCELOCALIZEDSTRINGS
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCELOCALIZEDSTRINGS

#include "../Resources/GuiResource.h"
#include "../../Import/VlppWorkflowCompiler.h"

namespace vl
{
	namespace presentation
	{
		class GuiInstanceLocalizedStrings : public Object, public Description<GuiInstanceLocalizedStrings>
		{
		public:
			struct StringItem
			{
			public:
				WString									name;
				WString									text;
				GuiResourceTextPos						textPosition;
			};

			struct Strings
			{
				using StringItemMap = collections::Dictionary<WString, Ptr<StringItem>>;

				collections::List<WString>				locales;
				StringItemMap							items;
				GuiResourceTextPos						tagPosition;

				WString									GetLocalesName();
			};

			WString										className;
			WString										defaultLocale;
			collections::List<Ptr<Strings>>				strings;
			GuiResourceTextPos							tagPosition;

			using ParameterPair = collections::Pair<Ptr<reflection::description::ITypeInfo>, WString>;
			using ParameterList = collections::List<ParameterPair>;
			using PositionList = collections::List<vint>;
			using TextList = collections::List<WString>;

			struct TextDesc
			{
				ParameterList							parameters;
				PositionList							positions;
				TextList								texts;
			};

			using TextDescMap = collections::Dictionary<collections::Pair<Ptr<Strings>, WString>, Ptr<TextDesc>>;

			static Ptr<GuiInstanceLocalizedStrings>		LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<parsing::xml::XmlElement>				SaveToXml();

			Ptr<Strings>								GetDefaultStrings();
			WString										GetInterfaceTypeName(bool hasNamespace);

			Ptr<TextDesc>								ParseLocalizedText(const WString& text, GuiResourceTextPos pos, GuiResourceError::List& errors);
			void										Validate(TextDescMap& textDescs, GuiResourcePrecompileContext& precompileContext, GuiResourceError::List& errors);
			Ptr<workflow::WfFunctionDeclaration>		GenerateFunction(Ptr<TextDesc> textDesc, const WString& functionName, workflow::WfClassMemberKind classMemberKind);
			Ptr<workflow::WfExpression>					GenerateStrings(TextDescMap& textDescs, Ptr<Strings> ls);
			Ptr<workflow::WfModule>						Compile(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, GuiResourceError::List& errors);
		};
	}
}

#endif