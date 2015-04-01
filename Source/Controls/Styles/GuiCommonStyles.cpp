#include <math.h>
#include "GuiCommonStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace common_styles
		{
			using namespace elements;
			using namespace compositions;
			using namespace controls;

/***********************************************************************
CommonScrollStyle
***********************************************************************/

			void CommonScrollStyle::UpdateHandle()
			{
				double handlePageSize=(double)pageSize/totalSize;
				double handleRatio=(double)position/totalSize;
				switch(direction)
				{
				case Horizontal:
					handleComposition->SetWidthRatio(handleRatio);
					handleComposition->SetWidthPageSize(handlePageSize);
					break;
				case Vertical:
					handleComposition->SetHeightRatio(handleRatio);
					handleComposition->SetHeightPageSize(handlePageSize);
					break;
				}
			}

			void CommonScrollStyle::OnDecreaseButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(commandExecutor)
				{
					commandExecutor->SmallDecrease();
				}
			}

			void CommonScrollStyle::OnIncreaseButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(commandExecutor)
				{
					commandExecutor->SmallIncrease();
				}
			}

			void CommonScrollStyle::OnHandleMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(commandExecutor && handleButton->GetVisuallyEnabled())
				{
					draggingHandle=true;
					draggingStartLocation=Point(arguments.x, arguments.y);
				}
			}

			void CommonScrollStyle::OnHandleMouseMove(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(draggingHandle)
				{
					vint totalPixels=0;
					vint currentOffset=0;
					vint newOffset=0;
					switch(direction)
					{
					case Horizontal:
						totalPixels=handleComposition->GetParent()->GetBounds().Width();
						currentOffset=handleComposition->GetBounds().Left();
						newOffset=currentOffset+(arguments.x-draggingStartLocation.x);
						break;
					case Vertical:
						totalPixels=handleComposition->GetParent()->GetBounds().Height();
						currentOffset=handleComposition->GetBounds().Top();
						newOffset=currentOffset+(arguments.y-draggingStartLocation.y);
						break;
					}

					double ratio=(double)newOffset/totalPixels;
					vint newPosition=(vint)(ratio*totalSize);

					vint offset1=(vint)(((double)newPosition/totalSize)*totalPixels);
					vint offset2=vint(((double)(newPosition+1)/totalSize)*totalPixels);
					vint delta1=abs((int)(offset1-newOffset));
					vint delta2=abs((int)(offset2-newOffset));
					if(delta1<delta2)
					{
						commandExecutor->SetPosition(newPosition);
					}
					else
					{
						commandExecutor->SetPosition(newPosition+1);
					}
				}
			}

			void CommonScrollStyle::OnHandleMouseUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				draggingHandle=false;
			}

			void CommonScrollStyle::OnBigMoveMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(commandExecutor && handleButton->GetVisuallyEnabled())
				{
					if(arguments.eventSource==arguments.compositionSource)
					{
						Rect handleBounds=handleComposition->GetBounds();
						switch(direction)
						{
						case Horizontal:
							if(arguments.x<handleBounds.x1)
							{
								commandExecutor->BigDecrease();
							}
							else if(arguments.x>=handleBounds.x2)
							{
								commandExecutor->BigIncrease();
							}
							break;
						case Vertical:
							if(arguments.y<handleBounds.y1)
							{
								commandExecutor->BigDecrease();
							}
							else if(arguments.y>=handleBounds.y2)
							{
								commandExecutor->BigIncrease();
							}
							break;
						}
					}
				}
			}

			void CommonScrollStyle::BuildStyle(vint defaultSize, vint arrowSize)
			{
				boundsComposition=new GuiBoundsComposition;
				containerComposition=InstallBackground(boundsComposition, direction);
				{
					GuiBoundsComposition* handleBoundsComposition=new GuiBoundsComposition;
					containerComposition->AddChild(handleBoundsComposition);

					handleComposition=new GuiPartialViewComposition;
					handleBoundsComposition->AddChild(handleComposition);
					handleBoundsComposition->GetEventReceiver()->leftButtonDown.AttachMethod(this, &CommonScrollStyle::OnBigMoveMouseDown);

					switch(direction)
					{
					case Horizontal:
						handleBoundsComposition->SetAlignmentToParent(Margin(defaultSize, 0, defaultSize, 0));
						handleComposition->SetPreferredMinSize(Size(defaultSize, 0));
						boundsComposition->SetPreferredMinSize(Size(0, defaultSize));
						break;
					case Vertical:
						handleBoundsComposition->SetAlignmentToParent(Margin(0, defaultSize, 0, defaultSize));
						handleComposition->SetPreferredMinSize(Size(0, defaultSize));
						boundsComposition->SetPreferredMinSize(Size(defaultSize, 0));
						break;
					}
					
					handleButton=new GuiButton(CreateHandleButtonStyle(direction));
					handleButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					handleComposition->AddChild(handleButton->GetBoundsComposition());

					handleButton->GetBoundsComposition()->GetEventReceiver()->leftButtonDown.AttachMethod(this, &CommonScrollStyle::OnHandleMouseDown);
					handleButton->GetBoundsComposition()->GetEventReceiver()->mouseMove.AttachMethod(this, &CommonScrollStyle::OnHandleMouseMove);
					handleButton->GetBoundsComposition()->GetEventReceiver()->leftButtonUp.AttachMethod(this, &CommonScrollStyle::OnHandleMouseUp);
				}
				{
					decreaseButton=new GuiButton(CreateDecreaseButtonStyle(direction));
					decreaseButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					decreaseButton->Clicked.AttachMethod(this, &CommonScrollStyle::OnDecreaseButtonClicked);
					
					increaseButton=new GuiButton(CreateIncreaseButtonStyle(direction));
					increaseButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					increaseButton->Clicked.AttachMethod(this, &CommonScrollStyle::OnIncreaseButtonClicked);
				}
				{
					GuiSideAlignedComposition* decreaseComposition=new GuiSideAlignedComposition;
					decreaseComposition->SetMaxLength(defaultSize);
					decreaseComposition->SetMaxRatio(0.5);
					decreaseComposition->AddChild(decreaseButton->GetBoundsComposition());
					containerComposition->AddChild(decreaseComposition);

					GuiSideAlignedComposition* increaseComposition=new GuiSideAlignedComposition;
					increaseComposition->SetMaxLength(defaultSize);
					increaseComposition->SetMaxRatio(0.5);
					increaseComposition->AddChild(increaseButton->GetBoundsComposition());
					containerComposition->AddChild(increaseComposition);

					switch(direction)
					{
					case Horizontal:
						{
							decreaseComposition->SetDirection(GuiSideAlignedComposition::Left);
							increaseComposition->SetDirection(GuiSideAlignedComposition::Right);
						}
						break;
					case Vertical:
						{
							decreaseComposition->SetDirection(GuiSideAlignedComposition::Top);
							increaseComposition->SetDirection(GuiSideAlignedComposition::Bottom);
						}
						break;
					}
				}
			}

			CommonScrollStyle::CommonScrollStyle(Direction _direction)
				:direction(_direction)
				,commandExecutor(0)
				,decreaseButton(0)
				,increaseButton(0)
				,boundsComposition(0)
				,containerComposition(0)
				,totalSize(1)
				,pageSize(1)
				,position(0)
				,draggingHandle(false)
			{
			}

			CommonScrollStyle::~CommonScrollStyle()
			{
			}

			compositions::GuiBoundsComposition* CommonScrollStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* CommonScrollStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void CommonScrollStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void CommonScrollStyle::SetText(const WString& value)
			{
			}

			void CommonScrollStyle::SetFont(const FontProperties& value)
			{
			}

			void CommonScrollStyle::SetVisuallyEnabled(bool value)
			{
			}

			void CommonScrollStyle::SetCommandExecutor(controls::GuiScroll::ICommandExecutor* value)
			{
				commandExecutor=value;
			}

			void CommonScrollStyle::SetTotalSize(vint value)
			{
				if(totalSize!=value)
				{
					totalSize=value;
					UpdateHandle();
				}
			}

			void CommonScrollStyle::SetPageSize(vint value)
			{
				if(pageSize!=value)
				{
					pageSize=value;
					UpdateHandle();
				}
			}

			void CommonScrollStyle::SetPosition(vint value)
			{
				if(position!=value)
				{
					position=value;
					UpdateHandle();
				}
			}

