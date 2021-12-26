/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Query

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_INSTANCEQUERY_GUIINSTANCEQUERY
#define VCZH_PRESENTATION_REFLECTION_INSTANCEQUERY_GUIINSTANCEQUERY

#include "Generated/GuiInstanceQueryParser.h"
#include "../GuiInstanceRepresentation.h"

namespace vl
{
	namespace presentation
	{
		extern void ExecuteQuery(Ptr<instancequery::GuiIqQuery> query, Ptr<GuiInstanceContext> context, collections::List<Ptr<GuiConstructorRepr>>& input, collections::List<Ptr<GuiConstructorRepr>>& output);
		extern void ExecuteQuery(Ptr<instancequery::GuiIqQuery> query, Ptr<GuiInstanceContext> context, collections::List<Ptr<GuiConstructorRepr>>& output);
		extern void ApplyStyle(Ptr<GuiInstanceStyle> style, Ptr<GuiConstructorRepr> ctor);
		extern void GuiIqPrint(Ptr<instancequery::GuiIqQuery> query, stream::StreamWriter& writer);
	}
}

#endif