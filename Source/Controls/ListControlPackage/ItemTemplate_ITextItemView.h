/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_ITEMTEMPLATE_ITEXTITEMVIEW
#define VCZH_PRESENTATION_CONTROLS_ITEMTEMPLATE_ITEXTITEMVIEW

#include "GuiListControls.h"

namespace vl::presentation::controls::list
{
	class DefaultTextListItemTemplate : public PredefinedListItemTemplate<templates::GuiTextListItemTemplate>
	{
	protected:
		using BulletStyle = templates::GuiControlTemplate;

		GuiSelectableButton*					bulletButton = nullptr;
		elements::GuiSolidLabelElement*			textElement = nullptr;
		bool									supressEdit = false;

		virtual TemplateProperty<BulletStyle>	CreateBulletStyle();
		void									OnInitialize()override;
		void									OnRefresh()override;
		void									OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnCheckedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnBulletSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
	public:
		DefaultTextListItemTemplate();
		~DefaultTextListItemTemplate();
	};

	class DefaultCheckTextListItemTemplate : public DefaultTextListItemTemplate
	{
	protected:
		TemplateProperty<BulletStyle>			CreateBulletStyle()override;
	public:
	};

	class DefaultRadioTextListItemTemplate : public DefaultTextListItemTemplate
	{
	protected:
		TemplateProperty<BulletStyle>			CreateBulletStyle()override;
	public:
	};
}

#endif