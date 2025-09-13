/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDOCUMENTCONFIG
#define VCZH_PRESENTATION_CONTROLS_GUIDOCUMENTCONFIG

#include "../../GuiTypes.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiDocumentConfig
***********************************************************************/
			
			/// <summary>Represents the edit mode.</summary>
			enum class GuiDocumentEditMode
			{
				/// <summary>View the rich text only.</summary>
				ViewOnly,
				/// <summary>The rich text is selectable.</summary>
				Selectable,
				/// <summary>The rich text is editable.</summary>
				Editable,
			};

			/// <summary>Represents the paragraph mode.</summary>
			enum class GuiDocumentParagraphMode
			{
				/// <summary>Only one paragraph is allowed, only one line in a paragraph is allowed.</summary>
				Singleline,
				/// <summary>Only one line in a paragraph is allowed.</summary>
				Multiline,
				/// <summary>No constraint.</summary>
				Paragraph,
			};

			/// <summary>Control of editing and rendering behavior.</summary>
			struct GuiDocumentConfig
			{
				/// <summary>For GuiDocumentLabel only. When it is true, or when wrapLine is true, or when paragraphMode is not Singleline, the control automatically expands to display all content.</summary>
				Nullable<bool>							autoExpand;
				/// <summary>When it is true, the defaut copy paste behavior ignores RTF format.</summary>
				Nullable<bool>							pasteAsPlainText;
				/// <summary>When it is true, document automatically wraps if the width of the control is not enough.</summary>
				Nullable<bool>							wrapLine;
				/// <summary>Control the paragraph and line behavior</summary>
				Nullable<GuiDocumentParagraphMode>		paragraphMode;
				/// <summary>Insert the space of a default font between paragraphs.</summary>
				Nullable<bool>							paragraphPadding;
				/// <summary>When it is true:
				///  double CrLf will be used between paragraphs, when the document converts to plain text.
				///  only double CrLf will be recognized as paragraph breaks, when the document converts from plain text.
				/// </summary>
				Nullable<bool>							doubleLineBreaksBetweenParagraph;
				/// <summary>When it is true, when removing a line break from a document due to paragraphMode, insert a extra space.</summary>
				Nullable<bool>							spaceForFlattenedLineBreak;
				/// <summary>Delete <see cref="IGuiGraphicsParagraph"/> cache immediately when it is scrolled out of the view.</summary>
				Nullable<bool>							paragraphRecycle;

				auto operator<=>(const GuiDocumentConfig&) const = default;

				static GuiDocumentConfig				GetDocumentLabelDefaultConfig();
				static GuiDocumentConfig				GetDocumentViewerDefaultConfig();
				static GuiDocumentConfig				GetSinglelineTextBoxDefaultConfig();
				static GuiDocumentConfig				GetMultilineTextBoxDefaultConfig();
				static GuiDocumentConfig				OverrideConfig(const GuiDocumentConfig& toOverride, const GuiDocumentConfig& newConfig);
			};

/***********************************************************************
GuiDocumentConfigEvaluated
***********************************************************************/

			struct GuiDocumentConfigEvaluated
			{
				bool									autoExpand;
				bool									pasteAsPlainText;
				bool									wrapLine;
				GuiDocumentParagraphMode				paragraphMode;
				bool									paragraphPadding;
				bool									doubleLineBreaksBetweenParagraph;
				bool									spaceForFlattenedLineBreak;
				bool									paragraphRecycle;

				GuiDocumentConfigEvaluated(const GuiDocumentConfig& config);
			};
		}
	}
}

#endif
