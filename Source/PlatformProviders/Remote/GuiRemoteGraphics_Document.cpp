#include "GuiRemoteGraphics_Document.h"
#include "GuiRemoteGraphics.h"
#include "GuiRemoteGraphics_BasicElements.h"
#include "GuiRemoteEvents.h"

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

	DocumentTextRunPropertyOverrides ApplyOverrides(
		const DocumentTextRunPropertyOverrides& base,
		const DocumentTextRunPropertyOverrides& overrides)
	{
		DocumentTextRunPropertyOverrides result;
		result.textColor = overrides.textColor ? overrides.textColor : base.textColor;
		result.backgroundColor = overrides.backgroundColor ? overrides.backgroundColor : base.backgroundColor;
		result.fontFamily = overrides.fontFamily ? overrides.fontFamily : base.fontFamily;
		result.size = overrides.size ? overrides.size : base.size;
		result.textStyle = overrides.textStyle ? overrides.textStyle : base.textStyle;
		return result;
	}

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
		List<Tuple<CaretRange, DocumentTextRunPropertyOverrides>> overlappingOldRuns;
		
		if (firstOverlap != -1)
		{
			auto&& keys = map.Keys();
			for (vint i = firstOverlap; i < keys.Count(); i++)
			{
				auto&& key = keys[i];
				if (key.caretBegin >= range.caretEnd)
					break;
			
				auto&& oldProperty = map[key];
			
				// Record the overlapping portion
				vint overlapBegin = key.caretBegin < range.caretBegin ? range.caretBegin : key.caretBegin;
				vint overlapEnd = key.caretEnd > range.caretEnd ? range.caretEnd : key.caretEnd;
				overlappingOldRuns.Add({CaretRange{overlapBegin, overlapEnd}, oldProperty});
			
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
	
		if (overlappingOldRuns.Count() > 0)
		{
			// The new range has overlaps with old runs - apply override semantics
			vint currentPos = range.caretBegin;
			
			for (vint i = 0; i < overlappingOldRuns.Count(); i++)
			{
				auto&& [oldRange, oldProp] = overlappingOldRuns[i];
				
				// Add any gap before this overlap with new properties as-is
				if (currentPos < oldRange.caretBegin)
				{
					map.Add(CaretRange{currentPos, oldRange.caretBegin}, propertyOverrides);
				}
				
				// Add the overlapping portion with merged properties
				auto mergedProp = ApplyOverrides(oldProp, propertyOverrides);
				map.Add(oldRange, mergedProp);
				
				currentPos = oldRange.caretEnd;
			}
			
			// Add any remaining part after all overlaps with new properties as-is
			if (currentPos < range.caretEnd)
			{
				map.Add(CaretRange{currentPos, range.caretEnd}, propertyOverrides);
			}
		}
		else
		{
			// No overlaps - add the entire range with new properties
			map.Add(range, propertyOverrides);
		}

		// Find the first key within the original range for merging
		vint newIndex = -1;
		auto&& keys = map.Keys();
		for (vint i = 0; i < keys.Count(); i++)
		{
			if (keys[i].caretBegin >= range.caretBegin)
			{
				newIndex = i;
				break;
			}
		}
		
		if (newIndex == -1)
			return;
		
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

	remoteprotocol::DocumentTextRunProperty ConvertToFullProperty(const DocumentTextRunPropertyOverrides& propertyOverrides)
	{
		remoteprotocol::DocumentTextRunProperty fullProp;
		
		// Required properties - validate non-null
		CHECK_ERROR(propertyOverrides.fontFamily, L"fontFamily must be defined");
		CHECK_ERROR(propertyOverrides.size, L"size must be defined");
		
		fullProp.fontProperties.fontFamily = propertyOverrides.fontFamily.Value();
		fullProp.fontProperties.size = propertyOverrides.size.Value();
		
		// Optional properties - use defaults if null
		fullProp.textColor = propertyOverrides.textColor ? 
			propertyOverrides.textColor.Value() : Color(0, 0, 0);
		fullProp.backgroundColor = propertyOverrides.backgroundColor ? 
			propertyOverrides.backgroundColor.Value() : Color(0, 0, 0);
		
		auto style = propertyOverrides.textStyle ? 
			propertyOverrides.textStyle.Value() : (IGuiGraphicsParagraph::TextStyle)0;
		
		// Convert TextStyle enum flags to individual bool fields
		fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != (IGuiGraphicsParagraph::TextStyle)0;
		fullProp.fontProperties.italic = (style & IGuiGraphicsParagraph::TextStyle::Italic) != (IGuiGraphicsParagraph::TextStyle)0;
		fullProp.fontProperties.underline = (style & IGuiGraphicsParagraph::TextStyle::Underline) != (IGuiGraphicsParagraph::TextStyle)0;
		fullProp.fontProperties.strikeline = (style & IGuiGraphicsParagraph::TextStyle::Strikeline) != (IGuiGraphicsParagraph::TextStyle)0;
		
		// Set default values for antialias properties
		fullProp.fontProperties.antialias = true;
		fullProp.fontProperties.verticalAntialias = true;
		
		return fullProp;
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
				remoteprotocol::DocumentRunProperty runProp = ConvertToFullProperty(currentTextProperty);
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
					remoteprotocol::DocumentRunProperty runProp = ConvertToFullProperty(currentTextProperty);
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
						
						remoteprotocol::DocumentRunProperty runProp = ConvertToFullProperty(currentTextProperty);
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
		if (renderTarget)
		{
			id = renderTarget->AllocateNewElementId();
			renderTarget->RegisterParagraph(this);
		}
	}

	GuiRemoteGraphicsParagraph::~GuiRemoteGraphicsParagraph()
	{
		if (renderTarget && id != -1)
		{
			renderTarget->UnregisterParagraph(id);
			id = -1;
		}
	}

	vint GuiRemoteGraphicsParagraph::GetParagraphId() const
	{
		return id;
	}

	bool GuiRemoteGraphicsParagraph::EnsureRemoteParagraphSynced()
	{
		if (!needUpdate)
		{
			return id != -1;
		}

		if (id == -1 || !renderTarget)
		{
			return false;
		}

		renderTarget->EnsureRequestedRenderersCreated();

		stagedRuns.Clear();
		MergeRuns(textRuns, inlineObjectRuns, stagedRuns);

		remoteprotocol::ElementDesc_DocumentParagraph desc;
		if (committedRuns.Count() == 0)
		{
			desc.text = text;
		}

		switch (paragraphAlignment)
		{
		case Alignment::Right:
			desc.alignment = remoteprotocol::ElementHorizontalAlignment::Right;
			break;
		case Alignment::Center:
			desc.alignment = remoteprotocol::ElementHorizontalAlignment::Center;
			break;
		default:
			desc.alignment = remoteprotocol::ElementHorizontalAlignment::Left;
			break;
		}

		desc.wrapLine = wrapLine;
		desc.maxWidth = maxWidth;
		desc.id = id;
		DiffRuns(committedRuns, stagedRuns, desc);

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestRendererUpdateElement_DocumentParagraph(desc);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			return false;
		}

		cachedSize = messages.RetrieveRendererUpdateElement_DocumentParagraph(requestId);
		committedRuns = std::move(stagedRuns);
		needUpdate = false;
		return true;
	}

	bool GuiRemoteGraphicsParagraph::TryBuildCaretRange(vint start, vint length, CaretRange& range)
	{
		if (length <= 0 || start < 0 || start + length > text.Length())
		{
			return false;
		}
		range.caretBegin = NativeTextPosToRemoteTextPos(start);
		range.caretEnd = NativeTextPosToRemoteTextPos(start + length);
		return true;
	}

	void GuiRemoteGraphicsParagraph::MarkParagraphDirty(bool invalidateSize)
	{
		needUpdate = true;
		if (invalidateSize)
		{
			cachedSize = Size(0, 0);
		}
	}

	vint GuiRemoteGraphicsParagraph::NativeTextPosToRemoteTextPos(vint textPos)
	{
		return textPos;
	}

	vint GuiRemoteGraphicsParagraph::RemoteTextPosToNativeTextPos(vint textPos)
	{
		return textPos;
	}

	IGuiGraphicsLayoutProvider* GuiRemoteGraphicsParagraph::GetProvider()
	{
		return resourceManager;
	}

	IGuiGraphicsRenderTarget* GuiRemoteGraphicsParagraph::GetRenderTarget()
	{
		return renderTarget;
	}

	bool GuiRemoteGraphicsParagraph::GetWrapLine()
	{
		return wrapLine;
	}

	void GuiRemoteGraphicsParagraph::SetWrapLine(bool value)
	{
		if (wrapLine != value)
		{
			wrapLine = value;
			MarkParagraphDirty(true);
		}
	}

	vint GuiRemoteGraphicsParagraph::GetMaxWidth()
	{
		return maxWidth;
	}

	void GuiRemoteGraphicsParagraph::SetMaxWidth(vint value)
	{
		if (maxWidth != value)
		{
			maxWidth = value;
			MarkParagraphDirty(true);
		}
	}

	Alignment GuiRemoteGraphicsParagraph::GetParagraphAlignment()
	{
		return paragraphAlignment;
	}

	void GuiRemoteGraphicsParagraph::SetParagraphAlignment(Alignment value)
	{
		if (paragraphAlignment != value)
		{
			paragraphAlignment = value;
			MarkParagraphDirty(true);
		}
	}

	bool GuiRemoteGraphicsParagraph::SetFont(vint start, vint length, const WString& value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.fontFamily = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetSize(vint start, vint length, vint value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.size = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetStyle(vint start, vint length, TextStyle value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.textStyle = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetColor(vint start, vint length, Color value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.textColor = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetBackgroundColor(vint start, vint length, Color value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.backgroundColor = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetInlineObject(vint start, vint length, const InlineObjectProperties& properties)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		vint backgroundElementId = -1;
		IGuiGraphicsRenderer* renderer = properties.backgroundImage ? properties.backgroundImage->GetRenderer() : nullptr;
		if (renderer)
		{
			renderer->SetRenderTarget(renderTarget);
			if (auto remoteRenderer = dynamic_cast<elements_remoteprotocol::IGuiRemoteProtocolElementRender*>(renderer))
			{
				backgroundElementId = remoteRenderer->GetID();
			}
		}

		remoteprotocol::DocumentInlineObjectRunProperty remoteProp;
		remoteProp.size = properties.size;
		remoteProp.baseline = properties.baseline;
		remoteProp.breakCondition = properties.breakCondition;
		remoteProp.backgroundElementId = backgroundElementId;
		remoteProp.callbackId = properties.callbackId;

		if (!AddInlineObjectRun(inlineObjectRuns, range, remoteProp))
		{
			return false;
		}

		inlineObjectProperties.Remove(range);
		inlineObjectProperties.Add(range, properties);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::ResetInlineObject(vint start, vint length)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;
		if (!ResetInlineObjectRun(inlineObjectRuns, range)) return false;

		vint index = inlineObjectProperties.Keys().IndexOf(range);
		if (index != -1)
		{
			auto stored = inlineObjectProperties.Values()[index];
			if (stored.backgroundImage)
			{
				if (auto renderer = stored.backgroundImage->GetRenderer())
				{
					renderer->SetRenderTarget(nullptr);
				}
			}
			inlineObjectProperties.Remove(range);
		}

		MarkParagraphDirty(true);
		return true;
	}

	Size GuiRemoteGraphicsParagraph::GetSize()
	{
		EnsureRemoteParagraphSynced();
		return cachedSize;
	}

	bool GuiRemoteGraphicsParagraph::OpenCaret(vint caret, Color color, bool frontSide)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return false;
		}

		remoteprotocol::OpenCaretRequest request;
		request.id = id;
		request.caret = NativeTextPosToRemoteTextPos(caret);
		request.caretColor = color;
		request.frontSide = frontSide;

		auto& messages = renderTarget->GetRemoteMessages();
		messages.RequestDocumentParagraph_OpenCaret(request);
		bool disconnected = false;
		messages.Submit(disconnected);
		return !disconnected;
	}

	bool GuiRemoteGraphicsParagraph::CloseCaret()
	{
		if (id == -1 || !renderTarget)
		{
			return false;
		}

		auto& messages = renderTarget->GetRemoteMessages();
		messages.RequestDocumentParagraph_CloseCaret(id);
		bool disconnected = false;
		messages.Submit(disconnected);
		return !disconnected;
	}

	void GuiRemoteGraphicsParagraph::Render(Rect bounds)
	{
		if (!renderTarget)
		{
			return;
		}

		if (!EnsureRemoteParagraphSynced())
		{
			return;
		}

		remoteprotocol::ElementRendering rendering;
		rendering.id = id;
		rendering.bounds = bounds;
		rendering.areaClippedByParent = renderTarget->GetClipperValidArea();

		auto& messages = renderTarget->GetRemoteMessages();
		messages.RequestRendererRenderElement(rendering);
		lastRenderedBatchId = renderTarget->renderingBatchId;
	}

	vint GuiRemoteGraphicsParagraph::GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			preferFrontSide = false;
			return comparingCaret;
		}

		auto& messages = renderTarget->GetRemoteMessages();
		remoteprotocol::GetCaretRequest request;
		request.id = id;
		request.caret = NativeTextPosToRemoteTextPos(comparingCaret);
		request.relativePosition = position;

		vint requestId = messages.RequestDocumentParagraph_GetCaret(request);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			preferFrontSide = false;
			return comparingCaret;
		}

		auto response = messages.RetrieveDocumentParagraph_GetCaret(requestId);
		preferFrontSide = response.preferFrontSide;
		return RemoteTextPosToNativeTextPos(response.newCaret);
	}

	Rect GuiRemoteGraphicsParagraph::GetCaretBounds(vint caret, bool frontSide)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return {};
		}

		remoteprotocol::GetCaretBoundsRequest request;
		request.id = id;
		request.caret = NativeTextPosToRemoteTextPos(caret);
		request.frontSide = frontSide;

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_GetCaretBounds(request);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			return {};
		}

		return messages.RetrieveDocumentParagraph_GetCaretBounds(requestId);
	}

	vint GuiRemoteGraphicsParagraph::GetCaretFromPoint(Point point)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return 0;
		}

		auto& messages = renderTarget->GetRemoteMessages();
		vint bestCaret = 0;
		vint bestDistance = -1;

		for (vint caret = 0; caret <= text.Length(); caret++)
		{
			remoteprotocol::GetCaretBoundsRequest request;
			request.id = id;
			request.caret = NativeTextPosToRemoteTextPos(caret);
			request.frontSide = true;

			bool disconnected = false;
			vint requestId = messages.RequestDocumentParagraph_GetCaretBounds(request);
			messages.Submit(disconnected);
			if (disconnected)
			{
				return bestCaret;
			}

			Rect bounds = messages.RetrieveDocumentParagraph_GetCaretBounds(requestId);
			if (bounds.x1 <= point.x && point.x < bounds.x2 && bounds.y1 <= point.y && point.y < bounds.y2)
			{
				return caret;
			}

			vint horizontalDistance = 0;
			if (point.x < bounds.x1) horizontalDistance = bounds.x1 - point.x;
			else if (point.x > bounds.x2) horizontalDistance = point.x - bounds.x2;

			vint verticalDistance = 0;
			if (point.y < bounds.y1) verticalDistance = bounds.y1 - point.y;
			else if (point.y > bounds.y2) verticalDistance = point.y - bounds.y2;

			vint distance = horizontalDistance + verticalDistance;
			if (bestDistance == -1 || distance < bestDistance)
			{
				bestDistance = distance;
				bestCaret = caret;
			}
		}

		return bestCaret;
	}

	Nullable<IGuiGraphicsParagraph::InlineObjectProperties> GuiRemoteGraphicsParagraph::GetInlineObjectFromPoint(Point point, vint& start, vint& length)
	{
		start = 0;
		length = 0;

		if (!EnsureRemoteParagraphSynced())
		{
			return {};
		}

		remoteprotocol::GetInlineObjectFromPointRequest request;
		request.id = id;
		request.point = point;

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_GetInlineObjectFromPoint(request);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			return {};
		}

		auto response = messages.RetrieveDocumentParagraph_GetInlineObjectFromPoint(requestId);
		if (!response)
		{
			return {};
		}

		CaretRange range;
		range.caretBegin = response.Value().caretBegin;
		range.caretEnd = response.Value().caretEnd;

		vint index = inlineObjectProperties.Keys().IndexOf(range);
		if (index == -1)
		{
			return {};
		}

		start = RemoteTextPosToNativeTextPos(range.caretBegin);
		length = RemoteTextPosToNativeTextPos(range.caretEnd) - start;
		return inlineObjectProperties.Values()[index];
	}

	vint GuiRemoteGraphicsParagraph::GetNearestCaretFromTextPos(vint textPos, bool frontSide)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return textPos;
		}

		remoteprotocol::GetCaretBoundsRequest request;
		request.id = id;
		request.caret = NativeTextPosToRemoteTextPos(textPos);
		request.frontSide = frontSide;

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_GetNearestCaretFromTextPos(request);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			return textPos;
		}

		auto response = messages.RetrieveDocumentParagraph_GetNearestCaretFromTextPos(requestId);
		return RemoteTextPosToNativeTextPos(response);
	}

	bool GuiRemoteGraphicsParagraph::IsValidCaret(vint caret)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return false;
		}

		remoteprotocol::IsValidCaretRequest request;
		request.id = id;
		request.caret = NativeTextPosToRemoteTextPos(caret);

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_IsValidCaret(request);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			return false;
		}

		return messages.RetrieveDocumentParagraph_IsValidCaret(requestId);
	}

	bool GuiRemoteGraphicsParagraph::IsValidTextPos(vint textPos)
	{
		return 0 <= textPos && textPos <= text.Length();
	}
}
