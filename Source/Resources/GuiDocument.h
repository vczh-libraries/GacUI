/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Resource

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_RESOURCES_GUIDOCUMENT
#define VCZH_PRESENTATION_RESOURCES_GUIDOCUMENT

#include "GuiResource.h"

namespace vl
{
	namespace presentation
	{
		class DocumentTextRun;
		class DocumentStylePropertiesRun;
		class DocumentStyleApplicationRun;
		class DocumentHyperlinkRun;
		class DocumentImageRun;
		class DocumentEmbeddedObjectRun;
		class DocumentParagraphRun;

/***********************************************************************
Rich Content Document (style)
***********************************************************************/

		struct DocumentFontSize
		{
			double							size = 0;
			bool							relative = false;

			DocumentFontSize()
			{
			}

			DocumentFontSize(double _size, bool _relative)
				:size(_size)
				, relative(_relative)
			{
			}

			static DocumentFontSize			Parse(const WString& value);
			WString							ToString()const;

			bool operator==(const DocumentFontSize& value)const
			{
				return size == value.size && relative == value.relative;
			}

			bool operator!=(const DocumentFontSize& value)const
			{
				return size != value.size || relative != value.relative;
			}
		};

		/// <summary>Represents a text style.</summary>
		class DocumentStyleProperties : public Object, public Description<DocumentStyleProperties>
		{
		public:
			/// <summary>Font face.</summary>
			Nullable<WString>				face;
			/// <summary>Font size.</summary>
			Nullable<DocumentFontSize>		size;
			/// <summary>Font color.</summary>
			Nullable<Color>					color;
			/// <summary>Font color.</summary>
			Nullable<Color>					backgroundColor;
			/// <summary>Bold.</summary>
			Nullable<bool>					bold;
			/// <summary>Italic.</summary>
			Nullable<bool>					italic;
			/// <summary>Underline.</summary>
			Nullable<bool>					underline;
			/// <summary>Strikeline.</summary>
			Nullable<bool>					strikeline;
			/// <summary>Antialias.</summary>
			Nullable<bool>					antialias;
			/// <summary>Vertical antialias.</summary>
			Nullable<bool>					verticalAntialias;
		};

/***********************************************************************
Rich Content Document (run)
***********************************************************************/

		/// <summary>Pepresents a logical run of a rich content document.</summary>
		class DocumentRun : public Object, public Description<DocumentRun>
		{
		public:
			/// <summary>A visitor interface for <see cref="DocumentRun"/>.</summary>
			class IVisitor : public Interface
			{
			public:
				/// <summary>Visit operation for <see cref="DocumentTextRun"/>.</summary>
				/// <param name="run">The run object.</param>
				virtual void				Visit(DocumentTextRun* run)=0;
				/// <summary>Visit operation for <see cref="DocumentStylePropertiesRun"/>.</summary>
				/// <param name="run">The run object.</param>
				virtual void				Visit(DocumentStylePropertiesRun* run)=0;
				/// <summary>Visit operation for <see cref="DocumentStyleApplicationRun"/>.</summary>
				/// <param name="run">The run object.</param>
				virtual void				Visit(DocumentStyleApplicationRun* run)=0;
				/// <summary>Visit operation for <see cref="DocumentHyperlinkRun"/>.</summary>
				/// <param name="run">The run object.</param>
				virtual void				Visit(DocumentHyperlinkRun* run)=0;
				/// <summary>Visit operation for <see cref="DocumentImageRun"/>.</summary>
				/// <param name="run">The run object.</param>
				virtual void				Visit(DocumentImageRun* run)=0;
				/// <summary>Visit operation for <see cref="DocumentEmbeddedObjectRun"/>.</summary>
				/// <param name="run">The run object.</param>
				virtual void				Visit(DocumentEmbeddedObjectRun* run)=0;
				/// <summary>Visit operation for <see cref="DocumentParagraphRun"/>.</summary>
				/// <param name="run">The run object.</param>
				virtual void				Visit(DocumentParagraphRun* run)=0;
			};

			DocumentRun(){}

			/// <summary>Accept a <see cref="IVisitor"/> and trigger the selected visit operation.</summary>
			/// <param name="visitor">The visitor.</param>
			virtual void					Accept(IVisitor* visitor)=0;
		};
		
		/// <summary>Pepresents a container run.</summary>
		class DocumentContainerRun : public DocumentRun, public Description<DocumentContainerRun>
		{
			typedef collections::List<Ptr<DocumentRun>>			RunList;
		public:
			/// <summary>Sub runs.</summary>
			RunList							runs;
		};
		
		/// <summary>Pepresents a content run.</summary>
		class DocumentContentRun : public DocumentRun, public Description<DocumentContentRun>
		{
		public:
			/// <summary>Get representation text.</summary>
			/// <returns>The representation text.</returns>
			virtual WString					GetRepresentationText()=0;
		};

		//-------------------------------------------------------------------------

