#include "GuiDocumentViewer.h"

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

			void GuiDocumentCommonInterface::Move(TextPos caret, bool shift, bool frontSide)
			{
				TextPos begin=documentElement->GetCaretBegin();
				TextPos end=documentElement->GetCaretEnd();
				
				TextPos newBegin=shift?begin:caret;
				TextPos newEnd=caret;
				documentElement->SetCaret(newBegin, newEnd, frontSide);
				documentElement->SetCaretVisible(true);

				Rect bounds=documentElement->GetCaretBounds(newEnd, frontSide);
				if(bounds!=Rect())
				{
					bounds.x1-=15;
					bounds.y1-=15;
					bounds.x2+=15;
					bounds.y2+=15;
					EnsureRectVisible(bounds);
				}
				UpdateCaretPoint();
				SelectionChanged.Execute(documentControl->GetNotifyEventArguments());
			}

			bool GuiDocumentCommonInterface::ProcessKey(vint code, bool shift, bool ctrl)
			{
				if(IGuiShortcutKeyItem* item=internalShortcutKeyManager->TryGetShortcut(ctrl, shift, false, code))
				{
					GuiEventArgs arguments;
					item->Executed.Execute(arguments);
					return true;
				}

				TextPos currentCaret=documentElement->GetCaretEnd();
				bool frontSide=documentElement->IsCaretEndPreferFrontSide();
				TextPos begin=documentElement->GetCaretBegin();
				TextPos end=documentElement->GetCaretEnd();

				switch(code)
				{
				case VKEY_UP:
					{
						TextPos newCaret=documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretMoveUp, frontSide);
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY_DOWN:
					{
						TextPos newCaret=documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretMoveDown, frontSide);
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY_LEFT:
					{
						TextPos newCaret=documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretMoveLeft, frontSide);
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY_RIGHT:
					{
						TextPos newCaret=documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretMoveRight, frontSide);
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY_HOME:
					{
						TextPos newCaret=documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLineFirst, frontSide);
						if(newCaret==currentCaret)
						{
							newCaret=documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretFirst, frontSide);
						}
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY_END:
					{
						TextPos newCaret=documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLineLast, frontSide);
						if(newCaret==currentCaret)
						{
							newCaret=documentElement->CalculateCaret(currentCaret, IGuiGraphicsParagraph::CaretLast, frontSide);
						}
						Move(newCaret, shift, frontSide);
					}
					break;
				case VKEY_PRIOR:
					{
					}
					break;
				case VKEY_NEXT:
					{
					}
					break;
				case VKEY_BACK:
					if(editMode==Editable)
					{
						if(begin==end)
						{
							ProcessKey(VKEY_LEFT, true, false);
						}
						Array<WString> text;
						EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
						return true;
					}
					break;
				case VKEY_DELETE:
					if(editMode==Editable)
					{
						if(begin==end)
						{
							ProcessKey(VKEY_RIGHT, true, false);
						}
						Array<WString> text;
						EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
						return true;
					}
					break;
				case VKEY_RETURN:
					if(editMode==Editable)
					{
						if(ctrl)
						{
							Array<WString> text(1);
							text[0]=L"\r\n";
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
				}
				return false;
			}

			void GuiDocumentCommonInterface::InstallDocumentViewer(GuiControl* _sender, compositions::GuiGraphicsComposition* _container)
			{
				documentControl=_sender;

				documentElement=GuiDocumentElement::Create();
				documentElement->SetCallback(this);
				documentElement->SetCaretColor(caretColor);

				documentComposition=new GuiBoundsComposition;
				documentComposition->SetOwnedElement(documentElement);
				documentComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
				documentComposition->SetAlignmentToParent(Margin(5, 5, 5, 5));
				_container->AddChild(documentComposition);

				documentComposition->GetEventReceiver()->mouseMove.AttachMethod(this, &GuiDocumentCommonInterface::OnMouseMove);
				documentComposition->GetEventReceiver()->leftButtonDown.AttachMethod(this, &GuiDocumentCommonInterface::OnMouseDown);
				documentComposition->GetEventReceiver()->leftButtonUp.AttachMethod(this, &GuiDocumentCommonInterface::OnMouseUp);
				documentComposition->GetEventReceiver()->mouseLeave.AttachMethod(this, &GuiDocumentCommonInterface::OnMouseLeave);

				_sender->FontChanged.AttachMethod(this, &GuiDocumentCommonInterface::OnFontChanged);
				_sender->GetFocusableComposition()->GetEventReceiver()->caretNotify.AttachMethod(this, &GuiDocumentCommonInterface::OnCaretNotify);
				_sender->GetFocusableComposition()->GetEventReceiver()->gotFocus.AttachMethod(this, &GuiDocumentCommonInterface::OnGotFocus);
				_sender->GetFocusableComposition()->GetEventReceiver()->lostFocus.AttachMethod(this, &GuiDocumentCommonInterface::OnLostFocus);
				_sender->GetFocusableComposition()->GetEventReceiver()->keyDown.AttachMethod(this, &GuiDocumentCommonInterface::OnKeyDown);
				_sender->GetFocusableComposition()->GetEventReceiver()->charInput.AttachMethod(this, &GuiDocumentCommonInterface::OnCharInput);

				undoRedoProcessor->Setup(documentElement, documentComposition);
				ActiveHyperlinkChanged.SetAssociatedComposition(_sender->GetBoundsComposition());
				ActiveHyperlinkExecuted.SetAssociatedComposition(_sender->GetBoundsComposition());
				SelectionChanged.SetAssociatedComposition(_sender->GetBoundsComposition());
				UndoRedoChanged.SetAssociatedComposition(_sender->GetBoundsComposition());
				ModifiedChanged.SetAssociatedComposition(_sender->GetBoundsComposition());

				undoRedoProcessor->UndoRedoChanged.Add(this, &GuiDocumentCommonInterface::InvokeUndoRedoChanged);
				undoRedoProcessor->ModifiedChanged.Add(this, &GuiDocumentCommonInterface::InvokeModifiedChanged);
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
					FOREACH(Ptr<DocumentHyperlinkRun>, run, activeHyperlinks->hyperlinks)
					{
						run->styleName = activate ? run->activeStyleName : run->normalStyleName;
					}
					documentElement->NotifyParagraphUpdated(activeHyperlinks->row, 1, 1, false);
				}
			}

			void GuiDocumentCommonInterface::AddShortcutCommand(vint key, const Func<void()>& eventHandler)
			{
				IGuiShortcutKeyItem* item=internalShortcutKeyManager->CreateShortcut(true, false, false, key);
				item->Executed.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
				{
					eventHandler();
				});
			}

			void GuiDocumentCommonInterface::EditTextInternal(TextPos begin, TextPos end, const Func<void(TextPos, TextPos, vint&, vint&)>& editor)
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
					documentControl->TextChanged.Execute(documentControl->GetNotifyEventArguments());
					UpdateCaretPoint();
					SelectionChanged.Execute(documentControl->GetNotifyEventArguments());

					// save run after editing
					Ptr<DocumentModel> inputModel=documentElement->GetDocument()->CopyDocument(begin, caret, true);

					// submit redo-undo
					GuiDocumentUndoRedoProcessor::ReplaceModelStruct arguments;
					arguments.originalStart=begin;
					arguments.originalEnd=end;
					arguments.originalModel=originalModel;
					arguments.inputStart=begin;
					arguments.inputEnd=caret;
					arguments.inputModel=inputModel;
					undoRedoProcessor->OnReplaceModel(arguments);
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
				document->MergeDefaultFont(documentControl->GetFont());
				if (baselineDocument)
				{
					document->MergeBaselineStyles(baselineDocument);
				}
			}

			void GuiDocumentCommonInterface::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				auto document = documentElement->GetDocument();
				MergeBaselineAndDefaultFont(document);
				documentElement->SetDocument(document);
			}

			void GuiDocumentCommonInterface::OnCaretNotify(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(documentControl->GetVisuallyEnabled())
				{
					if(editMode!=ViewOnly)
					{
						documentElement->SetCaretVisible(!documentElement->GetCaretVisible());
					}
				}
			}

			void GuiDocumentCommonInterface::OnGotFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(documentControl->GetVisuallyEnabled())
				{
					if(editMode!=ViewOnly)
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
				if(documentControl->GetVisuallyEnabled())
				{
					if(editMode!=ViewOnly)
					{
						if(ProcessKey(arguments.code, arguments.shift, arguments.ctrl))
						{
							arguments.handled=true;
						}
					}
				}
			}

			void GuiDocumentCommonInterface::OnCharInput(compositions::GuiGraphicsComposition* sender, compositions::GuiCharEventArgs& arguments)
			{
				if(documentControl->GetVisuallyEnabled())
				{
					if(editMode==Editable && arguments.code!=VKEY_ESCAPE && arguments.code!=VKEY_BACK && arguments.code!=VKEY_RETURN && !arguments.ctrl)
					{
						Array<WString> text(1);
						text[0]=WString(arguments.code);
						EditText(documentElement->GetCaretBegin(), documentElement->GetCaretEnd(), documentElement->IsCaretEndPreferFrontSide(), text);
					}
				}
			}

			void GuiDocumentCommonInterface::OnMouseMove(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(documentControl->GetVisuallyEnabled())
				{
					switch(editMode)
					{
					case ViewOnly:
						{
							auto package = documentElement->GetHyperlinkFromPoint({ arguments.x, arguments.y });
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
								documentComposition->SetAssociatedCursor(cursor);
							}
							else
							{
								documentComposition->SetAssociatedCursor(nullptr);
							}
						}
						break;
					case Selectable:
					case Editable:
						if(dragging)
						{
							TextPos caret=documentElement->CalculateCaretFromPoint(Point(arguments.x, arguments.y));
							TextPos oldCaret=documentElement->GetCaretBegin();
							Move(caret, true, (oldCaret==caret?documentElement->IsCaretEndPreferFrontSide():caret<oldCaret));
						}
						break;
					}
				}
			}

			void GuiDocumentCommonInterface::OnMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(documentControl->GetVisuallyEnabled())
				{
					documentControl->SetFocus();
					switch(editMode)
					{
					case ViewOnly:
						SetActiveHyperlink(documentElement->GetHyperlinkFromPoint({ arguments.x, arguments.y }));
						break;
					case Selectable:
					case Editable:
						{
							TextPos caret=documentElement->CalculateCaretFromPoint(Point(arguments.x, arguments.y));
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
				if(documentControl->GetVisuallyEnabled())
				{
					dragging=false;
					switch(editMode)
					{
					case ViewOnly:
						{
							auto package = documentElement->GetHyperlinkFromPoint({ arguments.x, arguments.y });
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
				SetActiveHyperlink(0);
			}

			Point GuiDocumentCommonInterface::GetDocumentViewPosition()
			{
				return Point(0, 0);
			}

			void GuiDocumentCommonInterface::EnsureRectVisible(Rect bounds)
			{
			}

			//================ callback

			void GuiDocumentCommonInterface::OnStartRender()
			{
				FOREACH(Ptr<GuiDocumentItem>, item, documentItems.Values())
				{
					item->visible = false;
				}
			}

			void GuiDocumentCommonInterface::OnFinishRender()
			{
				FOREACH(Ptr<GuiDocumentItem>, item, documentItems.Values())
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
					auto size = item->container->GetBounds().GetSize();
					item->container->SetBounds(Rect(location.LeftTop(), Size(0, 0)));
					item->visible = true;
					return size;
				}
				return Size();
			}

			//================ basic

			GuiDocumentCommonInterface::GuiDocumentCommonInterface(Ptr<DocumentModel> _baselineDocument, Color _caretColor)
				:baselineDocument(_baselineDocument)
				,caretColor(_caretColor)
				,documentElement(0)
				,documentComposition(0)
				,dragging(false)
				,editMode(ViewOnly)
				,documentControl(0)
			{
				undoRedoProcessor=new GuiDocumentUndoRedoProcessor;

				internalShortcutKeyManager=new GuiShortcutKeyManager;
				AddShortcutCommand('Z', Func<bool()>(this, &GuiDocumentCommonInterface::Undo));
				AddShortcutCommand('Y', Func<bool()>(this, &GuiDocumentCommonInterface::Redo));
				AddShortcutCommand('A', Func<void()>(this, &GuiDocumentCommonInterface::SelectAll));
				AddShortcutCommand('X', Func<bool()>(this, &GuiDocumentCommonInterface::Cut));
				AddShortcutCommand('C', Func<bool()>(this, &GuiDocumentCommonInterface::Copy));
				AddShortcutCommand('V', Func<bool()>(this, &GuiDocumentCommonInterface::Paste));
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
				SetActiveHyperlink(0);
				ClearUndoRedo();
				NotifyModificationSaved();
				if(value->paragraphs.Count()==0)
				{
					value->paragraphs.Add(new DocumentParagraphRun);
				}

				MergeBaselineAndDefaultFont(value);
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

			void GuiDocumentCommonInterface::NotifyParagraphUpdated(vint index, vint oldCount, vint newCount, bool updatedText)
			{
				documentElement->NotifyParagraphUpdated(index, oldCount, newCount, updatedText);
			}

			void GuiDocumentCommonInterface::EditRun(TextPos begin, TextPos end, Ptr<DocumentModel> model, bool copy)
			{
				EditTextInternal(begin, end, [=](TextPos begin, TextPos end, vint& paragraphCount, vint& lastParagraphLength)
				{
					documentElement->EditRun(begin, end, model, copy);
					paragraphCount=model->paragraphs.Count();
					lastParagraphLength=paragraphCount==0?0:model->paragraphs[paragraphCount-1]->GetText(false).Length();
				});
			}

			void GuiDocumentCommonInterface::EditText(TextPos begin, TextPos end, bool frontSide, const collections::Array<WString>& text)
			{
				EditTextInternal(begin, end, [=, &text](TextPos begin, TextPos end, vint& paragraphCount, vint& lastParagraphLength)
				{
					documentElement->EditText(begin, end, frontSide, text);
					paragraphCount=text.Count();
					lastParagraphLength=paragraphCount==0?0:text[paragraphCount-1].Length();
				});
			}

			void GuiDocumentCommonInterface::EditStyle(TextPos begin, TextPos end, Ptr<DocumentStyleProperties> style)
			{
				EditStyleInternal(begin, end, [=](TextPos begin, TextPos end)
				{
					documentElement->EditStyle(begin, end, style);
				});
			}

			void GuiDocumentCommonInterface::EditImage(TextPos begin, TextPos end, Ptr<GuiImageData> image)
			{
				EditTextInternal(begin, end, [=](TextPos begin, TextPos end, vint& paragraphCount, vint& lastParagraphLength)
				{
					documentElement->EditImage(begin, end, image);
					paragraphCount=1;
					lastParagraphLength=wcslen(DocumentImageRun::RepresentationText);
				});
			}

			void GuiDocumentCommonInterface::EditHyperlink(vint paragraphIndex, vint begin, vint end, const WString& reference, const WString& normalStyleName, const WString& activeStyleName)
			{
				EditStyleInternal(TextPos(paragraphIndex, begin), TextPos(paragraphIndex, end), [=](TextPos begin, TextPos end)
				{
					documentElement->EditHyperlink(begin.row, begin.column, end.column, reference, normalStyleName, activeStyleName);
				});
			}

			void GuiDocumentCommonInterface::RemoveHyperlink(vint paragraphIndex, vint begin, vint end)
			{
				EditStyleInternal(TextPos(paragraphIndex, begin), TextPos(paragraphIndex, end), [=](TextPos begin, TextPos end)
				{
					documentElement->RemoveHyperlink(begin.row, begin.column, end.column);
				});
			}

			void GuiDocumentCommonInterface::EditStyleName(TextPos begin, TextPos end, const WString& styleName)
			{
				EditStyleInternal(begin, end, [=](TextPos begin, TextPos end)
				{
					documentElement->EditStyleName(begin, end, styleName);
				});
			}

			void GuiDocumentCommonInterface::RemoveStyleName(TextPos begin, TextPos end)
			{
				EditStyleInternal(begin, end, [=](TextPos begin, TextPos end)
				{
					documentElement->RemoveStyleName(begin, end);
				});
			}

			void GuiDocumentCommonInterface::RenameStyle(const WString& oldStyleName, const WString& newStyleName)
			{
				documentElement->RenameStyle(oldStyleName, newStyleName);

				// submit redo-undo
				GuiDocumentUndoRedoProcessor::RenameStyleStruct arguments;
				arguments.oldStyleName=oldStyleName;
				arguments.newStyleName=newStyleName;
				undoRedoProcessor->OnRenameStyle(arguments);
			}

			void GuiDocumentCommonInterface::ClearStyle(TextPos begin, TextPos end)
			{
				EditStyleInternal(begin, end, [=](TextPos begin, TextPos end)
				{
					documentElement->ClearStyle(begin, end);
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
					Ptr<GuiDocumentUndoRedoProcessor::SetAlignmentStruct> arguments = new GuiDocumentUndoRedoProcessor::SetAlignmentStruct;
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

			GuiDocumentCommonInterface::EditMode GuiDocumentCommonInterface::GetEditMode()
			{
				return editMode;
			}

			void GuiDocumentCommonInterface::SetEditMode(EditMode value)
			{
				if(activeHyperlinks)
				{
					SetActiveHyperlink(nullptr);
				}

				editMode=value;
				if(editMode==ViewOnly)
				{
					documentComposition->SetAssociatedCursor(0);
				}
				else
				{
					INativeCursor* cursor=GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::IBeam);
					documentComposition->SetAssociatedCursor(cursor);
				}
			}

			//================ selection operations

			void GuiDocumentCommonInterface::SelectAll()
			{
				vint lastIndex=documentElement->GetDocument()->paragraphs.Count()-1;
				Ptr<DocumentParagraphRun> lastParagraph=documentElement->GetDocument()->paragraphs[lastIndex];

				TextPos begin(0, 0);
				TextPos end(lastIndex, lastParagraph->GetText(false).Length());
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
				return model->GetText(true);
			}

			void GuiDocumentCommonInterface::SetSelectionText(const WString& value)
			{
				List<WString> paragraphs;
				{
					stream::StringReader reader(value);
					WString paragraph;
					bool empty=true;

					while(!reader.IsEnd())
					{
						WString line=reader.ReadLine();
						if(empty)
						{
							paragraph+=line;
							empty=false;
						}
						else if(line!=L"")
						{
							paragraph+=L"\r\n"+line;
						}
						else
						{
							paragraphs.Add(paragraph);
							paragraph=L"";
							empty=true;
						}
					}

					if(!empty)
					{
						paragraphs.Add(paragraph);
					}
				}

				TextPos begin=documentElement->GetCaretBegin();
				TextPos end=documentElement->GetCaretEnd();
				if(begin>end)
				{
					TextPos temp=begin;
					begin=end;
					end=temp;
				}

				Array<WString> text;
				CopyFrom(text, paragraphs);
				EditText(begin, end, documentElement->IsCaretEndPreferFrontSide(), text);
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
				TextPos begin=documentElement->GetCaretBegin();
				TextPos end=documentElement->GetCaretEnd();
				if(begin>end)
				{
					TextPos temp=begin;
					begin=end;
					end=temp;
				}

				EditRun(begin, end, value, true);
			}

			//================ clipboard operations

			bool GuiDocumentCommonInterface::CanCut()
			{
				return editMode==Editable && documentElement->GetCaretBegin()!=documentElement->GetCaretEnd();
			}

			bool GuiDocumentCommonInterface::CanCopy()
			{
				return documentElement->GetCaretBegin()!=documentElement->GetCaretEnd();
			}

			bool GuiDocumentCommonInterface::CanPaste()
			{
				return editMode==Editable && GetCurrentController()->ClipboardService()->ContainsText();
			}

			bool GuiDocumentCommonInterface::Cut()
			{
				if(CanCut())
				{
					GetCurrentController()->ClipboardService()->SetText(GetSelectionText());
					SetSelectionText(L"");
					return true;
				}
				else
				{
					return false;
				}
			}

			bool GuiDocumentCommonInterface::Copy()
			{
				if(CanCopy())
				{
					GetCurrentController()->ClipboardService()->SetText(GetSelectionText());
					return true;
				}
				else
				{
					return false;
				}
			}

			bool GuiDocumentCommonInterface::Paste()
			{
				if(CanPaste())
				{
					SetSelectionText(GetCurrentController()->ClipboardService()->GetText());
					return true;
				}
				else
				{
					return false;
				}
			}

			//================ undo redo control

			bool GuiDocumentCommonInterface::CanUndo()
			{
				return editMode==Editable && undoRedoProcessor->CanUndo();
			}

			bool GuiDocumentCommonInterface::CanRedo()
			{
				return editMode==Editable && undoRedoProcessor->CanRedo();
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

/***********************************************************************
GuiDocumentViewer
***********************************************************************/

			Point GuiDocumentViewer::GetDocumentViewPosition()
			{
				return GetViewBounds().LeftTop();
			}

			void GuiDocumentViewer::EnsureRectVisible(Rect bounds)
			{
				Rect viewBounds=GetViewBounds();
				vint offset=0;
				if(bounds.y1<viewBounds.y1)
				{
					offset=bounds.y1-viewBounds.y1;
				}
				else if(bounds.y2>viewBounds.y2)
				{
					offset=bounds.y2-viewBounds.y2;
				}

				GetVerticalScroll()->SetPosition(viewBounds.y1+offset);
			}

			GuiDocumentViewer::GuiDocumentViewer(theme::ThemeName themeName)
				:GuiScrollContainer(themeName)
				, GuiDocumentCommonInterface(_controlTemplate->GetBaselineDocument(), _controlTemplate->GetCaretColor())
			{
				SetExtendToFullWidth(true);
				SetHorizontalAlwaysVisible(false);
				SetFocusableComposition(GetBoundsComposition());
				InstallDocumentViewer(this, GetContainerComposition());
				SetDocument(new DocumentModel);
			}

			GuiDocumentViewer::~GuiDocumentViewer()
			{
			}

			const WString& GuiDocumentViewer::GetText()
			{
				text=documentElement->GetDocument()->GetText(true);
				return text;
			}

			void GuiDocumentViewer::SetText(const WString& value)
			{
				SelectAll();
				SetSelectionText(value);
			}

/***********************************************************************
GuiDocumentLabel
***********************************************************************/

			GuiDocumentLabel::GuiDocumentLabel(theme::ThemeName themeName)
				:GuiControl(themeName)
				, GuiDocumentCommonInterface(_controlTemplate->GetBaselineDocument(), _controlTemplate->GetCaretColor())
			{
				GetContainerComposition()->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				SetFocusableComposition(GetBoundsComposition());
				InstallDocumentViewer(this, GetContainerComposition());
				SetDocument(new DocumentModel);
			}

			GuiDocumentLabel::~GuiDocumentLabel()
			{
			}

			const WString& GuiDocumentLabel::GetText()
			{
				text=documentElement->GetDocument()->GetText(true);
				return text;
			}

			void GuiDocumentLabel::SetText(const WString& value)
			{
				SelectAll();
				SetSelectionText(value);
			}
		}
	}
}
