#include "GuiGraphicsComposition.h"
#include "../Controls/GuiWindowControls.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace controls;

/***********************************************************************
Helper Functions
***********************************************************************/

			void NotifyFinalizeInstance(controls::GuiControl* value)
			{
				if (value)
				{
					NotifyFinalizeInstance(value->GetBoundsComposition());
				}
			}

			void NotifyFinalizeInstance(GuiGraphicsComposition* value)
			{
				if (value)
				{
					bool finalized = false;
					if (auto root = dynamic_cast<GuiInstanceRootObject*>(value))
					{
						if (root->IsFinalized())
						{
							finalized = true;
						}
						else
						{
							root->FinalizeInstance();
						}
					}

					if (auto control = value->GetAssociatedControl())
					{
						if (auto root = dynamic_cast<GuiInstanceRootObject*>(control))
						{
							if (root->IsFinalized())
							{
								finalized = true;
							}
							else
							{
								root->FinalizeInstance();
							}
						}
					}

					if (!finalized)
					{
						for (auto child : value->Children())
						{
							NotifyFinalizeInstance(child);
						}
					}
				}
			}

			void SafeDeleteControlInternal(controls::GuiControl* value)
			{
				if(value)
				{
					if (value->GetRelatedControlHost() != value)
					{
						GuiGraphicsComposition* bounds = value->GetBoundsComposition();
						if (bounds->GetParent())
						{
							bounds->GetParent()->RemoveChild(bounds);
						}
					}
					delete value;
				}
			}

			void SafeDeleteCompositionInternal(GuiGraphicsComposition* value)
			{
				if (value)
				{
					if (value->GetParent())
					{
						value->GetParent()->RemoveChild(value);
					}

					if (value->GetAssociatedControl())
					{
						SafeDeleteControlInternal(value->GetAssociatedControl());
					}
					else
					{
						// TODO: (enumerable) foreach:reversed
						for (vint i = value->Children().Count() - 1; i >= 0; i--)
						{
							SafeDeleteCompositionInternal(value->Children().Get(i));
						}
						delete value;
					}
				}
			}

			void SafeDeleteControl(controls::GuiControl* value)
			{
				if (auto controlHost = dynamic_cast<controls::GuiControlHost*>(value))
				{
					controlHost->DeleteAfterProcessingAllEvents({});
				}
				else
				{
					NotifyFinalizeInstance(value);
					SafeDeleteControlInternal(value);
				}
			}

			void SafeDeleteComposition(GuiGraphicsComposition* value)
			{
				NotifyFinalizeInstance(value);
				SafeDeleteCompositionInternal(value);
			}
		}
	}
}