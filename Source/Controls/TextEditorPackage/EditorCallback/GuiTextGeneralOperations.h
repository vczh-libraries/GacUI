/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITEXTGENERALOPERATIONS
#define VCZH_PRESENTATION_CONTROLS_GUITEXTGENERALOPERATIONS

#include "../../Templates/GuiControlShared.h"
#include "../../../GraphicsElement/GuiGraphicsTextElement.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
Common Operations
***********************************************************************/
			
			/// <summary>An text edit callback for text box controls.</summary>
			class ICommonTextEditCallback : public virtual IDescriptable, public Description<ICommonTextEditCallback>
			{
			public:
				/// <summary>Callback data for text editing preview.</summary>
				struct TextEditPreviewStruct
				{
					/// <summary>The start position of the selection before replacing. This field can be modified.</summary>
					TextPos								originalStart;
					/// <summary>The end position of the selection before replacing. This field can be modified.</summary>
					TextPos								originalEnd;
					/// <summary>The text of the selection before replacing.</summary>
					WString								originalText;
					/// <summary>The text of the selection after replacing. This field can be modified.</summary>
					WString								inputText;
					/// <summary>The base edit version.</summary>
					vuint								editVersion = 0;
					/// <summary>True if this modification is raised by the keyboard.</summary>
					bool								keyInput = false;
				};

				/// <summary>Callback data for text editing.</summary>
				struct TextEditNotifyStruct
				{
					/// <summary>The start position of the selection before replacing.</summary>
					TextPos								originalStart;
					/// <summary>The end position of the selection before replacing.</summary>
					TextPos								originalEnd;
					/// <summary>The text of the selection before replacing.</summary>
					WString								originalText;
					/// <summary>The start position of the selection after replacing.</summary>
					TextPos								inputStart;
					/// <summary>The end position of the selection after replacing.</summary>
					TextPos								inputEnd;
					/// <summary>The text of the selection after replacing.</summary>
					WString								inputText;
					/// <summary>The created edit version.</summary>
					vuint								editVersion = 0;
					/// <summary>True if this modification is raised by the keyboard.</summary>
					bool								keyInput = false;
				};
				
				/// <summary>Callback data for text caret changing.</summary>
				struct TextCaretChangedStruct
				{
					/// <summary>The start position of the selection before caret changing.</summary>
					TextPos								oldBegin;
					/// <summary>The end position of the selection before caret changing.</summary>
					TextPos								oldEnd;
					/// <summary>The start position of the selection after caret changing.</summary>
					TextPos								newBegin;
					/// <summary>The end position of the selection after caret changing.</summary>
					TextPos								newEnd;
					/// <summary>The current edit version.</summary>
					vuint								editVersion = 0;
				};

				/// <summary>Called when the callback is attached to a text box control.</summary>
				/// <param name="element">The element that used in the text box control.</param>
				/// <param name="elementModifyLock">The lock that pretect the element.</param>
				/// <param name="ownerComposition">The owner composition of this element.</param>
				/// <param name="editVersion">The current edit version.</param>
				virtual void							Attach(elements::GuiColorizedTextElement* element, SpinLock& elementModifyLock, compositions::GuiGraphicsComposition* ownerComposition, vuint editVersion)=0;
				/// <summary>Called when the callback is detached from a text box control.</summary>
				virtual void							Detach()=0;
				/// <summary>Called before the text is edited.</summary>
				/// <param name="arguments">The data for this callback.</param>
				virtual void							TextEditPreview(TextEditPreviewStruct& arguments)=0;
				/// <summary>Called after the text is edited and before the caret is changed.</summary>
				/// <param name="arguments">The data for this callback.</param>
				virtual void							TextEditNotify(const TextEditNotifyStruct& arguments)=0;
				/// <summary>Called after the caret is changed.</summary>
				/// <param name="arguments">The data for this callback.</param>
				virtual void							TextCaretChanged(const TextCaretChangedStruct& arguments)=0;
				/// <summary>Called after the text is edited and after the caret is changed.</summary>
				/// <param name="editVersion">The current edit version.</param>
				virtual void							TextEditFinished(vuint editVersion)=0;
			};
		}
	}
}

#endif