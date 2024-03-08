/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Resource

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_RESOURCES_GUIPLUGINMANAGER
#define VCZH_PRESENTATION_RESOURCES_GUIPLUGINMANAGER

#include "../GuiTypes.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
Plugin
***********************************************************************/

		/// <summary>Represents a plugin for the gui.</summary>
		class IGuiPlugin : public IDescriptable, public Description<IGuiPlugin>
		{
		public:
			/// <summary>Get the name of this plugin.</summary>
			/// <returns>Returns the name of the plugin.</returns>
			virtual WString									GetName() = 0;
			/// <summary>Get all dependencies of this plugin.</summary>
			/// <param name="dependencies">To receive all dependencies.</param>
			virtual void									GetDependencies(collections::List<WString>& dependencies) = 0;
			/// <summary>Called when the plugin manager want to load this plugin.</summary>
			/// <param name="controllerUnrelatedPlugins">A plugin only loads when it does not use anything from a <see cref="INativeController"/>.</param>
			/// <param name="controllerRelatedPlugins">A plugin only loads when it use anything from a <see cref="INativeController"/>.</param>
			virtual void									Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)=0;
			/// <summary>Called when the plugin manager want to unload this plugin.</summary>
			/// <param name="controllerUnrelatedPlugins">A plugin only unloads when it does not use anything from a <see cref="INativeController"/>.</param>
			/// <param name="controllerRelatedPlugins">A plugin only unloads when it use anything from a <see cref="INativeController"/>.</param>
			virtual void									Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)=0;
		};

		/// <summary>Represents a plugin manager.</summary>
		class IGuiPluginManager : public IDescriptable, public Description<IGuiPluginManager>
		{
		public:
			/// <summary>Add a plugin before [F:vl.presentation.controls.IGuiPluginManager.Load] is called.</summary>
			/// <param name="plugin">The plugin.</param>
			virtual void									AddPlugin(Ptr<IGuiPlugin> plugin)=0;
			/// <summary>Load all plugins, and check if dependencies of all plugins are ready.</summary>
			/// <param name="controllerUnrelatedPlugins">Load plugins that does not use anything from a <see cref="INativeController"/>.</param>
			/// <param name="controllerRelatedPlugins">Load plugins that it use anything from a <see cref="INativeController"/>.</param>
			virtual void									Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)=0;
			/// <summary>Unload all plugins.</summary>
			/// <param name="controllerUnrelatedPlugins">Unload plugins that does not use anything from a <see cref="INativeController"/>.</param>
			/// <param name="controllerRelatedPlugins">Unload plugins that it use anything from a <see cref="INativeController"/>.</param>
			virtual void									Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)=0;
			/// <returns>Returns true if all controller related plugins are loaded.</returns>
			virtual bool									IsControllerRelatedPluginsLoaded()=0;
			/// <returns>Returns true if all controller unrelated plugins are loaded.</returns>
			virtual bool									IsControllerUnrelatedPluginsLoaded()=0;
		};

/***********************************************************************
Plugin Manager
***********************************************************************/

		struct GuiPluginDescriptor
		{
			GuiPluginDescriptor*							next = nullptr;

			virtual Ptr<IGuiPlugin>							CreatePlugin() = 0;
		};

		/// <summary>Get the global <see cref="IGuiPluginManager"/> object.</summary>
		/// <returns>The global <see cref="IGuiPluginManager"/> object.</returns>
		extern IGuiPluginManager*							GetPluginManager();

		/// <summary>Register a plugin descriptor. Do not call this function directly, use GUI_REGISTER_PLUGIN macro instead.</summary>
		/// <param name="pluginDescriptor">The plugin descriptor.</param>
		extern void											RegisterPluginDescriptor(GuiPluginDescriptor* pluginDescriptor);

		/// <summary>Destroy the global <see cref="IGuiPluginManager"/> object.</summary>
		extern void											DestroyPluginManager();

#define GUI_REGISTER_PLUGIN(TYPE)\
	struct GuiRegisterPluginClass_##TYPE : private vl::presentation::GuiPluginDescriptor\
	{\
	private:\
		vl::Ptr<vl::presentation::IGuiPlugin> CreatePlugin() override\
		{\
			return vl::Ptr(new TYPE);\
		}\
	public:\
		GuiRegisterPluginClass_##TYPE()\
		{\
			vl::presentation::RegisterPluginDescriptor(this);\
		}\
	} instance_GuiRegisterPluginClass_##TYPE;\

#define GUI_PLUGIN_NAME(NAME)\
	vl::WString GetName()override { return L ## #NAME; }\
	void GetDependencies(vl::collections::List<WString>& dependencies)override\

#define GUI_PLUGIN_DEPEND(NAME) dependencies.Add(L ## #NAME)
	}
}

#endif