#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
Replace a style name with another one
***********************************************************************/

		namespace document_operation_visitors
		{
			class ReplaceStyleNameVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				WString							oldStyleName;
				WString							newStyleName;

				ReplaceStyleNameVisitor(const WString& _oldStyleName, const WString& _newStyleName)
					:oldStyleName(_oldStyleName)
					, newStyleName(_newStyleName)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					for (auto subRun : run->runs)
					{
						subRun->Accept(this);
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
					if (run->styleName == oldStyleName) run->styleName = newStyleName;
					VisitContainer(run);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					if (run->styleName == oldStyleName) run->styleName = newStyleName;
					if (run->normalStyleName == oldStyleName) run->normalStyleName = newStyleName;
					if (run->activeStyleName == oldStyleName) run->activeStyleName = newStyleName;
					VisitContainer(run);
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
			void ReplaceStyleName(DocumentParagraphRun* run, const WString& oldStyleName, const WString& newStyleName)
			{
				ReplaceStyleNameVisitor visitor(oldStyleName, newStyleName);
				run->Accept(&visitor);
			}
		}
	}
}