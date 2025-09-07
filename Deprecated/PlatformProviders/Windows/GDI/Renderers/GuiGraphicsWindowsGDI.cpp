			class CachedCharMeasurerAllocator : public GuiCachedResourceAllocatorBase<CachedCharMeasurerAllocator, FontProperties, Ptr<text::CharMeasurer>>
			{
			protected:
				class GdiCharMeasurer : public text::CharMeasurer
				{
				protected:
					Ptr<WinFont>			font;
					vint					size;

					Size MeasureInternal(text::UnicodeCodePoint codePoint, IGuiGraphicsRenderTarget* renderTarget)
					{
						if (renderTarget)
						{
							WindowsGDIRenderTarget* gdiRenderTarget = dynamic_cast<WindowsGDIRenderTarget*>(renderTarget);
							WinDC* dc = gdiRenderTarget->GetDC();
							dc->SetFont(font);

							vint count = text::UTF16SPFirst(codePoint.characters[0]) && text::UTF16SPSecond(codePoint.characters[1]) ? 2 : 1;
							SIZE size = dc->MeasureBuffer(codePoint.characters, count, -1);
							return Size(size.cx, size.cy);
						}
						else
						{
							return Size(0, 0);
						}
					}

					vint MeasureWidthInternal(text::UnicodeCodePoint codePoint, IGuiGraphicsRenderTarget* renderTarget)
					{
						return MeasureInternal(codePoint, renderTarget).x;
					}

					vint GetRowHeightInternal(IGuiGraphicsRenderTarget* renderTarget)
					{
						if (renderTarget)
						{
							return MeasureInternal({ L' ' }, renderTarget).y;
						}
						else
						{
							return size;
						}
					}
				public:
					GdiCharMeasurer(Ptr<WinFont> _font, vint _size)
						: text::CharMeasurer(_size)
						, size(_size)
						, font(_font)
					{
					}
				};
			public:
				Ptr<text::CharMeasurer> CreateInternal(const FontProperties& value)
				{
					return Ptr(new GdiCharMeasurer(CachedFontAllocator::CreateGdiFont(value), value.size));
				}
			};