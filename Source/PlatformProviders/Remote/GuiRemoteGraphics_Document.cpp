#include "GuiRemoteGraphics_Document.h"
#include "GuiRemoteGraphics.h"

namespace vl::presentation::elements
{
	using namespace collections;

/***********************************************************************
DiffRuns
***********************************************************************/

	bool operator==(const DocumentTextRunProperty& a, const DocumentTextRunProperty& b)
	{
		return a.textColor == b.textColor &&
			   a.backgroundColor == b.backgroundColor &&
			   a.fontProperties == b.fontProperties;
	}

	void AddTextRun(
		DocumentTextRunPropertyMap& map,
		CaretRange range,
		const DocumentTextRunProperty& property)
	{
		vint firstOverlap = -1;
		
		if (map.Count() > 0)
		{
			auto comparer = [&](const CaretRange& key, const CaretRange& searchRange) -> std::strong_ordering
			{
				if (key.caretEnd <= searchRange.caretBegin)
					return std::strong_ordering::less;
				else if (key.caretBegin >= searchRange.caretEnd)
					return std::strong_ordering::greater;
				else
					return std::strong_ordering::equal;
			};

			vint index = -1;
			firstOverlap = BinarySearchLambda(&map.Keys()[0], map.Keys().Count(), range, index, comparer);
		}

		List<Pair<CaretRange, DocumentTextRunProperty>> fragmentsToReinsert;
		
		if (firstOverlap != -1)
		{
			auto&& keys = map.Keys();
			for (vint i = firstOverlap; i < keys.Count(); i++)
			{
				auto&& key = keys[i];
				if (key.caretBegin >= range.caretEnd)
					break;

				auto&& oldProperty = map[key];

				if (key.caretBegin < range.caretBegin)
				{
					CaretRange beforeRange{ key.caretBegin, range.caretBegin };
					fragmentsToReinsert.Add({ beforeRange, oldProperty });
				}

				if (key.caretEnd > range.caretEnd)
				{
					CaretRange afterRange{ range.caretEnd, key.caretEnd };
					fragmentsToReinsert.Add({ afterRange, oldProperty });
				}

				map.Remove(key);
			}
		}

		for (auto&& fragment : fragmentsToReinsert)
		{
			map.Add(fragment.key, fragment.value);
		}

		map.Add(range, property);

		vint newIndex = map.Keys().IndexOf(range);
		
		while (newIndex > 0)
		{
			auto&& leftKey = map.Keys()[newIndex - 1];
			auto&& currentKey = map.Keys()[newIndex];
			
			if (leftKey.caretEnd == currentKey.caretBegin &&
				map[leftKey] == map[currentKey])
			{
				CaretRange mergedRange{ leftKey.caretBegin, currentKey.caretEnd };
				auto mergedProperty = map[leftKey];
				
				map.Remove(leftKey);
				map.Remove(currentKey);
				map.Add(mergedRange, mergedProperty);
				
				newIndex--;
			}
			else
			{
				break;
			}
		}

		while (newIndex < map.Keys().Count() - 1)
		{
			auto&& currentKey = map.Keys()[newIndex];
			auto&& rightKey = map.Keys()[newIndex + 1];
			
			if (currentKey.caretEnd == rightKey.caretBegin &&
				map[currentKey] == map[rightKey])
			{
				CaretRange mergedRange{ currentKey.caretBegin, rightKey.caretEnd };
				auto mergedProperty = map[currentKey];
				
				map.Remove(currentKey);
				map.Remove(rightKey);
				map.Add(mergedRange, mergedProperty);
			}
			else
			{
				break;
			}
		}
	}

	bool AddInlineObjectRun(
		DocumentInlineObjectRunPropertyMap& map,
		CaretRange range,
		const DocumentInlineObjectRunProperty& property)
	{
		if (map.Count() > 0)
		{
			auto comparer = [&](const CaretRange& key, const CaretRange& searchRange) -> std::strong_ordering
			{
				if (key.caretEnd <= searchRange.caretBegin)
					return std::strong_ordering::less;
				else if (key.caretBegin >= searchRange.caretEnd)
					return std::strong_ordering::greater;
				else
					return std::strong_ordering::equal;
			};

			vint index = -1;
			vint firstOverlap = BinarySearchLambda(&map.Keys()[0], map.Keys().Count(), range, index, comparer);
			
			if (firstOverlap != -1)
				return false;
		}

		map.Add(range, property);
		return true;
	}

	bool ResetInlineObjectRun(
		DocumentInlineObjectRunPropertyMap& map,
		CaretRange range)
	{
		return map.Remove(range);
	}

