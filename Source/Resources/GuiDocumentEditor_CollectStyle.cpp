#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
Search all used style names
***********************************************************************/

		namespace document_operation_visitors
		{
			class CollectStyleNameVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				List<WString>&					styleNames;

				CollectStyleNameVisitor(List<WString>& _styleNames)
					:styleNames(_styleNames)
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
					if (!styleNames.Contains(run->styleName))
					{
						styleNames.Add(run->styleName);
					}
					VisitContainer(run);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					if (!styleNames.Contains(run->normalStyleName))
					{
						styleNames.Add(run->normalStyleName);
					}
					if (!styleNames.Contains(run->activeStyleName))
					{
						styleNames.Add(run->activeStyleName);
					}
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
			void CollectStyleName(DocumentParagraphRun* run, List<WString>& styleNames)
			{
				CollectStyleNameVisitor visitor(styleNames);
				run->Accept(&visitor);
			}
		}
	}
}