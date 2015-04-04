#include "GuiDocument.h"
#include "GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace parsing::tabling;
		using namespace parsing::xml;
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
				Ptr<GuiResourcePathResolver>		resolver;
				Regex								regexAttributeApply;
				List<WString>&						errors;

				DeserializeNodeVisitor(Ptr<DocumentModel> _model, Ptr<DocumentParagraphRun> _paragraph, vint _paragraphIndex, Ptr<GuiResourcePathResolver> _resolver, List<WString>& _errors)
					:model(_model)
					, container(_paragraph)
					, paragraphIndex(_paragraphIndex)
					, resolver(_resolver)
					, regexAttributeApply(L"/{@(<value>[^{}]+)/}")
					, errors(_errors)
				{
				}

				void PrintText(const WString& text)
				{
					Ptr<DocumentTextRun> run=new DocumentTextRun;
					run->text=text;
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

				void Visit(XmlAttribute* node)override
				{
				}

				void Visit(XmlComment* node)override
				{
				}

				void Visit(XmlElement* node)override
				{
					Ptr<DocumentContainerRun> createdContainer;
					bool useTemplateInfo=false;
					XmlElement* subNodeContainer=node;

					if(node->name.value==L"br")
					{
						PrintText(L"\r\n");
					}
					else if(node->name.value==L"sp")
					{
						PrintText(L" ");
					}
					else if(node->name.value==L"tab")
					{
						PrintText(L"\t");
					}
					else if(node->name.value==L"img")
					{
						Ptr<DocumentImageRun> run=new DocumentImageRun;
						if(Ptr<XmlAttribute> source=XmlGetAttribute(node, L"source"))
						{
							run->source=source->value.value;
							WString protocol, path;
							if(IsResourceUrl(run->source, protocol, path))
							{
								Ptr<GuiImageData> imageData=resolver->ResolveResource(protocol, path).Cast<GuiImageData>();
								if(imageData)
								{
									run->image=imageData->GetImage();
								}
								if(run->image && run->image->GetFrameCount()>0)
								{
									run->size=run->image->GetFrame(0)->GetSize();
									run->baseline=run->size.y;
									run->frameIndex=0;
								}
							}

							FOREACH(Ptr<XmlAttribute>, att, node->attributes)
							{
								if(att->name.value==L"width")
								{
									run->size.x=wtoi(att->value.value);
								}
								else if(att->name.value==L"height")
								{
									run->size.y=wtoi(att->value.value);
								}
								else if(att->name.value==L"baseline")
								{
									run->baseline=wtoi(att->value.value);
								}
								else if(att->name.value==L"frameIndex")
								{
									run->frameIndex=wtoi(att->value.value);
								}
								else
								{
									errors.Add(L"Unknown attribute in <img> \"" + att->name.value + L"\".");
								}
							}
							container->runs.Add(run);
						}
					}
					else if(node->name.value==L"font")
					{
						Ptr<DocumentStylePropertiesRun> run=new DocumentStylePropertiesRun();
						Ptr<DocumentStyleProperties> sp=new DocumentStyleProperties;
						run->style=sp;

						FOREACH(Ptr<XmlAttribute>, att, node->attributes)
						{
							if(att->name.value==L"face")
							{
								sp->face=att->value.value;
							}
							else if(att->name.value==L"size")
							{
								sp->size=wtoi(att->value.value);
							}
							else if(att->name.value==L"color")
							{
								sp->color=Color::Parse(att->value.value);
							}
							else if(att->name.value==L"bkcolor")
							{
								sp->backgroundColor=Color::Parse(att->value.value);
							}
							else
							{
								errors.Add(L"Unknown attribute in <font> \"" + att->name.value + L"\".");
							}
						}
						container->runs.Add(run);
						createdContainer=run;
					}
					else if(node->name.value==L"b" || node->name.value==L"b-")
					{
						Ptr<DocumentStylePropertiesRun> run=new DocumentStylePropertiesRun();
						run->style=new DocumentStyleProperties;
						run->style->bold=node->name.value==L"b";
						container->runs.Add(run);
						createdContainer=run;
					}
					else if(node->name.value==L"i" || node->name.value==L"i-")
					{
						Ptr<DocumentStylePropertiesRun> run=new DocumentStylePropertiesRun();
						run->style=new DocumentStyleProperties;
						run->style->italic=node->name.value==L"i";
						container->runs.Add(run);
						createdContainer=run;
					}
					else if(node->name.value==L"u" || node->name.value==L"u-")
					{
						Ptr<DocumentStylePropertiesRun> run=new DocumentStylePropertiesRun();
						run->style=new DocumentStyleProperties;
						run->style->underline=node->name.value==L"u";
						container->runs.Add(run);
						createdContainer=run;
					}
					else if(node->name.value==L"s" || node->name.value==L"s-")
					{
						Ptr<DocumentStylePropertiesRun> run=new DocumentStylePropertiesRun();
						run->style=new DocumentStyleProperties;
						run->style->strikeline=node->name.value==L"s";
						container->runs.Add(run);
						createdContainer=run;
					}
					else if(node->name.value==L"ha")
					{
						Ptr<DocumentStylePropertiesRun> run=new DocumentStylePropertiesRun();
						run->style=new DocumentStyleProperties;
						run->style->antialias=true;
						run->style->verticalAntialias=false;
						container->runs.Add(run);
						createdContainer=run;
					}
					else if(node->name.value==L"va")
					{
						Ptr<DocumentStylePropertiesRun> run=new DocumentStylePropertiesRun();
						run->style=new DocumentStyleProperties;
						run->style->antialias=true;
						run->style->verticalAntialias=true;
						container->runs.Add(run);
						createdContainer=run;
					}
					else if(node->name.value==L"na")
					{
						Ptr<DocumentStylePropertiesRun> run=new DocumentStylePropertiesRun();
						run->style=new DocumentStyleProperties;
						run->style->antialias=false;
						run->style->verticalAntialias=false;
						container->runs.Add(run);
						createdContainer=run;
					}
					else if(node->name.value==L"div")
					{
						if(Ptr<XmlAttribute> att=XmlGetAttribute(node, L"style"))
						{
							WString styleName=att->value.value;
							
							Ptr<DocumentStyleApplicationRun> run=new DocumentStyleApplicationRun;
							run->styleName=styleName;
							container->runs.Add(run);
							createdContainer=run;
						}
						else
						{
							createdContainer=container;
						}
					}
					else if(node->name.value==L"a")
					{
						Ptr<DocumentHyperlinkRun> run=new DocumentHyperlinkRun;
						run->normalStyleName=L"#NormalLink";
						run->activeStyleName=L"#ActiveLink";
						if(Ptr<XmlAttribute> att=XmlGetAttribute(node, L"normal"))
						{
							run->normalStyleName=att->value.value;
						}
						if(Ptr<XmlAttribute> att=XmlGetAttribute(node, L"active"))
						{
							run->activeStyleName=att->value.value;
						}
						if(Ptr<XmlAttribute> att=XmlGetAttribute(node, L"href"))
						{
							run->reference=att->value.value;
						}
						run->styleName=run->normalStyleName;
						container->runs.Add(run);
						createdContainer=run;
					}
					else if(node->name.value==L"p")
					{
						FOREACH(Ptr<XmlNode>, sub, node->subNodes)
						{
							sub->Accept(this);
						}
					}
					else
					{
						if (node->name.value != L"nop")
						{
							errors.Add(L"Unknown tag in document resource \"" + node->name.value + L"\".");
						}
						FOREACH(Ptr<XmlNode>, sub, node->subNodes)
						{
							sub->Accept(this);
						}
					}

					if(createdContainer)
					{
						Ptr<DocumentContainerRun> oldContainer=container;
						container=createdContainer;
						FOREACH(Ptr<XmlNode>, subNode, subNodeContainer->subNodes)
						{
							subNode->Accept(this);
						}
						container=oldContainer;
					}
				}

				void Visit(XmlInstruction* node)override
				{
				}

				void Visit(XmlDocument* node)override
				{
				}
			};

			Ptr<DocumentStyle> ParseDocumentStyle(Ptr<XmlElement> styleElement)
			{
				Ptr<DocumentStyle> style=new DocumentStyle;

				if(Ptr<XmlAttribute> parent=XmlGetAttribute(styleElement, L"parent"))
				{
					style->parentStyleName=parent->value.value;
				}

				Ptr<DocumentStyleProperties> sp=new DocumentStyleProperties;
				style->styles=sp;

				FOREACH(Ptr<XmlElement>, att, XmlGetElements(styleElement))
				{
					if(att->name.value==L"face")
					{
						sp->face=XmlGetValue(att);
					}
					else if(att->name.value==L"size")
					{
						sp->size=wtoi(XmlGetValue(att));
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
				}

				return style;
			}
		}
		using namespace document_operation_visitors;

