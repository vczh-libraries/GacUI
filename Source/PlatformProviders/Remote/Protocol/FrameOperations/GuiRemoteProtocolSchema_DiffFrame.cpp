#include "GuiRemoteProtocolSchema_FrameOperations.h"

namespace vl::presentation::remoteprotocol
{
	using namespace collections;

	vint CountDomIndex(Ptr<RenderingDom> root)
	{
		vint counter = 1;
		if (root->children)
		{
			for (auto child : *root->children.Obj())
			{
				counter += CountDomIndex(child);
			} 
		}
		return counter;
	}

	void BuildDomIndexInternal(Ptr<RenderingDom> root, DomIndex& index, vint& writing)
	{
		index[writing++] = { root->id,root };
		if (root->children)
		{
			for (auto child : *root->children.Obj())
			{
				BuildDomIndexInternal(child, index, writing);
			}
		}
	}

	void BuildDomIndex(Ptr<RenderingDom> root, DomIndex& index)
	{
		vint count = CountDomIndex(root);
		vint writing = 0;
		index.Resize(count);
		if (count > 0)
		{
			BuildDomIndexInternal(root, index, writing);
			SortLambda(&index[0], count, [](const DomIndexItem& a, const DomIndexItem& b)
			{
				return a.key <=> b.key;
			});
		}
	}

	void UpdateDomInplace(Ptr<RenderingDom> root, DomIndex& index, const RenderingDom_DiffsInOrder& diffs)
	{
	}
}