		/// <summary>Pepresents a text run.</summary>
		class DocumentTextRun : public DocumentContentRun, public Description<DocumentTextRun>
		{
		public:
			/// <summary>Run text.</summary>
			WString							text;

			DocumentTextRun(){}

			WString							GetRepresentationText()override{return text;}
			void							Accept(IVisitor* visitor)override{visitor->Visit(this);}
		};
				
		/// <summary>Pepresents a inline object run.</summary>
		class DocumentInlineObjectRun : public DocumentContentRun, public Description<DocumentInlineObjectRun>
		{
		public:
			/// <summary>Size of the inline object.</summary>
			Size							size;
			/// <summary>Baseline of the inline object.</summary>
			vint							baseline;

			DocumentInlineObjectRun():baseline(-1){}
		};
				
		/// <summary>Pepresents a image run.</summary>
		class DocumentImageRun : public DocumentInlineObjectRun, public Description<DocumentImageRun>
		{
		public:
			static const wchar_t*			RepresentationText;

			/// <summary>The image.</summary>
			Ptr<INativeImage>				image;
			/// <summary>The frame index.</summary>
			vint							frameIndex;
			/// <summary>The image source string.</summary>
			WString							source;

			DocumentImageRun():frameIndex(0){}
			
			WString							GetRepresentationText()override{return RepresentationText;}
			void							Accept(IVisitor* visitor)override{visitor->Visit(this);}
		};
				
		/// <summary>Pepresents an embedded object run.</summary>
		class DocumentEmbeddedObjectRun : public DocumentInlineObjectRun, public Description<DocumentImageRun>
		{
		public:
			static const wchar_t*			RepresentationText;

			/// <summary>The object name.</summary>
			WString							name;
			
			WString							GetRepresentationText()override{return RepresentationText;}
			void							Accept(IVisitor* visitor)override{visitor->Visit(this);}
		};

		//-------------------------------------------------------------------------
				
		/// <summary>Pepresents a style properties run.</summary>
		class DocumentStylePropertiesRun : public DocumentContainerRun, public Description<DocumentStylePropertiesRun>
		{
		public:
			/// <summary>Style properties.</summary>
			Ptr<DocumentStyleProperties>	style;

			DocumentStylePropertiesRun(){}

			void							Accept(IVisitor* visitor)override{visitor->Visit(this);}
		};
				
		/// <summary>Pepresents a style application run.</summary>
		class DocumentStyleApplicationRun : public DocumentContainerRun, public Description<DocumentStyleApplicationRun>
		{
		public:
			/// <summary>Style name.</summary>
			WString							styleName;

			DocumentStyleApplicationRun(){}

			void							Accept(IVisitor* visitor)override{visitor->Visit(this);}
		};
		
		/// <summary>Pepresents a hyperlink text run.</summary>
		class DocumentHyperlinkRun : public DocumentStyleApplicationRun, public Description<DocumentHyperlinkRun>
		{
		public:
			class Package : public Object, public Description<Package>
			{
			public:
				collections::List<Ptr<DocumentHyperlinkRun>>		hyperlinks;
				vint												row = -1;
				vint												start = -1;
				vint												end = -1;
			};

			/// <summary>Style name for normal state.</summary>
			WString							normalStyleName;
			/// <summary>Style name for active state.</summary>
			WString							activeStyleName;
			/// <summary>The reference of the hyperlink.</summary>
			WString							reference;

			DocumentHyperlinkRun(){}

			void							Accept(IVisitor* visitor)override{visitor->Visit(this);}
		};
				
		/// <summary>Pepresents a paragraph run.</summary>
		class DocumentParagraphRun : public DocumentContainerRun, public Description<DocumentParagraphRun>
		{
		public:
			/// <summary>Paragraph alignment.</summary>
			Nullable<Alignment>				alignment;

			DocumentParagraphRun(){}

			void							Accept(IVisitor* visitor)override{visitor->Visit(this);}

			WString							GetText(bool skipNonTextContent);
			void							GetText(stream::TextWriter& writer, bool skipNonTextContent);
		};

/***********************************************************************
Rich Content Document (model)
***********************************************************************/

		/// <summary>Represents a text style.</summary>
		class DocumentStyle : public Object, public Description<DocumentStyle>
		{
		public:
			/// <summary>Parent style name, could be #Default, #Context, #NormalLink, #ActiveLink or style name of others</summary>
			WString							parentStyleName;

			/// <summary>Properties of this style.</summary>
			Ptr<DocumentStyleProperties>	styles;

			/// <summary>Resolved properties of this style using parent styles.</summary>
			Ptr<DocumentStyleProperties>	resolvedStyles;
		};

