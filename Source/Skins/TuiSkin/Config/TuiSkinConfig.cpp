#include "TuiSkinConfig.h"

namespace tuiskin
{
	ColorPackage CreateDefaultColorPackage()
	{
		return vl_workflow_global::TuiSkin::Instance().CreateDefaultColorPackage();
	}

	ColorPackage CreatePinkColorPackage()
	{
		return vl_workflow_global::TuiSkin::Instance().CreateColorPackageInternal(vl::presentation::Color(0xD9, 0x9A, 0xB7), vl::presentation::Color(0x74, 0x3C, 0x59));
	}

	ColorPackage CreateOrangeColorPackage()
	{
		return vl_workflow_global::TuiSkin::Instance().CreateColorPackageInternal(vl::presentation::Color(0xE3, 0xB0, 0x80), vl::presentation::Color(0x80, 0x51, 0x2F));
	}

	ColorPackage CreateGrassPackage()
	{
		return vl_workflow_global::TuiSkin::Instance().CreateColorPackageInternal(vl::presentation::Color(0xB5, 0xC9, 0x8A), vl::presentation::Color(0x4D, 0x62, 0x32));
	}

	ColorPackage CreateEmeraldPackage()
	{
		return vl_workflow_global::TuiSkin::Instance().CreateColorPackageInternal(vl::presentation::Color(0x88, 0xC9, 0xB0), vl::presentation::Color(0x2D, 0x67, 0x56));
	}

	ColorPackage CreatePurplePackage()
	{
		return vl_workflow_global::TuiSkin::Instance().CreateColorPackageInternal(vl::presentation::Color(0xB8, 0xA4, 0xDA), vl::presentation::Color(0x58, 0x40, 0x78));
	}

	void SetColorPackage(const ColorPackage& colors)
	{
		TuiTheme::SetColorPackage(colors);
	}
}
