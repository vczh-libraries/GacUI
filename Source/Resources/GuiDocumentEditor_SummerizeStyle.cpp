#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

		namespace document_operation_visitors
		{

/***********************************************************************
Calculate if all text in the specified range has some common styles
***********************************************************************/

			class SummarizeStyleVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				RunRangeMap&							runRanges;
				DocumentModel*							model;
				vint									start;
				vint									end;
				Ptr<DocumentStyleProperties>			style;
				List<DocumentModel::ResolvedStyle>		resolvedStyles;

				SummarizeStyleVisitor(RunRangeMap& _runRanges, DocumentModel* _model, vint _start, vint _end)
					:runRanges(_runRanges)
					, model(_model)
					, start(_start)
					, end(_end)
				{
					DocumentModel::ResolvedStyle resolvedStyle;
					resolvedStyle = model->GetStyle(DocumentModel::DefaultStyleName, resolvedStyle);
					resolvedStyles.Add(resolvedStyle);
				}

				const DocumentModel::ResolvedStyle& GetCurrentResolvedStyle()
				{
					return resolvedStyles[resolvedStyles.Count() - 1];
				}

				// ---------------------------------------------------------

				template<typename T>
				void SetStyleItem(Nullable<T> DocumentStyleProperties::* dstField, T FontProperties::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					if (style.Obj()->*dstField && (style.Obj()->*dstField).Value() != src.style.*srcField)
					{
						style.Obj()->*dstField = Nullable<T>();
					}
				}

				template<typename T>
				void SetStyleItem(Nullable<T> DocumentStyleProperties::* dstField, T DocumentModel::ResolvedStyle::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					if (style.Obj()->*dstField && (style.Obj()->*dstField).Value() != src.*srcField)
					{
						style.Obj()->*dstField = Nullable<T>();
					}
				}

				void SetStyleItem(Nullable<DocumentFontSize> DocumentStyleProperties::* dstField, vint FontProperties::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					if (style.Obj()->*dstField)
					{
						auto dfs = (style.Obj()->*dstField).Value();
						if (dfs.relative || dfs.size != src.style.*srcField)
						{
							style.Obj()->*dstField = Nullable<DocumentFontSize>();
						}
					}
				}

				// ---------------------------------------------------------

				template<typename T>
				void OverrideStyleItem(Nullable<T> DocumentStyleProperties::* dstField, T FontProperties::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					style.Obj()->*dstField = src.style.*srcField;
				}

				template<typename T>
				void OverrideStyleItem(Nullable<T> DocumentStyleProperties::* dstField, T DocumentModel::ResolvedStyle::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					style.Obj()->*dstField = src.*srcField;
				}

				void OverrideStyleItem(Nullable<DocumentFontSize> DocumentStyleProperties::* dstField, vint FontProperties::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					style.Obj()->*dstField = DocumentFontSize((double)(src.style.*srcField), false);
				}

				// ---------------------------------------------------------

				void VisitContainer(DocumentContainerRun* run)
				{
					for (vint i = run->runs.Count() - 1; i >= 0; i--)
					{
						Ptr<DocumentRun> subRun = run->runs[i];
						RunRange range = runRanges[subRun.Obj()];
						if (range.start<end && start<range.end)
						{
							subRun->Accept(this);
						}
					}
				}

				void Visit(DocumentTextRun* run)override
				{
					const DocumentModel::ResolvedStyle& currentResolvedStyle = GetCurrentResolvedStyle();
					if (style)
					{
						SetStyleItem(&DocumentStyleProperties::face, &FontProperties::fontFamily);
						SetStyleItem(&DocumentStyleProperties::size, &FontProperties::size);
						SetStyleItem(&DocumentStyleProperties::color, &DocumentModel::ResolvedStyle::color);
						SetStyleItem(&DocumentStyleProperties::backgroundColor, &DocumentModel::ResolvedStyle::backgroundColor);
						SetStyleItem(&DocumentStyleProperties::bold, &FontProperties::bold);
						SetStyleItem(&DocumentStyleProperties::italic, &FontProperties::italic);
						SetStyleItem(&DocumentStyleProperties::underline, &FontProperties::underline);
						SetStyleItem(&DocumentStyleProperties::strikeline, &FontProperties::strikeline);
						SetStyleItem(&DocumentStyleProperties::antialias, &FontProperties::antialias);
						SetStyleItem(&DocumentStyleProperties::verticalAntialias, &FontProperties::verticalAntialias);
					}
					else
					{
						style = new DocumentStyleProperties;
						OverrideStyleItem(&DocumentStyleProperties::face, &FontProperties::fontFamily);
						OverrideStyleItem(&DocumentStyleProperties::size, &FontProperties::size);
						OverrideStyleItem(&DocumentStyleProperties::color, &DocumentModel::ResolvedStyle::color);
						OverrideStyleItem(&DocumentStyleProperties::backgroundColor, &DocumentModel::ResolvedStyle::backgroundColor);
						OverrideStyleItem(&DocumentStyleProperties::bold, &FontProperties::bold);
						OverrideStyleItem(&DocumentStyleProperties::italic, &FontProperties::italic);
						OverrideStyleItem(&DocumentStyleProperties::underline, &FontProperties::underline);
						OverrideStyleItem(&DocumentStyleProperties::strikeline, &FontProperties::strikeline);
						OverrideStyleItem(&DocumentStyleProperties::antialias, &FontProperties::antialias);
						OverrideStyleItem(&DocumentStyleProperties::verticalAntialias, &FontProperties::verticalAntialias);
					}
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					const DocumentModel::ResolvedStyle& currentResolvedStyle = GetCurrentResolvedStyle();
					DocumentModel::ResolvedStyle resolvedStyle = model->GetStyle(run->style, currentResolvedStyle);
					resolvedStyles.Add(resolvedStyle);
					VisitContainer(run);
					resolvedStyles.RemoveAt(resolvedStyles.Count() - 1);
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					const DocumentModel::ResolvedStyle& currentResolvedStyle = GetCurrentResolvedStyle();
					DocumentModel::ResolvedStyle resolvedStyle = model->GetStyle(run->styleName, currentResolvedStyle);
					resolvedStyles.Add(resolvedStyle);
					VisitContainer(run);
					resolvedStyles.RemoveAt(resolvedStyles.Count() - 1);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					const DocumentModel::ResolvedStyle& currentResolvedStyle = GetCurrentResolvedStyle();
					DocumentModel::ResolvedStyle resolvedStyle = model->GetStyle(run->styleName, currentResolvedStyle);
					resolvedStyles.Add(resolvedStyle);
					VisitContainer(run);
					resolvedStyles.RemoveAt(resolvedStyles.Count() - 1);
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

/***********************************************************************
Calculate if all text in the specified range has a common style name
***********************************************************************/

			class SummarizeStyleNameVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				RunRangeMap&							runRanges;
				DocumentModel*							model;
				vint									start;
				vint									end;
				Nullable<WString>						currentStyleName;
				Nullable<WString>						styleName;
				bool									assignedStyleName = false;

				SummarizeStyleNameVisitor(RunRangeMap& _runRanges, DocumentModel* _model, vint _start, vint _end)
					:runRanges(_runRanges)
					, model(_model)
					, start(_start)
					, end(_end)
				{
				}

				void VisitContentRun(DocumentContentRun* run)
				{
					if (!assignedStyleName)
					{
						styleName = currentStyleName;
						assignedStyleName = true;
					}
					else if (styleName && (!currentStyleName || styleName.Value() != currentStyleName.Value()))
					{
						styleName = Nullable<WString>();
					}
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					for (vint i = run->runs.Count() - 1; i >= 0; i--)
					{
						Ptr<DocumentRun> subRun = run->runs[i];
						RunRange range = runRanges[subRun.Obj()];
						if (range.start<end && start<range.end)
						{
							subRun->Accept(this);
						}
					}
				}

				void Visit(DocumentTextRun* run)override
				{
					VisitContentRun(run);
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					VisitContainer(run);
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					auto oldStyleName = currentStyleName;
					currentStyleName = run->styleName;
					VisitContainer(run);
					currentStyleName = oldStyleName;
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					VisitContainer(run);
				}

				void Visit(DocumentImageRun* run)override
				{
					VisitContentRun(run);
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					VisitContentRun(run);
				}

				void Visit(DocumentParagraphRun* run)override
				{
					VisitContainer(run);
				}
			};
		}
		using namespace document_operation_visitors;

		namespace document_editor
		{
			Ptr<DocumentStyleProperties> SummarizeStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, DocumentModel* model, vint start, vint end)
			{
				SummarizeStyleVisitor visitor(runRanges, model, start, end);
				run->Accept(&visitor);
				return visitor.style;
			}

			Nullable<WString> SummarizeStyleName(DocumentParagraphRun* run, RunRangeMap& runRanges, DocumentModel* model, vint start, vint end)
			{
				SummarizeStyleNameVisitor visitor(runRanges, model, start, end);
				run->Accept(&visitor);
				return visitor.styleName;
			}

			template<typename T>
			void AggregateStyleItem(Ptr<DocumentStyleProperties>& dst, Ptr<DocumentStyleProperties> src, Nullable<T> DocumentStyleProperties::* field)
			{
				if (dst.Obj()->*field && (!(src.Obj()->*field) || (dst.Obj()->*field).Value() != (src.Obj()->*field).Value()))
				{
					dst.Obj()->*field = Nullable<T>();
				}
			}

			void AggregateStyle(Ptr<DocumentStyleProperties>& dst, Ptr<DocumentStyleProperties> src)
			{
				AggregateStyleItem(dst, src, &DocumentStyleProperties::face);
				AggregateStyleItem(dst, src, &DocumentStyleProperties::size);
				AggregateStyleItem(dst, src, &DocumentStyleProperties::color);
				AggregateStyleItem(dst, src, &DocumentStyleProperties::backgroundColor);
				AggregateStyleItem(dst, src, &DocumentStyleProperties::bold);
				AggregateStyleItem(dst, src, &DocumentStyleProperties::italic);
				AggregateStyleItem(dst, src, &DocumentStyleProperties::underline);
				AggregateStyleItem(dst, src, &DocumentStyleProperties::strikeline);
				AggregateStyleItem(dst, src, &DocumentStyleProperties::antialias);
				AggregateStyleItem(dst, src, &DocumentStyleProperties::verticalAntialias);
			}
		}
	}
}
