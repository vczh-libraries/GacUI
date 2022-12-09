#include "GuiLanguageColorizer.h"
#include "../../Templates/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace parsing;
			using namespace parsing::tabling;
			using namespace collections;
			using namespace theme;

/***********************************************************************
GuiGrammarColorizer
***********************************************************************/

			void GuiGrammarColorizer::OnParsingFinishedAsync(const RepeatingParsingOutput& output)
			{
				SPIN_LOCK(contextLock)
				{
					context=output;
					OnContextFinishedAsync(context);
				}
				RestartColorizer();
			}

			void GuiGrammarColorizer::OnContextFinishedAsync(const RepeatingParsingOutput& context)
			{
			}

			void GuiGrammarColorizer::Attach(elements::GuiColorizedTextElement* _element, SpinLock& _elementModifyLock, compositions::GuiGraphicsComposition* _ownerComposition, vuint editVersion)
			{
				GuiTextBoxRegexColorizer::Attach(_element, _elementModifyLock, _ownerComposition, editVersion);
				RepeatingParsingExecutor::CallbackBase::Attach(_element, _elementModifyLock, _ownerComposition, editVersion);
			}

			void GuiGrammarColorizer::Detach()
			{
				GuiTextBoxRegexColorizer::Detach();
				RepeatingParsingExecutor::CallbackBase::Detach();
				if(element && elementModifyLock)
				{
					parsingExecutor->EnsureTaskFinished();
					StopColorizer(false);
				}
			}

			void GuiGrammarColorizer::TextEditPreview(TextEditPreviewStruct& arguments)
			{
				GuiTextBoxRegexColorizer::TextEditPreview(arguments);
				RepeatingParsingExecutor::CallbackBase::TextEditPreview(arguments);
			}

			void GuiGrammarColorizer::TextEditNotify(const TextEditNotifyStruct& arguments)
			{
				GuiTextBoxRegexColorizer::TextEditNotify(arguments);
				RepeatingParsingExecutor::CallbackBase::TextEditNotify(arguments);
			}

			void GuiGrammarColorizer::TextCaretChanged(const TextCaretChangedStruct& arguments)
			{
				GuiTextBoxRegexColorizer::TextCaretChanged(arguments);
				RepeatingParsingExecutor::CallbackBase::TextCaretChanged(arguments);
			}

			void GuiGrammarColorizer::TextEditFinished(vuint editVersion)
			{
				GuiTextBoxRegexColorizer::TextEditFinished(editVersion);
				RepeatingParsingExecutor::CallbackBase::TextEditFinished(editVersion);
			}

			void GuiGrammarColorizer::OnSemanticColorize(SemanticColorizeContext& context, const RepeatingParsingOutput& input)
			{
				if (auto analyzer = parsingExecutor->GetAnalyzer())
				{
					auto semanticId = analyzer->GetSemanticIdForTokenAsync(context, input);
					if(semanticId!=-1)
					{
						context.semanticId=semanticId;
					}
				}
			}

			void GuiGrammarColorizer::EnsureColorizerFinished()
			{
				parsingExecutor->EnsureTaskFinished();
				StopColorizerForever();
				SPIN_LOCK(contextLock)
				{
					context=RepeatingParsingOutput();
				}
			}

			GuiGrammarColorizer::GuiGrammarColorizer(Ptr<RepeatingParsingExecutor> _parsingExecutor)
				:RepeatingParsingExecutor::CallbackBase(_parsingExecutor)
			{
				parsingExecutor->AttachCallback(this);
				BeginSetColors();
			}

			GuiGrammarColorizer::GuiGrammarColorizer(Ptr<parsing::tabling::ParsingGeneralParser> _grammarParser, const WString& _grammarRule)
				:RepeatingParsingExecutor::CallbackBase(Ptr(new RepeatingParsingExecutor(_grammarParser, _grammarRule)))
			{
				parsingExecutor->AttachCallback(this);
				BeginSetColors();
			}

			GuiGrammarColorizer::~GuiGrammarColorizer()
			{
				EnsureColorizerFinished();
				parsingExecutor->DetachCallback(this);
			}

			void GuiGrammarColorizer::BeginSetColors()
			{
				ClearTokens();
				colorSettings.Clear();

				text::ColorEntry entry;
				{
					entry.normal.text = Color(0, 0, 0);
					entry.normal.background = Color(0, 0, 0, 0);
					entry.selectedFocused.text = Color(255, 255, 255);
					entry.selectedFocused.background = Color(51, 153, 255);
					entry.selectedUnfocused.text = Color(255, 255, 255);
					entry.selectedUnfocused.background = Color(51, 153, 255);
				}

				SetDefaultColor(entry);
				colorSettings.Add(L"Default", entry);
			}

			const collections::SortedList<WString>& GuiGrammarColorizer::GetColorNames()
			{
				return colorSettings.Keys();
			}

			GuiGrammarColorizer::ColorEntry GuiGrammarColorizer::GetColor(const WString& name)
			{
				vint index=colorSettings.Keys().IndexOf(name);
				return index==-1?GetDefaultColor():colorSettings.Values().Get(index);
			}

			void GuiGrammarColorizer::SetColor(const WString& name, const ColorEntry& entry)
			{
				colorSettings.Set(name, entry);
			}

			void GuiGrammarColorizer::SetColor(const WString& name, const Color& color)
			{
				text::ColorEntry entry=GetDefaultColor();
				entry.normal.text=color;
				SetColor(name, entry);
			}

			void GuiGrammarColorizer::EndSetColors()
			{
				SortedList<WString> tokenColors;
				Ptr<ParsingTable> table=parsingExecutor->GetParser()->GetTable();
				semanticColorMap.Clear();

				vint tokenCount=table->GetTokenCount();
				for(vint token=ParsingTable::UserTokenStart;token<tokenCount;token++)
				{
					const ParsingTable::TokenInfo& tokenInfo=table->GetTokenInfo(token);
					const RepeatingParsingExecutor::TokenMetaData& md=parsingExecutor->GetTokenMetaData(token-ParsingTable::UserTokenStart);
					if(md.defaultColorIndex==-1)
					{
						AddToken(tokenInfo.regex, GetDefaultColor());
					}
					else
					{
						WString name=parsingExecutor->GetSemanticName(md.defaultColorIndex);
						vint color=AddToken(tokenInfo.regex, GetColor(name));
						semanticColorMap.Set(md.defaultColorIndex, color);
						tokenColors.Add(name);
					}
				}

				for (auto [color, index] : indexed(colorSettings.Keys()))
				{
					if(!tokenColors.Contains(color))
					{
						vint semanticId=parsingExecutor->GetSemanticId(color);
						if(semanticId!=-1)
						{
							vint tokenId=AddExtraToken(colorSettings.Values().Get(index));
							vint color=tokenId+tokenCount-ParsingTable::UserTokenStart;
							semanticColorMap.Set(semanticId, color);
						}
					}
				}
				Setup();
			}

			void GuiGrammarColorizer::ColorizeTokenContextSensitive(vint lineIndex, const wchar_t* text, vint start, vint length, vint& token, vint& contextState)
			{
				SPIN_LOCK(contextLock)
				{
					ParsingTreeObject* node=context.node.Obj();
					if(node && token!=-1 && parsingExecutor->GetTokenMetaData(token).hasContextColor)
					{
						ParsingTextPos pos(lineIndex, start);
						SemanticColorizeContext scContext;
						if(SemanticColorizeContext::RetriveContext(scContext, pos, node, parsingExecutor.Obj()))
						{
							const RepeatingParsingExecutor::FieldMetaData& md=parsingExecutor->GetFieldMetaData(scContext.type, scContext.field);
							vint semantic=md.colorIndex;
							scContext.semanticId=-1;

							if(scContext.acceptableSemanticIds)
							{
								OnSemanticColorize(scContext, context);
								if(md.semantics->Contains(scContext.semanticId))
								{
									semantic=scContext.semanticId;
								}
							}

							if(semantic!=-1)
							{
								vint index=semanticColorMap.Keys().IndexOf(semantic);
								if(index!=-1)
								{
									token=semanticColorMap.Values()[index];
								}
							}
						}
					}
				}
			}

			Ptr<RepeatingParsingExecutor> GuiGrammarColorizer::GetParsingExecutor()
			{
				return parsingExecutor;
			}
		}
	}
}