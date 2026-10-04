#include "GacUI.Windows.h"
#undef GetRoleText
#include "../../UiaList/ViewModel/CliViewModel.h"
#include <limits>
#include <cstdio>

using namespace vl;
using namespace vl::glr;
using namespace uialist;
using namespace uialist::native;

void GuiMain()
{
	UiaListViewModel root(Strings::Get(Locale(L"en-US")), L"en-US");
	cli::CliViewModel model(root);
	auto values = Ptr(new ValueData);
	values->kind = ValueKind::Array;
	values->type = VT_ARRAY | VT_VARIANT;
	values->dimensions.Add({ -3, 8 });
	for (vint i = 0; i <= static_cast<vint>(ValueKind::Opaque); i++)
	{
		auto value = Ptr(new ValueData);
		value->kind = static_cast<ValueKind>(i);
		value->type = VT_VARIANT;
		value->signedValue = -9007199254740993LL;
		value->unsignedValue = (std::numeric_limits<vuint64_t>::max)();
		value->realValue = std::numeric_limits<double>::quiet_NaN();
		const wchar_t text[] = { L'A', 0, 1, 11, 31, 0x4e2d, L'Z' };
		value->text = WString::CopyFrom(text, 7);
		if (value->kind == ValueKind::Boolean) value->signedValue = 0;
		if (value->kind == ValueKind::Array)
		{
			value->dimensions.Add({ 2, 3 });
			for (double number : { std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() })
			{
				auto item = Ptr(new ValueData);
				item->kind = ValueKind::Real;
				item->realValue = number;
				value->items.Add(item);
			}
		}
		values->items.Add(value);
	}
	auto output = wtou8(json::JsonToString(model.Value(values)) + L"\n");
	fwrite(output.Buffer(), 1, output.Length(), stdout);
}

int wmain()
{
	SetupWindowsDirect2DRenderer();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}
