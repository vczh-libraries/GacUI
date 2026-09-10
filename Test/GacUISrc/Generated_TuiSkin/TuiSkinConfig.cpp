#include "../../../Source/GacUI.h"
#include "TuiSkinConfig.h"

namespace tuiskin
{
	static ColorPackage CreateColorPackageInternal(vl::presentation::Color accent, vl::presentation::Color highlight)
	{
		ColorPackage colors;
		colors.ControlBackground = vl::presentation::Color(0x00, 0x00, 0x00);
		colors.ControlText = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ControlBorder = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.ControlBorderDisabled = vl::presentation::Color(0x40, 0x40, 0x40);
		colors.ControlBorderFocused = accent;
		colors.LabelText = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.ShortcutKeyBackground = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ShortcutKeyText = vl::presentation::Color(0x00, 0x00, 0x00);
		colors.MenuBackground = vl::presentation::Color(0x40, 0x40, 0x40);
		colors.MenuText = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.MenuTextDisabled = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.MenuBackgroundHighlighted = highlight;
		colors.MenuTextHighlighted = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ItemBackground = vl::presentation::Color(0x00, 0x00, 0x00);
		colors.ItemText = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ItemTextDisabled = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.ItemBackgroundHighlighted = highlight;
		colors.ItemTextHighlighted = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ItemBackgroundSelected = accent;
		colors.ItemTextSelected = vl::presentation::Color(0x40, 0x40, 0x40);
		colors.ButtonBackground = vl::presentation::Color(0x40, 0x40, 0x40);
		colors.ButtonText = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ButtonIcon = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.ButtonTextDisabled = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.ButtonBackgroundHighlighted = accent;
		colors.ButtonTextHighlighted = vl::presentation::Color(0x40, 0x40, 0x40);
		colors.ButtonBackgroundPressed = vl::presentation::Color(0xC0, 0xC0, 0xC0);
		colors.ButtonTextPressed = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		return colors;
	}

	ColorPackage CreateDefaultColorPackage()
	{
		return CreateSkyblueColorPackage();
	}

	ColorPackage CreatePinkColorPackage()
	{
		return CreateColorPackageInternal(vl::presentation::Color(0xD9, 0x9A, 0xB7), vl::presentation::Color(0x74, 0x3C, 0x59));
	}

	ColorPackage CreateOrangeColorPackage()
	{
		return CreateColorPackageInternal(vl::presentation::Color(0xE3, 0xB0, 0x80), vl::presentation::Color(0x80, 0x51, 0x2F));
	}

	ColorPackage CreateGrassPackage()
	{
		return CreateColorPackageInternal(vl::presentation::Color(0xB5, 0xC9, 0x8A), vl::presentation::Color(0x4D, 0x62, 0x32));
	}

	ColorPackage CreateEmeraldPackage()
	{
		return CreateColorPackageInternal(vl::presentation::Color(0x88, 0xC9, 0xB0), vl::presentation::Color(0x2D, 0x67, 0x56));
	}

	ColorPackage CreateSkyblueColorPackage()
	{
		return CreateColorPackageInternal(vl::presentation::Color(0x87, 0xCE, 0xFA), vl::presentation::Color(0x00, 0x00, 0x80));
	}

	ColorPackage CreatePurplePackage()
	{
		return CreateColorPackageInternal(vl::presentation::Color(0xB8, 0xA4, 0xDA), vl::presentation::Color(0x58, 0x40, 0x78));
	}

	void SetColorPackage(const ColorPackage& colors)
	{
		vl_workflow_global::TuiSkin::Instance().InstallColorPackage(colors);
	}
}
