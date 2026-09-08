#ifndef VCZH_PRESENTATION_ELEMENTS_TUIGRAPHICSELEMENT
#define VCZH_PRESENTATION_ELEMENTS_TUIGRAPHICSELEMENT

#include "GuiGraphicsElement.h"

namespace vl::presentation::elements
{
	enum class TuiLineStyle
	{
		Thin,
		Thick,
		Double,
	};

	class TuiBorderElement : public GuiElementBase<TuiBorderElement>
	{
		friend class GuiElementBase<TuiBorderElement>;
		static constexpr const wchar_t* ElementTypeName = L"TuiBorder";
	protected:
		Color			color;
		ElementShape	shape;
		TuiLineStyle	lineStyle = TuiLineStyle::Thin;
	public:
		Color			GetColor();
		void			SetColor(Color value);
		ElementShape	GetShape();
		void			SetShape(ElementShape value);
		TuiLineStyle	GetLineStyle();
		void			SetLineStyle(TuiLineStyle value);
	};
}

#endif
