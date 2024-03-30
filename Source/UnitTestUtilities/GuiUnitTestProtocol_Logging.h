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

	class UnitTestRenderingDom : public Object
	{
		using DomList = collections::List<Ptr<UnitTestRenderingDom>>;
	public:
		// when hitTestResult and element are both nullptr_t
		// it is the root node
		Nullable<INativeWindowListener::HitTestResult>		hitTestResult;
		Nullable<ElementDescVariantStrict>					element;
		Rect												bounds;
		Rect												clipper;
		DomList												children;
	};
	
	template<typename TProtocol>
	class UnitTestRemoteProtocol_Logging : public TProtocol
	{
		using CommandList = UnitTestRenderingCommandList;
		using CommandListRef = UnitTestRenderingCommandListRef;
		using RenderingResultRef = Ptr<UnitTestRenderingDom>;
		using RenderingResultRefList = collections::List<RenderingResultRef>;
	protected:

		bool								everRendered = false;
		CommandListRef						candidateRenderingResult;
		RenderingResultRefList				loggedRenderingResults;

		RenderingResultRef TransformLastRenderingResult(CommandListRef commandListRef)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Logging<TProtocol>::TransformLastRenderingResult(CommandListRef)#"

			RenderingResultRefList domStack;
			collections::List<vint> domBoundaries;

			auto domRoot = Ptr(new UnitTestRenderingDom);
			auto domCurrent = domRoot;
			domStack.Add(domRoot);

			auto push = [&](RenderingResultRef ref)
			{
				CHECK_ERROR(ref, ERROR_MESSAGE_PREFIX L"[push] Cannot push a null dom object.");
				vint index = domStack.Add(ref);
				domCurrent->children.Add(ref);
				domCurrent = ref;
				return index;
			};

			auto popTo = [&](vint index)
			{
				CHECK_ERROR(0 <= index && index < domStack.Count(), ERROR_MESSAGE_PREFIX L"[popTo] Cannot pop to an invalid position.");
				CHECK_ERROR(index == domStack.Count() - 1, ERROR_MESSAGE_PREFIX L"[popTo] Cannot pop nothing.");
				CHECK_ERROR(domBoundaries.Count() == 0 || index >= domBoundaries[domBoundaries.Count() - 1], ERROR_MESSAGE_PREFIX L"[popTo] Cannot pop across a boundary.");
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

			auto popByClipperInBoundary = [&](Rect clipper)
			{
				vint max = domStack.Count() - 1;
				vint min = 0;
				if (domBoundaries.Count() > 0) min = domBoundaries[domBoundaries.Count() - 1];

				vint index = max;
				while (index > min)
				{
					auto parent = domStack[index];
					if (parent->clipper.Contains(clipper)) break;
					index--;
				}

				if (index == min && index != 0)
				{
					auto parent = domStack[index];
					CHECK_ERROR(parent->clipper.Contains(clipper), ERROR_MESSAGE_PREFIX L"[popByClipperInBoundary] Clipper is not contained by the current boundary.");
				}

				if (index < max)
				{
					popTo(index);
				}
			};

			for (auto&& command : *commandListRef.Obj())
			{
				command.Apply(Overloading(
					[&](const UnitTestRenderingBeginBoundary& command)
					{
						auto& boundary = command.boundary;
						popByClipperInBoundary(boundary.clipper);
						auto dom = Ptr(new UnitTestRenderingDom);
						dom->hitTestResult = boundary.hitTestResult;
						dom->bounds = boundary.bounds;
						dom->clipper = boundary.clipper;
						domBoundaries.Add(push(dom));
					},
					[&](const UnitTestRenderingEndBoundary& command)
					{
						popBoundary();
					},
					[&](const UnitTestRenderingElement& command)
					{
						popByClipperInBoundary(command.rendering.clipper);
						auto dom = Ptr(new UnitTestRenderingDom);
						dom->element = command.desc;
						dom->bounds = command.rendering.bounds;
						dom->clipper = command.rendering.clipper;
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
				candidateRenderingResult = this->lastRenderingCommands;
				this->lastRenderingCommands = {};
				everRendered = true;
			}
			else if (everRendered)
			{
				if (candidateRenderingResult)
				{
					auto transformed = TransformLastRenderingResult(candidateRenderingResult);
					loggedRenderingResults.Add(transformed);
					candidateRenderingResult = {};
				}
				return true;
			}
			return false;
		}

	public:

		template<typename ...TArgs>
		UnitTestRemoteProtocol_Logging(TArgs&& ...args)
			: TProtocol(std::forward<TArgs&&>(args)...)
		{
		}

		RenderingResultRefList& GetLoggedRenderingResults()
		{
			return loggedRenderingResults;
		}
	};
}

#endif