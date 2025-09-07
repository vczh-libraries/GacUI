
			class CachedCharMeasurerAllocator : public GuiCachedResourceAllocatorBase<CachedCharMeasurerAllocator, FontProperties, Ptr<text::CharMeasurer>>
			{
			protected:
				class Direct2DCharMeasurer : public text::CharMeasurer
				{
				protected:
					ComPtr<IDWriteTextFormat>	font;
					vint						size;

					Size MeasureInternal(text::UnicodeCodePoint codePoint, IGuiGraphicsRenderTarget* renderTarget)
					{
						Size charSize(0, 0);
						IDWriteTextLayout* textLayout = 0;

						UINT32 count = text::UTF16SPFirst(codePoint.characters[0]) && text::UTF16SPSecond(codePoint.characters[1]) ? 2 : 1;
						HRESULT hr = GetWindowsDirect2DObjectProvider()->GetDirectWriteFactory()->CreateTextLayout(
							codePoint.characters,
							count,
							font.Obj(),
							0,
							0,
							&textLayout);
						CHECK_ERROR(SUCCEEDED(hr), L"You should check HRESULT to see why it failed.");

						DWRITE_TEXT_METRICS metrics;
						hr = textLayout->GetMetrics(&metrics);
						if (!FAILED(hr))
						{
							charSize = Size((vint)ceil(metrics.widthIncludingTrailingWhitespace), (vint)ceil(metrics.height));
						}
						textLayout->Release();
						return charSize;
					}

					vint MeasureWidthInternal(text::UnicodeCodePoint codePoint, IGuiGraphicsRenderTarget* renderTarget)
					{
						return MeasureInternal(codePoint, renderTarget).x;
					}

					vint GetRowHeightInternal(IGuiGraphicsRenderTarget* renderTarget)
					{
						return MeasureInternal({ L' ' }, renderTarget).y;
					}
				public:
					Direct2DCharMeasurer(ComPtr<IDWriteTextFormat> _font, vint _size)
						:text::CharMeasurer(_size)
						,size(_size)
						,font(_font)
					{
					}
				};
			public:
				Ptr<text::CharMeasurer> CreateInternal(const FontProperties& value)
				{
					return Ptr(new Direct2DCharMeasurer(CachedTextFormatAllocator::CreateDirect2DFont(value), value.size));
				}
			};