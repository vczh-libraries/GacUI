#include "GuiReflectionEvents.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			using namespace collections;
			using namespace parsing;
			using namespace parsing::tabling;
			using namespace parsing::xml;
			using namespace stream;
			using namespace presentation;
			using namespace presentation::compositions;

#ifndef VCZH_DEBUG_NO_REFLECTION

			GUIREFLECTIONCOMPOSITION_TYPELIST(IMPL_VL_TYPE_INFO)

/***********************************************************************
External Functions
***********************************************************************/

			void GuiTableComposition_SetRows(GuiTableComposition* thisObject, vint value)
			{
				vint columns=thisObject->GetColumns();
				if(columns<=0) columns=1;
				thisObject->SetRowsAndColumns(value, columns);
			}

			void GuiTableComposition_SetColumns(GuiTableComposition* thisObject, vint value)
			{
				vint row=thisObject->GetRows();
				if(row<=0) row=1;
				thisObject->SetRowsAndColumns(row, value);
			}

			void IGuiAltActionHost_CollectAltActions(IGuiAltActionHost* host, List<IGuiAltAction*>& actions)
			{
				Group<WString, IGuiAltAction*> group;
				host->CollectAltActions(group);
				for (vint i = 0; i < group.Count(); i++)
				{
					CopyFrom(actions, group.GetByIndex(i), true);
				}
			}

/***********************************************************************
Type Declaration
***********************************************************************/

#define _ ,

