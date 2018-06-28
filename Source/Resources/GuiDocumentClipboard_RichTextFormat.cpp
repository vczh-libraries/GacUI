#include "GuiDocumentClipboard.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace stream;

		namespace document_clipboard_visitors
		{
			class GenerateRtfVisitor : public Object, public DocumentRun::IVisitor
			{
				typedef DocumentModel::ResolvedStyle					ResolvedStyle;
			public:
				List<ResolvedStyle>				styles;
				DocumentModel*					model;
				StreamWriter&					writer;

				List<WString>&					fontTable;
				List<Color>&					colorTable;
				Dictionary<WString, vint>		fontIndex;
				Dictionary<Color, vint>			colorIndex;

				vint GetFont(const WString& fontName)
				{
					vint index = fontIndex.Keys().IndexOf(fontName);
					if (index == -1)
					{
						index = fontTable.Add(fontName);
						fontIndex.Add(fontName, index);
						return index;
					}
					return fontIndex.Values()[index];
				}

				vint GetColor(Color color)
				{
					if (color.a == 0) return 0;
					vint index = colorIndex.Keys().IndexOf(color);
					if (index == -1)
					{
						index = colorTable.Add(color) + 1;
						colorIndex.Add(color, index);
						return index;
					}
					return colorIndex.Values()[index];
				}

				GenerateRtfVisitor(DocumentModel* _model, List<WString>& _fontTable, List<Color>& _colorTable, StreamWriter& _writer)
					:model(_model)
					, writer(_writer)
					, fontTable(_fontTable)
					, colorTable(_colorTable)
				{
					ResolvedStyle style;
					style.color = Color(0, 0, 0, 0);
					style.backgroundColor = Color(0, 0, 0, 0);
					style = model->GetStyle(DocumentModel::DefaultStyleName, style);
					styles.Add(style);
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					FOREACH(Ptr<DocumentRun>, subRun, run->runs)
					{
						subRun->Accept(this);
					}
				}

				void Visit(DocumentTextRun* run)override
				{
					WString text = run->GetRepresentationText();
					if (text.Length() > 0)
					{
						ResolvedStyle style = styles[styles.Count() - 1];

						writer.WriteString(L"{\\f" + itow(GetFont(style.style.fontFamily)));
						writer.WriteString(L"\\fs" + itow((vint)(style.style.size * 1.5)));
						writer.WriteString(L"\\cf" + itow(GetColor(style.color)));
						writer.WriteString(L"\\cb" + itow(GetColor(style.backgroundColor)));
						writer.WriteString(L"\\chshdng" + itow(GetColor(style.backgroundColor)));
						writer.WriteString(L"\\chcbpat" + itow(GetColor(style.backgroundColor)));

						if (style.style.bold) writer.WriteString(L"\\b");
						if (style.style.italic) writer.WriteString(L"\\i");
						if (style.style.underline) writer.WriteString(L"\\ul");
						if (style.style.strikeline) writer.WriteString(L"\\strike");

						for (vint i = 0; i < text.Length(); i++)
						{
							writer.WriteString(L"\\u" + itow(text[i]));
						}

						writer.WriteString(L"}");
					}
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					ResolvedStyle style = styles[styles.Count() - 1];
					style = model->GetStyle(run->style, style);
					styles.Add(style);
					VisitContainer(run);
					styles.RemoveAt(styles.Count() - 1);
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					ResolvedStyle style = styles[styles.Count() - 1];
					style = model->GetStyle(run->styleName, style);
					styles.Add(style);
					VisitContainer(run);
					styles.RemoveAt(styles.Count() - 1);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					ResolvedStyle style = styles[styles.Count() - 1];
					style = model->GetStyle((run->normalStyleName == L"" ? DocumentModel::NormalLinkStyleName : run->normalStyleName), style);
					styles.Add(style);
					VisitContainer(run);
					styles.RemoveAt(styles.Count() - 1);
				}

				void Visit(DocumentImageRun* run)override
				{
					if (run->image)
					{
						writer.WriteString(L"{\\pict\\pngblip");
						writer.WriteString(L"\\picw" + itow(run->size.x) + L"\\pich" + itow(run->size.y));
						writer.WriteString(L"\\picwgoal" + itow(run->size.x * 15) + L"\\pichgoal" + itow(run->size.y * 15) + L" ");

						MemoryStream memoryStream;
						run->image->SaveToStream(memoryStream, INativeImage::Png);
						vint count = (vint)memoryStream.Size();
						vuint8_t* buffer = (vuint8_t*)memoryStream.GetInternalBuffer();
						for (vint i = 0; i < count; i++)
						{
							writer.WriteChar(L"0123456789abcdef"[buffer[i] / 16]);
							writer.WriteChar(L"0123456789abcdef"[buffer[i] % 16]);
						}

						writer.WriteString(L"}");
					}
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
				}

				void Visit(DocumentParagraphRun* run)override
				{
					VisitContainer(run);
				}
			};
		}
		using namespace document_clipboard_visitors;

		void SaveDocumentToRtf(Ptr<DocumentModel> model, AString& rtf)
		{
			List<WString> fontTable;
			List<Color> colorTable;
			MemoryStream bodyStream;
			{
				StreamWriter writer(bodyStream);
				GenerateRtfVisitor visitor(model.Obj(), fontTable, colorTable, writer);

				FOREACH(Ptr<DocumentParagraphRun>, paragraph, model->paragraphs)
				{
					if (paragraph->alignment)
					{
						switch (paragraph->alignment.Value())
						{
						case Alignment::Left: writer.WriteString(L"\\ql{"); break;
						case Alignment::Center: writer.WriteString(L"\\qc{"); break;
						case Alignment::Right: writer.WriteString(L"\\qr{"); break;
						}
					}
					else
					{
						writer.WriteString(L"\\ql{");
					}
					paragraph->Accept(&visitor);
					writer.WriteString(L"}\\par");
				}
			}

			MemoryStream rtfStream;
			{
				Utf8Encoder encoder;
				EncoderStream encoderStream(rtfStream, encoder);
				StreamWriter writer(encoderStream);

				writer.WriteString(L"{\\rtf1\\ansi\\deff0{\\fonttbl");
				FOREACH_INDEXER(WString, fontName, index, fontTable)
				{
					writer.WriteString(L"{\\f");
					writer.WriteString(itow(index));
					writer.WriteString(L" ");
					writer.WriteString(fontName);
					writer.WriteString(L";}");
				}

				writer.WriteString(L"}{\\colortbl");
				FOREACH_INDEXER(Color, color, index, colorTable)
				{
					writer.WriteString(L";\\red");
					writer.WriteString(itow(color.r));
					writer.WriteString(L"\\green");
					writer.WriteString(itow(color.g));
					writer.WriteString(L"\\blue");
					writer.WriteString(itow(color.b));
				}

				writer.WriteString(L";}{\\*\\generator GacUI Document 1.0}\\uc0");
				{
					bodyStream.SeekFromBegin(0);
					StreamReader reader(bodyStream);
					writer.WriteString(reader.ReadToEnd());
				}
				writer.WriteString(L"}");
			}
			char zero = 0;
			rtfStream.Write(&zero, sizeof(zero));
			rtf = (const char*)rtfStream.GetInternalBuffer();
		}

		void SaveDocumentToRtfStream(Ptr<DocumentModel> model, stream::IStream& stream)
		{
			AString rtf;
			SaveDocumentToRtf(model, rtf);
			stream.Write((void*)rtf.Buffer(), rtf.Length());
		}
	}
}