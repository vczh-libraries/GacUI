#include "GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace theme
		{
			using namespace collections;
			using namespace controls;
			using namespace templates;

			class Theme : public Object, public virtual theme::ITheme
			{
			public:
				Dictionary<WString, Ptr<ThemeTemplates>>	templates;
				ThemeTemplates*								first = nullptr;
				ThemeTemplates*								last = nullptr;

				bool RegisterTheme(const WString& name, Ptr<ThemeTemplates> theme)
				{
					CHECK_ERROR(theme->previous == nullptr, L"vl::presentation::theme::RegisterTheme(const WString&, Ptr<ThemeTemplates>)#Theme object has been registered");
					CHECK_ERROR(theme->next == nullptr, L"vl::presentation::theme::RegisterTheme(const WString&, Ptr<ThemeTemplates>)#Theme object has been registered");

					if (templates.Keys().Contains(name))
					{
						return false;
					}
					templates.Add(name, theme);

					if (last)
					{
						last->next = theme.Obj();
					}
					theme->previous = last;
					last = theme.Obj();

					return true;
				}

				Ptr<ThemeTemplates> UnregisterTheme(const WString& name)
				{
					vint index = templates.Keys().IndexOf(name);
					if (index == -1)
					{
						return nullptr;
					}

					auto themeTemplates = templates.Values().Get(index);

					if (themeTemplates->previous)
					{
						themeTemplates->previous->next = themeTemplates->next;
					}
					else
					{
						first = themeTemplates->next;
					}

					if (themeTemplates->next)
					{
						themeTemplates->next->previous = themeTemplates->previous;

					}
					else
					{
						last = themeTemplates->previous;
					}

					templates.Remove(name);
					return themeTemplates;
				}

				TemplateProperty<templates::GuiControlTemplate> CreateStyle(ThemeName themeName)override
				{
					switch (themeName)
					{
#define GUI_DEFINE_ITEM_PROPERTY(TEMPLATE, CONTROL) \
					case ThemeName::CONTROL:\
						{\
							auto current = last;\
							while (current) \
							{\
								if (current->CONTROL)\
								{\
									return current->CONTROL; \
								}\
								current = current->previous;\
							}\
							throw Exception(L"Control template for \"" L ## #CONTROL L"\" is not defined.");\
						}\

						GUI_CONTROL_TEMPLATE_TYPES(GUI_DEFINE_ITEM_PROPERTY)
#undef GUI_DEFINE_ITEM_PROPERTY
					default:
						CHECK_FAIL(L"vl::presentation::theme::ITheme::CreateStyle(ThemeName)#Unknown theme name.");
					}
				}
			};

			controls::GuiControlHost* ThemeTemplates::GetControlHostForInstance()
			{
				return nullptr;
			}

			ThemeTemplates::~ThemeTemplates()
			{
				FinalizeAggregation();
			}

			Theme* currentTheme = nullptr;

			ITheme* GetCurrentTheme()
			{
				return currentTheme;
			}

			void InitializeTheme()
			{
				CHECK_ERROR(currentTheme == nullptr, L"vl::presentation::theme::InitializeTheme()#Theme has already been initialized");
				currentTheme = new Theme;
			}

			void FinalizeTheme()
			{
				CHECK_ERROR(currentTheme != nullptr, L"vl::presentation::theme::FinalizeTheme()#Theme has not been initialized");
				delete currentTheme;
				currentTheme = nullptr;
			}

			bool RegisterTheme(Ptr<ThemeTemplates> theme)
			{
				CHECK_ERROR(currentTheme != nullptr, L"vl::presentation::theme::RegisterTheme(const WString&, Ptr<ThemeTemplates>)#Theme has already been initialized");
				return currentTheme->RegisterTheme(theme->Name, theme);
			}

			Ptr<ThemeTemplates> UnregisterTheme(const WString& name)
			{
				CHECK_ERROR(currentTheme != nullptr, L"vl::presentation::theme::UnregisterTheme(const WString&)#Theme has already been initialized");
				return currentTheme->UnregisterTheme(name);
			}
		}
	}
}