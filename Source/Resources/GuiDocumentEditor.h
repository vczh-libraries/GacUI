/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Resource

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_RESOURCES_GUIDOCUMENTEDITOR
#define VCZH_PRESENTATION_RESOURCES_GUIDOCUMENTEDITOR

#include "GuiDocument.h"

namespace vl
{
	namespace presentation
	{
		typedef DocumentModel::RunRange			RunRange;
		typedef DocumentModel::RunRangeMap		RunRangeMap;

		namespace document_editor
		{
			extern void									GetRunRange(DocumentParagraphRun* run, RunRangeMap& runRanges);
			extern void									LocateStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, vint position, bool frontSide, collections::List<DocumentContainerRun*>& locatedRuns);
			extern Ptr<DocumentHyperlinkRun::Package>	LocateHyperlink(DocumentParagraphRun* run, RunRangeMap& runRanges, vint row, vint start, vint end);
			extern Ptr<DocumentStyleProperties>			CopyStyle(Ptr<DocumentStyleProperties> style);
			extern Ptr<DocumentRun>						CopyRun(DocumentRun* run);
			extern Ptr<DocumentRun>						CopyStyledText(collections::List<DocumentContainerRun*>& styleRuns, const WString& text);
			extern Ptr<DocumentRun>						CopyRunRecursively(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, bool deepCopy);
			extern void									CollectStyleName(DocumentParagraphRun* run, collections::List<WString>& styleNames);
			extern void									ReplaceStyleName(DocumentParagraphRun* run, const WString& oldStyleName, const WString& newStyleName);
			extern void									RemoveRun(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end);
			extern void									CutRun(DocumentParagraphRun* run, RunRangeMap& runRanges, vint position, Ptr<DocumentRun>& leftRun, Ptr<DocumentRun>& rightRun);
			extern void									ClearUnnecessaryRun(DocumentParagraphRun* run, DocumentModel* model);
			extern void									AddStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, Ptr<DocumentStyleProperties> style);
			extern void									AddHyperlink(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, const WString& reference, const WString& normalStyleName, const WString& activeStyleName);
			extern void									AddStyleName(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, const WString& styleName);
			extern void									RemoveHyperlink(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end);
			extern void									RemoveStyleName(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end);
			extern void									ClearStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end);
			extern Ptr<DocumentStyleProperties>			SummarizeStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, DocumentModel* model, vint start, vint end);
			extern Nullable<WString>					SummarizeStyleName(DocumentParagraphRun* run, RunRangeMap& runRanges, DocumentModel* model, vint start, vint end);
			extern void									AggregateStyle(Ptr<DocumentStyleProperties>& dst, Ptr<DocumentStyleProperties> src);
		}
	}
}

#endif