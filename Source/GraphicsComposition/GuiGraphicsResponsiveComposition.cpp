#include "GuiGraphicsResponsiveComposition.h"
#include "../Controls/GuiBasicControls.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;
			using namespace controls;

/***********************************************************************
GuiResponsiveCompositionBase
***********************************************************************/

			void GuiResponsiveCompositionBase::OnParentLineChanged()
			{
				GuiBoundsComposition::OnParentLineChanged();
				GuiResponsiveCompositionBase* responsive = nullptr;
				{
					auto parent = GetParent();
					while (parent)
					{
						if ((responsive = dynamic_cast<GuiResponsiveCompositionBase*>(parent)))
						{
							break;
						}
						parent = parent->GetParent();
					}
				}

				if (responsiveParent != responsive)
				{
					if (responsiveParent)
					{
						responsiveParent->OnResponsiveChildRemoved(this);
						responsiveParent->OnResponsiveChildLevelUpdated();
					}
					responsiveParent = responsive;
					if (responsiveParent)
					{
						responsiveParent->OnResponsiveChildInserted(this);
						responsiveParent->OnResponsiveChildLevelUpdated();
					}
				}
			}

			void GuiResponsiveCompositionBase::OnResponsiveChildInserted(GuiResponsiveCompositionBase* child)
			{
			}

			void GuiResponsiveCompositionBase::OnResponsiveChildRemoved(GuiResponsiveCompositionBase* child)
			{
			}

			void GuiResponsiveCompositionBase::OnResponsiveChildLevelUpdated()
			{
				if (responsiveParent)
				{
					responsiveParent->OnResponsiveChildLevelUpdated();
				}
				else
				{
					InvokeOnCompositionStateChanged();
				}
			}

			GuiResponsiveCompositionBase::GuiResponsiveCompositionBase()
			{
				SetMinSizeLimitation(LimitToElementAndChildren);
				SetPreferredMinSize(Size(1, 1));

				LevelCountChanged.SetAssociatedComposition(this);
				CurrentLevelChanged.SetAssociatedComposition(this);
			}

			GuiResponsiveCompositionBase::~GuiResponsiveCompositionBase()
			{
			}

			ResponsiveDirection GuiResponsiveCompositionBase::GetDirection()
			{
				return direction;
			}

			void GuiResponsiveCompositionBase::SetDirection(ResponsiveDirection value)
			{
				if (direction != value)
				{
					direction = value;
					OnResponsiveChildLevelUpdated();
				}
			}

/***********************************************************************
GuiResponsiveSharedCollection
***********************************************************************/

			void GuiResponsiveSharedCollection::BeforeInsert(vint index, controls::GuiControl* const& value)
			{
				CHECK_ERROR(!value->GetBoundsComposition()->GetParent(), L"GuiResponsiveSharedCollection::BeforeInsert(vint, GuiResponsiveSharedCollection* const&)#Cannot insert a shared control that is currently in use.");
			}

			void GuiResponsiveSharedCollection::AfterInsert(vint index, controls::GuiControl* const& value)
			{
				view->OnResponsiveChildLevelUpdated();
			}

			void GuiResponsiveSharedCollection::BeforeRemove(vint index, controls::GuiControl* const& value)
			{
				CHECK_ERROR(!value->GetBoundsComposition()->GetParent(), L"GuiResponsiveSharedCollection::BeforeRemove(vint, GuiResponsiveSharedCollection* const&)#Cannot remove a shared control that is currently in use.");
			}

			void GuiResponsiveSharedCollection::AfterRemove(vint index, vint count)
			{
				view->OnResponsiveChildLevelUpdated();
			}

			GuiResponsiveSharedCollection::GuiResponsiveSharedCollection(GuiResponsiveViewComposition* _view)
				:view(_view)
			{
			}

			GuiResponsiveSharedCollection::~GuiResponsiveSharedCollection()
			{
			}

