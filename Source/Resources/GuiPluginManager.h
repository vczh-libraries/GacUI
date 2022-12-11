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
			virtual void									Load()=0;
			/// <summary>Called when the plugin manager want to unload this plugin.</summary>
			virtual void									Unload()=0;
		};

		/// <summary>Represents a plugin manager.</summary>
		class IGuiPluginManager : public IDescriptable, public Description<IGuiPluginManager>
		{
		public:
			/// <summary>Add a plugin before [F:vl.presentation.controls.IGuiPluginManager.Load] is called.</summary>
			/// <param name="plugin">The plugin.</param>
			virtual void									AddPlugin(Ptr<IGuiPlugin> plugin)=0;
			/// <summary>Load all plugins, and check if dependencies of all plugins are ready.</summary>
			virtual void									Load()=0;
			/// <summary>Unload all plugins.</summary>
			virtual void									Unload()=0;
			/// <returns>Returns true if all plugins are loaded.</returns>
			virtual bool									IsLoaded()=0;
		};

/***********************************************************************
Plugin Manager
***********************************************************************/

		/// <summary>Get the global <see cref="IGuiPluginManager"/> object.</summary>
		/// <returns>The global <see cref="IGuiPluginManager"/> object.</returns>
		extern IGuiPluginManager*							GetPluginManager();

		/// <summary>Destroy the global <see cref="IGuiPluginManager"/> object.</summary>
		extern void											DestroyPluginManager();

#define GUI_REGISTER_PLUGIN(TYPE)\
	class GuiRegisterPluginClass_##TYPE\
	{\
	public:\
		GuiRegisterPluginClass_##TYPE()\
		{\
			vl::presentation::GetPluginManager()->AddPlugin(Ptr(new TYPE));\
		}\
	} instance_GuiRegisterPluginClass_##TYPE;\

#define GUI_PLUGIN_NAME(NAME)\
	vl::WString GetName()override { return L ## #NAME; }\
	void GetDependencies(vl::collections::List<WString>& dependencies)override\

#define GUI_PLUGIN_DEPEND(NAME) dependencies.Add(L ## #NAME)
	}
}

#endif