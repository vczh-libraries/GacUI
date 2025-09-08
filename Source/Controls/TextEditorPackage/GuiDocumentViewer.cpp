#include "GuiDocumentViewer.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace compositions;

/***********************************************************************
GuiDocumentViewer
***********************************************************************/

			void GuiDocumentViewer::BeforeControlTemplateUninstalled_()
			{
				ReplaceMouseArea(nullptr);
			}

			void GuiDocumentViewer::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = TypedControlTemplateObject(true);
				baselineDocument = ct->GetBaselineDocument();
				if (documentElement)
				{
					documentElement->SetCaretColor(ct->GetCaretColor());
					SetDocument(GetDocument());
				}
				ReplaceMouseArea(containerComposition->GetParent());
			}

			void GuiDocumentViewer::UpdateDisplayFont()
			{
				GuiScrollContainer::UpdateDisplayFont();
				OnFontChanged();
			}

			Point GuiDocumentViewer::GetDocumentViewPosition()
			{
				// the document does not move in containerComposition
				return { 0,0 };
			}

			void GuiDocumentViewer::EnsureRectVisible(Rect bounds)
			{
				Rect viewBounds=GetViewBounds();
				{
					vint offset = 0;
					if (bounds.x1 < viewBounds.x1)
					{
						offset = bounds.x1 - viewBounds.x1;
					}
					else if (bounds.x2 > viewBounds.x2)
					{
						offset = bounds.x2 - viewBounds.x2;
					}

					if (auto scroll = GetHorizontalScroll())
					{
						scroll->SetPosition(viewBounds.x1 + offset);
					}
				}
				{
					vint offset = 0;
					if (bounds.y1 < viewBounds.y1)
					{
						offset = bounds.y1 - viewBounds.y1;
					}
					else if (bounds.y2 > viewBounds.y2)
					{
						offset = bounds.y2 - viewBounds.y2;
					}

					if (auto scroll = GetVerticalScroll())
					{
						scroll->SetPosition(viewBounds.y1 + offset);
					}
				}
			}

			GuiDocumentConfig GuiDocumentViewer::FixConfig(const GuiDocumentConfig& config)
			{
				auto result = config;
				result.autoExpand = false;
				return config;
			}

			GuiDocumentViewer::GuiDocumentViewer(theme::ThemeName themeName, const GuiDocumentConfig& _config)
				: GuiScrollContainer(themeName)
				, GuiDocumentCommonInterface(FixConfig(GuiDocumentConfig::OverrideConfig(GuiDocumentConfig::GetDocumentViewerDefaultConfig(), _config)))
			{
				SetAcceptTabInput(true);
				SetFocusableComposition(boundsComposition);
				InstallDocumentViewer(this, containerComposition->GetParent(), containerComposition, boundsComposition, focusableComposition);

				SetExtendToFullWidth(config.wrapLine);
				SetHorizontalAlwaysVisible(!config.wrapLine);
			}

			GuiDocumentViewer::~GuiDocumentViewer()
			{
			}

			const WString& GuiDocumentViewer::GetText()
			{
				text = UserInput_ConvertDocumentToText(documentElement->GetDocument());
				return text;
			}

			void GuiDocumentViewer::SetText(const WString& value)
			{
				SelectAll();
				SetSelectionText(value);
			}

/***********************************************************************
GuiMultilineTextBox
***********************************************************************/

			GuiMultilineTextBox::GuiMultilineTextBox(theme::ThemeName themeName, const GuiDocumentConfig& _config)
				: GuiDocumentViewer(themeName, FixConfig(GuiDocumentConfig::OverrideConfig(GuiDocumentConfig::GetMultilineTextBoxDefaultConfig(), _config)))
			{
				SetEditMode(GuiDocumentEditMode::Editable);
			}

			GuiMultilineTextBox::~GuiMultilineTextBox()
			{
			}

