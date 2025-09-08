/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDOCUMENTVIEWER
#define VCZH_PRESENTATION_CONTROLS_GUIDOCUMENTVIEWER

#include "GuiDocumentCommonInterface.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiDocumentViewer
***********************************************************************/
			
			/// <summary>Scrollable document viewer for displaying <see cref="DocumentModel"/>.</summary>
			class GuiDocumentViewer : public GuiScrollContainer, public GuiDocumentCommonInterface, public Description<GuiDocumentViewer>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(DocumentViewerTemplate, GuiScrollContainer)
			protected:

				void										UpdateDisplayFont()override;
				Point										GetDocumentViewPosition()override;
				void										EnsureRectVisible(Rect bounds)override;

				static GuiDocumentConfig					FixConfig(const GuiDocumentConfig& config);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_config">(Optional): configuration of document editing and rendering behavior.</param>
				GuiDocumentViewer(theme::ThemeName themeName, const GuiDocumentConfig& _config = {});
				~GuiDocumentViewer();

				const WString&								GetText()override;
				void										SetText(const WString& value)override;
			};

			/// <summary>Scrollable text box for displaying multiple lines of text.</summary>
			class GuiMultilineTextBox : public GuiDocumentViewer, public Description<GuiMultilineTextBox>
			{
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_config">(Optional): configuration of document editing and rendering behavior.</param>
				GuiMultilineTextBox(theme::ThemeName themeName, const GuiDocumentConfig& _config = {});
				~GuiMultilineTextBox();
			};

/***********************************************************************
GuiDocumentLabel
***********************************************************************/
			
			/// <summary>Static document viewer for displaying <see cref="DocumentModel"/>.</summary>
			class GuiDocumentLabel : public GuiControl, public GuiDocumentCommonInterface, public Description<GuiDocumentLabel>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(DocumentLabelTemplate, GuiControl)
			protected:
				compositions::GuiBoundsComposition*			scrollingContainer = nullptr;
				compositions::GuiBoundsComposition*			documentContainer = nullptr;

				void										UpdateDisplayFont()override;
				Point										GetDocumentViewPosition()override;
				void										EnsureRectVisible(Rect bounds)override;
				void										scrollingContainer_CachedBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										documentContainer_CachedMinSizeChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

				static GuiDocumentConfig					FixConfig(const GuiDocumentConfig& config);
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_config">(Optional): configuration of document editing and rendering behavior.</param>
				GuiDocumentLabel(theme::ThemeName themeName, const GuiDocumentConfig& _config = {});
				~GuiDocumentLabel();
				
				const WString&								GetText()override;
				void										SetText(const WString& value)override;
			};

			/// <summary>Scrollable text box for displaying single line of text.</summary>
			class GuiSinglelineTextBox : public GuiDocumentLabel, public Description<GuiSinglelineTextBox>
			{
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_config">(Optional): configuration of document editing and rendering behavior.</param>
				GuiSinglelineTextBox(theme::ThemeName themeName, const GuiDocumentConfig& _config = {});
				~GuiSinglelineTextBox();

				/// <summary>Get the password char. A password char is a character that replaces every characters in the document while rendering.</summary>
				/// <returns>Returns the passwrd char. 0 means no password char.</returns>
				wchar_t										GetPasswordChar();
				/// <summary>Set the password char.</summary>
				/// <param name="value">Set to 0 to remove the password char.</param>
				void										SetPasswordChar(wchar_t value);
			};
		}
	}
}

#endif
