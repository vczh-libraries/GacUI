/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSRESPONSIVECOMPOSITION
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSRESPONSIVECOMPOSITION

#include "IncludeForward.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiResponsiveCompositionBase
***********************************************************************/

			enum class ResponsiveDirection
			{
				Horizontal = 1,
				Vertical = 2,
				Both = 3,
			};

			/// <summary>Base class for responsive layout compositions.</summary>
			class GuiResponsiveCompositionBase abstract : public GuiBoundsComposition, public Description<GuiResponsiveCompositionBase>
			{
			protected:
				GuiResponsiveCompositionBase*		responsiveParent = nullptr;
				ResponsiveDirection					direction = ResponsiveDirection::Both;

				void								OnParentLineChanged()override;
				virtual void						OnResponsiveChildInserted(GuiResponsiveCompositionBase* child);
				virtual void						OnResponsiveChildRemoved(GuiResponsiveCompositionBase* child);
				virtual void						OnResponsiveChildLevelUpdated();

			public:
				GuiResponsiveCompositionBase();
				~GuiResponsiveCompositionBase();

				/// <summary>LevelCount changed event.</summary>
				GuiNotifyEvent						LevelCountChanged;
				/// <summary>CurrentLevel chagned event.</summary>
				GuiNotifyEvent						CurrentLevelChanged;

				/// <summary>Get the level count. A level count represents how many views this composition carries.</summary>
				/// <returns>The level count.</returns>
				virtual vint						GetLevelCount() = 0;
				/// <summary>Get the current level. Zero is the view with the smallest size.</summary>
				/// <returns>The current level.</returns>
				virtual vint						GetCurrentLevel() = 0;
				/// <summary>Switch to a smaller view.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool						LevelDown() = 0;
				/// <summary>Switch to a larger view.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool						LevelUp() = 0;

				/// <summary>Get all supported directions. If all directions of a child [T:vl.presentation.compositions.GuiResponsiveCompositionBase] are not supported, its view will not be changed when the parent composition changes its view .</summary>
				/// <returns>All supported directions.</returns>
				ResponsiveDirection					GetDirection();
				/// <summary>Set all supported directions.</summary>
				/// <param name="value">All supported directions.</param>
				void								SetDirection(ResponsiveDirection value);
			};

/***********************************************************************
GuiResponsiveViewComposition
***********************************************************************/

			class GuiResponsiveSharedCollection : public collections::ObservableListBase<controls::GuiControl*>
			{
			protected:
				GuiResponsiveViewComposition*		view = nullptr;

				void								BeforeInsert(vint index, controls::GuiControl* const& value)override;
				void								AfterInsert(vint index, controls::GuiControl* const& value)override;
				void								BeforeRemove(vint index, controls::GuiControl* const& value)override;
				void								AfterRemove(vint index, vint count)override;

			public:
				GuiResponsiveSharedCollection(GuiResponsiveViewComposition* _view);
				~GuiResponsiveSharedCollection();
			};

			class GuiResponsiveViewCollection : public collections::ObservableListBase<GuiResponsiveCompositionBase*>
			{
			protected:
				GuiResponsiveViewComposition*		view = nullptr;

				void								BeforeInsert(vint index, GuiResponsiveCompositionBase* const& value)override;
				void								AfterInsert(vint index, GuiResponsiveCompositionBase* const& value)override;
				void								BeforeRemove(vint index, GuiResponsiveCompositionBase* const& value)override;
				void								AfterRemove(vint index, vint count)override;

			public:
				GuiResponsiveViewCollection(GuiResponsiveViewComposition* _view);
				~GuiResponsiveViewCollection();
			};

			/// <summary>Represents a composition, which will pick up a shared control and install inside it, when it is displayed by a [T:vl.presentation.compositions.GuiResponsiveViewComposition]</summary>
			class GuiResponsiveSharedComposition : public GuiBoundsComposition, public Description<GuiResponsiveSharedComposition>
			{
			protected:
				GuiResponsiveViewComposition*		view = nullptr;
				controls::GuiControl*				shared = nullptr;

				void								SetSharedControl();
				void								OnParentLineChanged()override;

			public:
				GuiResponsiveSharedComposition();
				~GuiResponsiveSharedComposition();

				/// <summary>Get the selected shared control.</summary>
				/// <returns>The selected shared control.</returns>
				controls::GuiControl*				GetShared();
				/// <summary>Set the selected shared control, which should be stored in [M:vl.presentation.compositions.GuiResponsiveViewComposition.GetSharedControls].</summary>
				/// <param name="value">The selected shared control.</param>
				void								SetShared(controls::GuiControl* value);
			};

			/// <summary>A responsive layout composition defined by views of different sizes.</summary>
			class GuiResponsiveViewComposition : public GuiResponsiveCompositionBase, public Description<GuiResponsiveViewComposition>
			{
				friend class GuiResponsiveSharedCollection;
				friend class GuiResponsiveViewCollection;
				friend class GuiResponsiveSharedComposition;
				using ControlSet = collections::SortedList<controls::GuiControl*>;
			protected:
				vint								levelCount = 1;
				vint								currentLevel = 0;
				bool								skipUpdatingLevels = false;
				GuiResponsiveCompositionBase*		currentView = nullptr;

				ControlSet							usedSharedControls;
				GuiResponsiveSharedCollection		sharedControls;
				GuiResponsiveViewCollection			views;
				bool								destructing = false;

				bool								CalculateLevelCount();
				bool								CalculateCurrentLevel();
				void								OnResponsiveChildLevelUpdated()override;

			public:
				GuiResponsiveViewComposition();
				~GuiResponsiveViewComposition();

				/// <summary>Before switch view event. This event happens between hiding the previous view and showing the next view. The itemIndex field can be used to access [M:vl.presentation.compositions.GuiResponsiveViewComposition.GetViews], it is not the level number.</summary>
				GuiItemNotifyEvent													BeforeSwitchingView;

				vint																GetLevelCount()override;
				vint																GetCurrentLevel()override;
				bool																LevelDown()override;
				bool																LevelUp()override;

				/// <summary>Get the current displaying view.</summary>
				/// <returns>The current displaying view.</returns>
				GuiResponsiveCompositionBase*										GetCurrentView();

				/// <summary>Get all shared controls. A shared control can jump between different views if it is contained in a [T:vl.presentation.compositions.GuiResponsiveSharedComposition]. This helps to keep control states during switching views.</summary>
				/// <returns>All shared controls.</returns>
				collections::ObservableListBase<controls::GuiControl*>&				GetSharedControls();

				/// <summary>Get all individual views to switch.</summary>
				/// <returns>All individual views to switch.</returns>
				collections::ObservableListBase<GuiResponsiveCompositionBase*>&		GetViews();
			};

