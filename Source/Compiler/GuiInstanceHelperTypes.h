/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Helper Types

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCEHELPERTYPES
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCEHELPERTYPES

#include "../Resources/GuiResource.h"

#if defined(__APPLE__) || defined(__APPLE_CC__)

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::templates;
using namespace vl::presentation::theme;

#endif

namespace vl
{
	namespace presentation
	{

/***********************************************************************
Helper Types
***********************************************************************/

		namespace helper_types
		{
			struct SiteValue
			{
				vint			row;
				vint			column;
				vint			rowSpan;
				vint			columnSpan;

				SiteValue() :row(0), column(0), rowSpan(1), columnSpan(1){}
			};

			enum class ListViewViewType
			{
				BigIcon,
				SmallIcon,
				List,
				Tile,
				Information,
				Detail,
			};
		}
	}

#ifndef VCZH_DEBUG_NO_REFLECTION

	namespace reflection
	{
		namespace description
		{

/***********************************************************************
Type List
***********************************************************************/

#define GUIREFLECTIONHELPERTYPES_TYPELIST(F)\
			F(presentation::helper_types::SiteValue)\
			F(presentation::helper_types::ListViewViewType)\

			GUIREFLECTIONHELPERTYPES_TYPELIST(DECL_TYPE_INFO)
		}
	}

#endif
}

#endif