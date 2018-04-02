#include "GuiTextUndoRedo.h"
#include "../GuiTextCommonInterface.h"
#include "../GuiDocumentViewer.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace elements::text;
			using namespace compositions;

/***********************************************************************
GuiGeneralUndoRedoProcessor
***********************************************************************/

			GuiGeneralUndoRedoProcessor::GuiGeneralUndoRedoProcessor()
				:firstFutureStep(0)
				,savedStep(0)
				,performingUndoRedo(false)
			{
			}

			GuiGeneralUndoRedoProcessor::~GuiGeneralUndoRedoProcessor()
			{
			}

			void GuiGeneralUndoRedoProcessor::PushStep(Ptr<IEditStep> step)
			{
				if(!performingUndoRedo)
				{
					if(firstFutureStep<savedStep)
					{
						savedStep=-1;
					}

					vint count=steps.Count()-firstFutureStep;
					if(count>0)
					{
						steps.RemoveRange(firstFutureStep, count);
					}
				
					steps.Add(step);
					firstFutureStep=steps.Count();
					UndoRedoChanged();
					ModifiedChanged();
				}
			}

			bool GuiGeneralUndoRedoProcessor::CanUndo()
			{
				return firstFutureStep>0;
			}

			bool GuiGeneralUndoRedoProcessor::CanRedo()
			{
				return steps.Count()>firstFutureStep;
			}

			void GuiGeneralUndoRedoProcessor::ClearUndoRedo()
			{
				if(!performingUndoRedo)
				{
					steps.Clear();
					firstFutureStep=0;
					savedStep=0;
				}
			}

			bool GuiGeneralUndoRedoProcessor::GetModified()
			{
				return firstFutureStep!=savedStep;
			}

			void GuiGeneralUndoRedoProcessor::NotifyModificationSaved()
			{
				if(!performingUndoRedo)
				{
					savedStep=firstFutureStep;
					ModifiedChanged();
				}
			}

			bool GuiGeneralUndoRedoProcessor::Undo()
			{
				if(!CanUndo()) return false;
				performingUndoRedo=true;
				firstFutureStep--;
				steps[firstFutureStep]->Undo();
				performingUndoRedo=false;
				UndoRedoChanged();
				ModifiedChanged();
				return true;
			}

			bool GuiGeneralUndoRedoProcessor::Redo()
			{
				if(!CanRedo()) return false;
				performingUndoRedo=true;
				firstFutureStep++;
				steps[firstFutureStep-1]->Redo();
				performingUndoRedo=false;
				UndoRedoChanged();
				ModifiedChanged();
				return true;
			}

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
				Ptr<EditStep> step=new EditStep;
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

/***********************************************************************
GuiDocumentUndoRedoProcessor::ReplaceModelStep
***********************************************************************/

			void GuiDocumentUndoRedoProcessor::ReplaceModelStep::Undo()
			{
				GuiDocumentCommonInterface* ci=dynamic_cast<GuiDocumentCommonInterface*>(processor->ownerComposition->GetRelatedControl());
				if(ci)
				{
					ci->EditRun(arguments.inputStart, arguments.inputEnd, arguments.originalModel, true);
					ci->SetCaret(arguments.originalStart, arguments.originalEnd);
				}
			}

			void GuiDocumentUndoRedoProcessor::ReplaceModelStep::Redo()
			{
				GuiDocumentCommonInterface* ci=dynamic_cast<GuiDocumentCommonInterface*>(processor->ownerComposition->GetRelatedControl());
				if(ci)
				{
					ci->EditRun(arguments.originalStart, arguments.originalEnd, arguments.inputModel, true);
					ci->SetCaret(arguments.inputStart, arguments.inputEnd);
				}
			}

/***********************************************************************
GuiDocumentUndoRedoProcessor::RenameStyleStep
***********************************************************************/

			void GuiDocumentUndoRedoProcessor::RenameStyleStep::Undo()
			{
				GuiDocumentCommonInterface* ci=dynamic_cast<GuiDocumentCommonInterface*>(processor->ownerComposition->GetRelatedControl());
				if(ci)
				{
					ci->RenameStyle(arguments.newStyleName, arguments.oldStyleName);
				}
			}

			void GuiDocumentUndoRedoProcessor::RenameStyleStep::Redo()
			{
				GuiDocumentCommonInterface* ci=dynamic_cast<GuiDocumentCommonInterface*>(processor->ownerComposition->GetRelatedControl());
				if(ci)
				{
					ci->RenameStyle(arguments.oldStyleName, arguments.newStyleName);
				}
			}

/***********************************************************************
GuiDocumentUndoRedoProcessor::SetAlignmentStep
***********************************************************************/

			void GuiDocumentUndoRedoProcessor::SetAlignmentStep::Undo()
			{
				GuiDocumentCommonInterface* ci=dynamic_cast<GuiDocumentCommonInterface*>(processor->ownerComposition->GetRelatedControl());
				if(ci)
				{
					ci->SetParagraphAlignments(TextPos(arguments->start, 0), TextPos(arguments->end, 0), arguments->originalAlignments);
				}
			}

			void GuiDocumentUndoRedoProcessor::SetAlignmentStep::Redo()
			{
				GuiDocumentCommonInterface* ci=dynamic_cast<GuiDocumentCommonInterface*>(processor->ownerComposition->GetRelatedControl());
				if(ci)
				{
					ci->SetParagraphAlignments(TextPos(arguments->start, 0), TextPos(arguments->end, 0), arguments->inputAlignments);
				}
			}

/***********************************************************************
GuiDocumentUndoRedoProcessor
***********************************************************************/

			GuiDocumentUndoRedoProcessor::GuiDocumentUndoRedoProcessor()
				:element(0)
				,ownerComposition(0)
			{
			}

			GuiDocumentUndoRedoProcessor::~GuiDocumentUndoRedoProcessor()
			{
			}

			void GuiDocumentUndoRedoProcessor::Setup(elements::GuiDocumentElement* _element, compositions::GuiGraphicsComposition* _ownerComposition)
			{
				element=_element;
				ownerComposition=_ownerComposition;
			}

			void GuiDocumentUndoRedoProcessor::OnReplaceModel(const ReplaceModelStruct& arguments)
			{
				Ptr<ReplaceModelStep> step=new ReplaceModelStep;
				step->processor=this;
				step->arguments=arguments;
				PushStep(step);
			}

			void GuiDocumentUndoRedoProcessor::OnRenameStyle(const RenameStyleStruct& arguments)
			{
				Ptr<RenameStyleStep> step=new RenameStyleStep;
				step->processor=this;
				step->arguments=arguments;
				PushStep(step);
			}

			void GuiDocumentUndoRedoProcessor::OnSetAlignment(Ptr<SetAlignmentStruct> arguments)
			{
				Ptr<SetAlignmentStep> step=new SetAlignmentStep;
				step->processor=this;
				step->arguments=arguments;
				PushStep(step);
			}
		}
	}
}