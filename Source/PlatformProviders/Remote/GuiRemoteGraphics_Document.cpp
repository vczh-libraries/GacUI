#include "GuiRemoteGraphics_Document.h"
#include "GuiRemoteGraphics.h"

namespace vl::presentation::elements
{
	using namespace collections;

/***********************************************************************
Comparison
***********************************************************************/

	bool AreEqual(const DocumentTextRunPropertyOverrides& a, const DocumentTextRunPropertyOverrides& b)
	{
		return a.textColor == b.textColor &&
			   a.backgroundColor == b.backgroundColor &&
			   a.fontFamily == b.fontFamily &&
			   a.size == b.size &&
			   a.textStyle == b.textStyle;
	}

	bool AreEqual(const remoteprotocol::DocumentTextRunProperty& a, const remoteprotocol::DocumentTextRunProperty& b)
	{
		return a.textColor == b.textColor &&
			   a.backgroundColor == b.backgroundColor &&
			   a.fontProperties == b.fontProperties;
	}

	bool AreEqual(const remoteprotocol::DocumentRunProperty& a, const remoteprotocol::DocumentRunProperty& b)
	{
		if (a.Index() != b.Index())
			return false;

		if (auto textA = a.TryGet<remoteprotocol::DocumentTextRunProperty>())
		{
			auto textB = b.Get<remoteprotocol::DocumentTextRunProperty>();
			return AreEqual(*textA, textB);
		}
		else
		{
			auto inlineA = a.Get<remoteprotocol::DocumentInlineObjectRunProperty>();
			auto inlineB = b.Get<remoteprotocol::DocumentInlineObjectRunProperty>();
			return inlineA.size == inlineB.size &&
				inlineA.baseline == inlineB.baseline &&
				inlineA.breakCondition == inlineB.breakCondition &&
				inlineA.backgroundElementId == inlineB.backgroundElementId &&
				inlineA.callbackId == inlineB.callbackId;
		}
	}

/***********************************************************************
DiffRuns
***********************************************************************/

	void AddTextRun(
		DocumentTextRunPropertyMap& map,
		CaretRange range,
		const DocumentTextRunPropertyOverrides& propertyOverrides)
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
			