	void MergeRuns(
		const DocumentTextRunPropertyMap& textRuns,
		const DocumentInlineObjectRunPropertyMap& inlineObjectRuns,
		DocumentRunPropertyMap& result)
	{
		result.Clear();

		vint textIdx = 0;
		vint inlineIdx = 0;

		auto&& textKeys = textRuns.Keys();
		auto&& inlineKeys = inlineObjectRuns.Keys();

		CaretRange currentTextRange;
		DocumentTextRunProperty currentTextProperty;
		bool hasCurrentText = false;

		while (textIdx < textKeys.Count() || inlineIdx < inlineKeys.Count() || hasCurrentText)
		{
			if (!hasCurrentText && textIdx < textKeys.Count())
			{
				currentTextRange = textKeys[textIdx];
				currentTextProperty = textRuns[currentTextRange];
				hasCurrentText = true;
				textIdx++;
			}

			if (!hasCurrentText && inlineIdx < inlineKeys.Count())
			{
				auto&& inlineKey = inlineKeys[inlineIdx];
				DocumentRunProperty runProp = inlineObjectRuns[inlineKey];
				result.Add(inlineKey, runProp);
				inlineIdx++;
				continue;
			}

			if (hasCurrentText && inlineIdx >= inlineKeys.Count())
			{
				DocumentRunProperty runProp = currentTextProperty;
				result.Add(currentTextRange, runProp);
				hasCurrentText = false;
				continue;
			}

			if (hasCurrentText && inlineIdx < inlineKeys.Count())
			{
				auto&& inlineKey = inlineKeys[inlineIdx];

				if (currentTextRange.caretEnd <= inlineKey.caretBegin)
				{
					DocumentRunProperty runProp = currentTextProperty;
					result.Add(currentTextRange, runProp);
					hasCurrentText = false;
				}
				else if (inlineKey.caretEnd <= currentTextRange.caretBegin)
				{
					DocumentRunProperty runProp = inlineObjectRuns[inlineKey];
					result.Add(inlineKey, runProp);
					inlineIdx++;
				}
				else
				{
					if (currentTextRange.caretBegin < inlineKey.caretBegin)
					{
						CaretRange beforeRange{ currentTextRange.caretBegin, inlineKey.caretBegin };
						DocumentRunProperty runProp = currentTextProperty;
						result.Add(beforeRange, runProp);
					}

					DocumentRunProperty runProp = inlineObjectRuns[inlineKey];
					result.Add(inlineKey, runProp);
					inlineIdx++;

					if (currentTextRange.caretEnd > inlineKey.caretEnd)
					{
						currentTextRange.caretBegin = inlineKey.caretEnd;
					}
					else
					{
						hasCurrentText = false;
					}
				}
			}
		}
	}

	bool operator==(const DocumentRunProperty& a, const DocumentRunProperty& b)
	{
		if (a.Index() != b.Index())
			return false;

		if (auto textA = a.TryGet<DocumentTextRunProperty>())
		{
			auto textB = b.Get<DocumentTextRunProperty>();
			return *textA == textB;
		}
		else
		{
			auto inlineA = a.Get<DocumentInlineObjectRunProperty>();
			auto inlineB = b.Get<DocumentInlineObjectRunProperty>();
			return inlineA.size == inlineB.size &&
				   inlineA.baseline == inlineB.baseline &&
				   inlineA.breakCondition == inlineB.breakCondition &&
				   inlineA.backgroundElementId == inlineB.backgroundElementId &&
				   inlineA.callbackId == inlineB.callbackId;
		}
	}

