#include "GuiWin7StylesCommon.h"

namespace vl
{
	namespace presentation
	{
		namespace win7
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;
			using namespace controls;

/***********************************************************************
Win7ButtonColors
***********************************************************************/

			void Win7ButtonColors::SetAlphaWithoutText(unsigned char a)
			{
				borderColor.a=a;
				backgroundColor.a=a;
				g1.a=a;
				g2.a=a;
				g3.a=a;
				g4.a=a;
			}

			Win7ButtonColors Win7ButtonColors::Blend(const Win7ButtonColors& c1, const Win7ButtonColors& c2, vint ratio, vint total)
			{
				if(ratio<0) ratio=0;
				else if(ratio>total) ratio=total;

				Win7ButtonColors result;
				result.borderColor=BlendColor(c1.borderColor, c2.borderColor, ratio, total);
				result.backgroundColor=BlendColor(c1.backgroundColor, c2.backgroundColor, ratio, total);
				result.g1=BlendColor(c1.g1, c2.g1, ratio, total);
				result.g2=BlendColor(c1.g2, c2.g2, ratio, total);
				result.g3=BlendColor(c1.g3, c2.g3, ratio, total);
				result.g4=BlendColor(c1.g4, c2.g4, ratio, total);
				result.textColor=BlendColor(c1.textColor, c2.textColor, ratio, total);
				result.bulletLight=BlendColor(c1.bulletLight, c2.bulletLight, ratio, total);
				result.bulletDark=BlendColor(c1.bulletDark, c2.bulletDark, ratio, total);
				return result;
			}

			//---------------------------------------------------------