/***********************************************************************
GuiResponsiveViewCollection
***********************************************************************/

			void GuiResponsiveViewCollection::BeforeInsert(vint index, GuiResponsiveCompositionBase* const& value)
			{
				CHECK_ERROR(!value->GetParent(), L"GuiResponsiveViewCollection::BeforeRemove(vint, GuiResponsiveCompositionBase* const&)#Cannot insert a view that is currently in use.");
			}

			void GuiResponsiveViewCollection::AfterInsert(vint index, GuiResponsiveCompositionBase* const& value)
			{
				if (!view->currentView)
				{
					view->skipUpdatingLevels = true;
					view->currentView = value;
					view->currentView->SetAlignmentToParent(Margin(0, 0, 0, 0));
					view->AddChild(view->currentView);
					view->skipUpdatingLevels = false;
				}
				view->OnResponsiveChildLevelUpdated();
			}

			void GuiResponsiveViewCollection::BeforeRemove(vint index, GuiResponsiveCompositionBase* const& value)
			{
				CHECK_ERROR(!value->GetParent(), L"GuiResponsiveViewCollection::BeforeRemove(vint, GuiResponsiveCompositionBase* const&)#Cannot remove a view that is currently in use.");
			}

			void GuiResponsiveViewCollection::AfterRemove(vint index, vint count)
			{
				view->OnResponsiveChildLevelUpdated();
			}

			GuiResponsiveViewCollection::GuiResponsiveViewCollection(GuiResponsiveViewComposition* _view)
				:view(_view)
			{
			}

			GuiResponsiveViewCollection::~GuiResponsiveViewCollection()
			{
			}

/***********************************************************************
GuiResponsiveSharedComposition
***********************************************************************/

			void GuiResponsiveSharedComposition::SetSharedControl()
			{
				if (shared && view && !view->destructing)
				{
					auto sharedParent = shared->GetBoundsComposition()->GetParent();
					CHECK_ERROR(view->sharedControls.Contains(shared), L"GuiResponsiveSharedComposition::SetSharedControl()#The specified shared control is not in GuiResponsiveViewComposition::GetSharedControls().");
					CHECK_ERROR(!sharedParent || sharedParent == this, L"GuiResponsiveSharedComposition::SetSharedControl()#The specified shared control has not been released. This usually means this control is not in GuiResponsiveViewComposition::GetSharedControls().");

					if (!sharedParent)
					{
						shared->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
						AddChild(shared->GetBoundsComposition());
						view->usedSharedControls.Add(shared);
					}
				}
			}

			void GuiResponsiveSharedComposition::OnParentLineChanged()
			{
				GuiBoundsComposition::OnParentLineChanged();
				if (view && view->destructing)
				{
					return;
				}

				GuiResponsiveViewComposition* currentView = nullptr;
				{
					auto parent = GetParent();
					while (parent)
					{
						if ((currentView = dynamic_cast<GuiResponsiveViewComposition*>(parent)))
						{
							break;
						}
						parent = parent->GetParent();
					}
				}

				if (currentView != view && view && shared)
				{
					RemoveChild(shared->GetBoundsComposition());
					view->usedSharedControls.Remove(shared);
				}
				view = currentView;

				SetSharedControl();
			}

			GuiResponsiveSharedComposition::GuiResponsiveSharedComposition()
			{
				SetMinSizeLimitation(LimitToElementAndChildren);
			}

			GuiResponsiveSharedComposition::~GuiResponsiveSharedComposition()
			{
			}

			controls::GuiControl* GuiResponsiveSharedComposition::GetShared()
			{
				return shared;
			}

			void GuiResponsiveSharedComposition::SetShared(controls::GuiControl* value)
			{
				if (shared != value)
				{
					CHECK_ERROR(!shared || !shared->GetBoundsComposition()->GetParent(), L"GuiResponsiveSharedComposition::SetShared(GuiControl*)#Cannot replace a shared control that is currently in use.");
					shared = value;
					SetSharedControl();
				}
			}

