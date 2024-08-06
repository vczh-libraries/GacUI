/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_LOGGING
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_LOGGING

#include "GuiUnitTestProtocol_Rendering.h"

namespace vl::presentation::unittest
{

/***********************************************************************
UnitTestRemoteProtocol
***********************************************************************/
	
	template<typename TProtocol>
	class UnitTestRemoteProtocol_Logging : public TProtocol
	{
		using CommandList = UnitTestRenderingCommandList;
		using CommandListRef = UnitTestRenderingCommandListRef;
		using RenderingResultRef = Ptr<UnitTestRenderingDom>;
		using RenderingResultRefList = collections::List<RenderingResultRef>;
		using LoggedFrameList = collections::List<remoteprotocol::RenderingFrame>;
	protected:

		bool								everRendered = false;
		vint								candidateFrameId = 0;
		CommandListRef						candidateRenderingResult;

		RenderingResultRef TransformLastRenderingResult(CommandListRef commandListRef)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Logging<TProtocol>::TransformLastRenderingResult(CommandListRef)#"

			RenderingResultRefList domStack;
			collections::List<vint> domBoundaries;

			auto domRoot = Ptr(new UnitTestRenderingDom);
			auto domCurrent = domRoot;
			domStack.Add(domRoot);

			auto getCurrentBoundary = [&]() -> vint
			{
				if (domBoundaries.Count() > 0)
				{
					return domBoundaries[domBoundaries.Count() - 1];
				}
				else
				{
					return 0;
				}
			};

			auto push = [&](RenderingResultRef ref)
			{
				CHECK_ERROR(ref, ERROR_MESSAGE_PREFIX L"[push] Cannot push a null dom object.");
				vint index = domStack.Add(ref);
				if (!domCurrent->children) domCurrent->children = Ptr(new RenderingResultRefList);
				domCurrent->children->Add(ref);
				domCurrent = ref;
				return index;
			};

			auto popTo = [&](vint index)
			{
				if (index == domStack.Count() - 1) return;
				CHECK_ERROR(0 <= index && index < domStack.Count(), ERROR_MESSAGE_PREFIX L"[popTo] Cannot pop to an invalid position.");
				CHECK_ERROR(index >= getCurrentBoundary(), ERROR_MESSAGE_PREFIX L"[popTo] Cannot pop across a boundary.");
				while (domStack.Count() - 1 > index)
				{
					domStack.RemoveAt(domStack.Count() - 1);
				}
				domCurrent = domStack[index];
			};

			auto pop = [&]()
			{
				popTo(domStack.Count() - 2);
			};

			auto popBoundary = [&]()
			{
				CHECK_ERROR(domBoundaries.Count() > 0, ERROR_MESSAGE_PREFIX L"[popBoundary] Cannot pop a boundary when none is in the stack.");
				auto boundaryIndex = domBoundaries.Count() - 1;
				auto boundary = domBoundaries[boundaryIndex];
				domBoundaries.RemoveAt(boundaryIndex);
				popTo(boundary - 1);
			};

			auto prepareParentFromCommand = [&](
				Rect commandBounds,
				Rect commandValidArea,
				auto&& calculateValidAreaFromDom
				)
			{
				vint min = getCurrentBoundary();
				bool found = false;
				if (commandValidArea.Contains(commandBounds))
				{
					// if the command is not clipped
					for (vint i = domStack.Count() - 1; i >= min; i--)
					{
						if (domStack[i]->validArea.Contains(commandBounds) || i == 0)
						{
							// find the deepest node that could contain the command
							popTo(i);
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
							popTo(i);
							found = true;
							break;
						}
						else if (domValidArea.Contains(commandValidArea) || i == 0)
						{
							// otherwise find a deepest node who could visually contain the command
							// create a virtual node to satisfy the clipper
							popTo(i);
							auto parent = Ptr(new UnitTestRenderingDom);
							parent->bounds = commandValidArea;
							parent->validArea = commandValidArea;
							push(parent);
							found = true;
							break;
						}
					}
				}

				// if the new boundary could not fit in the current boundary
				// there must be something wrong
				CHECK_ERROR(found, ERROR_MESSAGE_PREFIX L"Incorrect valid area of dom.");
			};

			for (auto&& command : *commandListRef.Obj())
			{
				command.Apply(Overloading(
					[&](const remoteprotocol::RenderingCommand_BeginBoundary& command)
					{
						// a new boundary should be a new node covering existing nodes
						// the valid area of boundary is clipped by its bounds
						// so the valid area to compare from its potential parent dom needs to clipped by its bounds
						prepareParentFromCommand(
							command.boundary.bounds,
							command.boundary.areaClippedBySelf,
							[&](auto&& dom) { return dom->validArea.Intersect(command.boundary.bounds); }
							);

						auto dom = Ptr(new UnitTestRenderingDom);
						dom->hitTestResult = command.boundary.hitTestResult;
						dom->cursor = command.boundary.cursor;
						dom->bounds = command.boundary.bounds;
						dom->validArea = command.boundary.areaClippedBySelf;
						domBoundaries.Add(push(dom));
					},
					[&](const remoteprotocol::RenderingCommand_EndBoundary& command)
					{
						popBoundary();
					},
					[&](const remoteprotocol::RenderingCommand_Element& command)
					{
						// a new element should be a new node covering existing nodes
						// the valid area of boundary is clipped by its parent
						// so the valid area to compare from its potential parent dom is its valid area
						prepareParentFromCommand(
							command.rendering.bounds,
							command.rendering.areaClippedByParent,
							[&](auto&& dom) { return dom->validArea; }
							);

						auto dom = Ptr(new UnitTestRenderingDom);
						dom->element = command.element;
						dom->bounds = command.rendering.bounds;
						dom->validArea = command.rendering.bounds.Intersect(command.rendering.areaClippedByParent);
						push(dom);
					}));
			}

			return domRoot;
#undef ERROR_MESSAGE_PREFIX
		}

		bool LogRenderingResult()
		{
			if (this->lastRenderingCommands)
			{
				candidateFrameId = this->lastFrameId;
				candidateRenderingResult = this->lastRenderingCommands;
				this->lastRenderingCommands = {};
				everRendered = true;
			}
			else if (everRendered)
			{
				if (candidateRenderingResult)
				{
					auto descs = Ptr(new collections::Dictionary<vint, remoteprotocol::ElementDescVariant>);
					CopyFrom(*descs.Obj(), this->lastElementDescs);
					auto transformed = TransformLastRenderingResult(candidateRenderingResult);
					this->loggedTrace.frames->Add({
						candidateFrameId,
						{},
						this->sizingConfig,
						descs,
						candidateRenderingResult,
						transformed
						});
					candidateRenderingResult = {};
					return true;
				}
			}
			return false;
		}

	public:

		template<typename ...TArgs>
		UnitTestRemoteProtocol_Logging(TArgs&& ...args)
			: TProtocol(std::forward<TArgs&&>(args)...)
		{
		}

		const auto& GetLoggedTrace()
		{
			return this->loggedTrace;
		}
	};
}

#endif