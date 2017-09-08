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
			class GuiToolstripCollection : public collections::ObservableListBase<GuiControl*>
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
Toolstrip Container
***********************************************************************/

			/// <summary>Toolstrip menu.</summary>
			class GuiToolstripMenu : public GuiMenu, protected GuiToolstripCollection::IContentCallback,  Description<GuiToolstripMenu>
			{
			protected:
				compositions::GuiSharedSizeRootComposition*	sharedSizeRootComposition;
				compositions::GuiStackComposition*			stackComposition;
				Ptr<GuiToolstripCollection>					toolstripItems;

				void										UpdateLayout()override;
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_controlTemplate">The control template.</param>
				/// <param name="_owner">The owner menu item of the parent menu.</param>
				GuiToolstripMenu(ControlTemplateType* _controlTemplate, GuiControl* _owner);
				~GuiToolstripMenu();
				
				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				GuiToolstripCollection&						GetToolstripItems();
			};

			/// <summary>Toolstrip menu bar.</summary>
			class GuiToolstripMenuBar : public GuiMenuBar, public Description<GuiToolstripMenuBar>
			{
			protected:
				compositions::GuiStackComposition*			stackComposition;
				Ptr<GuiToolstripCollection>					toolstripItems;

			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_controlTemplate">The control template.</param>
				GuiToolstripMenuBar(ControlTemplateType* _controlTemplate);
				~GuiToolstripMenuBar();
				
				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				GuiToolstripCollection&						GetToolstripItems();
			};

			/// <summary>Toolstrip tool bar.</summary>
			class GuiToolstripToolBar : public GuiControl, public Description<GuiToolstripToolBar>
			{
			protected:
				compositions::GuiStackComposition*			stackComposition;
				Ptr<GuiToolstripCollection>					toolstripItems;

			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_controlTemplate">The control template.</param>
				GuiToolstripToolBar(ControlTemplateType* _controlTemplate);
				~GuiToolstripToolBar();
				
				/// <summary>Get all managed child controls ordered by their positions.</summary>
				/// <returns>All managed child controls.</returns>
				GuiToolstripCollection&						GetToolstripItems();
			};

/***********************************************************************
Toolstrip Component
***********************************************************************/

			/// <summary>Toolstrip button that can connect with a <see cref="GuiToolstripCommand"/>.</summary>
			class GuiToolstripButton : public GuiMenuButton, public Description<GuiToolstripButton>
			{
			protected:
				GuiToolstripCommand*							command;
				Ptr<compositions::IGuiGraphicsEventHandler>		descriptionChangedHandler;

				void											UpdateCommandContent();
				void											OnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void											OnCommandDescriptionChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_controlTemplate">The control template.</param>
				GuiToolstripButton(ControlTemplateType* _controlTemplate);
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
				void											CreateToolstripSubMenu(templates::GuiMenuTemplate* subMenuTemplate = nullptr);
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