/***********************************************************************
GuiResponsiveViewComposition
***********************************************************************/

			bool GuiResponsiveViewComposition::CalculateLevelCount()
			{
				vint old = levelCount;
				if (views.Count() == 0)
				{
					levelCount = 1;
				}
				else
				{
					levelCount = 0;
					for (vint i = 0; i < views.Count(); i++)
					{
						auto view = views[i];
						if (((vint)direction & (vint)view->GetDirection()) != 0)
						{
							levelCount += view->GetLevelCount();
						}
						else
						{
							levelCount += 1;
						}
					}
				}

				if (old != levelCount)
				{
					LevelCountChanged.Execute(GuiEventArgs(this));
					return true;
				}
				return false;
			}

			bool GuiResponsiveViewComposition::CalculateCurrentLevel()
			{
				vint old = currentLevel;
				currentLevel = 0;
				for (vint i = views.Count() - 1; i >= 0; i--)
				{
					auto view = views[i];
					if (((vint)direction & (vint)view->GetDirection()) != 0)
					{
						if (currentView == view)
						{
							currentLevel += view->GetCurrentLevel() + 1;
							break;
						}
						else
						{
							currentLevel += view->GetLevelCount();
						}
					}
					else
					{
						currentLevel++;
					}
				}
				currentLevel--;

				if (old != currentLevel)
				{
					CurrentLevelChanged.Execute(GuiEventArgs(this));
					return true;
				}
				return false;
			}

			void GuiResponsiveViewComposition::OnResponsiveChildLevelUpdated()
			{
				if (!skipUpdatingLevels)
				{
					CalculateLevelCount();
					CalculateCurrentLevel();
					GuiResponsiveCompositionBase::OnResponsiveChildLevelUpdated();
				}
			}

			GuiResponsiveViewComposition::GuiResponsiveViewComposition()
				:sharedControls(this)
				, views(this)
			{
				BeforeSwitchingView.SetAssociatedComposition(this);
			}

			GuiResponsiveViewComposition::~GuiResponsiveViewComposition()
			{
				destructing = true;

				FOREACH(GuiResponsiveCompositionBase*, view, views)
				{
					if (view != currentView)
					{
						SafeDeleteComposition(view);
					}
				}

				FOREACH(GuiControl*, shared, From(sharedControls).Except(usedSharedControls))
				{
					SafeDeleteControl(shared);
				}
			}

			vint GuiResponsiveViewComposition::GetLevelCount()
			{
				return levelCount;
			}

			vint GuiResponsiveViewComposition::GetCurrentLevel()
			{
				return currentLevel;
			}

			bool GuiResponsiveViewComposition::LevelDown()
			{
				skipUpdatingLevels = true;
				if (((vint)direction & (vint)currentView->GetDirection()) != 0 && !currentView->LevelDown())
				{
					vint index = views.IndexOf(currentView);
					if (index < views.Count() - 1)
					{
						RemoveChild(currentView);
						currentView = views[index + 1];
						currentView->SetAlignmentToParent(Margin(0, 0, 0, 0));
						{
							GuiItemEventArgs arguments(this);
							arguments.itemIndex = views.IndexOf(currentView);
							BeforeSwitchingView.Execute(arguments);
						}
						AddChild(currentView);
					}
				}
				skipUpdatingLevels = false;
				
				auto x = CalculateLevelCount();
				auto y = CalculateCurrentLevel();
				if (!x && !y) return false;
				InvokeOnCompositionStateChanged();
				return true;
			}

			bool GuiResponsiveViewComposition::LevelUp()
			{
				skipUpdatingLevels = true;
				if (((vint)direction & (vint)currentView->GetDirection()) != 0 && !currentView->LevelUp())
				{
					vint index = views.IndexOf(currentView);
					if (index > 0)
					{
						RemoveChild(currentView);
						currentView = views[index - 1];
						currentView->SetAlignmentToParent(Margin(0, 0, 0, 0));
						{
							GuiItemEventArgs arguments(this);
							arguments.itemIndex = views.IndexOf(currentView);
							BeforeSwitchingView.Execute(arguments);
						}
						AddChild(currentView);
					}
				}
				skipUpdatingLevels = false;

				auto x = CalculateLevelCount();
				auto y = CalculateCurrentLevel();
				if (!x && !y) return false;
				InvokeOnCompositionStateChanged();
				return true;
			}

			GuiResponsiveCompositionBase* GuiResponsiveViewComposition::GetCurrentView()
			{
				return currentView;
			}

			collections::ObservableListBase<controls::GuiControl*>& GuiResponsiveViewComposition::GetSharedControls()
			{
				return sharedControls;
			}

			collections::ObservableListBase<GuiResponsiveCompositionBase*>& GuiResponsiveViewComposition::GetViews()
			{
				return views;
			}

