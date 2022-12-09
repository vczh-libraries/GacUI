#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
Get the hyperlink run that contains the specified position
***********************************************************************/

		namespace document_operation_visitors
		{
			class LocateHyperlinkVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				Ptr<DocumentHyperlinkRun::Package>	package;
				RunRangeMap&						runRanges;
				vint								start;
				vint								end;

				LocateHyperlinkVisitor(RunRangeMap& _runRanges, Ptr<DocumentHyperlinkRun::Package> _package, vint _start, vint _end)
					:runRanges(_runRanges)
					, package(_package)
					, start(_start)
					, end(_end)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					Ptr<DocumentRun> selectedRun;
					for (auto subRun : run->runs)
					{
						RunRange range = runRanges[subRun.Obj()];
						if (range.start <= start && end <= range.end)
						{
							subRun->Accept(this);
							break;
						}
					}
				}

				void Visit(DocumentTextRun* run)override
				{
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
					package->hyperlinks.Add(Ptr(run));
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
		using namespace document_operation_visitors;

		namespace document_editor
		{
			Ptr<DocumentHyperlinkRun::Package> LocateHyperlink(DocumentParagraphRun* run, RunRangeMap& runRanges, vint row, vint start, vint end)
			{
				auto package = Ptr(new DocumentHyperlinkRun::Package);
				package->row = row;
				{
					LocateHyperlinkVisitor visitor(runRanges, package, start, end);
					run->Accept(&visitor);
				}

				Ptr<DocumentHyperlinkRun> startRun, endRun;
				for (auto run : package->hyperlinks)
				{
					auto range = runRanges[run.Obj()];
					if (package->start == -1 || range.start < package->start)
					{
						package->start = range.start;
						startRun = run;
					}
					if (package->end == -1 || range.end > package->end)
					{
						package->end = range.end;
						endRun = run;
					}
				}

				while (startRun)
				{
					vint pos = runRanges[startRun.Obj()].start;
					if (pos == 0) break;

					auto newPackage = Ptr(new DocumentHyperlinkRun::Package);
					LocateHyperlinkVisitor visitor(runRanges, newPackage, pos - 1, pos);
					run->Accept(&visitor);
					if (newPackage->hyperlinks.Count() == 0) break;

					auto newRun = newPackage->hyperlinks[0];
					if (startRun->reference != newRun->reference) break;

					auto range = runRanges[newRun.Obj()];
					package->hyperlinks.Add(newRun);
					package->start = range.start;
					startRun = newRun;
				}

				vint length = runRanges[run].end;
				while (endRun)
				{
					vint pos = runRanges[endRun.Obj()].end;
					if (pos == length) break;

					auto newPackage = Ptr(new DocumentHyperlinkRun::Package);
					LocateHyperlinkVisitor visitor(runRanges, newPackage, pos, pos + 1);
					run->Accept(&visitor);
					if (newPackage->hyperlinks.Count() == 0) break;

					auto newRun = newPackage->hyperlinks[0];
					if (endRun->reference != newRun->reference) break;

					auto range = runRanges[newRun.Obj()];
					package->hyperlinks.Add(newRun);
					package->end = range.end;
					endRun = newRun;
				}

				return package;
			}
		}
	}
}