#include "GuiDocument.h"
#include "GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace glr::xml;
		using namespace regex;

/***********************************************************************
document_operation_visitors::DeserializeNodeVisitor
***********************************************************************/

		namespace document_operation_visitors
		{
			class DeserializeNodeVisitor : public XmlNode::IVisitor
			{
			public:
				Ptr<DocumentModel>					model;
				Ptr<DocumentContainerRun>			container;
				vint								paragraphIndex;
				Ptr<GuiResourceItem>				resource;
				Ptr<GuiResourcePathResolver>		resolver;
				Regex								regexAttributeApply;
				GuiResourceError::List&				errors;

				DeserializeNodeVisitor(Ptr<DocumentModel> _model, Ptr<DocumentParagraphRun> _paragraph, vint _paragraphIndex, Ptr<GuiResourceItem> _resource, Ptr<GuiResourcePathResolver> _resolver, GuiResourceError::List& _errors)
					:model(_model)
					, container(_paragraph)
					, paragraphIndex(_paragraphIndex)
					, resource(_resource)
					, resolver(_resolver)
					, regexAttributeApply(L"/{@(<value>[^{}]+)/}")
					, errors(_errors)
				{
				}

				void PrintText(const WString& text)
				{
					auto run = Ptr(new DocumentTextRun);
					run->text = text;
					container->runs.Add(run);
				}

				void Visit(XmlText* node)override
				{
					PrintText(node->content.value);
				}

				void Visit(XmlCData* node)override
				{
					PrintText(node->content.value);
				}

				void Visit(XmlComment* node)override
				{
				}

				void Visit(XmlElement* node)override
				{
					Ptr<DocumentContainerRun> createdContainer;
					bool useTemplateInfo = false;
					XmlElement* subNodeContainer = node;

					if (node->name.value == L"br")
					{
						PrintText(L"\r\n");
					}
					else if (node->name.value == L"sp")
					{
						PrintText(L" ");
					}
					else if (node->name.value == L"tab")
					{
						PrintText(L"\t");
					}
					else if (node->name.value == L"img")
					{
						auto run = Ptr(new DocumentImageRun);
						run->baseline = -1;

						if (Ptr<XmlAttribute> source = XmlGetAttribute(node, L"source"))
						{
							run->source = source->value.value;
							WString protocol, path;
							if (IsResourceUrl(run->source, protocol, path))
							{
								Ptr<GuiImageData> imageData = resolver->ResolveResource(protocol, path).Cast<GuiImageData>();
								if (imageData)
								{
									run->image = imageData->GetImage();
								}
								if (run->image && run->image->GetFrameCount() > 0)
								{
									run->size = run->image->GetFrame(0)->GetSize();
									run->frameIndex = 0;
								}
							}

							for (auto att : node->attributes)
							{
								if (att->name.value == L"width")
								{
									run->size.x = wtoi(att->value.value);
								}
								else if (att->name.value == L"height")
								{
									run->size.y = wtoi(att->value.value);
								}
								else if (att->name.value == L"baseline")
								{
									run->baseline = wtoi(att->value.value);
								}
								else if (att->name.value == L"frameIndex")
								{
									run->frameIndex = wtoi(att->value.value);
								}
								else if (att->name.value != L"source")
								{
									errors.Add(GuiResourceError({ {resource},att->name.codeRange.start }, L"Unknown attribute in <img>: \"" + att->name.value + L"\"."));
								}
							}

							container->runs.Add(run);
						}
						else
						{
							errors.Add(GuiResourceError({ {resource},node->codeRange.start }, L"Attribute \"source\" is missing in <img>."));
						}
					}
					else if (node->name.value == L"object")
					{
						auto run = Ptr(new DocumentEmbeddedObjectRun);
						run->baseline = -1;

						if (auto name = XmlGetAttribute(node, L"name"))
						{
							run->name = name->value.value;
							container->runs.Add(run);
						}
						else
						{
							errors.Add(GuiResourceError({ {resource},node->codeRange.start }, L"The \"name\" attribute in <object> is missing."));
						}
					}
					else if (node->name.value == L"font")
					{
						auto run = Ptr(new DocumentStylePropertiesRun);
						auto sp = Ptr(new DocumentStyleProperties);
						run->style = sp;

						for (auto att : node->attributes)
						{
							if (att->name.value == L"face")
							{
								sp->face = att->value.value;
							}
							else if (att->name.value == L"size")
							{
								sp->size = DocumentFontSize::Parse(att->value.value);
							}
							else if (att->name.value == L"color")
							{
								sp->color = Color::Parse(att->value.value);
							}
							else if (att->name.value == L"bkcolor")
							{
								sp->backgroundColor = Color::Parse(att->value.value);
							}
							else
							{
								errors.Add(GuiResourceError({ {resource},att->name.codeRange.start }, L"Unknown attribute in <font>: \"" + att->name.value + L"\"."));
							}
						}
						container->runs.Add(run);
						createdContainer = run;
					}
					else if (node->name.value == L"b" || node->name.value == L"b-")
					{
						auto run = Ptr(new DocumentStylePropertiesRun);
						run->style = Ptr(new DocumentStyleProperties);
						run->style->bold = node->name.value == L"b";
						container->runs.Add(run);
						createdContainer = run;
					}
					else if (node->name.value == L"i" || node->name.value == L"i-")
					{
						auto run = Ptr(new DocumentStylePropertiesRun);
						run->style = Ptr(new DocumentStyleProperties);
						run->style->italic = node->name.value == L"i";
						container->runs.Add(run);
						createdContainer = run;
					}
					else if (node->name.value == L"u" || node->name.value == L"u-")
					{
						auto run = Ptr(new DocumentStylePropertiesRun);
						run->style = Ptr(new DocumentStyleProperties);
						run->style->underline = node->name.value == L"u";
						container->runs.Add(run);
						createdContainer = run;
					}
					else if (node->name.value == L"s" || node->name.value == L"s-")
					{
						auto run = Ptr(new DocumentStylePropertiesRun);
						run->style = Ptr(new DocumentStyleProperties);
						run->style->strikeline = node->name.value == L"s";
						container->runs.Add(run);
						createdContainer = run;
					}
					else if (node->name.value == L"ha")
					{
						auto run = Ptr(new DocumentStylePropertiesRun);
						run->style = Ptr(new DocumentStyleProperties);
						run->style->antialias = true;
						run->style->verticalAntialias = false;
						container->runs.Add(run);
						createdContainer = run;
					}
					else if (node->name.value == L"va")
					{
						auto run = Ptr(new DocumentStylePropertiesRun);
						run->style = Ptr(new DocumentStyleProperties);
						run->style->antialias = true;
						run->style->verticalAntialias = true;
						container->runs.Add(run);
						createdContainer = run;
					}
					else if (node->name.value == L"na")
					{
						auto run = Ptr(new DocumentStylePropertiesRun);
						run->style = Ptr(new DocumentStyleProperties);
						run->style->antialias = false;
						run->style->verticalAntialias = false;
						container->runs.Add(run);
						createdContainer = run;
					}
					else if (node->name.value == L"div")
					{
						if (Ptr<XmlAttribute> att = XmlGetAttribute(node, L"style"))
						{
							WString styleName = att->value.value;

							auto run = Ptr(new DocumentStyleApplicationRun);
							run->styleName = styleName;
							container->runs.Add(run);
							createdContainer = run;
						}
						else
						{
							createdContainer = container;
						}
					}
					else if (node->name.value == L"a")
					{
						auto run = Ptr(new DocumentHyperlinkRun);
						run->normalStyleName = L"#NormalLink";
						run->activeStyleName = L"#ActiveLink";
						if (Ptr<XmlAttribute> att = XmlGetAttribute(node, L"normal"))
						{
							run->normalStyleName = att->value.value;
						}
						if (Ptr<XmlAttribute> att = XmlGetAttribute(node, L"active"))
						{
							run->activeStyleName = att->value.value;
						}
						if (Ptr<XmlAttribute> att = XmlGetAttribute(node, L"href"))
						{
							run->reference = att->value.value;
						}
						run->styleName = run->normalStyleName;
						container->runs.Add(run);
						createdContainer = run;
					}
					else if (node->name.value == L"p")
					{
						for (auto sub : node->subNodes)
						{
							sub->Accept(this);
						}
					}
					else
					{
						if (node->name.value != L"nop")
						{
							errors.Add(GuiResourceError({ {resource},node->codeRange.start }, L"Unknown element in <p>: \"" + node->name.value + L"\"."));
						}
						for (auto sub : node->subNodes)
						{
							sub->Accept(this);
						}
					}

					if (createdContainer)
					{
						Ptr<DocumentContainerRun> oldContainer = container;
						container = createdContainer;
						for (auto subNode : subNodeContainer->subNodes)
						{
							subNode->Accept(this);
						}
						container = oldContainer;
					}
				}

				void Visit(XmlInstruction* node)override
				{
				}

				void Visit(XmlDocument* node)override
				{
				}
			};

			Ptr<DocumentStyle> ParseDocumentStyle(Ptr<GuiResourceItem> resource, Ptr<XmlElement> styleElement, GuiResourceError::List& errors)
			{
				auto style = Ptr(new DocumentStyle);

				if(Ptr<XmlAttribute> parent=XmlGetAttribute(styleElement, L"parent"))
				{
					style->parentStyleName=parent->value.value;
				}

				auto sp = Ptr(new DocumentStyleProperties);
				style->styles=sp;

				for (auto att : XmlGetElements(styleElement))
				{
					if(att->name.value==L"face")
					{
						sp->face=XmlGetValue(att);
					}
					else if(att->name.value==L"size")
					{
						sp->size=DocumentFontSize::Parse(XmlGetValue(att));
					}
					else if(att->name.value==L"color")
					{
						sp->color=Color::Parse(XmlGetValue(att));
					}
					else if(att->name.value==L"bkcolor")
					{
						sp->backgroundColor=Color::Parse(XmlGetValue(att));
					}
					else if(att->name.value==L"b")
					{
						sp->bold=XmlGetValue(att)==L"true";
					}
					else if(att->name.value==L"i")
					{
						sp->italic=XmlGetValue(att)==L"true";
					}
					else if(att->name.value==L"u")
					{
						sp->underline=XmlGetValue(att)==L"true";
					}
					else if(att->name.value==L"s")
					{
						sp->strikeline=XmlGetValue(att)==L"true";
					}
					else if(att->name.value==L"antialias")
					{
						WString value=XmlGetValue(att);
						if(value==L"horizontal" || value==L"default")
						{
							sp->antialias=true;
							sp->verticalAntialias=false;
						}
						else if(value==L"no")
						{
							sp->antialias=false;
							sp->verticalAntialias=false;
						}
						else if(value==L"vertical")
						{
							sp->antialias=true;
							sp->verticalAntialias=true;
						}
					}
					else
					{
						errors.Add(GuiResourceError({ {resource},att->codeRange.start }, L"Unknown element in <Style>: \"" + att->name.value + L"\"."));
					}
				}

				return style;
			}
		}
		using namespace document_operation_visitors;

