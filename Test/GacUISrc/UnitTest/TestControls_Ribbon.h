#include "TestControls.h"

namespace gacui_unittest_template
{
	template<typename TLastCallback>
	void TestReactiveView(UnitTestRemoteProtocol* protocol, const WString& firstName, vint minWidth, vint maxWidth, vint step, TLastCallback&& lastCallback)
	{
		TEST_ASSERT((maxWidth - minWidth) % step == 0);
		vint frames = (maxWidth - minWidth) / step;

		for (vint i = 0; i < frames; i++)
		{
			vint currentWidth = maxWidth - i * step;
			vint nextWidth = currentWidth - step;
			auto name = L"Width [" + itow(currentWidth) + L"]";
			protocol->OnNextIdleFrame((i == 0 ? firstName : name), [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->SetClientSize({ nextWidth,window->GetClientSize().y });
			});
		}

		for (vint i = 0; i < frames; i++)
		{
			vint currentWidth = minWidth + i * step;
			vint nextWidth = currentWidth + step;
			auto name = L"Width [" + itow(currentWidth) + L"]";
			protocol->OnNextIdleFrame(name, [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->SetClientSize({ nextWidth,window->GetClientSize().y });
			});
		}

		{
			auto name = L"Width [" + itow(maxWidth) + L"]";
			protocol->OnNextIdleFrame(name, std::forward<TLastCallback&&>(lastCallback));
		}
	}
}