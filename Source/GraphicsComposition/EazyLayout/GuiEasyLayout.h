/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_EAZYLAYOUT_GUIEASYLAYOUT
#define VCZH_PRESENTATION_COMPOSITION_EAZYLAYOUT_GUIEASYLAYOUT

#include "../GuiGraphicsTableComposition.h"

namespace vl::presentation::compositions::eazy_layout
{
	enum class GuiEasyLayoutDirection
	{
		Inherited,
		Horizontal,
		Vertical,
	};

	class GuiEasyLayoutComposition;
	class GuiEasyLayoutBuilder;
	class GuiEasyPayloadState;

	/// <summary>Stored layout configuration containing descriptor children or one independently positioned payload. Changes are applied by the owning composition's BuildLayout.</summary>
	class GuiEasyLayout : public Object, public Description<GuiEasyLayout>
	{
		friend class GuiEasyLayoutComposition;
		friend class GuiEasyLayoutBuilder;
	protected:
		enum class Kind { Root, Top, Bottom, Left, Right, Row, Column, Fill, Splitter };

		Kind										kind;
		collections::List<Ptr<GuiEasyLayout>>			layouts;
		Ptr<GuiEasyPayloadState>						payload;
		GuiEasyLayoutComposition*					owner = nullptr;

		GuiEasyLayout(Kind _kind);
	public:
		~GuiEasyLayout();

		collections::List<Ptr<GuiEasyLayout>>&		GetLayouts();
		GuiGraphicsComposition*						GetComposition();
		/// <summary>Transfers an unattached payload to this descriptor. An attached payload remains owned by its composition tree until rebuilding removes it.</summary>
		void										SetComposition(GuiGraphicsComposition* value);
	};

	class GuiEasyTopLayout : public GuiEasyLayout, public Description<GuiEasyTopLayout>
	{
	public:
		GuiEasyTopLayout();
	};

	class GuiEasyBottomLayout : public GuiEasyLayout, public Description<GuiEasyBottomLayout>
	{
	public:
		GuiEasyBottomLayout();
	};

	class GuiEasyLeftLayout : public GuiEasyLayout, public Description<GuiEasyLeftLayout>
	{
	public:
		GuiEasyLeftLayout();
	};

	class GuiEasyRightLayout : public GuiEasyLayout, public Description<GuiEasyRightLayout>
	{
	public:
		GuiEasyRightLayout();
	};

	class GuiEasySplitterLayout : public GuiEasyLayout, public Description<GuiEasySplitterLayout>
	{
	public:
		GuiEasySplitterLayout();
	};

	class GuiEasyCellLayout : public GuiEasyLayout, public Description<GuiEasyCellLayout>
	{
	protected:
		GuiCellOption								cellOption = GuiCellOption::MinSizeOption();
		vint										cellSpan = 1;

		GuiEasyCellLayout(Kind _kind);
	public:
		GuiCellOption								GetCellOption();
		void										SetCellOption(GuiCellOption value);
		vint										GetCellSpan();
		void										SetCellSpan(vint value);
	};

	class GuiEasyRowLayout : public GuiEasyCellLayout, public Description<GuiEasyRowLayout>
	{
	public:
		GuiEasyRowLayout();
	};

	class GuiEasyColumnLayout : public GuiEasyCellLayout, public Description<GuiEasyColumnLayout>
	{
	public:
		GuiEasyColumnLayout();
	};

	class GuiEasyFillLayout : public GuiEasyLayout, public Description<GuiEasyFillLayout>
	{
	protected:
		double										percentage = 1;
		GuiEasyLayoutDirection						direction = GuiEasyLayoutDirection::Inherited;
	public:
		GuiEasyFillLayout();

		double										GetPercentage();
		void										SetPercentage(double value);
		GuiEasyLayoutDirection						GetDirection();
		void										SetDirection(GuiEasyLayoutDirection value);
	};

	/// <summary>Lowers stored descriptors to ordinary tables and stacks when explicitly built. Rebuilding retains reused payload instances.</summary>
	class GuiEasyLayoutComposition : public GuiBoundsComposition, public Description<GuiEasyLayoutComposition>
	{
		friend class GuiEasyLayoutBuilder;
	protected:
		vint										padding = 5;
		bool										border = true;
		Ptr<GuiEasyLayout>							content;
		collections::List<Ptr<GuiEasyLayout>>			builtLayouts;
		collections::List<Ptr<GuiEasyPayloadState>>	builtPayloads;
		GuiGraphicsComposition*						generated = nullptr;

	public:
		GuiEasyLayoutComposition();
		~GuiEasyLayoutComposition();

		vint										GetPadding();
		void										SetPadding(vint value);
		bool										GetBorder();
		void										SetBorder(bool value);
		collections::List<Ptr<GuiEasyLayout>>&		GetLayouts();
		GuiGraphicsComposition*						GetComposition();
		void										SetComposition(GuiGraphicsComposition* value);
		/// <summary>Validates the current configuration, replaces generated containers, and applies spacing. Invalid descriptor grammar or track weights raise Error; invalid cell sites follow GuiCellComposition::SetSite.</summary>
		void										BuildLayout();
	};
}

#endif
