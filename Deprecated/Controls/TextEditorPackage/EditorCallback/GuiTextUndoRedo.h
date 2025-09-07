
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