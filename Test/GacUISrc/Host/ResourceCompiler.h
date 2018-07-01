#ifndef GACUISRC_RESOURCECOMPILER
#define GACUISRC_RESOURCECOMPILER

#define GAC_HEADER_USE_NAMESPACE
#include "..\..\..\Source\GacUI.h"
#include "..\..\..\Source\Compiler\GuiCppGen.h"
#ifndef VCZH_DEBUG_NO_REFLECTION

#include "..\..\..\Source\Compiler\GuiInstanceLoader.h"
#include "..\..\..\Source\Reflection\GuiInstanceCompiledWorkflow.h"
#include "..\..\..\Source\Compiler\WorkflowCodegen\GuiInstanceLoader_WorkflowCodegen.h"

using namespace vl;
using namespace vl::workflow;
using namespace vl::filesystem;

class DebugCallback : public Object, public IGuiResourcePrecompileCallback, public IWfCompilerCallback
{
public:
	vint lastPassIndex = -1;

	void OnLoadEnvironment()override;
	void OnInitialize(analyzer::WfLexicalScopeManager* manager)override;
	void OnValidateModule(Ptr<WfModule> module)override;
	void OnGenerateMetadata()override;
	void OnGenerateCode(Ptr<WfModule> module)override;
	void OnGenerateDebugInfo()override;
	IWfCompilerCallback* GetCompilerCallback()override;

	void PrintPassName(vint passIndex);
	void OnPerPass(vint passIndex)override;
	void OnPerResource(vint passIndex, Ptr<GuiResourceItem> resource)override;
};

extern FilePath CompileResources(
	const WString& name,
	collections::List<WString>& dependencies,
	const WString& resourcePath,
	const WString& outputBinaryFolder,
	const WString& outputCppFolder,
	bool compressResource
	);

extern void LoadResource(FilePath binaryPath);
extern void DumpComposition(presentation::compositions::GuiGraphicsComposition* composition, stream::TextWriter& writer);

#endif

#endif