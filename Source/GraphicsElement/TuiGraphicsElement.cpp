#include "TuiGraphicsElement.h"

namespace vl::presentation::elements
{
	Color TuiBorderElement::GetColor()
	{
		return color;
	}

	void TuiBorderElement::SetColor(Color value)
	{
		if (color != value)
		{
			color = value;
			InvokeOnElementStateChanged();
		}
	}

	ElementShape TuiBorderElement::GetShape()
	{
		return shape;
	}

	void TuiBorderElement::SetShape(ElementShape value)
	{
		if (shape != value)
		{
			shape = value;
			InvokeOnElementStateChanged();
		}
	}

	TuiLineStyle TuiBorderElement::GetLineStyle()
	{
		return lineStyle;
	}

	void TuiBorderElement::SetLineStyle(TuiLineStyle value)
	{
		if (lineStyle != value)
		{
			lineStyle = value;
			InvokeOnElementStateChanged();
		}
	}
}