	void DiffRuns(
		const DocumentRunPropertyMap& oldRuns,
		const DocumentRunPropertyMap& newRuns,
		ElementDesc_DocumentParagraph& result)
	{
		result.runsDiff = Ptr(new List<DocumentRun>());
		result.createdInlineObjects = Ptr(new List<vint>());
		result.removedInlineObjects = Ptr(new List<vint>());

		SortedList<vint> oldInlineCallbackIds;
		SortedList<vint> newInlineCallbackIds;

		vint oldIdx = 0;
		vint newIdx = 0;

		auto&& oldKeys = oldRuns.Keys();
		auto&& newKeys = newRuns.Keys();

		while (oldIdx < oldKeys.Count() || newIdx < newKeys.Count())
		{
			if (oldIdx >= oldKeys.Count())
			{
				auto&& newKey = newKeys[newIdx];
				auto&& newValue = newRuns[newKey];

				DocumentRun run;
				run.caretBegin = newKey.caretBegin;
				run.caretEnd = newKey.caretEnd;
				run.props = newValue;
				result.runsDiff->Add(run);

				if (auto inlineObj = newValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					newInlineCallbackIds.Add(inlineObj->callbackId);
				}

				newIdx++;
				continue;
			}

			if (newIdx >= newKeys.Count())
			{
				auto&& oldKey = oldKeys[oldIdx];
				auto&& oldValue = oldRuns[oldKey];

				if (auto inlineObj = oldValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					oldInlineCallbackIds.Add(inlineObj->callbackId);
				}

				oldIdx++;
				continue;
			}

			auto&& oldKey = oldKeys[oldIdx];
			auto&& newKey = newKeys[newIdx];

			if (oldKey == newKey)
			{
				auto&& oldValue = oldRuns[oldKey];
				auto&& newValue = newRuns[newKey];

				if (auto inlineObj = oldValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					oldInlineCallbackIds.Add(inlineObj->callbackId);
				}
				if (auto inlineObj = newValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					newInlineCallbackIds.Add(inlineObj->callbackId);
				}

				if (!(oldValue == newValue))
				{
					DocumentRun run;
					run.caretBegin = newKey.caretBegin;
					run.caretEnd = newKey.caretEnd;
					run.props = newValue;
					result.runsDiff->Add(run);
				}

				oldIdx++;
				newIdx++;
			}
			else if (newKey < oldKey)
			{
				auto&& newValue = newRuns[newKey];

				DocumentRun run;
				run.caretBegin = newKey.caretBegin;
				run.caretEnd = newKey.caretEnd;
				run.props = newValue;
				result.runsDiff->Add(run);

				if (auto inlineObj = newValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					newInlineCallbackIds.Add(inlineObj->callbackId);
				}

				newIdx++;
			}
			else
			{
				auto&& oldValue = oldRuns[oldKey];

				if (auto inlineObj = oldValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					oldInlineCallbackIds.Add(inlineObj->callbackId);
				}

				oldIdx++;
			}
		}

		for (vint i = 0; i < newInlineCallbackIds.Count(); i++)
		{
			auto id = newInlineCallbackIds[i];
			if (!oldInlineCallbackIds.Contains(id))
			{
				result.createdInlineObjects->Add(id);
			}
		}

		for (vint i = 0; i < oldInlineCallbackIds.Count(); i++)
		{
			auto id = oldInlineCallbackIds[i];
			if (!newInlineCallbackIds.Contains(id))
			{
				result.removedInlineObjects->Add(id);
			}
		}
	}

/***********************************************************************
GuiRemoteGraphicsParagraph
***********************************************************************/

	GuiRemoteGraphicsParagraph::GuiRemoteGraphicsParagraph(const WString& _text, GuiRemoteController* _remote, GuiRemoteGraphicsResourceManager* _resourceManager, GuiRemoteGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _callback)
		: text(_text)
		, remote(_remote)
		, resourceManager(_resourceManager)
		, renderTarget(_renderTarget)
		, callback(_callback)
	{
	}

	GuiRemoteGraphicsParagraph::~GuiRemoteGraphicsParagraph()
	{
	}

	IGuiGraphicsLayoutProvider* GuiRemoteGraphicsParagraph::GetProvider()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	IGuiGraphicsRenderTarget* GuiRemoteGraphicsParagraph::GetRenderTarget()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::GetWrapLine()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteGraphicsParagraph::SetWrapLine(bool value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	vint GuiRemoteGraphicsParagraph::GetMaxWidth()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteGraphicsParagraph::SetMaxWidth(vint value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Alignment GuiRemoteGraphicsParagraph::GetParagraphAlignment()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteGraphicsParagraph::SetParagraphAlignment(Alignment value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetFont(vint start, vint length, const WString& value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetSize(vint start, vint length, vint value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetStyle(vint start, vint length, TextStyle value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetColor(vint start, vint length, Color value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetBackgroundColor(vint start, vint length, Color value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetInlineObject(vint start, vint length, const InlineObjectProperties& properties)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::ResetInlineObject(vint start, vint length)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Size GuiRemoteGraphicsParagraph::GetSize()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::OpenCaret(vint caret, Color color, bool frontSide)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::CloseCaret()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteGraphicsParagraph::Render(Rect bounds)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	vint GuiRemoteGraphicsParagraph::GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Rect GuiRemoteGraphicsParagraph::GetCaretBounds(vint caret, bool frontSide)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	vint GuiRemoteGraphicsParagraph::GetCaretFromPoint(Point point)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Nullable<IGuiGraphicsParagraph::InlineObjectProperties> GuiRemoteGraphicsParagraph::GetInlineObjectFromPoint(Point point, vint& start, vint& length)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	vint GuiRemoteGraphicsParagraph::GetNearestCaretFromTextPos(vint textPos, bool frontSide)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::IsValidCaret(vint caret)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::IsValidTextPos(vint textPos)
	{
		CHECK_FAIL(L"Not Implemented!");
	}
}