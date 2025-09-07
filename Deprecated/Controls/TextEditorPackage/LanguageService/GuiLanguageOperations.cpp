#include "GuiLanguageOperations.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace parsing;
			using namespace parsing::tabling;
			using namespace regex_internal;

/***********************************************************************
ParsingContext
***********************************************************************/

			bool ParsingTokenContext::RetriveContext(ParsingTokenContext& output, parsing::ParsingTreeNode* foundNode, RepeatingParsingExecutor* executor)
			{
				ParsingTreeToken* foundToken=dynamic_cast<ParsingTreeToken*>(foundNode);
				if(!foundToken) return false;
				ParsingTreeObject* tokenParent=dynamic_cast<ParsingTreeObject*>(foundNode->GetParent());
				if(!tokenParent) return false;
				vint index=tokenParent->GetMembers().Values().IndexOf(foundNode);
				if(index==-1) return false;

				WString type=tokenParent->GetType();
				WString field=tokenParent->GetMembers().Keys().Get(index);
				const RepeatingParsingExecutor::FieldMetaData& md=executor->GetFieldMetaData(type, field);

				output.foundToken=foundToken;
				output.tokenParent=tokenParent;
				output.type=type;
				output.field=field;
				output.acceptableSemanticIds=md.semantics;
				return true;
			}

			bool ParsingTokenContext::RetriveContext(ParsingTokenContext& output, parsing::ParsingTextPos pos, parsing::ParsingTreeObject* rootNode, RepeatingParsingExecutor* executor)
			{
				ParsingTreeNode* foundNode=rootNode->FindDeepestNode(pos);
				if(!foundNode) return false;
				return RetriveContext(output, foundNode, executor);
			}

			bool ParsingTokenContext::RetriveContext(ParsingTokenContext& output, parsing::ParsingTextRange range, ParsingTreeObject* rootNode, RepeatingParsingExecutor* executor)
			{
				ParsingTreeNode* foundNode=rootNode->FindDeepestNode(range);
				if(!foundNode) return false;
				return RetriveContext(output, foundNode, executor);
			}

/***********************************************************************
RepeatingParsingExecutor::IParsingAnalyzer
***********************************************************************/

		parsing::ParsingTreeNode* RepeatingParsingExecutor::IParsingAnalyzer::ToParent(parsing::ParsingTreeNode* node, const RepeatingPartialParsingOutput* output)
		{
			if (!output || !output->modifiedNode) return node;
			return node == output->modifiedNode.Obj()
				? output->originalNode.Obj()
				: node;
		}

		parsing::ParsingTreeObject* RepeatingParsingExecutor::IParsingAnalyzer::ToChild(parsing::ParsingTreeObject* node, const RepeatingPartialParsingOutput* output)
		{
			if (!output || !output->modifiedNode) return node;
			return node == output->originalNode.Obj()
				? output->modifiedNode.Obj()
				: node;
		}

		Ptr<parsing::ParsingTreeNode> RepeatingParsingExecutor::IParsingAnalyzer::ToChild(Ptr<parsing::ParsingTreeNode> node, const RepeatingPartialParsingOutput* output)
		{
			if (!output) return node;
			return node == output->originalNode
				? output->modifiedNode.Cast<ParsingTreeNode>()
				: node;
		}

		parsing::ParsingTreeNode* RepeatingParsingExecutor::IParsingAnalyzer::GetParent(parsing::ParsingTreeNode* node, const RepeatingPartialParsingOutput* output)
		{
			return ToParent(node, output)->GetParent();
		}

		Ptr<parsing::ParsingTreeNode> RepeatingParsingExecutor::IParsingAnalyzer::GetMember(parsing::ParsingTreeObject* node, const WString& name, const RepeatingPartialParsingOutput* output)
		{
			return ToChild(ToChild(node, output)->GetMember(name), output);
		}

		Ptr<parsing::ParsingTreeNode> RepeatingParsingExecutor::IParsingAnalyzer::GetItem(parsing::ParsingTreeArray* node, vint index, const RepeatingPartialParsingOutput* output)
		{
			return ToChild(node->GetItem(index), output);
		}

