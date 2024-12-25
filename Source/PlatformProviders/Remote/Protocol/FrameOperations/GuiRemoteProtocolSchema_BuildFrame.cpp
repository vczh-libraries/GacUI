#include "GuiRemoteProtocolSchema_FrameOperations.h"

namespace vl::presentation::remoteprotocol
{
	vint RenderingDomBuilder::GetCurrentBoundary()
	{
		if (domBoundaries.Count() > 0)
		{
			return domBoundaries[domBoundaries.Count() - 1];
		}
		else
		{
			return 0;
		}
	}

	vint RenderingDomBuilder::Push(RenderingResultRef ref)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::RenderingDomBuilder::Push(RenderingResultRef)#"
		CHECK_ERROR(ref, ERROR_MESSAGE_PREFIX L"Cannot push a null dom object.");
		vint index = domStack.Add(ref);
		if (!domCurrent->children) domCurrent->children = Ptr(new RenderingResultRefList);
		domCurrent->children->Add(ref);
		domCurrent = ref;
		return index;
#undef ERROR_MESSAGE_PREFIX
	}

	void RenderingDomBuilder::PopTo(vint index)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::RenderingDomBuilder::PopTo(vint)#"
		if (index == domStack.Count() - 1) return;
		CHECK_ERROR(0 <= index && index < domStack.Count(), ERROR_MESSAGE_PREFIX L"Cannot pop to an invalid position.");
		CHECK_ERROR(index >= GetCurrentBoundary(), ERROR_MESSAGE_PREFIX L"Cannot pop across a boundary.");
		while (domStack.Count() - 1 > index)
		{
			domStack.RemoveAt(domStack.Count() - 1);
		}
		domCurrent = domStack[index];
#undef ERROR_MESSAGE_PREFIX
	}

	void RenderingDomBuilder::Pop()
	{
		PopTo(domStack.Count() - 2);
	}

	void RenderingDomBuilder::PopBoundary()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::RenderingDomBuilder::PopBoundary()#"
		CHECK_ERROR(domBoundaries.Count() > 0, ERROR_MESSAGE_PREFIX L"Cannot pop a boundary when none is in the stack.");
		auto boundaryIndex = domBoundaries.Count() - 1;
		auto boundary = domBoundaries[boundaryIndex];
		domBoundaries.RemoveAt(boundaryIndex);
		PopTo(boundary - 1);
#undef ERROR_MESSAGE_PREFIX
	}


	template<typename TCallback>
	void RenderingDomBuilder::PrepareParentFromCommand(Rect commandBounds, Rect commandValidArea, vint newDomId, TCallback&& calculateValidAreaFromDom)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::RenderingDomBuilder::PrepareParentFromCommand(Rect, Rect, vint, auto&&)#"
		vint min = GetCurrentBoundary();
		bool found = false;
		if (commandValidArea.Contains(commandBounds))
		{
			// if the command is not clipped
			for (vint i = domStack.Count() - 1; i >= min; i--)
			{
				if (domStack[i]->content.validArea.Contains(commandBounds) || i == 0)
				{
					// find the deepest node that could contain the command
					PopTo(i);
					found = true;
					break;
				}
			}
		}
		else
		{
			// otherwise, a parent node causing such clipping should be found or created
			for (vint i = domStack.Count() - 1; i >= min; i--)
			{
				auto domValidArea = calculateValidAreaFromDom(domStack[i]);
				if (domValidArea == commandValidArea)
				{
					// if there is a node who clips command's bound to its valid area
					// that is the parent node of the command
					PopTo(i);
					found = true;
					break;
				}
				else if (domValidArea.Contains(commandValidArea) || i == 0)
				{
					// otherwise find a deepest node who could visually contain the command
					// create a virtual node to satisfy the clipper
					PopTo(i);
					auto parent = Ptr(new RenderingDom);
					parent->id = newDomId;
					parent->content.bounds = commandValidArea;
					parent->content.validArea = commandValidArea;
					Push(parent);
					found = true;
					break;
				}
			}
		}

		// if the new boundary could not fit in the current boundary
		// there must be something wrong
		CHECK_ERROR(found, ERROR_MESSAGE_PREFIX L"Incorrect valid area of dom.");
#undef ERROR_MESSAGE_PREFIX
	}

	void RenderingDomBuilder::RequestRendererBeginRendering()
	{
		domStack.Clear();
		domBoundaries.Clear();
		domRoot = Ptr(new RenderingDom);
		domRoot->id = 0;
		domCurrent = domRoot;
		domStack.Add(domRoot);
	}

	void RenderingDomBuilder::RequestRendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments)
	{
		// a new boundary should be a new node covering existing nodes
		// the valid area of boundary is clipped by its bounds
		// so the valid area to compare from its potential parent dom needs to clipped by its bounds
		PrepareParentFromCommand(
			arguments.bounds,
			arguments.areaClippedBySelf,
			(arguments.id << 2) + 3,
			[&](auto&& dom) { return dom->content.validArea.Intersect(arguments.bounds); }
			);

		auto dom = Ptr(new RenderingDom);
		dom->id = (arguments.id << 2) + 2;
		dom->content.hitTestResult = arguments.hitTestResult;
		dom->content.cursor = arguments.cursor;
		dom->content.bounds = arguments.bounds;
		dom->content.validArea = arguments.areaClippedBySelf;
		domBoundaries.Add(Push(dom));
	}

	void RenderingDomBuilder::RequestRendererEndBoundary()
	{
		PopBoundary();
	}

	void RenderingDomBuilder::RequestRendererRenderElement(const remoteprotocol::ElementRendering& arguments)
	{
		// a new element should be a new node covering existing nodes
		// the valid area of boundary is clipped by its parent
		// so the valid area to compare from its potential parent dom is its valid area
		PrepareParentFromCommand(
			arguments.bounds,
			arguments.areaClippedByParent,
			(arguments.id << 2) + 1,
			[&](auto&& dom) { return dom->content.validArea; }
			);

		auto dom = Ptr(new RenderingDom);
		dom->id = (arguments.id << 2) + 0;
		dom->content.element = arguments.id;
		dom->content.bounds = arguments.bounds;
		dom->content.validArea = arguments.bounds.Intersect(arguments.areaClippedByParent);
		Push(dom);
	}

	Ptr<RenderingDom> RenderingDomBuilder::RequestRendererEndRendering()
	{
		return domRoot;
	}
}