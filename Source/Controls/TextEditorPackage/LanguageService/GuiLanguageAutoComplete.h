/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUILANGUAGEAUTOCOMPLETE
#define VCZH_PRESENTATION_CONTROLS_GUILANGUAGEAUTOCOMPLETE

#include "GuiLanguageOperations.h"
#include "../EditorCallback/GuiTextAutoComplete.h"
namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiGrammarAutoComplete
***********************************************************************/
			
			/// <summary>Grammar based auto complete controller.</summary>
			class GuiGrammarAutoComplete
				: public GuiTextBoxAutoCompleteBase
				, private RepeatingParsingExecutor::CallbackBase
				, private RepeatingTaskExecutor<RepeatingParsingOutput>
			{
			public:

				/// <summary>The auto complete list data.</summary>
				struct AutoCompleteData : ParsingTokenContext
				{
					/// <summary>Available candidate tokens (in lexer token index).</summary>
					collections::List<vint>							candidates;
					/// <summary>Available candidate tokens (in lexer token index) that marked with @AutoCompleteCandidate().</summary>
					collections::List<vint>							shownCandidates;
					/// <summary>Candidate items.</summary>
					collections::List<ParsingCandidateItem>			candidateItems;
					/// <summary>The start position of the editing token in global coordination.</summary>
					TextPos											startPosition;
				};

				/// <summary>The analysed data from an input code.</summary>
				struct AutoCompleteContext : RepeatingPartialParsingOutput
				{
					/// <summary>The edit version of modified code.</summary>
					vuint											modifiedEditVersion = 0;
					/// <summary>The analysed auto complete list data.</summary>
					Ptr<AutoCompleteData>							autoComplete;
				};
			private:
				Ptr<parsing::tabling::ParsingGeneralParser>			grammarParser;
				collections::SortedList<WString>					leftRecursiveRules;
				bool												editing;

				SpinLock											editTraceLock;
				collections::List<TextEditNotifyStruct>				editTrace;

				SpinLock											contextLock;
				AutoCompleteContext									context;
				
				void												Attach(elements::GuiColorizedTextElement* _element, SpinLock& _elementModifyLock, compositions::GuiGraphicsComposition* _ownerComposition, vuint editVersion)override;
				void												Detach()override;
				void												TextEditPreview(TextEditPreviewStruct& arguments)override;
				void												TextEditNotify(const TextEditNotifyStruct& arguments)override;
				void												TextCaretChanged(const TextCaretChangedStruct& arguments)override;
				void												TextEditFinished(vuint editVersion)override;
				void												OnParsingFinishedAsync(const RepeatingParsingOutput& output)override;
				void												CollectLeftRecursiveRules();

				vint												UnsafeGetEditTraceIndex(vuint editVersion);
				TextPos												ChooseCorrectTextPos(TextPos pos, const regex::RegexTokens& tokens);
				void												ExecuteRefresh(AutoCompleteContext& newContext);

				bool												NormalizeTextPos(AutoCompleteContext& newContext, elements::text::TextLines& lines, TextPos& pos);
				void												ExecuteEdit(AutoCompleteContext& newContext);

				void												DeleteFutures(collections::List<parsing::tabling::ParsingState::Future*>& futures);
				regex::RegexToken*									TraverseTransitions(
																		parsing::tabling::ParsingState& state,
																		parsing::tabling::ParsingTransitionCollector& transitionCollector,
																		TextPos stopPosition,
																		collections::List<parsing::tabling::ParsingState::Future*>& nonRecoveryFutures,
																		collections::List<parsing::tabling::ParsingState::Future*>& recoveryFutures
																		);
				regex::RegexToken*									SearchValidInputToken(
																		parsing::tabling::ParsingState& state,
																		parsing::tabling::ParsingTransitionCollector& transitionCollector,
																		TextPos stopPosition,
																		AutoCompleteContext& newContext,
																		collections::SortedList<vint>& tableTokenIndices
																		);

				TextPos												GlobalTextPosToModifiedTextPos(AutoCompleteContext& newContext, TextPos pos);
				TextPos												ModifiedTextPosToGlobalTextPos(AutoCompleteContext& newContext, TextPos pos);
				void												ExecuteCalculateList(AutoCompleteContext& newContext);

				void												Execute(const RepeatingParsingOutput& input)override;
				void												PostList(const AutoCompleteContext& newContext, bool byGlobalCorrection);
				void												Initialize();
			protected:

				/// <summary>Called when the context of the code is selected. It is encouraged to set the "candidateItems" field in "context.autoComplete" during the call. If there is an <see cref="RepeatingParsingExecutor::IParsingAnalyzer"/> binded to the <see cref="RepeatingParsingExecutor"/>, this function can be automatically done.</summary>
				/// <param name="context">The selected context.</param>
				virtual void										OnContextFinishedAsync(AutoCompleteContext& context);

				/// <summary>Call this function in the derived class's destructor when it overrided <see cref="OnContextFinishedAsync"/>.</summary>
				void												EnsureAutoCompleteFinished();
			public:
				/// <summary>Create the auto complete controller with a created parsing executor.</summary>
				/// <param name="_parsingExecutor">The parsing executor.</param>
				GuiGrammarAutoComplete(Ptr<RepeatingParsingExecutor> _parsingExecutor);
				/// <summary>Create the auto complete controller with a specified grammar and start rule to create a <see cref="RepeatingParsingExecutor"/>.</summary>
				/// <param name="_grammarParser">Parser generated from a grammar.</param>
				/// <param name="_grammarRule"></param>
				GuiGrammarAutoComplete(Ptr<parsing::tabling::ParsingGeneralParser> _grammarParser, const WString& _grammarRule);
				~GuiGrammarAutoComplete();

				/// <summary>Get the internal parsing executor.</summary>
				/// <returns>The parsing executor.</returns>
				Ptr<RepeatingParsingExecutor>						GetParsingExecutor();
			};
		}
	}
}

#endif