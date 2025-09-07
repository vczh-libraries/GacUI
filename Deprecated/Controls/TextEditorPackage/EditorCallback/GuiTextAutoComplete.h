/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITEXTAUTOCOMPLETE
#define VCZH_PRESENTATION_CONTROLS_GUITEXTAUTOCOMPLETE

#include "GuiTextGeneralOperations.h"
#include "../../IncludeForward.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiTextBoxAutoCompleteBase
***********************************************************************/
			
			/// <summary>The base class of text box auto complete controller.</summary>
			class GuiTextBoxAutoCompleteBase : public Object, public virtual ICommonTextEditCallback
			{
			public:
				/// <summary>Represents an auto complete candidate item.</summary>
				struct AutoCompleteItem
				{
					/// <summary>Tag object for any purpose, e.g., data binding.</summary>
					description::Value								tag;
					/// <summary>Display text for the item.</summary>
					WString											text;
				};

				/// <summary>Auto complete control provider.</summary>
				class IAutoCompleteControlProvider : public virtual Interface
				{
				public:
					/// <summary>Get the auto complete control that will be installed in a popup to show candidate items.</summary>
					/// <returns>The auto complete control.</returns>
					virtual GuiControl*								GetAutoCompleteControl() = 0;

					/// <summary>Get the list control storing candidate items.</summary>
					/// <returns>The list control. It should be inside the auto complete control, or the auto complete control itself.</returns>
					virtual GuiSelectableListControl*				GetListControl() = 0;

					/// <summary>Store candidate items in the list control.</summary>
					/// <param name="items">Candidate items.</param>
					virtual void									SetSortedContent(const collections::List<AutoCompleteItem>& items) = 0;

					/// <summary>Get the numbers of all stored candidate items.</summary>
					/// <returns>The number of all stored candidate items.</returns>
					virtual vint									GetItemCount() = 0;

					/// <summary>Get the text of a specified item.</summary>
					/// <param name="index">The index of the item.</param>
					/// <returns>The text of the item.</returns>
					virtual WString									GetItemText(vint index) = 0;
				};

				class TextListControlProvider : public Object, public virtual IAutoCompleteControlProvider
				{
				protected:
					GuiTextList*									autoCompleteList;

				public:
					TextListControlProvider(TemplateProperty<templates::GuiTextListTemplate> controlTemplate = {});
					~TextListControlProvider();

					GuiControl*										GetAutoCompleteControl()override;
					GuiSelectableListControl*						GetListControl()override;
					void											SetSortedContent(const collections::List<AutoCompleteItem>& items)override;
					vint											GetItemCount()override;
					WString											GetItemText(vint index)override;
				};

			protected:
				elements::GuiColorizedTextElement*					element;
				SpinLock*											elementModifyLock;
				compositions::GuiGraphicsComposition*				ownerComposition;
				GuiPopup*											autoCompletePopup;
				Ptr<IAutoCompleteControlProvider>					autoCompleteControlProvider;
				TextPos												autoCompleteStartPosition;

				bool												IsPrefix(const WString& prefix, const WString& candidate);
			public:
				/// <summary>Create an auto complete.</summary>
				/// <param name="_autoCompleteControlProvider">A auto complete control provider. Set to null to use a default one.</param>
				GuiTextBoxAutoCompleteBase(Ptr<IAutoCompleteControlProvider> _autoCompleteControlProvider = nullptr);
				~GuiTextBoxAutoCompleteBase();

				void												Attach(elements::GuiColorizedTextElement* _element, SpinLock& _elementModifyLock, compositions::GuiGraphicsComposition* _ownerComposition, vuint editVersion)override;
				void												Detach()override;
				void												TextEditPreview(TextEditPreviewStruct& arguments)override;
				void												TextEditNotify(const TextEditNotifyStruct& arguments)override;
				void												TextCaretChanged(const TextCaretChangedStruct& arguments)override;
				void												TextEditFinished(vuint editVersion)override;

				/// <summary>Get the list state.</summary>
				/// <returns>Returns true if the list is visible.</returns>
				bool												IsListOpening();
				/// <summary>Notify the list to be visible.</summary>
				/// <param name="startPosition">The text position to show the list.</param>
				void												OpenList(TextPos startPosition);
				/// <summary>Notify the list to be invisible.</summary>
				void												CloseList();
				/// <summary>Set the content of the list.</summary>
				/// <param name="items">The content of the list.</param>
				void												SetListContent(const collections::List<AutoCompleteItem>& items);
				/// <summary>Get the last start position when the list is opened.</summary>
				/// <returns>The start position.</returns>
				TextPos												GetListStartPosition();
				/// <summary>Select the previous item.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				bool												SelectPreviousListItem();
				/// <summary>Select the next item.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				bool												SelectNextListItem();
				/// <summary>Apply the selected item into the text box.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				bool												ApplySelectedListItem();
				/// <summary>Get the selected item.</summary>
				/// <returns>The text of the selected item. Returns empty if there is no selected item.</returns>
				WString												GetSelectedListItem();
				/// <summary>Highlight a candidate item in the list.</summary>
				/// <param name="editingText">The text to match an item.</param>
				void												HighlightList(const WString& editingText);
			};
		}
	}
}

#endif