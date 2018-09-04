/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITOOLSTRIPMENU
#define VCZH_PRESENTATION_CONTROLS_GUITOOLSTRIPMENU

#include "GuiToolstripCommand.h"
#include "GuiMenuControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
Toolstrip Item Collection
***********************************************************************/

			/// <summary>IToolstripUpdateLayout is a required service for all menu item container.</summary>
			class IToolstripUpdateLayout : public IDescriptable
			{
			public:
				virtual void								UpdateLayout() = 0;
			};

			/// <summary>IToolstripUpdateLayout is a required service for a menu item which want to force the container to redo layout.</summary>
			class IToolstripUpdateLayoutInvoker : public IDescriptable
			{
			public:
				/// <summary>The identifier for this service.</summary>
				static const wchar_t* const					Identifier;

				virtual void								SetCallback(IToolstripUpdateLayout* callback) = 0;
			};

			/// <summary>Toolstrip item collection.</summary>
			class GuiToolstripCollectionBase : public collections::ObservableListBase<GuiControl*>
			{
			public:

			protected:
				IToolstripUpdateLayout *					contentCallback;

				void										InvokeUpdateLayout();
				bool										QueryInsert(vint index, GuiControl* const& child)override;
				void										BeforeRemove(vint index, GuiControl* const& child)override;
				void										AfterInsert(vint index, GuiControl* const& child)override;
				void										AfterRemove(vint index, vint count)override;
			public:
				GuiToolstripCollectionBase(IToolstripUpdateLayout* _contentCallback);
				~GuiToolstripCollectionBase();
			};

			/// <summary>Toolstrip item collection.</summary>
			class GuiToolstripCollection : public GuiToolstripCollectionBase
			{
				using EventHandlerList = collections::List<Ptr<compositions::IGuiGraphicsEventHandler>>;
			protected:
				compositions::GuiStackComposition*			stackComposition;
				EventHandlerList							eventHandlers;

				void										UpdateItemVisibility(vint index, GuiControl* child);
				void										OnItemVisibleChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										BeforeRemove(vint index, GuiControl* const& child)override;
				void										AfterInsert(vint index, GuiControl* const& child)override;
				void										AfterRemove(vint index, vint count)override;
			public:
				GuiToolstripCollection(IToolstripUpdateLayout* _contentCallback, compositions::GuiStackComposition* _stackComposition);
				~GuiToolstripCollection();
			};

/***********************************************************************
Toolstrip Container
***********************************************************************/

			/// <summary>Toolstrip menu.</summary>
			class GuiToolstripMenu : public GuiMenu, protected IToolstripUpdateLayout,  Description<GuiToolstripMenu>
			{
			protected:
				compositions::GuiSharedSizeRootComposition*		sharedSizeRootComposition;
				compositions::GuiStackComposition*				stackComposition;
				Ptr<GuiToolstripCollection>						toolstripItems;

				void											UpdateLayout()override;
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_owner">The owner menu item of the parent menu.</param>
				GuiToolstripMenu(theme::ThemeName themeName, GuiControl* _owner);
				~GuiToolstripMenu();
				
				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				collections::ObservableListBase<GuiControl*>&	GetToolstripItems();
			};

			/// <summary>Toolstrip menu bar.</summary>
			class GuiToolstripMenuBar : public GuiMenuBar, public Description<GuiToolstripMenuBar>
			{
			protected:
				compositions::GuiStackComposition*				stackComposition;
				Ptr<GuiToolstripCollection>						toolstripItems;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiToolstripMenuBar(theme::ThemeName themeName);
				~GuiToolstripMenuBar();
				
				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				collections::ObservableListBase<GuiControl*>&	GetToolstripItems();
			};

			/// <summary>Toolstrip tool bar.</summary>
			class GuiToolstripToolBar : public GuiControl, public Description<GuiToolstripToolBar>
			{
			protected:
				compositions::GuiStackComposition*				stackComposition;
				Ptr<GuiToolstripCollection>						toolstripItems;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiToolstripToolBar(theme::ThemeName themeName);
				~GuiToolstripToolBar();
				
				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				collections::ObservableListBase<GuiControl*>&	GetToolstripItems();
			};