/***********************************************************************
RepeatingParsingExecutor::CallbackBase
***********************************************************************/

			RepeatingParsingExecutor::CallbackBase::CallbackBase(Ptr<RepeatingParsingExecutor> _parsingExecutor)
				:parsingExecutor(_parsingExecutor)
				,callbackAutoPushing(false)
				,callbackElement(0)
				,callbackElementModifyLock(0)
			{
			}

			RepeatingParsingExecutor::CallbackBase::~CallbackBase()
			{
			}

			void RepeatingParsingExecutor::CallbackBase::RequireAutoSubmitTask(bool enabled)
			{
				callbackAutoPushing=enabled;
			}

			void RepeatingParsingExecutor::CallbackBase::Attach(elements::GuiColorizedTextElement* _element, SpinLock& _elementModifyLock, compositions::GuiGraphicsComposition* _ownerComposition, vuint editVersion)
			{
				if(_element)
				{
					SPIN_LOCK(_elementModifyLock)
					{
						callbackElement=_element;
						callbackElementModifyLock=&_elementModifyLock;
					}
				}
				
				parsingExecutor->ActivateCallback(this);
				if(callbackElement && callbackElementModifyLock && callbackAutoPushing)
				{
					SPIN_LOCK(*callbackElementModifyLock)
					{
						RepeatingParsingInput input;
						input.editVersion=editVersion;
						input.code=callbackElement->GetLines().GetText();
						parsingExecutor->SubmitTask(input);
					}
				}
			}

			void RepeatingParsingExecutor::CallbackBase::Detach()
			{
				if(callbackElement && callbackElementModifyLock)
				{
					SPIN_LOCK(*callbackElementModifyLock)
					{
						callbackElement=0;
						callbackElementModifyLock=0;
					}
				}
				
				parsingExecutor->DeactivateCallback(this);
			}

			void RepeatingParsingExecutor::CallbackBase::TextEditPreview(TextEditPreviewStruct& arguments)
			{
			}

			void RepeatingParsingExecutor::CallbackBase::TextEditNotify(const TextEditNotifyStruct& arguments)
			{
			}

			void RepeatingParsingExecutor::CallbackBase::TextCaretChanged(const TextCaretChangedStruct& arguments)
			{
			}

			void RepeatingParsingExecutor::CallbackBase::TextEditFinished(vuint editVersion)
			{
				if(callbackElement && callbackElementModifyLock && callbackAutoPushing)
				{
					SPIN_LOCK(*callbackElementModifyLock)
					{
						RepeatingParsingInput input;
						input.editVersion=editVersion;
						input.code=callbackElement->GetLines().GetText();
						parsingExecutor->SubmitTask(input);
					}
				}
			}

