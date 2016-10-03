/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUILANGUAGECOLORIZER
#define VCZH_PRESENTATION_CONTROLS_GUILANGUAGECOLORIZER

#include "GuiLanguageOperations.h"
#include "../EditorCallback/GuiTextColorizer.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiGrammarColorizer
***********************************************************************/

			/// <summary>Grammar based colorizer.</summary>
			class GuiGrammarColorizer : public GuiTextBoxRegexColorizer, protected RepeatingParsingExecutor::CallbackBase
			{
				typedef collections::Pair<WString, WString>					FieldDesc;
				typedef collections::Dictionary<FieldDesc, vint>			FieldContextColors;
				typedef collections::Dictionary<FieldDesc, vint>			FieldSemanticColors;
				typedef elements::text::ColorEntry							ColorEntry;
			public:
				/// <summary>Context for doing semantic colorizing.</summary>
				struct SemanticColorizeContext : ParsingTokenContext
				{
					/// <summary>Output semantic id that comes from one the argument in the @Semantic attribute.</summary>
					vint													semanticId;
				};
			private:
				collections::Dictionary<WString, ColorEntry>				colorSettings;
				collections::Dictionary<vint, vint>							semanticColorMap;

				SpinLock													contextLock;
				RepeatingParsingOutput										context;

				void														OnParsingFinishedAsync(const RepeatingParsingOutput& output)override;
			protected:
				/// <summary>Called when the node is parsed successfully before restarting colorizing.</summary>
				/// <param name="context">The result of the parsing.</param>
				virtual void												OnContextFinishedAsync(const RepeatingParsingOutput& context);

				void														Attach(elements::GuiColorizedTextElement* _element, SpinLock& _elementModifyLock, compositions::GuiGraphicsComposition* _ownerComposition, vuint editVersion)override;
				void														Detach()override;
				void														TextEditPreview(TextEditPreviewStruct& arguments)override;
				void														TextEditNotify(const TextEditNotifyStruct& arguments)override;
				void														TextCaretChanged(const TextCaretChangedStruct& arguments)override;
				void														TextEditFinished(vuint editVersion)override;

				/// <summary>Called when a @SemanticColor attribute in a grammar is activated during colorizing to determine a color for the token. If there is an <see cref="RepeatingParsingExecutor::IParsingAnalyzer"/> binded to the <see cref="RepeatingParsingExecutor"/>, this function can be automatically done.</summary>
				/// <param name="context">Context for doing semantic colorizing.</param>
				/// <param name="input">The corressponding result from the <see cref="RepeatingParsingExecutor"/>.</param>
				virtual void												OnSemanticColorize(SemanticColorizeContext& context, const RepeatingParsingOutput& input);

				/// <summary>Call this function in the derived class's destructor when it overrided <see cref="OnSemanticColorize"/>.</summary>
				void														EnsureColorizerFinished();
			public:
				/// <summary>Create the colorizer with a created parsing executor.</summary>
				/// <param name="_parsingExecutor">The parsing executor.</param>
				GuiGrammarColorizer(Ptr<RepeatingParsingExecutor> _parsingExecutor);
				/// <summary>Create the colorizer with a specified grammar and start rule to create a <see cref="RepeatingParsingExecutor"/>.</summary>
				/// <param name="_grammarParser">Parser generated from a grammar.</param>
				/// <param name="_grammarRule"></param>
				GuiGrammarColorizer(Ptr<parsing::tabling::ParsingGeneralParser> _grammarParser, const WString& _grammarRule);
				~GuiGrammarColorizer();

				/// <summary>Reset all color settings.</summary>
				void														BeginSetColors();
				/// <summary>Get all color names.</summary>
				/// <returns>All color names.</returns>
				const collections::SortedList<WString>&						GetColorNames();
				/// <summary>Get the color for a token theme name (@Color or @ContextColor("theme-name") in the grammar).</summary>
				/// <returns>The color.</returns>
				/// <param name="name">The token theme name.</param>
				ColorEntry													GetColor(const WString& name);
				/// <summary>Set a color for a token theme name (@Color or @ContextColor("theme-name") in the grammar).</summary>
				/// <param name="name">The token theme name.</param>
				/// <param name="entry">The color.</param>
				void														SetColor(const WString& name, const ColorEntry& entry);
				/// <summary>Set a color for a token theme name (@Color or @ContextColor("theme-name") in the grammar).</summary>
				/// <param name="name">The token theme name.</param>
				/// <param name="color">The color.</param>
				void														SetColor(const WString& name, const Color& color);
				/// <summary>Submit all color settings.</summary>
				void														EndSetColors();
				void														ColorizeTokenContextSensitive(vint lineIndex, const wchar_t* text, vint start, vint length, vint& token, vint& contextState)override;

				/// <summary>Get the internal parsing executor.</summary>
				/// <returns>The parsing executor.</returns>
				Ptr<RepeatingParsingExecutor>								GetParsingExecutor();
			};
		}
	}
}

#endif