# Editor Controls

Document and Text controls are two kinds of editor controls for different purpose.

## GuiDocumentCommonInterface

**GuiDocumentCommonInterface**is the base class of all editor controls mentioned in this page. This class offers all available rendering and editing features for documents.

Sub classes of**GuiDocumentCommonInterface**implements different rendering and editing behaviors by offering different default controls via**GuiDocumentConfig**. These default values could be found in**GuiDocumentConfig**'s static methods:
- **GetDocumentLabelDefaultConfig()**
- **GetDocumentViewerDefaultConfig()**
- **GetSinglelineTextBoxDefaultConfig()**
- **GetMultilineTextBoxDefaultConfig()**

## Document Controls (default behavior)

Document controls are designed to display documents. It supports all kinds of Unicode-definded languages and characters.

A document could be rich formatted:
- Text and Images are organized in multiple paragraphs.
- Each paragraph has its own alignment option. multiple**CRLF**characters supported in a paragrpah.
- Different parts of text could be in different sizes, fonts and colors.
- Controls and compositions could be embedded into a document. This feature could be used to create a static table, a interactive list view or even a submittable form with verifiable inputs.

There are 3 document controls in GacUI:
- **\<DocumentViewer/\>**
- **\<DocumentTextBox/\>**
- **\<DocumentLabel/\>**

## Text Controls (default behavior)

Text controls are designed to display plain text.

Text control doesn't support rich format:
- Only characters are supported. Images, controls or compositions cannot be embedded into characters.
- Each line forms a paragraph. Multiple lines of text are separated by**CRLF**characters. You can insert**LF**but it will be converted to**CRLF**immediately.
- All characters are in the same size and font but different colors.

There are 2 text controls in GacUI:
- **\<SinglelineTextBox/\>**
- **\<MultilineTextBox/\>**

## Mouse Operations

When**EditMode**is**Selectable**, the document cannot be edited by mouse operations or keyboard operations.

When**EditMode**is**ViewOnly**, moving a mouse could cause hyperlinks to visually changed, clicking a mouse could execute a hyperlink under the cursor.

**ActiveHyperlinkChanged**raises when the mouse cursor enter or leave a hyperlink.

When**EditMode**is**Selectable**or**Editable**, mouse operations could change selections.**SHIFT**could use to change selection with mouse clicking. Drag and drop is not supported yet at this version.

## Keyboard Operations

When**EditMode**is**Selectable**, the document cannot be edited by mouse operations or keyboard operations.

**UP**,**DOWN**,**LEFT**,**RIGHT**move the caret.

**HOME**and**END**move the caret to the beginning or the ending of the current line. If the caret is already there, it move to the beginning or the ending of the current paragraph.

When holding**SHIFT**and move the caret, it set the end of the selection to the updated caret position.

**BACK**and**DELETE**delete characters in different direction. If there is any selection, they deleted the selection.

## Caret Position

**(vl::)presentation::TextPos**struct represents a caret position in a document.**row**is the index of the paragraph,**column**is the index of the next character from the caret in the current paragraph.

If the caret is at the end of a paragraph,**column**equals to the total character number of the paragraph.

For left-to-right languages, the "next character" is the character at the right side of the caret.

For non-character objects, each of them count as one character.

The**CaretBegin**and**CaretEnd**properties represent the range of the selection. If the mouse dragging from right to left on a piece of text in a left-to-right language,**CaretBegin**is larger than**CaretEnd**.

**SetCaret**changes**CaretBegin**and**CaretEnd**at the same time. To cancel the selection and move the caret, just set these two properties to the same value.

**CalculateCaretFromPoint**returns the nearest caret at a position. The coordinate origin is the left-top corner of the document.

**GetCaretBounds**returns the position of a caret. It is a rectangle with**zero**width, the**y1**and**y2**components represent the position of the vertical caret line.

## Selection

The**SelectionText**and**SelectionModel**properties return the selection of the document as pure text or document fragment. Assigning them behaves as calling**EditRun**and**EditText**on the selection.

**SelectAll**set the selection to the whole document.

## Initialize with Document or Text

**LoadTextAndClearUndoRedo**or**LoadDocumentAndClearUndoRedo**are recommended to initialize or reset editor controls with desired content. These function load new content to the control with optimized performance and clear all undo and redo history. Undo and redo are available for future editing.

## Assigning Document and Document Items

Assign a**(vl::)presentation::DocumentModel**to the**Document**property updates the whole content in the control and move the caret to the beginning of the document.

For any document item placeholder in an document like:
```
<object name="SOMETHING"/>
```
A**\<DocumentItem/\>**with exactly the same name in any document editor controls embeds anything in this tag to the document at the placeholder:
```
<DocumentViewer>
  <DocumentItem Name="SOMETHING">
    <Button Text="SOMETHING"/>
  </DocumentItem>
</DocumentViewer>
```
Only one composition or one control could be the direct child tag in**\<DocumentItem/\>**, but it could have any number of child compositions or controls.

The size of the placeholder is the minimum size of the composition or the control. There could be multiple**\<DocumentItem/\>**in a document editor control. If any**\<object/\>**or**\<DocumentItem/\>**fails to match each other, it is ignored, but the**\<object/\>**still take a caret position of one character.

