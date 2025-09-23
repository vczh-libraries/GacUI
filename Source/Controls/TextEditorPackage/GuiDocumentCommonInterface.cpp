#include "GuiDocumentCommonInterface.h"
#include "../../Application/GraphicsHost/GuiGraphicsHost.h"
#include "../../Resources/GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;

/***********************************************************************
GuiDocumentItem
***********************************************************************/

			GuiDocumentItem::GuiDocumentItem(const WString& _name)
				:name(_name)
			{
				container = new GuiBoundsComposition;
				container->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				container->SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::Arrow));
			}

			GuiDocumentItem::~GuiDocumentItem()
			{
				if (!owned)
				{
					SafeDeleteComposition(container);
				}
			}

			compositions::GuiGraphicsComposition* GuiDocumentItem::GetContainer()
			{
				return container;
			}

			WString GuiDocumentItem::GetName()
			{
				return name;
			}

/***********************************************************************
GuiDocumentCommonInterface
***********************************************************************/
			
			void GuiDocumentCommonInterface::InvokeUndoRedoChanged()
			{
				UndoRedoChanged.Execute(documentControl->GetNotifyEventArguments());
			}

			void GuiDocumentCommonInterface::InvokeModifiedChanged()
			{
				ModifiedChanged.Execute(documentControl->GetNotifyEventArguments());
			}

			void GuiDocumentCommonInterface::UpdateCaretPoint()
			{
				GuiGraphicsHost* host=documentComposition->GetRelatedGraphicsHost();
				if(host)
				{
					Rect caret=documentElement->GetCaretBounds(documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide());
					Point view=GetDocumentViewPosition();
					vint x=caret.x1-view.x;
					vint y=caret.y2-view.y;
					host->SetCaretPoint(Point(x, y), documentComposition);
				}
			}

			void GuiDocumentCommonInterface::EnsureDocumentRectVisible(Rect bounds)
			{
				if (bounds != Rect())
				{
					bounds.x1 -= 15;
					bounds.y1 -= 15;
					bounds.x2 += 15;
					bounds.y2 += 15;
					EnsureRectVisible(bounds);
				}
			}

			void GuiDocumentCommonInterface::Move(TextPos caret, bool shift, bool frontSide)
			{
				TextPos begin=documentElement->GetCaretBegin();
				TextPos end=documentElement->GetCaretEnd();
				
				TextPos newBegin=shift?begin:caret;
				TextPos newEnd=caret;
				documentElement->SetCaret(newBegin, newEnd, frontSide);
				documentElement->SetCaretVisible(true);
				EnsureDocumentRectVisible(documentElement->GetCaretBounds(newEnd, frontSide));
				UpdateCaretPoint();
				SelectionChanged.Execute(documentControl->GetNotifyEventArguments());
			}

			bool GuiDocumentCommonInterface::ProcessKey(VKEY code, bool shift, bool ctrl)
			{
				if (IGuiShortcutKeyItem* item = internalShortcutKeyManager->TryGetShortcut(ctrl, shift, false, code))
				{
					GuiEventArgs arguments(documentControl->GetBoundsComposition());
					item->Executed.Execute(arguments);
					return true;
				}

				TextPos currentCaret = documentElement->GetCaretEnd();
				bool frontSide = documentElement->IsCaretEndPreferFrontSide();
				TextPos begin = documentElement->GetCaretBegin();
				TextPos end = documentElement->GetCaretEnd();

				switch (code)
				{
				case VKEY::KEY_UP:
					{
						TextPos newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretMoveUp, frontSide);
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY::KEY_DOWN:
					{
						TextPos newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretMoveDown, frontSide);
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY::KEY_LEFT:
					{
						TextPos newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretMoveLeft, frontSide);
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY::KEY_RIGHT:
					{
						TextPos newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretMoveRight, frontSide);
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY::KEY_HOME:
					{
						TextPos newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLineFirst, frontSide);
						if (newCaret == currentCaret)
						{
							newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretFirst, frontSide);
						}
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY::KEY_END:
					{
						TextPos newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLineLast, frontSide);
						if (newCaret == currentCaret)
						{
							newCaret = documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLast, frontSide);
						}
						Move(newCaret, shift, frontSide);
					}
				break;
					case VKEY::KEY_PRIOR:
					{
					}
					break;
				case VKEY::KEY_NEXT:
					{
					}
					break;
				case VKEY::KEY_BACK:
					if (editMode == GuiDocumentEditMode::Editable)
					{
						if (begin == end)
						{
							ProcessKey(VKEY::KEY_LEFT, true, false);
						}
						Array<WString> text;
						EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
						return true;
					}
					break;
				case VKEY::KEY_DELETE:
					if (editMode == GuiDocumentEditMode::Editable)
					{
						if (begin == end)
						{
							ProcessKey(VKEY::KEY_RIGHT, true, false);
						}
						Array<WString> text;
						EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
						return true;
					}
					break;
				case VKEY::KEY_RETURN:
					if (editMode == GuiDocumentEditMode::Editable)
					{
						if (ctrl)
						{
							Array<WString> text(1);
							text[0] = L"\r\n";
							EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
						}
						else
						{
							Array<WString> text(2);
							EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
						}
						return true;
					}
					break;
				default:;
				}
				return false;
			}

			void GuiDocumentCommonInterface::InstallDocumentViewer(
				GuiControl* _sender,
				compositions::GuiGraphicsComposition* _mouseArea,
				compositions::GuiGraphicsComposition* _container,
				compositions::GuiGraphicsComposition* eventComposition,
				compositions::GuiGraphicsComposition* focusableComposition
			)
			{
				documentControl = _sender;

				documentElement = GuiDocumentElement::Create();
				documentElement->SetCallback(this);
				documentElement->SetParagraphPadding(config.paragraphPadding);
				documentElement->SetWrapLine(config.wrapLine);
				documentElement->SetParagraphRecycle(config.paragraphRecycle);

				documentComposition = new GuiBoundsComposition;
				documentComposition->SetOwnedElement(Ptr(documentElement));
				documentComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
				documentComposition->SetAlignmentToParent(Margin(2, 2, 2, 2));
				_container->AddChild(documentComposition);
				ReplaceMouseArea(_mouseArea);

				focusableComposition->GetEventReceiver()->caretNotify.AttachMethod(this, &GuiDocumentCommonInterface::OnCaretNotify);
				focusableComposition->GetEventReceiver()->gotFocus.AttachMethod(this, &GuiDocumentCommonInterface::OnGotFocus);
				focusableComposition->GetEventReceiver()->lostFocus.AttachMethod(this, &GuiDocumentCommonInterface::OnLostFocus);
				focusableComposition->GetEventReceiver()->keyDown.AttachMethod(this, &GuiDocumentCommonInterface::OnKeyDown);
				focusableComposition->GetEventReceiver()->charInput.AttachMethod(this, &GuiDocumentCommonInterface::OnCharInput);

				undoRedoProcessor->Setup(documentElement, documentComposition);
				ActiveHyperlinkChanged.SetAssociatedComposition(eventComposition);
				ActiveHyperlinkExecuted.SetAssociatedComposition(eventComposition);
				SelectionChanged.SetAssociatedComposition(eventComposition);
				UndoRedoChanged.SetAssociatedComposition(eventComposition);
				ModifiedChanged.SetAssociatedComposition(eventComposition);

				undoRedoProcessor->UndoRedoChanged.Add(this, &GuiDocumentCommonInterface::InvokeUndoRedoChanged);
				undoRedoProcessor->ModifiedChanged.Add(this, &GuiDocumentCommonInterface::InvokeModifiedChanged);
				SetDocument(Ptr(new DocumentModel));
			}

			void GuiDocumentCommonInterface::ReplaceMouseArea(compositions::GuiGraphicsComposition* _mouseArea)
			{
				if (documentMouseArea)
				{
					documentMouseArea->GetEventReceiver()->mouseMove.Detach(onMouseMoveHandler);
					documentMouseArea->GetEventReceiver()->leftButtonDown.Detach(onMouseDownHandler);
					documentMouseArea->GetEventReceiver()->leftButtonUp.Detach(onMouseUpHandler);
					documentMouseArea->GetEventReceiver()->mouseLeave.Detach(onMouseLeaveHandler);

					onMouseMoveHandler = nullptr;
					onMouseDownHandler = nullptr;
					onMouseUpHandler = nullptr;
					onMouseLeaveHandler = nullptr;
				}
				documentMouseArea = _mouseArea;
				if (documentMouseArea)
				{
					onMouseMoveHandler = documentMouseArea->GetEventReceiver()->mouseMove.AttachMethod(this, &GuiDocumentCommonInterface::OnMouseMove);
					onMouseDownHandler = documentMouseArea->GetEventReceiver()->leftButtonDown.AttachMethod(this, &GuiDocumentCommonInterface::OnMouseDown);
					onMouseUpHandler = documentMouseArea->GetEventReceiver()->leftButtonUp.AttachMethod(this, &GuiDocumentCommonInterface::OnMouseUp);
					onMouseLeaveHandler = documentMouseArea->GetEventReceiver()->mouseLeave.AttachMethod(this, &GuiDocumentCommonInterface::OnMouseLeave);
				}
			}

			void GuiDocumentCommonInterface::SetActiveHyperlink(Ptr<DocumentHyperlinkRun::Package> package)
			{
				ActivateActiveHyperlink(false);
				activeHyperlinks =
					!package ? nullptr :
					package->hyperlinks.Count() == 0 ? nullptr :
					package;
				ActivateActiveHyperlink(true);
				ActiveHyperlinkChanged.Execute(documentControl->GetNotifyEventArguments());
			}

			void GuiDocumentCommonInterface::ActivateActiveHyperlink(bool activate)
			{
				if (activeHyperlinks)
				{
					for (auto run : activeHyperlinks->hyperlinks)
					{
						run->styleName = activate ? run->activeStyleName : run->normalStyleName;
					}
					documentElement->NotifyParagraphUpdated(activeHyperlinks->row, 1, 1, false);
				}
			}

			void GuiDocumentCommonInterface::AddShortcutCommand(VKEY key, const Func<void()>& eventHandler)
			{
				IGuiShortcutKeyItem* item=internalShortcutKeyManager->CreateNewShortcut(true, false, false, key);
				item->Executed.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
				{
					eventHandler();
				});
			}

			void GuiDocumentCommonInterface::EditTextInternal(TextPos begin, TextPos end, const Func<void(TextPos, TextPos, vint&, vint&)>& editor, bool clearUndoRedo)
			{
				// save run before editing
				if(begin>end)
				{
					TextPos temp=begin;
					begin=end;
					end=temp;
				}
				Ptr<DocumentModel> originalModel=documentElement->GetDocument()->CopyDocument(begin, end, true);
				if(originalModel)
				{
					// edit
					vint paragraphCount=0;
					vint lastParagraphLength=0;
					editor(begin, end, paragraphCount, lastParagraphLength);

					// calculate new caret
					TextPos caret;
					if(paragraphCount==0)
					{
						caret=begin;
					}
					else if(paragraphCount==1)
					{
						caret=TextPos(begin.row, begin.column+lastParagraphLength);
					}
					else
					{
						caret=TextPos(begin.row+paragraphCount-1, lastParagraphLength);
					}
					documentElement->SetCaret(caret, caret, true);
					EnsureDocumentRectVisible(documentElement->GetCaretBounds(caret, true));
					documentControl->TextChanged.Execute(documentControl->GetNotifyEventArguments());
					UpdateCaretPoint();
					SelectionChanged.Execute(documentControl->GetNotifyEventArguments());

					if (clearUndoRedo)
					{
						undoRedoProcessor->ClearUndoRedo();
					}
					else
					{
						// save run after editing
						Ptr<DocumentModel> inputModel = documentElement->GetDocument()->CopyDocument(begin, caret, true);

						// submit redo-undo
						GuiDocumentUndoRedoProcessor::ReplaceModelStruct arguments;
						arguments.originalStart = begin;
						arguments.originalEnd = end;
						arguments.originalModel = originalModel;
						arguments.inputStart = begin;
						arguments.inputEnd = caret;
						arguments.inputModel = inputModel;
						undoRedoProcessor->OnReplaceModel(arguments);
					}
				}
			}

			void GuiDocumentCommonInterface::EditStyleInternal(TextPos begin, TextPos end, const Func<void(TextPos, TextPos)>& editor)
			{
				// save run before editing
				if(begin>end)
				{
					TextPos temp=begin;
					begin=end;
					end=temp;
				}
				Ptr<DocumentModel> originalModel=documentElement->GetDocument()->CopyDocument(begin, end, true);
				if(originalModel)
				{
					// edit
					editor(begin, end);

					// save run after editing
					Ptr<DocumentModel> inputModel=documentElement->GetDocument()->CopyDocument(begin, end, true);

					// submit redo-undo
					GuiDocumentUndoRedoProcessor::ReplaceModelStruct arguments;
					arguments.originalStart=begin;
					arguments.originalEnd=end;
					arguments.originalModel=originalModel;
					arguments.inputStart=begin;
					arguments.inputEnd=end;
					arguments.inputModel=inputModel;
					undoRedoProcessor->OnReplaceModel(arguments);
				}
			}

			void GuiDocumentCommonInterface::MergeBaselineAndDefaultFont(Ptr<DocumentModel> document)
			{
				document->MergeDefaultFont(documentControl->GetDisplayFont());
				if (baselineDocument)
				{
					document->MergeBaselineStyles(baselineDocument);
				}
			}

			void GuiDocumentCommonInterface::OnFontChanged()
			{
				auto document = documentElement->GetDocument();
				MergeBaselineAndDefaultFont(document);
				documentElement->SetDocument(document);
			}

			void GuiDocumentCommonInterface::OnCaretNotify(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (documentControl->GetVisuallyEnabled())
				{
					if (editMode != GuiDocumentEditMode::ViewOnly)
					{
						documentElement->SetCaretVisible(!documentElement->GetCaretVisible());
					}
				}
			}

			void GuiDocumentCommonInterface::OnGotFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (documentControl->GetVisuallyEnabled())
				{
					if (editMode != GuiDocumentEditMode::ViewOnly)
					{
						documentElement->SetCaretVisible(true);
						UpdateCaretPoint();
					}
				}
			}

			void GuiDocumentCommonInterface::OnLostFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(documentControl->GetVisuallyEnabled())
				{
					documentElement->SetCaretVisible(false);
				}
			}

			void GuiDocumentCommonInterface::OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments)
			{
				if (documentControl->GetVisuallyEnabled())
				{
					if (editMode != GuiDocumentEditMode::ViewOnly)
					{
						if (ProcessKey(arguments.code, arguments.shift, arguments.ctrl))
						{
							arguments.handled = true;
						}
					}
				}
			}

			void GuiDocumentCommonInterface::OnCharInput(compositions::GuiGraphicsComposition* sender, compositions::GuiCharEventArgs& arguments)
			{
				if (documentControl->GetVisuallyEnabled())
				{
					if (editMode == GuiDocumentEditMode::Editable &&
						arguments.code != (wchar_t)VKEY::KEY_ESCAPE &&
						arguments.code != (wchar_t)VKEY::KEY_BACK &&
						arguments.code != (wchar_t)VKEY::KEY_RETURN &&
						(arguments.code != (wchar_t)VKEY::KEY_TAB || documentControl->GetAcceptTabInput()) &&
						!arguments.ctrl)
					{
						Array<WString> text(1);
						text[0] = WString::FromChar(arguments.code);
						EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
					}
				}
			}

			void GuiDocumentCommonInterface::UpdateCursor(INativeCursor* cursor)
			{
				if (documentMouseArea)
				{
					documentMouseArea->SetAssociatedCursor(cursor);
				}
			}

			Point GuiDocumentCommonInterface::GetMouseOffset()
			{
				if (documentMouseArea)
				{
					auto documentBounds = documentComposition->GetGlobalBounds();
					auto mouseAreaBounds = documentMouseArea->GetGlobalBounds();
					return Point(
						documentBounds.x1 - mouseAreaBounds.x1,
						documentBounds.y1 - mouseAreaBounds.y1
					);
				}
				else
				{
					return Point(0, 0);
				}
			}

			void GuiDocumentCommonInterface::OnMouseMove(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				auto offset = GetMouseOffset();
				auto x = arguments.x - offset.x;
				auto y = arguments.y - offset.y;

				if(documentControl->GetVisuallyEnabled())
				{
					switch(editMode)
					{
					case GuiDocumentEditMode::ViewOnly:
						{
							auto package = documentElement->GetHyperlinkFromPoint({ x, y });
							bool handCursor = false;

							if(dragging)
							{
								if(activeHyperlinks)
								{
									if (package && CompareEnumerable(activeHyperlinks->hyperlinks, package->hyperlinks) == 0)
									{
										ActivateActiveHyperlink(true);
										handCursor = true;
									}
									else
									{
										ActivateActiveHyperlink(false);
									}
								}
							}
							else
							{
								SetActiveHyperlink(package);
								handCursor = activeHyperlinks;
							}

							if(handCursor)
							{
								auto cursor = GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::Hand);
								UpdateCursor(cursor);
							}
							else
							{
								UpdateCursor(nullptr);
							}
						}
						break;
					case GuiDocumentEditMode::Selectable:
					case GuiDocumentEditMode::Editable:
						if(dragging)
						{
							TextPos caret=documentElement->CalculateCaretFromPoint(Point(x, y));
							TextPos oldCaret=documentElement->GetCaretBegin();
							Move(caret, true, (oldCaret==caret?documentElement->IsCaretEndPreferFrontSide():caret<oldCaret));
						}
						break;
					}
				}
			}

			void GuiDocumentCommonInterface::OnMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				auto offset = GetMouseOffset();
				auto x = arguments.x - offset.x;
				auto y = arguments.y - offset.y;

				if(documentControl->GetVisuallyEnabled())
				{
					switch(editMode)
					{
					case GuiDocumentEditMode::ViewOnly:
						SetActiveHyperlink(documentElement->GetHyperlinkFromPoint({ x, y }));
						break;
					case GuiDocumentEditMode::Selectable:
					case GuiDocumentEditMode::Editable:
						{
							documentControl->SetFocused();
							TextPos caret=documentElement->CalculateCaretFromPoint(Point(x, y));
							TextPos oldCaret=documentElement->GetCaretEnd();
							if(caret!=oldCaret)
							{
								Move(caret, arguments.shift, caret<oldCaret);
							}
						}
						break;
					}
					dragging=true;
				}
			}

			void GuiDocumentCommonInterface::OnMouseUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				auto offset = GetMouseOffset();
				auto x = arguments.x - offset.x;
				auto y = arguments.y - offset.y;

				if(documentControl->GetVisuallyEnabled())
				{
					dragging=false;
					switch(editMode)
					{
					case GuiDocumentEditMode::ViewOnly:
						{
							auto package = documentElement->GetHyperlinkFromPoint({ x, y });
							if(activeHyperlinks)
							{
								if (package && CompareEnumerable(activeHyperlinks->hyperlinks, package->hyperlinks) == 0)
								{
									ActiveHyperlinkExecuted.Execute(documentControl->GetNotifyEventArguments());
								}
								else
								{
									SetActiveHyperlink(nullptr);
								}
							}
						}
						break;
					default:;
					}
				}
			}

			void GuiDocumentCommonInterface::OnMouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				SetActiveHyperlink(nullptr);
			}

			//================ callback

			void GuiDocumentCommonInterface::OnStartRender()
			{
				for (auto item : documentItems.Values())
				{
					item->visible = false;
				}
			}

			void GuiDocumentCommonInterface::OnFinishRender()
			{
				for (auto item : documentItems.Values())
				{
					if (item->container->GetVisible() != item->visible)
					{
						item->container->SetVisible(item->visible);
					}
				}
			}

			Size GuiDocumentCommonInterface::OnRenderEmbeddedObject(const WString& name, const Rect& location)
			{
				vint index = documentItems.Keys().IndexOf(name);
				if (index != -1)
				{
					auto item = documentItems.Values()[index];
					auto size = item->container->GetCachedBounds().GetSize();
					item->container->SetExpectedBounds(Rect(location.LeftTop(), Size(0, 0)));
					item->visible = true;
					return size;
				}
				return Size();
			}

			//================ basic

			struct FetchLineRecord
			{
				const wchar_t*			begin = nullptr;
				const wchar_t*			end = nullptr;
				const wchar_t*			next = nullptr;
			};

			FetchLineRecord FetchLineRecord_Init(const wchar_t* text)
			{
				return { text,text,nullptr };
			}

			FetchLineRecord FetchLineRecord_Join(const FetchLineRecord& flr1, const FetchLineRecord& flr2)
			{
				return { flr1.begin, flr2.end, flr2.next };
			}

			void FetchLineRecord_Next(FetchLineRecord& record)
			{
				while (*record.end != '\n' && *record.end != '\0') record.end++;
				record.next = record.end;
				while (record.end > record.begin && record.end[-1] == L'\r') record.end--;
				if (*record.next == '\n') record.next++;
			}

			WString FetchLineRecord_Get(const FetchLineRecord& flr, const wchar_t* buffer, const WString& text)
			{
				return text.Sub(flr.begin - buffer, flr.end - flr.begin);
			}

			void GuiDocumentCommonInterface::UserInput_ConvertToPlainText(Ptr<DocumentModel> model, vint beginParagraph, vint endParagraph)
			{
				if (beginParagraph > endParagraph) return;

				RunRangeMap runRanges;
				document_editor::GetRunRange(model->paragraphs[endParagraph].Obj(), runRanges);

				TextPos begin(beginParagraph, 0);
				TextPos end(endParagraph, runRanges[model->paragraphs[endParagraph].Obj()].end);
				model->ConvertToPlainText(begin, end);

				for (vint i = beginParagraph; i <= endParagraph; i++)
				{
					model->paragraphs[i]->alignment.Reset();
				}
			}

			void GuiDocumentCommonInterface::UserInput_JoinParagraphs(collections::List<WString>& paragraphTexts, bool spaceForFlattenedLineBreak)
			{
				auto line = stream::GenerateToStream([&](stream::StreamWriter& writer)
				{
					for(auto [paragraph, index] : indexed(paragraphTexts))
					{
						if (index > 0 && spaceForFlattenedLineBreak)
						{
							writer.WriteChar(L' ');
						}
						writer.WriteString(paragraph);
					}
				});
				paragraphTexts.Clear();
				paragraphTexts.Add(line);
			}

			void GuiDocumentCommonInterface::UserInput_JoinParagraphs(Ptr<DocumentModel> model, bool spaceForFlattenedLineBreak)
			{
				auto firstParagraph = model->paragraphs[0];
				for (auto paragraph : From(model->paragraphs).Skip(1))
				{
					if (spaceForFlattenedLineBreak)
					{
						auto textRun = Ptr(new DocumentTextRun);
						textRun->text = WString::Unmanaged(L" ");
						firstParagraph->runs.Add(textRun);
					}
					CopyFrom(firstParagraph->runs, paragraph->runs, true);
				}
				model->paragraphs.Clear();
				model->paragraphs.Add(firstParagraph);
			}

			void GuiDocumentCommonInterface::UserInput_JoinLinesInsideParagraph(WString& text, bool spaceForFlattenedLineBreak)
			{
				const wchar_t* buffer = text.Buffer();
				auto flr = FetchLineRecord_Init(buffer);
				FetchLineRecord_Next(flr);
				if (!*flr.next)
				{
					bool addSpace = flr.end < flr.next && flr.next[-1] != L'\r' && spaceForFlattenedLineBreak;
					text = FetchLineRecord_Get(flr, buffer, text);
					if (addSpace)
					{
						text += WString::Unmanaged(L" ");
					}
					return;
				}

				text = stream::GenerateToStream([&](stream::StreamWriter& writer)
				{
					writer.WriteString(FetchLineRecord_Get(flr, buffer, text));
					while (*flr.end)
					{
						if (spaceForFlattenedLineBreak)
						{
							writer.WriteChar(L' ');
						}

						flr = FetchLineRecord_Init(flr.next);
						FetchLineRecord_Next(flr);
						writer.WriteString(FetchLineRecord_Get(flr, buffer, text));
					}
				});
			}

			void GuiDocumentCommonInterface::UserInput_JoinLinesInsideParagraph(Ptr<DocumentParagraphRun> paragraph, bool spaceForFlattenedLineBreak)
			{
				List<Ptr<DocumentContainerRun>> containers;
				containers.Add(paragraph);

				for (vint i = 0; i < containers.Count(); i++)
				{
					auto container = containers[i];
					for (auto run : container->runs)
					{
						if (auto subContainer = run.Cast<DocumentContainerRun>())
						{
							containers.Add(subContainer);
						}
						else if (auto textRun = run.Cast<DocumentTextRun>())
						{
							UserInput_JoinLinesInsideParagraph(textRun->text, spaceForFlattenedLineBreak);
						}
					}
				}
			}

			void GuiDocumentCommonInterface::UserInput_FormatText(collections::List<WString>& paragraphTexts, const GuiDocumentConfigEvaluated& config)
			{
				if (config.paragraphMode != GuiDocumentParagraphMode::Paragraph)
				{
					for (vint i = 0; i < paragraphTexts.Count(); i++)
					{
						UserInput_JoinLinesInsideParagraph(paragraphTexts[i], config.spaceForFlattenedLineBreak);
					}
				}
				if (config.paragraphMode == GuiDocumentParagraphMode::Singleline)
				{
					UserInput_JoinParagraphs(paragraphTexts, config.spaceForFlattenedLineBreak);
				}
			}

			void GuiDocumentCommonInterface::UserInput_FormatText(const WString& text, collections::List<WString>& paragraphTexts, const GuiDocumentConfigEvaluated& config)
			{
				if (config.doubleLineBreaksBetweenParagraph)
				{
					const wchar_t* buffer = text.Buffer();
					auto flr = FetchLineRecord_Init(buffer);
					FetchLineRecord_Next(flr);
					bool remaining = false;
					while (*flr.begin)
					{
						paragraphTexts.Add(stream::GenerateToStream([&](stream::StreamWriter& writer)
						{
							auto flrFragmentFirst = flr;
							auto flrFragmentLast = flrFragmentFirst;
							auto SubmitFragment = [&](bool endingEmptyLines)
							{
								auto flrFragment = FetchLineRecord_Join(flrFragmentFirst, flrFragmentLast);
								writer.WriteString(FetchLineRecord_Get(flrFragment, buffer, text));
								if (flrFragment.end != flrFragment.next && endingEmptyLines)
								{
									if (config.paragraphMode == GuiDocumentParagraphMode::Paragraph)
									{
										writer.WriteString(L"\r\n");
									}
									else if (config.spaceForFlattenedLineBreak)
									{
										writer.WriteChar(L' ');
									}
								}
							};

							while (true)
							{
								if (!*flrFragmentLast.next)
								{
									SubmitFragment(true);
									flr = FetchLineRecord_Init(flrFragmentLast.next);
									remaining = false;
									return;
								}

								auto flrNext = FetchLineRecord_Init(flrFragmentLast.next);
								FetchLineRecord_Next(flrNext);
								if (flrNext.end == flrNext.begin)
								{
									SubmitFragment(false);
									flr = FetchLineRecord_Init(flrNext.next);
									FetchLineRecord_Next(flr);
									remaining = true;
									return;
								}

								if (flrFragmentLast.next - flrFragmentLast.end == 2)
								{
									flrFragmentLast = flrNext;
								}
								else
								{
									SubmitFragment(true);
									flrFragmentFirst = flrNext;
									flrFragmentLast = flrNext;
								}
							}
						}));
					}

					if (remaining)
					{
						paragraphTexts.Add(WString::Empty);
					}
				}
				else
				{
					stream::StringReader reader(text);
					while (!reader.IsEnd())
					{
						WString line = reader.ReadLine();
						paragraphTexts.Add(line);
					}
				}

				if (config.paragraphMode == GuiDocumentParagraphMode::Singleline)
				{
					UserInput_JoinParagraphs(paragraphTexts, config.spaceForFlattenedLineBreak);
				}
			}

			void GuiDocumentCommonInterface::UserInput_FormatDocument(Ptr<DocumentModel> model, Ptr<DocumentModel> baselineDocument, const GuiDocumentConfigEvaluated& config)
			{
				if (!model) return;
				if (config.pasteAsPlainText)
				{
					UserInput_ConvertToPlainText(model, 0, model->paragraphs.Count() - 1);

					if (baselineDocument)
					{
						CopyFrom(model->styles, baselineDocument->styles);
					}
					else
					{
						model->styles.Clear();
					}
				}

				if (model->paragraphs.Count() == 0)
				{
					return;
				}

				if (config.paragraphMode != GuiDocumentParagraphMode::Paragraph)
				{
					for (auto paragraph : model->paragraphs)
					{
						UserInput_JoinLinesInsideParagraph(paragraph, config.spaceForFlattenedLineBreak);
					}
				}
				if (config.paragraphMode == GuiDocumentParagraphMode::Singleline)
				{
					UserInput_JoinParagraphs(model, config.spaceForFlattenedLineBreak);
				}
			}

			WString GuiDocumentCommonInterface::UserInput_ConvertDocumentToText(Ptr<DocumentModel> model)
			{
				return model->GetTextForReading(WString::Unmanaged(config.doubleLineBreaksBetweenParagraph ? L"\r\n\r\n" : L"\r\n"));
			}

			GuiDocumentCommonInterface::GuiDocumentCommonInterface(const GuiDocumentConfig& _config)
				: config(_config)
			{
				undoRedoProcessor = Ptr(new GuiDocumentUndoRedoProcessor);
				internalShortcutKeyManager = Ptr(new GuiShortcutKeyManager);
				AddShortcutCommand(VKEY::KEY_Z, Func<bool()>(this, &GuiDocumentCommonInterface::Undo));
				AddShortcutCommand(VKEY::KEY_Y, Func<bool()>(this, &GuiDocumentCommonInterface::Redo));
				AddShortcutCommand(VKEY::KEY_A, Func<void()>(this, &GuiDocumentCommonInterface::SelectAll));
				AddShortcutCommand(VKEY::KEY_X, Func<bool()>(this, &GuiDocumentCommonInterface::Cut));
				AddShortcutCommand(VKEY::KEY_C, Func<bool()>(this, &GuiDocumentCommonInterface::Copy));
				AddShortcutCommand(VKEY::KEY_V, Func<bool()>(this, &GuiDocumentCommonInterface::Paste));
			}

			GuiDocumentCommonInterface::~GuiDocumentCommonInterface()
			{
			}

			Ptr<DocumentModel> GuiDocumentCommonInterface::GetDocument()
			{
				return documentElement->GetDocument();
			}

			void GuiDocumentCommonInterface::SetDocument(Ptr<DocumentModel> value)
			{
				value = value ? value->CopyDocument() : nullptr;
				UserInput_FormatDocument(value, baselineDocument, config);
				SetActiveHyperlink(0);
				ClearUndoRedo();
				NotifyModificationSaved();

				if (value)
				{
					if (value->paragraphs.Count() == 0)
					{
						value->paragraphs.Add(Ptr(new DocumentParagraphRun));
					}
					MergeBaselineAndDefaultFont(value);
				}

				documentElement->SetDocument(value);
			}

			//================ document items

			bool GuiDocumentCommonInterface::AddDocumentItem(Ptr<GuiDocumentItem> value)
			{
				if (documentItems.Keys().Contains(value->GetName()))
				{
					return false;
				}
				documentItems.Add(value->GetName(), value);
				documentComposition->AddChild(value->container);
				value->visible = false;
				value->owned = true;
				value->container->SetVisible(false);
				return true;
			}

			bool GuiDocumentCommonInterface::RemoveDocumentItem(Ptr<GuiDocumentItem> value)
			{
				vint index = documentItems.Keys().IndexOf(value->GetName());
				if (index == -1)
				{
					return false;
				}
				if (documentItems.Values()[index] != value)
				{
					return false;
				}
				value->owned = false;
				documentComposition->RemoveChild(value->container);
				documentItems.Remove(value->GetName());
				return true;
			}

			const GuiDocumentCommonInterface::DocumentItemMap& GuiDocumentCommonInterface::GetDocumentItems()
			{
				return documentItems;
			}

			//================ caret operations

			TextPos GuiDocumentCommonInterface::GetCaretBegin()
			{
				return documentElement->GetCaretBegin();
			}

			TextPos GuiDocumentCommonInterface::GetCaretEnd()
			{
				return documentElement->GetCaretEnd();
			}

			void GuiDocumentCommonInterface::SetCaret(TextPos begin, TextPos end)
			{
				documentElement->SetCaret(begin, end, end>=begin);
				UpdateCaretPoint();
				SelectionChanged.Execute(documentControl->GetNotifyEventArguments());
			}

			TextPos GuiDocumentCommonInterface::CalculateCaretFromPoint(Point point)
			{
				return documentElement->CalculateCaretFromPoint(point);
			}

			Rect GuiDocumentCommonInterface::GetCaretBounds(TextPos caret, bool frontSide)
			{
				return documentElement->GetCaretBounds(caret, frontSide);
			}

			//================ editing operations

			void GuiDocumentCommonInterface::NotifyParagraphUpdated(vint index, vint oldCount, vint newCount, bool updatedText, bool skipFormatting)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::GuiDocumentCommonInterface::NotifyParagraphUpdated(vint, vint, vint, bool, bool)#"
				auto model = documentElement->GetDocument();
				if (config.paragraphMode == GuiDocumentParagraphMode::Singleline)
				{
					CHECK_ERROR(model->paragraphs.Count() <= 1, ERROR_MESSAGE_PREFIX L"In Singleline mode, there should be no more than 1 paragraphs in the document.");
				}

				if (0 <= index && index + newCount <= model->paragraphs.Count())
				{
					if (config.pasteAsPlainText && updatedText)
					{
						if(!skipFormatting)
						{
							UserInput_ConvertToPlainText(model, index, index + newCount - 1);
						}
						if (baselineDocument)
						{
							CopyFrom(model->styles, baselineDocument->styles);
						}
						else
						{
							model->styles.Clear();
						}
					}

					if (config.paragraphMode != GuiDocumentParagraphMode::Paragraph && !skipFormatting)
					{
						for (vint i = index; i < index + newCount; i++)
						{
							UserInput_JoinLinesInsideParagraph(model->paragraphs[i], config.spaceForFlattenedLineBreak);
						}
					}

					documentElement->NotifyParagraphUpdated(index, oldCount, newCount, updatedText);
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void GuiDocumentCommonInterface::EditRun(TextPos begin, TextPos end, Ptr<DocumentModel> model, bool copy, bool skipFormatting)
			{
				if (!skipFormatting)
				{
					UserInput_FormatDocument(model, baselineDocument, config);
				}
				EditTextInternal(begin, end, [=, this](TextPos begin, TextPos end, vint& paragraphCount, vint& lastParagraphLength)
				{
					documentElement->EditRun(begin, end, model, copy);
					paragraphCount=model->paragraphs.Count();
					lastParagraphLength=paragraphCount==0?0:model->paragraphs[paragraphCount-1]->GetTextForCaret().Length();
				});
			}

			void GuiDocumentCommonInterface::EditText(TextPos begin, TextPos end, bool frontSide, const collections::Array<WString>& text, bool skipFormatting)
			{
				EditTextInternal(begin, end, [=, this, &text](TextPos begin, TextPos end, vint& paragraphCount, vint& lastParagraphLength)
				{
					Array<WString> updatedText;
					bool useUpdatedText = config.paragraphMode != GuiDocumentParagraphMode::Paragraph && !skipFormatting;
					if (useUpdatedText)
					{
						List<WString> paragraphTexts;
						CopyFrom(paragraphTexts, text);
						UserInput_FormatText(paragraphTexts, config);
						CopyFrom(updatedText, paragraphTexts);
					}

					const Array<WString>& textToUse = useUpdatedText ? updatedText : text;
					documentElement->EditText(begin, end, frontSide, textToUse);
					paragraphCount = textToUse.Count();
					lastParagraphLength = paragraphCount == 0 ? 0 : textToUse[paragraphCount - 1].Length();
				});
			}

			void GuiDocumentCommonInterface::EditStyle(TextPos begin, TextPos end, Ptr<DocumentStyleProperties> style)
			{
				if (config.pasteAsPlainText) return;
				EditStyleInternal(begin, end, [=, this](TextPos begin, TextPos end)
				{
					documentElement->EditStyle(begin, end, style);
				});
			}

			void GuiDocumentCommonInterface::EditImage(TextPos begin, TextPos end, Ptr<GuiImageData> image)
			{
				if (config.pasteAsPlainText) return;
				EditTextInternal(begin, end, [=, this](TextPos begin, TextPos end, vint& paragraphCount, vint& lastParagraphLength)
				{
					documentElement->EditImage(begin, end, image);
					paragraphCount=1;
					lastParagraphLength=wcslen(DocumentImageRun::RepresentationText);
				});
			}

			void GuiDocumentCommonInterface::EditHyperlink(vint paragraphIndex, vint begin, vint end, const WString& reference, const WString& normalStyleName, const WString& activeStyleName)
			{
				if (config.pasteAsPlainText) return;
				EditStyleInternal(TextPos(paragraphIndex, begin), TextPos(paragraphIndex, end), [=, this](TextPos begin, TextPos end)
				{
					documentElement->EditHyperlink(begin.row, begin.column, end.column, reference, normalStyleName, activeStyleName);
				});
			}

			void GuiDocumentCommonInterface::RemoveHyperlink(vint paragraphIndex, vint begin, vint end)
			{
				if (config.pasteAsPlainText) return;
				EditStyleInternal(TextPos(paragraphIndex, begin), TextPos(paragraphIndex, end), [=, this](TextPos begin, TextPos end)
				{
					documentElement->RemoveHyperlink(begin.row, begin.column, end.column);
				});
			}

			void GuiDocumentCommonInterface::EditStyleName(TextPos begin, TextPos end, const WString& styleName)
			{
				if (config.pasteAsPlainText) return;
				EditStyleInternal(begin, end, [=, this](TextPos begin, TextPos end)
				{
					documentElement->EditStyleName(begin, end, styleName);
				});
			}

			void GuiDocumentCommonInterface::RemoveStyleName(TextPos begin, TextPos end)
			{
				if (config.pasteAsPlainText) return;
				EditStyleInternal(begin, end, [=, this](TextPos begin, TextPos end)
				{
					documentElement->RemoveStyleName(begin, end);
				});
			}

			void GuiDocumentCommonInterface::RenameStyle(const WString& oldStyleName, const WString& newStyleName)
			{
				if (config.pasteAsPlainText) return;
				documentElement->RenameStyle(oldStyleName, newStyleName);

				// submit redo-undo
				GuiDocumentUndoRedoProcessor::RenameStyleStruct arguments;
				arguments.oldStyleName=oldStyleName;
				arguments.newStyleName=newStyleName;
				undoRedoProcessor->OnRenameStyle(arguments);
			}

			void GuiDocumentCommonInterface::ClearStyle(TextPos begin, TextPos end)
			{
				if (config.pasteAsPlainText) return;
				EditStyleInternal(begin, end, [=, this](TextPos begin, TextPos end)
				{
					documentElement->ClearStyle(begin, end);
				});
			}

			void GuiDocumentCommonInterface::ConvertToPlainText(TextPos begin, TextPos end)
			{
				EditStyleInternal(begin, end, [=, this](TextPos begin, TextPos end)
				{
					documentElement->ConvertToPlainText(begin, end);
				});
			}

			Ptr<DocumentStyleProperties> GuiDocumentCommonInterface::SummarizeStyle(TextPos begin, TextPos end)
			{
				if (begin>end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}
				return documentElement->SummarizeStyle(begin, end);
			}

			Nullable<WString> GuiDocumentCommonInterface::SummarizeStyleName(TextPos begin, TextPos end)
			{
				if (begin>end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}
				return documentElement->SummarizeStyleName(begin, end);
			}

			void GuiDocumentCommonInterface::SetParagraphAlignments(TextPos begin, TextPos end, const collections::Array<Nullable<Alignment>>& alignments)
			{
				vint first = begin.row;
				vint last = end.row;
				if (first > last)
				{
					vint temp = first;
					first = last;
					last = temp;
				}

				Ptr<DocumentModel> document = documentElement->GetDocument();
				if (0 <= first && first < document->paragraphs.Count() && 0 <= last && last < document->paragraphs.Count() && last - first + 1 == alignments.Count())
				{
					auto arguments = Ptr(new GuiDocumentUndoRedoProcessor::SetAlignmentStruct);
					arguments->start = first;
					arguments->end = last;
					arguments->originalAlignments.Resize(alignments.Count());
					arguments->inputAlignments.Resize(alignments.Count());
					for (vint i = first; i <= last; i++)
					{
						arguments->originalAlignments[i - first] = document->paragraphs[i]->alignment;
						arguments->inputAlignments[i - first] = alignments[i - first];
					}
					documentElement->SetParagraphAlignment(begin, end, alignments);
					undoRedoProcessor->OnSetAlignment(arguments);
				}
			}

			void GuiDocumentCommonInterface::SetParagraphAlignment(TextPos begin, TextPos end, Nullable<Alignment> alignment)
			{
#if defined VCZH_GCC && defined VCZH_64
#define abs labs
#endif
				Array<Nullable<Alignment>> alignments(abs(begin.row - end.row) + 1);
#if defined VCZH_GCC && defined VCZH_64
#undef abs
#endif
				for (vint i = 0; i < alignments.Count(); i++)
				{
					alignments[i] = alignment;
				}
				SetParagraphAlignments(begin, end, alignments);
			}

			Nullable<Alignment> GuiDocumentCommonInterface::SummarizeParagraphAlignment(TextPos begin, TextPos end)
			{
				if (begin>end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}
				return documentElement->SummarizeParagraphAlignment(begin, end);
			}

			//================ editing control

			WString GuiDocumentCommonInterface::GetActiveHyperlinkReference()
			{
				return activeHyperlinks ? activeHyperlinks->hyperlinks[0]->reference : L"";
			}

			GuiDocumentEditMode GuiDocumentCommonInterface::GetEditMode()
			{
				return editMode;
			}

			void GuiDocumentCommonInterface::SetEditMode(GuiDocumentEditMode value)
			{
				if (activeHyperlinks)
				{
					SetActiveHyperlink(nullptr);
				}

				editMode = value;
				if (editMode == GuiDocumentEditMode::ViewOnly)
				{
					UpdateCursor(nullptr);
				}
				else
				{
					INativeCursor* cursor = GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::IBeam);
					UpdateCursor(cursor);
				}
			}

			void GuiDocumentCommonInterface::LoadTextAndClearUndoRedo(const WString& text)
			{
				vint lastIndex = documentElement->GetDocument()->paragraphs.Count() - 1;
				Ptr<DocumentParagraphRun> lastParagraph = documentElement->GetDocument()->paragraphs[lastIndex];

				TextPos begin(0, 0);
				TextPos end(lastIndex, lastParagraph->GetTextForCaret().Length());

				List<WString> paragraphTexts;
				UserInput_FormatText(text, paragraphTexts, config);
				Array<WString> paragraphLines;
				CopyFrom(paragraphLines, paragraphTexts);

				EditTextInternal(begin, end, [=, this, &paragraphLines](TextPos begin, TextPos end, vint& paragraphCount, vint& lastParagraphLength)
				{
					documentElement->EditText(begin, end, true, paragraphLines);
					paragraphCount = paragraphLines.Count();
					lastParagraphLength = paragraphCount == 0 ? 0 : paragraphLines[paragraphCount - 1].Length();
				}, true);

				SetCaret(begin, begin);
				EnsureDocumentRectVisible(documentElement->GetCaretBounds(begin, true));
			}

			void GuiDocumentCommonInterface::LoadDocumentAndClearUndoRedo(Ptr<DocumentModel> document, bool copy)
			{
				vint lastIndex = documentElement->GetDocument()->paragraphs.Count() - 1;
				Ptr<DocumentParagraphRun> lastParagraph = documentElement->GetDocument()->paragraphs[lastIndex];

				TextPos begin(0, 0);
				TextPos end(lastIndex, lastParagraph->GetTextForCaret().Length());

				document = copy ? (document ? document->CopyDocument() : nullptr) : document;
				EditTextInternal(begin, end, [=, this](TextPos begin, TextPos end, vint& paragraphCount, vint& lastParagraphLength)
				{
					documentElement->EditRun(begin, end, document, false);
					paragraphCount = document->paragraphs.Count();
					lastParagraphLength = paragraphCount == 0 ? 0 : document->paragraphs[paragraphCount - 1]->GetTextForCaret().Length();
				}, true);

				SetCaret(begin, begin);
				EnsureDocumentRectVisible(documentElement->GetCaretBounds(begin, true));
			}

			//================ selection operations

			void GuiDocumentCommonInterface::SelectAll()
			{
				vint lastIndex = documentElement->GetDocument()->paragraphs.Count() - 1;
				Ptr<DocumentParagraphRun> lastParagraph = documentElement->GetDocument()->paragraphs[lastIndex];

				TextPos begin(0, 0);
				TextPos end(lastIndex, lastParagraph->GetTextForCaret().Length());
				SetCaret(begin, end);
			}

			WString GuiDocumentCommonInterface::GetSelectionText()
			{
				TextPos begin=documentElement->GetCaretBegin();
				TextPos end=documentElement->GetCaretEnd();
				if(begin>end)
				{
					TextPos temp=begin;
					begin=end;
					end=temp;
				}

				Ptr<DocumentModel> model=documentElement->GetDocument()->CopyDocument(begin, end, false);
				return UserInput_ConvertDocumentToText(model);
			}

			void GuiDocumentCommonInterface::SetSelectionText(const WString& value)
			{
				List<WString> paragraphTexts;
				UserInput_FormatText(value, paragraphTexts, config);

				TextPos begin = documentElement->GetCaretBegin();
				TextPos end = documentElement->GetCaretEnd();
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				Array<WString> text;
				CopyFrom(text, paragraphTexts);
				EditText(begin, end, documentElement->IsCaretEndPreferFrontSide(), text, true);
			}

			Ptr<DocumentModel> GuiDocumentCommonInterface::GetSelectionModel()
			{
				TextPos begin=documentElement->GetCaretBegin();
				TextPos end=documentElement->GetCaretEnd();
				if(begin>end)
				{
					TextPos temp=begin;
					begin=end;
					end=temp;
				}

				Ptr<DocumentModel> model=documentElement->GetDocument()->CopyDocument(begin, end, true);
				return model;
			}

			void GuiDocumentCommonInterface::SetSelectionModel(Ptr<DocumentModel> value)
			{
				value = value ? value->CopyDocument() : nullptr;
				UserInput_FormatDocument(value, baselineDocument, config);
				TextPos begin=documentElement->GetCaretBegin();
				TextPos end=documentElement->GetCaretEnd();
				if(begin>end)
				{
					TextPos temp=begin;
					begin=end;
					end=temp;
				}

				EditRun(begin, end, value, true, true);
			}

			//================ clipboard operations

			bool GuiDocumentCommonInterface::CanCut()
			{
				return editMode == GuiDocumentEditMode::Editable && documentElement->GetCaretBegin() != documentElement->GetCaretEnd();
			}

			bool GuiDocumentCommonInterface::CanCopy()
			{
				return documentElement->GetCaretBegin()!=documentElement->GetCaretEnd();
			}

			bool GuiDocumentCommonInterface::CanPaste()
			{
				if (editMode == GuiDocumentEditMode::Editable)
				{
					auto reader = GetCurrentController()->ClipboardService()->ReadClipboard();
					if (config.pasteAsPlainText)
					{
						return reader->ContainsText();
					}
					else
					{
						return reader->ContainsText() || reader->ContainsDocument() || reader->ContainsImage();
					}
				}
				return false;
			}

			bool GuiDocumentCommonInterface::Cut()
			{
				if (!CanCut())return false;
				auto writer = GetCurrentController()->ClipboardService()->WriteClipboard();
				auto model = GetSelectionModel();
				writer->SetText(UserInput_ConvertDocumentToText(model));
				if (!config.pasteAsPlainText)
				{
					writer->SetDocument(model);
				}
				writer->Submit();
				SetSelectionText(L"");
				return true;
			}

			bool GuiDocumentCommonInterface::Copy()
			{
				if (!CanCopy()) return false;
				auto writer = GetCurrentController()->ClipboardService()->WriteClipboard();
				auto model = GetSelectionModel();
				writer->SetText(UserInput_ConvertDocumentToText(model));
				if (!config.pasteAsPlainText)
				{
					writer->SetDocument(model);
				}
				writer->Submit();
				return true;
			}

			bool GuiDocumentCommonInterface::Paste()
			{
				if (!CanPaste()) return false;
				auto reader = GetCurrentController()->ClipboardService()->ReadClipboard();
				if (reader->ContainsDocument() && !config.pasteAsPlainText)
				{
					if (auto document = reader->GetDocument())
					{
						SetSelectionModel(document);
						return true;
					}
				}
				if (reader->ContainsText())
				{
					SetSelectionText(reader->GetText());
					return true;
				}
				if (reader->ContainsImage() && !config.pasteAsPlainText)
				{
					if (auto image = reader->GetImage())
					{
						auto imageData = Ptr(new GuiImageData(image, 0));
						EditImage(GetCaretBegin(), GetCaretEnd(), imageData);
						return true;
					}
				}
				return false;
			}

			//================ undo redo control

			bool GuiDocumentCommonInterface::CanUndo()
			{
				return editMode == GuiDocumentEditMode::Editable && undoRedoProcessor->CanUndo();
			}

			bool GuiDocumentCommonInterface::CanRedo()
			{
				return editMode == GuiDocumentEditMode::Editable && undoRedoProcessor->CanRedo();
			}

			void GuiDocumentCommonInterface::ClearUndoRedo()
			{
				undoRedoProcessor->ClearUndoRedo();
			}

			bool GuiDocumentCommonInterface::GetModified()
			{
				return undoRedoProcessor->GetModified();
			}

			void GuiDocumentCommonInterface::NotifyModificationSaved()
			{
				undoRedoProcessor->NotifyModificationSaved();
			}

			bool GuiDocumentCommonInterface::Undo()
			{
				if(CanUndo())
				{
					return undoRedoProcessor->Undo();
				}
				else
				{
					return false;
				}
			}

			bool GuiDocumentCommonInterface::Redo()
			{
				if(CanRedo())
				{
					return undoRedoProcessor->Redo();
				}
				else
				{
					return false;
				}
			}
		}
	}
}