/***********************************************************************
GuiDocumentLabel
***********************************************************************/

			void GuiDocumentLabel::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiDocumentLabel::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = TypedControlTemplateObject(true);
				baselineDocument = ct->GetBaselineDocument();
				if (documentElement)
				{
					documentElement->SetCaretColor(ct->GetCaretColor());
					SetDocument(GetDocument());
				}
			}

			void GuiDocumentLabel::UpdateDisplayFont()
			{
				GuiControl::UpdateDisplayFont();
				OnFontChanged();
			}

			Point GuiDocumentLabel::GetDocumentViewPosition()
			{
				// when autoExpand is true, the document does not move in containerComposition
				// when autoExpand is not false, the document does not move in documentContainer
				return{ 0,0 };
			}

			void GuiDocumentLabel::EnsureRectVisible(Rect bounds)
			{
				if (!scrollingContainer) return;
				vint documentWidth = documentContainer->GetCachedBounds().Width();
				vint scrollingWidth = scrollingContainer->GetCachedBounds().Width();
				if (documentWidth <= scrollingWidth)
				{
					documentContainer->SetExpectedBounds({});
					return;
				}

				vint x1 = -documentContainer->GetCachedBounds().x1;
				vint x2 = x1 + scrollingWidth;
				vint offset = 0;

				if (bounds.x1 < x1)
				{
					offset = bounds.x1 - x1;
				}
				else if (bounds.x2 > x2)
				{
					offset = bounds.x2 - x2;
				}

				auto expectedBounds = documentContainer->GetExpectedBounds();
				expectedBounds.x1 -= offset;
				expectedBounds.x2 -= offset;

				if (expectedBounds.x1 > 0)
				{
					expectedBounds.x1 = 0;
				}
				else if (expectedBounds.x1 + documentWidth < scrollingWidth)
				{
					expectedBounds.x1 = scrollingWidth - documentWidth;
				}
				expectedBounds.x2 = expectedBounds.x1;
				documentContainer->SetExpectedBounds(expectedBounds);
				documentContainer->ForceCalculateSizeImmediately();
			}

			void GuiDocumentLabel::scrollingContainer_CachedBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				Rect bounds = scrollingContainer->GetCachedBounds();
				vint x1 = documentContainer->GetCachedBounds().x1;
				vint offset = -x1 - bounds.x1;
				bounds.x1 += offset;
				bounds.x2 += offset;
				EnsureRectVisible(bounds);
			}

			void GuiDocumentLabel::documentContainer_CachedMinSizeChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				scrollingContainer->SetPreferredMinSize({ 0,documentContainer->GetCachedMinSize().y });
			}

			GuiDocumentConfig GuiDocumentLabel::FixConfig(const GuiDocumentConfig& config)
			{
				auto result = config;
				if (!result.autoExpand.Value())
				{
					if (result.wrapLine.Value() || result.paragraphMode.Value() != GuiDocumentParagraphMode::Singleline)
					{
						result.autoExpand = true;
					}
				}
				return result;
			}

			GuiDocumentLabel::GuiDocumentLabel(theme::ThemeName themeName, const GuiDocumentConfig& _config)
				: GuiControl(themeName)
				, GuiDocumentCommonInterface(FixConfig(GuiDocumentConfig::OverrideConfig(GuiDocumentConfig::GetDocumentLabelDefaultConfig(), _config)))
			{
				SetAcceptTabInput(true);
				SetFocusableComposition(boundsComposition);

				if (config.autoExpand)
				{
					InstallDocumentViewer(this, containerComposition, containerComposition, boundsComposition, focusableComposition);
				}
				else
				{
					scrollingContainer = new GuiBoundsComposition();
					scrollingContainer->SetAlignmentToParent(Margin(0, 0, 0, 0));
					containerComposition->AddChild(scrollingContainer);

					documentContainer = new GuiBoundsComposition();
					documentContainer->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					scrollingContainer->AddChild(documentContainer);

					scrollingContainer->CachedBoundsChanged.AttachMethod(this, &GuiDocumentLabel::scrollingContainer_CachedBoundsChanged);
					documentContainer->CachedBoundsChanged.AttachMethod(this, &GuiDocumentLabel::documentContainer_CachedMinSizeChanged);
					InstallDocumentViewer(this, containerComposition, documentContainer, boundsComposition, focusableComposition);
				}
			}

			GuiDocumentLabel::~GuiDocumentLabel()
			{
			}

			const WString& GuiDocumentLabel::GetText()
			{
				text = UserInput_ConvertDocumentToText(documentElement->GetDocument());
				return text;
			}

			void GuiDocumentLabel::SetText(const WString& value)
			{
				SelectAll();
				SetSelectionText(value);
			}

/***********************************************************************
GuiSinglelineTextBox
***********************************************************************/

			GuiSinglelineTextBox::GuiSinglelineTextBox(theme::ThemeName themeName, const GuiDocumentConfig& _config)
				: GuiDocumentLabel(themeName, FixConfig(GuiDocumentConfig::OverrideConfig(GuiDocumentConfig::GetSinglelineTextBoxDefaultConfig(), _config)))
			{
				SetEditMode(GuiDocumentEditMode::Editable);
			}

			GuiSinglelineTextBox::~GuiSinglelineTextBox()
			{
			}

			wchar_t GuiSinglelineTextBox::GetPasswordChar()
			{
				return documentElement->GetPasswordChar();
			}

			void GuiSinglelineTextBox::SetPasswordChar(wchar_t value)
			{
				documentElement->SetPasswordChar(value);
			}
		}
	}
}
