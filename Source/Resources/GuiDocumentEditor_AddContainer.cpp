#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace document_editor;

/***********************************************************************
Insert container runs on top of all text ranges that intersect with the specified range
AddStyleVisitor		: Apply a style on the specified range
AddHyperlinkVisitor	: Apply a hyperlink on the specified range
AddStyleNameVisitor	: Apply a style name on the specified range
***********************************************************************/

		namespace document_operation_visitors
		{
			class AddContainerVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				RunRangeMap&							runRanges;
				vint									start;
				vint									end;
				bool									insertStyle;

				virtual Ptr<DocumentContainerRun>		CreateContainer() = 0;

				AddContainerVisitor(RunRangeMap& _runRanges, vint _start, vint _end)
					:runRanges(_runRanges)
					, start(_start)
					, end(_end)
					, insertStyle(false)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					// TODO: (enumerable) foreach:indexed(alterable(reversed))
					for (vint i = run->runs.Count() - 1; i >= 0; i--)
					{
						Ptr<DocumentRun> subRun = run->runs[i];
						RunRange range = runRanges[subRun.Obj()];
						if (range.start<end && start<range.end)
						{
							insertStyle = false;
							subRun->Accept(this);
							if (insertStyle)
							{
								Ptr<DocumentContainerRun> containerRun = CreateContainer();
								run->runs.RemoveAt(i);
								containerRun->runs.Add(subRun);
								run->runs.Insert(i, containerRun);
							}
						}
					}
					insertStyle = false;
				}

				void Visit(DocumentTextRun* run)override
				{
					insertStyle = true;
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					VisitContainer(run);
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					VisitContainer(run);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					VisitContainer(run);
				}

				void Visit(DocumentImageRun* run)override
				{
					insertStyle = false;
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					insertStyle = false;
				}

				void Visit(DocumentParagraphRun* run)override
				{
					VisitContainer(run);
				}
			};

			class AddStyleVisitor : public AddContainerVisitor
			{
			public:
				Ptr<DocumentStyleProperties>	style;

				Ptr<DocumentContainerRun> CreateContainer()override
				{
					auto containerRun = Ptr(new DocumentStylePropertiesRun);
					containerRun->style = CopyStyle(style);
					return containerRun;
				}

				AddStyleVisitor(RunRangeMap& _runRanges, vint _start, vint _end, Ptr<DocumentStyleProperties> _style)
					:AddContainerVisitor(_runRanges, _start, _end)
					, style(_style)
				{
				}
			};

			class AddHyperlinkVisitor : public AddContainerVisitor
			{
			public:
				WString							reference;
				WString							normalStyleName;
				WString							activeStyleName;

				Ptr<DocumentContainerRun> CreateContainer()override
				{
					auto containerRun = Ptr(new DocumentHyperlinkRun);
					containerRun->reference = reference;
					containerRun->normalStyleName = normalStyleName;
					containerRun->activeStyleName = activeStyleName;
					containerRun->styleName = normalStyleName;
					return containerRun;
				}

				AddHyperlinkVisitor(RunRangeMap& _runRanges, vint _start, vint _end, const WString& _reference, const WString& _normalStyleName, const WString& _activeStyleName)
					:AddContainerVisitor(_runRanges, _start, _end)
					, reference(_reference)
					, normalStyleName(_normalStyleName)
					, activeStyleName(_activeStyleName)
				{
				}
			};

			class AddStyleNameVisitor : public AddContainerVisitor
			{
			public:
				WString							styleName;

				Ptr<DocumentContainerRun> CreateContainer()override
				{
					auto containerRun = Ptr(new DocumentStyleApplicationRun);
					containerRun->styleName = styleName;
					return containerRun;
				}

				AddStyleNameVisitor(RunRangeMap& _runRanges, vint _start, vint _end, const WString& _styleName)
					:AddContainerVisitor(_runRanges, _start, _end)
					, styleName(_styleName)
				{
				}
			};
		}
		using namespace document_operation_visitors;

		namespace document_editor
		{
			void AddStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, Ptr<DocumentStyleProperties> style)
			{
				AddStyleVisitor visitor(runRanges, start, end, style);
				run->Accept(&visitor);
			}

			void AddHyperlink(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, const WString& reference, const WString& normalStyleName, const WString& activeStyleName)
			{
				AddHyperlinkVisitor visitor(runRanges, start, end, reference, normalStyleName, activeStyleName);
				run->Accept(&visitor);
			}

			void AddStyleName(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, const WString& styleName)
			{
				AddStyleNameVisitor visitor(runRanges, start, end, styleName);
				run->Accept(&visitor);
			}
		}
	}
}