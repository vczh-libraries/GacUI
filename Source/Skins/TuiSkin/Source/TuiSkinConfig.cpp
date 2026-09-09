#include "../../../GacUI.h"
#include "TuiSkinConfig.h"

namespace tuiskin
{
	ColorPackage CreateDefaultColorPackage()
	{
		ColorPackage colors;
		colors.ControlBackground = vl::presentation::Color(0x00, 0x00, 0x00);
		colors.ControlText = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ControlBorder = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.ControlBorderDisabled = vl::presentation::Color(0x40, 0x40, 0x40);
		colors.ControlBorderFocused = vl::presentation::Color(0x87, 0xCE, 0xFA);
		colors.LabelText = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.ShortcutKeyBackground = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ShortcutKeyText = vl::presentation::Color(0x00, 0x00, 0x00);
		colors.MenuBackground = vl::presentation::Color(0x40, 0x40, 0x40);
		colors.MenuText = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.MenuTextDisabled = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.MenuBackgroundHighlighted = vl::presentation::Color(0x00, 0x00, 0x80);
		colors.MenuTextHighlighted = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ItemBackground = vl::presentation::Color(0x00, 0x00, 0x00);
		colors.ItemText = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ItemTextDisabled = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.ItemBackgroundHighlighted = vl::presentation::Color(0x00, 0x00, 0x80);
		colors.ItemTextHighlighted = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ItemBackgroundSelected = vl::presentation::Color(0x87, 0xCE, 0xFA);
		colors.ItemTextSelected = vl::presentation::Color(0x40, 0x40, 0x40);
		colors.ButtonBackground = vl::presentation::Color(0x40, 0x40, 0x40);
		colors.ButtonText = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		colors.ButtonIcon = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.ButtonTextDisabled = vl::presentation::Color(0x80, 0x80, 0x80);
		colors.ButtonBackgroundHighlighted = vl::presentation::Color(0x87, 0xCE, 0xFA);
		colors.ButtonTextHighlighted = vl::presentation::Color(0x40, 0x40, 0x40);
		colors.ButtonBackgroundPressed = vl::presentation::Color(0xC0, 0xC0, 0xC0);
		colors.ButtonTextPressed = vl::presentation::Color(0xFF, 0xFF, 0xFF);
		return colors;
	}

	void SetColorPackage(const ColorPackage& colors)
	{
		vl_workflow_global::TuiSkin::Instance().InstallColorPackage(colors);
	}
}