#define INTERFACE_IDENTIFIER(INTERFACE)\
	CLASS_MEMBER_STATIC_EXTERNALMETHOD(GetIdentifier, NO_PARAMETER, WString(*)(), []()->WString{return INTERFACE::Identifier;})

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

			BEGIN_ENUM_ITEM(GuiStackComposition::Direction)
				ENUM_ITEM_NAMESPACE(GuiStackComposition)
				ENUM_NAMESPACE_ITEM(Horizontal)
				ENUM_NAMESPACE_ITEM(Vertical)
				ENUM_NAMESPACE_ITEM(ReversedHorizontal)
				ENUM_NAMESPACE_ITEM(ReversedVertical)
			END_ENUM_ITEM(GuiStackComposition::Direction)

			BEGIN_CLASS_MEMBER(GuiStackItemComposition)
				CLASS_MEMBER_BASE(GuiGraphicsSite)
				CLASS_MEMBER_CONSTRUCTOR(GuiStackItemComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_EVENT_FAST(Bounds, BoundsChanged)
				CLASS_MEMBER_PROPERTY_FAST(ExtraMargin)
			END_CLASS_MEMBER(GuiStackItemComposition)

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

			BEGIN_CLASS_MEMBER(GuiTableComposition)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiTableComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_FAST(CellPadding)

				CLASS_MEMBER_METHOD(GetRows, NO_PARAMETER)
				CLASS_MEMBER_EXTERNALMETHOD(SetRows, {L"value"}, void(GuiTableComposition::*)(vint), &GuiTableComposition_SetRows)
				CLASS_MEMBER_PROPERTY(Rows, GetRows, SetRows)
				CLASS_MEMBER_METHOD(GetColumns, NO_PARAMETER)
				CLASS_MEMBER_EXTERNALMETHOD(SetColumns, {L"value"}, void(GuiTableComposition::*)(vint), &GuiTableComposition_SetColumns)
				CLASS_MEMBER_PROPERTY(Columns, GetColumns, SetColumns)
				CLASS_MEMBER_METHOD(SetRowsAndColumns, {L"rows" _ L"columns"})

				CLASS_MEMBER_METHOD(GetSitedCell, {L"rows" _ L"columns"})
				CLASS_MEMBER_METHOD(GetRowOption, {L"row"})
				CLASS_MEMBER_METHOD(SetRowOption, {L"row" _ L"option"})
				CLASS_MEMBER_METHOD(GetColumnOption, {L"column"})
				CLASS_MEMBER_METHOD(SetColumnOption, {L"column" _ L"option"})
				CLASS_MEMBER_METHOD(GetCellArea, NO_PARAMETER)
				CLASS_MEMBER_METHOD(UpdateCellBounds, NO_PARAMETER)
			END_CLASS_MEMBER(GuiTableComposition)

			BEGIN_CLASS_MEMBER(GuiCellComposition)
				CLASS_MEMBER_BASE(GuiGraphicsSite)
				CLASS_MEMBER_CONSTRUCTOR(GuiCellComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(TableParent)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Row)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(RowSpan)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Column)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(ColumnSpan)

				CLASS_MEMBER_METHOD(SetSite, {L"row" _ L"column" _ L"rowSpan" _ L"columnSpan"})
			END_CLASS_MEMBER(GuiCellComposition)

			BEGIN_ENUM_ITEM(FlowAlignment)
				ENUM_CLASS_ITEM(Left)
				ENUM_CLASS_ITEM(Center)
				ENUM_CLASS_ITEM(Extend)
			END_ENUM_ITEM(FlowAlignment)

			BEGIN_CLASS_MEMBER(GuiFlowComposition)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiFlowComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(FlowItems)
				CLASS_MEMBER_PROPERTY_FAST(ExtraMargin)
				CLASS_MEMBER_PROPERTY_FAST(RowPadding)
				CLASS_MEMBER_PROPERTY_FAST(ColumnPadding)
				CLASS_MEMBER_PROPERTY_FAST(Axis)
				CLASS_MEMBER_PROPERTY_FAST(Alignment)
			END_CLASS_MEMBER(GuiFlowComposition)

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

			BEGIN_CLASS_MEMBER(GuiFlowItemComposition)
				CLASS_MEMBER_BASE(GuiGraphicsSite)
				CLASS_MEMBER_CONSTRUCTOR(GuiFlowItemComposition*(), NO_PARAMETER)

				CLASS_MEMBER_PROPERTY_EVENT_FAST(Bounds, BoundsChanged)
				CLASS_MEMBER_PROPERTY_FAST(ExtraMargin)
				CLASS_MEMBER_PROPERTY_FAST(FlowOption)
			END_CLASS_MEMBER(GuiFlowItemComposition)

			BEGIN_CLASS_MEMBER(GuiSideAlignedComposition)
				CLASS_MEMBER_BASE(GuiGraphicsSite)
				CLASS_MEMBER_CONSTRUCTOR(GuiSideAlignedComposition*(), NO_PARAMETER)
				
				CLASS_MEMBER_PROPERTY_FAST(Direction)
				CLASS_MEMBER_PROPERTY_FAST(MaxLength)
				CLASS_MEMBER_PROPERTY_FAST(MaxRatio)
			END_CLASS_MEMBER(GuiSideAlignedComposition)

			BEGIN_ENUM_ITEM(GuiSideAlignedComposition::Direction)
				ENUM_ITEM_NAMESPACE(GuiSideAlignedComposition)
				ENUM_NAMESPACE_ITEM(Left)
				ENUM_NAMESPACE_ITEM(Top)
				ENUM_NAMESPACE_ITEM(Right)
				ENUM_NAMESPACE_ITEM(Bottom)
			END_ENUM_ITEM(GuiSideAlignedComposition::Direction)

			BEGIN_CLASS_MEMBER(GuiPartialViewComposition)
				CLASS_MEMBER_BASE(GuiGraphicsSite)
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
			END_CLASS_MEMBER(GuiSubComponentMeasurer)

			BEGIN_CLASS_MEMBER(GuiSharedSizeRootComposition)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_CONSTRUCTOR(GuiSharedSizeRootComposition*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiSubComponentMeasurerSource)

			BEGIN_INTERFACE_MEMBER(IGuiGraphicsAnimation)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(TotalLength)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(CurrentPosition)

				CLASS_MEMBER_METHOD(Play, {L"currentPosition" _ L"totalLength"})
				CLASS_MEMBER_METHOD(Stop, NO_PARAMETER)
			END_INTERFACE_MEMBER(IGuiGraphicsAnimation)

			BEGIN_CLASS_MEMBER(GuiGraphicsAnimationManager)
				CLASS_MEMBER_METHOD(AddAnimation, {L"animation"})
				CLASS_MEMBER_METHOD(HasAnimation, NO_PARAMETER)
				CLASS_MEMBER_METHOD(Play, NO_PARAMETER)
			END_CLASS_MEMBER(GuiGraphicsAnimationManager)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiShortcutKeyItem)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Manager)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Name)
			END_INTERFACE_MEMBER(IGuiShortcutKeyItem)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiShortcutKeyManager)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(ItemCount)

				CLASS_MEMBER_METHOD(GetItem, {L"index"})
			END_INTERFACE_MEMBER(IGuiShortcutKeyManager)

			BEGIN_CLASS_MEMBER(GuiShortcutKeyManager)
				CLASS_MEMBER_BASE(IGuiShortcutKeyManager)
				CLASS_MEMBER_CONSTRUCTOR(GuiShortcutKeyManager*(), NO_PARAMETER)

				CLASS_MEMBER_METHOD(CreateShortcut, {L"ctrl" _ L"shift" _ L"alt" _ L"ket"})
				CLASS_MEMBER_METHOD(DestroyShortcut, {L"ctrl" _ L"shift" _ L"alt" _ L"ket"})
				CLASS_MEMBER_METHOD(TryGetShortcut, {L"ctrl" _ L"shift" _ L"alt" _ L"ket"})
			END_CLASS_MEMBER(GuiShortcutKeyManager)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiAltAction)
				INTERFACE_IDENTIFIER(IGuiAltAction)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(Alt)

				CLASS_MEMBER_METHOD(IsAltEnabled, NO_PARAMETER)
				CLASS_MEMBER_METHOD(IsAltAvailable, NO_PARAMETER)
				CLASS_MEMBER_METHOD(GetAltComposition, NO_PARAMETER)
				CLASS_MEMBER_METHOD(GetActivatingAltHost, NO_PARAMETER)
				CLASS_MEMBER_METHOD(OnActiveAlt, NO_PARAMETER)
			END_INTERFACE_MEMBER(IGuiAltAction)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiAltActionContainer)
				INTERFACE_IDENTIFIER(IGuiAltActionContainer)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(AltActionCount)
				
				CLASS_MEMBER_METHOD(GetAltAction, { L"index" })
			END_INTERFACE_MEMBER(IGuiAltActionContainer)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IGuiAltActionHost)
				INTERFACE_IDENTIFIER(IGuiAltActionHost)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(PreviousAltHost)

				CLASS_MEMBER_METHOD(OnActivatedAltHost, { L"previousHost" })
				CLASS_MEMBER_METHOD(OnDeactivatedAltHost, NO_PARAMETER)
				CLASS_MEMBER_EXTERNALMETHOD(CollectAltActions, {L"actions"}, void(IGuiAltActionHost::*)(List<IGuiAltAction*>&), &IGuiAltActionHost_CollectAltActions)
			END_INTERFACE_MEMBER(IGuiAltActionHost)

#undef INTERFACE_EXTERNALCTOR
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
#ifndef VCZH_DEBUG_NO_REFLECTION
				ITypeManager* manager=GetGlobalTypeManager();
				if(manager)
				{
					Ptr<ITypeLoader> loader=new GuiCompositionTypeLoader;
					return manager->AddTypeLoader(loader);
				}
#endif
				return false;
			}
		}
	}
}