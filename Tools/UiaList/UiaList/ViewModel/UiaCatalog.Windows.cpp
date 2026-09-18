#include "UiaCatalog.Windows.h"
#include "UiaSession.Windows.h"
#include <cmath>

using namespace vl;

namespace uialist::native
{
	ComPtr<IUnknown> AcquireCatalogPattern(IUIAutomationElement* element, PATTERNID id)
	{
		for (vint i = 0; i < PatternCatalogCount; i++) if (PatternCatalog[i].id == id)
		{
			IUnknown* value = nullptr;
			auto hr = element->GetCurrentPatternAs(id, *PatternCatalog[i].iid, reinterpret_cast<void**>(&value));
			if (hr == E_NOINTERFACE || hr == UIA_E_NOTSUPPORTED) return nullptr;
			CheckUia(hr, L"GetCurrentPatternAs(" + itow(id) + L")");
			return value;
		}
		CHECK_FAIL(L"Unknown pattern descriptor");
	}

	const IdDescriptor PropertyCatalog[] =
	{
		{ UIA_RuntimeIdPropertyId, L"UIA_RuntimeIdPropertyId", VT_I4 | VT_ARRAY },
		{ UIA_BoundingRectanglePropertyId, L"UIA_BoundingRectanglePropertyId", VT_R8 | VT_ARRAY },
		{ UIA_ProcessIdPropertyId, L"UIA_ProcessIdPropertyId", VT_I4 },
		{ UIA_ControlTypePropertyId, L"UIA_ControlTypePropertyId", VT_I4 },
		{ UIA_LocalizedControlTypePropertyId, L"UIA_LocalizedControlTypePropertyId", VT_BSTR },
		{ UIA_NamePropertyId, L"UIA_NamePropertyId", VT_BSTR },
		{ UIA_AcceleratorKeyPropertyId, L"UIA_AcceleratorKeyPropertyId", VT_BSTR },
		{ UIA_AccessKeyPropertyId, L"UIA_AccessKeyPropertyId", VT_BSTR },
		{ UIA_HasKeyboardFocusPropertyId, L"UIA_HasKeyboardFocusPropertyId", VT_BOOL },
		{ UIA_IsKeyboardFocusablePropertyId, L"UIA_IsKeyboardFocusablePropertyId", VT_BOOL },
		{ UIA_IsEnabledPropertyId, L"UIA_IsEnabledPropertyId", VT_BOOL },
		{ UIA_AutomationIdPropertyId, L"UIA_AutomationIdPropertyId", VT_BSTR },
		{ UIA_ClassNamePropertyId, L"UIA_ClassNamePropertyId", VT_BSTR },
		{ UIA_HelpTextPropertyId, L"UIA_HelpTextPropertyId", VT_BSTR },
		{ UIA_ClickablePointPropertyId, L"UIA_ClickablePointPropertyId", VT_R8 | VT_ARRAY },
		{ UIA_CulturePropertyId, L"UIA_CulturePropertyId", VT_I4 },
		{ UIA_IsControlElementPropertyId, L"UIA_IsControlElementPropertyId", VT_BOOL },
		{ UIA_IsContentElementPropertyId, L"UIA_IsContentElementPropertyId", VT_BOOL },
		{ UIA_LabeledByPropertyId, L"UIA_LabeledByPropertyId", VT_UNKNOWN },
		{ UIA_IsPasswordPropertyId, L"UIA_IsPasswordPropertyId", VT_BOOL },
		{ UIA_NativeWindowHandlePropertyId, L"UIA_NativeWindowHandlePropertyId", VT_I4 },
		{ UIA_ItemTypePropertyId, L"UIA_ItemTypePropertyId", VT_BSTR },
		{ UIA_IsOffscreenPropertyId, L"UIA_IsOffscreenPropertyId", VT_BOOL },
		{ UIA_OrientationPropertyId, L"UIA_OrientationPropertyId", VT_I4 },
		{ UIA_FrameworkIdPropertyId, L"UIA_FrameworkIdPropertyId", VT_BSTR },
		{ UIA_IsRequiredForFormPropertyId, L"UIA_IsRequiredForFormPropertyId", VT_BOOL },
		{ UIA_ItemStatusPropertyId, L"UIA_ItemStatusPropertyId", VT_BSTR },
		{ UIA_IsDockPatternAvailablePropertyId, L"UIA_IsDockPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsExpandCollapsePatternAvailablePropertyId, L"UIA_IsExpandCollapsePatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsGridItemPatternAvailablePropertyId, L"UIA_IsGridItemPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsGridPatternAvailablePropertyId, L"UIA_IsGridPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsInvokePatternAvailablePropertyId, L"UIA_IsInvokePatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsMultipleViewPatternAvailablePropertyId, L"UIA_IsMultipleViewPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsRangeValuePatternAvailablePropertyId, L"UIA_IsRangeValuePatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsScrollPatternAvailablePropertyId, L"UIA_IsScrollPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsScrollItemPatternAvailablePropertyId, L"UIA_IsScrollItemPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsSelectionItemPatternAvailablePropertyId, L"UIA_IsSelectionItemPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsSelectionPatternAvailablePropertyId, L"UIA_IsSelectionPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsTablePatternAvailablePropertyId, L"UIA_IsTablePatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsTableItemPatternAvailablePropertyId, L"UIA_IsTableItemPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsTextPatternAvailablePropertyId, L"UIA_IsTextPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsTogglePatternAvailablePropertyId, L"UIA_IsTogglePatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsTransformPatternAvailablePropertyId, L"UIA_IsTransformPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsValuePatternAvailablePropertyId, L"UIA_IsValuePatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsWindowPatternAvailablePropertyId, L"UIA_IsWindowPatternAvailablePropertyId", VT_BOOL },
		{ UIA_ValueValuePropertyId, L"UIA_ValueValuePropertyId", VT_BSTR },
		{ UIA_ValueIsReadOnlyPropertyId, L"UIA_ValueIsReadOnlyPropertyId", VT_BOOL },
		{ UIA_RangeValueValuePropertyId, L"UIA_RangeValueValuePropertyId", VT_R8 },
		{ UIA_RangeValueIsReadOnlyPropertyId, L"UIA_RangeValueIsReadOnlyPropertyId", VT_BOOL },
		{ UIA_RangeValueMinimumPropertyId, L"UIA_RangeValueMinimumPropertyId", VT_R8 },
		{ UIA_RangeValueMaximumPropertyId, L"UIA_RangeValueMaximumPropertyId", VT_R8 },
		{ UIA_RangeValueLargeChangePropertyId, L"UIA_RangeValueLargeChangePropertyId", VT_R8 },
		{ UIA_RangeValueSmallChangePropertyId, L"UIA_RangeValueSmallChangePropertyId", VT_R8 },
		{ UIA_ScrollHorizontalScrollPercentPropertyId, L"UIA_ScrollHorizontalScrollPercentPropertyId", VT_R8 },
		{ UIA_ScrollHorizontalViewSizePropertyId, L"UIA_ScrollHorizontalViewSizePropertyId", VT_R8 },
		{ UIA_ScrollVerticalScrollPercentPropertyId, L"UIA_ScrollVerticalScrollPercentPropertyId", VT_R8 },
		{ UIA_ScrollVerticalViewSizePropertyId, L"UIA_ScrollVerticalViewSizePropertyId", VT_R8 },
		{ UIA_ScrollHorizontallyScrollablePropertyId, L"UIA_ScrollHorizontallyScrollablePropertyId", VT_BOOL },
		{ UIA_ScrollVerticallyScrollablePropertyId, L"UIA_ScrollVerticallyScrollablePropertyId", VT_BOOL },
		{ UIA_SelectionSelectionPropertyId, L"UIA_SelectionSelectionPropertyId", VT_UNKNOWN, true },
		{ UIA_SelectionCanSelectMultiplePropertyId, L"UIA_SelectionCanSelectMultiplePropertyId", VT_BOOL },
		{ UIA_SelectionIsSelectionRequiredPropertyId, L"UIA_SelectionIsSelectionRequiredPropertyId", VT_BOOL },
		{ UIA_GridRowCountPropertyId, L"UIA_GridRowCountPropertyId", VT_I4 },
		{ UIA_GridColumnCountPropertyId, L"UIA_GridColumnCountPropertyId", VT_I4 },
		{ UIA_GridItemRowPropertyId, L"UIA_GridItemRowPropertyId", VT_I4 },
		{ UIA_GridItemColumnPropertyId, L"UIA_GridItemColumnPropertyId", VT_I4 },
		{ UIA_GridItemRowSpanPropertyId, L"UIA_GridItemRowSpanPropertyId", VT_I4 },
		{ UIA_GridItemColumnSpanPropertyId, L"UIA_GridItemColumnSpanPropertyId", VT_I4 },
		{ UIA_GridItemContainingGridPropertyId, L"UIA_GridItemContainingGridPropertyId", VT_UNKNOWN },
		{ UIA_DockDockPositionPropertyId, L"UIA_DockDockPositionPropertyId", VT_I4 },
		{ UIA_ExpandCollapseExpandCollapseStatePropertyId, L"UIA_ExpandCollapseExpandCollapseStatePropertyId", VT_I4 },
		{ UIA_MultipleViewCurrentViewPropertyId, L"UIA_MultipleViewCurrentViewPropertyId", VT_I4 },
		{ UIA_MultipleViewSupportedViewsPropertyId, L"UIA_MultipleViewSupportedViewsPropertyId", VT_I4 | VT_ARRAY },
		{ UIA_WindowCanMaximizePropertyId, L"UIA_WindowCanMaximizePropertyId", VT_BOOL },
		{ UIA_WindowCanMinimizePropertyId, L"UIA_WindowCanMinimizePropertyId", VT_BOOL },
		{ UIA_WindowWindowVisualStatePropertyId, L"UIA_WindowWindowVisualStatePropertyId", VT_I4 },
		{ UIA_WindowWindowInteractionStatePropertyId, L"UIA_WindowWindowInteractionStatePropertyId", VT_I4 },
		{ UIA_WindowIsModalPropertyId, L"UIA_WindowIsModalPropertyId", VT_BOOL },
		{ UIA_WindowIsTopmostPropertyId, L"UIA_WindowIsTopmostPropertyId", VT_BOOL },
		{ UIA_SelectionItemIsSelectedPropertyId, L"UIA_SelectionItemIsSelectedPropertyId", VT_BOOL },
		{ UIA_SelectionItemSelectionContainerPropertyId, L"UIA_SelectionItemSelectionContainerPropertyId", VT_UNKNOWN },
		{ UIA_TableRowHeadersPropertyId, L"UIA_TableRowHeadersPropertyId", VT_UNKNOWN, true },
		{ UIA_TableColumnHeadersPropertyId, L"UIA_TableColumnHeadersPropertyId", VT_UNKNOWN, true },
		{ UIA_TableRowOrColumnMajorPropertyId, L"UIA_TableRowOrColumnMajorPropertyId", VT_I4 },
		{ UIA_TableItemRowHeaderItemsPropertyId, L"UIA_TableItemRowHeaderItemsPropertyId", VT_UNKNOWN, true },
		{ UIA_TableItemColumnHeaderItemsPropertyId, L"UIA_TableItemColumnHeaderItemsPropertyId", VT_UNKNOWN, true },
		{ UIA_ToggleToggleStatePropertyId, L"UIA_ToggleToggleStatePropertyId", VT_I4 },
		{ UIA_TransformCanMovePropertyId, L"UIA_TransformCanMovePropertyId", VT_BOOL },
		{ UIA_TransformCanResizePropertyId, L"UIA_TransformCanResizePropertyId", VT_BOOL },
		{ UIA_TransformCanRotatePropertyId, L"UIA_TransformCanRotatePropertyId", VT_BOOL },
		{ UIA_IsLegacyIAccessiblePatternAvailablePropertyId, L"UIA_IsLegacyIAccessiblePatternAvailablePropertyId", VT_BOOL },
		{ UIA_LegacyIAccessibleChildIdPropertyId, L"UIA_LegacyIAccessibleChildIdPropertyId", VT_I4 },
		{ UIA_LegacyIAccessibleNamePropertyId, L"UIA_LegacyIAccessibleNamePropertyId", VT_BSTR },
		{ UIA_LegacyIAccessibleValuePropertyId, L"UIA_LegacyIAccessibleValuePropertyId", VT_BSTR },
		{ UIA_LegacyIAccessibleDescriptionPropertyId, L"UIA_LegacyIAccessibleDescriptionPropertyId", VT_BSTR },
		{ UIA_LegacyIAccessibleRolePropertyId, L"UIA_LegacyIAccessibleRolePropertyId", VT_I4 },
		{ UIA_LegacyIAccessibleStatePropertyId, L"UIA_LegacyIAccessibleStatePropertyId", VT_I4 },
		{ UIA_LegacyIAccessibleHelpPropertyId, L"UIA_LegacyIAccessibleHelpPropertyId", VT_BSTR },
		{ UIA_LegacyIAccessibleKeyboardShortcutPropertyId, L"UIA_LegacyIAccessibleKeyboardShortcutPropertyId", VT_BSTR },
		{ UIA_LegacyIAccessibleSelectionPropertyId, L"UIA_LegacyIAccessibleSelectionPropertyId", VT_UNKNOWN, true },
		{ UIA_LegacyIAccessibleDefaultActionPropertyId, L"UIA_LegacyIAccessibleDefaultActionPropertyId", VT_BSTR },
		{ UIA_AriaRolePropertyId, L"UIA_AriaRolePropertyId", VT_BSTR },
		{ UIA_AriaPropertiesPropertyId, L"UIA_AriaPropertiesPropertyId", VT_BSTR },
		{ UIA_IsDataValidForFormPropertyId, L"UIA_IsDataValidForFormPropertyId", VT_BOOL },
		{ UIA_ControllerForPropertyId, L"UIA_ControllerForPropertyId", VT_UNKNOWN, true },
		{ UIA_DescribedByPropertyId, L"UIA_DescribedByPropertyId", VT_UNKNOWN, true },
		{ UIA_FlowsToPropertyId, L"UIA_FlowsToPropertyId", VT_UNKNOWN, true },
		{ UIA_ProviderDescriptionPropertyId, L"UIA_ProviderDescriptionPropertyId", VT_BSTR },
		{ UIA_IsItemContainerPatternAvailablePropertyId, L"UIA_IsItemContainerPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsVirtualizedItemPatternAvailablePropertyId, L"UIA_IsVirtualizedItemPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsSynchronizedInputPatternAvailablePropertyId, L"UIA_IsSynchronizedInputPatternAvailablePropertyId", VT_BOOL },
		{ UIA_OptimizeForVisualContentPropertyId, L"UIA_OptimizeForVisualContentPropertyId", VT_BOOL },
		{ UIA_IsObjectModelPatternAvailablePropertyId, L"UIA_IsObjectModelPatternAvailablePropertyId", VT_BOOL },
		{ UIA_AnnotationAnnotationTypeIdPropertyId, L"UIA_AnnotationAnnotationTypeIdPropertyId", VT_I4 },
		{ UIA_AnnotationAnnotationTypeNamePropertyId, L"UIA_AnnotationAnnotationTypeNamePropertyId", VT_BSTR },
		{ UIA_AnnotationAuthorPropertyId, L"UIA_AnnotationAuthorPropertyId", VT_BSTR },
		{ UIA_AnnotationDateTimePropertyId, L"UIA_AnnotationDateTimePropertyId", VT_BSTR },
		{ UIA_AnnotationTargetPropertyId, L"UIA_AnnotationTargetPropertyId", VT_UNKNOWN },
		{ UIA_IsAnnotationPatternAvailablePropertyId, L"UIA_IsAnnotationPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsTextPattern2AvailablePropertyId, L"UIA_IsTextPattern2AvailablePropertyId", VT_BOOL },
		{ UIA_StylesStyleIdPropertyId, L"UIA_StylesStyleIdPropertyId", VT_I4 },
		{ UIA_StylesStyleNamePropertyId, L"UIA_StylesStyleNamePropertyId", VT_BSTR },
		{ UIA_StylesFillColorPropertyId, L"UIA_StylesFillColorPropertyId", VT_I4 },
		{ UIA_StylesFillPatternStylePropertyId, L"UIA_StylesFillPatternStylePropertyId", VT_BSTR },
		{ UIA_StylesShapePropertyId, L"UIA_StylesShapePropertyId", VT_BSTR },
		{ UIA_StylesFillPatternColorPropertyId, L"UIA_StylesFillPatternColorPropertyId", VT_I4 },
		{ UIA_StylesExtendedPropertiesPropertyId, L"UIA_StylesExtendedPropertiesPropertyId", VT_BSTR },
		{ UIA_IsStylesPatternAvailablePropertyId, L"UIA_IsStylesPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsSpreadsheetPatternAvailablePropertyId, L"UIA_IsSpreadsheetPatternAvailablePropertyId", VT_BOOL },
		{ UIA_SpreadsheetItemFormulaPropertyId, L"UIA_SpreadsheetItemFormulaPropertyId", VT_BSTR },
		{ UIA_SpreadsheetItemAnnotationObjectsPropertyId, L"UIA_SpreadsheetItemAnnotationObjectsPropertyId", VT_UNKNOWN, true },
		{ UIA_SpreadsheetItemAnnotationTypesPropertyId, L"UIA_SpreadsheetItemAnnotationTypesPropertyId", VT_I4 | VT_ARRAY },
		{ UIA_IsSpreadsheetItemPatternAvailablePropertyId, L"UIA_IsSpreadsheetItemPatternAvailablePropertyId", VT_BOOL },
		{ UIA_Transform2CanZoomPropertyId, L"UIA_Transform2CanZoomPropertyId", VT_BOOL },
		{ UIA_IsTransformPattern2AvailablePropertyId, L"UIA_IsTransformPattern2AvailablePropertyId", VT_BOOL },
		{ UIA_LiveSettingPropertyId, L"UIA_LiveSettingPropertyId", VT_I4 },
		{ UIA_IsTextChildPatternAvailablePropertyId, L"UIA_IsTextChildPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsDragPatternAvailablePropertyId, L"UIA_IsDragPatternAvailablePropertyId", VT_BOOL },
		{ UIA_DragIsGrabbedPropertyId, L"UIA_DragIsGrabbedPropertyId", VT_BOOL },
		{ UIA_DragDropEffectPropertyId, L"UIA_DragDropEffectPropertyId", VT_BSTR },
		{ UIA_DragDropEffectsPropertyId, L"UIA_DragDropEffectsPropertyId", VT_BSTR | VT_ARRAY },
		{ UIA_IsDropTargetPatternAvailablePropertyId, L"UIA_IsDropTargetPatternAvailablePropertyId", VT_BOOL },
		{ UIA_DropTargetDropTargetEffectPropertyId, L"UIA_DropTargetDropTargetEffectPropertyId", VT_BSTR },
		{ UIA_DropTargetDropTargetEffectsPropertyId, L"UIA_DropTargetDropTargetEffectsPropertyId", VT_BSTR | VT_ARRAY },
		{ UIA_DragGrabbedItemsPropertyId, L"UIA_DragGrabbedItemsPropertyId", VT_UNKNOWN, true },
		{ UIA_Transform2ZoomLevelPropertyId, L"UIA_Transform2ZoomLevelPropertyId", VT_R8 },
		{ UIA_Transform2ZoomMinimumPropertyId, L"UIA_Transform2ZoomMinimumPropertyId", VT_R8 },
		{ UIA_Transform2ZoomMaximumPropertyId, L"UIA_Transform2ZoomMaximumPropertyId", VT_R8 },
		{ UIA_FlowsFromPropertyId, L"UIA_FlowsFromPropertyId", VT_UNKNOWN, true },
		{ UIA_IsTextEditPatternAvailablePropertyId, L"UIA_IsTextEditPatternAvailablePropertyId", VT_BOOL },
		{ UIA_IsPeripheralPropertyId, L"UIA_IsPeripheralPropertyId", VT_BOOL },
		{ UIA_IsCustomNavigationPatternAvailablePropertyId, L"UIA_IsCustomNavigationPatternAvailablePropertyId", VT_BOOL },
		{ UIA_PositionInSetPropertyId, L"UIA_PositionInSetPropertyId", VT_I4 },
		{ UIA_SizeOfSetPropertyId, L"UIA_SizeOfSetPropertyId", VT_I4 },
		{ UIA_LevelPropertyId, L"UIA_LevelPropertyId", VT_I4 },
		{ UIA_AnnotationTypesPropertyId, L"UIA_AnnotationTypesPropertyId", VT_I4 | VT_ARRAY },
		{ UIA_AnnotationObjectsPropertyId, L"UIA_AnnotationObjectsPropertyId", VT_UNKNOWN, true },
		{ UIA_LandmarkTypePropertyId, L"UIA_LandmarkTypePropertyId", VT_I4 },
		{ UIA_LocalizedLandmarkTypePropertyId, L"UIA_LocalizedLandmarkTypePropertyId", VT_BSTR },
		{ UIA_FullDescriptionPropertyId, L"UIA_FullDescriptionPropertyId", VT_BSTR },
		{ UIA_FillColorPropertyId, L"UIA_FillColorPropertyId", VT_I4 },
		{ UIA_OutlineColorPropertyId, L"UIA_OutlineColorPropertyId", VT_I4 | VT_ARRAY },
		{ UIA_FillTypePropertyId, L"UIA_FillTypePropertyId", VT_I4 },
		{ UIA_VisualEffectsPropertyId, L"UIA_VisualEffectsPropertyId", VT_I4 },
		{ UIA_OutlineThicknessPropertyId, L"UIA_OutlineThicknessPropertyId", VT_R8 | VT_ARRAY },
		{ UIA_CenterPointPropertyId, L"UIA_CenterPointPropertyId", VT_R8 | VT_ARRAY },
		{ UIA_RotationPropertyId, L"UIA_RotationPropertyId", VT_R8 },
		{ UIA_SizePropertyId, L"UIA_SizePropertyId", VT_R8 | VT_ARRAY },
		{ UIA_IsSelectionPattern2AvailablePropertyId, L"UIA_IsSelectionPattern2AvailablePropertyId", VT_BOOL },
		{ UIA_Selection2FirstSelectedItemPropertyId, L"UIA_Selection2FirstSelectedItemPropertyId", VT_UNKNOWN },
		{ UIA_Selection2LastSelectedItemPropertyId, L"UIA_Selection2LastSelectedItemPropertyId", VT_UNKNOWN },
		{ UIA_Selection2CurrentSelectedItemPropertyId, L"UIA_Selection2CurrentSelectedItemPropertyId", VT_UNKNOWN },
		{ UIA_Selection2ItemCountPropertyId, L"UIA_Selection2ItemCountPropertyId", VT_I4 },
		{ UIA_HeadingLevelPropertyId, L"UIA_HeadingLevelPropertyId", VT_I4 },
		{ UIA_IsDialogPropertyId, L"UIA_IsDialogPropertyId", VT_BOOL },
	};
	const vint PropertyCatalogCount = sizeof(PropertyCatalog) / sizeof(PropertyCatalog[0]);

	const IdDescriptor AttributeCatalog[] =
	{
		{ UIA_AnimationStyleAttributeId, L"UIA_AnimationStyleAttributeId", VT_I4 },
		{ UIA_BackgroundColorAttributeId, L"UIA_BackgroundColorAttributeId", VT_I4 },
		{ UIA_BulletStyleAttributeId, L"UIA_BulletStyleAttributeId", VT_I4 },
		{ UIA_CapStyleAttributeId, L"UIA_CapStyleAttributeId", VT_I4 },
		{ UIA_CultureAttributeId, L"UIA_CultureAttributeId", VT_I4 },
		{ UIA_FontNameAttributeId, L"UIA_FontNameAttributeId", VT_BSTR },
		{ UIA_FontSizeAttributeId, L"UIA_FontSizeAttributeId", VT_R8 },
		{ UIA_FontWeightAttributeId, L"UIA_FontWeightAttributeId", VT_I4 },
		{ UIA_ForegroundColorAttributeId, L"UIA_ForegroundColorAttributeId", VT_I4 },
		{ UIA_HorizontalTextAlignmentAttributeId, L"UIA_HorizontalTextAlignmentAttributeId", VT_I4 },
		{ UIA_IndentationFirstLineAttributeId, L"UIA_IndentationFirstLineAttributeId", VT_R8 },
		{ UIA_IndentationLeadingAttributeId, L"UIA_IndentationLeadingAttributeId", VT_R8 },
		{ UIA_IndentationTrailingAttributeId, L"UIA_IndentationTrailingAttributeId", VT_R8 },
		{ UIA_IsHiddenAttributeId, L"UIA_IsHiddenAttributeId", VT_BOOL },
		{ UIA_IsItalicAttributeId, L"UIA_IsItalicAttributeId", VT_BOOL },
		{ UIA_IsReadOnlyAttributeId, L"UIA_IsReadOnlyAttributeId", VT_BOOL },
		{ UIA_IsSubscriptAttributeId, L"UIA_IsSubscriptAttributeId", VT_BOOL },
		{ UIA_IsSuperscriptAttributeId, L"UIA_IsSuperscriptAttributeId", VT_BOOL },
		{ UIA_MarginBottomAttributeId, L"UIA_MarginBottomAttributeId", VT_R8 },
		{ UIA_MarginLeadingAttributeId, L"UIA_MarginLeadingAttributeId", VT_R8 },
		{ UIA_MarginTopAttributeId, L"UIA_MarginTopAttributeId", VT_R8 },
		{ UIA_MarginTrailingAttributeId, L"UIA_MarginTrailingAttributeId", VT_R8 },
		{ UIA_OutlineStylesAttributeId, L"UIA_OutlineStylesAttributeId", VT_I4 },
		{ UIA_OverlineColorAttributeId, L"UIA_OverlineColorAttributeId", VT_I4 },
		{ UIA_OverlineStyleAttributeId, L"UIA_OverlineStyleAttributeId", VT_I4 },
		{ UIA_StrikethroughColorAttributeId, L"UIA_StrikethroughColorAttributeId", VT_I4 },
		{ UIA_StrikethroughStyleAttributeId, L"UIA_StrikethroughStyleAttributeId", VT_I4 },
		{ UIA_TabsAttributeId, L"UIA_TabsAttributeId", VT_ARRAY | VT_R8 },
		{ UIA_TextFlowDirectionsAttributeId, L"UIA_TextFlowDirectionsAttributeId", VT_I4 },
		{ UIA_UnderlineColorAttributeId, L"UIA_UnderlineColorAttributeId", VT_I4 },
		{ UIA_UnderlineStyleAttributeId, L"UIA_UnderlineStyleAttributeId", VT_I4 },
		{ UIA_AnnotationTypesAttributeId, L"UIA_AnnotationTypesAttributeId", VT_ARRAY | VT_I4 },
		{ UIA_AnnotationObjectsAttributeId, L"UIA_AnnotationObjectsAttributeId", VT_UNKNOWN, true },
		{ UIA_StyleNameAttributeId, L"UIA_StyleNameAttributeId", VT_BSTR },
		{ UIA_StyleIdAttributeId, L"UIA_StyleIdAttributeId", VT_I4 },
		{ UIA_LinkAttributeId, L"UIA_LinkAttributeId", VT_UNKNOWN },
		{ UIA_IsActiveAttributeId, L"UIA_IsActiveAttributeId", VT_BOOL },
		{ UIA_SelectionActiveEndAttributeId, L"UIA_SelectionActiveEndAttributeId", VT_I4 },
		{ UIA_CaretPositionAttributeId, L"UIA_CaretPositionAttributeId", VT_I4 },
		{ UIA_CaretBidiModeAttributeId, L"UIA_CaretBidiModeAttributeId", VT_I4 },
		{ UIA_LineSpacingAttributeId, L"UIA_LineSpacingAttributeId", VT_BSTR },
		{ UIA_BeforeParagraphSpacingAttributeId, L"UIA_BeforeParagraphSpacingAttributeId", VT_R8 },
		{ UIA_AfterParagraphSpacingAttributeId, L"UIA_AfterParagraphSpacingAttributeId", VT_R8 },
		{ UIA_SayAsInterpretAsAttributeId, L"UIA_SayAsInterpretAsAttributeId", VT_I4 },
	};
	const vint AttributeCatalogCount = sizeof(AttributeCatalog) / sizeof(AttributeCatalog[0]);

	const IdDescriptor ControlTypeCatalog[] =
	{
		{ UIA_ButtonControlTypeId, L"UIA_ButtonControlTypeId" },
		{ UIA_CalendarControlTypeId, L"UIA_CalendarControlTypeId" },
		{ UIA_CheckBoxControlTypeId, L"UIA_CheckBoxControlTypeId" },
		{ UIA_ComboBoxControlTypeId, L"UIA_ComboBoxControlTypeId" },
		{ UIA_EditControlTypeId, L"UIA_EditControlTypeId" },
		{ UIA_HyperlinkControlTypeId, L"UIA_HyperlinkControlTypeId" },
		{ UIA_ImageControlTypeId, L"UIA_ImageControlTypeId" },
		{ UIA_ListItemControlTypeId, L"UIA_ListItemControlTypeId" },
		{ UIA_ListControlTypeId, L"UIA_ListControlTypeId" },
		{ UIA_MenuControlTypeId, L"UIA_MenuControlTypeId" },
		{ UIA_MenuBarControlTypeId, L"UIA_MenuBarControlTypeId" },
		{ UIA_MenuItemControlTypeId, L"UIA_MenuItemControlTypeId" },
		{ UIA_ProgressBarControlTypeId, L"UIA_ProgressBarControlTypeId" },
		{ UIA_RadioButtonControlTypeId, L"UIA_RadioButtonControlTypeId" },
		{ UIA_ScrollBarControlTypeId, L"UIA_ScrollBarControlTypeId" },
		{ UIA_SliderControlTypeId, L"UIA_SliderControlTypeId" },
		{ UIA_SpinnerControlTypeId, L"UIA_SpinnerControlTypeId" },
		{ UIA_StatusBarControlTypeId, L"UIA_StatusBarControlTypeId" },
		{ UIA_TabControlTypeId, L"UIA_TabControlTypeId" },
		{ UIA_TabItemControlTypeId, L"UIA_TabItemControlTypeId" },
		{ UIA_TextControlTypeId, L"UIA_TextControlTypeId" },
		{ UIA_ToolBarControlTypeId, L"UIA_ToolBarControlTypeId" },
		{ UIA_ToolTipControlTypeId, L"UIA_ToolTipControlTypeId" },
		{ UIA_TreeControlTypeId, L"UIA_TreeControlTypeId" },
		{ UIA_TreeItemControlTypeId, L"UIA_TreeItemControlTypeId" },
		{ UIA_CustomControlTypeId, L"UIA_CustomControlTypeId" },
		{ UIA_GroupControlTypeId, L"UIA_GroupControlTypeId" },
		{ UIA_ThumbControlTypeId, L"UIA_ThumbControlTypeId" },
		{ UIA_DataGridControlTypeId, L"UIA_DataGridControlTypeId" },
		{ UIA_DataItemControlTypeId, L"UIA_DataItemControlTypeId" },
		{ UIA_DocumentControlTypeId, L"UIA_DocumentControlTypeId" },
		{ UIA_SplitButtonControlTypeId, L"UIA_SplitButtonControlTypeId" },
		{ UIA_WindowControlTypeId, L"UIA_WindowControlTypeId" },
		{ UIA_PaneControlTypeId, L"UIA_PaneControlTypeId" },
		{ UIA_HeaderControlTypeId, L"UIA_HeaderControlTypeId" },
		{ UIA_HeaderItemControlTypeId, L"UIA_HeaderItemControlTypeId" },
		{ UIA_TableControlTypeId, L"UIA_TableControlTypeId" },
		{ UIA_TitleBarControlTypeId, L"UIA_TitleBarControlTypeId" },
		{ UIA_SeparatorControlTypeId, L"UIA_SeparatorControlTypeId" },
		{ UIA_SemanticZoomControlTypeId, L"UIA_SemanticZoomControlTypeId" },
		{ UIA_AppBarControlTypeId, L"UIA_AppBarControlTypeId" },
	};
	const vint ControlTypeCatalogCount = sizeof(ControlTypeCatalog) / sizeof(ControlTypeCatalog[0]);

	const IdDescriptor MetadataCatalog[] =
	{
		{ UIA_SayAsInterpretAsMetadataId, L"UIA_SayAsInterpretAsMetadataId", VT_I4 },
	};
	const vint MetadataCatalogCount = sizeof(MetadataCatalog) / sizeof(MetadataCatalog[0]);

	const PatternDescriptor PatternCatalog[] =
	{
		{ UIA_InvokePatternId, &__uuidof(IUIAutomationInvokePattern), L"IUIAutomationInvokePattern", L"IInvokeProvider" },
		{ UIA_SelectionPatternId, &__uuidof(IUIAutomationSelectionPattern), L"IUIAutomationSelectionPattern", L"ISelectionProvider" },
		{ UIA_ValuePatternId, &__uuidof(IUIAutomationValuePattern), L"IUIAutomationValuePattern", L"IValueProvider" },
		{ UIA_RangeValuePatternId, &__uuidof(IUIAutomationRangeValuePattern), L"IUIAutomationRangeValuePattern", L"IRangeValueProvider" },
		{ UIA_ScrollPatternId, &__uuidof(IUIAutomationScrollPattern), L"IUIAutomationScrollPattern", L"IScrollProvider" },
		{ UIA_ExpandCollapsePatternId, &__uuidof(IUIAutomationExpandCollapsePattern), L"IUIAutomationExpandCollapsePattern", L"IExpandCollapseProvider" },
		{ UIA_GridPatternId, &__uuidof(IUIAutomationGridPattern), L"IUIAutomationGridPattern", L"IGridProvider" },
		{ UIA_GridItemPatternId, &__uuidof(IUIAutomationGridItemPattern), L"IUIAutomationGridItemPattern", L"IGridItemProvider" },
		{ UIA_MultipleViewPatternId, &__uuidof(IUIAutomationMultipleViewPattern), L"IUIAutomationMultipleViewPattern", L"IMultipleViewProvider" },
		{ UIA_WindowPatternId, &__uuidof(IUIAutomationWindowPattern), L"IUIAutomationWindowPattern", L"IWindowProvider" },
		{ UIA_SelectionItemPatternId, &__uuidof(IUIAutomationSelectionItemPattern), L"IUIAutomationSelectionItemPattern", L"ISelectionItemProvider" },
		{ UIA_DockPatternId, &__uuidof(IUIAutomationDockPattern), L"IUIAutomationDockPattern", L"IDockProvider" },
		{ UIA_TablePatternId, &__uuidof(IUIAutomationTablePattern), L"IUIAutomationTablePattern", L"ITableProvider" },
		{ UIA_TableItemPatternId, &__uuidof(IUIAutomationTableItemPattern), L"IUIAutomationTableItemPattern", L"ITableItemProvider" },
		{ UIA_TextPatternId, &__uuidof(IUIAutomationTextPattern), L"IUIAutomationTextPattern", L"ITextProvider" },
		{ UIA_TogglePatternId, &__uuidof(IUIAutomationTogglePattern), L"IUIAutomationTogglePattern", L"IToggleProvider" },
		{ UIA_TransformPatternId, &__uuidof(IUIAutomationTransformPattern), L"IUIAutomationTransformPattern", L"ITransformProvider" },
		{ UIA_ScrollItemPatternId, &__uuidof(IUIAutomationScrollItemPattern), L"IUIAutomationScrollItemPattern", L"IScrollItemProvider" },
		{ UIA_LegacyIAccessiblePatternId, &__uuidof(IUIAutomationLegacyIAccessiblePattern), L"IUIAutomationLegacyIAccessiblePattern", L"ILegacyIAccessibleProvider" },
		{ UIA_ItemContainerPatternId, &__uuidof(IUIAutomationItemContainerPattern), L"IUIAutomationItemContainerPattern", L"IItemContainerProvider" },
		{ UIA_VirtualizedItemPatternId, &__uuidof(IUIAutomationVirtualizedItemPattern), L"IUIAutomationVirtualizedItemPattern", L"IVirtualizedItemProvider" },
		{ UIA_SynchronizedInputPatternId, &__uuidof(IUIAutomationSynchronizedInputPattern), L"IUIAutomationSynchronizedInputPattern", L"ISynchronizedInputProvider" },
		{ UIA_ObjectModelPatternId, &__uuidof(IUIAutomationObjectModelPattern), L"IUIAutomationObjectModelPattern", L"IObjectModelProvider" },
		{ UIA_AnnotationPatternId, &__uuidof(IUIAutomationAnnotationPattern), L"IUIAutomationAnnotationPattern", L"IAnnotationProvider" },
		{ UIA_TextPattern2Id, &__uuidof(IUIAutomationTextPattern2), L"IUIAutomationTextPattern2", L"ITextProvider2" },
		{ UIA_StylesPatternId, &__uuidof(IUIAutomationStylesPattern), L"IUIAutomationStylesPattern", L"IStylesProvider" },
		{ UIA_SpreadsheetPatternId, &__uuidof(IUIAutomationSpreadsheetPattern), L"IUIAutomationSpreadsheetPattern", L"ISpreadsheetProvider" },
		{ UIA_SpreadsheetItemPatternId, &__uuidof(IUIAutomationSpreadsheetItemPattern), L"IUIAutomationSpreadsheetItemPattern", L"ISpreadsheetItemProvider" },
		{ UIA_TransformPattern2Id, &__uuidof(IUIAutomationTransformPattern2), L"IUIAutomationTransformPattern2", L"ITransformProvider2" },
		{ UIA_TextChildPatternId, &__uuidof(IUIAutomationTextChildPattern), L"IUIAutomationTextChildPattern", L"ITextChildProvider" },
		{ UIA_DragPatternId, &__uuidof(IUIAutomationDragPattern), L"IUIAutomationDragPattern", L"IDragProvider" },
		{ UIA_DropTargetPatternId, &__uuidof(IUIAutomationDropTargetPattern), L"IUIAutomationDropTargetPattern", L"IDropTargetProvider" },
		{ UIA_TextEditPatternId, &__uuidof(IUIAutomationTextEditPattern), L"IUIAutomationTextEditPattern", L"ITextEditProvider" },
		{ UIA_CustomNavigationPatternId, &__uuidof(IUIAutomationCustomNavigationPattern), L"IUIAutomationCustomNavigationPattern", L"ICustomNavigationProvider" },
		{ UIA_SelectionPattern2Id, &__uuidof(IUIAutomationSelectionPattern2), L"IUIAutomationSelectionPattern2", L"ISelectionProvider2" },
	};
	const vint PatternCatalogCount = sizeof(PatternCatalog) / sizeof(PatternCatalog[0]);

	Ptr<SetterSpec> UiaSession::DescribeSetter(vint nodeKey, PROPERTYID property)
	{
		auto spec = Ptr(new SetterSpec);
		auto element = elements[nodeKey];
		switch (property)
		{
		case UIA_ValueValuePropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationValuePattern>(element.Obj(), UIA_ValuePatternId))
			{
				BOOL readOnly = TRUE;
				CheckUia(pattern->get_CurrentIsReadOnly(&readOnly), L"Value.IsReadOnly");
				if (readOnly) break;
				spec->kind = SetterKind::Text;
				spec->multiline = true;
				CONTROLTYPEID type;
				UIA_HWND handle;
				CheckUia(element->get_CurrentControlType(&type), L"ControlType(single-line predicate)");
				CheckUia(element->get_CurrentNativeWindowHandle(&handle), L"NativeWindowHandle(single-line predicate)");
				if (type == UIA_EditControlTypeId && handle)
				{
					auto window = reinterpret_cast<HWND>(handle);
					wchar_t className[256] = {};
					if (GetClassNameW(window, className, 256) && _wcsicmp(className, L"Edit") == 0 && !(GetWindowLongPtrW(window, GWL_STYLE) & ES_MULTILINE))
					{
						IUIAutomationElement* nativeValue = nullptr;
						CheckUia(automation->ElementFromHandle(window, &nativeValue), L"ElementFromHandle(single-line predicate)");
						ComPtr<IUIAutomationElement> nativeElement(nativeValue);
						BOOL equal = FALSE;
						CheckUia(automation->CompareElements(element.Obj(), nativeElement.Obj(), &equal), L"CompareElements(single-line predicate)");
						BSTR value = nullptr;
						CheckUia(pattern->get_CurrentValue(&value), L"Value.Value(single-line predicate)");
						bool lineBreak = false;
						for (UINT i = 0; i < SysStringLen(value); i++) if (value[i] == L'\r' || value[i] == L'\n') lineBreak = true;
						SysFreeString(value);
						spec->multiline = !equal || lineBreak;
					}
				}
			}
			break;
		case UIA_RangeValueValuePropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationRangeValuePattern>(element.Obj(), UIA_RangeValuePatternId))
			{
				BOOL readOnly;
				CheckUia(pattern->get_CurrentIsReadOnly(&readOnly), L"RangeValue.IsReadOnly");
				if (readOnly) break;
				CheckUia(pattern->get_CurrentMinimum(&spec->minimum), L"RangeValue.Minimum");
				CheckUia(pattern->get_CurrentMaximum(&spec->maximum), L"RangeValue.Maximum");
				spec->kind = SetterKind::Number;
			}
			break;
		case UIA_WindowWindowVisualStatePropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationWindowPattern>(element.Obj(), UIA_WindowPatternId))
			{
				spec->kind = SetterKind::Choice;
				spec->choices.Add({ WindowVisualState_Normal, L"WindowVisualState_Normal" });
				BOOL can;
				CheckUia(pattern->get_CurrentCanMaximize(&can), L"Window.CanMaximize");
				if (can) spec->choices.Add({ WindowVisualState_Maximized, L"WindowVisualState_Maximized" });
				CheckUia(pattern->get_CurrentCanMinimize(&can), L"Window.CanMinimize");
				if (can) spec->choices.Add({ WindowVisualState_Minimized, L"WindowVisualState_Minimized" });
			}
			break;
		case UIA_DockDockPositionPropertyId:
			if (AcquirePattern<IUIAutomationDockPattern>(element.Obj(), UIA_DockPatternId))
			{
				spec->kind = SetterKind::Choice;
				spec->choices.Add({ DockPosition_Top, L"DockPosition_Top" });
				spec->choices.Add({ DockPosition_Left, L"DockPosition_Left" });
				spec->choices.Add({ DockPosition_Bottom, L"DockPosition_Bottom" });
				spec->choices.Add({ DockPosition_Right, L"DockPosition_Right" });
				spec->choices.Add({ DockPosition_Fill, L"DockPosition_Fill" });
				spec->choices.Add({ DockPosition_None, L"DockPosition_None" });
			}
			break;
		case UIA_MultipleViewCurrentViewPropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationMultipleViewPattern>(element.Obj(), UIA_MultipleViewPatternId))
			{
				spec->kind = SetterKind::Choice;
				SAFEARRAY* views = nullptr;
				CheckUia(pattern->GetCurrentSupportedViews(&views), L"MultipleView.GetCurrentSupportedViews");
				VARIANT array = {}; array.vt = VT_ARRAY | VT_I4; array.parray = views;
				auto values = Convert(array);
				VariantClear(&array);
				for (auto&& value : values->items)
				{
					int id = static_cast<int>(value->signedValue);
					BSTR name = nullptr;
					CheckUia(pattern->GetViewName(id, &name), L"MultipleView.GetViewName");
					spec->choices.Add({ id, WString::CopyFrom(name, SysStringLen(name)) + L" (" + itow(id) + L")" });
					SysFreeString(name);
				}
			}
			break;
		case UIA_Transform2ZoomLevelPropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationTransformPattern2>(element.Obj(), UIA_TransformPattern2Id))
			{
				BOOL can;
				CheckUia(pattern->get_CurrentCanZoom(&can), L"Transform2.CanZoom");
				if (!can) break;
				CheckUia(pattern->get_CurrentZoomMinimum(&spec->minimum), L"Transform2.ZoomMinimum");
				CheckUia(pattern->get_CurrentZoomMaximum(&spec->maximum), L"Transform2.ZoomMaximum");
				spec->kind = SetterKind::Number;
			}
			break;
		case UIA_ScrollHorizontalScrollPercentPropertyId:
		case UIA_ScrollVerticalScrollPercentPropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationScrollPattern>(element.Obj(), UIA_ScrollPatternId))
			{
				BOOL can;
				CheckUia(property == UIA_ScrollHorizontalScrollPercentPropertyId ? pattern->get_CurrentHorizontallyScrollable(&can) : pattern->get_CurrentVerticallyScrollable(&can), L"Scroll.Scrollable");
				if (!can) break;
				spec->kind = SetterKind::Number;
				spec->minimum = 0; spec->maximum = 100;
			}
			break;
		}
		return spec;
	}

	bool UiaSession::SetProperty(vint nodeKey, PROPERTYID property, const WString& text)
	{
		for (vint i = 0; i < text.Length(); i++) if (!text[i]) return false;
		auto spec = DescribeSetter(nodeKey, property);
		if (spec->kind == SetterKind::None) return false;
		bool valid = true;
		double number = 0;
		vint choice = 0;
		if (spec->kind == SetterKind::Number)
		{
			number = wtof_test(text, valid);
			if (!valid || !std::isfinite(number) || number < spec->minimum || number > spec->maximum) return false;
		}
		if (spec->kind == SetterKind::Choice)
		{
			choice = wtoi_test(text, valid);
			if (!valid) return false;
			valid = false;
			for (auto&& item : spec->choices) if (item.id == choice) valid = true;
			if (!valid) return false;
		}
		auto element = elements[nodeKey];
		HRESULT hr = E_UNEXPECTED;
		switch (property)
		{
		case UIA_ValueValuePropertyId:
			{
				auto pattern = AcquirePattern<IUIAutomationValuePattern>(element.Obj(), UIA_ValuePatternId);
				if (!pattern) return false;
				BSTR value = SysAllocStringLen(text.Buffer(), static_cast<UINT>(text.Length()));
				if (!value) CheckUia(E_OUTOFMEMORY, L"SysAllocStringLen(SetValue)");
				hr = pattern->SetValue(value);
				SysFreeString(value);
			}
			break;
		case UIA_RangeValueValuePropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationRangeValuePattern>(element.Obj(), UIA_RangeValuePatternId)) hr = pattern->SetValue(number); else return false;
			break;
		case UIA_WindowWindowVisualStatePropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationWindowPattern>(element.Obj(), UIA_WindowPatternId)) hr = pattern->SetWindowVisualState(static_cast<WindowVisualState>(choice)); else return false;
			break;
		case UIA_DockDockPositionPropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationDockPattern>(element.Obj(), UIA_DockPatternId)) hr = pattern->SetDockPosition(static_cast<DockPosition>(choice)); else return false;
			break;
		case UIA_MultipleViewCurrentViewPropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationMultipleViewPattern>(element.Obj(), UIA_MultipleViewPatternId)) hr = pattern->SetCurrentView(static_cast<int>(choice)); else return false;
			break;
		case UIA_Transform2ZoomLevelPropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationTransformPattern2>(element.Obj(), UIA_TransformPattern2Id)) hr = pattern->Zoom(number); else return false;
			break;
		case UIA_ScrollHorizontalScrollPercentPropertyId:
		case UIA_ScrollVerticalScrollPercentPropertyId:
			if (auto pattern = AcquirePattern<IUIAutomationScrollPattern>(element.Obj(), UIA_ScrollPatternId))
				hr = pattern->SetScrollPercent(property == UIA_ScrollHorizontalScrollPercentPropertyId ? number : UIA_ScrollPatternNoScroll, property == UIA_ScrollVerticalScrollPercentPropertyId ? number : UIA_ScrollPatternNoScroll);
			else return false;
			break;
		default: CHECK_FAIL(L"Property setter catalog mismatch.");
		}
		CheckUia(hr, L"Set property " + IdName(PropertyCatalog, PropertyCatalogCount, property));
		return true;
	}

	WString Hex(vuint64_t value)
	{
		wchar_t buffer[32];
		swprintf_s(buffer, L"0x%llX", value);
		return buffer;
	}

	UiaFailure::UiaFailure(HRESULT value, const WString& operation)
		:Exception(operation + L": HRESULT " + Hex(static_cast<ULONG>(value))), result(value)
	{
	}

	bool UiaFailure::IsUnavailable()const
	{
		return result == UIA_E_ELEMENTNOTAVAILABLE || result == CO_E_OBJNOTCONNECTED || result == RPC_E_DISCONNECTED;
	}

	bool UiaFailure::IsExpected()const
	{
		return IsUnavailable() || result == UIA_E_NOTSUPPORTED || result == E_NOTIMPL || result == UIA_E_ELEMENTNOTENABLED;
	}

	void CheckUia(HRESULT result, const WString& operation)
	{
		if (FAILED(result)) throw UiaFailure(result, operation);
	}

	WString EnumerationName(const WString& field, LONG value)
	{
		auto format = [value](const wchar_t* type, std::initializer_list<IdDescriptor> values, bool flags = false)
		{
			WString name; LONG remaining = value;
			for (auto&& item : values)
			{
				if (item.id == value) return WString(item.name) + L" (" + itow(value) + L")";
				if (flags && item.id && (remaining & item.id) == item.id) { name += (name.Length() ? L" | " : L"") + WString(item.name); remaining &= ~item.id; }
			}
			if (!name.Length() || remaining) name += (name.Length() ? L" | " : L"") + WString(type);
			return name + L" (" + itow(value) + L")";
		};
#define ENUM_WIDE_(N) L##N
#define ENUM_WIDE(N) ENUM_WIDE_(N)
#define E(N) { N, ENUM_WIDE(#N) }
#define FIELD(P, M) (field == ENUM_WIDE(#P) || field == L##M)
		if (FIELD(UIA_OrientationPropertyId, "Orientation")) return format(L"OrientationType", { E(OrientationType_None), E(OrientationType_Horizontal), E(OrientationType_Vertical) });
		if (FIELD(UIA_LiveSettingPropertyId, "LiveSetting")) return format(L"LiveSetting", { E(Off), E(Polite), E(Assertive) });
		if (FIELD(UIA_ExpandCollapseExpandCollapseStatePropertyId, "ExpandCollapseState")) return format(L"ExpandCollapseState", { E(ExpandCollapseState_Collapsed), E(ExpandCollapseState_Expanded), E(ExpandCollapseState_PartiallyExpanded), E(ExpandCollapseState_LeafNode) });
		if (FIELD(UIA_ToggleToggleStatePropertyId, "ToggleState")) return format(L"ToggleState", { E(ToggleState_Off), E(ToggleState_On), E(ToggleState_Indeterminate) });
		if (FIELD(UIA_DockDockPositionPropertyId, "DockPosition")) return format(L"DockPosition", { E(DockPosition_Top), E(DockPosition_Left), E(DockPosition_Bottom), E(DockPosition_Right), E(DockPosition_Fill), E(DockPosition_None) });
		if (FIELD(UIA_WindowWindowVisualStatePropertyId, "WindowVisualState")) return format(L"WindowVisualState", { E(WindowVisualState_Normal), E(WindowVisualState_Maximized), E(WindowVisualState_Minimized) });
		if (FIELD(UIA_WindowWindowInteractionStatePropertyId, "WindowInteractionState")) return format(L"WindowInteractionState", { E(WindowInteractionState_Running), E(WindowInteractionState_Closing), E(WindowInteractionState_ReadyForUserInteraction), E(WindowInteractionState_BlockedByModalWindow), E(WindowInteractionState_NotResponding) });
		if (FIELD(UIA_TableRowOrColumnMajorPropertyId, "RowOrColumnMajor")) return format(L"RowOrColumnMajor", { E(RowOrColumnMajor_RowMajor), E(RowOrColumnMajor_ColumnMajor), E(RowOrColumnMajor_Indeterminate) });
		if (field == L"SupportedTextSelection") return format(L"SupportedTextSelection", { E(SupportedTextSelection_None), E(SupportedTextSelection_Single), E(SupportedTextSelection_Multiple) });
		if (field == L"UIA_AnimationStyleAttributeId") return format(L"AnimationStyle", { E(AnimationStyle_None), E(AnimationStyle_LasVegasLights), E(AnimationStyle_BlinkingBackground), E(AnimationStyle_SparkleText), E(AnimationStyle_MarchingBlackAnts), E(AnimationStyle_MarchingRedAnts), E(AnimationStyle_Shimmer), E(AnimationStyle_Other) });
		if (field == L"UIA_BulletStyleAttributeId") return format(L"BulletStyle", { E(BulletStyle_None), E(BulletStyle_HollowRoundBullet), E(BulletStyle_FilledRoundBullet), E(BulletStyle_HollowSquareBullet), E(BulletStyle_FilledSquareBullet), E(BulletStyle_DashBullet), E(BulletStyle_Other) });
		if (field == L"UIA_CapStyleAttributeId") return format(L"CapStyle", { E(CapStyle_None), E(CapStyle_SmallCap), E(CapStyle_AllCap), E(CapStyle_AllPetiteCaps), E(CapStyle_PetiteCaps), E(CapStyle_Unicase), E(CapStyle_Titling), E(CapStyle_Other) });
		if (field == L"UIA_TextFlowDirectionsAttributeId") return format(L"FlowDirections", { E(FlowDirections_Default), E(FlowDirections_RightToLeft), E(FlowDirections_BottomToTop), E(FlowDirections_Vertical) }, true);
		if (field == L"UIA_OutlineStylesAttributeId") return format(L"OutlineStyles", { E(OutlineStyles_None), E(OutlineStyles_Outline), E(OutlineStyles_Shadow), E(OutlineStyles_Engraved), E(OutlineStyles_Embossed) }, true);
		if (field == L"UIA_HorizontalTextAlignmentAttributeId") return format(L"HorizontalTextAlignment", { E(HorizontalTextAlignment_Left), E(HorizontalTextAlignment_Centered), E(HorizontalTextAlignment_Right), E(HorizontalTextAlignment_Justified) });
		if (field == L"UIA_OverlineStyleAttributeId" || field == L"UIA_UnderlineStyleAttributeId" || field == L"UIA_StrikethroughStyleAttributeId") return format(L"TextDecorationLineStyle", { E(TextDecorationLineStyle_None), E(TextDecorationLineStyle_Single), E(TextDecorationLineStyle_WordsOnly), E(TextDecorationLineStyle_Double), E(TextDecorationLineStyle_Dot), E(TextDecorationLineStyle_Dash), E(TextDecorationLineStyle_DashDot), E(TextDecorationLineStyle_DashDotDot), E(TextDecorationLineStyle_Wavy), E(TextDecorationLineStyle_ThickSingle), E(TextDecorationLineStyle_DoubleWavy), E(TextDecorationLineStyle_ThickWavy), E(TextDecorationLineStyle_LongDash), E(TextDecorationLineStyle_ThickDash), E(TextDecorationLineStyle_ThickDashDot), E(TextDecorationLineStyle_ThickDashDotDot), E(TextDecorationLineStyle_ThickDot), E(TextDecorationLineStyle_ThickLongDash), E(TextDecorationLineStyle_Other) });
#undef FIELD
#undef E
#undef ENUM_WIDE
#undef ENUM_WIDE_
		return {};
	}

	WString IdName(const IdDescriptor* catalog, vint count, LONG id)
	{
		for (vint i = 0; i < count; i++)
		{
			if (catalog[i].id == id) return WString(catalog[i].name) + L" (" + itow(id) + L")";
		}
		return itow(id);
	}
}
