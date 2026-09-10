#include "DarkSkinConfig.h"

namespace darkskin
{
	ColorPackage CreateDefaultColorPackage()
	{
		return vl_workflow_global::DarkSkin::Instance().CreateDefaultColorPackage();
	}

	ColorPackage CreateAuroraColorPackage()
	{
		ColorPackage colors;
		colors.GeneralAccent = vl::presentation::Color(0x16, 0x8A, 0x7A);
		colors.WindowBorderActive = vl::presentation::Color(0x17, 0x8A, 0x7A);
		colors.ControlAccentHovered = vl::presentation::Color(0x31, 0xAA, 0x97);
		colors.ArrowAccentHovered = vl::presentation::Color(0x2E, 0xA9, 0x95);
		colors.ItemBackgroundSelected = vl::presentation::Color(0x29, 0x9C, 0x89);
		colors.ExpandingArrowHovered = vl::presentation::Color(0x16, 0x7D, 0x6D);
		colors.RibbonExpandingArrow = vl::presentation::Color(0x99, 0xDB, 0xCB);
		colors.RibbonExpandingArrowPressed = vl::presentation::Color(0x12, 0x5A, 0x50);
		colors.TabHighlightedSelected = vl::presentation::Color(0xB0, 0x96, 0xDD);
		colors.TabHighlightedHovered = vl::presentation::Color(0xC8, 0xB2, 0xEC);
		colors.TabHighlightedBackground = vl::presentation::Color(0x51, 0x45, 0x5F);
		colors.ProgressFilling = vl::presentation::Color(0xC7, 0xA3, 0x4D);
		return vl_workflow_global::DarkSkin::Instance().CreateColorPackageInternal(colors);
	}

	ColorPackage CreateEmberColorPackage()
	{
		ColorPackage colors;
		colors.GeneralAccent = vl::presentation::Color(0xB6, 0x63, 0x32);
		colors.WindowBorderActive = vl::presentation::Color(0xB7, 0x64, 0x33);
		colors.ControlAccentHovered = vl::presentation::Color(0xD2, 0x83, 0x4A);
		colors.ArrowAccentHovered = vl::presentation::Color(0xCF, 0x80, 0x47);
		colors.ItemBackgroundSelected = vl::presentation::Color(0xC7, 0x75, 0x40);
		colors.ExpandingArrowHovered = vl::presentation::Color(0xA6, 0x5A, 0x2F);
		colors.RibbonExpandingArrow = vl::presentation::Color(0xED, 0xBE, 0x94);
		colors.RibbonExpandingArrowPressed = vl::presentation::Color(0x75, 0x40, 0x20);
		colors.TabHighlightedSelected = vl::presentation::Color(0xCE, 0x8E, 0xA5);
		colors.TabHighlightedHovered = vl::presentation::Color(0xE4, 0xAB, 0xC0);
		colors.TabHighlightedBackground = vl::presentation::Color(0x60, 0x43, 0x4E);
		colors.ProgressFilling = vl::presentation::Color(0x8C, 0xA9, 0x5C);
		return vl_workflow_global::DarkSkin::Instance().CreateColorPackageInternal(colors);
	}

	ColorPackage CreateMoonstoneColorPackage()
	{
		ColorPackage colors;
		colors.GeneralAccent = vl::presentation::Color(0x68, 0x66, 0xB4);
		colors.WindowBorderActive = vl::presentation::Color(0x69, 0x67, 0xB5);
		colors.ControlAccentHovered = vl::presentation::Color(0x89, 0x85, 0xD2);
		colors.ArrowAccentHovered = vl::presentation::Color(0x86, 0x82, 0xCE);
		colors.ItemBackgroundSelected = vl::presentation::Color(0x79, 0x75, 0xC5);
		colors.ExpandingArrowHovered = vl::presentation::Color(0x5E, 0x5C, 0xA4);
		colors.RibbonExpandingArrow = vl::presentation::Color(0xC6, 0xC2, 0xF0);
		colors.RibbonExpandingArrowPressed = vl::presentation::Color(0x44, 0x41, 0x73);
		colors.TabHighlightedSelected = vl::presentation::Color(0x70, 0xB7, 0xCD);
		colors.TabHighlightedHovered = vl::presentation::Color(0x97, 0xCF, 0xE0);
		colors.TabHighlightedBackground = vl::presentation::Color(0x35, 0x57, 0x63);
		colors.ProgressFilling = vl::presentation::Color(0xD0, 0x83, 0x77);
		return vl_workflow_global::DarkSkin::Instance().CreateColorPackageInternal(colors);
	}

