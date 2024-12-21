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

	void BuildDomIndexInternal(Ptr<RenderingDom> dom, vint parentId, DomIndex& index, vint& writing)
	{
		index[writing++] = { dom->id,parentId,dom };
		if (dom->children)
		{
			for (auto child : *dom->children.Obj())
			{
				BuildDomIndexInternal(child, dom->id, index, writing);
			}
		}
	}

	void SortDomIndex(DomIndex& index)
	{
		if (index.Count() > 0)
		{
			SortLambda(&index[0], index.Count(), [](const DomIndexItem& a, const DomIndexItem& b)
			{
				return a.id <=> b.id;
			});
		}
	}

	void BuildDomIndex(Ptr<RenderingDom> root, DomIndex& index)
	{
		vint count = CountDomIndex(root);
		vint writing = 0;
		index.Resize(count);
		if (count > 0)
		{
			BuildDomIndexInternal(root, -1, index, writing);
			SortDomIndex(index);
		}
	}

	void UpdateDomInplace(Ptr<RenderingDom> root, DomIndex& index, const RenderingDom_DiffsInOrder& diffs)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::UpdateDomInplace(Ptr<RenderingDom>, DomIndex&, const RenderingDom_DiffsInOrder&)#"
		CHECK_ERROR(root && root->id == -1, ERROR_MESSAGE_PREFIX L"Roots of a DOM must have ID -1.");

		vint createdCount = 0;

		// creating
		{
			vint readingFrom = 0;
			vint readingTo = 0;

			auto markCreated = [&]()
			{
				auto&& to = diffs.diffsInOrder->Get(readingTo++);
				CHECK_ERROR(to.diffType == RenderingDom_DiffType::Created, ERROR_MESSAGE_PREFIX L"Diff of unexisting node must have diffType == Created.");
				createdCount++;
			};

			while (diffs.diffsInOrder && readingTo < diffs.diffsInOrder->Count())
			{
				if (readingFrom < index.Count())
				{
					auto&& from = index[readingFrom];
					auto&& to = diffs.diffsInOrder->Get(readingTo);
					if (from.id < to.id)
					{
						// Nothing happened to this DOM node
						readingFrom++;
					}
					else if (from.id > to.id)
					{
						markCreated();
					}
					else
					{
						// Modified will be delayed and processed together with Created 
						readingFrom++;
						readingTo++;
						CHECK_ERROR(to.diffType != RenderingDom_DiffType::Created, ERROR_MESSAGE_PREFIX L"Diff of existing node must have diffType != Created.");
					}
				}
				else
				{
					markCreated();
				}
			}
		}

		{
			vint writing = index.Count();
			index.Resize(index.Count() + createdCount);
			if (diffs.diffsInOrder)
			{

				for (auto&& to : *diffs.diffsInOrder.Obj())
				{
					if (to.diffType == RenderingDom_DiffType::Created)
					{
						// parentId will be filled later
						auto dom = Ptr(new RenderingDom);
						dom->id = to.id;
						index[writing++] = { to.id,-1,dom };
					}
				}
			}
			SortDomIndex(index);
		}

		// modifying
		{
			vint readingFrom = 0;
			vint readingTo = 0;

			while (readingFrom < index.Count() && (diffs.diffsInOrder && readingTo < diffs.diffsInOrder->Count()))
			{
				bool hasFrom = readingFrom < index.Count();
				bool hasTo = diffs.diffsInOrder && readingTo < diffs.diffsInOrder->Count();

				auto&& from = index[readingFrom];
				auto&& to = diffs.diffsInOrder->Get(readingTo);
				if (from.id < to.id)
				{
					readingFrom++;
				}
				else if (from.id > to.id)
				{
					readingTo++;
				}
				else if (to.diffType != RenderingDom_DiffType::Deleted)
				{
					readingFrom++;
					readingTo++;

					if (to.content)
					{
						from.dom->content = to.content.Value();
					}

					if (to.children)
					{
						if (to.children->Count() == 0)
						{
							from.dom->children = nullptr;
						}
						else
						{
							from.dom->children = Ptr(new List<Ptr<RenderingDom>>);
							for (vint childId : *to.children.Obj())
							{
								// Binary search in index for childId
								vint start = 0;
								vint end = index.Count() - 1;
								bool found = false;
								while (start <= end)
								{
									vint mid = (start + end) / 2;
									vint midId = index[mid].id;
									if (childId < midId)
									{
										end = mid - 1;
									}
									else if (childId > midId)
									{
										start = mid + 1;
									}
									else
									{
										// Fill parentId of the new DOM node
										index[mid].parentId = from.id;
										from.dom->children->Add(index[mid].dom);
										found = true;
										break;
									}
								}
								CHECK_ERROR(found, ERROR_MESSAGE_PREFIX L"Unknown DOM id in diff.");
							}
						}
					}
				}
			}
		}

		// deleting
		{
			vint readingFrom = 0;
			vint readingTo = 0;
			List<vint> deleteIndices;

			while (diffs.diffsInOrder && readingTo < diffs.diffsInOrder->Count())
			{
				if (readingFrom < index.Count())
				{
					auto&& from = index[readingFrom];
					auto&& to = diffs.diffsInOrder->Get(readingTo);
					if (from.id < to.id)
					{
						readingFrom++;
					}
					else if (from.id > to.id)
					{
						readingTo++;
					}
					else
					{
						if (to.diffType == RenderingDom_DiffType::Deleted)
						{
							deleteIndices.Add(readingFrom);
						}
						readingFrom++;
						readingTo++;
					}
				}
				else
				{
					CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Nodes to be deleted must should appear in the index before modification");
				}
			}

			vint reading = 0;
			vint writing = 0;
			vint testing = 0;

			while (reading < index.Count())
			{
				if (testing < deleteIndices.Count() && deleteIndices[testing] == reading)
				{
					// A node to delete is found, mark and skip
					testing++;
					reading++;
				}
				else
				{
					if (reading != writing)
					{
						// Compact index by removing deleted entries
						index[writing] = index[reading];
					}
					reading++;
					writing++;
				}
			}
			index.Resize(index.Count() - deleteIndices.Count());
		}
#undef ERROR_MESSAGE_PREFIX
	}

	void DiffDom(Ptr<RenderingDom> domFrom, DomIndex& indexFrom, Ptr<RenderingDom> domTo, DomIndex& indexTo, RenderingDom_DiffsInOrder& diffs)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::DiffDom(Ptr<RenderingDom>, DomIndex&, Ptr<RenderingDom>, DomIndex&, RenderingDom_DiffsInOrder&)#"
		CHECK_ERROR(domFrom && domTo && domFrom->id == domTo->id, ERROR_MESSAGE_PREFIX L"Roots of two DOMs tree must have the same ID.");
		diffs.diffsInOrder = Ptr(new List<RenderingDom_Diff>);

		vint readingFrom = 0;
		vint readingTo = 0;

		auto pushDeleted = [&]()
		{
			auto&& dom = indexFrom[readingFrom++].dom;
			RenderingDom_Diff diff;

			diff.id = dom->id;
			diff.diffType = RenderingDom_DiffType::Deleted;
			diffs.diffsInOrder->Add(diff);
		};

		auto pushCreated = [&]()
		{
			auto&& dom = indexTo[readingTo++].dom;
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
			auto&& domFrom = indexFrom[readingFrom++].dom;
			auto&& domTo = indexTo[readingTo++].dom;
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
				if (indexFrom[readingFrom].id < indexTo[readingTo].id)
				{
					pushDeleted();
				}
				else if (indexFrom[readingFrom].id > indexTo[readingTo].id)
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