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
#include "Protocol/Generated/GuiRemoteProtocolSchema.h"

namespace vl::presentation
{
	class GuiRemoteController;
}

namespace vl::presentation::elements
{
/***********************************************************************
DiffRuns
***********************************************************************/

	struct CaretRange
	{
		vint caretBegin = 0;
		vint caretEnd = 0;

		auto operator<=>(const CaretRange&) const = default;
	};

	struct DocumentTextRunPropertyOverrides
	{
		Nullable<Color> textColor;
		Nullable<Color> backgroundColor;
		Nullable<WString> fontFamily;
		Nullable<vint> size;
		Nullable<IGuiGraphicsParagraph::TextStyle> textStyle;
	};

	using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, DocumentTextRunPropertyOverrides>;
	using DocumentInlineObjectRunPropertyMap = collections::Dictionary<CaretRange, remoteprotocol::DocumentInlineObjectRunProperty>;
	using DocumentRunPropertyMap = collections::Dictionary<CaretRange, remoteprotocol::DocumentRunProperty>;

	extern void AddTextRun(
		DocumentTextRunPropertyMap& map,
		CaretRange range,
		const DocumentTextRunPropertyOverrides& propertyOverrides);

	extern bool AddInlineObjectRun(
		DocumentInlineObjectRunPropertyMap& map,
		CaretRange range,
		const remoteprotocol::DocumentInlineObjectRunProperty& property);

	extern bool ResetInlineObjectRun(
		DocumentInlineObjectRunPropertyMap& map,
		CaretRange range);

	extern void MergeRuns(
		const DocumentTextRunPropertyMap& textRuns,
		const DocumentInlineObjectRunPropertyMap& inlineObjectRuns,
		DocumentRunPropertyMap& result);

	extern void DiffRuns(
		const DocumentRunPropertyMap& oldRuns,
		const DocumentRunPropertyMap& newRuns,
		remoteprotocol::ElementDesc_DocumentParagraph& result);

/***********************************************************************
GuiRemoteGraphicsParagraph
***********************************************************************/

	class GuiRemoteGraphicsResourceManager;
	class GuiRemoteGraphicsRenderTarget;

	class GuiRemoteGraphicsParagraph : public Object, public IGuiGraphicsParagraph
	{
	protected:
		WString															text;
		GuiRemoteController*											remote = nullptr;
		GuiRemoteGraphicsResourceManager*								resourceManager = nullptr;
		GuiRemoteGraphicsRenderTarget*									renderTarget = nullptr;
		IGuiGraphicsParagraphCallback*									callback = nullptr;

		DocumentTextRunPropertyMap										textRuns;
		DocumentInlineObjectRunPropertyMap								inlineObjectRuns;
		DocumentRunPropertyMap											stagedRuns;
		DocumentRunPropertyMap											committedRuns;
		collections::Dictionary<CaretRange, InlineObjectProperties>		inlineObjectProperties;
		bool															wrapLine = false;
		vint															maxWidth = -1;
		Alignment														paragraphAlignment = Alignment::Left;
		Size															cachedSize = Size(0, 0);
		bool															needUpdate = true;
		vint															id = -1;
		vuint64_t														lastRenderedBatchId = 0;

	public:
		GuiRemoteGraphicsParagraph(const WString& _text, GuiRemoteController* _remote, GuiRemoteGraphicsResourceManager* _resourceManager, GuiRemoteGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _callback);
		~GuiRemoteGraphicsParagraph();

		vint											GetParagraphId() const;

	protected:
		vint											NativeTextPosToRemoteTextPos(vint textPos);
		vint											RemoteTextPosToNativeTextPos(vint textPos);
		bool											TryBuildCaretRange(vint start, vint length, CaretRange& range);
		bool											EnsureRemoteParagraphSynced();
		void											MarkParagraphDirty(bool invalidateSize);

	public:
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