/***********************************************************************
DocumentModel
***********************************************************************/

		Ptr<DocumentModel> DocumentModel::LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlDocument> xml, Ptr<GuiResourcePathResolver> resolver, GuiResourceError::List& errors)
		{
			auto model = Ptr(new DocumentModel);
			if (xml->rootElement->name.value == L"Doc")
			{
				for (auto partElement : XmlGetElements(xml->rootElement))
				{
					if (partElement->name.value == L"Styles")
					{
						for (auto styleElement : XmlGetElements(partElement))
						{
							if (styleElement->name.value == L"Style")
							{
								if (Ptr<XmlAttribute> name = XmlGetAttribute(styleElement, L"name"))
								{
									auto style = ParseDocumentStyle(resource, styleElement, errors);
									auto styleName = name->value.value;
									if (!model->styles.Keys().Contains(styleName))
									{
										model->styles.Add(styleName, style);
										if (styleName.Length() > 9 && styleName.Right(9) == L"-Override")
										{
											auto overridedStyle = Ptr(new DocumentStyle);
											overridedStyle->styles = Ptr(new DocumentStyleProperties);
											MergeStyle(overridedStyle->styles, style->styles);

											styleName = styleName.Left(styleName.Length() - 9);
											auto index = model->styles.Keys().IndexOf(styleName);
											if (index == -1)
											{
												model->styles.Add(styleName, overridedStyle);
											}
											else
											{
												auto originalStyle = model->styles.Values()[index];
												MergeStyle(overridedStyle->styles, originalStyle->styles);
												originalStyle->styles = overridedStyle->styles;
											}
										}
									}
								}
								else
								{
									errors.Add(GuiResourceError({ {resource},styleElement->codeRange.start }, L"Attribute \"name\" is missing in <Style>."));
								}
							}
							else
							{
								errors.Add(GuiResourceError({ {resource},styleElement->codeRange.start }, L"Unknown element in <Styles>: \"" + styleElement->name.value + L"\"."));
							}
						}
					}
					else if (partElement->name.value == L"Content")
					{
						for (auto [p, i] : indexed(XmlGetElements(partElement)))
						{
							if (p->name.value == L"p")
							{
								auto paragraph = Ptr(new DocumentParagraphRun);
								if (Ptr<XmlAttribute> att = XmlGetAttribute(p, L"align"))
								{
									if (att->value.value == L"Left")
									{
										paragraph->alignment = Alignment::Left;
									}
									else if (att->value.value == L"Center")
									{
										paragraph->alignment = Alignment::Center;
									}
									else if (att->value.value == L"Right")
									{
										paragraph->alignment = Alignment::Right;
									}
									else
									{
										errors.Add(GuiResourceError({ {resource},att->value.codeRange.start }, L"Unknown value in align attribute \"" + att->value.value + L"\"."));
									}
								}
								model->paragraphs.Add(paragraph);
								DeserializeNodeVisitor visitor(model, paragraph, i, resource, resolver, errors);
								p->Accept(&visitor);
							}
							else
							{
								errors.Add(GuiResourceError({ {resource},p->codeRange.start }, L"Unknown element in <Content>: \"" + p->name.value + L"\"."));
							}
						}
					}
					else
					{
						errors.Add(GuiResourceError({ {resource},partElement->codeRange.start }, L"Unknown element in <Doc>: \"" + partElement->name.value + L"\"."));
					}
				}
			}
			else
			{
				errors.Add(GuiResourceError({ {resource},xml->rootElement->codeRange.start }, L"The root element of document should be \"Doc\"."));
			}
			return model;
		}
	}
}