			// Binary search may return any overlapping entry, scan backwards to find the first one
			if (firstOverlap != -1)
			{
				while (firstOverlap > 0)
				{
					auto&& prevKey = map.Keys()[firstOverlap - 1];
					if (prevKey.caretEnd <= range.caretBegin || prevKey.caretBegin >= range.caretEnd)
						break;
					firstOverlap--;
				}
			}
		}
	
		List<Pair<CaretRange, DocumentTextRunPropertyOverrides>> fragmentsToReinsert;
		List<CaretRange> keysToRemove;
		
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
			
				keysToRemove.Add(key);
			}
			
			for (auto&& key : keysToRemove)
			{
				map.Remove(key);
			}
		}
	
		for (auto&& fragment : fragmentsToReinsert)
		{
			map.Add(fragment.key, fragment.value);
		}
	
		map.Add(range, propertyOverrides);

		vint newIndex = map.Keys().IndexOf(range);
		
		while (newIndex > 0)
		{
			CaretRange leftKey = map.Keys()[newIndex - 1];
			CaretRange currentKey = map.Keys()[newIndex];
			
			if (leftKey.caretEnd == currentKey.caretBegin &&
				AreEqual(map[leftKey], map[currentKey]))
			{
				CaretRange mergedRange{ leftKey.caretBegin, currentKey.caretEnd };
				auto mergedProperty = map[leftKey];
				
				map.Remove(leftKey);
				map.Remove(currentKey);
				map.Add(mergedRange, mergedProperty);
				
				newIndex = map.Keys().IndexOf(mergedRange);
			}
			else
			{
				break;
			}
		}

		while (newIndex < map.Keys().Count() - 1)
		{
			CaretRange currentKey = map.Keys()[newIndex];
			CaretRange rightKey = map.Keys()[newIndex + 1];
			
			if (currentKey.caretEnd == rightKey.caretBegin &&
				AreEqual(map[currentKey], map[rightKey]))
			{
				CaretRange mergedRange{ currentKey.caretBegin, rightKey.caretEnd };
				auto mergedProperty = map[currentKey];
				
				map.Remove(currentKey);
				map.Remove(rightKey);
				map.Add(mergedRange, mergedProperty);
				
				newIndex = map.Keys().IndexOf(mergedRange);
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
		const remoteprotocol::DocumentInlineObjectRunProperty& property)
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
		DocumentTextRunPropertyOverrides currentTextProperty;
		bool hasCurrentText = false;
		vint lastInlineEnd = -1;

		while (textIdx < textKeys.Count() || inlineIdx < inlineKeys.Count() || hasCurrentText)
		{
			if (!hasCurrentText && textIdx < textKeys.Count())
			{
				currentTextRange = textKeys[textIdx];
				currentTextProperty = textRuns[currentTextRange];
				hasCurrentText = true;
				textIdx++;
				
				// Trim text run if it overlaps with the last processed inline object
				if (lastInlineEnd > currentTextRange.caretBegin)
				{
					if (lastInlineEnd >= currentTextRange.caretEnd)
					{
						// Text run is completely within the last inline object
						hasCurrentText = false;
						continue;
					}
					else
					{
						// Text run partially overlaps, trim the beginning
						currentTextRange.caretBegin = lastInlineEnd;
					}
				}
			}

			if (hasCurrentText && inlineIdx >= inlineKeys.Count())
			{
				// Convert DocumentTextRunPropertyOverrides to DocumentTextRunProperty
				remoteprotocol::DocumentTextRunProperty fullProp;
				fullProp.textColor = currentTextProperty.textColor.Value();
				fullProp.backgroundColor = currentTextProperty.backgroundColor.Value();
				fullProp.fontProperties.fontFamily = currentTextProperty.fontFamily.Value();
				fullProp.fontProperties.size = currentTextProperty.size.Value();
				
				// Convert TextStyle enum flags to individual bool fields
				auto style = currentTextProperty.textStyle.Value();
				fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != (IGuiGraphicsParagraph::TextStyle)0;
				fullProp.fontProperties.italic = (style & IGuiGraphicsParagraph::TextStyle::Italic) != (IGuiGraphicsParagraph::TextStyle)0;
				fullProp.fontProperties.underline = (style & IGuiGraphicsParagraph::TextStyle::Underline) != (IGuiGraphicsParagraph::TextStyle)0;
				fullProp.fontProperties.strikeline = (style & IGuiGraphicsParagraph::TextStyle::Strikeline) != (IGuiGraphicsParagraph::TextStyle)0;
				
				// Set default values for antialias properties
				fullProp.fontProperties.antialias = true;
				fullProp.fontProperties.verticalAntialias = true;
				
				remoteprotocol::DocumentRunProperty runProp = fullProp;
				result.Add(currentTextRange, runProp);
				hasCurrentText = false;
				continue;
			}

			if (!hasCurrentText && inlineIdx < inlineKeys.Count())
			{
				auto&& inlineKey = inlineKeys[inlineIdx];
				remoteprotocol::DocumentRunProperty runProp = inlineObjectRuns[inlineKey];
				result.Add(inlineKey, runProp);
				lastInlineEnd = inlineKey.caretEnd;
				inlineIdx++;
				continue;
			}

			if (hasCurrentText && inlineIdx < inlineKeys.Count())
			{
				auto&& inlineKey = inlineKeys[inlineIdx];

				if (currentTextRange.caretEnd <= inlineKey.caretBegin)
				{
					// Convert DocumentTextRunPropertyOverrides to DocumentTextRunProperty
					remoteprotocol::DocumentTextRunProperty fullProp;
					fullProp.textColor = currentTextProperty.textColor.Value();
					fullProp.backgroundColor = currentTextProperty.backgroundColor.Value();
					fullProp.fontProperties.fontFamily = currentTextProperty.fontFamily.Value();
					fullProp.fontProperties.size = currentTextProperty.size.Value();
					
					// Convert TextStyle enum flags to individual bool fields
					auto style = currentTextProperty.textStyle.Value();
					fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != (IGuiGraphicsParagraph::TextStyle)0;
					fullProp.fontProperties.italic = (style & IGuiGraphicsParagraph::TextStyle::Italic) != (IGuiGraphicsParagraph::TextStyle)0;
					fullProp.fontProperties.underline = (style & IGuiGraphicsParagraph::TextStyle::Underline) != (IGuiGraphicsParagraph::TextStyle)0;
					fullProp.fontProperties.strikeline = (style & IGuiGraphicsParagraph::TextStyle::Strikeline) != (IGuiGraphicsParagraph::TextStyle)0;
					
					// Set default values for antialias properties
					fullProp.fontProperties.antialias = true;
					fullProp.fontProperties.verticalAntialias = true;
					
					remoteprotocol::DocumentRunProperty runProp = fullProp;
					result.Add(currentTextRange, runProp);
					hasCurrentText = false;
				}
				else if (inlineKey.caretEnd <= currentTextRange.caretBegin)
				{
					remoteprotocol::DocumentRunProperty runProp = inlineObjectRuns[inlineKey];
					result.Add(inlineKey, runProp);
					lastInlineEnd = inlineKey.caretEnd;
					inlineIdx++;
				}
				else
				{
					if (currentTextRange.caretBegin < inlineKey.caretBegin)
					{
						CaretRange beforeRange{ currentTextRange.caretBegin, inlineKey.caretBegin };
						
						// Convert DocumentTextRunPropertyOverrides to DocumentTextRunProperty
						remoteprotocol::DocumentTextRunProperty fullProp;
						fullProp.textColor = currentTextProperty.textColor.Value();
						fullProp.backgroundColor = currentTextProperty.backgroundColor.Value();
						fullProp.fontProperties.fontFamily = currentTextProperty.fontFamily.Value();
						fullProp.fontProperties.size = currentTextProperty.size.Value();
						
						// Convert TextStyle enum flags to individual bool fields
						auto style = currentTextProperty.textStyle.Value();
						fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != (IGuiGraphicsParagraph::TextStyle)0;
						fullProp.fontProperties.italic = (style & IGuiGraphicsParagraph::TextStyle::Italic) != (IGuiGraphicsParagraph::TextStyle)0;
						fullProp.fontProperties.underline = (style & IGuiGraphicsParagraph::TextStyle::Underline) != (IGuiGraphicsParagraph::TextStyle)0;
						fullProp.fontProperties.strikeline = (style & IGuiGraphicsParagraph::TextStyle::Strikeline) != (IGuiGraphicsParagraph::TextStyle)0;
						
						// Set default values for antialias properties
						fullProp.fontProperties.antialias = true;
						fullProp.fontProperties.verticalAntialias = true;
						
						remoteprotocol::DocumentRunProperty runProp = fullProp;
						result.Add(beforeRange, runProp);
					}

					remoteprotocol::DocumentRunProperty runProp = inlineObjectRuns[inlineKey];
					result.Add(inlineKey, runProp);
					lastInlineEnd = inlineKey.caretEnd;
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

	void DiffRuns(
		const DocumentRunPropertyMap& oldRuns,
		const DocumentRunPropertyMap& newRuns,
		remoteprotocol::ElementDesc_DocumentParagraph& result)
	{
		result.runsDiff = Ptr(new List<remoteprotocol::DocumentRun>());
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

				remoteprotocol::DocumentRun run;
				run.caretBegin = newKey.caretBegin;
				run.caretEnd = newKey.caretEnd;
				run.props = newValue;
				result.runsDiff->Add(run);

				if (auto inlineObj = newValue.TryGet<remoteprotocol::DocumentInlineObjectRunProperty>())
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

				if (auto inlineObj = oldValue.TryGet<remoteprotocol::DocumentInlineObjectRunProperty>())
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

				if (auto inlineObj = oldValue.TryGet<remoteprotocol::DocumentInlineObjectRunProperty>())
				{
					oldInlineCallbackIds.Add(inlineObj->callbackId);
				}
				if (auto inlineObj = newValue.TryGet<remoteprotocol::DocumentInlineObjectRunProperty>())
				{
					newInlineCallbackIds.Add(inlineObj->callbackId);
				}

				if (!AreEqual(oldValue, newValue))
				{
					remoteprotocol::DocumentRun run;
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

				remoteprotocol::DocumentRun run;
				run.caretBegin = newKey.caretBegin;
				run.caretEnd = newKey.caretEnd;
				run.props = newValue;
				result.runsDiff->Add(run);

				if (auto inlineObj = newValue.TryGet<remoteprotocol::DocumentInlineObjectRunProperty>())
				{
					newInlineCallbackIds.Add(inlineObj->callbackId);
				}

				newIdx++;
			}
			else
			{
				auto&& oldValue = oldRuns[oldKey];

				if (auto inlineObj = oldValue.TryGet<remoteprotocol::DocumentInlineObjectRunProperty>())
				{
					oldInlineCallbackIds.Add(inlineObj->callbackId);
				}

				oldIdx++;
			}
		}

#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::DiffRuns(const DocumentRunPropertyMap&, const DocumentRunPropertyMap&, remoteprotocol::ElementDesc_DocumentParagraph&)#"
		// Validate that all old run ranges are fully covered by new run ranges
		for (vint oldI = 0; oldI < oldKeys.Count(); oldI++)
		{
			auto&& oldKey = oldKeys[oldI];
			vint coveredEnd = oldKey.caretBegin;

			// Find all new ranges that cover this old range
			for (vint newI = 0; newI < newKeys.Count(); newI++)
			{
				auto&& newKey = newKeys[newI];

				// Check if this new range overlaps with uncovered part of old range
				if (newKey.caretBegin <= coveredEnd && newKey.caretEnd > coveredEnd)
				{
					coveredEnd = newKey.caretEnd > oldKey.caretEnd ? oldKey.caretEnd : newKey.caretEnd;
				}
			}

			CHECK_ERROR(coveredEnd >= oldKey.caretEnd,
				ERROR_MESSAGE_PREFIX L"Old run range not fully covered by new runs");
		}
#undef ERROR_MESSAGE_PREFIX

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