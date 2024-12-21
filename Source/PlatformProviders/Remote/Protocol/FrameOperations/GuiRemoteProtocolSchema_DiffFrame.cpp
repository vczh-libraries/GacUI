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

	void DiffDom(Ptr<RenderingDom> domFrom, DomIndex& indexFrom, Ptr<RenderingDom> domTo, DomIndex& indexTo, RenderingDom_DiffsInOrder& diffs)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::DiffDom(Ptr<RenderingDom>, DomIndex&, Ptr<RenderingDom>, DomIndex&, RenderingDom_DiffsInOrder&)#"
		CHECK_ERROR(domFrom && domTo && domFrom->id == domTo->id, ERROR_MESSAGE_PREFIX L"Roots if two DOM tree must have the same ID.");
		diffs.diffsInOrder = Ptr(new List<RenderingDom_Diff>);

		vint readingFrom = 0;
		vint readingTo = 0;

		auto pushDeleted = [&]()
		{
			auto&& dom = indexFrom[readingFrom++].value;
			RenderingDom_Diff diff;

			diff.id = dom->id;
			diff.diffType = RenderingDom_DiffType::Deleted;
			diffs.diffsInOrder->Add(diff);
		};

		auto pushCreated = [&]()
		{
			auto&& dom = indexTo[readingTo++].value;
			RenderingDom_Diff diff;
			diff.id = dom->id;
			diff.diffType = RenderingDom_DiffType::Created;

			diff.content = dom->content;
			if (dom->children && dom->children->Count() > 0)
			{
				diff.children = Ptr(new List<vint>);
				CopyFrom(
					*diff.children.Obj(),
					From(*dom->children.Obj())
						.Select([](Ptr<RenderingDom> child) { return child->id; })
					);
			}
			diffs.diffsInOrder->Add(diff);
		};

		auto pushModified = [&]()
		{
			auto&& domFrom = indexFrom[readingFrom++].value;
			auto&& domTo = indexTo[readingTo++].value;
			RenderingDom_Diff diff;
			diff.id = domFrom->id;
			diff.diffType = RenderingDom_DiffType::Modified;

			if (
				domFrom->content.hitTestResult != domTo->content.hitTestResult ||
				domFrom->content.cursor != domTo->content.cursor ||
				domFrom->content.element != domTo->content.element ||
				domFrom->content.bounds != domTo->content.bounds ||
				domFrom->content.validArea != domTo->content.validArea
				)
			{
				diff.content = domTo->content;
			}

			bool fromHasChild = domFrom->children && domFrom->children->Count() > 0;
			bool toHasChild = domTo->children && domTo->children->Count() > 0;
			bool childDifferent = false;

			if (fromHasChild != toHasChild)
			{
				childDifferent = true;
			}
			else if (fromHasChild && toHasChild)
			{
				auto fromIds = From(*domFrom->children.Obj())
					.Select([](Ptr<RenderingDom> child) { return child->id; });
				auto toIds = From(*domTo->children.Obj())
					.Select([](Ptr<RenderingDom> child) { return child->id; });
				childDifferent = CompareEnumerable(fromIds, toIds) != 0;
			}

			if (childDifferent)
			{
				diff.children = Ptr(new List<vint>);
				if (toHasChild)
				{
					CopyFrom(
						*diff.children.Obj(),
						From(*domTo->children.Obj())
							.Select([](Ptr<RenderingDom> child) { return child->id; })
						);
				}
			}

			if (diff.content || diff.children)
			{
				diffs.diffsInOrder->Add(diff);
			}
		};

		while (true)
		{
			if (readingFrom < indexFrom.Count() || readingTo < indexTo.Count())
			{
				if (indexFrom[readingFrom].key < indexTo[readingTo].key)
				{
					pushDeleted();
				}
				else if (indexFrom[readingFrom].key > indexTo[readingTo].key)
				{
					pushCreated();
				}
				else
				{
					pushModified();
				}
			}
			else if (readingFrom < indexFrom.Count())
			{
				pushDeleted();
			}
			else if (readingTo < indexTo.Count())
			{
				pushCreated();
			}
			else
			{
				break;
			}
		}

#undef ERROR_MESSAGE_PREFIX
	}
}