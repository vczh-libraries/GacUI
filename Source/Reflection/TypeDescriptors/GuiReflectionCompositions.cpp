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

#define INTERFACE_EXTERNALCTOR(CONTROL, INTERFACE)\
	CLASS_MEMBER_EXTERNALCTOR(decltype(interface_proxy::CONTROL##_##INTERFACE::Create(0))(Ptr<IValueInterfaceProxy>), {L"proxy"}, &interface_proxy::CONTROL##_##INTERFACE::Create)

#define INTERFACE_IDENTIFIER(INTERFACE)\
	CLASS_MEMBER_STATIC_EXTERNALMETHOD(GetIdentifier, NO_PARAMETER, WString(*)(), []()->WString{return INTERFACE::Identifier;})

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

			BEGIN_CLASS_MEMBER(IGuiGraphicsAnimation)
				CLASS_MEMBER_BASE(IDescriptable)
				INTERFACE_EXTERNALCTOR(composition, IGuiGraphicsAnimation)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(TotalLength)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(CurrentPosition)

				CLASS_MEMBER_METHOD(Play, {L"currentPosition" _ L"totalLength"})
				CLASS_MEMBER_METHOD(Stop, NO_PARAMETER)
			END_CLASS_MEMBER(IGuiGraphicsAnimation)

			BEGIN_CLASS_MEMBER(GuiGraphicsAnimationManager)
				CLASS_MEMBER_METHOD(AddAnimation, {L"animation"})
				CLASS_MEMBER_METHOD(HasAnimation, NO_PARAMETER)
				CLASS_MEMBER_METHOD(Play, NO_PARAMETER)
			END_CLASS_MEMBER(GuiGraphicsAnimationManager)

			BEGIN_CLASS_MEMBER(IGuiShortcutKeyItem)
				CLASS_MEMBER_BASE(IDescriptable)
				
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Manager)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(Name)
			END_CLASS_MEMBER(IGuiShortcutKeyItem)

			BEGIN_CLASS_MEMBER(IGuiShortcutKeyManager)
				CLASS_MEMBER_BASE(IDescriptable)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(ItemCount)

				CLASS_MEMBER_METHOD(GetItem, {L"index"})
			END_CLASS_MEMBER(IGuiShortcutKeyManager)

			BEGIN_CLASS_MEMBER(GuiShortcutKeyManager)
				CLASS_MEMBER_BASE(IGuiShortcutKeyManager)
				CLASS_MEMBER_CONSTRUCTOR(GuiShortcutKeyManager*(), NO_PARAMETER)

				CLASS_MEMBER_METHOD(CreateShortcut, {L"ctrl" _ L"shift" _ L"alt" _ L"ket"})
				CLASS_MEMBER_METHOD(DestroyShortcut, {L"ctrl" _ L"shift" _ L"alt" _ L"ket"})
				CLASS_MEMBER_METHOD(TryGetShortcut, {L"ctrl" _ L"shift" _ L"alt" _ L"ket"})
			END_CLASS_MEMBER(GuiShortcutKeyManager)

			BEGIN_CLASS_MEMBER(IGuiAltAction)
				INTERFACE_IDENTIFIER(IGuiAltAction)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(Alt)

				CLASS_MEMBER_METHOD(IsAltEnabled, NO_PARAMETER)
				CLASS_MEMBER_METHOD(IsAltAvailable, NO_PARAMETER)
				CLASS_MEMBER_METHOD(GetAltComposition, NO_PARAMETER)
				CLASS_MEMBER_METHOD(GetActivatingAltHost, NO_PARAMETER)
				CLASS_MEMBER_METHOD(OnActiveAlt, NO_PARAMETER)
			END_CLASS_MEMBER(IGuiAltAction)

			BEGIN_CLASS_MEMBER(IGuiAltActionContainer)
				INTERFACE_IDENTIFIER(IGuiAltActionContainer)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(AltActionCount)
				
				CLASS_MEMBER_METHOD(GetAltAction, { L"index" })
			END_CLASS_MEMBER(IGuiAltActionContainer)

			BEGIN_CLASS_MEMBER(IGuiAltActionHost)
				INTERFACE_IDENTIFIER(IGuiAltActionHost)

				CLASS_MEMBER_PROPERTY_READONLY_FAST(PreviousAltHost)

				CLASS_MEMBER_METHOD(OnActivatedAltHost, { L"previousHost" })
				CLASS_MEMBER_METHOD(OnDeactivatedAltHost, NO_PARAMETER)
				CLASS_MEMBER_EXTERNALMETHOD(CollectAltActions, {L"actions"}, void(IGuiAltActionHost::*)(List<IGuiAltAction*>&), &IGuiAltActionHost_CollectAltActions)
			END_CLASS_MEMBER(IGuiAltActionHost)

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