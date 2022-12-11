/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Common Style Helpers

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITHEMEMANAGER
#define VCZH_PRESENTATION_CONTROLS_GUITHEMEMANAGER

#include "../GraphicsCompositions/GuiGraphicsBasicComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace theme
		{

/***********************************************************************
Theme Builders
***********************************************************************/

#define GUI_TEMPLATE_PROPERTY_DECL(CLASS, TYPE, NAME, VALUE)\
			private:\
				TYPE NAME##_ = VALUE;\
			public:\
				TYPE Get##NAME();\
				void Set##NAME(TYPE const& value);\
				compositions::GuiNotifyEvent NAME##Changed;\

#define GUI_TEMPLATE_PROPERTY_IMPL(CLASS, TYPE, NAME, VALUE)\
			TYPE CLASS::Get##NAME()\
			{\
				return NAME##_;\
			}\
			void CLASS::Set##NAME(TYPE const& value)\
			{\
				if (NAME##_ != value)\
				{\
					NAME##_ = value;\
					NAME##Changed.Execute(compositions::GuiEventArgs(this));\
				}\
			}\

#define GUI_TEMPLATE_PROPERTY_EVENT_INIT(CLASS, TYPE, NAME, VALUE)\
			NAME##Changed.SetAssociatedComposition(this);

#define GUI_TEMPLATE_CLASS_DECL(CLASS, BASE)\
			class CLASS : public BASE, public AggregatableDescription<CLASS>\
			{\
			public:\
				CLASS();\
				~CLASS();\
				CLASS ## _PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)\
			};\

#define GUI_TEMPLATE_CLASS_IMPL(CLASS, BASE)\
			CLASS ## _PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)\
			CLASS::CLASS()\
			{\
				CLASS ## _PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)\
			}\
			CLASS::~CLASS()\
			{\
				FinalizeAggregation();\
			}\

/***********************************************************************
GuiTemplate
***********************************************************************/

			/// <summary>Represents a user customizable template.</summary>
			class GuiTemplate : public compositions::GuiBoundsComposition, public controls::GuiInstanceRootObject, public Description<GuiTemplate>
			{
			protected:
				controls::GuiControlHost*		GetControlHostForInstance()override;
				void							OnParentLineChanged()override;
			public:
				/// <summary>Create a template.</summary>
				GuiTemplate();
				~GuiTemplate();
				
#define GuiTemplate_PROPERTIES(F)\
				F(GuiTemplate,	FontProperties,		Font,				{}	)\
				F(GuiTemplate,	description::Value,	Context,			{}	)\
				F(GuiTemplate,	WString,			Text,				{}	)\
				F(GuiTemplate,	bool,				VisuallyEnabled,	true)\

				GuiTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

/***********************************************************************
Core Themes
***********************************************************************/

/***********************************************************************
Theme Names
***********************************************************************/

#define GUI_CONTROL_TEMPLATE_TYPES(F) \
			F(WindowTemplate,				Window)						\
			F(ControlTemplate,				CustomControl)				\
			F(WindowTemplate,				Tooltip)					\
			F(LabelTemplate,				Label)						\
			F(LabelTemplate,				ShortcutKey)				\
			F(ScrollViewTemplate,			ScrollView)					\
			F(ControlTemplate,				GroupBox)					\
			F(TabTemplate,					Tab)						\
			F(ComboBoxTemplate,				ComboBox)					\
			F(MultilineTextBoxTemplate,		MultilineTextBox)			\
			F(SinglelineTextBoxTemplate,	SinglelineTextBox)			\
			F(DocumentViewerTemplate,		DocumentViewer)				\
			F(DocumentLabelTemplate,		DocumentLabel)				\
			F(DocumentLabelTemplate,		DocumentTextBox)			\
			F(ListViewTemplate,				ListView)					\
			F(TreeViewTemplate,				TreeView)					\
			F(TextListTemplate,				TextList)					\
			F(SelectableButtonTemplate,		ListItemBackground)			\
			F(SelectableButtonTemplate,		TreeItemExpander)			\
			F(SelectableButtonTemplate,		CheckTextListItem)			\
			F(SelectableButtonTemplate,		RadioTextListItem)			\
			F(MenuTemplate,					Menu)						\
			F(ControlTemplate,				MenuBar)					\
			F(ControlTemplate,				MenuSplitter)				\
			F(ToolstripButtonTemplate,		MenuBarButton)				\
			F(ToolstripButtonTemplate,		MenuItemButton)				\
			F(ControlTemplate,				ToolstripToolBar)			\
			F(ToolstripButtonTemplate,		ToolstripButton)			\
			F(ToolstripButtonTemplate,		ToolstripDropdownButton)	\
			F(ToolstripButtonTemplate,		ToolstripSplitButton)		\
			F(ControlTemplate,				ToolstripSplitter)			\
			F(RibbonTabTemplate,			RibbonTab)					\
			F(RibbonGroupTemplate,			RibbonGroup)				\
			F(RibbonIconLabelTemplate,		RibbonIconLabel)			\
			F(RibbonIconLabelTemplate,		RibbonSmallIconLabel)		\
			F(RibbonButtonsTemplate,		RibbonButtons)				\
			F(RibbonToolstripsTemplate,		RibbonToolstrips)			\
			F(RibbonGalleryTemplate,		RibbonGallery)				\
			F(RibbonToolstripMenuTemplate,	RibbonToolstripMenu)		\
			F(RibbonGalleryListTemplate,	RibbonGalleryList)			\
			F(TextListTemplate,				RibbonGalleryItemList)		\
			F(ToolstripButtonTemplate,		RibbonSmallButton)			\
			F(ToolstripButtonTemplate,		RibbonSmallDropdownButton)	\
			F(ToolstripButtonTemplate,		RibbonSmallSplitButton)		\
			F(ToolstripButtonTemplate,		RibbonLargeButton)			\
			F(ToolstripButtonTemplate,		RibbonLargeDropdownButton)	\
			F(ToolstripButtonTemplate,		RibbonLargeSplitButton)		\
			F(ControlTemplate,				RibbonSplitter)				\
			F(ControlTemplate,				RibbonToolstripHeader)		\
			F(ButtonTemplate,				Button)						\
			F(SelectableButtonTemplate,		CheckBox)					\
			F(SelectableButtonTemplate,		RadioButton)				\
			F(DatePickerTemplate,			DatePicker)					\
			F(DateComboBoxTemplate,			DateComboBox)				\
			F(ScrollTemplate,				HScroll)					\
			F(ScrollTemplate,				VScroll)					\
			F(ScrollTemplate,				HTracker)					\
			F(ScrollTemplate,				VTracker)					\
			F(ScrollTemplate,				ProgressBar)				\

			enum class ThemeName
			{
				Unknown,
#define GUI_DEFINE_THEME_NAME(TEMPLATE, CONTROL) CONTROL,
				GUI_CONTROL_TEMPLATE_TYPES(GUI_DEFINE_THEME_NAME)
#undef GUI_DEFINE_THEME_NAME
			};

			/// <summary>Theme interface. A theme creates appropriate style controllers or style providers for default controls. Call [M:vl.presentation.theme.GetCurrentTheme] to access this interface.</summary>
			class ITheme : public virtual IDescriptable, public Description<ITheme>
			{
			public:
				virtual TemplateProperty<templates::GuiControlTemplate>				CreateStyle(ThemeName themeName) = 0;
			};

			/// <summary>Get the current theme style factory object. Call <see cref="RegisterTheme"/> or <see cref="UnregisterTheme"/> to change the default theme.</summary>
			/// <returns>The current theme style factory object.</returns>
			extern ITheme*						GetCurrentTheme();
			extern void							InitializeTheme();
			extern void							FinalizeTheme();
		}
	}
}

#endif