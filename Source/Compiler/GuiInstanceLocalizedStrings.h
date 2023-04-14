/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Shared Script

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCELOCALIZEDSTRINGS
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCELOCALIZEDSTRINGS

#include "../Resources/GuiResource.h"
#include <VlppWorkflowCompiler.h>

namespace vl
{
	namespace presentation
	{
		class GuiInstanceLocalizedStringsBase : public Description<GuiInstanceLocalizedStringsBase>
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

			collections::List<Ptr<Strings>>				strings;
			GuiResourceTextPos							tagPosition;

		public:
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

		public:
			virtual Ptr<Strings>						GetDefaultStrings() = 0;
			virtual WString								GetInterfaceTypeName(bool hasNamespace) = 0;
		};

		class GuiInstanceLocalizedStrings : public GuiInstanceLocalizedStringsBase, public Description<GuiInstanceLocalizedStrings>
		{
		public:
			WString										className;
			WString										defaultLocale;

		public:

			static Ptr<GuiInstanceLocalizedStrings>		LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<glr::xml::XmlElement>					SaveToXml();

			Ptr<Strings>								GetDefaultStrings() override;
			WString										GetInterfaceTypeName(bool hasNamespace) override;

			Ptr<TextDesc>								ParseLocalizedText(const WString& text, GuiResourceTextPos pos, GuiResourceError::List& errors);
			void										Validate(TextDescMap& textDescs, GuiResourcePrecompileContext& precompileContext, GuiResourceError::List& errors);
			Ptr<workflow::WfFunctionDeclaration>		GenerateFunction(Ptr<TextDesc> textDesc, const WString& functionName, workflow::WfFunctionKind functionKind);
			WString										GenerateStringsCppName(Ptr<Strings> ls);
			Ptr<workflow::WfExpression>					GenerateStrings(TextDescMap& textDescs, Ptr<Strings> ls);
			Ptr<workflow::WfFunctionDeclaration>		GenerateStringsFunction(const WString& name, TextDescMap& textDescs, Ptr<Strings> ls);
			Ptr<workflow::WfFunctionDeclaration>		GenerateInstallFunction(const WString& cacheName);
			Ptr<workflow::WfFunctionDeclaration>		GenerateGetFunction(const WString& cacheName);
			Ptr<workflow::WfModule>						Compile(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, GuiResourceError::List& errors);
		};

		class GuiInstanceLocalizedStringsInjection : public GuiInstanceLocalizedStringsBase, public Description<GuiInstanceLocalizedStringsInjection>
		{
		public:
			WString										className;
			WString										extendFromClassName;

		public:

			static Ptr<GuiInstanceLocalizedStringsInjection>		LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<glr::xml::XmlElement>								SaveToXml();

			Ptr<Strings>								GetDefaultStrings() override;
			WString										GetInterfaceTypeName(bool hasNamespace) override;
			Ptr<workflow::WfModule>						Compile(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, GuiResourceError::List& errors);
		};
	}
}

#endif