/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSSHAREDSIZECOMPOSITION
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSSHAREDSIZECOMPOSITION

#include "IncludeForward.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			/// <summary>A shared size composition that shares the same size with all other <see cref="GuiSharedSizeItemComposition"/> that has a same group name.</summary>
			class GuiSharedSizeItemComposition : public GuiBoundsComposition, public Description<GuiSharedSizeItemComposition>
			{
			protected:
				GuiSharedSizeRootComposition*						parentRoot = nullptr;
				WString												group;
				bool												sharedWidth = false;
				bool												sharedHeight = false;

				void												Update();
				void												OnParentLineChanged()override;
			public:
				GuiSharedSizeItemComposition();
				~GuiSharedSizeItemComposition();
				
				/// <summary>Get the group name of this item.</summary>
				/// <returns>The group name.</returns>
				const WString&										GetGroup();
				/// <summary>Set the group name of this item.</summary>
				/// <param name="value">The group name.</param>
				void												SetGroup(const WString& value);
				/// <summary>Test is the width of this item is shared.</summary>
				/// <returns>Returns true if the width of this item is shared.</returns>
				bool												GetSharedWidth();
				/// <summary>Enable or disable sharing the width of this item.</summary>
				/// <param name="value">Set to true to share the width of this item.</param>
				void												SetSharedWidth(bool value);
				/// <summary>Test is the height of this item is shared.</summary>
				/// <returns>Returns true if the height of this item is shared.</returns>
				bool												GetSharedHeight();
				/// <summary>Enable or disable sharing the height of this item.</summary>
				/// <param name="value">Set to true to share the height of this item.</param>
				void												SetSharedHeight(bool value);
			};

			/// <summary>A root composition that takes care of all direct or indirect <see cref="GuiSharedSizeItemComposition"/> to enable size sharing.</summary>
			class GuiSharedSizeRootComposition :public GuiBoundsComposition, public Description<GuiSharedSizeRootComposition>
			{
				friend class GuiSharedSizeItemComposition;
			protected:
				collections::Dictionary<WString, vint>				itemWidths;
				collections::Dictionary<WString, vint>				itemHeights;
				collections::List<GuiSharedSizeItemComposition*>	childItems;

				void												AddSizeComponent(collections::Dictionary<WString, vint>& sizes, const WString& group, vint sizeComponent);
				void												CollectSizes(collections::Dictionary<WString, vint>& widths, collections::Dictionary<WString, vint>& heights);
				void												AlignSizes(collections::Dictionary<WString, vint>& widths, collections::Dictionary<WString, vint>& heights);
			public:
				GuiSharedSizeRootComposition();
				~GuiSharedSizeRootComposition();

				void												ForceCalculateSizeImmediately()override;
				Rect												GetBounds()override;
			};
		}
	}
}

#endif