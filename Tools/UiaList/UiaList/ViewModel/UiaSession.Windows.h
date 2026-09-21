#ifndef VCZH_UIALIST_SESSION_WINDOWS
#define VCZH_UIALIST_SESSION_WINDOWS

#include "UiaCatalog.Windows.h"

namespace uialist::native
{
	enum class ValueKind
	{
		Null, Unsupported, Mixed, Boolean, Signed, Unsigned, Real, String, Element, Range, Array, Opaque,
	};

	struct ValueData
	{
		ValueKind					kind = ValueKind::Null;
		VARTYPE						type = VT_EMPTY;
		vl::WString					text;
		vl::vint64_t					signedValue = 0;
		vl::vuint64_t				unsignedValue = 0;
		double							realValue = 0;
		vl::vint					key = 0;
		vl::collections::List<vl::Ptr<ValueData>> items;
		vl::collections::List<vl::collections::Pair<LONG, LONG>> dimensions;
	};

	enum class SetterKind { None, Text, Number, Choice };

	struct SetterChoice
	{
		int id = 0;
		vl::WString label;
	};

	struct SetterSpec
	{
		SetterKind kind = SetterKind::None;
		bool multiline = false;
		double minimum = 0, maximum = 0;
		vl::collections::List<SetterChoice> choices;
	};

	struct PropertyData
	{
		LONG						id = 0;
		vl::WString					name;
		vl::Ptr<ValueData>			value;
		vl::Ptr<SetterSpec>			setter;
		PATTERNID sourcePattern = 0;
	};

	enum class ArgumentKind { Text, Integer, Number, Choice, Element, Range, Variant, AttributeList };
	enum class ActionCode
	{
		SetFocus, ClickablePoint, ElementMenu, Metadata,
		Invoke, SetValue, SetRangeValue, Scroll, SetScrollPercent, Expand, Collapse, GridItem,
		ViewName, SetView, WindowState, WaitForIdle, CloseWindow, SelectItem, AddItem, RemoveItem,
		SetDock, Selection, RowHeaders, ColumnHeaders, RowHeaderItems, ColumnHeaderItems,
		DocumentRange, TextSelection, VisibleRanges, RangeFromPoint, RangeFromChild, Toggle,
		Move, Resize, Rotate, ScrollItem, LegacySelection, LegacySelect, LegacyDefault, LegacyValue,
		LegacyObject, FindItem, Realize, StartListening, CancelListening, ObjectModel,
		RangeFromAnnotation, CaretRange, ExtendedProperties, SpreadsheetItem, AnnotationObjects,
		AnnotationTypes, Zoom, ZoomByUnit, ChildRange, GrabbedItems, CompositionRange,
		ConversionRange, Navigate,
		RangeClone, RangeCompare, RangeCompareEndpoints, RangeExpand, RangeMove, RangeMoveEndpoint,
		RangeTransferEndpoint, RangeFindText, RangeFindAttribute, RangeAttribute, RangeAllAttributes,
		RangeText, RangeRectangles, RangeEnclosing, RangeChildren, RangeSelect, RangeAdd,
		RangeRemove, RangeScroll, RangeMenu, RangeEnclosingCache, RangeChildrenCache, RangeAttributes,
	};

	struct ArgumentSpec
	{
		vl::WString name;
		ArgumentKind kind = ArgumentKind::Text;
		vl::WString initial;
		double minimum = 0, maximum = 0;
		bool nullable = false, multiline = false;
		vl::collections::List<SetterChoice> choices;
	};

	struct ActionSpec
	{
		ActionCode code = ActionCode::Invoke;
		PATTERNID pattern = 0;
		vl::vint rangeKey = 0;
		vl::WString name;
		bool enabled = true, mutation = false, pureGetter = false;
		vl::collections::List<vl::Ptr<ArgumentSpec>> parameters;
	};

	struct ActionArgument
	{
		ArgumentKind kind = ArgumentKind::Text;
		vl::WString text;
		int integer = 0;
		double number = 0;
		vl::vint reference = 0;
		vl::Ptr<ValueData> value;
		vl::collections::List<int> ids;
	};

	struct ActionSectionData
	{
		PATTERNID pattern = 0;
		vl::vint rangeKey = 0, documentKey = 0;
		vl::WString name;
		vl::collections::List<PropertyData> readouts;
		vl::collections::List<vl::Ptr<ActionSpec>> commands;
	};

	struct ReferenceData
	{
		ValueKind kind = ValueKind::Element;
		vl::vint key = 0, documentKey = 0;
		vl::WString label;
	};

