#include "GacGen.h"

/***********************************************************************
Codegen::EventHandlers
***********************************************************************/

WString GetEventHandlerCommentBegin(const WString& prefix)
{
	return prefix + L"// #region CLASS_MEMBER_GUIEVENT_HANDLER (DO NOT PUT OTHER CONTENT IN THIS #region.)";
}

WString GetEventHandlerCommentEnd(const WString& prefix)
{
	return prefix + L"// #endregion CLASS_MEMBER_GUIEVENT_HANDLER";
}

WString GetEventHandlerHeader(const WString& prefix, Ptr<Instance> instance, const WString& name, bool addClassName)
{
	return prefix + L"void " + (addClassName ? instance->typeName + L"::" : L"") + name + L"(GuiGraphicsComposition* sender, " + GetCppTypeName(instance->eventHandlers[name].argumentType) + L"& arguments)" + (addClassName ? L"" : L";");
}

void WriteControlClassHeaderFileEventHandlers(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, StreamWriter& writer)
{
	writer.WriteLine(GetEventHandlerCommentBegin(prefix + L"\t"));
	FOREACH(WString, name, instance->eventHandlers.Keys())
	{
		writer.WriteLine(GetEventHandlerHeader(prefix + L"\t", instance, name, false));
	}
	writer.WriteLine(GetEventHandlerCommentEnd(prefix + L"\t"));
}

void WriteControlClassCppFileEventHandlers(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, Group<WString, WString>& existingEventHandlers, List<WString>& additionalLines, StreamWriter& writer)
{
	writer.WriteLine(GetEventHandlerCommentBegin(prefix));
	writer.WriteLine(L"");
	FOREACH(WString, name, instance->eventHandlers.Keys())
	{
		writer.WriteLine(GetEventHandlerHeader(prefix, instance, name, true));
		vint index = existingEventHandlers.Keys().IndexOf(name);
		if (index == -1)
		{
			writer.WriteLine(prefix + L"{");
			writer.WriteLine(prefix + L"}");
		}
		else
		{
			FOREACH(WString, line, existingEventHandlers.GetByIndex(index))
			{
				writer.WriteLine(line);
			}
		}
		writer.WriteLine(L"");
	}
	FOREACH(WString, line, additionalLines)
	{
		writer.WriteLine(line);
	}
	writer.WriteLine(GetEventHandlerCommentEnd(prefix));
}