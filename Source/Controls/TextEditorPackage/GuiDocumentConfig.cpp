#include "GuiDocumentConfig.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
/***********************************************************************
GuiDocumentConfig
***********************************************************************/

			GuiDocumentConfig GuiDocumentConfig::GetDocumentLabelDefaultConfig()
			{
				GuiDocumentConfig config;
				config.autoExpand = true;
				config.pasteAsPlainText = false;
				config.wrapLine = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				config.paragraphPadding = true;
				config.doubleLineBreaksBetweenParagraph = true;

				config.spaceForFlattenedLineBreak = false;
				config.paragraphRecycle = false;
				return config;
			}

			GuiDocumentConfig GuiDocumentConfig::GetDocumentViewerDefaultConfig()
			{
				GuiDocumentConfig config;
				config.autoExpand = false;
				config.pasteAsPlainText = false;
				config.wrapLine = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				config.paragraphPadding = true;
				config.doubleLineBreaksBetweenParagraph = true;

				config.spaceForFlattenedLineBreak = false;
				config.paragraphRecycle = true;
				return config;
			}

			GuiDocumentConfig GuiDocumentConfig::GetSinglelineTextBoxDefaultConfig()
			{
				GuiDocumentConfig config;
				config.autoExpand = false;
				config.pasteAsPlainText = true;
				config.wrapLine = false;
				config.paragraphMode = GuiDocumentParagraphMode::Singleline;
				config.paragraphPadding = false;
				config.doubleLineBreaksBetweenParagraph = false;

				config.spaceForFlattenedLineBreak = false;
				config.paragraphRecycle = false;
				return config;
			}

			GuiDocumentConfig GuiDocumentConfig::GetMultilineTextBoxDefaultConfig()
			{
				GuiDocumentConfig config;
				config.autoExpand = false;
				config.pasteAsPlainText = true;
				config.wrapLine = false;
				config.paragraphMode = GuiDocumentParagraphMode::Multiline;
				config.paragraphPadding = false;
				config.doubleLineBreaksBetweenParagraph = false;

				config.spaceForFlattenedLineBreak = false;
				config.paragraphRecycle = true;
				return config;
			}

			GuiDocumentConfig GuiDocumentConfig::OverrideConfig(const GuiDocumentConfig& toOverride, const GuiDocumentConfig& newConfig)
			{
				GuiDocumentConfig result = toOverride;
				if (newConfig.autoExpand) result.autoExpand = newConfig.autoExpand;
				if (newConfig.pasteAsPlainText) result.pasteAsPlainText = newConfig.pasteAsPlainText;
				if (newConfig.wrapLine) result.wrapLine = newConfig.wrapLine;
				if (newConfig.paragraphMode) result.paragraphMode = newConfig.paragraphMode;
				if (newConfig.paragraphPadding) result.paragraphPadding = newConfig.paragraphPadding;
				if (newConfig.doubleLineBreaksBetweenParagraph) result.doubleLineBreaksBetweenParagraph = newConfig.doubleLineBreaksBetweenParagraph;
				if (newConfig.spaceForFlattenedLineBreak) result.spaceForFlattenedLineBreak = newConfig.spaceForFlattenedLineBreak;
				if (newConfig.paragraphRecycle) result.paragraphRecycle = newConfig.paragraphRecycle;
				return result;
			}

/***********************************************************************
GuiDocumentConfigEvaluated
***********************************************************************/

			GuiDocumentConfigEvaluated::GuiDocumentConfigEvaluated(const GuiDocumentConfig& config)
				: autoExpand(config.autoExpand.Value())
				, pasteAsPlainText(config.pasteAsPlainText.Value())
				, wrapLine(config.wrapLine.Value())
				, paragraphMode(config.paragraphMode.Value())
				, paragraphPadding(config.paragraphPadding.Value())
				, doubleLineBreaksBetweenParagraph(config.doubleLineBreaksBetweenParagraph.Value())
				, spaceForFlattenedLineBreak(config.spaceForFlattenedLineBreak.Value())
				, paragraphRecycle(config.paragraphRecycle.Value())
			{
			}
		}
	}
}
