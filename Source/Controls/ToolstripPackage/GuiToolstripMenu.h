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
		namespace theme
		{
			class ITheme;
		}

		namespace controls
		{

/***********************************************************************
Toolstrip Item Collection
***********************************************************************/

			/// <summary>Toolstrip item collection.</summary>
			class GuiToolstripCollection : public list::ItemsBase<GuiControl*>
			{
			public:
				class IContentCallback : public Interface
				{
				public:
					virtual void							UpdateLayout()=0;
				};
			protected:
				IContentCallback*							contentCallback;
				compositions::GuiStackComposition*			stackComposition;

				void										InvokeUpdateLayout();
				void										OnInterestingMenuButtonPropertyChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				bool										QueryInsert(vint index, GuiControl* const& child)override;
				bool										QueryRemove(vint index, GuiControl* const& child)override;
				void										BeforeInsert(vint index, GuiControl* const& child)override;
				void										BeforeRemove(vint index, GuiControl* const& child)override;
				void										AfterInsert(vint index, GuiControl* const& child)override;
				void										AfterRemove(vint index, vint count)override;
			public:
				GuiToolstripCollection(IContentCallback* _contentCallback, compositions::GuiStackComposition* _stackComposition);
				~GuiToolstripCollection();
			};

/***********************************************************************
Toolstrip Builder Facade
***********************************************************************/

			class GuiToolstripButton;

			/// <summary>Toolstrip builder.</summary>
			class GuiToolstripBuilder : public Object
			{
				friend class GuiToolstripMenu;
				friend class GuiToolstripMenuBar;
				friend class GuiToolstripToolBar;
			protected:
				enum Environment
				{
					Menu,
					MenuBar,
					ToolBar,
				};

				Environment									environment;
				GuiToolstripCollection*						toolstripItems;
				GuiToolstripBuilder*						previousBuilder;
				theme::ITheme*								theme;
				GuiToolstripButton*							lastCreatedButton;
				
				GuiToolstripBuilder(Environment _environment, GuiToolstripCollection* _toolstripItems);
			public:
				~GuiToolstripBuilder();

				/// <summary>Create a button.</summary>
				/// <returns>The current builder for continuing builder actions.</returns>
				/// <param name="image">The image for the created control.</param>
				/// <param name="text">The text for the created control.</param>
				GuiToolstripBuilder*						Button(Ptr<GuiImageData> image, const WString& text, GuiToolstripButton** result=0);
				/// <summary>Create a button.</summary>
				/// <returns>The current builder for continuing builder actions.</returns>
				/// <param name="command">The command for the created control.</param>
				GuiToolstripBuilder*						Button(GuiToolstripCommand* command, GuiToolstripButton** result=0);
				/// <summary>Create a dropdown button.</summary>
				/// <returns>The current builder for continuing builder actions.</returns>
				/// <param name="image">The image for the created control.</param>
				/// <param name="text">The text for the created control.</param>
				GuiToolstripBuilder*						DropdownButton(Ptr<GuiImageData> image, const WString& text, GuiToolstripButton** result=0);
				/// <summary>Create a dropdown button.</summary>
				/// <returns>The current builder for continuing builder actions.</returns>
				/// <param name="command">The command for the created control.</param>
				GuiToolstripBuilder*						DropdownButton(GuiToolstripCommand* command, GuiToolstripButton** result=0);
				/// <summary>Create a split button.</summary>
				/// <returns>The current builder for continuing builder actions.</returns>
				/// <param name="image">The image for the created control.</param>
				/// <param name="text">The text for the created control.</param>
				GuiToolstripBuilder*						SplitButton(Ptr<GuiImageData> image, const WString& text, GuiToolstripButton** result=0);
				/// <summary>Create a split button.</summary>
				/// <returns>The current builder for continuing builder actions.</returns>
				/// <param name="command">The command for the created control.</param>
				GuiToolstripBuilder*						SplitButton(GuiToolstripCommand* command, GuiToolstripButton** result=0);
				/// <summary>Create a splitter.</summary>
				/// <returns>The current builder for continuing builder actions.</returns>
				GuiToolstripBuilder*						Splitter();
				/// <summary>Install a control.</summary>
				/// <returns>The current builder for continuing builder actions.</returns>
				GuiToolstripBuilder*						Control(GuiControl* control);
				/// <summary>Begin create sub menu.</summary>
				/// <returns>The builder of the last created control's sub menu for continuing builder actions.</returns>
				GuiToolstripBuilder*						BeginSubMenu();
				/// <summary>End create sub menu.</summary>
				/// <returns>The parent builder for continuing builder actions.</returns>
				GuiToolstripBuilder*						EndSubMenu();
			};

/***********************************************************************
Toolstrip Container
***********************************************************************/

			/// <summary>Toolstrip menu.</summary>
			class GuiToolstripMenu : public GuiMenu, protected GuiToolstripCollection::IContentCallback,  Description<GuiToolstripMenu>
			{
			protected:
				compositions::GuiSharedSizeRootComposition*	sharedSizeRootComposition;
				compositions::GuiStackComposition*			stackComposition;
				Ptr<GuiToolstripCollection>					toolstripItems;
				Ptr<GuiToolstripBuilder>					builder;

				void										UpdateLayout()override;
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				/// <param name="_owner">The owner menu item of the parent menu.</param>
				GuiToolstripMenu(IStyleController* _styleController, GuiControl* _owner);
				~GuiToolstripMenu();
				
				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				GuiToolstripCollection&						GetToolstripItems();
				/// <summary>Get the builder facade.</summary>
				/// <returns>The builder facade.</returns>
				/// <param name="themeObject">The theme to use. If this argument is null, the current theme will be used.</param>
				GuiToolstripBuilder*						GetBuilder(theme::ITheme* themeObject=0);
			};

			/// <summary>Toolstrip menu bar.</summary>
			class GuiToolstripMenuBar : public GuiMenuBar, public Description<GuiToolstripMenuBar>
			{
			protected:
				compositions::GuiStackComposition*			stackComposition;
				Ptr<GuiToolstripCollection>					toolstripItems;
				Ptr<GuiToolstripBuilder>					builder;

			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiToolstripMenuBar(IStyleController* _styleController);
				~GuiToolstripMenuBar();
				
				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				GuiToolstripCollection&						GetToolstripItems();
				/// <summary>Get the builder facade.</summary>
				/// <returns>The builder facade.</returns>
				/// <param name="themeObject">The theme to use. If this argument is null, the current theme will be used.</param>
				GuiToolstripBuilder*						GetBuilder(theme::ITheme* themeObject=0);
			};

			/// <summary>Toolstrip tool bar.</summary>
			class GuiToolstripToolBar : public GuiControl, public Description<GuiToolstripToolBar>
			{
			protected:
				compositions::GuiStackComposition*			stackComposition;
				Ptr<GuiToolstripCollection>					toolstripItems;
				Ptr<GuiToolstripBuilder>					builder;

			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiToolstripToolBar(IStyleController* _styleController);
				~GuiToolstripToolBar();
				
				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				GuiToolstripCollection&						GetToolstripItems();
				/// <summary>Get the builder facade.</summary>
				/// <returns>The builder facade.</returns>
				/// <param name="themeObject">The theme to use. If this argument is null, the current theme will be used.</param>
				GuiToolstripBuilder*						GetBuilder(theme::ITheme* themeObject=0);
			};

/***********************************************************************
Toolstrip Component
***********************************************************************/

			/// <summary>Toolstrip button that can connect with a <see cref="GuiToolstripCommand"/>.</summary>
			class GuiToolstripButton : public GuiMenuButton, public Description<GuiToolstripButton>
			{
			protected:
				GuiToolstripCommand*							command;
				Ptr<compositions::GuiNotifyEvent::IHandler>		descriptionChangedHandler;

				void											UpdateCommandContent();
				void											OnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void											OnCommandDescriptionChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiToolstripButton(IStyleController* _styleController);
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
				/// <summary>Create the toolstrip sub menu if necessary. The created toolstrip sub menu is owned by this menu button.</summary>
				/// <param name="subMenuStyleController">The style controller for the toolstrip sub menu. If this argument is null, it will call <see cref="IStyleController::CreateSubMenuStyleController"/> for a style controller.</param>
				void											CreateToolstripSubMenu(GuiToolstripMenu::IStyleController* subMenuStyleController=0);
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