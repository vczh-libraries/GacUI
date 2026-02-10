#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace collections;
	using namespace elements;
	using namespace remoteprotocol;

/***********************************************************************
* Rendering (Elemnents -- Label)
***********************************************************************/

	void GuiRemoteRendererSingle::StoreLabelMeasuring(vint id, remoteprotocol::ElementSolidLabelMeasuringRequest request, Ptr<elements::GuiSolidLabelElement> solidLabel, Size minSize)
	{
		switch (request)
		{
		case ElementSolidLabelMeasuringRequest::FontHeight:
			{
				Pair<WString, vint> key = { solidLabel->GetFont().fontFamily,solidLabel->GetFont().size };
				if (fontHeightMeasurings.Contains(key)) return;
				fontHeightMeasurings.Add(key);

				ElementMeasuring_FontHeight response;
				response.fontFamily = key.key;
				response.fontSize = key.value;
				response.height = minSize.y;

				if (!elementMeasurings.fontHeights)
				{
					elementMeasurings.fontHeights = Ptr(new List<ElementMeasuring_FontHeight>);
				}
				elementMeasurings.fontHeights->Add(response);
			}
			break;
		case ElementSolidLabelMeasuringRequest::TotalSize:
			{
				ElementMeasuring_ElementMinSize response;
				response.id = id;
				response.minSize = minSize;

				if (!elementMeasurings.minSizes)
				{
					elementMeasurings.minSizes = Ptr(new List<ElementMeasuring_ElementMinSize>);
				}
				elementMeasurings.minSizes->Add(response);
			}
			break;
		}
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SolidLabel(const remoteprotocol::ElementDesc_SolidLabel& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiSolidLabelElement>();
		if (!element) return;

		element->SetColor(arguments.textColor);
		element->SetAlignments(GetAlignment(arguments.horizontalAlignment), GetAlignment(arguments.verticalAlignment));
		element->SetWrapLine(arguments.wrapLine);
		element->SetWrapLineHeightCalculation(arguments.wrapLineHeightCalculation);
		element->SetEllipse(arguments.ellipse);
		element->SetMultiline(arguments.multiline);

		if (arguments.font)
		{
			element->SetFont(arguments.font.Value());
		}
		if (arguments.text)
		{
			element->SetText(arguments.text.Value());
		}

		if (arguments.measuringRequest)
		{
			SolidLabelMeasuring measuring;
			measuring.request = arguments.measuringRequest.Value();
			index = solidLabelMeasurings.Keys().IndexOf(arguments.id);
			if (solidLabelMeasurings.Keys().Contains(arguments.id))
			{
				solidLabelMeasurings.Set(arguments.id, measuring);
			}
			else
			{
				solidLabelMeasurings.Add(arguments.id, measuring);
			}

			StoreLabelMeasuring(arguments.id, measuring.request, element, element->GetRenderer()->GetMinSize());
		}
	}
}