/***********************************************************************
Toolstrip Component
***********************************************************************/

			/// <summary>Toolstrip button that can connect with a <see cref="GuiToolstripCommand"/>.</summary>
			class GuiToolstripButton : public GuiMenuButton, protected IToolstripUpdateLayoutInvoker, public Description<GuiToolstripButton>
			{
			protected:
				GuiToolstripCommand*							command;
				IToolstripUpdateLayout*							callback = nullptr;
				Ptr<compositions::IGuiGraphicsEventHandler>		descriptionChangedHandler;

				void											SetCallback(IToolstripUpdateLayout* _callback)override;
				void											OnActiveAlt()override;
				void											UpdateCommandContent();
				void											OnLayoutAwaredPropertyChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void											OnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void											OnCommandDescriptionChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiToolstripButton(theme::ThemeName themeName);
				~GuiToolstripButton();

				/// <summary>Get the attached <see cref="GuiToolstripCommand"/>.</summary>
				/// <returns>The attached toolstrip command.</returns>
				GuiToolstripCommand*							GetCommand();
				/// <summary>Detach from the previous <see cref="GuiToolstripCommand"/> and attach to a new one. If the command is null, this function only do detaching.</summary>
				/// <param name="value">The new toolstrip command.</param>
				void											SetCommand(GuiToolstripCommand* value);

				/// <summary>Get the toolstrip sub menu. If the sub menu is not created, it returns null.</summary>
				/// <returns>The toolstrip sub menu.</returns>
				GuiToolstripMenu*								GetToolstripSubMenu();

				/// <summary>Get the toolstrip sub menu. If the sub menu is not created, it returns null.</summary>
				/// <returns>The toolstrip sub menu.</returns>
				GuiToolstripMenu*								EnsureToolstripSubMenu();
				/// <summary>Create the toolstrip sub menu if necessary. The created toolstrip sub menu is owned by this menu button.</summary>
				/// <param name="subMenuTemplate">The style controller for the toolstrip sub menu. Set to null to use the default control template.</param>
				void											CreateToolstripSubMenu(TemplateProperty<templates::GuiMenuTemplate> subMenuTemplate);

				IDescriptable*									QueryService(const WString& identifier)override;
			};

/***********************************************************************
Toolstrip Group
***********************************************************************/

			class GuiToolstripNestedContainer : public GuiControl, protected IToolstripUpdateLayout, protected IToolstripUpdateLayoutInvoker
			{
			protected:
				IToolstripUpdateLayout*							callback = nullptr;

				void											UpdateLayout()override;
				void											SetCallback(IToolstripUpdateLayout* _callback)override;
			public:
				GuiToolstripNestedContainer(theme::ThemeName themeName);
				~GuiToolstripNestedContainer();

				IDescriptable*									QueryService(const WString& identifier)override;
			};

			/// <summary>A toolstrip item, which is also a toolstrip item container, automatically maintaining splitters between items.</summary>
			class GuiToolstripGroupContainer : public GuiToolstripNestedContainer, public Description<GuiToolstripGroupContainer>
			{
			protected:
				class GroupCollection : public GuiToolstripCollectionBase
				{
				protected:
					GuiToolstripGroupContainer*					container;
					ControlTemplatePropertyType					splitterTemplate;

					void										BeforeRemove(vint index, GuiControl* const& child)override;
					void										AfterInsert(vint index, GuiControl* const& child)override;
					void										AfterRemove(vint index, vint count)override;
				public:
					GroupCollection(GuiToolstripGroupContainer* _container);
					~GroupCollection();

					ControlTemplatePropertyType					GetSplitterTemplate();
					void										SetSplitterTemplate(const ControlTemplatePropertyType& value);
					void										RebuildSplitters();
				};

			protected:
				compositions::GuiStackComposition*				stackComposition;
				theme::ThemeName								splitterThemeName;
				Ptr<GroupCollection>							groupCollection;

				void											OnParentLineChanged()override;
			public:
				GuiToolstripGroupContainer(theme::ThemeName themeName);
				~GuiToolstripGroupContainer();

				ControlTemplatePropertyType						GetSplitterTemplate();
				void											SetSplitterTemplate(const ControlTemplatePropertyType& value);

				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				collections::ObservableListBase<GuiControl*>&	GetToolstripItems();
			};

			/// <summary>A toolstrip item, which is also a toolstrip item container.</summary>
			class GuiToolstripGroup : public GuiToolstripNestedContainer, public Description<GuiToolstripGroup>
			{
			protected:
				compositions::GuiStackComposition*				stackComposition;
				Ptr<GuiToolstripCollection>						toolstripItems;

				void											OnParentLineChanged()override;
			public:
				GuiToolstripGroup(theme::ThemeName themeName);
				~GuiToolstripGroup();

				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				collections::ObservableListBase<GuiControl*>&	GetToolstripItems();
			};
		}
	}

	namespace collections
	{
		namespace randomaccess_internal
		{
			template<>
			struct RandomAccessable<presentation::controls::GuiToolstripCollection>
			{
				static const bool							CanRead = true;
				static const bool							CanResize = false;
			};
		}
	}
}

#endif
