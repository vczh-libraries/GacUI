#include "GuiGraphicsResponsiveComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;

/***********************************************************************
GuiResponsiveCompositionBase
***********************************************************************/

			void GuiResponsiveCompositionBase::OnParentLineChanged()
			{
				GuiBoundsComposition::OnParentLineChanged();
				auto parent = GetParent();
				while (parent)
				{
					if (auto responsive = dynamic_cast<GuiResponsiveCompositionBase*>(parent))
					{
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
						break;
					}
					parent = parent->GetParent();
				}
			}

			void GuiResponsiveCompositionBase::NotifyLevelUpdated()
			{
				auto current = this;
				while (current)
				{
					if (current->responsiveParent)
					{
						current->responsiveParent->OnResponsiveChildLevelUpdated();
					}
					current = current->responsiveParent;
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
			}

			GuiResponsiveCompositionBase::GuiResponsiveCompositionBase()
			{
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
					NotifyLevelUpdated();
				}
			}

/***********************************************************************
GuiResponsiveViewComposition
***********************************************************************/

			void GuiResponsiveViewComposition::CalculateLevelCount()
			{
				throw 0;
			}

			void GuiResponsiveViewComposition::CalculateCurrentLevel()
			{
				throw 0;
			}

			void GuiResponsiveViewComposition::OnResponsiveChildLevelUpdated()
			{
				CalculateLevelCount();
				CalculateCurrentLevel();
			}

			GuiResponsiveViewComposition::GuiResponsiveViewComposition()
			{
			}

			GuiResponsiveViewComposition::~GuiResponsiveViewComposition()
			{
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
				throw 0;
			}

			bool GuiResponsiveViewComposition::LevelUp()
			{
				throw 0;
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

			void GuiResponsiveStackComposition::CalculateLevelCount()
			{
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
						.Aggregate([](vint a, vint b)
							{
								return a + b;
							});
				}
			}

			void GuiResponsiveStackComposition::CalculateCurrentLevel()
			{
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
								return child->GetCurrentLevel() + 1;
							})
						.Aggregate([](vint a, vint b)
							{
								return a + b;
							}) - 1;
				}
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
			}

			bool GuiResponsiveStackComposition::ChangeLevel(bool levelDown)
			{
				DEFINE_AVAILABLE;

				vint level = currentLevel;
				SortedList<GuiResponsiveCompositionBase*> ignored;
				while (true)
				{
					GuiResponsiveCompositionBase* selected = false;
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

					if (selected)
					{
						if (!(levelDown ? selected->LevelDown() : selected->LevelUp()))
						{
							ignored.Add(selected);
						}
					}
					else
					{
						break;
					}
				}

				CalculateCurrentLevel();
				return level != currentLevel;
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

			void GuiResponsiveGroupComposition::CalculateLevelCount()
			{
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
			}

			void GuiResponsiveGroupComposition::CalculateCurrentLevel()
			{
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
						child->LevelDown();
					}
				}

				CalculateCurrentLevel();
				return level != currentLevel;
			}

			bool GuiResponsiveGroupComposition::LevelUp()
			{
				DEFINE_AVAILABLE;
				vint level = currentLevel;
				FOREACH(GuiResponsiveCompositionBase*, child, availables)
				{
					while (child->GetCurrentLevel() <= level)
					{
						child->LevelUp();
					}
				}

				CalculateCurrentLevel();
				return level != currentLevel;
			}

#undef DEFINE_AVAILABLE
		}
	}
}
