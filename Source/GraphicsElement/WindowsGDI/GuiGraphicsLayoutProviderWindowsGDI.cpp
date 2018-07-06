#include "GuiGraphicsLayoutProviderWindowsGDI.h"
#include "GuiGraphicsUniscribe.h"
#include <math.h>

namespace vl
{
	namespace presentation
	{
		namespace elements_windows_gdi
		{
			using namespace elements;
			using namespace collections;
			using namespace windows;

/***********************************************************************
WindowsGDIParagraph
***********************************************************************/

			class WindowsGDIParagraph : public Object, public IGuiGraphicsParagraph, protected UniscribeRun::IRendererCallback
			{
			protected:
				IGuiGraphicsLayoutProvider*			provider;
				Ptr<UniscribeParagraph>				paragraph;
				WString								text;
				IWindowsGDIRenderTarget*			renderTarget;

				vint								caret;
				Color								caretColor;
				bool								caretFrontSide;
				Ptr<WinPen>							caretPen;

				WinDC*								paragraphDC;
				Point								paragraphOffset;
				IGuiGraphicsParagraphCallback*		paragraphCallback;

				void PrepareUniscribeData()
				{
					if(paragraph->BuildUniscribeData(renderTarget->GetDC()))
					{
						vint width=paragraph->lastAvailableWidth==-1?65536:paragraph->lastAvailableWidth;
						paragraph->Layout(width, paragraph->paragraphAlignment);
					}
				}

				WinDC* GetWinDC()
				{
					return paragraphDC;
				}

				Point GetParagraphOffset()
				{
					return paragraphOffset;
				}

				IGuiGraphicsParagraphCallback* GetParagraphCallback()
				{
					return paragraphCallback;
				}
			public:
				WindowsGDIParagraph(IGuiGraphicsLayoutProvider* _provider, const WString& _text, IGuiGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _paragraphCallback)
					:provider(_provider)
					,text(_text)
					,renderTarget(dynamic_cast<IWindowsGDIRenderTarget*>(_renderTarget))
					,caret(-1)
					,caretFrontSide(false)
					,paragraphDC(nullptr)
					,paragraphCallback(_paragraphCallback)
				{
					paragraph=new UniscribeParagraph;
					paragraph->paragraphText=text;

					Ptr<UniscribeFragment> fragment=new UniscribeFragment(_text);
					fragment->fontStyle=GetCurrentController()->ResourceService()->GetDefaultFont();
					paragraph->documentFragments.Add(fragment);
				}

				~WindowsGDIParagraph()
				{
					CloseCaret();
				}

				IGuiGraphicsLayoutProvider* GetProvider()override
				{
					return provider;
				}

				IGuiGraphicsRenderTarget* GetRenderTarget()override
				{
					return renderTarget;
				}

				bool GetWrapLine()override
				{
					return true;
				}

				void SetWrapLine(bool value)override
				{
				}

				vint GetMaxWidth()override
				{
					return paragraph->lastAvailableWidth;
				}

				void SetMaxWidth(vint value)override
				{
					paragraph->BuildUniscribeData(renderTarget->GetDC());
					paragraph->Layout(value, paragraph->paragraphAlignment);
				}

				Alignment GetParagraphAlignment()override
				{
					return paragraph->paragraphAlignment;
				}

				void SetParagraphAlignment(Alignment value)override
				{
					paragraph->BuildUniscribeData(renderTarget->GetDC());
					paragraph->Layout(paragraph->lastAvailableWidth, value);
				}

				bool SetFont(vint start, vint length, const WString& value)override
				{
					if(length==0) return true;
					if(0<=start && start<text.Length() && length>=0 && 0<=start+length && start+length<=text.Length())
					{
						return paragraph->SetFont(start, length, value);
					}
					else
					{
						return false;
					}
				}

				bool SetSize(vint start, vint length, vint value)override
				{
					if(length==0) return true;
					if(0<=start && start<text.Length() && length>=0 && 0<=start+length && start+length<=text.Length())
					{
						return paragraph->SetSize(start, length, value);
					}
					else
					{
						return false;
					}
				}

				bool SetStyle(vint start, vint length, TextStyle value)override
				{
					if(length==0) return true;
					if(0<=start && start<text.Length() && length>=0 && 0<=start+length && start+length<=text.Length())
					{
						return paragraph->SetStyle(start, length, (value&Bold)!=0, (value&Italic)!=0, (value&Underline)!=0, (value&Strikeline)!=0);
					}
					else
					{
						return false;
					}
				}

				bool SetColor(vint start, vint length, Color value)override
				{
					if(length==0) return true;
					if(0<=start && start<text.Length() && length>=0 && 0<=start+length && start+length<=text.Length())
					{
						return paragraph->SetColor(start, length, value);
					}
					else
					{
						return false;
					}
				}

