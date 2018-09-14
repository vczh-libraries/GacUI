/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Graphics Composition Host

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST_ALT
#define VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST_ALT

#include "../GraphicsComposition/IncludeForward.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
Alt-Combined Shortcut Key Interfaces
***********************************************************************/

			class IGuiAltActionHost;
			
			/// <summary>IGuiAltAction is the handler when an alt-combined shortcut key is activated.</summary>
			class IGuiAltAction : public virtual IDescriptable
			{
			public:
				/// <summary>The identifier for this service.</summary>
				static const wchar_t* const				Identifier;

				static bool								IsLegalAlt(const WString& alt);

				virtual const WString&					GetAlt() = 0;
				virtual bool							IsAltEnabled() = 0;
				virtual bool							IsAltAvailable() = 0;
				virtual GuiGraphicsComposition*			GetAltComposition() = 0;
				virtual IGuiAltActionHost*				GetActivatingAltHost() = 0;
				virtual void							OnActiveAlt() = 0;
			};
			
			/// <summary>IGuiAltActionContainer enumerates multiple <see cref="IGuiAltAction"/>.</summary>
			class IGuiAltActionContainer : public virtual IDescriptable
			{
			public:
				/// <summary>The identifier for this service.</summary>
				static const wchar_t* const				Identifier;

				virtual vint							GetAltActionCount() = 0;
				virtual IGuiAltAction*					GetAltAction(vint index) = 0;
			};
			
			/// <summary>IGuiAltActionHost is an alt-combined shortcut key host. A host can also be entered or leaved, with multiple sub actions enabled or disabled.</summary>
			class IGuiAltActionHost : public virtual IDescriptable
			{
			public:
				/// <summary>The identifier for this service.</summary>
				static const wchar_t* const				Identifier;

				static void								CollectAltActionsFromControl(controls::GuiControl* control, bool includeThisControl, collections::Group<WString, IGuiAltAction*>& actions);
				
				virtual GuiGraphicsComposition*			GetAltComposition() = 0;
				virtual IGuiAltActionHost*				GetPreviousAltHost() = 0;
				virtual void							OnActivatedAltHost(IGuiAltActionHost* previousHost) = 0;
				virtual void							OnDeactivatedAltHost() = 0;
				virtual void							CollectAltActions(collections::Group<WString, IGuiAltAction*>& actions) = 0;
			};

			/// <summary>Default implementation for <see cref="IGuiAltActionHost"/></summary>
			class GuiAltActionHostBase : public virtual IGuiAltActionHost
			{
			private:
				GuiGraphicsComposition*					composition = nullptr;
				controls::GuiControl*					control = nullptr;
				bool									includeControl = true;
				IGuiAltActionHost*						previousHost = nullptr;

			protected:
				void									SetAltComposition(GuiGraphicsComposition* _composition);
				void									SetAltControl(controls::GuiControl* _control, bool _includeControl);

			public:
				GuiGraphicsComposition*					GetAltComposition()override;
				IGuiAltActionHost*						GetPreviousAltHost()override;
				void									OnActivatedAltHost(IGuiAltActionHost* _previousHost)override;
				void									OnDeactivatedAltHost()override;
				void									CollectAltActions(collections::Group<WString, IGuiAltAction*>& actions)override;
			};

/***********************************************************************
Alt-Combined Shortcut Key Interfaces Helpers
***********************************************************************/

			class GuiAltActionManager : public Object
			{
				typedef collections::Dictionary<WString, IGuiAltAction*>					AltActionMap;
				typedef collections::Dictionary<WString, controls::GuiControl*>				AltControlMap;
			protected:
				controls::GuiControlHost*				controlHost = nullptr;
				IGuiAltActionHost*						currentAltHost = nullptr;
				AltActionMap							currentActiveAltActions;
				AltControlMap							currentActiveAltTitles;
				WString									currentAltPrefix;
				VKEY									supressAltKey = VKEY::_UNKNOWN;

				void									EnterAltHost(IGuiAltActionHost* host);
				void									LeaveAltHost();
				bool									EnterAltKey(wchar_t key);
				void									LeaveAltKey();
				void									CreateAltTitles(const collections::Group<WString, IGuiAltAction*>& actions);
				vint									FilterTitles();
				void									ClearAltHost();
			public:
				GuiAltActionManager(controls::GuiControlHost* _controlHost);
				~GuiAltActionManager();

				void									CloseAltHost();
				bool									KeyDown(const NativeWindowKeyInfo& info);
				bool									KeyUp(const NativeWindowKeyInfo& info);
				bool									SysKeyDown(const NativeWindowKeyInfo& info);
				bool									SysKeyUp(const NativeWindowKeyInfo& info);
				bool									Char(const NativeWindowCharInfo& info);
			};
		}
	}
}

#endif