/***********************************************************************
RepeatingParsingExecutor
***********************************************************************/

			void RepeatingParsingExecutor::Execute(const RepeatingParsingInput& input)
			{
				List<Ptr<ParsingError>> errors;
				Ptr<ParsingTreeObject> node=grammarParser->Parse(input.code, grammarRule, errors).Cast<ParsingTreeObject>();
				if(node)
				{
					node->InitializeQueryCache();
				}

				RepeatingParsingOutput result;
				result.node=node;
				result.editVersion=input.editVersion;
				result.code=input.code;
				if(node)
				{
					OnContextFinishedAsync(result);
					for (auto callback : callbacks)
					{
						callback->OnParsingFinishedAsync(result);
					}
				}
			}

			void RepeatingParsingExecutor::PrepareMetaData()
			{
				Ptr<ParsingTable> table=grammarParser->GetTable();
				tokenIndexMap.Clear();
				semanticIndexMap.Clear();
				tokenMetaDatas.Clear();
				fieldMetaDatas.Clear();

				Dictionary<vint, Ptr<ParsingTable::AttributeInfo>> tokenColorAtts, tokenContextColorAtts, tokenCandidateAtts, tokenAutoCompleteAtts;
				Dictionary<FieldDesc, Ptr<ParsingTable::AttributeInfo>> fieldColorAtts, fieldSemanticAtts;

				{
					vint tokenCount=table->GetTokenCount();
					for(vint token=ParsingTable::UserTokenStart;token<tokenCount;token++)
					{
						const ParsingTable::TokenInfo& tokenInfo=table->GetTokenInfo(token);
						vint tokenIndex=token-ParsingTable::UserTokenStart;
						tokenIndexMap.Add(tokenInfo.name, tokenIndex);

						if(Ptr<ParsingTable::AttributeInfo> att=GetColorAttribute(tokenInfo.attributeIndex))
						{
							tokenColorAtts.Add(tokenIndex, att);
						}
						if(Ptr<ParsingTable::AttributeInfo> att=GetContextColorAttribute(tokenInfo.attributeIndex))
						{
							tokenContextColorAtts.Add(tokenIndex, att);
						}
						if(Ptr<ParsingTable::AttributeInfo> att=GetCandidateAttribute(tokenInfo.attributeIndex))
						{
							tokenCandidateAtts.Add(tokenIndex, att);
						}
						if(Ptr<ParsingTable::AttributeInfo> att=GetAutoCompleteAttribute(tokenInfo.attributeIndex))
						{
							tokenAutoCompleteAtts.Add(tokenIndex, att);
						}
					}
				}
				{
					vint fieldCount=table->GetTreeFieldInfoCount();
					for(vint field=0;field<fieldCount;field++)
					{
						const ParsingTable::TreeFieldInfo& fieldInfo=table->GetTreeFieldInfo(field);
						FieldDesc fieldDesc(fieldInfo.type, fieldInfo.field);

						if(Ptr<ParsingTable::AttributeInfo> att=GetColorAttribute(fieldInfo.attributeIndex))
						{
							fieldColorAtts.Add(fieldDesc, att);
						}
						if(Ptr<ParsingTable::AttributeInfo> att=GetSemanticAttribute(fieldInfo.attributeIndex))
						{
							fieldSemanticAtts.Add(fieldDesc, att);
						}
					}
				}

				for (auto att :
					From(tokenColorAtts.Values())
						.Concat(tokenContextColorAtts.Values())
						.Concat(fieldColorAtts.Values())
						.Concat(fieldSemanticAtts.Values())
					)
				{
					for (auto argument : att->arguments)
					{
						if(!semanticIndexMap.Contains(argument))
						{
							semanticIndexMap.Add(argument);
						}
					}
				}

				vint index=0;
				for (auto tokenIndex : tokenIndexMap.Values())
				{
					TokenMetaData md;
					md.tableTokenIndex=tokenIndex+ParsingTable::UserTokenStart;
					md.lexerTokenIndex=tokenIndex;
					md.defaultColorIndex=-1;
					md.hasContextColor=false;
					md.hasAutoComplete=false;
					md.isCandidate=false;

					if((index=tokenColorAtts.Keys().IndexOf(tokenIndex))!=-1)
					{
						md.defaultColorIndex=semanticIndexMap.IndexOf(tokenColorAtts.Values()[index]->arguments[0]);
					}
					md.hasContextColor=tokenContextColorAtts.Keys().Contains(tokenIndex);
					md.hasAutoComplete=tokenAutoCompleteAtts.Keys().Contains(tokenIndex);
					if ((md.isCandidate = tokenCandidateAtts.Keys().Contains(tokenIndex)))
					{
						const ParsingTable::TokenInfo& tokenInfo = table->GetTokenInfo(md.tableTokenIndex);
						auto regex = wtou32(tokenInfo.regex);
						if (IsRegexEscapedLiteralString(regex))
						{
							md.unescapedRegexText = u32tow(UnescapeTextForRegex(regex));
						}
						else
						{
							md.isCandidate = false;
						}
					}

					tokenMetaDatas.Add(tokenIndex, md);
				}
				{
					vint fieldCount=table->GetTreeFieldInfoCount();
					for(vint field=0;field<fieldCount;field++)
					{
						const ParsingTable::TreeFieldInfo& fieldInfo=table->GetTreeFieldInfo(field);
						FieldDesc fieldDesc(fieldInfo.type, fieldInfo.field);

						FieldMetaData md;
						md.colorIndex=-1;

						if((index=fieldColorAtts.Keys().IndexOf(fieldDesc))!=-1)
						{
							md.colorIndex=semanticIndexMap.IndexOf(fieldColorAtts.Values()[index]->arguments[0]);
						}
						if((index=fieldSemanticAtts.Keys().IndexOf(fieldDesc))!=-1)
						{
							md.semantics=Ptr(new List<vint>);
							for (auto argument : fieldSemanticAtts.Values()[index]->arguments)
							{
								md.semantics->Add(semanticIndexMap.IndexOf(argument));
							}
						}

						fieldMetaDatas.Add(fieldDesc, md);
					}
				}
			}

			void RepeatingParsingExecutor::OnContextFinishedAsync(RepeatingParsingOutput& context)
			{
				if(analyzer)
				{
					context.cache = analyzer->CreateCacheAsync(context);
				}
			}

			RepeatingParsingExecutor::RepeatingParsingExecutor(Ptr<parsing::tabling::ParsingGeneralParser> _grammarParser, const WString& _grammarRule, Ptr<IParsingAnalyzer> _analyzer)
				:grammarParser(_grammarParser)
				,grammarRule(_grammarRule)
				,analyzer(_analyzer)
				,autoPushingCallback(0)
			{
				PrepareMetaData();
				if (analyzer)
				{
					analyzer->Attach(this);
				}
			}

			RepeatingParsingExecutor::~RepeatingParsingExecutor()
			{
				EnsureTaskFinished();
				if (analyzer)
				{
					analyzer->Detach(this);
				}
			}

			Ptr<parsing::tabling::ParsingGeneralParser> RepeatingParsingExecutor::GetParser()
			{
				return grammarParser;
			}

			bool RepeatingParsingExecutor::AttachCallback(ICallback* value)
			{
				if(!value) return false;
				if(callbacks.Contains(value)) return false;
				callbacks.Add(value);
				return true;
			}

			bool RepeatingParsingExecutor::DetachCallback(ICallback* value)
			{
				if(!value) return false;
				if(!callbacks.Contains(value)) return false;
				DeactivateCallback(value);
				callbacks.Remove(value);
				return true;
			}

			bool RepeatingParsingExecutor::ActivateCallback(ICallback* value)
			{
				if(!value) return false;
				if(!callbacks.Contains(value)) return false;
				if(activatedCallbacks.Contains(value)) return false;
				activatedCallbacks.Add(value);

				if(!autoPushingCallback)
				{
					autoPushingCallback=value;
					autoPushingCallback->RequireAutoSubmitTask(true);
				}
				return true;
			}

			bool RepeatingParsingExecutor::DeactivateCallback(ICallback* value)
			{
				if(!value) return false;
				if(!callbacks.Contains(value)) return false;
				if(!activatedCallbacks.Contains(value)) return false;

				if(autoPushingCallback==value)
				{
					autoPushingCallback->RequireAutoSubmitTask(false);
					autoPushingCallback=0;
				}
				activatedCallbacks.Remove(value);
				if(!autoPushingCallback && activatedCallbacks.Count()>0)
				{
					autoPushingCallback=activatedCallbacks[0];
					autoPushingCallback->RequireAutoSubmitTask(true);
				}
				return true;
			}

			Ptr<RepeatingParsingExecutor::IParsingAnalyzer> RepeatingParsingExecutor::GetAnalyzer()
			{
				return analyzer;
			}

			vint RepeatingParsingExecutor::GetTokenIndex(const WString& tokenName)
			{
				vint index=tokenIndexMap.Keys().IndexOf(tokenName);
				return index==-1?-1:tokenIndexMap.Values()[index];
			}

			vint RepeatingParsingExecutor::GetSemanticId(const WString& name)
			{
				return semanticIndexMap.IndexOf(name);
			}

			WString RepeatingParsingExecutor::GetSemanticName(vint id)
			{
				return 0<=id&&id<semanticIndexMap.Count()?semanticIndexMap[id]:L"";
			}

			const RepeatingParsingExecutor::TokenMetaData& RepeatingParsingExecutor::GetTokenMetaData(vint regexTokenIndex)
			{
				return tokenMetaDatas[regexTokenIndex];
			}

			const RepeatingParsingExecutor::FieldMetaData& RepeatingParsingExecutor::GetFieldMetaData(const WString& type, const WString& field)
			{
				return fieldMetaDatas[FieldDesc(type, field)];
			}

			Ptr<parsing::tabling::ParsingTable::AttributeInfo> RepeatingParsingExecutor::GetAttribute(vint index, const WString& name, vint argumentCount)
			{
				if(index!=-1)
				{
					Ptr<ParsingTable::AttributeInfo> att=grammarParser->GetTable()->GetAttributeInfo(index)->FindFirst(name);
					if(att && (argumentCount==-1 || att->arguments.Count()==argumentCount))
					{
						return att;
					}
				}
				return 0;
			}

			Ptr<parsing::tabling::ParsingTable::AttributeInfo> RepeatingParsingExecutor::GetColorAttribute(vint index)
			{
				return GetAttribute(index, L"Color", 1);
			}

			Ptr<parsing::tabling::ParsingTable::AttributeInfo> RepeatingParsingExecutor::GetContextColorAttribute(vint index)
			{
				return GetAttribute(index, L"ContextColor", 0);
			}

			Ptr<parsing::tabling::ParsingTable::AttributeInfo> RepeatingParsingExecutor::GetSemanticAttribute(vint index)
			{
				return GetAttribute(index, L"Semantic", -1);
			}

			Ptr<parsing::tabling::ParsingTable::AttributeInfo> RepeatingParsingExecutor::GetCandidateAttribute(vint index)
			{
				return GetAttribute(index, L"Candidate", 0);
			}

			Ptr<parsing::tabling::ParsingTable::AttributeInfo> RepeatingParsingExecutor::GetAutoCompleteAttribute(vint index)
			{
				return GetAttribute(index, L"AutoComplete", 0);
			}
		}
	}
}