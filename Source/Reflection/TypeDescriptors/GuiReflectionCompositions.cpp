#include "GuiReflectionPlugin.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			using namespace collections;
			using namespace presentation;
			using namespace presentation::compositions;
			using namespace presentation::controls;

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

#define _ ,

#define INTERFACE_IDENTIFIER(INTERFACE)\
	CLASS_MEMBER_STATIC_EXTERNALMETHOD(GetIdentifier, NO_PARAMETER, WString(*)(), vl::presentation::controls::QueryServiceHelper<::INTERFACE>::GetIdentifier)

/***********************************************************************
Type Declaration (Extra)
***********************************************************************/

			BEGIN_ENUM_ITEM(KeyDirection)
				ENUM_CLASS_ITEM(Up)
				ENUM_CLASS_ITEM(Down)
				ENUM_CLASS_ITEM(Left)
				ENUM_CLASS_ITEM(Right)
				ENUM_CLASS_ITEM(Home)
				ENUM_CLASS_ITEM(End)
				ENUM_CLASS_ITEM(PageUp)
				ENUM_CLASS_ITEM(PageDown)
				ENUM_CLASS_ITEM(PageLeft)
				ENUM_CLASS_ITEM(PageRight)
			END_ENUM_ITEM(KeyDirection)

			BEGIN_INTERFACE_MEMBER(IGuiAxis)
				CLASS_MEMBER_METHOD(RealSizeToVirtualSize, {L"size"})
				CLASS_MEMBER_METHOD(VirtualSizeToRealSize, {L"size"})
				CLASS_MEMBER_METHOD(RealPointToVirtualPoint, {L"realFullSize" _ L"point"})
				CLASS_MEMBER_METHOD(VirtualPointToRealPoint, {L"realFullSize" _ L"point"})
				CLASS_MEMBER_METHOD(RealRectToVirtualRect, {L"realFullSize" _ L"rect"})
				CLASS_MEMBER_METHOD(VirtualRectToRealRect, {L"realFullSize" _ L"rect"})
				CLASS_MEMBER_METHOD(RealMarginToVirtualMargin, {L"margin"})
				CLASS_MEMBER_METHOD(VirtualMarginToRealMargin, {L"margin"})
				CLASS_MEMBER_METHOD(RealKeyDirectionToVirtualKeyDirection, {L"key"})
			END_INTERFACE_MEMBER(IGuiAxis)

			BEGIN_CLASS_MEMBER(GuiDefaultAxis)
				CLASS_MEMBER_BASE(IGuiAxis)
				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiDefaultAxis>(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiDefaultAxis)

			BEGIN_CLASS_MEMBER(GuiAxis)
				CLASS_MEMBER_BASE(IGuiAxis)
				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiAxis>(AxisDirection), {L"axisDirection"})

				CLASS_MEMBER_PROPERTY_READONLY_FAST(Direction)
			END_CLASS_MEMBER(GuiAxis)

			BEGIN_ENUM_ITEM(GuiGraphicsComposition::MinSizeLimitation)
				ENUM_ITEM_NAMESPACE(GuiGraphicsComposition)
				ENUM_NAMESPACE_ITEM(NoLimit)
				ENUM_NAMESPACE_ITEM(LimitToElement)
				ENUM_NAMESPACE_ITEM(LimitToElementAndChildren)
			END_ENUM_ITEM(GuiGraphicsComposition::MinSizeLimitation)

			BEGIN_ENUM_ITEM(GuiStackComposition::Direction)
				ENUM_ITEM_NAMESPACE(GuiStackComposition)
				ENUM_NAMESPACE_ITEM(Horizontal)
				ENUM_NAMESPACE_ITEM(Vertical)
				ENUM_NAMESPACE_ITEM(ReversedHorizontal)
				ENUM_NAMESPACE_ITEM(ReversedVertical)
			END_ENUM_ITEM(GuiStackComposition::Direction)

			BEGIN_STRUCT_MEMBER(GuiCellOption)
				STRUCT_MEMBER(composeType)
				STRUCT_MEMBER(absolute)
				STRUCT_MEMBER(percentage)
			END_STRUCT_MEMBER(GuiCellOption)

			BEGIN_ENUM_ITEM(GuiCellOption::ComposeType)
				ENUM_ITEM_NAMESPACE(GuiCellOption)
				ENUM_NAMESPACE_ITEM(Absolute)
				ENUM_NAMESPACE_ITEM(Percentage)
				ENUM_NAMESPACE_ITEM(MinSize)
			END_ENUM_ITEM(GuiCellOption::ComposeType)

			BEGIN_STRUCT_MEMBER(GuiFlowOption)
				STRUCT_MEMBER(baseline)
				STRUCT_MEMBER(percentage)
				STRUCT_MEMBER(distance)
			END_STRUCT_MEMBER(GuiFlowOption)

			BEGIN_ENUM_ITEM(GuiFlowOption::BaselineType)
				ENUM_ITEM_NAMESPACE(GuiFlowOption)
				ENUM_NAMESPACE_ITEM(Percentage)
				ENUM_NAMESPACE_ITEM(FromTop)
				ENUM_NAMESPACE_ITEM(FromBottom)
			END_ENUM_ITEM(GuiFlowOption::BaselineType)

			BEGIN_ENUM_ITEM(FlowAlignment)
				ENUM_CLASS_ITEM(Left)
				ENUM_CLASS_ITEM(Center)
				ENUM_CLASS_ITEM(Right)
				ENUM_CLASS_ITEM(Extend)
			END_ENUM_ITEM(FlowAlignment)

			BEGIN_ENUM_ITEM(GuiSideAlignedComposition::Direction)
				ENUM_ITEM_NAMESPACE(GuiSideAlignedComposition)
				ENUM_NAMESPACE_ITEM(Left)
				ENUM_NAMESPACE_ITEM(Top)
				ENUM_NAMESPACE_ITEM(Right)
				ENUM_NAMESPACE_ITEM(Bottom)
			END_ENUM_ITEM(GuiSideAlignedComposition::Direction)

			BEGIN_ENUM_ITEM(ResponsiveDirection)
				ENUM_CLASS_ITEM(Horizontal)
				ENUM_CLASS_ITEM(Vertical)
				ENUM_CLASS_ITEM(Both)
			END_ENUM_ITEM(ResponsiveDirection)

			BEGIN_ENUM_ITEM(VirtualRepeatEnsureItemVisibleResult)
				ENUM_CLASS_ITEM(ItemNotExists)
				ENUM_CLASS_ITEM(Moved)
				ENUM_CLASS_ITEM(NotMoved)
			END_ENUM_ITEM(VirtualRepeatEnsureItemVisibleResult)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiShortcutKeyItem)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Manager)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Name)
			END_INTERFACE_MEMBER(IGuiShortcutKeyItem)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiShortcutKeyManager)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(ItemCount)

				CLASS_MEMBER_METHOD(GetItem, {L"index"})
				CLASS_MEMBER_METHOD(TryGetShortcut, { L"ctrl" _ L"shift" _ L"alt" _ L"ket" })
				CLASS_MEMBER_METHOD(CreateNewShortcut, { L"ctrl" _ L"shift" _ L"alt" _ L"ket" })
				CLASS_MEMBER_METHOD(CreateShortcutIfNotExist, { L"ctrl" _ L"shift" _ L"alt" _ L"ket" })
				CLASS_MEMBER_METHOD(DestroyShortcut, { L"ctrl" _ L"shift" _ L"alt" _ L"ket" })
			END_INTERFACE_MEMBER(IGuiShortcutKeyManager)

			BEGIN_CLASS_MEMBER(GuiShortcutKeyManager)
				CLASS_MEMBER_BASE(IGuiShortcutKeyManager)
				CLASS_MEMBER_CONSTRUCTOR(GuiShortcutKeyManager*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiShortcutKeyManager)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiAltAction)
				INTERFACE_IDENTIFIER(vl::presentation::compositions::IGuiAltAction)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(Alt)

				CLASS_MEMBER_METHOD(IsAltEnabled, NO_PARAMETER)
				CLASS_MEMBER_METHOD(IsAltAvailable, NO_PARAMETER)
				CLASS_MEMBER_METHOD(GetAltComposition, NO_PARAMETER)
				CLASS_MEMBER_METHOD(GetActivatingAltHost, NO_PARAMETER)
				CLASS_MEMBER_METHOD(OnActiveAlt, NO_PARAMETER)
			END_INTERFACE_MEMBER(IGuiAltAction)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiAltActionContainer)
				INTERFACE_IDENTIFIER(vl::presentation::compositions::IGuiAltActionContainer)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(AltActionCount)
				
				CLASS_MEMBER_METHOD(GetAltAction, { L"index" })
			END_INTERFACE_MEMBER(IGuiAltActionContainer)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiAltActionHost)
				INTERFACE_IDENTIFIER(vl::presentation::compositions::IGuiAltActionHost)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(AltComposition)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(PreviousAltHost)

				CLASS_MEMBER_METHOD(OnActivatedAltHost, { L"previousHost" })
				CLASS_MEMBER_METHOD(OnDeactivatedAltHost, NO_PARAMETER)
				CLASS_MEMBER_EXTERNALMETHOD(CollectAltActions, {L"actions"}, void(IGuiAltActionHost::*)(List<IGuiAltAction*>&), vl::reflection::description::IGuiAltActionHost_CollectAltActions)
			END_INTERFACE_MEMBER(IGuiAltActionHost)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiTabAction)
				INTERFACE_IDENTIFIER(vl::presentation::compositions::IGuiTabAction)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(AcceptTabInput)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(TabPriority)
				CLASS_MEMBER_METHOD(IsTabEnabled, NO_PARAMETER)
				CLASS_MEMBER_METHOD(IsTabAvailable, NO_PARAMETER)
			END_INTERFACE_MEMBER(IGuiTabAction)

