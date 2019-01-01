/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Loader

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIRESOURCEMANAGER
#define VCZH_PRESENTATION_REFLECTION_GUIRESOURCEMANAGER

#include "../Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
IGuiResourceManager
***********************************************************************/

		class GuiResourceClassNameRecord : public Object, public Description<GuiResourceClassNameRecord>
		{
		public:
			collections::List<WString>								classNames;
			collections::Dictionary<WString, Ptr<GuiResourceItem>>	classResources;
		};

		class IGuiResourceManager : public IDescriptable, public Description<IGuiResourceManager>
		{
		public:
			virtual void								SetResource(Ptr<GuiResource> resource, GuiResourceError::List& errors, GuiResourceUsage usage = GuiResourceUsage::DataOnly) = 0;
			virtual Ptr<GuiResource>					GetResource(const WString& name) = 0;
			virtual Ptr<GuiResource>					GetResourceFromClassName(const WString& classFullName) = 0;
			virtual void								UnloadResource(const WString& name) = 0;
			virtual void								LoadResourceOrPending(stream::IStream& stream, GuiResourceError::List& errors, GuiResourceUsage usage = GuiResourceUsage::DataOnly) = 0;
			virtual void								LoadResourceOrPending(stream::IStream& stream, GuiResourceUsage usage = GuiResourceUsage::DataOnly) = 0;
			virtual void								GetPendingResourceNames(collections::List<WString>& names) = 0;
		};

		extern IGuiResourceManager*						GetResourceManager();
	}
}

#endif