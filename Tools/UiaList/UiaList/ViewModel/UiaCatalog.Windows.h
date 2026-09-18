#ifndef VCZH_UIALIST_CATALOG_WINDOWS
#define VCZH_UIALIST_CATALOG_WINDOWS

#include "ProcessDiscovery.Windows.h"
#include <UIAutomation.h>

namespace uialist::native
{
	class UiaFailure : public vl::Exception
	{
	public:
		HRESULT						result;
		UiaFailure(HRESULT result, const vl::WString& operation);
		bool IsUnavailable()const;
		bool IsExpected()const;
	};

	struct IdDescriptor
	{
		LONG						id;
		const wchar_t*				name;
		VARTYPE expectedType = VT_EMPTY;
		bool elementArray = false;
	};

	struct PatternDescriptor
	{
		PATTERNID					id;
		const IID*					iid;
		const wchar_t*				client;
		const wchar_t*				provider;
	};

	extern const IdDescriptor		PropertyCatalog[];
	extern const vl::vint			PropertyCatalogCount;
	extern const IdDescriptor		AttributeCatalog[];
	extern const vl::vint			AttributeCatalogCount;
	extern const IdDescriptor		ControlTypeCatalog[];
	extern const vl::vint			ControlTypeCatalogCount;
	extern const IdDescriptor		MetadataCatalog[];
	extern const vl::vint			MetadataCatalogCount;
	extern const PatternDescriptor	PatternCatalog[];
	extern const vl::vint			PatternCatalogCount;
	extern void						CheckUia(HRESULT result, const vl::WString& operation);
	extern vl::WString				Hex(vl::vuint64_t value);
	extern vl::WString				IdName(const IdDescriptor* catalog, vl::vint count, LONG id);
	extern vl::WString				EnumerationName(const vl::WString& field, LONG value);
	extern vl::ComPtr<IUnknown>		AcquireCatalogPattern(IUIAutomationElement* element, PATTERNID id);

	template<typename T>
	vl::ComPtr<T> PatternInterface(IUnknown* pattern)
	{
		T* result = nullptr;
		CheckUia(pattern->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&result)), L"Acquired pattern interface contract");
		return result;
	}

	template<typename T>
	vl::ComPtr<T> QueryInterface(IUnknown* object)
	{
		if (!object) return nullptr;
		T* result = nullptr;
		auto hr = object->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&result));
		if (hr == E_NOINTERFACE) return nullptr;
		CheckUia(hr, L"QueryInterface");
		return result;
	}

	template<typename T>
	vl::ComPtr<T> AcquirePattern(IUIAutomationElement* element, PATTERNID id)
	{
		T* result = nullptr;
		auto hr = element->GetCurrentPatternAs(id, __uuidof(T), reinterpret_cast<void**>(&result));
		if (hr == E_NOINTERFACE || hr == UIA_E_NOTSUPPORTED) return nullptr;
		CheckUia(hr, L"GetCurrentPatternAs(" + vl::itow(id) + L")");
		return result;
	}
}

#endif