			Win7ButtonColors Win7ButtonColors::ButtonNormal()
			{
				Win7ButtonColors colors=
				{
					Color(122, 122, 122),
					Color(251, 251, 251),
					Color(250, 250, 250),
					Color(235, 235, 235),
					Color(221, 221, 221),
					Color(207, 207, 207),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::ButtonActive()
			{
				Win7ButtonColors colors=
				{
					Color(60, 127, 177),
					Color(220, 244, 254),
					Color(246, 252, 255),
					Color(240, 250, 255),
					Color(225, 245, 254),
					Color(215, 245, 254),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::ButtonPressed()
			{
				Win7ButtonColors colors=
				{
					Color(44, 98, 139),
					Color(158, 176, 186),
					Color(225, 246, 255),
					Color(204, 239, 254),
					Color(181, 231, 253),
					Color(164, 225, 251),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::ButtonDisabled()
			{
				Win7ButtonColors colors=
				{
					Color(173, 178, 181),
					Color(252, 252, 252),
					Color(244, 244, 244),
					Color(244, 244, 244),
					Color(244, 244, 244),
					Color(244, 244, 244),
					Win7GetSystemTextColor(false),
				};
				return colors;
			}

			//---------------------------------------------------------

			Win7ButtonColors Win7ButtonColors::ItemNormal()
			{
				Win7ButtonColors colors=
				{
					Color(184, 214, 251, 0),
					Color(252, 253, 254, 0),
					Color(250, 251, 253, 0),
					Color(242, 247, 253, 0),
					Color(242, 247, 253, 0),
					Color(235, 243, 253, 0),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::ItemActive()
			{
				Win7ButtonColors colors=
				{
					Color(184, 214, 251),
					Color(252, 253, 254),
					Color(250, 251, 253),
					Color(242, 247, 253),
					Color(242, 247, 253),
					Color(235, 243, 253),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::ItemSelected()
			{
				Win7ButtonColors colors=
				{
					Color(118, 158, 206),
					Color(235, 244, 253),
					Color(220, 235, 252),
					Color(208, 227, 252),
					Color(208, 227, 252),
					Color(193, 219, 252),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::ItemDisabled()
			{
				Win7ButtonColors colors=
				{
					Color(184, 214, 251, 0),
					Color(252, 253, 254, 0),
					Color(250, 251, 253, 0),
					Color(242, 247, 253, 0),
					Color(242, 247, 253, 0),
					Color(235, 243, 253, 0),
					Win7GetSystemTextColor(false),
				};
				return colors;
			}

			//---------------------------------------------------------

			Win7ButtonColors Win7ButtonColors::CheckedNormal(bool selected)
			{
				Win7ButtonColors colors=
				{
					Color(142, 143, 143),
					Color(244, 244, 244),
					Color(174, 179, 185),
					Color(233, 233, 234),
					Color(203, 207, 213),
					Color(235, 235, 236),
					Win7GetSystemTextColor(true),
					Color(76, 97, 152),
					Color(76, 97, 152),
				};
				if(!selected)
				{
					colors.bulletLight.a=0;
					colors.bulletDark.a=0;
				}
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::CheckedActive(bool selected)
			{
				Win7ButtonColors colors=
				{
					Color(85, 134, 163),
					Color(222, 249, 250),
					Color(121, 198, 249),
					Color(207, 236, 253),
					Color(177, 233, 253),
					Color(231, 247, 254),
					Win7GetSystemTextColor(true),
					Color(4, 34, 113),
					Color(4, 34, 113),
				};
				if(!selected)
				{
					colors.bulletLight.a=0;
					colors.bulletDark.a=0;
				}
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::CheckedPressed(bool selected)
			{
				Win7ButtonColors colors=
				{
					Color(44, 98, 139),
					Color(194, 228, 254),
					Color(94, 182, 247),
					Color(193, 230, 252),
					Color(157, 213, 252),
					Color(224, 244, 254),
					Win7GetSystemTextColor(true),
					Color(63, 93, 153),
					Color(63, 93, 153),
				};
				if(!selected)
				{
					colors.bulletLight.a=0;
					colors.bulletDark.a=0;
				}
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::CheckedDisabled(bool selected)
			{
				Win7ButtonColors colors=
				{
					Color(177, 177, 177),
					Color(240, 240, 240),
					Color(240, 240, 240),
					Color(240, 240, 240),
					Color(240, 240, 240),
					Color(240, 240, 240),
					Win7GetSystemTextColor(false),
					Color(177, 177, 177),
					Color(177, 177, 177),
				};
				if(!selected)
				{
					colors.bulletLight.a=0;
					colors.bulletDark.a=0;
				}
				return colors;
			}

			//---------------------------------------------------------

			Win7ButtonColors Win7ButtonColors::ToolstripButtonNormal()
			{
				Win7ButtonColors colors=
				{
					Win7GetSystemWindowColor(),
					Win7GetSystemWindowColor(),
					Win7GetSystemWindowColor(),
					Win7GetSystemWindowColor(),
					Win7GetSystemWindowColor(),
					Win7GetSystemWindowColor(),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::ToolstripButtonActive()
			{
				Win7ButtonColors colors=
				{
					Color(168, 168, 168),
					Color(248, 248, 248),
					Color(250, 250, 250),
					Color(232, 232, 232),
					Color(218, 218, 218),
					Color(240, 240, 240),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::ToolstripButtonPressed()
			{
				Win7ButtonColors colors=
				{
					Color(84, 84, 84),
					Color(156, 156, 156),
					Color(240, 240, 240),
					Color(228, 228, 228),
					Color(222, 222, 222),
					Color(230, 230, 230),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::ToolstripButtonSelected()
			{
				Win7ButtonColors colors=
				{
					Color(84, 84, 84),
					Color(250, 250, 250),
					Color(250, 250, 250),
					Color(250, 250, 250),
					Color(250, 250, 250),
					Color(250, 250, 250),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::ToolstripButtonDisabled()
			{
				Win7ButtonColors colors=
				{
					Win7GetSystemWindowColor(),
					Win7GetSystemWindowColor(),
					Win7GetSystemWindowColor(),
					Win7GetSystemWindowColor(),
					Win7GetSystemWindowColor(),
					Win7GetSystemWindowColor(),
					Win7GetSystemTextColor(false),
				};
				return colors;
			}

			//---------------------------------------------------------

			Win7ButtonColors Win7ButtonColors::MenuBarButtonNormal()
			{
				Win7ButtonColors colors=
				{
					Color(176, 176, 176),
					Color(234, 238, 247),
					Color(246, 248, 250),
					Color(211, 217, 231),
					Color(199, 204, 218),
					Color(224, 228, 243),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::MenuBarButtonActive()
			{
				Win7ButtonColors colors=
				{
					Color(176, 176, 176),
					Color(234, 238, 247),
					Color(246, 248, 250),
					Color(211, 217, 231),
					Color(199, 204, 218),
					Color(224, 228, 243),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::MenuBarButtonPressed()
			{
				Win7ButtonColors colors=
				{
					Color(88, 88, 89),
					Color(159, 160, 162),
					Color(200, 202, 208),
					Color(205, 209, 219),
					Color(197, 204, 221),
					Color(213, 218, 233),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::MenuBarButtonDisabled()
			{
				Win7ButtonColors colors=
				{
					Color(176, 176, 176),
					Color(234, 238, 247),
					Color(246, 248, 250),
					Color(211, 217, 231),
					Color(199, 204, 218),
					Color(224, 228, 243),
					Win7GetSystemTextColor(false),
				};
				return colors;
			}

			//---------------------------------------------------------

			Win7ButtonColors Win7ButtonColors::MenuItemButtonNormal()
			{
				Win7ButtonColors colors=
				{
					Color(0, 0, 0, 0),
					Color(0, 0, 0, 0),
					Color(0, 0, 0, 0),
					Color(0, 0, 0, 0),
					Color(227, 227, 227),
					Color(255, 255, 255),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::MenuItemButtonNormalActive()
			{
				Win7ButtonColors colors=
				{
					Color(175, 208, 247),
					Color(248, 248, 250),
					Color(243, 245, 247),
					Color(231, 238, 247),
					Color(229, 233, 238),
					Color(245, 249, 255),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::MenuItemButtonSelected()
			{
				Win7ButtonColors colors=
				{
					Color(175, 208, 247),
					Color(248, 248, 250),
					Color(0, 0, 0, 0),
					Color(0, 0, 0, 0),
					Color(0, 0, 0, 0),
					Color(0, 0, 0, 0),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::MenuItemButtonSelectedActive()
			{
				Win7ButtonColors colors=
				{
					Color(175, 208, 247),
					Color(248, 248, 250),
					Color(243, 245, 247),
					Color(231, 238, 247),
					Color(0, 0, 0, 0),
					Color(0, 0, 0, 0),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::MenuItemButtonDisabled()
			{
				Win7ButtonColors colors=
				{
					Color(0, 0, 0, 0),
					Color(0, 0, 0, 0),
					Color(0, 0, 0, 0),
					Color(0, 0, 0, 0),
					Color(227, 227, 227),
					Color(255, 255, 255),
					Win7GetSystemTextColor(false),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::MenuItemButtonDisabledActive()
			{
				Win7ButtonColors colors=
				{
					Color(212, 212, 212),
					Color(244, 244, 244),
					Color(243, 243, 243),
					Color(229, 229, 229),
					Color(232, 232, 232),
					Color(239, 239, 239),
					Win7GetSystemTextColor(false),
				};
				return colors;
			}

			//---------------------------------------------------------

			Win7ButtonColors Win7ButtonColors::TabPageHeaderNormal()
			{
				Win7ButtonColors colors=
				{
					Color(137, 140, 149),
					Color(243, 243, 243),
					Color(242, 242, 242),
					Color(235, 235, 235),
					Color(219, 219, 219),
					Color(207, 207, 207),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::TabPageHeaderActive()
			{
				Win7ButtonColors colors=
				{
					Color(60, 127, 177),
					Color(233, 245, 252),
					Color(234, 246, 253),
					Color(217, 240, 252),
					Color(188, 229, 252),
					Color(167, 217, 245),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

			Win7ButtonColors Win7ButtonColors::TabPageHeaderSelected()
			{
				Win7ButtonColors colors=
				{
					Color(137, 140, 149),
					Win7GetSystemTabContentColor(),
					Win7GetSystemTabContentColor(),
					Win7GetSystemTabContentColor(),
					Win7GetSystemTabContentColor(),
					Win7GetSystemTabContentColor(),
					Win7GetSystemTextColor(true),
				};
				return colors;
			}

/***********************************************************************
Win7ButtonElements
***********************************************************************/

			Win7ButtonElements Win7ButtonElements::Create(bool verticalGradient, bool roundBorder, Alignment horizontal, Alignment vertical)
			{
				Win7ButtonElements button;
				button.rectBorderElement=0;
				button.roundBorderElement=0;
				{
					button.mainComposition=new GuiBoundsComposition;
					button.mainComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				}
				if(roundBorder)
				{
					{
						GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
						button.backgroundElement=element;

						GuiBoundsComposition* composition=new GuiBoundsComposition;
						button.backgroundComposition=composition;
						button.mainComposition->AddChild(composition);
						composition->SetAlignmentToParent(Margin(1, 1, 1, 1));
						composition->SetOwnedElement(element);
					}
					{
						GuiRoundBorderElement* element=GuiRoundBorderElement::Create();
						button.roundBorderElement=element;
						element->SetRadius(2);

						GuiBoundsComposition* composition=new GuiBoundsComposition;
						button.mainComposition->AddChild(composition);
						composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
						composition->SetOwnedElement(element);
					}
				}
				else
				{
					{
						GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
						button.rectBorderElement=element;

						GuiBoundsComposition* composition=new GuiBoundsComposition;
						button.mainComposition->AddChild(composition);
						composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
						composition->SetOwnedElement(element);
					}
					{
						GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
						button.backgroundElement=element;

						GuiBoundsComposition* composition=new GuiBoundsComposition;
						button.backgroundComposition=composition;
						button.mainComposition->AddChild(composition);
						composition->SetAlignmentToParent(Margin(1, 1, 1, 1));
						composition->SetOwnedElement(element);
					}
				}
				{
					GuiTableComposition* table=new GuiTableComposition;
					button.gradientComposition=table;
					table->SetAlignmentToParent(Margin(2, 2, 2, 2));
					button.mainComposition->AddChild(table);
					if(verticalGradient)
					{
						table->SetRowsAndColumns(2, 1);
						table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
						table->SetRowOption(1, GuiCellOption::PercentageOption(0.5));
						table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
					}
					else
					{
						table->SetRowsAndColumns(1, 2);
						table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
						table->SetColumnOption(0, GuiCellOption::PercentageOption(0.5));
						table->SetColumnOption(1, GuiCellOption::PercentageOption(0.5));
					}
					{
						GuiGradientBackgroundElement* element=GuiGradientBackgroundElement::Create();
						button.topGradientElement=element;
						element->SetDirection(verticalGradient?GuiGradientBackgroundElement::Vertical:GuiGradientBackgroundElement::Horizontal);

						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 0, 1, 1);
						cell->SetOwnedElement(element);
					}
					{
						GuiGradientBackgroundElement* element=GuiGradientBackgroundElement::Create();
						button.bottomGradientElement=element;
						element->SetDirection(verticalGradient?GuiGradientBackgroundElement::Vertical:GuiGradientBackgroundElement::Horizontal);

						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						if(verticalGradient)
						{
							cell->SetSite(1, 0, 1, 1);
						}
						else
						{
							cell->SetSite(0, 1, 1, 1);
						}
						cell->SetOwnedElement(element);
					}
				}
				{
					Win7CreateSolidLabelElement(button.textElement, button.textComposition, horizontal, vertical);
					button.mainComposition->AddChild(button.textComposition);
				}
				return button;
			}

			void Win7ButtonElements::Apply(const Win7ButtonColors& colors)
			{
				if(rectBorderElement)
				{
					rectBorderElement->SetColor(colors.borderColor);
				}
				if(roundBorderElement)
				{
					roundBorderElement->SetColor(colors.borderColor);
				}
				backgroundElement->SetColor(colors.backgroundColor);
				topGradientElement->SetColors(colors.g1, colors.g2);
				bottomGradientElement->SetColors(colors.g3, colors.g4);
				textElement->SetColor(colors.textColor);
			}

/***********************************************************************
Win7CheckedButtonElements
***********************************************************************/

			Win7CheckedButtonElements Win7CheckedButtonElements::Create(elements::ElementShape shape, bool backgroundVisible)
			{
				const vint checkSize=13;
				const vint checkPadding=2;

				Win7CheckedButtonElements button;
				{
					button.mainComposition=new GuiBoundsComposition;
					button.mainComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				}
				{
					GuiTableComposition* mainTable=new GuiTableComposition;
					button.mainComposition->AddChild(mainTable);
					if(backgroundVisible)
					{
						GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
						element->SetColor(Win7GetSystemWindowColor());
						mainTable->SetOwnedElement(element);
					}
					mainTable->SetRowsAndColumns(1, 2);
					mainTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
					mainTable->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
					mainTable->SetColumnOption(0, GuiCellOption::AbsoluteOption(checkSize+2*checkPadding));
					mainTable->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
					
					{
						GuiCellComposition* cell=new GuiCellComposition;
						mainTable->AddChild(cell);
						cell->SetSite(0, 0, 1, 1);

						GuiTableComposition* table=new GuiTableComposition;
						cell->AddChild(table);
						table->SetRowsAndColumns(3, 1);
						table->SetAlignmentToParent(Margin(0, 0, 0, 0));
						table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
						table->SetRowOption(1, GuiCellOption::MinSizeOption());
						table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));

						{
							GuiCellComposition* checkCell=new GuiCellComposition;
							table->AddChild(checkCell);
							checkCell->SetSite(1, 0, 1, 1);
							{
								GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
								button.backgroundElement=element;
								element->SetShape(shape);

								GuiBoundsComposition* borderBounds=new GuiBoundsComposition;
								checkCell->AddChild(borderBounds);
								borderBounds->SetOwnedElement(element);
								borderBounds->SetAlignmentToParent(Margin(checkPadding, -1, checkPadding, -1));
								borderBounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
								{
									GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
									button.borderElement=element;
									element->SetShape(shape);

									GuiBoundsComposition* bounds=new GuiBoundsComposition;
									borderBounds->AddChild(bounds);
									bounds->SetOwnedElement(element);
									bounds->SetAlignmentToParent(Margin(0, 0, 0, 0));
									bounds->SetBounds(Rect(Point(0, 0), Size(checkSize, checkSize)));
								}
								{
									GuiGradientBackgroundElement* element=GuiGradientBackgroundElement::Create();
									button.outerGradientElement=element;
									element->SetShape(shape);
									element->SetDirection(GuiGradientBackgroundElement::Backslash);

									GuiBoundsComposition* bounds=new GuiBoundsComposition;
									borderBounds->AddChild(bounds);
									bounds->SetOwnedElement(element);
									bounds->SetAlignmentToParent(Margin(2, 2, 2, 2));
								}
								{
									GuiGradientBackgroundElement* element=GuiGradientBackgroundElement::Create();
									button.innerGradientElement=element;
									element->SetDirection(GuiGradientBackgroundElement::Backslash);
									element->SetShape(shape);

									GuiBoundsComposition* bounds=new GuiBoundsComposition;
									borderBounds->AddChild(bounds);
									bounds->SetOwnedElement(element);
									bounds->SetAlignmentToParent(Margin(3, 3, 3, 3));
								}
							}

							button.bulletCheckElement=0;
							button.bulletRadioElement=0;
							if(shape==ElementShape::Rectangle)
							{
								button.bulletCheckElement=GuiSolidLabelElement::Create();
								{
									FontProperties font;
									font.fontFamily=L"Webdings";
									font.size=16;
									font.bold=true;
									button.bulletCheckElement->SetFont(font);
								}
								button.bulletCheckElement->SetText(L"a");
								button.bulletCheckElement->SetAlignments(Alignment::Center, Alignment::Center);

								GuiBoundsComposition* composition=new GuiBoundsComposition;
								composition->SetOwnedElement(button.bulletCheckElement);
								composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
								checkCell->AddChild(composition);
							}
							else
							{
								button.bulletRadioElement=GuiSolidBackgroundElement::Create();
								button.bulletRadioElement->SetShape(ElementShape::Ellipse);

								GuiBoundsComposition* composition=new GuiBoundsComposition;
								composition->SetOwnedElement(button.bulletRadioElement);
								composition->SetAlignmentToParent(Margin(checkPadding+3, 3, checkPadding+3, 3));
								checkCell->AddChild(composition);
							}
						}
					}

					{
						GuiCellComposition* textCell=new GuiCellComposition;
						mainTable->AddChild(textCell);
						textCell->SetSite(0, 1, 1, 1);
						{
							Win7CreateSolidLabelElement(button.textElement, button.textComposition, Alignment::Left, Alignment::Center);
							textCell->AddChild(button.textComposition);
						}
					}
				}
				return button;
			}

			void Win7CheckedButtonElements::Apply(const Win7ButtonColors& colors)
			{
				borderElement->SetColor(colors.borderColor);
				backgroundElement->SetColor(colors.backgroundColor);
				outerGradientElement->SetColors(colors.g1, colors.g2);
				innerGradientElement->SetColors(colors.g3, colors.g4);
				textElement->SetColor(colors.textColor);
				if(bulletCheckElement)
				{
					bulletCheckElement->SetColor(colors.bulletDark);
				}
				if(bulletRadioElement)
				{
					bulletRadioElement->SetColor(colors.bulletDark);
				}
			}

/***********************************************************************
Win7MenuItemButtonElements
***********************************************************************/

			Win7MenuItemButtonElements Win7MenuItemButtonElements::Create()
			{
				Win7MenuItemButtonElements button;
				{
					button.mainComposition=new GuiBoundsComposition;
					button.mainComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				}
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					button.backgroundElement=element;

					GuiBoundsComposition* composition=new GuiBoundsComposition;
					button.mainComposition->AddChild(composition);
					composition->SetAlignmentToParent(Margin(1, 1, 1, 1));
					composition->SetOwnedElement(element);
				}
				{
					GuiRoundBorderElement* element=GuiRoundBorderElement::Create();
					button.borderElement=element;
					element->SetRadius(2);

					GuiBoundsComposition* composition=new GuiBoundsComposition;
					button.mainComposition->AddChild(composition);
					composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
					composition->SetOwnedElement(element);
				}
				{
					GuiGradientBackgroundElement* element=GuiGradientBackgroundElement::Create();
					button.gradientElement=element;
					element->SetDirection(GuiGradientBackgroundElement::Vertical);

					GuiBoundsComposition* composition=new GuiBoundsComposition;
					button.mainComposition->AddChild(composition);
					composition->SetAlignmentToParent(Margin(2, 2, 2, 2));
					composition->SetOwnedElement(element);
				}
				{
					GuiTableComposition* table=new GuiTableComposition;
					button.mainComposition->AddChild(table);
					table->SetAlignmentToParent(Margin(2, 0, 2, 0));
					table->SetRowsAndColumns(1, 5);

					table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
					table->SetColumnOption(0, GuiCellOption::AbsoluteOption(24));
					table->SetColumnOption(1, GuiCellOption::AbsoluteOption(2));
					table->SetColumnOption(2, GuiCellOption::PercentageOption(1.0));
					table->SetColumnOption(3, GuiCellOption::MinSizeOption());
					table->SetColumnOption(4, GuiCellOption::AbsoluteOption(10));
					
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 0, 1, 1);
						button.splitterComposition=cell;

						GuiImageFrameElement* element=GuiImageFrameElement::Create();
						button.imageElement=element;
						element->SetStretch(false);
						element->SetAlignments(Alignment::Center, Alignment::Center);
						cell->SetOwnedElement(element);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 1, 1);
						button.splitterComposition=cell;

						Gui3DSplitterElement* element=Gui3DSplitterElement::Create();
						button.splitterElement=element;
						element->SetDirection(Gui3DSplitterElement::Vertical);
						cell->SetOwnedElement(element);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 2, 1, 1);

						Win7CreateSolidLabelElement(button.textElement, button.textComposition, L"MenuItem-Text", Alignment::Left, Alignment::Center);
						cell->AddChild(button.textComposition);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 3, 1, 1);

						Win7CreateSolidLabelElement(button.shortcutElement, button.shortcutComposition, L"MenuItem-Shortcut", Alignment::Right, Alignment::Center);
						cell->AddChild(button.shortcutComposition);
					}
					{
						button.subMenuArrowElement=common_styles::CommonFragmentBuilder::BuildRightArrow();

						GuiCellComposition* cell=new GuiCellComposition;
						button.subMenuArrowComposition=cell;
						cell->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
						table->AddChild(cell);
						cell->SetSite(0, 4, 1, 1);
						cell->SetOwnedElement(button.subMenuArrowElement);
						cell->SetVisible(false);
					}
				}
				return button;
			}

			void Win7MenuItemButtonElements::Apply(const Win7ButtonColors& colors)
			{
				borderElement->SetColor(colors.borderColor);
				backgroundElement->SetColor(colors.backgroundColor);
				gradientElement->SetColors(colors.g1, colors.g2);
				splitterElement->SetColors(colors.g3, colors.g4);
				textElement->SetColor(colors.textColor);
				shortcutElement->SetColor(colors.textColor);
				subMenuArrowElement->SetBackgroundColor(colors.textColor);
				subMenuArrowElement->SetBorderColor(colors.textColor);
			}

			void Win7MenuItemButtonElements::SetActive(bool value)
			{
				if(value)
				{
					splitterComposition->SetMargin(Margin(0, 1, 0, 2));
				}
				else
				{
					splitterComposition->SetMargin(Margin(0, 0, 0, 0));
				}
			}

			void Win7MenuItemButtonElements::SetSubMenuExisting(bool value)
			{
				subMenuArrowComposition->SetVisible(value);
			}

/***********************************************************************
Win7TextBoxColors
***********************************************************************/

			Win7TextBoxColors Win7TextBoxColors::Blend(const Win7TextBoxColors& c1, const Win7TextBoxColors& c2, vint ratio, vint total)
			{
				if(ratio<0) ratio=0;
				else if(ratio>total) ratio=total;

				Win7TextBoxColors result;
				result.borderColor=BlendColor(c1.borderColor, c2.borderColor, ratio, total);
				result.backgroundColor=BlendColor(c1.backgroundColor, c2.backgroundColor, ratio, total);
				return result;
			}
			
			Win7TextBoxColors Win7TextBoxColors::Normal()
			{
				Win7TextBoxColors result=
				{
					Color(197, 197, 197),
					Color(255, 255, 255),
				};
				return result;
			}

			Win7TextBoxColors Win7TextBoxColors::Active()
			{
				Win7TextBoxColors result=
				{
					Color(197, 218, 237),
					Color(255, 255, 255),
				};
				return result;
			}

			Win7TextBoxColors Win7TextBoxColors::Focused()
			{
				Win7TextBoxColors result=
				{
					Color(61, 123, 173),
					Color(255, 255, 255),
				};
				return result;
			}

			Win7TextBoxColors Win7TextBoxColors::Disabled()
			{
				Win7TextBoxColors result=
				{
					Color(175, 175, 175),
					Win7GetSystemWindowColor(),
				};
				return result;
			}

/***********************************************************************
Helpers
***********************************************************************/

			Color Win7GetSystemWindowColor()
			{
				return Color(240, 240, 240);
			}

			Color Win7GetSystemTabContentColor()
			{
				return Color(255, 255, 255);
			}

			Color Win7GetSystemBorderColor()
			{
				return Color(100, 100, 100);
			}

			Color Win7GetSystemBorderSinkColor()
			{
				return Color(227, 227, 227);
			}

			Color Win7GetSystemBorderRaiseColor()
			{
				return Color(255, 255, 255);
			}

			Color Win7GetSystemTextColor(bool enabled)
			{
				return enabled?Color(0, 0, 0):Color(131, 131, 131);
			}

			void Win7SetFont(GuiSolidLabelElement* element, GuiBoundsComposition* composition, const FontProperties& fontProperties)
			{
				vint margin=3;
				element->SetFont(fontProperties);
				composition->SetMargin(Margin(margin, margin, margin, margin));
			}

			void Win7CreateSolidLabelElement(GuiSolidLabelElement*& element, GuiBoundsComposition*& composition, Alignment horizontal, Alignment vertical)
			{
				element=GuiSolidLabelElement::Create();
				element->SetAlignments(horizontal, vertical);

				composition=new GuiBoundsComposition;
				composition->SetOwnedElement(element);
				composition->SetMargin(Margin(0, 0, 0, 0));
				composition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
				composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
			}

			void Win7CreateSolidLabelElement(elements::GuiSolidLabelElement*& element, compositions::GuiSharedSizeItemComposition*& composition, const WString& group, Alignment horizontal, Alignment vertical)
			{
				element=GuiSolidLabelElement::Create();
				element->SetAlignments(horizontal, vertical);

				composition=new GuiSharedSizeItemComposition;
				composition->SetGroup(group);
				composition->SetSharedWidth(true);
				composition->SetOwnedElement(element);
				composition->SetMargin(Margin(0, 0, 0, 0));
				composition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
				composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
			}

			elements::text::ColorEntry Win7GetTextBoxTextColor()
			{
				elements::text::ColorEntry entry;
				entry.normal.text=Color(0, 0, 0);
				entry.normal.background=Color(0, 0, 0, 0);
				entry.selectedFocused.text=Color(255, 255, 255);
				entry.selectedFocused.background=Color(51, 153, 255);
				entry.selectedUnfocused.text=Color(255, 255, 255);
				entry.selectedUnfocused.background=Color(51, 153, 255);
				return entry;
			}
		}
	}
}