#include "GuiDocument.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace parsing::xml;

/***********************************************************************
document_operation_visitors::SerializeRunVisitor
***********************************************************************/

		namespace document_operation_visitors
		{
			class SerializeRunVisitor : public Object, public DocumentRun::IVisitor
			{
			protected:
				Ptr<XmlElement>				parent;

			public:
				SerializeRunVisitor(Ptr<XmlElement> _parent)
					:parent(_parent)
				{
				}

				void VisitContainer(Ptr<XmlElement> replacedParent, DocumentContainerRun* run)
				{
					if (replacedParent)
					{
						parent->subNodes.Add(replacedParent);
						Ptr<XmlElement> oldParent = parent;
						parent = replacedParent;
						FOREACH(Ptr<DocumentRun>, subRun, run->runs)
						{
							subRun->Accept(this);
						}
						parent = oldParent;
					}
					else
					{
						FOREACH(Ptr<DocumentRun>, subRun, run->runs)
						{
							subRun->Accept(this);
						}
					}
				}

				void Visit(DocumentTextRun* run)override
				{
					if (run->text != L"")
					{
						auto writer = XmlElementWriter(parent).Element(L"nop");
						auto begin = run->text.Buffer();
						auto reading = begin;
						auto last = reading;
						while (true)
						{
							const wchar_t* tag = nullptr;
							auto c = *reading;
							switch (c)
							{
							case L'\n':
								tag = L"br";
								break;
							case L' ':
								tag = L"sp";
								break;
							case L'\t':
								tag = L"tab";
								break;
							}

							if (tag || c == 0)
							{
								if (reading > last)
								{
									auto end = reading[-1] == L'\r' ? reading - 1 : reading;
									if (end > last)
									{
										writer.Text(run->text.Sub(last - begin, end - last));
									}
									last = reading;
								}
							}

							if (tag)
							{
								writer.Element(tag);
							}
							else if (c == 0)
							{
								break;
							}
							reading++;
						}
					}
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					Ptr<DocumentStyleProperties> sp = run->style;
					Ptr<XmlElement> oldParent = parent;
					if (sp->face || sp->size || sp->color)
					{
						Ptr<XmlElement> element = new XmlElement;
						element->name.value = L"font";
						parent->subNodes.Add(element);

						XmlElementWriter writer(element);
						if (sp->face)
						{
							writer.Attribute(L"face", sp->face.Value());
						}
						if (sp->size)
						{
							writer.Attribute(L"size", sp->size.Value().ToString());
						}
						if (sp->color)
						{
							writer.Attribute(L"color", sp->color.Value().ToString());
						}
						if (sp->backgroundColor)
						{
							writer.Attribute(L"bkcolor", sp->backgroundColor.Value().ToString());
						}
						parent = element;
					}
					if (sp->bold)
					{
						Ptr<XmlElement> element = new XmlElement;
						element->name.value = sp->bold.Value() ? L"b" : L"b-";
						parent->subNodes.Add(element);
						parent = element;
					}
					if (sp->italic)
					{
						Ptr<XmlElement> element = new XmlElement;
						element->name.value = sp->italic.Value() ? L"i" : L"i-";
						parent->subNodes.Add(element);
						parent = element;
					}
					if (sp->underline)
					{
						Ptr<XmlElement> element = new XmlElement;
						element->name.value = sp->underline.Value() ? L"u" : L"u-";
						parent->subNodes.Add(element);
						parent = element;
					}
					if (sp->strikeline)
					{
						Ptr<XmlElement> element = new XmlElement;
						element->name.value = sp->strikeline.Value() ? L"s" : L"s-";
						parent->subNodes.Add(element);
						parent = element;
					}
					if (sp->antialias || sp->verticalAntialias)
					{
						bool ha = sp->antialias ? sp->antialias.Value() : true;
						bool va = sp->verticalAntialias ? sp->verticalAntialias.Value() : false;
						if (!ha)
						{
							Ptr<XmlElement> element = new XmlElement;
							element->name.value = L"ha";
							parent->subNodes.Add(element);
							parent = element;
						}
						else if (!va)
						{
							Ptr<XmlElement> element = new XmlElement;
							element->name.value = L"va";
							parent->subNodes.Add(element);
							parent = element;
						}
						else
						{
							Ptr<XmlElement> element = new XmlElement;
							element->name.value = L"na";
							parent->subNodes.Add(element);
							parent = element;
						}
					}
					VisitContainer(0, run);
					parent = oldParent;
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					Ptr<XmlElement> element = new XmlElement;
					element->name.value = L"div";
					XmlElementWriter(element).Attribute(L"style", run->styleName);
					VisitContainer(element, run);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					Ptr<XmlElement> element = new XmlElement;
					element->name.value = L"a";
					XmlElementWriter writer(element);
					if (run->normalStyleName != L"#NormalLink")
					{
						writer.Attribute(L"normal", run->normalStyleName);
					}
					if (run->activeStyleName != L"#ActiveLink")
					{
						writer.Attribute(L"active", run->activeStyleName);
					}
					if (run->reference != L"")
					{
						writer.Attribute(L"href", run->reference);
					}
					VisitContainer(element, run);
				}

				void Visit(DocumentImageRun* run)override
				{
					XmlElementWriter writer(parent);
					writer
						.Element(L"img")
						.Attribute(L"width", itow(run->size.x))
						.Attribute(L"height", itow(run->size.y))
						.Attribute(L"baseline", itow(run->baseline))
						.Attribute(L"frameIndex", itow(run->frameIndex))
						.Attribute(L"source", run->source)
						;
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					XmlElementWriter writer(parent);
					writer
						.Element(L"object")
						.Attribute(L"name", run->name)
						;
				}

				void Visit(DocumentParagraphRun* run)override
				{
					Ptr<XmlElement> element = new XmlElement;
					element->name.value = L"p";

					XmlElementWriter writer(element);
					if (run->alignment)
					{
						switch (run->alignment.Value())
						{
						case Alignment::Left:
							writer.Attribute(L"align", L"Left");
							break;
						case Alignment::Center:
							writer.Attribute(L"align", L"Center");
							break;
						case Alignment::Right:
							writer.Attribute(L"align", L"Right");
							break;
						}
					}
					VisitContainer(element, run);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
DocumentModel
***********************************************************************/

		Ptr<parsing::xml::XmlDocument> DocumentModel::SaveToXml()
		{
			Ptr<XmlDocument> xml=new XmlDocument;
			Ptr<XmlElement> doc=new XmlElement;
			doc->name.value=L"Doc";
			xml->rootElement=doc;
			{
				Ptr<XmlElement> content=new XmlElement;
				content->name.value=L"Content";
				doc->subNodes.Add(content);
				
				FOREACH(Ptr<DocumentParagraphRun>, p, paragraphs)
				{
					SerializeRunVisitor visitor(content);
					p->Accept(&visitor);
				}
			}
			{
				Ptr<XmlElement> stylesElement=new XmlElement;
				stylesElement->name.value=L"Styles";
				doc->subNodes.Add(stylesElement);

				for(vint i=0;i<styles.Count();i++)
				{
					WString name=styles.Keys()[i];
					if (name.Length()>0 && name[0] == L'#' && (name.Length() <= 9 || name.Right(9) != L"-Override")) continue;

					Ptr<DocumentStyle> style=styles.Values().Get(i);
					Ptr<DocumentStyleProperties> sp=style->styles;
					Ptr<XmlElement> styleElement=new XmlElement;
					styleElement->name.value=L"Style";
					stylesElement->subNodes.Add(styleElement);

					XmlElementWriter(styleElement).Attribute(L"name", name);
					if(style->parentStyleName!=L"")
					{
						XmlElementWriter(styleElement).Attribute(L"parent", style->parentStyleName);
					}

					if(sp->face)				XmlElementWriter(styleElement).Element(L"face").Text(		sp->face.Value()						);
					if(sp->size)				XmlElementWriter(styleElement).Element(L"size").Text(		sp->size.Value().ToString()				);
					if(sp->color)				XmlElementWriter(styleElement).Element(L"color").Text(		sp->color.Value().ToString()			);
					if(sp->backgroundColor)		XmlElementWriter(styleElement).Element(L"bkcolor").Text(	sp->backgroundColor.Value().ToString()	);
					if(sp->bold)				XmlElementWriter(styleElement).Element(L"b").Text(			sp->bold.Value()?L"true":L"false"		);
					if(sp->italic)				XmlElementWriter(styleElement).Element(L"i").Text(			sp->italic.Value()?L"true":L"false"		);
					if(sp->underline)			XmlElementWriter(styleElement).Element(L"u").Text(			sp->underline.Value()?L"true":L"false"	);
					if(sp->strikeline)			XmlElementWriter(styleElement).Element(L"s").Text(			sp->strikeline.Value()?L"true":L"false"	);
					if(sp->antialias && sp->verticalAntialias)
					{
						bool h=sp->antialias;
						bool v=sp->verticalAntialias;
						if(!h)
						{
							XmlElementWriter(styleElement).Element(L"antialias").Text(L"no");
						}
						else if(!v)
						{
							XmlElementWriter(styleElement).Element(L"antialias").Text(L"horizontal");
						}
						else
						{
							XmlElementWriter(styleElement).Element(L"antialias").Text(L"vertical");
						}
					}
				}
			}
			return xml;
		}
	}
}