				bool SetBackgroundColor(vint start, vint length, Color value)override
				{
					if(length==0) return true;
					if(0<=start && start<text.Length() && length>=0 && 0<=start+length && start+length<=text.Length())
					{
						return paragraph->SetBackgroundColor(start, length, value);
					}
					else
					{
						return false;
					}
				}

				bool SetInlineObject(vint start, vint length, const InlineObjectProperties& properties)override
				{
					if(length==0) return true;
					if(0<=start && start<text.Length() && length>=0 && 0<=start+length && start+length<=text.Length())
					{
						if(paragraph->SetInlineObject(start, length, properties))
						{
							if (properties.backgroundImage)
							{
								IGuiGraphicsRenderer* renderer=properties.backgroundImage->GetRenderer();
								if(renderer)
								{
									renderer->SetRenderTarget(renderTarget);
								}
							}
							return true;
						}
					}
					return false;
				}

				bool ResetInlineObject(vint start, vint length)override
				{
					if(length==0) return true;
					if(0<=start && start<text.Length() && length>=0 && 0<=start+length && start+length<=text.Length())
					{
						if (auto inlineObject = paragraph->ResetInlineObject(start, length))
						{
							if (auto element = inlineObject.Value().backgroundImage)
							{
								auto renderer=element->GetRenderer();
								if(renderer)
								{
									renderer->SetRenderTarget(0);
								}
							}
							return true;
						}
					}
					return false;
				}

				vint GetHeight()override
				{
					PrepareUniscribeData();
					return paragraph->bounds.Height();
				}

				bool OpenCaret(vint _caret, Color _color, bool _frontSide)override
				{
					if(!IsValidCaret(_caret)) return false;
					if(caret!=-1) CloseCaret();
					caret=_caret;
					caretColor=_color;
					caretFrontSide=_frontSide;
					caretPen=GetWindowsGDIResourceManager()->CreateGdiPen(caretColor);
					return true;
				}

				bool CloseCaret()override
				{
					if(caret==-1) return false;
					caret=-1;
					GetWindowsGDIResourceManager()->DestroyGdiPen(caretColor);
					caretPen=0;
					return true;
				}

				void Render(Rect bounds)override
				{
					PrepareUniscribeData();

					paragraphDC = renderTarget->GetDC();
					paragraphOffset = bounds.LeftTop();
					paragraph->Render(this, true);
					paragraph->Render(this, false);
					paragraphDC = 0;

					if(caret!=-1)
					{
						Rect caretBounds=GetCaretBounds(caret, caretFrontSide);
						vint x=caretBounds.x1+bounds.x1;
						vint y1=caretBounds.y1+bounds.y1;
						vint y2=y1+(vint)(caretBounds.Height()*1.5);

						WinDC* dc=renderTarget->GetDC();
						dc->SetPen(caretPen);
						dc->MoveTo(x-1, y1);
						dc->LineTo(x-1, y2);
						dc->MoveTo(x, y1);
						dc->LineTo(x, y2);
					}
				}

				vint GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide)override
				{
					PrepareUniscribeData();
					return paragraph->GetCaret(comparingCaret, position, preferFrontSide);
				}

				Rect GetCaretBounds(vint caret, bool frontSide)override
				{
					PrepareUniscribeData();
					return paragraph->GetCaretBounds(caret, frontSide);
				}

				vint GetCaretFromPoint(Point point)override
				{
					PrepareUniscribeData();
					return paragraph->GetCaretFromPoint(point);
				}

				Nullable<InlineObjectProperties> GetInlineObjectFromPoint(Point point, vint& start, vint& length)override
				{
					PrepareUniscribeData();
					return paragraph->GetInlineObjectFromPoint(point, start, length);
				}

				vint GetNearestCaretFromTextPos(vint textPos, bool frontSide)override
				{
					PrepareUniscribeData();
					return paragraph->GetNearestCaretFromTextPos(textPos, frontSide);
				}

				bool IsValidCaret(vint caret)override
				{
					PrepareUniscribeData();
					return paragraph->IsValidCaret(caret);
				}

				bool IsValidTextPos(vint textPos)override
				{
					PrepareUniscribeData();
					return paragraph->IsValidTextPos(textPos);
				}
			};

/***********************************************************************
WindowsGDILayoutProvider
***********************************************************************/

			Ptr<IGuiGraphicsParagraph> WindowsGDILayoutProvider::CreateParagraph(const WString& text, IGuiGraphicsRenderTarget* renderTarget, elements::IGuiGraphicsParagraphCallback* callback)
			{
				return new WindowsGDIParagraph(this, text, renderTarget, callback);
			}
		}
	}
}