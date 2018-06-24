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
						if (style.style.bold) writer.WriteString(L"<b>");
						if (style.style.italic) writer.WriteString(L"<i>");
						if (style.style.underline) writer.WriteString(L"<ins>");
						if (style.style.strikeline) writer.WriteString(L"<del>");

						bool span = style.style.fontFamily != L"" || style.style.size != 0 || style.color.a != 0 || style.backgroundColor.a != 0;
						if (span)
						{
							writer.WriteString(L"<span style=\"");
							if (style.style.fontFamily != L"") writer.WriteString(L"font-family:" + style.style.fontFamily + L"; ");
							if (style.style.size != 0) writer.WriteString(L"font-size:" + itow(style.style.size) + L"px; ");
							if (style.color.a != 0) writer.WriteString(L"color:" + ColorToString(style.color) + L"; ");
							if (style.backgroundColor.a != 0)writer.WriteString(L"background-color:" + ColorToString(style.backgroundColor) + L"; ");
							writer.WriteString(L"\">");
						}

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

						if (span) writer.WriteString(L"</span>");
						if (style.style.strikeline) writer.WriteString(L"</del>");
						if (style.style.underline) writer.WriteString(L"</ins>");
						if (style.style.italic) writer.WriteString(L"</i>");
						if (style.style.bold) writer.WriteString(L"</b>");
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
					style = model->GetStyle(DocumentModel::NormalLinkStyleName, style);
					styles.Add(style);
					VisitContainer(run);
					styles.RemoveAt(styles.Count() - 1);
				}

				void Visit(DocumentImageRun* run)override
				{
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
					writer.WriteString(L"<p>");
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