/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteController

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTEGRAPHICS_DOCUMENT
#define VCZH_PRESENTATION_GUIREMOTEGRAPHICS_DOCUMENT

#include "../../GraphicsElement/GuiGraphicsResourceManager.h"

namespace vl::presentation
{
	class GuiRemoteController;
}

namespace vl::presentation::elements
{
	class GuiRemoteGraphicsResourceManager;
	class GuiRemoteGraphicsRenderTarget;

/***********************************************************************
GuiRemoteGraphicsParagraph
***********************************************************************/

	class GuiRemoteGraphicsParagraph : public Object, public IGuiGraphicsParagraph
	{
	protected:
		WString									text;
		GuiRemoteController*					remote = nullptr;
		GuiRemoteGraphicsResourceManager*		resourceManager = nullptr;
		GuiRemoteGraphicsRenderTarget*			renderTarget = nullptr;
		IGuiGraphicsParagraphCallback*			callback = nullptr;

	public:
		GuiRemoteGraphicsParagraph(const WString& _text, GuiRemoteController* _remote, GuiRemoteGraphicsResourceManager* _resourceManager, GuiRemoteGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _callback);
		~GuiRemoteGraphicsParagraph();

		// =============================================================
		// IGuiGraphicsParagraph
		// =============================================================

		IGuiGraphicsLayoutProvider*						GetProvider() override;
		IGuiGraphicsRenderTarget*						GetRenderTarget() override;
		bool											GetWrapLine() override;
		void											SetWrapLine(bool value) override;
		vint											GetMaxWidth() override;
		void											SetMaxWidth(vint value) override;
		Alignment										GetParagraphAlignment() override;
		void											SetParagraphAlignment(Alignment value) override;

		bool											SetFont(vint start, vint length, const WString& value) override;
		bool											SetSize(vint start, vint length, vint value) override;
		bool											SetStyle(vint start, vint length, TextStyle value) override;
		bool											SetColor(vint start, vint length, Color value) override;
		bool											SetBackgroundColor(vint start, vint length, Color value) override;
		bool											SetInlineObject(vint start, vint length, const InlineObjectProperties& properties) override;
		bool											ResetInlineObject(vint start, vint length) override;

		Size											GetSize() override;
		bool											OpenCaret(vint caret, Color color, bool frontSide) override;
		bool											CloseCaret() override;
		void											Render(Rect bounds) override;

		vint											GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide) override;
		Rect											GetCaretBounds(vint caret, bool frontSide) override;
		vint											GetCaretFromPoint(Point point) override;
		Nullable<InlineObjectProperties>				GetInlineObjectFromPoint(Point point, vint& start, vint& length) override;
		vint											GetNearestCaretFromTextPos(vint textPos, bool frontSide) override;
		bool											IsValidCaret(vint caret) override;
		bool											IsValidTextPos(vint textPos) override;
	};
}

#endif