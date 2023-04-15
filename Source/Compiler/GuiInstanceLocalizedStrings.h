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
				WString												name;
				WString												text;
				GuiResourceTextPos									textPosition;
			};

			struct Strings
			{
				using StringItemMap = collections::Dictionary<WString, Ptr<StringItem>>;

				collections::List<WString>							locales;
				StringItemMap										items;
				GuiResourceTextPos									tagPosition;

				WString												GetLocalesName();
			};

			collections::List<Ptr<Strings>>							strings;
			GuiResourceTextPos										tagPosition;

		public:
			using ParameterPair = collections::Pair<Ptr<reflection::description::ITypeInfo>, WString>;
			using ParameterList = collections::List<ParameterPair>;
			using PositionList = collections::List<vint>;
			using TextList = collections::List<WString>;

			struct TextDesc
			{
				ParameterList										parameters;
				PositionList										positions;
				TextList											texts;
			};

			using TextDescMap = collections::Dictionary<collections::Pair<Ptr<Strings>, WString>, Ptr<TextDesc>>;

		protected:
			static Ptr<Strings>										LoadStringsFromXml(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlElement> xmlStrings, collections::SortedList<WString>& existingLocales, GuiResourceError::List& errors);
			static Ptr<glr::xml::XmlElement>						SaveStringsToXml(Ptr<Strings> lss);
			static Ptr<TextDesc>									ParseLocalizedText(const WString& text, GuiResourceTextPos pos, GuiResourceError::List& errors);

			static void												FillStringsToTextDescMap(Ptr<Strings> lss, TextDescMap& textDescs, GuiResourceError::List& errors);
			static void												ValidateNamesAgainstDefaultStrings(Ptr<Strings> defaultStrings, Ptr<Strings> lss, GuiResourceError::List& errors);
			static void												ValidateSignatureAgainstDefaultStrings(Ptr<Strings> defaultStrings, Ptr<Strings> lss, TextDescMap& textDescs, GuiResourceError::List& errors);
			static void												ValidateAgainstDefaultStrings(Ptr<Strings> defaultStrings, collections::List<Ptr<Strings>>& nonDefaultStrings, TextDescMap& textDescs, GuiResourceError::List& errors);

			static WString											GetInterfaceTypeName(const WString& className, bool hasNamespace);
			static WString											GenerateStringsCppName(Ptr<Strings> lss);
			static Ptr<workflow::WfFunctionDeclaration>				GenerateTextDescFunctionHeader(Ptr<TextDesc> textDesc, const WString& functionName, workflow::WfFunctionKind functionKind);
			static Ptr<workflow::WfExpression>						GenerateTextDescArgumentFormatting(Ptr<description::ITypeInfo> type, const WString& function, vint argumentIndex);
			static Ptr<workflow::WfBlockStatement>					GenerateTextDescFunctionBody(Ptr<TextDesc> textDesc);
			static Ptr<workflow::WfExpression>						GenerateStringsConstructor(const WString& interfaceFullName, TextDescMap& textDescs, Ptr<Strings> lss);
			static Ptr<workflow::WfFunctionDeclaration>				GenerateBuildStringsFunction(const WString& interfaceFullName, TextDescMap& textDescs, Ptr<Strings> lss);
			static Ptr<workflow::WfBlockStatement>					GenerateStaticInit(const WString& stringsClassWithoutNs, const WString& installClassFullName, collections::List<Ptr<Strings>>& strings);
		};

		class GuiInstanceLocalizedStrings : public GuiInstanceLocalizedStringsBase, public Description<GuiInstanceLocalizedStrings>
		{
		public:
			WString													className;
			WString													defaultLocale;
			Ptr<Strings>											defaultStrings;

		public:

			static Ptr<GuiInstanceLocalizedStrings>					LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<glr::xml::XmlElement>								SaveToXml();

			Ptr<workflow::WfFunctionDeclaration>					GenerateInstallFunction(const WString& cacheName);
			Ptr<workflow::WfFunctionDeclaration>					GenerateGetFunction(const WString& cacheName);
			Ptr<workflow::WfModule>									Compile(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, GuiResourceError::List& errors);
		};

		class GuiInstanceLocalizedStringsInjection : public GuiInstanceLocalizedStringsBase, public Description<GuiInstanceLocalizedStringsInjection>
		{
		public:
			WString													className;
			WString													injectIntoClassName;

		public:

			static Ptr<GuiInstanceLocalizedStringsInjection>		LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<glr::xml::XmlElement>								SaveToXml();

			void													DecompileDefaultStrings(description::ITypeDescriptor* td, Ptr<Strings> defaultStrings, TextDescMap& textDescs, GuiResourceError::List& errors);
			Ptr<workflow::WfModule>									Compile(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, GuiResourceError::List& errors);
		};
	}
}

#endif