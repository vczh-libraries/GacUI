/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection Helper

***********************************************************************/

#ifndef VCZH_PRESENTATION_GACUIREFLECTIONHELPER
#define VCZH_PRESENTATION_GACUIREFLECTIONHELPER

#include "Resources/GuiResourceManager.h"
#include "GraphicsElement/GuiGraphicsTextElement.h"
#include "GraphicsElement/GuiGraphicsDocumentElement.h"
#include "GraphicsComposition/IncludeAll.h"
#include "Application/GraphicsHost/GuiGraphicsHost.h"
#include "Controls/IncludeAll.h"
#include "Controls/Templates/GuiAnimation.h"
#include "Controls/Templates/GuiCommonTemplates.h"
#include "Controls/Templates/GuiThemeStyleFactory.h"
#include "Utilities/FakeServices/GuiFakeDialogServiceBase.h"

namespace vl
{
	namespace presentation
	{
		namespace helper_types
		{
			struct SiteValue
			{
				vint					row = 0;
				vint					column = 0;
				vint					rowSpan = 1;
				vint					columnSpan = 1;

				auto operator<=>(const SiteValue&) const = default;
			};

			class LocalizedStrings
			{
			public:
				static WString			FirstOrEmpty(const collections::LazyList<WString>& formats);
			};
		}
	}

	namespace reflection
	{
		namespace description
		{

/***********************************************************************
Serialization
***********************************************************************/

			template<>
			struct TypedValueSerializerProvider<presentation::Color>
			{
				static presentation::Color GetDefaultValue();
				static bool Serialize(const presentation::Color& input, WString& output);
				static bool Deserialize(const WString& input, presentation::Color& output);
			};

			template<>
			struct TypedValueSerializerProvider<presentation::DocumentFontSize>
			{
				static presentation::DocumentFontSize GetDefaultValue();
				static bool Serialize(const presentation::DocumentFontSize& input, WString& output);
				static bool Deserialize(const WString& input, presentation::DocumentFontSize& output);
			};

			template<>
			struct TypedValueSerializerProvider<presentation::GlobalStringKey>
			{
				static presentation::GlobalStringKey GetDefaultValue();
				static bool Serialize(const presentation::GlobalStringKey& input, WString& output);
				static bool Deserialize(const WString& input, presentation::GlobalStringKey& output);
			};

/***********************************************************************
External Functions
***********************************************************************/

			extern Ptr<presentation::INativeImage>							INativeImage_Constructor(const WString& path);
			extern presentation::INativeCursor*								INativeCursor_Constructor1();
			extern presentation::INativeCursor*								INativeCursor_Constructor2(presentation::INativeCursor::SystemCursorType type);

			template<typename T>
			Ptr<T> Element_Constructor()
			{
				return Ptr(T::Create());
			}
			extern presentation::elements::text::TextLines*					GuiColorizedTextElement_GetLines(presentation::elements::GuiColorizedTextElement* thisObject);

			extern void														GuiTableComposition_SetRows(presentation::compositions::GuiTableComposition* thisObject, vint value);
			extern void														GuiTableComposition_SetColumns(presentation::compositions::GuiTableComposition* thisObject, vint value);
			extern void														IGuiAltActionHost_CollectAltActions(presentation::compositions::IGuiAltActionHost* host, collections::List<presentation::compositions::IGuiAltAction*>& actions);
		}
	}
}

#endif