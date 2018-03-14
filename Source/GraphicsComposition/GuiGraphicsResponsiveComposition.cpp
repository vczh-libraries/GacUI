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
							if (responsiveParent) responsiveParent->OnResponsiveChildRemoved(this);
							responsiveParent = responsive;
							if (responsiveParent) responsiveParent->OnResponsiveChildInserted(this);
						}
						break;
					}
					parent = parent->GetParent();
				}
			}

			void GuiResponsiveCompositionBase::OnResponsiveChildInserted(GuiResponsiveCompositionBase* child)
			{
			}

			void GuiResponsiveCompositionBase::OnResponsiveChildRemoved(GuiResponsiveCompositionBase* child)
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

			GuiResponsiveViewComposition::GuiResponsiveViewComposition()
			{
			}

			GuiResponsiveViewComposition::~GuiResponsiveViewComposition()
			{
			}

			vint GuiResponsiveViewComposition::GetLevelCount()
			{
				throw 0;
			}

			vint GuiResponsiveViewComposition::GetCurrentLevel()
			{
				throw 0;
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
				throw 0;
			}

			collections::ObservableListBase<GuiResponsiveCompositionBase*>& GuiResponsiveViewComposition::GetViews()
			{
				throw 0;
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

			GuiResponsiveStackComposition::GuiResponsiveStackComposition()
			{
			}

			GuiResponsiveStackComposition::~GuiResponsiveStackComposition()
			{
			}

			vint GuiResponsiveStackComposition::GetLevelCount()
			{
				throw 0;
			}

			vint GuiResponsiveStackComposition::GetCurrentLevel()
			{
				throw 0;
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

			GuiResponsiveGroupComposition::GuiResponsiveGroupComposition()
			{
			}

			GuiResponsiveGroupComposition::~GuiResponsiveGroupComposition()
			{
			}

			vint GuiResponsiveGroupComposition::GetLevelCount()
			{
				throw 0;
			}

			vint GuiResponsiveGroupComposition::GetCurrentLevel()
			{
				throw 0;
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