/***********************************************************************
Others
***********************************************************************/

			/// <summary>A responsive layout composition which stop parent responsive composition to search its children.</summary>
			class GuiResponsiveFixedComposition : public GuiResponsiveCompositionBase, public Description<GuiResponsiveFixedComposition>
			{
			protected:
				void					OnResponsiveChildLevelUpdated()override;

			public:
				GuiResponsiveFixedComposition();
				~GuiResponsiveFixedComposition();

				vint					GetLevelCount()override;
				vint					GetCurrentLevel()override;
				bool					LevelDown()override;
				bool					LevelUp()override;
			};

			/// <summary>A responsive layout composition which change its size by changing children's views one by one in one direction.</summary>
			class GuiResponsiveStackComposition : public GuiResponsiveCompositionBase, public Description<GuiResponsiveStackComposition>
			{
				using ResponsiveChildList = collections::List<GuiResponsiveCompositionBase*>;
			protected:
				vint					levelCount = 1;
				vint					currentLevel = 0;
				ResponsiveChildList		responsiveChildren;

				bool					CalculateLevelCount();
				bool					CalculateCurrentLevel();
				void					OnResponsiveChildInserted(GuiResponsiveCompositionBase* child)override;
				void					OnResponsiveChildRemoved(GuiResponsiveCompositionBase* child)override;
				void					OnResponsiveChildLevelUpdated()override;
				bool					ChangeLevel(bool levelDown);

			public:
				GuiResponsiveStackComposition();
				~GuiResponsiveStackComposition();

				vint					GetLevelCount()override;
				vint					GetCurrentLevel()override;
				bool					LevelDown()override;
				bool					LevelUp()override;
			};

			/// <summary>A responsive layout composition which change its size by changing children's views at the same time.</summary>
			class GuiResponsiveGroupComposition : public GuiResponsiveCompositionBase, public Description<GuiResponsiveGroupComposition>
			{
				using ResponsiveChildList = collections::List<GuiResponsiveCompositionBase*>;
			protected:
				vint					levelCount = 1;
				vint					currentLevel = 0;
				ResponsiveChildList		responsiveChildren;

				bool					CalculateLevelCount();
				bool					CalculateCurrentLevel();
				void					OnResponsiveChildInserted(GuiResponsiveCompositionBase* child)override;
				void					OnResponsiveChildRemoved(GuiResponsiveCompositionBase* child)override;
				void					OnResponsiveChildLevelUpdated()override;

			public:
				GuiResponsiveGroupComposition();
				~GuiResponsiveGroupComposition();

				vint					GetLevelCount()override;
				vint					GetCurrentLevel()override;
				bool					LevelDown()override;
				bool					LevelUp()override;
			};

/***********************************************************************
GuiResponsiveContainerComposition
***********************************************************************/

			/// <summary>A composition which will automatically tell its target responsive composition to switch between views according to its size.</summary>
			class GuiResponsiveContainerComposition : public GuiBoundsComposition, public Description<GuiResponsiveContainerComposition>
			{
			protected:
				GuiResponsiveCompositionBase*			responsiveTarget = nullptr;
				Size									upperLevelSize;

				void									AdjustLevel();
				void									OnBoundsChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments);

			public:
				GuiResponsiveContainerComposition();
				~GuiResponsiveContainerComposition();

				/// <summary>Get the responsive composition to control.</summary>
				/// <returns>The responsive composition to control.</returns>
				GuiResponsiveCompositionBase*			GetResponsiveTarget();
				/// <summary>Get the responsive composition to control.</summary>
				/// <param name="value">The responsive composition to control.</param>
				void									SetResponsiveTarget(GuiResponsiveCompositionBase* value);
			};
		}
	}
}

#endif
