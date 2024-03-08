#include "GuiPluginManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
GuiPluginManager
***********************************************************************/

		class GuiPluginManager : public Object, public IGuiPluginManager
		{
		protected:
			List<Ptr<IGuiPlugin>>				plugins;
			bool								controllerRelatedLoaded = false;
			bool								controllerUnrelatedLoaded = false;
		public:
			GuiPluginManager()
			{
			}

			~GuiPluginManager()
			{
			}

			void AddPlugin(Ptr<IGuiPlugin> plugin)override
			{
#define ERROR_MESSAGE_PREFIX L"GuiPluginManager::AddPlugin(Ptr<IGuiPlugin>)#"
				CHECK_ERROR(!controllerUnrelatedLoaded, ERROR_MESSAGE_PREFIX L"Load function has already been executed.");
				auto name = plugin->GetName();
				if (name != L"")
				{
					for (auto plugin : plugins)
					{
						CHECK_ERROR(plugin->GetName() != name, ERROR_MESSAGE_PREFIX L"Duplicated plugin name.");
					}
				}
				plugins.Add(plugin);
#undef ERROR_MESSAGE_PREFIX
			}

			void Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
			{
#define ERROR_MESSAGE_PREFIX L"GuiPluginManager::Load(bool, bool)#"
				CHECK_ERROR(controllerUnrelatedPlugins || controllerRelatedPlugins, L"At least one of the parameters should be true.");
				if (controllerUnrelatedPlugins)
				{
					CHECK_ERROR(!controllerUnrelatedLoaded, ERROR_MESSAGE_PREFIX L"A second Load(true, *) could only be called after Unload(true, *).");
					controllerUnrelatedLoaded = true;
				}
				if (controllerRelatedPlugins)
				{
					CHECK_ERROR(controllerUnrelatedLoaded, ERROR_MESSAGE_PREFIX L"Load(*, true) could only be called between Load(true, *) and Unload(true, *).");
					CHECK_ERROR(!controllerRelatedLoaded, ERROR_MESSAGE_PREFIX L"A second Load(*, true) could only be called after Unload(*, true).");
					controllerRelatedLoaded = true;
				}

				SortedList<WString> loaded;
				Group<WString, WString> loading;
				Dictionary<WString, Ptr<IGuiPlugin>> pluginsToLoad;
				for (auto plugin : plugins)
				{
					auto name = plugin->GetName();
					pluginsToLoad.Add(name, plugin);
					List<WString> dependencies;
					plugin->GetDependencies(dependencies);
					for (auto dependency : dependencies)
					{
						loading.Add(name, dependency);
					}
				}

				while (pluginsToLoad.Count() > 0)
				{
					vint count = pluginsToLoad.Count();
					{
						// TODO: (enumerable) foreach:alterable(reversed) on dictionary
						for (auto [name, index] : indexed(pluginsToLoad.Keys()))
						{
							if (!loading.Keys().Contains(name))
							{
								// TODO: (enumerable) foreach:alterable(reversed) on group
								for (vint i = loading.Count() - 1; i >= 0; i--)
								{
									loading.Remove(loading.Keys()[i], name);
								}
								loaded.Add(name);

								auto plugin = pluginsToLoad.Values()[index];
								pluginsToLoad.Remove(name);
								plugin->Load(controllerUnrelatedPlugins, controllerRelatedPlugins);
								break;
							}
						}
					}
					if (count == pluginsToLoad.Count())
					{
						WString message;
						for (auto plugin : pluginsToLoad.Values())
						{
							message += L"Cannot load plugin \"" + plugin->GetName() + L"\" because part of its dependencies are not ready:";
							List<WString> dependencies;
							plugin->GetDependencies(dependencies);
							bool first = true;
							for (auto dependency : dependencies)
							{
								if (!loaded.Contains(dependency))
								{
									message += L" \"" + dependency + L"\";";
								}
							}
							message += L"\r\n";
						}
						throw Exception(message);
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
			{
#define ERROR_MESSAGE_PREFIX L"GuiPluginManager::Unload(bool, bool)#"
				CHECK_ERROR(controllerUnrelatedPlugins || controllerRelatedPlugins, L"At least one of the parameters should be true.");
				if (controllerRelatedPlugins)
				{
					CHECK_ERROR(controllerUnrelatedLoaded, ERROR_MESSAGE_PREFIX L"Unload(*, true) could only be called between Load(true, *) and Unload(true, *).");
					CHECK_ERROR(controllerRelatedLoaded, ERROR_MESSAGE_PREFIX L"Unload(*, true) could only be called after Load(*, true).");
					controllerRelatedLoaded = false;
				}
				if (controllerUnrelatedPlugins)
				{
					CHECK_ERROR(controllerUnrelatedLoaded, ERROR_MESSAGE_PREFIX L"Unload(true, *) could only be called after Load(true, *).");
					CHECK_ERROR(!controllerRelatedLoaded, ERROR_MESSAGE_PREFIX L"Unload(true, *) could only be called after Load(*, true).");
					controllerUnrelatedLoaded = false;
				}

				for (auto plugin : plugins)
				{
					plugin->Unload(controllerUnrelatedPlugins, controllerRelatedPlugins);
				}
#undef ERROR_MESSAGE_PREFIX
			}

			bool IsControllerRelatedPluginsLoaded()override
			{
				return controllerRelatedLoaded;
			}

			bool IsControllerUnrelatedPluginsLoaded()override
			{
				return controllerUnrelatedLoaded;
			}
		};

/***********************************************************************
Helpers
***********************************************************************/

		GuiPluginDescriptor* firstPluginDescriptor = nullptr;
		GuiPluginDescriptor** lastPluginDescriptor = &firstPluginDescriptor;
		IGuiPluginManager* pluginManager = nullptr;

		IGuiPluginManager* GetPluginManager()
		{
			if (!pluginManager)
			{
				pluginManager = new GuiPluginManager;

				auto current = firstPluginDescriptor;
				while (current)
				{
					pluginManager->AddPlugin(current->CreatePlugin());
					current = current->next;
				}
			}
			return pluginManager;
		}

		void RegisterPluginDescriptor(GuiPluginDescriptor* pluginDescriptor)
		{
			CHECK_ERROR(!pluginManager, L"vl::presentation::RegisterPluginDescriptor(GuiPluginDescriptor*)#This function should be called before calling GetPluginManager.");
			*lastPluginDescriptor = pluginDescriptor;
			lastPluginDescriptor = &pluginDescriptor->next;
		}

		void DestroyPluginManager()
		{
			if (pluginManager)
			{
				CHECK_ERROR(
					!pluginManager->IsControllerRelatedPluginsLoaded() && !pluginManager->IsControllerUnrelatedPluginsLoaded(),
					L"vl::presentation::DestroyPluginManager()#Plugins have not been unloaded.");
				delete pluginManager;
				pluginManager = nullptr;
			}
		}
	}
}