	ColorPackage CreateLagoonColorPackage()
	{
		ColorPackage colors;
		colors.GeneralAccent = vl::presentation::Color(0x14, 0x7F, 0x9C);
		colors.WindowBorderActive = vl::presentation::Color(0x15, 0x80, 0x9D);
		colors.ControlAccentHovered = vl::presentation::Color(0x32, 0x9F, 0xBD);
		colors.ArrowAccentHovered = vl::presentation::Color(0x2F, 0x9C, 0xBA);
		colors.ItemBackgroundSelected = vl::presentation::Color(0x26, 0x8F, 0xA9);
		colors.ExpandingArrowHovered = vl::presentation::Color(0x16, 0x72, 0x8A);
		colors.RibbonExpandingArrow = vl::presentation::Color(0x9C, 0xD3, 0xE0);
		colors.RibbonExpandingArrowPressed = vl::presentation::Color(0x0D, 0x53, 0x66);
		colors.TabHighlightedSelected = vl::presentation::Color(0xCF, 0xAC, 0x6A);
		colors.TabHighlightedHovered = vl::presentation::Color(0xE5, 0xC8, 0x8C);
		colors.TabHighlightedBackground = vl::presentation::Color(0x61, 0x53, 0x37);
		colors.ProgressFilling = vl::presentation::Color(0x52, 0xAF, 0x91);
		return vl_workflow_global::DarkSkin::Instance().CreateColorPackageInternal(colors);
	}

	ColorPackage CreateRosewoodColorPackage()
	{
		ColorPackage colors;
		colors.GeneralAccent = vl::presentation::Color(0xA0, 0x4E, 0x72);
		colors.WindowBorderActive = vl::presentation::Color(0xA1, 0x4F, 0x73);
		colors.ControlAccentHovered = vl::presentation::Color(0xBF, 0x73, 0x93);
		colors.ArrowAccentHovered = vl::presentation::Color(0xBC, 0x70, 0x90);
		colors.ItemBackgroundSelected = vl::presentation::Color(0xAD, 0x60, 0x82);
		colors.ExpandingArrowHovered = vl::presentation::Color(0x91, 0x48, 0x67);
		colors.RibbonExpandingArrow = vl::presentation::Color(0xE4, 0xB1, 0xC8);
		colors.RibbonExpandingArrowPressed = vl::presentation::Color(0x69, 0x33, 0x4C);
		colors.TabHighlightedSelected = vl::presentation::Color(0x78, 0xB5, 0xAE);
		colors.TabHighlightedHovered = vl::presentation::Color(0x9E, 0xCE, 0xC8);
		colors.TabHighlightedBackground = vl::presentation::Color(0x3C, 0x59, 0x55);
		colors.ProgressFilling = vl::presentation::Color(0xC2, 0xA5, 0x53);
		return vl_workflow_global::DarkSkin::Instance().CreateColorPackageInternal(colors);
	}

	void SetColorPackage(const ColorPackage& colors)
	{
		vl_workflow_global::DarkSkin::Instance().InstallColorPackage(colors);
	}

	class DarkSkinColorPackagePlugin : public vl::presentation::IGuiPlugin
	{
	public:
		vl::WString GetName() override
		{
			return L"DarkSkin_ColorPackages";
		}

		void GetDependencies(vl::collections::List<vl::WString>& dependencies) override
		{
		}

		void Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins) override
		{
			if (controllerUnrelatedPlugins)
			{
				vl_workflow_global::DarkSkin::Instance().customColorPackageFactory = [](vl::vint preset) -> ColorPackage
				{
#define ERROR_MESSAGE_PREFIX L"darkskin::DarkSkinColorPackagePlugin::Load()#"
					using vl::Error;
					switch (preset)
					{
					case 1: return CreateAuroraColorPackage();
					case 2: return CreateEmberColorPackage();
					case 3: return CreateMoonstoneColorPackage();
					case 4: return CreateLagoonColorPackage();
					case 5: return CreateRosewoodColorPackage();
					default: CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unknown DarkSkin palette.");
					}
#undef ERROR_MESSAGE_PREFIX
				};
				vl_workflow_global::DarkSkin::Instance().customColorPackagesAvailable = true;
			}
		}

		void Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins) override
		{
			if (controllerUnrelatedPlugins)
			{
				vl_workflow_global::DarkSkin::Instance().customColorPackagesAvailable = false;
				vl_workflow_global::DarkSkin::Instance().customColorPackageFactory = {};
			}
		}
	};
	GUI_REGISTER_PLUGIN(DarkSkinColorPackagePlugin)
}
