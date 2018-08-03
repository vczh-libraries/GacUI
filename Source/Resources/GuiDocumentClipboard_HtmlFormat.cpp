#include "GuiDocumentClipboard.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace stream;

		namespace document_clipboard_visitors
		{
			class GenerateHtmlVisitor : public Object, public DocumentRun::IVisitor
			{
				typedef DocumentModel::ResolvedStyle					ResolvedStyle;
			public:
				List<ResolvedStyle>				styles;
				DocumentModel*					model;
				StreamWriter&					writer;

				GenerateHtmlVisitor(DocumentModel* _model, StreamWriter& _writer)
					:model(_model)
					, writer(_writer)
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

				WString ColorToString(Color c)
				{
					auto result = c.ToString();
					if (result.Length() == 9) result = result.Left(7);
					return result;
				}

				void Visit(DocumentTextRun* run)override
				{
					WString text = run->GetRepresentationText();
					if (text.Length() > 0)
					{
						ResolvedStyle style = styles[styles.Count() - 1];

						writer.WriteString(L"<span style=\"");
						if (style.style.bold) writer.WriteString(L"font-weight:bold; ");
						if (style.style.italic) writer.WriteString(L"font-style:italic; ");
						if (style.style.underline && style.style.strikeline) writer.WriteString(L"text-decoration:underline line-through; ");
						else if (style.style.underline) writer.WriteString(L"text-decoration:underline; ");
						else if (style.style.strikeline) writer.WriteString(L"text-decoration:line-through; ");
						if (style.style.fontFamily != L"") writer.WriteString(L"font-family:" + style.style.fontFamily + L"; ");
						if (style.style.size != 0) writer.WriteString(L"font-size:" + itow(style.style.size) + L"px; ");
						if (style.color.a != 0) writer.WriteString(L"color:" + ColorToString(style.color) + L"; ");
						if (style.backgroundColor.a != 0)writer.WriteString(L"background-color:" + ColorToString(style.backgroundColor) + L"; ");
						writer.WriteString(L"\">");

						for (vint i = 0; i < text.Length(); i++)
						{
							switch (wchar_t c = text[i])
							{
							case L'&': writer.WriteString(L"&amp;"); break;
							case L'<': writer.WriteString(L"&lt;"); break;
							case L'>': writer.WriteString(L"&gt;"); break;
							case L'\r': break;
							case L'\n': writer.WriteString(L"<br>"); break;
							case L' ': writer.WriteString(L"&nbsp;"); break;
							case L'\t': writer.WriteString(L"<pre>\t</pre>"); break;
							default: writer.WriteChar(c); break;
							}
						}

						writer.WriteString(L"</span>");
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
					writer.WriteString(L"<a href=\"");
					for (vint i = 0; i < run->reference.Length(); i++)
					{
						switch (wchar_t c = run->reference[i])
						{
						case L'&': writer.WriteString(L"&amp;"); break;
						case L'<': writer.WriteString(L"&lt;"); break;
						case L'>': writer.WriteString(L"&gt;"); break;
						case L'"': writer.WriteString(L"&quot;"); break;
						case L'\'': writer.WriteString(L"&#39;"); break;
						default: writer.WriteChar(c); break;
						}
					}
					writer.WriteString(L"\">");

					ResolvedStyle style = styles[styles.Count() - 1];
					style = model->GetStyle((run->normalStyleName == L"" ? DocumentModel::NormalLinkStyleName : run->normalStyleName), style);
					styles.Add(style);
					VisitContainer(run);
					styles.RemoveAt(styles.Count() - 1);

					writer.WriteString(L"</a>");
				}

				void Visit(DocumentImageRun* run)override
				{
					if (run->image)
					{
						writer.WriteString(L"<img width=\"" + itow(run->size.x) + L"\" height=\"" + itow(run->size.y) + L"\" src=\"data:image/");
						auto format = run->image->GetFormat();
						if (format == INativeImage::Gif)
						{
							format = INativeImage::Png;
						}

						switch (format)
						{
						case INativeImage::Bmp: writer.WriteString(L"bmp;base64,"); break;
						case INativeImage::Gif: writer.WriteString(L"gif;base64,"); break;
						case INativeImage::Icon: writer.WriteString(L"icon;base64,"); break;
						case INativeImage::Jpeg: writer.WriteString(L"jpeg;base64,"); break;
						case INativeImage::Png: writer.WriteString(L"png;base64,"); break;
						case INativeImage::Tiff: writer.WriteString(L"tiff;base64,"); break;
						case INativeImage::Wmp: writer.WriteString(L"wmp;base64,"); break;
						default: writer.WriteString(L"unsupported;base64,\"/>"); return;
						}

						MemoryStream memoryStream;
						run->image->SaveToStream(memoryStream, format);
						memoryStream.SeekFromBegin(0);
						while (true)
						{
							vuint8_t bytes[3] = { 0,0,0 };
							vint read = memoryStream.Read(&bytes, sizeof(bytes));
							if (read == 0) break;

							vuint8_t b1 = bytes[0] / (1 << 2);
							vuint8_t b2 = ((bytes[0] % (1 << 2)) << 4) + bytes[1] / (1 << 4);
							vuint8_t b3 = ((bytes[1] % (1 << 4)) << 2) + bytes[2] / (1 << 6);
							vuint8_t b4 = bytes[2] % (1 << 6);

							const wchar_t* BASE64 =
								L"ABCDEFG"
								L"HIJKLMN"
								L"OPQRST"
								L"UVWXYZ"
								L"abcdefg"
								L"hijklmn"
								L"opqrst"
								L"uvwxyz"
								L"0123456789"
								L"+/";
#define BASE64_CHAR(b)		BASE64[b]
							switch (read)
							{
							case 1:
								writer.WriteChar(BASE64_CHAR(b1));
								writer.WriteChar(BASE64_CHAR(b2));
								writer.WriteChar(L'=');
								writer.WriteChar(L'=');
								break;
							case 2:
								writer.WriteChar(BASE64_CHAR(b1));
								writer.WriteChar(BASE64_CHAR(b2));
								writer.WriteChar(BASE64_CHAR(b3));
								writer.WriteChar(L'=');
								break;
							case 3:
								writer.WriteChar(BASE64_CHAR(b1));
								writer.WriteChar(BASE64_CHAR(b2));
								writer.WriteChar(BASE64_CHAR(b3));
								writer.WriteChar(BASE64_CHAR(b4));
								break;
							}
#undef BASE64_CHAR
						}

						writer.WriteString(L"\"/>");
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

#define HTML_LINE(LINE) LINE "\r\n"

		void SaveDocumentToHtmlUtf8(Ptr<DocumentModel> model, AString& header, AString& content, AString& footer)
		{
			header =
				HTML_LINE("<!DOCTYPE html>")
				HTML_LINE("<html>")
				HTML_LINE("<header>")
				HTML_LINE("<title>GacUI Document 1.0</title>")
				HTML_LINE("<meta charset=\"utf-8\"/>")
				HTML_LINE("</header>")
				HTML_LINE("<body>")
				;

			MemoryStream memoryStream;
			{
				Utf8Encoder encoder;
				EncoderStream encoderStream(memoryStream, encoder);
				StreamWriter writer(encoderStream);
				GenerateHtmlVisitor visitor(model.Obj(), writer);

				FOREACH(Ptr<DocumentParagraphRun>, paragraph, model->paragraphs)
				{
					writer.WriteString(L"<p style=\"text-align:");
					if (paragraph->alignment)
					{
						switch (paragraph->alignment.Value())
						{
						case Alignment::Left: writer.WriteString(L"left;"); break;
						case Alignment::Center: writer.WriteString(L"center;"); break;
						case Alignment::Right: writer.WriteString(L"right;"); break;
						}
					}
					else
					{
						writer.WriteString(L"left;");
					}
					writer.WriteString(L"\">");
					paragraph->Accept(&visitor);
					writer.WriteString(L"</p>\r\n");
				}
			}
			char zero = 0;
			memoryStream.Write(&zero, sizeof(zero));
			content = (const char*)memoryStream.GetInternalBuffer();

			footer =
				HTML_LINE("</body>")
				HTML_LINE("</html>")
				;
		}

		void SaveDocumentToHtmlClipboardStream(Ptr<DocumentModel> model, stream::IStream& stream)
		{
			AString header, content, footer;
			SaveDocumentToHtmlUtf8(model, header, content, footer);

			char clipboardHeader[] =
				HTML_LINE("StartHTML:-1")
				HTML_LINE("EndHTML:-1")
				HTML_LINE("StartFragment:0000000000")
				HTML_LINE("EndFragment:0000000000")
				;
			char commentStart[] = "<!--StartFragment-->";
			char commentEnd[] = "<!--EndFragment-->";
			vint offsetStart = sizeof(clipboardHeader) - 1 + header.Length() + sizeof(commentStart) - 1;
			vint offsetEnd = offsetStart + content.Length();

			AString offsetStartString = itoa(offsetStart);
			AString offsetEndString = itoa(offsetEnd);
			memcpy(strstr(clipboardHeader, "EndFragment:") - offsetStartString.Length() - 2, offsetStartString.Buffer(), offsetStartString.Length());
			memcpy(clipboardHeader + sizeof(clipboardHeader) - 1 - offsetEndString.Length() - 2, offsetEndString.Buffer(), offsetEndString.Length());

			stream.Write(clipboardHeader, sizeof(clipboardHeader) - 1);
			if (header.Length() > 0) stream.Write((void*)header.Buffer(), header.Length());
			stream.Write(commentStart, sizeof(commentStart) - 1);
			if (content.Length() > 0) stream.Write((void*)content.Buffer(), content.Length());
			stream.Write(commentEnd, sizeof(commentEnd) - 1);
			if (footer.Length() > 0) stream.Write((void*)footer.Buffer(), footer.Length());
		}

#undef HTML_LINE
	}
}