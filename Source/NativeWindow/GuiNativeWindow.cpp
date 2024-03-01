#include "GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		const NativeWindowFrameConfig NativeWindowFrameConfig::Default = {};

/***********************************************************************
INativeWindowListener
***********************************************************************/

		INativeWindowListener::HitTestResult INativeWindowListener::HitTest(NativePoint location)
		{
			return INativeWindowListener::NoDecision;
		}

		void INativeWindowListener::Moving(NativeRect& bounds, bool fixSizeOnly, bool draggingBorder)
		{
		}

		void INativeWindowListener::Moved()
		{
		}

		void INativeWindowListener::DpiChanged(bool preparing)
		{
		}

		void INativeWindowListener::Enabled()
		{
		}

		void INativeWindowListener::Disabled()
		{
		}

		void INativeWindowListener::GotFocus()
		{
		}

		void INativeWindowListener::LostFocus()
		{
		}

		void INativeWindowListener::RenderingAsActivated()
		{
		}

		void INativeWindowListener::RenderingAsDeactivated()
		{
		}

		void INativeWindowListener::Opened()
		{
		}

		void INativeWindowListener::BeforeClosing(bool& cancel)
		{
		}

		void INativeWindowListener::AfterClosing()
		{
		}

		void INativeWindowListener::Closed()
		{
		}

		void INativeWindowListener::Paint()
		{
		}

		void INativeWindowListener::Destroying()
		{
		}

		void INativeWindowListener::Destroyed()
		{
		}

		void INativeWindowListener::LeftButtonDown(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::LeftButtonUp(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::LeftButtonDoubleClick(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::RightButtonDown(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::RightButtonUp(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::RightButtonDoubleClick(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::MiddleButtonDown(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::MiddleButtonUp(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::MiddleButtonDoubleClick(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::HorizontalWheel(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::VerticalWheel(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::MouseMoving(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::MouseEntered()
		{
		}

		void INativeWindowListener::MouseLeaved()
		{
		}

		void INativeWindowListener::KeyDown(const NativeWindowKeyInfo& info)
		{
		}

		void INativeWindowListener::KeyUp(const NativeWindowKeyInfo& info)
		{
		}

		void INativeWindowListener::Char(const NativeWindowCharInfo& info)
		{
		}

		bool INativeWindowListener::NeedRefresh()
		{
			return false;
		}

		void INativeWindowListener::ForceRefresh(bool handleFailure, bool& updated, bool& failureByResized, bool& failureByLostDevice)
		{
		}

		void INativeWindowListener::AssignFrameConfig(const NativeWindowFrameConfig& config)
		{
		}

/***********************************************************************
INativeControllerListener
***********************************************************************/

		void INativeControllerListener::GlobalTimer()
		{
		}

		void INativeControllerListener::ClipboardUpdated()
		{
		}

		void INativeControllerListener::GlobalShortcutKeyActivated(vint id)
		{
		}

		void INativeControllerListener::NativeWindowCreated(INativeWindow* window)
		{
		}

		void INativeControllerListener::NativeWindowDestroying(INativeWindow* window)
		{
		}

/***********************************************************************
Native Window Provider
***********************************************************************/

		class SubstitutableController;
		INativeController* nativeController = nullptr;
		SubstitutableController* substitutableController = nullptr;

		class SubstitutableController
			: public Object
			, public INativeController
			, public INativeServiceSubstitution
		{
		public:
			WString GetExecutablePath() override
			{
				return nativeController->GetExecutablePath();
			}

			// Substitutable Service

			template<typename T, T* (INativeController::* Getter)()>
			struct Substitution
			{
				T*					service = nullptr;
				bool				optional = false;
				bool				requested = false;
				bool				unsubstituted = false;

				void Substitute(T* _service, bool _optional)
				{
					CHECK_ERROR(
						!requested,
						L"The service cannot be substituted because it has been used."
						);
					service = _service;
					optional = _optional;
				}

				void Unsubstitute(T* _service)
				{
					if (service == _service)
					{
						if (requested)
						{
							unsubstituted = true;
						}
						service = nullptr;
					}
				}

				T* GetService()
				{
					CHECK_ERROR(
						!unsubstituted,
						L"The service cannot be used because it has been unsubstituted."
						);
					requested = true;

					auto nativeService = (nativeController->*Getter)();
					if (service && (!nativeService || !optional))
					{
						return service;
					}

					CHECK_ERROR(
						nativeService != nullptr,
						L"Required service does not exist."
						);
					return nativeService;
				}
			};

			// Unsubstitutable Service

			template<typename T, T* (INativeController::* Getter)()>
			T* GetUnsubstitutableService()
			{
				auto nativeService = (nativeController->*Getter)();
				CHECK_ERROR(
					nativeService != nullptr,
					L"Required service does not exist."
				);
				return nativeService;
			}

			// INativeServiceSubstitution and INativeController

#define GET_SUBSTITUTABLE_SERVICE(NAME)														\
			Substitution<																	\
				INative##NAME##Service,														\
				&INativeController::NAME##Service											\
				> substituted##NAME;														\
																							\
			void Substitute(INative##NAME##Service* service, bool optional) override		\
			{																				\
				substituted##NAME.Substitute(service, optional);							\
			}																				\
																							\
			void Unsubstitute(INative##NAME##Service* service) override						\
			{																				\
				substituted##NAME.Unsubstitute(service);									\
			}																				\
																							\
			INative##NAME##Service* NAME##Service() override								\
			{																				\
				return substituted##NAME.GetService();										\
			}																				\

			GUI_SUBSTITUTABLE_SERVICES(GET_SUBSTITUTABLE_SERVICE)
#undef GET_SUBSTITUTABLE_SERVICE

#define GET_UNSUBSTITUTABLE_SERVICE(NAME)													\
			INative##NAME##Service* NAME##Service() override								\
			{																				\
				return GetUnsubstitutableService<											\
					INative##NAME##Service,													\
					&INativeController::NAME##Service										\
					>();																	\
			}																				\

			GUI_UNSUBSTITUTABLE_SERVICES(GET_UNSUBSTITUTABLE_SERVICE)
#undef GET_UNSUBSTITUTABLE_SERVICE
		};

		INativeServiceSubstitution* GetNativeServiceSubstitution()
		{
			return substitutableController;
		}

		INativeController* GetCurrentController()
		{
			return substitutableController;
		}

		void SetNativeController(INativeController* controller)
		{
			nativeController = controller;

			if (nativeController)
			{
				if (!substitutableController)
				{
					substitutableController = new SubstitutableController();
				}
			}
			else
			{
				if (substitutableController)
				{
					delete substitutableController;
					substitutableController = 0;
				}
			}
		}

/***********************************************************************
Helper Functions
***********************************************************************/

		NativeImageFrameBase::NativeImageFrameBase()
		{
		}

		NativeImageFrameBase::~NativeImageFrameBase()
		{
			// TODO: (enumerable) foreach
			for (vint i = 0; i < caches.Count(); i++)
			{
				caches.Values().Get(i)->OnDetach(this);
			}
		}

		bool NativeImageFrameBase::SetCache(void* key, Ptr<INativeImageFrameCache> cache)
		{
			vint index = caches.Keys().IndexOf(key);
			if (index != -1)
			{
				return false;
			}
			caches.Add(key, cache);
			cache->OnAttach(this);
			return true;
		}

		Ptr<INativeImageFrameCache> NativeImageFrameBase::GetCache(void* key)
		{
			vint index = caches.Keys().IndexOf(key);
			return index == -1 ? nullptr : caches.Values().Get(index);
		}

		Ptr<INativeImageFrameCache> NativeImageFrameBase::RemoveCache(void* key)
		{
			vint index = caches.Keys().IndexOf(key);
			if (index == -1)
			{
				return 0;
			}
			Ptr<INativeImageFrameCache> cache = caches.Values().Get(index);
			cache->OnDetach(this);
			caches.Remove(key);
			return cache;
		}

/***********************************************************************
Helper Functions
***********************************************************************/

		INativeCursor* GetCursorFromHitTest(INativeWindowListener::HitTestResult hitTestResult, INativeResourceService* resourceService)
		{
			switch (hitTestResult)
			{
			case INativeWindowListener::BorderLeft:
			case INativeWindowListener::BorderRight:
				return resourceService->GetSystemCursor(INativeCursor::SizeWE);
			case INativeWindowListener::BorderTop:
			case INativeWindowListener::BorderBottom:
				return resourceService->GetSystemCursor(INativeCursor::SizeNS);
			case INativeWindowListener::BorderLeftTop:
			case INativeWindowListener::BorderRightBottom:
				return resourceService->GetSystemCursor(INativeCursor::SizeNWSE);
			case INativeWindowListener::BorderRightTop:
			case INativeWindowListener::BorderLeftBottom:
				return resourceService->GetSystemCursor(INativeCursor::SizeNESW);
			default:
				return nullptr;
			}
		}
	}
}