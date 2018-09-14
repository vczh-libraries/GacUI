/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITOOLSTRIPCOMMAND
#define VCZH_PRESENTATION_CONTROLS_GUITOOLSTRIPCOMMAND

#include "../GuiBasicControls.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			class IGuiShortcutKeyItem;
		}

		namespace controls
		{
			/// <summary>A command for toolstrip controls.</summary>
			class GuiToolstripCommand : public GuiComponent, public Description<GuiToolstripCommand>
			{
			public:
				class ShortcutBuilder : public Object
				{
				public:
					WString									text;
					bool									ctrl;
					bool									shift;
					bool									alt;
					VKEY									key;
				};
			protected:
				Ptr<GuiImageData>							image;
				Ptr<GuiImageData>							largeImage;
				WString										text;
				compositions::IGuiShortcutKeyItem*			shortcutKeyItem = nullptr;
				bool										enabled = true;
				bool										selected = false;
				Ptr<compositions::IGuiGraphicsEventHandler>	shortcutKeyItemExecutedHandler;
				Ptr<ShortcutBuilder>						shortcutBuilder;

				GuiInstanceRootObject*						attachedRootObject = nullptr;
				Ptr<compositions::IGuiGraphicsEventHandler>	renderTargetChangedHandler;
				GuiControlHost*								shortcutOwner = nullptr;

				void										OnShortcutKeyItemExecuted(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnRenderTargetChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										InvokeDescriptionChanged();
				void										ReplaceShortcut(compositions::IGuiShortcutKeyItem* value, Ptr<ShortcutBuilder> builder);
				void										BuildShortcut(const WString& builderText);
				void										UpdateShortcutOwner();
			public:
				/// <summary>Create the command.</summary>
				GuiToolstripCommand();
				~GuiToolstripCommand();

				void										Attach(GuiInstanceRootObject* rootObject)override;
				void										Detach(GuiInstanceRootObject* rootObject)override;

				/// <summary>Executed event.</summary>
				compositions::GuiNotifyEvent				Executed;

				/// <summary>Description changed event, raised when any description property is modified.</summary>
				compositions::GuiNotifyEvent				DescriptionChanged;

				/// <summary>Get the large image for this command.</summary>
				/// <returns>The large image for this command.</returns>
				Ptr<GuiImageData>							GetLargeImage();
				/// <summary>Set the large image for this command.</summary>
				/// <param name="value">The large image for this command.</param>
				void										SetLargeImage(Ptr<GuiImageData> value);
				/// <summary>Get the image for this command.</summary>
				/// <returns>The image for this command.</returns>
				Ptr<GuiImageData>							GetImage();
				/// <summary>Set the image for this command.</summary>
				/// <param name="value">The image for this command.</param>
				void										SetImage(Ptr<GuiImageData> value);
				/// <summary>Get the text for this command.</summary>
				/// <returns>The text for this command.</returns>
				const WString&								GetText();
				/// <summary>Set the text for this command.</summary>
				/// <param name="value">The text for this command.</param>
				void										SetText(const WString& value);
				/// <summary>Get the shortcut key item for this command.</summary>
				/// <returns>The shortcut key item for this command.</returns>
				compositions::IGuiShortcutKeyItem*			GetShortcut();
				/// <summary>Set the shortcut key item for this command.</summary>
				/// <param name="value">The shortcut key item for this command.</param>
				void										SetShortcut(compositions::IGuiShortcutKeyItem* value);
				/// <summary>Get the shortcut builder for this command.</summary>
				/// <returns>The shortcut builder for this command.</returns>
				WString										GetShortcutBuilder();
				/// <summary>Set the shortcut builder for this command. When the command is attached to a window as a component without a shortcut, the command will try to convert the shortcut builder to a shortcut key item.</summary>
				/// <param name="value">The shortcut builder for this command.</param>
				void										SetShortcutBuilder(const WString& value);
				/// <summary>Get the enablility for this command.</summary>
				/// <returns>The enablility for this command.</returns>
				bool										GetEnabled();
				/// <summary>Set the enablility for this command.</summary>
				/// <param name="value">The enablility for this command.</param>
				void										SetEnabled(bool value);
				/// <summary>Get the selection for this command.</summary>
				/// <returns>The selection for this command.</returns>
				bool										GetSelected();
				/// <summary>Set the selection for this command.</summary>
				/// <param name="value">The selection for this command.</param>
				void										SetSelected(bool value);
			};
		}
	}
}

#endif