	struct RangeRecord
	{
		vl::ComPtr<IUIAutomationTextRange> range;
		vl::vint documentKey = 0;
		vl::WString name;
	};

	struct ActionOutcome
	{
		bool available = true, mutation = false, closedWindow = false;
		vl::Ptr<ValueData> value;
		vl::collections::List<ReferenceData> references;
	};

	struct NodeData
	{
		vl::vint					key = 0;
		vl::vint					parent = 0;
		vl::vint					depth = 0;
		vl::WString					runtimeId;
		vl::WString					name;
		vl::WString					client;
		vl::WString					role;
		CONTROLTYPEID				controlType = 0;
		vl::WString					providers;
		RECT						bounds = {};
		bool						offscreen = false;
	};

	struct TreeSnapshot
	{
		WindowIdentity				window;
		vl::vint					generation = 0;
		vl::collections::List<NodeData> nodes;
	};

	struct InspectionSnapshot
	{
		vl::vint					nodeKey = 0;
		vl::collections::List<PropertyData> properties;
		vl::collections::List<PATTERNID> patterns;
		vl::collections::List<vl::Ptr<ActionSectionData>> sections;
		vl::collections::List<ReferenceData> references;
	};

	class UiaSession : public vl::Object
	{
	public:
		vl::ComPtr<IUIAutomation>	automation;
		vl::ComPtr<IUIAutomationCacheRequest> cache;
		vl::ComPtr<IUIAutomationTreeWalker> walker;
		vl::ComPtr<IUnknown>			unsupported;
		vl::ComPtr<IUnknown>			mixed;
		WindowIdentity				window;
		vl::vint					nextKey = 1;
		vl::vint conversionDocumentKey = 0;
		vl::collections::Dictionary<vl::vint, vl::ComPtr<IUIAutomationElement>> elements;
		vl::collections::Dictionary<vl::WString, vl::vint> runtimeIds;
		vl::collections::Dictionary<vl::vint, vl::ComPtr<IUnknown>> opaqueObjects;
		vl::collections::Dictionary<vl::vint, RangeRecord> ranges;
		vl::collections::Dictionary<vl::vint, vl::ComPtr<IUIAutomationSynchronizedInputPattern>> listening;
		vl::collections::Dictionary<vl::vint, vl::WString> referenceNames;

										UiaSession(WindowIdentity target);
										~UiaSession();
		vl::vint					RetainElement(vl::ComPtr<IUIAutomationElement> element);
		vl::Ptr<ValueData>			Convert(const VARIANT& value);
		vl::Ptr<ValueData>			ConvertObject(IUnknown* value);
		vl::Ptr<TreeSnapshot>		ReadTree(vl::vint generation, const vl::Func<bool()>& canceled, const vl::Func<void(vl::vint)>& progress);
		vl::Ptr<InspectionSnapshot>	Inspect(vl::vint nodeKey);
		vl::Ptr<SetterSpec>			DescribeSetter(vl::vint nodeKey, PROPERTYID property);
		bool						SetProperty(vl::vint nodeKey, PROPERTYID property, const vl::WString& text);
		vl::Ptr<ValueData>			ConvertRange(IUIAutomationTextRange* value, vl::vint document, const vl::WString& name);
		vl::Ptr<ValueData>			ConvertRanges(IUIAutomationTextRangeArray* value, vl::vint document, const vl::WString& name);
		vl::Ptr<ValueData>			ConvertArray(SAFEARRAY* value, VARTYPE type);
		vl::collections::List<ReferenceData> GetReferences();
		vl::Ptr<ActionSectionData>	DescribeActions(vl::vint nodeKey, PATTERNID pattern);
		vl::Ptr<ActionSectionData>	DescribeRange(vl::vint rangeKey);
		ActionOutcome				Execute(vl::vint nodeKey, const ActionSpec& action, const vl::collections::List<ActionArgument>& arguments);
		ActionOutcome				ExecuteRange(const ActionSpec& action, const vl::collections::List<ActionArgument>& arguments);
		void						DiscardRanges();
	};

	extern vl::Ptr<ValueData>		IntegerValue(vl::vint64_t value);
	extern vl::Ptr<ValueData>		NumberValue(double value);
	extern vl::Ptr<ValueData>		BooleanValue(bool value);
	extern vl::Ptr<ValueData>		StringValue(const vl::WString& value);
	extern bool						ParseArgument(const ArgumentSpec& spec, const vl::WString& text, vl::vint reference, ActionArgument& value);

	class UiaWorker : public vl::Thread
	{
	protected:
		void						Run() override;
	public:
		vl::TaskQueue				queue;
		vl::Ptr<UiaSession>			session;
	};
}

#endif
