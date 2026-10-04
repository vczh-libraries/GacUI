#ifndef VCZH_UIALIST_CLIVIEWMODEL
#define VCZH_UIALIST_CLIVIEWMODEL

#include "UiaListViewModel.h"

namespace uialist::cli
{
	using Json = vl::Ptr<vl::glr::json::JsonNode>;

	class CommandFailure : public vl::Exception
	{
	public:
		vl::WString code;
		CommandFailure(const vl::WString& code, const vl::WString& message);
	};

	struct Reference
	{
		native::ValueKind kind = native::ValueKind::Element;
		vl::vint key = 0, generation = 0, document = 0;
	};

	class CliViewModel : public vl::Object
	{
	public:
		UiaListViewModel& root;
		vl::glr::json::Parser parser;
		vl::WString session, command, target, phase;
		vl::vint nextId = 0;
		vl::collections::Dictionary<vl::WString, Reference> references;
		vl::collections::Dictionary<vl::collections::Pair<vl::vint, vl::vint>, vl::WString> currentIds;
		vl::vint currentIdGeneration = -1;
		vl::collections::List<native::ReferenceData> nativeReferences;
		vl::collections::Dictionary<vl::WString, vl::Ptr<ProcessNodeViewModel>> windows;
		vl::Ptr<native::InspectionSnapshot> inspection;
		vl::collections::Dictionary<vl::vint, vl::Ptr<native::ActionSectionData>> ranges;
		vl::Func<void(const vl::WString&, bool, bool)> output;
		vl::Func<void()> published;
		bool busy = false;
		vl::Ptr<vl::EventHandler> publicationHandler;

		CliViewModel(UiaListViewModel& root);
		~CliViewModel();
		void Execute(const vl::WString& line);
		void Dispatch(const vl::WString& verb, const vl::WString& id, vl::Ptr<vl::glr::json::JsonObject> arguments);
		void Finish(Json result, bool exit = false);
		void Fail(const vl::WString& code, const vl::WString& message, const vl::WString& operation = {}, HRESULT hresult = S_OK, bool fatal = false);
		void Inspect(vl::vint key, vl::Func<void()> completion);
		void Refresh(vl::Func<void()> completion);
		void Native(const vl::WString& operation, vl::Func<void(native::UiaSession&)> task);
		void PublishReferences(const vl::collections::List<native::ReferenceData>& values);
		vl::WString Id(native::ValueKind kind, vl::vint key, vl::vint document = 0);
		Reference Resolve(const vl::WString& id, native::ValueKind kind);
		vl::Ptr<ProcessNodeViewModel> ResolveWindow(const vl::WString& id);
		Json Value(vl::Ptr<native::ValueData> value);
		Json Property(const native::PropertyData& property);
		Json Section(vl::Ptr<native::ActionSectionData> section, bool catalog = false);
		Json Inspection();
		Json References();
		Json Tree();
		Json Processes();
		Json Preview();
		Json Help(const vl::WString& name);
		vl::collections::List<native::ActionArgument> Arguments(const native::ActionSpec& spec, vl::Ptr<vl::glr::json::JsonObject> args);
		vl::Ptr<native::ValueData> InputValue(Json value, VARTYPE type, bool elementArray);
		void Run(Reference reference, const vl::WString& name, vl::Ptr<vl::glr::json::JsonObject> args);
	};

	extern Json String(const vl::WString& value);
	extern Json Number(double value);
	extern Json Boolean(bool value);
	extern Json Null();
	extern Json JsonMap(std::initializer_list<vl::collections::Pair<vl::WString, Json>> fields);
	extern vl::WString OperationName(const native::ActionSpec& spec);
}

#endif