/***********************************************************************
Type Declaration (Class)
***********************************************************************/

			BEGIN_CLASS_MEMBER(GuiGraphicsComposition)
				CLASS_MEMBER_EXTERNALMETHOD(SafeDelete, NO_PARAMETER, void(GuiGraphicsComposition::*)(), vl::presentation::compositions::SafeDeleteComposition)

				CLASS_MEMBER_GUIEVENT_COMPOSITION(leftButtonDown)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(leftButtonUp)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(leftButtonDoubleClick)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(middleButtonDown)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(middleButtonUp)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(middleButtonDoubleClick)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(rightButtonDown)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(rightButtonUp)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(rightButtonDoubleClick)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(horizontalWheel)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(verticalWheel)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(mouseMove)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(mouseEnter)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(mouseLeave)
				
				CLASS_MEMBER_GUIEVENT_COMPOSITION(previewKey)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(keyDown)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(keyUp)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(previewCharInput)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(charInput)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(gotFocus)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(lostFocus)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(caretNotify)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(clipboardNotify)
				CLASS_MEMBER_GUIEVENT_COMPOSITION(renderTargetChanged)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(Parent)
				CLASS_MEMBER_PROPERTY_FAST(OwnedElement)
				CLASS_MEMBER_PROPERTY_FAST(Visible)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(EventuallyVisible)
				CLASS_MEMBER_PROPERTY_FAST(MinSizeLimitation)
				CLASS_MEMBER_PROPERTY_FAST(TransparentToMouse)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(AssociatedControl)
				CLASS_MEMBER_PROPERTY_FAST(AssociatedCursor)
				CLASS_MEMBER_PROPERTY_FAST(AssociatedHitTestResult)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(RelatedControl)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(RelatedControlHost)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(RelatedCursor)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(RelatedHitTestResult)
				CLASS_MEMBER_PROPERTY_FAST(InternalMargin)
				CLASS_MEMBER_PROPERTY_FAST(PreferredMinSize)

				CLASS_MEMBER_METHOD_RENAME(GetChildren, Children, NO_PARAMETER)
				CLASS_MEMBER_PROPERTY_READONLY(Children, GetChildren)

				CLASS_MEMBER_METHOD(AddChild, {L"child"})
				CLASS_MEMBER_METHOD(InsertChild, {L"index" _ L"child"})
				CLASS_MEMBER_METHOD(RemoveChild, {L"child"})
				CLASS_MEMBER_METHOD(MoveChild, {L"child" _ L"newIndex"})
				CLASS_MEMBER_METHOD(Render, {L"size"})
				CLASS_MEMBER_METHOD(FindVisibleComposition, {L"location" _ L"forMouseEvent"})

				CLASS_MEMBER_GUIEVENT(CachedMinSizeChanged)
				CLASS_MEMBER_GUIEVENT(CachedBoundsChanged)
				CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST(CachedMinSize, CachedMinSizeChanged)
				CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST(CachedMinClientSize, CachedMinSizeChanged)
				CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST(CachedBounds, CachedBoundsChanged)
				CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST(CachedClientArea, CachedBoundsChanged)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(GlobalBounds)
			END_CLASS_MEMBER(GuiGraphicsComposition)

			BEGIN_CLASS_MEMBER(GuiWindowComposition)
				CLASS_MEMBER_BASE(GuiGraphicsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiWindowComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiWindowComposition)

			BEGIN_CLASS_MEMBER(GuiBoundsComposition)
				CLASS_MEMBER_BASE(GuiGraphicsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiBoundsComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_FAST(ExpectedBounds)
				CLASS_MEMBER_PROPERTY_FAST(AlignmentToParent)
				
				CLASS_MEMBER_METHOD(IsAlignedToParent, NO_PARAMETER)
			END_CLASS_MEMBER(GuiBoundsComposition)

			BEGIN_CLASS_MEMBER(GuiStackComposition)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiStackComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(StackItems)

				CLASS_MEMBER_PROPERTY_FAST(Direction)
				CLASS_MEMBER_PROPERTY_FAST(Padding)
				CLASS_MEMBER_PROPERTY_FAST(ExtraMargin)

				CLASS_MEMBER_METHOD(InsertStackItem, {L"index" _ L"item"})
				CLASS_MEMBER_METHOD(IsStackItemClipped, NO_PARAMETER)
				CLASS_MEMBER_METHOD(EnsureVisible, {L"index"})
			END_CLASS_MEMBER(GuiStackComposition)

			BEGIN_CLASS_MEMBER(GuiStackItemComposition)
				CLASS_MEMBER_BASE(GuiGraphicsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiStackItemComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_FAST(ExtraMargin)
			END_CLASS_MEMBER(GuiStackItemComposition)

			BEGIN_CLASS_MEMBER(GuiTableComposition)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiTableComposition*(), NO_PARAMETER)

				CLASS_MEMBER_GUIEVENT(ConfigChanged)

				CLASS_MEMBER_PROPERTY_FAST(CellPadding)
				CLASS_MEMBER_PROPERTY_FAST(BorderVisible)

				CLASS_MEMBER_METHOD(GetRows, NO_PARAMETER)
				CLASS_MEMBER_EXTERNALMETHOD(SetRows, {L"value"}, void(GuiTableComposition::*)(vint), vl::reflection::description::GuiTableComposition_SetRows)
				CLASS_MEMBER_PROPERTY(Rows, GetRows, SetRows)
				CLASS_MEMBER_METHOD(GetColumns, NO_PARAMETER)
				CLASS_MEMBER_EXTERNALMETHOD(SetColumns, {L"value"}, void(GuiTableComposition::*)(vint), vl::reflection::description::GuiTableComposition_SetColumns)
				CLASS_MEMBER_PROPERTY(Columns, GetColumns, SetColumns)
				CLASS_MEMBER_METHOD(SetRowsAndColumns, {L"rows" _ L"columns"})

				CLASS_MEMBER_METHOD(GetSitedCell, {L"rows" _ L"columns"})
				CLASS_MEMBER_METHOD(GetRowOption, {L"row"})
				CLASS_MEMBER_METHOD(SetRowOption, {L"row" _ L"option"})
				CLASS_MEMBER_METHOD(GetColumnOption, {L"column"})
				CLASS_MEMBER_METHOD(SetColumnOption, {L"column" _ L"option"})
			END_CLASS_MEMBER(GuiTableComposition)

			BEGIN_CLASS_MEMBER(GuiCellComposition)
				CLASS_MEMBER_BASE(GuiGraphicsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiCellComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(TableParent)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Row)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(RowSpan)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Column)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(ColumnSpan)

				CLASS_MEMBER_METHOD(SetSite, {L"row" _ L"column" _ L"rowSpan" _ L"columnSpan"})
			END_CLASS_MEMBER(GuiCellComposition)

			BEGIN_CLASS_MEMBER(GuiTableSplitterCompositionBase)
				CLASS_MEMBER_BASE(GuiGraphicsComposition)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(TableParent)
			END_CLASS_MEMBER(GuiRowSplitterComposition)

			BEGIN_CLASS_MEMBER(GuiRowSplitterComposition)
				CLASS_MEMBER_BASE(GuiTableSplitterCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiRowSplitterComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_FAST(RowsToTheTop)
			END_CLASS_MEMBER(GuiRowSplitterComposition)

			BEGIN_CLASS_MEMBER(GuiColumnSplitterComposition)
				CLASS_MEMBER_BASE(GuiTableSplitterCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiColumnSplitterComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_FAST(ColumnsToTheLeft)
			END_CLASS_MEMBER(GuiColumnSplitterComposition)

			BEGIN_CLASS_MEMBER(GuiFlowComposition)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiFlowComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(FlowItems)
				CLASS_MEMBER_PROPERTY_FAST(ExtraMargin)
				CLASS_MEMBER_PROPERTY_FAST(RowPadding)
				CLASS_MEMBER_PROPERTY_FAST(ColumnPadding)
				CLASS_MEMBER_PROPERTY_FAST(Axis)
				CLASS_MEMBER_PROPERTY_FAST(Alignment)

				CLASS_MEMBER_METHOD(InsertFlowItem, { L"index" _ L"item" })
			END_CLASS_MEMBER(GuiFlowComposition)

			BEGIN_CLASS_MEMBER(GuiFlowItemComposition)
				CLASS_MEMBER_BASE(GuiGraphicsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiFlowItemComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_FAST(ExtraMargin)
				CLASS_MEMBER_PROPERTY_FAST(FlowOption)
			END_CLASS_MEMBER(GuiFlowItemComposition)

			BEGIN_CLASS_MEMBER(GuiSideAlignedComposition)
				CLASS_MEMBER_BASE(GuiGraphicsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiSideAlignedComposition*(), NO_PARAMETER)
				
				CLASS_MEMBER_PROPERTY_FAST(Direction)
				CLASS_MEMBER_PROPERTY_FAST(MaxLength)
				CLASS_MEMBER_PROPERTY_FAST(MaxRatio)
			END_CLASS_MEMBER(GuiSideAlignedComposition)

			BEGIN_CLASS_MEMBER(GuiPartialViewComposition)
				CLASS_MEMBER_BASE(GuiGraphicsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiPartialViewComposition*(), NO_PARAMETER)
				
				CLASS_MEMBER_PROPERTY_FAST(WidthRatio)
				CLASS_MEMBER_PROPERTY_FAST(WidthPageSize)
				CLASS_MEMBER_PROPERTY_FAST(HeightRatio)
				CLASS_MEMBER_PROPERTY_FAST(HeightPageSize)
			END_CLASS_MEMBER(GuiPartialViewComposition)

			BEGIN_CLASS_MEMBER(GuiSharedSizeItemComposition)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiSharedSizeItemComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_FAST(Group)
				CLASS_MEMBER_PROPERTY_FAST(SharedWidth)
				CLASS_MEMBER_PROPERTY_FAST(SharedHeight)
			END_CLASS_MEMBER(GuiSharedSizeItemComposition)

			BEGIN_CLASS_MEMBER(GuiSharedSizeRootComposition)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiSharedSizeRootComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiSharedSizeRootComposition)

			BEGIN_CLASS_MEMBER(GuiRepeatCompositionBase)
				CLASS_MEMBER_PROPERTY_FAST(ItemTemplate)
				CLASS_MEMBER_PROPERTY_FAST(ItemSource)
				CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(Context)
			END_CLASS_MEMBER(GuiRepeatCompositionBase)

			BEGIN_CLASS_MEMBER(GuiNonVirtialRepeatCompositionBase)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_BASE(GuiRepeatCompositionBase)
				CLASS_MEMBER_GUIEVENT(ItemInserted)
				CLASS_MEMBER_GUIEVENT(ItemRemoved)
			END_CLASS_MEMBER(GuiNonVirtialRepeatCompositionBase)

			BEGIN_CLASS_MEMBER(GuiRepeatStackComposition)
				CLASS_MEMBER_BASE(GuiStackComposition)
				CLASS_MEMBER_BASE(GuiNonVirtialRepeatCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiRepeatStackComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiRepeatStackComposition)

			BEGIN_CLASS_MEMBER(GuiRepeatFlowComposition)
				CLASS_MEMBER_BASE(GuiFlowComposition)
				CLASS_MEMBER_BASE(GuiNonVirtialRepeatCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiRepeatFlowComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiRepeatFlowComposition)

			BEGIN_CLASS_MEMBER(GuiVirtualRepeatCompositionBase)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_BASE(GuiRepeatCompositionBase)
				CLASS_MEMBER_GUIEVENT(AdoptedSizeInvalidated)
				CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(Axis)
				CLASS_MEMBER_PROPERTY_FAST(UseMinimumTotalSize)
				CLASS_MEMBER_PROPERTY_GUIEVENT_READONLY_FAST(TotalSize)
				CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(ViewLocation)
				CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST(AdoptedSize, AdoptedSizeInvalidated)
				CLASS_MEMBER_METHOD(GetVisibleStyle, { L"itemIndex" })
				CLASS_MEMBER_METHOD(GetVisibleIndex, { L"style" })
				CLASS_MEMBER_METHOD(ResetLayout, { L"recreateVisibleStyles" })
				CLASS_MEMBER_METHOD(InvalidateLayout, NO_PARAMETER)
				CLASS_MEMBER_METHOD(FindItemByRealKeyDirection, { L"itemIndex" _ L"key" })
				CLASS_MEMBER_METHOD(FindItemByVirtualKeyDirection, { L"itemIndex" _ L"key" })
				CLASS_MEMBER_METHOD(EnsureItemVisible, { L"itemIndex" })
				CLASS_MEMBER_METHOD(GetAdoptedSize, { L"expectedSize" })
			END_CLASS_MEMBER(GuiNonVirtialRepeatCompositionBase)

			BEGIN_CLASS_MEMBER(GuiRepeatFreeHeightItemComposition)
				CLASS_MEMBER_BASE(GuiVirtualRepeatCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiRepeatFreeHeightItemComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiRepeatFreeHeightItemComposition)

			BEGIN_CLASS_MEMBER(GuiRepeatFixedHeightItemComposition)
				CLASS_MEMBER_BASE(GuiVirtualRepeatCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiRepeatFixedHeightItemComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiRepeatFixedHeightItemComposition)

			BEGIN_CLASS_MEMBER(GuiRepeatFixedSizeMultiColumnItemComposition)
				CLASS_MEMBER_BASE(GuiVirtualRepeatCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiRepeatFixedSizeMultiColumnItemComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiRepeatFixedSizeMultiColumnItemComposition)

			BEGIN_CLASS_MEMBER(GuiRepeatFixedHeightMultiColumnItemComposition)
				CLASS_MEMBER_BASE(GuiVirtualRepeatCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiRepeatFixedHeightMultiColumnItemComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiRepeatFixedHeightMultiColumnItemComposition)

			BEGIN_CLASS_MEMBER(GuiResponsiveCompositionBase)
				CLASS_MEMBER_BASE(GuiBoundsComposition)

				CLASS_MEMBER_PROPERTY_GUIEVENT_READONLY_FAST(LevelCount)
				CLASS_MEMBER_PROPERTY_GUIEVENT_READONLY_FAST(CurrentLevel)
				CLASS_MEMBER_PROPERTY_FAST(Direction)
				CLASS_MEMBER_METHOD(LevelDown, NO_PARAMETER)
				CLASS_MEMBER_METHOD(LevelUp, NO_PARAMETER)
			END_CLASS_MEMBER(GuiResponsiveCompositionBase)

			BEGIN_CLASS_MEMBER(GuiResponsiveSharedComposition)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiResponsiveSharedComposition*(), NO_PARAMETER)
				CLASS_MEMBER_PROPERTY_FAST(Shared)
			END_CLASS_MEMBER(GuiResponsiveSharedComposition)

			BEGIN_CLASS_MEMBER(GuiResponsiveViewComposition)
				CLASS_MEMBER_BASE(GuiResponsiveCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiResponsiveViewComposition*(), NO_PARAMETER)

				CLASS_MEMBER_GUIEVENT(BeforeSwitchingView)
					
				CLASS_MEMBER_PROPERTY_READONLY_FAST(CurrentView)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(SharedControls)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Views)
			END_CLASS_MEMBER(GuiResponsiveViewComposition)

			BEGIN_CLASS_MEMBER(GuiResponsiveFixedComposition)
				CLASS_MEMBER_BASE(GuiResponsiveCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiResponsiveFixedComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiResponsiveFixedComposition)

			BEGIN_CLASS_MEMBER(GuiResponsiveStackComposition)
				CLASS_MEMBER_BASE(GuiResponsiveCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiResponsiveStackComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiResponsiveStackComposition)

			BEGIN_CLASS_MEMBER(GuiResponsiveGroupComposition)
				CLASS_MEMBER_BASE(GuiResponsiveCompositionBase)
				CLASS_MEMBER_CONSTRUCTOR(GuiResponsiveGroupComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiResponsiveGroupComposition)

			BEGIN_CLASS_MEMBER(GuiResponsiveContainerComposition)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiResponsiveContainerComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_FAST(ResponsiveTarget)
			END_CLASS_MEMBER(GuiResponsiveContainerComposition)

#undef INTERFACE_IDENTIFIER
#undef _

/***********************************************************************
Type Loader
***********************************************************************/

			class GuiCompositionTypeLoader : public Object, public ITypeLoader
			{
			public:
				void Load(ITypeManager* manager)
				{
					GUIREFLECTIONCOMPOSITION_TYPELIST(ADD_TYPE_INFO)
				}

				void Unload(ITypeManager* manager)
				{
				}
			};

#endif

			bool LoadGuiCompositionTypes()
			{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				ITypeManager* manager=GetGlobalTypeManager();
				if(manager)
				{
					auto loader=Ptr(new GuiCompositionTypeLoader);
					return manager->AddTypeLoader(loader);
				}
#endif
				return false;
			}
		}
	}
}