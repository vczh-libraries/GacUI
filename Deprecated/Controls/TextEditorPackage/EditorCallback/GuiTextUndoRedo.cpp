
/***********************************************************************
GuiTextBoxUndoRedoProcessor::EditStep
***********************************************************************/

			void GuiTextBoxUndoRedoProcessor::EditStep::Undo()
			{
				GuiTextBoxCommonInterface* ci=dynamic_cast<GuiTextBoxCommonInterface*>(processor->ownerComposition->GetRelatedControl());
				if(ci)
				{
					ci->Select(arguments.inputStart, arguments.inputEnd);
					ci->SetSelectionText(arguments.originalText);
					ci->Select(arguments.originalStart, arguments.originalEnd);
				}
			}

			void GuiTextBoxUndoRedoProcessor::EditStep::Redo()
			{
				GuiTextBoxCommonInterface* ci=dynamic_cast<GuiTextBoxCommonInterface*>(processor->ownerComposition->GetRelatedControl());
				if(ci)
				{
					ci->Select(arguments.originalStart, arguments.originalEnd);
					ci->SetSelectionText(arguments.inputText);
					ci->Select(arguments.inputStart, arguments.inputEnd);
				}
			}

/***********************************************************************
GuiTextBoxUndoRedoProcessor
***********************************************************************/

			GuiTextBoxUndoRedoProcessor::GuiTextBoxUndoRedoProcessor()
				:ownerComposition(0)
			{
			}

			GuiTextBoxUndoRedoProcessor::~GuiTextBoxUndoRedoProcessor()
			{
			}

			void GuiTextBoxUndoRedoProcessor::Attach(elements::GuiColorizedTextElement* element, SpinLock& elementModifyLock, compositions::GuiGraphicsComposition* _ownerComposition, vuint editVersion)
			{
				ownerComposition=_ownerComposition;
			}

			void GuiTextBoxUndoRedoProcessor::Detach()
			{
				ClearUndoRedo();
			}

			void GuiTextBoxUndoRedoProcessor::TextEditPreview(TextEditPreviewStruct& arguments)
			{
			}

			void GuiTextBoxUndoRedoProcessor::TextEditNotify(const TextEditNotifyStruct& arguments)
			{
				auto step=Ptr(new EditStep);
				step->processor=this;
				step->arguments=arguments;
				PushStep(step);
			}

			void GuiTextBoxUndoRedoProcessor::TextCaretChanged(const TextCaretChangedStruct& arguments)
			{
			}

			void GuiTextBoxUndoRedoProcessor::TextEditFinished(vuint editVersion)
			{
			}