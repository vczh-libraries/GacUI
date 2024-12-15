#include "GuiRemoteProtocolSchema_FrameOperations.h"

namespace vl::presentation::remoteprotocol
{
	using namespace collections;

	Ptr<RenderingDom> CopyDom(Ptr<RenderingDom> root)
	{
		auto newRoot = Ptr(new RenderingDom);
		newRoot->id = root->id;
		newRoot->content = root->content;
		if (root->children)
		{
			newRoot->children = Ptr(new List<Ptr<RenderingDom>>);
			for (auto child : *root->children.Obj())
			{
				newRoot->children->Add(CopyDom(child));
			}
		}
		return newRoot;
	}
}