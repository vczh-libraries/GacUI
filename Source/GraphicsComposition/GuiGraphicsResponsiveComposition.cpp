#include "GuiGraphicsResponsiveComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

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
				direction = value;
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

			void GuiResponsiveStackComposition::CalculateLevelCount()
			{
				if (responsiveChildren.Count() == 0)
				{
					levelCount = 1;
				}
				else
				{
					levelCount = From(responsiveChildren)
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
				if (responsiveChildren.Count() == 0)
				{
					currentLevel = 0;
				}
				else
				{
					currentLevel = From(responsiveChildren)
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
				throw 0;
			}

			bool GuiResponsiveStackComposition::LevelUp()
			{
				throw 0;
			}

/***********************************************************************
GuiResponsiveGroupComposition
***********************************************************************/

			void GuiResponsiveGroupComposition::CalculateLevelCount()
			{
				if (responsiveChildren.Count() == 0)
				{
					levelCount = 1;
				}
				else
				{
					levelCount = From(responsiveChildren)
						.Select([](GuiResponsiveCompositionBase* child)
							{
								return child->GetLevelCount();
							})
						.Max();
				}
			}

			void GuiResponsiveGroupComposition::CalculateCurrentLevel()
			{
				if (responsiveChildren.Count() == 0)
				{
					currentLevel = 0;
				}
				else
				{
					currentLevel = From(responsiveChildren)
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
				throw 0;
			}

			bool GuiResponsiveGroupComposition::LevelUp()
			{
				throw 0;
			}
		}
	}
}