/***********************************************************************
GuiResponsiveFixedComposition
***********************************************************************/

			void GuiResponsiveFixedComposition::OnResponsiveChildLevelUpdated()
			{
				InvokeOnCompositionStateChanged();
			}

			GuiResponsiveFixedComposition::GuiResponsiveFixedComposition()
			{
			}

			GuiResponsiveFixedComposition::~GuiResponsiveFixedComposition()
			{
			}

			vint GuiResponsiveFixedComposition::GetLevelCount()
			{
				return 1;
			}

			vint GuiResponsiveFixedComposition::GetCurrentLevel()
			{
				return 0;
			}

			bool GuiResponsiveFixedComposition::LevelDown()
			{
				return false;
			}

			bool GuiResponsiveFixedComposition::LevelUp()
			{
				return false;
			}

/***********************************************************************
GuiResponsiveStackComposition
***********************************************************************/

#define DEFINE_AVAILABLE \
			auto availables = From(responsiveChildren) \
				.Where([=](GuiResponsiveCompositionBase* child) \
				{ \
					return ((vint)direction & (vint)child->GetDirection()) != 0; \
				}) \

			bool GuiResponsiveStackComposition::CalculateLevelCount()
			{
				vint old = levelCount;
				DEFINE_AVAILABLE;
				if (availables.IsEmpty())
				{
					levelCount = 1;
				}
				else
				{
					levelCount = availables
						.Select([](GuiResponsiveCompositionBase* child)
						{
							return child->GetLevelCount() - 1;
						})
						.Aggregate([](vint a, vint b)
						{
							return a + b;
						}) + 1;
				}

				if (old != levelCount)
				{
					LevelCountChanged.Execute(GuiEventArgs(this));
					return true;
				}
				return false;
			}

			bool GuiResponsiveStackComposition::CalculateCurrentLevel()
			{
				vint old = currentLevel;
				DEFINE_AVAILABLE;
				if (availables.IsEmpty())
				{
					currentLevel = 0;
				}
				else
				{
					currentLevel = availables
						.Select([](GuiResponsiveCompositionBase* child)
						{
							return child->GetCurrentLevel();
						})
						.Aggregate([](vint a, vint b)
						{
							return a + b;
						});
				}

				if (old != currentLevel)
				{
					CurrentLevelChanged.Execute(GuiEventArgs(this));
					return true;
				}
				return false;
			}

			void GuiResponsiveStackComposition::OnResponsiveChildInserted(GuiResponsiveCompositionBase* child)
			{
				responsiveChildren.Add(child);
			}

			void GuiResponsiveStackComposition::OnResponsiveChildRemoved(GuiResponsiveCompositionBase* child)
			{
				responsiveChildren.Remove(child);
			}

			void GuiResponsiveStackComposition::OnResponsiveChildLevelUpdated()
			{
				CalculateLevelCount();
				CalculateCurrentLevel();
				GuiResponsiveCompositionBase::OnResponsiveChildLevelUpdated();
			}

			bool GuiResponsiveStackComposition::ChangeLevel(bool levelDown)
			{
				DEFINE_AVAILABLE;

				SortedList<GuiResponsiveCompositionBase*> ignored;
				while (true)
				{
					GuiResponsiveCompositionBase* selected = nullptr;
					vint size = 0;

					FOREACH(GuiResponsiveCompositionBase*, child, availables)
					{
						if (!ignored.Contains(child))
						{
							Size childSize = child->GetPreferredBounds().GetSize();
							vint childSizeToCompare =
								direction == ResponsiveDirection::Horizontal ? childSize.x :
								direction == ResponsiveDirection::Vertical ? childSize.y :
								childSize.x * childSize.y;

							if (!selected || (levelDown ? size < childSizeToCompare : size > childSizeToCompare))
							{
								selected = child;
								size = childSizeToCompare;
							}
						}
					}

					if (!selected)
					{
						break;
					}
					else if (levelDown ? selected->LevelDown() : selected->LevelUp())
					{
						break;
					}
					else
					{
						ignored.Add(selected);
					}
				}

				if (!CalculateCurrentLevel()) return false;
				InvokeOnCompositionStateChanged();
				return true;
			}

			GuiResponsiveStackComposition::GuiResponsiveStackComposition()
			{
			}

			GuiResponsiveStackComposition::~GuiResponsiveStackComposition()
			{
			}

			vint GuiResponsiveStackComposition::GetLevelCount()
			{
				return levelCount;
			}

			vint GuiResponsiveStackComposition::GetCurrentLevel()
			{
				return currentLevel;
			}

			bool GuiResponsiveStackComposition::LevelDown()
			{
				return ChangeLevel(true);
			}

			bool GuiResponsiveStackComposition::LevelUp()
			{
				return ChangeLevel(false);
			}