/***********************************************************************
DocumentModel
***********************************************************************/

		Ptr<DocumentModel> DocumentModel::LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)
		{
			Ptr<DocumentModel> model=new DocumentModel;
			if(xml->rootElement->name.value==L"Doc")
			{
				if(Ptr<XmlElement> styles=XmlGetElement(xml->rootElement, L"Styles"))
				{
					FOREACH(Ptr<XmlElement>, styleElement, XmlGetElements(styles, L"Style"))
					if (Ptr<XmlAttribute> name = XmlGetAttribute(styleElement, L"name"))
					{
						auto style = ParseDocumentStyle(styleElement);
						auto styleName = name->value.value;
						if(!model->styles.Keys().Contains(styleName))
						{
							model->styles.Add(styleName, style);
							if (styleName.Length() > 9 && styleName.Right(9) == L"-Override")
							{
								auto overridedStyle = MakePtr<DocumentStyle>();
								overridedStyle->styles = new DocumentStyleProperties;
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
				}
				if(Ptr<XmlElement> content=XmlGetElement(xml->rootElement, L"Content"))
				{
					FOREACH_INDEXER(Ptr<XmlElement>, p, i, XmlGetElements(content, L"p"))
					{
						Ptr<DocumentParagraphRun> paragraph=new DocumentParagraphRun;
						if(Ptr<XmlAttribute> att=XmlGetAttribute(p, L"align"))
						{
							if(att->value.value==L"Left")
							{
								paragraph->alignment=Alignment::Left;
							}
							else if(att->value.value==L"Center")
							{
								paragraph->alignment=Alignment::Center;
							}
							else if(att->value.value==L"Right")
							{
								paragraph->alignment=Alignment::Right;
							}
						}
						model->paragraphs.Add(paragraph);
						DeserializeNodeVisitor visitor(model, paragraph, i, resolver, errors);
						p->Accept(&visitor);
					}
				}
			}
			return model;
		}

		Ptr<DocumentModel> DocumentModel::LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, const WString& workingDirectory, collections::List<WString>& errors)
		{
			Ptr<GuiResourcePathResolver> resolver=new GuiResourcePathResolver(0, workingDirectory);
			return LoadFromXml(xml, resolver, errors);
		}

		Ptr<DocumentModel> DocumentModel::LoadFromXml(const WString& filePath, collections::List<WString>& errors)
		{
			Ptr<XmlDocument> xml;
			Ptr<DocumentModel> document;
			if(auto parser=GetParserManager()->GetParser<XmlDocument>(L"XML"))
			{
				WString text;
				if(LoadTextFile(filePath, text))
				{
					xml = parser->TypedParse(text, errors);
				}
				else
				{
					errors.Add(L"Failed to load file \"" + filePath + L"\".");
				}
			}
			if(xml)
			{
				document = DocumentModel::LoadFromXml(xml, GetFolderPath(filePath), errors);
			}
			return document;
		}
	}
}