		/// <summary>Represents a rich content document model.</summary>
		class DocumentModel : public Object, public Description<DocumentModel>
		{
		public:
			static const wchar_t*			DefaultStyleName;
			static const wchar_t*			SelectionStyleName;
			static const wchar_t*			ContextStyleName;
			static const wchar_t*			NormalLinkStyleName;
			static const wchar_t*			ActiveLinkStyleName;
		public:
			/// <summary>Represents a resolved style.</summary>
			struct ResolvedStyle
			{
				/// <summary>The style of the text.</summary>
				FontProperties				style;
				/// <summary>The color of the text.</summary>
				Color						color;
				/// <summary>The background color of the text.</summary>
				Color						backgroundColor;

				ResolvedStyle()
				{
				}

				ResolvedStyle(const FontProperties& _style, Color _color, Color _backgroundColor)
					:style(_style)
					,color(_color)
					,backgroundColor(_backgroundColor)
				{
				}
			};

			struct RunRange
			{
				vint			start;
				vint			end;
			};

			typedef collections::Dictionary<DocumentRun*, RunRange>						RunRangeMap;
		private:
			typedef collections::List<Ptr<DocumentParagraphRun>>						ParagraphList;
			typedef collections::Dictionary<WString, Ptr<DocumentStyle>>				StyleMap;
		public:
			/// <summary>All paragraphs.</summary>
			ParagraphList							paragraphs;
			/// <summary>All available styles. These will not be persistant.</summary>
			StyleMap								styles;
			
			DocumentModel();

			static void								MergeStyle(Ptr<DocumentStyleProperties> style, Ptr<DocumentStyleProperties> parent);
			void									MergeBaselineStyle(Ptr<DocumentStyleProperties> style, const WString& styleName);
			void									MergeBaselineStyle(Ptr<DocumentModel> baselineDocument, const WString& styleName);
			void									MergeBaselineStyles(Ptr<DocumentModel> baselineDocument);
			void									MergeDefaultFont(const FontProperties& defaultFont);
			ResolvedStyle							GetStyle(Ptr<DocumentStyleProperties> sp, const ResolvedStyle& context);
			ResolvedStyle							GetStyle(const WString& styleName, const ResolvedStyle& context);

			WString									GetText(bool skipNonTextContent);
			void									GetText(stream::TextWriter& writer, bool skipNonTextContent);
			
			bool									CheckEditRange(TextPos begin, TextPos end, RunRangeMap& relatedRanges);
			Ptr<DocumentModel>						CopyDocument(TextPos begin, TextPos end, bool deepCopy);
			Ptr<DocumentModel>						CopyDocument();
			bool									CutParagraph(TextPos position);
			bool									CutEditRange(TextPos begin, TextPos end);
			bool									EditContainer(TextPos begin, TextPos end, const Func<void(DocumentParagraphRun*, RunRangeMap&, vint, vint)>& editor);
			
			vint									EditRun(TextPos begin, TextPos end, Ptr<DocumentModel> replaceToModel, bool copy);
			vint									EditRunNoCopy(TextPos begin, TextPos end, const collections::Array<Ptr<DocumentParagraphRun>>& runs);
			vint									EditText(TextPos begin, TextPos end, bool frontSide, const collections::Array<WString>& text);
			bool									EditStyle(TextPos begin, TextPos end, Ptr<DocumentStyleProperties> style);
			Ptr<DocumentImageRun>					EditImage(TextPos begin, TextPos end, Ptr<GuiImageData> image);
			bool									EditHyperlink(vint paragraphIndex, vint begin, vint end, const WString& reference, const WString& normalStyleName=NormalLinkStyleName, const WString& activeStyleName=ActiveLinkStyleName);
			bool									RemoveHyperlink(vint paragraphIndex, vint begin, vint end);
			Ptr<DocumentHyperlinkRun::Package>		GetHyperlink(vint paragraphIndex, vint begin, vint end);
			bool									EditStyleName(TextPos begin, TextPos end, const WString& styleName);
			bool									RemoveStyleName(TextPos begin, TextPos end);
			bool									RenameStyle(const WString& oldStyleName, const WString& newStyleName);
			bool									ClearStyle(TextPos begin, TextPos end);
			Ptr<DocumentStyleProperties>			SummarizeStyle(TextPos begin, TextPos end);
			Nullable<WString>						SummarizeStyleName(TextPos begin, TextPos end);
			Nullable<Alignment>						SummarizeParagraphAlignment(TextPos begin, TextPos end);

			/// <summary>Load a document model from an xml.</summary>
			/// <returns>The loaded document model.</returns>
			/// <param name="resource">The resource item containing the resource.</param>
			/// <param name="xml">The xml document.</param>
			/// <param name="resolver">A document resolver to resolve symbols in non-embedded objects like image.</param>
			/// <param name="errors">All collected errors during loading a resource.</param>
			static Ptr<DocumentModel>		LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, Ptr<GuiResourcePathResolver> resolver, GuiResourceError::List& errors);

			/// <summary>Save a document model to an xml.</summary>
			/// <returns>The saved xml document.</returns>
			Ptr<parsing::xml::XmlDocument>	SaveToXml();
		};
	}
}

#endif