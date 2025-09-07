/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITEXTUNDOREDO
#define VCZH_PRESENTATION_CONTROLS_GUITEXTUNDOREDO

#include "GuiTextGeneralOperations.h"
#include "../../../GraphicsElement/GuiGraphicsDocumentElement.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiTextBoxCommonInterface;

/***********************************************************************
Undo Redo
***********************************************************************/

			class GuiGeneralUndoRedoProcessor : public Object
			{
			protected:
				class IEditStep : public Interface
				{
				public:
					virtual void							Undo()=0;
					virtual void							Redo()=0;
				};
				friend class collections::ArrayBase<Ptr<IEditStep>>;

			protected:
				collections::List<Ptr<IEditStep>>			steps;
				vint										firstFutureStep;
				vint										savedStep;
				bool										performingUndoRedo;

				void										PushStep(Ptr<IEditStep> step);
			public:
				GuiGeneralUndoRedoProcessor();
				~GuiGeneralUndoRedoProcessor();

				Event<void()>								UndoRedoChanged;
				Event<void()>								ModifiedChanged;

				bool										CanUndo();
				bool										CanRedo();
				void										ClearUndoRedo();
				bool										GetModified();
				void										NotifyModificationSaved();
				bool										Undo();
				bool										Redo();
			};

/***********************************************************************
Undo Redo (Text)
***********************************************************************/

			class GuiTextBoxUndoRedoProcessor : public GuiGeneralUndoRedoProcessor, public ICommonTextEditCallback
			{
			protected:
				class EditStep : public Object, public IEditStep
				{
				public:
					GuiTextBoxUndoRedoProcessor*			processor;
					TextEditNotifyStruct					arguments;
					
					void									Undo();
					void									Redo();
				};

				compositions::GuiGraphicsComposition*		ownerComposition;
			public:
				GuiTextBoxUndoRedoProcessor();
				~GuiTextBoxUndoRedoProcessor();

				void										Attach(elements::GuiColorizedTextElement* element, SpinLock& elementModifyLock, compositions::GuiGraphicsComposition* _ownerComposition, vuint editVersion)override;
				void										Detach()override;
				void										TextEditPreview(TextEditPreviewStruct& arguments)override;
				void										TextEditNotify(const TextEditNotifyStruct& arguments)override;
				void										TextCaretChanged(const TextCaretChangedStruct& arguments)override;
				void										TextEditFinished(vuint editVersion)override;
			};

/***********************************************************************
Undo Redo (Document)
***********************************************************************/

			class GuiDocumentUndoRedoProcessor : public GuiGeneralUndoRedoProcessor
			{
			public:
				struct ReplaceModelStruct
				{
					TextPos									originalStart;
					TextPos									originalEnd;
					Ptr<DocumentModel>						originalModel;
					TextPos									inputStart;
					TextPos									inputEnd;
					Ptr<DocumentModel>						inputModel;

					ReplaceModelStruct()
					{
					}
				};

				struct RenameStyleStruct
				{
					WString									oldStyleName;
					WString									newStyleName;

					RenameStyleStruct()
					{
					}
				};

				struct SetAlignmentStruct
				{
					vint									start;
					vint									end;
					collections::Array<Nullable<Alignment>>	originalAlignments;
					collections::Array<Nullable<Alignment>>	inputAlignments;
				};

			protected:
				elements::GuiDocumentElement*				element;
				compositions::GuiGraphicsComposition*		ownerComposition;
				
				class ReplaceModelStep : public Object, public IEditStep
				{
				public:
					GuiDocumentUndoRedoProcessor*			processor;
					ReplaceModelStruct						arguments;
					
					void									Undo();
					void									Redo();
				};

				class RenameStyleStep : public Object, public IEditStep
				{
				public:
					GuiDocumentUndoRedoProcessor*			processor;
					RenameStyleStruct						arguments;
					
					void									Undo();
					void									Redo();
				};

				class SetAlignmentStep : public Object, public IEditStep
				{
				public:
					GuiDocumentUndoRedoProcessor*			processor;
					Ptr<SetAlignmentStruct>					arguments;
					
					void									Undo();
					void									Redo();
				};
			public:

				GuiDocumentUndoRedoProcessor();
				~GuiDocumentUndoRedoProcessor();

				void										Setup(elements::GuiDocumentElement* _element, compositions::GuiGraphicsComposition* _ownerComposition);
				void										OnReplaceModel(const ReplaceModelStruct& arguments);
				void										OnRenameStyle(const RenameStyleStruct& arguments);
				void										OnSetAlignment(Ptr<SetAlignmentStruct> arguments);
			};
		}
	}
}

#endif