/***********************************************************************
GuiResponsiveGroupComposition
***********************************************************************/

			bool GuiResponsiveGroupComposition::CalculateLevelCount()
			{
				vint old = levelCount;
				DEFINE_AVAILABLE;
				if (availables.IsEmpty())
				{
					levelCount = 1;
				}
				else
				{
					levelCount = availables
						.Select([](GuiResponsiveCompositionBase* child)
						{
							return child->GetLevelCount();
						})
						.Max();
				}

				if (old != levelCount)
				{
					LevelCountChanged.Execute(GuiEventArgs(this));
					return true;
				}
				return false;
			}

			bool GuiResponsiveGroupComposition::CalculateCurrentLevel()
			{
				vint old = currentLevel;
				DEFINE_AVAILABLE;
				if (availables.IsEmpty())
				{
					currentLevel = 0;
				}
				else
				{
					currentLevel = availables
						.Select([](GuiResponsiveCompositionBase* child)
						{
							return child->GetCurrentLevel();
						})
						.Max();
				}

				if (old != currentLevel)
				{
					CurrentLevelChanged.Execute(GuiEventArgs(this));
					return true;
				}
				return false;
			}

			void GuiResponsiveGroupComposition::OnResponsiveChildInserted(GuiResponsiveCompositionBase* child)
			{
				responsiveChildren.Add(child);
			}

			void GuiResponsiveGroupComposition::OnResponsiveChildRemoved(GuiResponsiveCompositionBase* child)
			{
				responsiveChildren.Remove(child);
			}

			void GuiResponsiveGroupComposition::OnResponsiveChildLevelUpdated()
			{
				CalculateLevelCount();
				CalculateCurrentLevel();
				GuiResponsiveCompositionBase::OnResponsiveChildLevelUpdated();
			}

			GuiResponsiveGroupComposition::GuiResponsiveGroupComposition()
			{
			}

			GuiResponsiveGroupComposition::~GuiResponsiveGroupComposition()
			{
			}

			vint GuiResponsiveGroupComposition::GetLevelCount()
			{
				return levelCount;
			}

			vint GuiResponsiveGroupComposition::GetCurrentLevel()
			{
				return currentLevel;
			}

			bool GuiResponsiveGroupComposition::LevelDown()
			{
				DEFINE_AVAILABLE;
				vint level = currentLevel;
				FOREACH(GuiResponsiveCompositionBase*, child, availables)
				{
					if (child->GetCurrentLevel() >= level)
					{
						if (!child->LevelDown())
						{
							break;
						}
					}
				}

				if (!CalculateCurrentLevel()) return false;
				InvokeOnCompositionStateChanged();
				return true;
			}

			bool GuiResponsiveGroupComposition::LevelUp()
			{
				DEFINE_AVAILABLE;
				vint level = currentLevel;
				FOREACH(GuiResponsiveCompositionBase*, child, availables)
				{
					while (child->GetCurrentLevel() <= level)
					{
						if (!child->LevelUp())
						{
							break;
						}
					}
				}

				if (!CalculateCurrentLevel()) return false;
				InvokeOnCompositionStateChanged();
				return true;
			}