**AddDocumentItem**,**RemoveDocumentItem**and**GetDocumentItems**can be used before and after assigning a document to the**Document**property.

## Editing

All**Edit***methods takes two carets and the content and replace the specified range of the current document with the specified content. If the two carets are the same value, they insert instead of replace.

The**EditRun**insert or replace with a**(vl::)presentation::DocumentMode**object. If the**copy**argument is set to**true**, sub nodes will not be reused.

If sub nodes in**Document**is changed without calling any**Edit***method,**NotifyParagraphUpdated(index, oldCount, newCount)**and**ClearUndoRedo**must be called, which means paragraphcs from**index**-th to**index+oldCount-1**-th has been replaced by**newCount**paragraphs.

Obviously, if all changes happen in one paragraph, arguments**NotifyParagraphUpdated(index, 1, 1)**are expected.

### Selection, Editing Texts and Images

**EditText**replaces a part of the document with multiple paragraphcs of texts. if the**frontSide**argument is set to**true**, it uses the text style in the character before the specified range, otherwise it uses the one after.

**EditImage**replaces a part of the document with an image.

### Editing and Reacting to Hyperlinks

**EditHyperlink**converts a part of the document to a hyperlink in a single paragraph. The**reference**argument is an attribute that attach to the hyperlink, when the hyperlink is executed, the value of the reference will be passed to the**ActiveHyperlinkExecuted**event, or be returned from**GetActiveHyperlinkReference**when this hyperlink is activated.

**RemoveHyperlink**converts a part of the document to non-hyperlink in a single paragraph.

### Editing and Summarizing Formats

A combination of styles could be given a name and[stored in the document](../../../.././gacui/xmlres/tag_doc.md). Names of existing styles could be used in**EditStyleName**,**RemoveStyleName**or**RenameStyle**.

**EditStyle**changes styles to a part of the document. Components of the last argument in**(vl::)presentation::DocumentStyleProperties**have one-to-one matching to style tags in**\<Style/\>**or**\<Font/\>**:
- **face**:**\<face/\>**
- **size**:**\<size/\>**
- **color**:**\<color/\>**
- **backgroundColor**:**\<bkcolor/\>**
- **bold**:**\<b/\>**
- **italic**:**\<i/\>**
- **underline**:**\<u/\>**
- **strikeline**:**\<s/\>**
- **antialias**:**\<antialias/\>**
- **verticalAntialias**: not supported in GacUI XML Resource

**EditStyleName**changes styles to a part of the document by associating a style name to it.

**RemoveStyleName**changes styles to a part of the document by removing all associated style name to it.

**RenameStyle**changes the name of a style, it also updates affected associated style names to any part of the document.

**SummarizeStyle**checks if all characters in the specified range of the document has any common in styles. For example, if characters are all**italic**and not**underline**, but only some of them are**bold**,**italic**will be**true**,**bold**will be**null**, and**underline**will be**false**.

**SummarizeStyleName**checks if all characters in the specified range are associated with the same style name.**null**is returned when they are not, otherwise the style name will be returned.

### Editing and Summarizing Alignments

**SetParagraphAlignments**updates alignments on multiple paragraphs with different values at the same time.**SetParagraphAlignment**updates alignments on multiple paragraphs with a same value. Since alignment is an attribute of paragraph instead of characters, the specified range of the document doesn't need to be whole paragraphs.

**SummarizeParagraphAlignment**checks if all paragraphs in the specified range of the document have the same alignment configuration.**null**is returned when they are not, otherwise the alignment value will be returned.

Valid values for alignment is**Left**(default),**Right**and**Center**.**Top**and**Bottom**are not used here, using them results in undefined behavior.

## Clipboard

Return values from**CanCut**,**CanCopy**and**CanPaste**indicates whether**Cut**,**Copy**and**Paste**will be successful or not.

**Cut**,**Copy**and**Paste**apply on selections. Only non-empty selection could be**Cut**or**Copy**. Calling**Paste**with a non-empty selection replaces the selection with the content from the clipboard.

**CanPaste**also checks the content from the clipboard and see if it is a valid document fragment.

When a piece of document is cut or copied, following formats are written to the clipboard:
- The private format of GacUI document.
- RTF.
- HTML.
- Plain text representation of the document. In Windows, both ANSI and UNICODE formats are available.
- If exactly one image is selected, the image is also copied to the clipboard.

Acceptable content from the clipboard is:
- The private format of GacUI document.
- Plain text. In Windows, both ANSI and UNICODE formats are acceptable.
- If exactly one image is selected, the image is also copied to the clipboard.

## Undo and Redo

Return values from**CanUndo**and**CanRedo**indicates whether**Undo**and**Redo**will be successful or not.

**IMPORTANT**: Changing sub nodes in**Document**will mess up undo and redo, so**ClearUndoRedo**is always necessary after**NotifyParagraphUpdated**.

All other operations, including keyboard operations and all editing methods mentioned above, will be recorded and are all undo-able and redo-able.

**NotifyParagraphUpdated**marks the current version as "saved" and clear the previous mark.**GetModified**returns true when the current version is marked as "saved". Only one version will be marked as "saved".

