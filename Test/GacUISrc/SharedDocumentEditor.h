#ifndef VCZH_DEMO_SHAREDDOCUMENTEDITOR
#define VCZH_DEMO_SHAREDDOCUMENTEDITOR

#ifdef GACUI_SKIN_DEVELOPMENT
#include "../../Source/GacUI.h"
#else
#include <GacUI.h>
#endif

namespace gacui_demo
{
	extern vl::Ptr<vl::presentation::DocumentModel> LoadDocument(const vl::WString& fileName);
	extern void SaveDocument(vl::Ptr<vl::presentation::DocumentModel> model, const vl::WString& fileName);
	extern void ExportDocumentRtf(vl::Ptr<vl::presentation::DocumentModel> model, const vl::WString& fileName);
	extern void ExportDocumentHtml(vl::Ptr<vl::presentation::DocumentModel> model, const vl::WString& fileName);
}

#endif