#undef DEFINE_AVAILABLE

/***********************************************************************
GuiResponsiveContainerComposition
***********************************************************************/

#define RESPONSIVE_INVALID_SIZE Size(-1, -1)

			void GuiResponsiveContainerComposition::AdjustLevel()
			{
				if (!responsiveTarget) return;
				const Size containerSize = GetBounds().GetSize();
				const Size responsiveOriginalSize = responsiveTarget->GetPreferredBounds().GetSize();
				const bool testX = (vint)responsiveTarget->GetDirection() & (vint)ResponsiveDirection::Horizontal;
				const bool testY = (vint)responsiveTarget->GetDirection() & (vint)ResponsiveDirection::Vertical;

#define RESPONSIVE_IF_CONTAINER(OP, SIZE) ((testX && (containerSize).x OP SIZE.x) || (testY && (containerSize).y OP SIZE.y))

				if (upperLevelSize != RESPONSIVE_INVALID_SIZE && RESPONSIVE_IF_CONTAINER(>=, upperLevelSize))
				{
					upperLevelSize = RESPONSIVE_INVALID_SIZE;
				}

				if (upperLevelSize == RESPONSIVE_INVALID_SIZE && RESPONSIVE_IF_CONTAINER(>=, responsiveOriginalSize))
				{
					while (true)
					{
						if (responsiveTarget->GetCurrentLevel() == responsiveTarget->GetLevelCount() - 1)
						{
							break;
						}
						else if (responsiveTarget->LevelUp())
						{
							responsiveTarget->ForceCalculateSizeImmediately();
							auto currentSize = responsiveTarget->GetPreferredBounds().GetSize();
							if (RESPONSIVE_IF_CONTAINER(<, currentSize))
							{
								upperLevelSize = currentSize;
								responsiveTarget->LevelDown();
								break;
							}
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					while (true)
					{
						responsiveTarget->ForceCalculateSizeImmediately();
						auto currentSize = responsiveTarget->GetPreferredBounds().GetSize();
						if (RESPONSIVE_IF_CONTAINER(>=, currentSize))
						{
							break;
						}

						if (responsiveTarget->GetCurrentLevel() == 0)
						{
							break;
						}
						else if(responsiveTarget->LevelDown())
						{
							upperLevelSize = currentSize;
						}
						else
						{
							break;
						}
					}
				}

#undef RESPONSIVE_IF_CONTAINER
			}

			void GuiResponsiveContainerComposition::OnBoundsChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				auto control = GetRelatedControl();
				if (control)
				{
					control->InvokeOrDelayIfRendering([=]()
					{
						AdjustLevel();
					});
				}
				else
				{
					AdjustLevel();
				}
			}

			GuiResponsiveContainerComposition::GuiResponsiveContainerComposition()
				:upperLevelSize(RESPONSIVE_INVALID_SIZE)
			{
				BoundsChanged.AttachMethod(this, &GuiResponsiveContainerComposition::OnBoundsChanged);
			}

			GuiResponsiveContainerComposition::~GuiResponsiveContainerComposition()
			{
			}

			GuiResponsiveCompositionBase* GuiResponsiveContainerComposition::GetResponsiveTarget()
			{
				return responsiveTarget;
			}

			void GuiResponsiveContainerComposition::SetResponsiveTarget(GuiResponsiveCompositionBase* value)
			{
				if (responsiveTarget != value)
				{
					if (responsiveTarget)
					{
						RemoveChild(responsiveTarget);
					}

					responsiveTarget = value;
					upperLevelSize = RESPONSIVE_INVALID_SIZE;

					if (responsiveTarget)
					{
						responsiveTarget->SetAlignmentToParent(Margin(0, 0, 0, 0));
						while (responsiveTarget->LevelUp());
						AddChild(responsiveTarget);

						GuiEventArgs arguments(this);
						OnBoundsChanged(this, arguments);
					}
				}
			}

#undef RESPONSIVE_INVALID_SIZE
		}
	}
}
