#ifndef VCZH_UIALIST_ACTIONCATALOG_WINDOWS
#define VCZH_UIALIST_ACTIONCATALOG_WINDOWS

#include "UiaSession.Windows.h"

namespace uialist::native
{
	extern vl::Ptr<ArgumentSpec> TextArgument(const wchar_t* name, const vl::WString& initial = {}, bool multiline = false);
	extern vl::Ptr<ArgumentSpec> NumberArgument(const wchar_t* name, double minimum, double maximum, double initial = 0, bool integer = false);
	extern vl::Ptr<ArgumentSpec> ChoiceArgument(const wchar_t* name, std::initializer_list<SetterChoice> choices);
	extern vl::Ptr<ArgumentSpec> ReferenceArgument(const wchar_t* name, bool range = false, bool nullable = false);
	extern vl::Ptr<ArgumentSpec> CatalogArgument(const wchar_t* name, const IdDescriptor* catalog, vl::vint count, bool allowZero = false);
	extern vl::Ptr<ActionSpec> AddAction(ActionSectionData& section, ActionCode code, const wchar_t* name, bool mutation, std::initializer_list<vl::Ptr<ArgumentSpec>> arguments = {}, bool enabled = true);
	extern void AddReadout(ActionSectionData& section, const wchar_t* name, vl::Ptr<ValueData> value);
	extern void ToVariant(UiaSession& session, const ValueData& value, VARIANT& result);
}

#endif
