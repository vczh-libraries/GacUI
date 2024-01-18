/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Remote Protocol

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_REMOTEPROTOCOL_GUIREMOTEPROTOCOL
#define VCZH_PRESENTATION_REFLECTION_REMOTEPROTOCOL_GUIREMOTEPROTOCOL

#include "Generated/GuiRemoteProtocolParser.h"

namespace vl::presentation
{
	struct GuiRpError
	{
		glr::ParsingTextRange		position;
		WString						errorMessage;
	};

	struct GuiRpSymbols
	{
		collections::Dictionary<WString, WString>								structCppMapping;
		collections::Dictionary<WString, remoteprotocol::GuiRpStructDecl*>		structDecls;
		collections::Dictionary<WString, remoteprotocol::GuiRpMessageDecl*>		messageDecls;
		collections::Dictionary<WString, remoteprotocol::GuiRpEventDecl*>		eventDecls;
	};

	extern Ptr<GuiRpSymbols>		CheckRemoteProtocolSchema(Ptr<remoteprotocol::GuiRpSchema> schema, collections::List<GuiRpError>& errors);
}

#endif