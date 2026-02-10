#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace collections;
	using namespace elements;
	using namespace remoteprotocol;

/***********************************************************************
* Rendering (Elemnents)
***********************************************************************/

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SolidBorder(const remoteprotocol::ElementDesc_SolidBorder& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiSolidBorderElement>();
		if (!element) return;

		element->SetColor(arguments.borderColor);
		element->SetShape(arguments.shape);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SinkBorder(const remoteprotocol::ElementDesc_SinkBorder& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<Gui3DBorderElement>();
		if (!element) return;

		element->SetColors(arguments.leftTopColor, arguments.rightBottomColor);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SinkSplitter(const remoteprotocol::ElementDesc_SinkSplitter& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<Gui3DSplitterElement>();
		if (!element) return;

		element->SetColors(arguments.leftTopColor, arguments.rightBottomColor);
		element->SetDirection(arguments.direction);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SolidBackground(const remoteprotocol::ElementDesc_SolidBackground& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiSolidBackgroundElement>();
		if (!element) return;

		element->SetColor(arguments.backgroundColor);
		element->SetShape(arguments.shape);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_GradientBackground(const remoteprotocol::ElementDesc_GradientBackground& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiGradientBackgroundElement>();
		if (!element) return;

		element->SetColors(arguments.leftTopColor, arguments.rightBottomColor);
		element->SetDirection(arguments.direction);
		element->SetShape(arguments.shape);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_InnerShadow(const remoteprotocol::ElementDesc_InnerShadow& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiInnerShadowElement>();
		if (!element) return;

		element->SetColor(arguments.shadowColor);
		element->SetThickness(arguments.thickness);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_Polygon(const remoteprotocol::ElementDesc_Polygon& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiPolygonElement>();
		if (!element) return;

		element->SetSize(arguments.size);
		element->SetBorderColor(arguments.borderColor);
		element->SetBackgroundColor(arguments.backgroundColor);

		if (arguments.points && arguments.points->Count() > 0)
		{
			element->SetPoints(&arguments.points->Get(0), arguments.points->Count());
		}
	}
}