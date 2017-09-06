/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATESTYLES
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATESTYLES

#include "GuiControlTemplates.h"
#include "../ListControlPackage/GuiDataGridExtensions.h"
#include "../Styles/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{
#pragma warning(push)
#pragma warning(disable:4250)

/***********************************************************************
Control Template
***********************************************************************/

			class GuiControlTemplate_StyleProvider
				: public Object
				, public virtual controls::GuiControl::IStyleController
				, public virtual controls::GuiControl::IStyleProvider
				, public Description<GuiControlTemplate_StyleProvider>
			{
			protected:
				controls::GuiControl::IStyleController*							associatedStyleController;
				GuiControlTemplate*												controlTemplate;

			public:
				GuiControlTemplate_StyleProvider(TemplateProperty<GuiControlTemplate> factory, description::Value viewModel = description::Value());
				~GuiControlTemplate_StyleProvider();

				compositions::GuiBoundsComposition*								GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*							GetContainerComposition()override;
				void															AssociateStyleController(controls::GuiControl::IStyleController* controller)override;
				void															SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void															SetText(const WString& value)override;
				void															SetFont(const FontProperties& value)override;
				void															SetVisuallyEnabled(bool value)override;
			};

			class GuiLabelTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public controls::GuiLabel::IStyleController
				, public Description<GuiLabelTemplate_StyleProvider>
			{
			protected:
				GuiLabelTemplate*												controlTemplate;

			public:
				GuiLabelTemplate_StyleProvider(TemplateProperty<GuiLabelTemplate> factory);
				~GuiLabelTemplate_StyleProvider();

				Color															GetDefaultTextColor()override;
				void															SetTextColor(Color value)override;
			};

			class GuiSinglelineTextBoxTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiSinglelineTextBox::IStyleProvider
				, public Description<GuiSinglelineTextBoxTemplate_StyleProvider>
			{
			protected:
				GuiSinglelineTextBoxTemplate*									controlTemplate;
				
			public:
				GuiSinglelineTextBoxTemplate_StyleProvider(TemplateProperty<GuiSinglelineTextBoxTemplate> factory);
				~GuiSinglelineTextBoxTemplate_StyleProvider();
				
				void															SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				compositions::GuiGraphicsComposition*							InstallBackground(compositions::GuiBoundsComposition* boundsComposition)override;
			};

			class GuiDocumentLabelTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiDocumentLabel::IStyleController
				, public Description<GuiDocumentLabelTemplate_StyleProvider>
			{
			protected:
				GuiDocumentLabelTemplate*										controlTemplate;
				
			public:
				GuiDocumentLabelTemplate_StyleProvider(TemplateProperty<GuiDocumentLabelTemplate> factory);
				~GuiDocumentLabelTemplate_StyleProvider();
				
				Ptr<DocumentModel>												GetBaselineDocument()override;
				Color															GetCaretColor()override;
			};

			class GuiMenuTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public controls::GuiWindow::DefaultBehaviorStyleController
				, public Description<GuiMenuTemplate_StyleProvider>
			{
			public:
				GuiMenuTemplate_StyleProvider(TemplateProperty<GuiMenuTemplate> factory);
				~GuiMenuTemplate_StyleProvider();
			};

			class GuiWindowTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public controls::GuiWindow::IStyleController
				, public Description<GuiWindowTemplate_StyleProvider>
			{
			protected:
				GuiWindowTemplate*												controlTemplate;
				controls::GuiWindow*											window;

			public:
				GuiWindowTemplate_StyleProvider(TemplateProperty<GuiWindowTemplate> factory);
				~GuiWindowTemplate_StyleProvider();

				void															AttachWindow(controls::GuiWindow* _window)override;
				void															InitializeNativeWindowProperties()override;
				void															SetSizeState(INativeWindow::WindowSizeState value)override;
				bool															GetMaximizedBox()override;
				void															SetMaximizedBox(bool visible)override;
				bool															GetMinimizedBox()override;
				void															SetMinimizedBox(bool visible)override;
				bool															GetBorder()override;
				void															SetBorder(bool visible)override;
				bool															GetSizeBox()override;
				void															SetSizeBox(bool visible)override;
				bool															GetIconVisible()override;
				void															SetIconVisible(bool visible)override;
				bool															GetTitleBar()override;
				void															SetTitleBar(bool visible)override;
				controls::GuiWindow::IStyleController*							CreateTooltipStyle()override;
				controls::GuiLabel::IStyleController*							CreateShortcutKeyStyle()override;
			};

			class GuiButtonTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiButton::IStyleController
				, public Description<GuiButtonTemplate_StyleProvider>
			{
			protected:
				GuiButtonTemplate*												controlTemplate;

			public:
				GuiButtonTemplate_StyleProvider(TemplateProperty<GuiButtonTemplate> factory);
				~GuiButtonTemplate_StyleProvider();

				void															Transfer(controls::ButtonState value)override;
			};

			class GuiSelectableButtonTemplate_StyleProvider
				: public GuiButtonTemplate_StyleProvider
				, public virtual controls::GuiSelectableButton::IStyleController
				, public Description<GuiSelectableButtonTemplate_StyleProvider>
			{
			protected:
				GuiSelectableButtonTemplate*									controlTemplate;

			public:
				GuiSelectableButtonTemplate_StyleProvider(TemplateProperty<GuiSelectableButtonTemplate> factory);
				~GuiSelectableButtonTemplate_StyleProvider();

				void															SetSelected(bool value)override;
			};

			class GuiToolstripButtonTemplate_StyleProvider
				: public GuiSelectableButtonTemplate_StyleProvider
				, public virtual controls::GuiMenuButton::IStyleController
				, public Description<GuiToolstripButtonTemplate_StyleProvider>
			{
			protected:
				GuiToolstripButtonTemplate*										controlTemplate;

			public:
				GuiToolstripButtonTemplate_StyleProvider(TemplateProperty<GuiToolstripButtonTemplate> factory);
				~GuiToolstripButtonTemplate_StyleProvider();
				
				controls::GuiMenu::IStyleController*							CreateSubMenuStyleController()override;
				void															SetSubMenuExisting(bool value)override;
				void															SetSubMenuOpening(bool value)override;
				controls::GuiButton*											GetSubMenuHost()override;
				void															SetImage(Ptr<GuiImageData> value)override;
				void															SetShortcutText(const WString& value)override;
			};

			class GuiListViewColumnHeaderTemplate_StyleProvider
				: public GuiToolstripButtonTemplate_StyleProvider
				, public virtual controls::GuiListViewColumnHeader::IStyleController
				, public Description<GuiListViewColumnHeaderTemplate_StyleProvider>
			{
			protected:
				GuiListViewColumnHeaderTemplate*								controlTemplate;

			public:
				GuiListViewColumnHeaderTemplate_StyleProvider(TemplateProperty<GuiListViewColumnHeaderTemplate> factory);
				~GuiListViewColumnHeaderTemplate_StyleProvider();

				void															SetColumnSortingState(controls::ColumnSortingState value)override;
			};

			class GuiComboBoxTemplate_StyleProvider
				: public GuiToolstripButtonTemplate_StyleProvider
				, public virtual controls::GuiComboBoxListControl::IStyleController
				, public Description<GuiComboBoxTemplate_StyleProvider>
			{
			protected:
				GuiComboBoxTemplate*											controlTemplate;

			public:
				GuiComboBoxTemplate_StyleProvider(TemplateProperty<GuiComboBoxTemplate> factory);
				~GuiComboBoxTemplate_StyleProvider();
				
				void															SetCommandExecutor(controls::IComboBoxCommandExecutor* value)override;
				void															OnItemSelected()override;
				void															SetTextVisible(bool value)override;
			};

			class GuiScrollTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiScroll::IStyleController
				, public Description<GuiScrollTemplate_StyleProvider>
			{
			protected:
				GuiScrollTemplate*												controlTemplate;

			public:
				GuiScrollTemplate_StyleProvider(TemplateProperty<GuiScrollTemplate> factory);
				~GuiScrollTemplate_StyleProvider();

				void															SetCommandExecutor(controls::IScrollCommandExecutor* value)override;
				void															SetTotalSize(vint value)override;
				void															SetPageSize(vint value)override;
				void															SetPosition(vint value)override;
			};

			class GuiScrollViewTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiScrollView::IStyleProvider
				, public Description<GuiScrollViewTemplate_StyleProvider>
			{
			protected:
				GuiScrollViewTemplate*											controlTemplate;
				
			public:
				GuiScrollViewTemplate_StyleProvider(TemplateProperty<GuiScrollViewTemplate> factory);
				~GuiScrollViewTemplate_StyleProvider();
				
				controls::GuiScroll::IStyleController*							CreateHorizontalScrollStyle()override;
				controls::GuiScroll::IStyleController*							CreateVerticalScrollStyle()override;
				vint															GetDefaultScrollSize()override;
				compositions::GuiGraphicsComposition*							InstallBackground(compositions::GuiBoundsComposition* boundsComposition)override;
			};

			class GuiMultilineTextBoxTemplate_StyleProvider
				: public GuiScrollViewTemplate_StyleProvider
				, public Description<GuiMultilineTextBoxTemplate_StyleProvider>
			{
			protected:
				GuiMultilineTextBoxTemplate*									controlTemplate;
				
			public:
				GuiMultilineTextBoxTemplate_StyleProvider(TemplateProperty<GuiMultilineTextBoxTemplate> factory);
				~GuiMultilineTextBoxTemplate_StyleProvider();
				
				void															SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
			};

			class GuiDocumentViewerTemplate_StyleProvider
				: public GuiScrollViewTemplate_StyleProvider
				, public virtual controls::GuiDocumentViewer::IStyleProvider
				, public Description<GuiDocumentViewerTemplate_StyleProvider>
			{
			protected:
				GuiDocumentViewerTemplate*										controlTemplate;
				
			public:
				GuiDocumentViewerTemplate_StyleProvider(TemplateProperty<GuiDocumentViewerTemplate> factory);
				~GuiDocumentViewerTemplate_StyleProvider();
				
				Ptr<DocumentModel>												GetBaselineDocument()override;
				Color															GetCaretColor()override;
			};

			class GuiTextListTemplate_StyleProvider
				: public GuiScrollViewTemplate_StyleProvider
				, public virtual controls::GuiVirtualTextList::IStyleProvider
				, public Description<GuiTextListTemplate_StyleProvider>
			{
			protected:
				GuiTextListTemplate*											controlTemplate;

			public:
				GuiTextListTemplate_StyleProvider(TemplateProperty<GuiTextListTemplate> factory);
				~GuiTextListTemplate_StyleProvider();
				
				Color															GetTextColor()override;
				controls::GuiSelectableButton::IStyleController*				CreateItemBackground()override;
				controls::GuiSelectableButton::IStyleController*				CreateCheckBulletStyle()override;
				controls::GuiSelectableButton::IStyleController*				CreateRadioBulletStyle()override;
			};

			class GuiListViewTemplate_StyleProvider
				: public GuiScrollViewTemplate_StyleProvider
				, public virtual controls::GuiListViewBase::IStyleProvider
				, public Description<GuiListViewTemplate_StyleProvider>
			{
			protected:
				GuiListViewTemplate*											controlTemplate;
				
			public:
				GuiListViewTemplate_StyleProvider(TemplateProperty<GuiListViewTemplate> factory);
				~GuiListViewTemplate_StyleProvider();
				
				controls::GuiSelectableButton::IStyleController*				CreateItemBackground()override;
				controls::GuiListViewColumnHeader::IStyleController*			CreateColumnStyle()override;
				Color															GetPrimaryTextColor()override;
				Color															GetSecondaryTextColor()override;
				Color															GetItemSeparatorColor()override;
			};

			class GuiTreeViewTemplate_StyleProvider
				: public GuiScrollViewTemplate_StyleProvider
				, public virtual controls::GuiVirtualTreeView::IStyleProvider
				, public Description<GuiTreeViewTemplate_StyleProvider>
			{
			protected:
				GuiTreeViewTemplate*											controlTemplate;
				
			public:
				GuiTreeViewTemplate_StyleProvider(TemplateProperty<GuiTreeViewTemplate> factory);
				~GuiTreeViewTemplate_StyleProvider();
				
				controls::GuiSelectableButton::IStyleController*				CreateItemBackground()override;
				controls::GuiSelectableButton::IStyleController*				CreateItemExpandingDecorator()override;
				Color															GetTextColor()override;
			};

			class GuiTabTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiTab::IStyleController
				, public Description<GuiTabTemplate_StyleProvider>
			{
			protected:
				GuiTabTemplate*													controlTemplate;

			public:
				GuiTabTemplate_StyleProvider(TemplateProperty<GuiTabTemplate> factory);
				~GuiTabTemplate_StyleProvider();

				void															SetCommandExecutor(controls::ITabCommandExecutor* value)override;
				void															SetTabPages(Ptr<reflection::description::IValueObservableList> value)override;
				void															SetSelectedTabPage(controls::GuiTabPage* value)override;
			};

			class GuiDatePickerTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiDatePicker::IStyleController
				, public Description<GuiDatePickerTemplate_StyleProvider>
			{
			protected:
				GuiDatePickerTemplate*											controlTemplate;

			public:
				GuiDatePickerTemplate_StyleProvider(TemplateProperty<GuiDatePickerTemplate> factory);
				~GuiDatePickerTemplate_StyleProvider();

				void															SetCommandExecutor(controls::IDatePickerCommandExecutor* value)override;
				void															SetDateLocale(const Locale& value)override;
				const DateTime&													GetDate()override;
				void															SetDate(const DateTime& value)override;
			};

			class GuiDateComboBoxTemplate_StyleProvider
				: public GuiComboBoxTemplate_StyleProvider
				, public Description<GuiDateComboBoxTemplate_StyleProvider>
			{
			protected:
				GuiDateComboBoxTemplate*										controlTemplate;

			public:
				GuiDateComboBoxTemplate_StyleProvider(TemplateProperty<GuiDateComboBoxTemplate> factory);
				~GuiDateComboBoxTemplate_StyleProvider();

				controls::GuiDatePicker*										CreateArgument();
				controls::GuiDatePicker::IStyleProvider*						CreateDatePickerStyle();
			};

/***********************************************************************
Helper Functions
***********************************************************************/

			extern void												SplitBySemicolon(const WString& input, collections::List<WString>& fragments);

#pragma warning(pop)
		}
	}
}

#endif