/***********************************************************************
CommonTrackStyle
***********************************************************************/

			void CommonTrackStyle::UpdateHandle()
			{
				vint maxSize=totalSize-pageSize;
				if(maxSize<1) maxSize=1;
				double ratio=(double)position/maxSize;
				switch(direction)
				{
				case Horizontal:
					handleComposition->SetColumnOption(0, GuiCellOption::PercentageOption(ratio));
					handleComposition->SetColumnOption(2, GuiCellOption::PercentageOption(1-ratio));
					break;
				case Vertical:
					handleComposition->SetRowOption(0, GuiCellOption::PercentageOption(1-ratio));
					handleComposition->SetRowOption(2, GuiCellOption::PercentageOption(ratio));
					break;
				}
				handleComposition->UpdateCellBounds();
			}

			void CommonTrackStyle::OnHandleMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(commandExecutor && handleButton->GetVisuallyEnabled())
				{
					draggingHandle=true;
					draggingStartLocation=Point(arguments.x, arguments.y);
				}
			}

			void CommonTrackStyle::OnHandleMouseMove(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(draggingHandle)
				{
					vint totalPixels=0;
					vint currentOffset=0;
					vint newOffset=0;

					Rect handleArea=handleComposition->GetBounds();
					Rect handleBounds=handleButton->GetBoundsComposition()->GetParent()->GetBounds();
					switch(direction)
					{
					case Horizontal:
						totalPixels=handleArea.Width()-handleBounds.Width();
						currentOffset=handleBounds.Left();
						newOffset=currentOffset+(arguments.x-draggingStartLocation.x);
						break;
					case Vertical:
						totalPixels=handleArea.Height()-handleBounds.Height();
						currentOffset=handleArea.Height()-handleBounds.Bottom();
						newOffset=currentOffset-(arguments.y-draggingStartLocation.y);
						break;
					}

					double ratio=(double)newOffset/totalPixels;
					vint maxSize=totalSize-pageSize;
					if(maxSize<1) maxSize=1;
					vint newPosition=(vint)(ratio*maxSize);

					vint offset1=(vint)(((double)newPosition/maxSize)*totalPixels);
					vint offset2=vint(((double)(newPosition+1)/maxSize)*totalPixels);
					vint delta1=abs((int)(offset1-newOffset));
					vint delta2=abs((int)(offset2-newOffset));
					if(delta1<delta2)
					{
						commandExecutor->SetPosition(newPosition);
					}
					else
					{
						commandExecutor->SetPosition(newPosition+1);
					}
				}
			}

			void CommonTrackStyle::OnHandleMouseUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				draggingHandle=false;
			}

			void CommonTrackStyle::BuildStyle(vint trackThickness, vint trackPadding, vint handleLong, vint handleShort)
			{
				boundsComposition=new GuiBoundsComposition;
				InstallBackground(boundsComposition, direction);
				{
					GuiTableComposition* table=new GuiTableComposition;
					boundsComposition->AddChild(table);
					boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetRowsAndColumns(3, 3);
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					switch(direction)
					{
					case Horizontal:
						table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
						table->SetRowOption(1, GuiCellOption::AbsoluteOption(handleLong));
						table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
						table->SetColumnOption(0, GuiCellOption::AbsoluteOption(trackPadding));
						table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
						table->SetColumnOption(2, GuiCellOption::AbsoluteOption(trackPadding));
						break;
					case Vertical:
						table->SetRowOption(0, GuiCellOption::AbsoluteOption(trackPadding));
						table->SetRowOption(1, GuiCellOption::PercentageOption(1.0));
						table->SetRowOption(2, GuiCellOption::AbsoluteOption(trackPadding));
						table->SetColumnOption(0, GuiCellOption::PercentageOption(0.5));
						table->SetColumnOption(1, GuiCellOption::AbsoluteOption(handleLong));
						table->SetColumnOption(2, GuiCellOption::PercentageOption(0.5));
						break;
					}
					{
						GuiCellComposition* contentCell=new GuiCellComposition;
						table->AddChild(contentCell);
						contentCell->SetSite(1, 1, 1, 1);
						{
							GuiTableComposition* trackTable=new GuiTableComposition;
							trackTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
							contentCell->AddChild(trackTable);
							GuiCellComposition* trackCell=new GuiCellComposition;
							trackTable->AddChild(trackCell);

							switch(direction)
							{
							case Horizontal:
								trackTable->SetRowsAndColumns(3, 1);
								trackTable->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
								trackTable->SetRowOption(1, GuiCellOption::AbsoluteOption(trackThickness));
								trackTable->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
								trackCell->SetSite(1, 0, 1, 1);
								break;
							case Vertical:
								trackTable->SetRowsAndColumns(1, 3);
								trackTable->SetColumnOption(0, GuiCellOption::PercentageOption(0.5));
								trackTable->SetColumnOption(1, GuiCellOption::AbsoluteOption(trackThickness));
								trackTable->SetColumnOption(2, GuiCellOption::PercentageOption(0.5));
								trackCell->SetSite(0, 1, 1, 1);
								break;
							}
							InstallTrack(trackCell, direction);
						}
						{
							handleComposition=new GuiTableComposition;
							handleComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
							contentCell->AddChild(handleComposition);
							GuiCellComposition* handleCell=new GuiCellComposition;
							handleComposition->AddChild(handleCell);

							switch(direction)
							{
							case Horizontal:
								handleComposition->SetRowsAndColumns(1, 3);
								handleComposition->SetColumnOption(0, GuiCellOption::PercentageOption(0.0));
								handleComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(handleShort));
								handleComposition->SetColumnOption(2, GuiCellOption::PercentageOption(1.0));
								handleCell->SetSite(0, 1, 1, 1);
								break;
							case Vertical:
								handleComposition->SetRowsAndColumns(3, 1);
								handleComposition->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
								handleComposition->SetRowOption(1, GuiCellOption::AbsoluteOption(handleShort));
								handleComposition->SetRowOption(2, GuiCellOption::PercentageOption(0.0));
								handleCell->SetSite(1, 0, 1, 1);
								break;
							}

							handleButton=new GuiButton(CreateHandleButtonStyle(direction));
							handleButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
							handleCell->AddChild(handleButton->GetBoundsComposition());

							handleButton->GetBoundsComposition()->GetEventReceiver()->leftButtonDown.AttachMethod(this, &CommonTrackStyle::OnHandleMouseDown);
							handleButton->GetBoundsComposition()->GetEventReceiver()->mouseMove.AttachMethod(this, &CommonTrackStyle::OnHandleMouseMove);
							handleButton->GetBoundsComposition()->GetEventReceiver()->leftButtonUp.AttachMethod(this, &CommonTrackStyle::OnHandleMouseUp);
						}
					}
				}
			}

			CommonTrackStyle::CommonTrackStyle(Direction _direction)
				:direction(_direction)
				,commandExecutor(0)
				,totalSize(1)
				,pageSize(1)
				,position(0)
				,draggingHandle(false)
			{
			}

			CommonTrackStyle::~CommonTrackStyle()
			{
			}

			compositions::GuiBoundsComposition* CommonTrackStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* CommonTrackStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void CommonTrackStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void CommonTrackStyle::SetText(const WString& value)
			{
			}

			void CommonTrackStyle::SetFont(const FontProperties& value)
			{
			}

			void CommonTrackStyle::SetVisuallyEnabled(bool value)
			{
			}

			void CommonTrackStyle::SetCommandExecutor(controls::GuiScroll::ICommandExecutor* value)
			{
				commandExecutor=value;
				if(value)
				{
					value->SetPageSize(0);
				}
			}

			void CommonTrackStyle::SetTotalSize(vint value)
			{
				if(totalSize!=value)
				{
					totalSize=value;
					UpdateHandle();
				}
			}

			void CommonTrackStyle::SetPageSize(vint value)
			{
				if(pageSize!=value)
				{
					pageSize=value;
					UpdateHandle();
				}
			}

			void CommonTrackStyle::SetPosition(vint value)
			{
				if(position!=value)
				{
					position=value;
					UpdateHandle();
				}
			}

