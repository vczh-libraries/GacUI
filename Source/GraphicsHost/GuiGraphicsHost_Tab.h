/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Graphics Composition Host

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST_TAB
#define VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST_TAB

#include "../GraphicsComposition/IncludeForward.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
Tab-Combined Shortcut Key Interfaces
***********************************************************************/
			
			/// <summary>IGuiTabAction is the handler when an tab-combined shortcut key is activated.</summary>
			class IGuiTabAction : public virtual IDescriptable
			{
			public:
				/// <summary>The identifier for this service.</summary>
				static const wchar_t* const				Identifier;

				virtual bool							GetAcceptTabInput() = 0;
				virtual vint							GetTabPriority() = 0;
				virtual bool							IsTabEnabled() = 0;
				virtual bool							IsTabAvailable() = 0;
			};

/***********************************************************************
Tab-Combined Shortcut Key Interfaces Helpers
***********************************************************************/

			class GuiTabActionManager : public Object
			{
				using ControlList = collections::List<controls::GuiControl*>;
			protected:
				controls::GuiControlHost*				controlHost = nullptr;
				ControlList								controlsInOrder;
				bool									available = true;
				bool									supressTabOnce = false;

				void									BuildControlList();
				controls::GuiControl*					GetNextFocusControl(controls::GuiControl* focusedControl, vint offset);
			public:
				GuiTabActionManager(controls::GuiControlHost* _controlHost);
				~GuiTabActionManager();

				void									InvalidateTabOrderCache();
				bool									KeyDown(const NativeWindowKeyInfo& info, GuiGraphicsComposition* focusedComposition);
				bool									Char(const NativeWindowCharInfo& info);
			};
		}
	}
}

#endif