/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSRESPONSIVECOMPOSITION
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSRESPONSIVECOMPOSITION

#include "GuiGraphicsBasicComposition.h"

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

				GuiNotifyEvent						LevelCountChanged;
				GuiNotifyEvent						CurrentLevelChanged;

				virtual vint						GetLevelCount() = 0;
				virtual vint						GetCurrentLevel() = 0;
				virtual bool						LevelDown() = 0;
				virtual bool						LevelUp() = 0;

				ResponsiveDirection					GetDirection();
				void								SetDirection(ResponsiveDirection value);
			};

/***********************************************************************
GuiResponsiveViewComposition
***********************************************************************/

			class GuiResponsiveViewComposition;
			class GuiResponsiveSharedComposition;

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

				controls::GuiControl*				GetShared();
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
				void								OnResponsiveChildLevelUpdated();

			public:
				GuiResponsiveViewComposition();
				~GuiResponsiveViewComposition();

				vint																GetLevelCount()override;
				vint																GetCurrentLevel()override;
				bool																LevelDown()override;
				bool																LevelUp()override;

				collections::ObservableListBase<controls::GuiControl*>&				GetSharedControls();
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
				void					OnResponsiveChildInserted(GuiResponsiveCompositionBase* child);
				void					OnResponsiveChildRemoved(GuiResponsiveCompositionBase* child);
				void					OnResponsiveChildLevelUpdated();
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
				void					OnResponsiveChildInserted(GuiResponsiveCompositionBase* child);
				void					OnResponsiveChildRemoved(GuiResponsiveCompositionBase* child);
				void					OnResponsiveChildLevelUpdated();

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

			class GuiResponsiveContainerComposition : public GuiBoundsComposition, public Description<GuiResponsiveContainerComposition>
			{
			protected:
				GuiResponsiveCompositionBase*			responsiveTarget = nullptr;

			public:
				GuiResponsiveContainerComposition();
				~GuiResponsiveContainerComposition();

				GuiResponsiveCompositionBase*			GetResponsiveTarget();
				void									SetResponsiveTarget(GuiResponsiveCompositionBase* value);
			};
		}
	}
}

#endif