/***********************************************************************
CommonFragmentBuilder
***********************************************************************/

			void CommonFragmentBuilder::FillUpArrow(elements::GuiPolygonElement* element)
			{
				Point points[]={Point(0, 3), Point(3, 0), Point(6, 3)};
				element->SetSize(Size(7, 4));
				element->SetPoints(points, sizeof(points)/sizeof(*points));
			}

			void CommonFragmentBuilder::FillDownArrow(elements::GuiPolygonElement* element)
			{
				Point points[]={Point(0, 0), Point(3, 3), Point(6, 0)};
				element->SetSize(Size(7, 4));
				element->SetPoints(points, sizeof(points)/sizeof(*points));
			}

			void CommonFragmentBuilder::FillLeftArrow(elements::GuiPolygonElement* element)
			{
				Point points[]={Point(3, 0), Point(0, 3), Point(3, 6)};
				element->SetSize(Size(4, 7));
				element->SetPoints(points, sizeof(points)/sizeof(*points));
			}

			void CommonFragmentBuilder::FillRightArrow(elements::GuiPolygonElement* element)
			{
				Point points[]={Point(0, 0), Point(3, 3), Point(0, 6)};
				element->SetSize(Size(4, 7));
				element->SetPoints(points, sizeof(points)/sizeof(*points));
			}

			elements::GuiPolygonElement* CommonFragmentBuilder::BuildUpArrow()
			{
				GuiPolygonElement* element=GuiPolygonElement::Create();
				FillUpArrow(element);
				element->SetBorderColor(Color(0, 0, 0));
				element->SetBackgroundColor(Color(0, 0, 0));
				return element;
			}

			elements::GuiPolygonElement* CommonFragmentBuilder::BuildDownArrow()
			{
				GuiPolygonElement* element=GuiPolygonElement::Create();
				FillDownArrow(element);
				element->SetBorderColor(Color(0, 0, 0));
				element->SetBackgroundColor(Color(0, 0, 0));
				return element;
			}

			elements::GuiPolygonElement* CommonFragmentBuilder::BuildLeftArrow()
			{
				GuiPolygonElement* element=GuiPolygonElement::Create();
				FillLeftArrow(element);
				element->SetBorderColor(Color(0, 0, 0));
				element->SetBackgroundColor(Color(0, 0, 0));
				return element;
			}

			elements::GuiPolygonElement* CommonFragmentBuilder::BuildRightArrow()
			{
				GuiPolygonElement* element=GuiPolygonElement::Create();
				FillRightArrow(element);
				element->SetBorderColor(Color(0, 0, 0));
				element->SetBackgroundColor(Color(0, 0, 0));
				return element;
			}

			compositions::GuiBoundsComposition* CommonFragmentBuilder::BuildDockedElementContainer(elements::IGuiGraphicsElement* element)
			{
				GuiBoundsComposition* composition=new GuiBoundsComposition;
				composition->SetOwnedElement(element);
				composition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
				composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				return composition;
			}

			compositions::GuiBoundsComposition* CommonFragmentBuilder::BuildUpArrow(elements::GuiPolygonElement*& elementOut)
			{
				elementOut=BuildUpArrow();
				return BuildDockedElementContainer(elementOut);
			}

			compositions::GuiBoundsComposition* CommonFragmentBuilder::BuildDownArrow(elements::GuiPolygonElement*& elementOut)
			{
				elementOut=BuildDownArrow();
				return BuildDockedElementContainer(elementOut);
			}

			compositions::GuiBoundsComposition* CommonFragmentBuilder::BuildLeftArrow(elements::GuiPolygonElement*& elementOut)
			{
				elementOut=BuildLeftArrow();
				return BuildDockedElementContainer(elementOut);
			}

			compositions::GuiBoundsComposition* CommonFragmentBuilder::BuildRightArrow(elements::GuiPolygonElement*& elementOut)
			{
				elementOut=BuildRightArrow();
				return BuildDockedElementContainer(elementOut);
			}
		}

/***********************************************************************
Helpers
***********************************************************************/

		unsigned char IntToColor(vint color)
		{
			return color<0?0:color>255?255:(unsigned char)color;
		}

		Color BlendColor(Color c1, Color c2, vint currentPosition, vint totalLength)
		{
			return Color(
				(unsigned char)IntToColor((c2.r*currentPosition+c1.r*(totalLength-currentPosition))/totalLength),
				(unsigned char)IntToColor((c2.g*currentPosition+c1.g*(totalLength-currentPosition))/totalLength),
				(unsigned char)IntToColor((c2.b*currentPosition+c1.b*(totalLength-currentPosition))/totalLength),
				(unsigned char)IntToColor((c2.a*currentPosition+c1.a*(totalLength-currentPosition))/totalLength)
				);
		}
	}
}