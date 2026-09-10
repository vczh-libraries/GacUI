#include "DarkSkinConfig.h"

namespace darkskin
{
	ColorPackage CreateDefaultColorPackage()
	{
		return vl_workflow_global::DarkSkin::Instance().CreateDefaultColorPackage();
	}

	ColorPackage CreateAuroraColorPackage()
	{
		return vl_workflow_global::DarkSkin::Instance().CreateAuroraColorPackage();
	}

	ColorPackage CreateEmberColorPackage()
	{
		return vl_workflow_global::DarkSkin::Instance().CreateEmberColorPackage();
	}

	ColorPackage CreateMoonstoneColorPackage()
	{
		return vl_workflow_global::DarkSkin::Instance().CreateMoonstoneColorPackage();
	}

	ColorPackage CreateLagoonColorPackage()
	{
		return vl_workflow_global::DarkSkin::Instance().CreateLagoonColorPackage();
	}

	ColorPackage CreateRosewoodColorPackage()
	{
		return vl_workflow_global::DarkSkin::Instance().CreateRosewoodColorPackage();
	}

	void SetColorPackage(const ColorPackage& colors)
	{
		vl_workflow_global::DarkSkin::Instance().InstallColorPackage(colors);
	}
}
