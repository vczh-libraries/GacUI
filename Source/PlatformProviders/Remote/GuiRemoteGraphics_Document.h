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

	/// <summary>
	/// Updates style properties of a text run. Ranges will be splitted or merged accordingly.
	/// </summary>
	/// <returns>Returns true if the map is changed semantically.</returns>
	/// <param name="map">Current text runs</param>
	/// <param name="range">Range of the text run to update</param>
	/// <param name="propertyOverrides">Properties to override</param>
	extern bool AddTextRun(
		DocumentTextRunPropertyMap& map,
		CaretRange range,
		const DocumentTextRunPropertyOverrides& propertyOverrides);

	/// <summary>
	/// Adds an inline object run. 
	/// The function succeeds if the target range has no inline object or has an exactly same inline object.
	/// Inline object runs cannot be splitted or merged.
	/// To replace an inline object, call <see cref="ResetInlineObjectRun"/> to remove it first.
	/// </summary>
	/// <param name="map">Current inline object runs</param>
	/// <param name="range">Range of the inline object run to add</param>
	/// <param name="property">Properties of the inline object run</param>
	/// <returns>Returns true if the inline object run was added successfully.</returns>
	extern bool AddInlineObjectRun(
		DocumentInlineObjectRunPropertyMap& map,
		CaretRange range,
		const remoteprotocol::DocumentInlineObjectRunProperty& property);

	/// <summary>
	/// Removes an inline object run. The function succeeds if the target range exactly matches an existing inline object run.
	/// </summary>
	/// <param name="map">Current inline object runs</param>
	/// <param name="range">Range of the inline object run to remove</param>
	/// <returns>Returns true if the inline object run was removed successfully.</returns>
	extern bool ResetInlineObjectRun(
		DocumentInlineObjectRunPropertyMap& map,
		CaretRange range);

	/// <summary>
	/// Merge text runs and inline object runs into a single run map.
	/// Inline object runs has a higher priority, if a text run and inline object run overlapps, only unoverlapping part of the text run survives.
	/// </summary>
	/// <param name="textRuns">Current text runs</param>
	/// <param name="inlineObjectRuns">Current inline object runs</param>
	/// <param name="result">Resulting merged run map</param>
	extern void MergeRuns(
		const DocumentTextRunPropertyMap& textRuns,
		const DocumentInlineObjectRunPropertyMap& inlineObjectRuns,
		DocumentRunPropertyMap& result);

	/// <summary>
	/// Build diff from two run maps.
	/// </summary>
	/// <param name="oldRuns">Old run maps</param>
	/// <param name="newRuns">New run maps</param>
	/// <param name="result">Resulting diff</param>
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
		Ptr<collections::Dictionary<vint, Rect>>						cachedInlineObjectBounds;
		bool															needUpdate = true;
		bool															needUpdateCaretBoundsCache = true;
		vint															id = -1;
		vuint64_t														lastRenderedBatchId = 0;

		collections::Array<Rect>										caretBoundsFrontSide, caretBoundsBackSide;

	public:
		GuiRemoteGraphicsParagraph(const WString& _text, GuiRemoteController* _remote, GuiRemoteGraphicsResourceManager* _resourceManager, GuiRemoteGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _callback);
		~GuiRemoteGraphicsParagraph();

		vint											GetParagraphId() const;

	protected:
		vint											NativeTextPosToRemoteTextPos(vint textPos);
		vint											RemoteTextPosToNativeTextPos(vint textPos);
		bool											TryBuildCaretRange(vint start, vint length, CaretRange& range);
		void											ResetCaretBoundsCache();

	public:
		bool											EnsureRemoteParagraphSynced();
		void											MarkParagraphDirty(bool invalidateSize, bool invalidateCaretBoundsCache);

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
		bool											EnableCaret(vint caret, Color color, bool frontSide) override;
		void											DisableCaret() override;
		bool											BlinkCaret() override;
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
