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
		Nullable<INativeWindowListener::HitTestResult>		hitTestResult;
		ElementDescVariantStrictNullable					element;
		Rect												bounds;
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
			RenderingResultRef domRoot, domCurrent;
			RenderingResultRefList domStack;
			collections::List<vint> domBoundaries;

			auto push = [&](RenderingResultRef ref)
			{
				CHECK_ERROR(ref, ERROR_MESSAGE_PREFIX L"[push] Cannot push a null dom object.");
				CHECK_ERROR(!domRoot || domStack.Count() > 0, ERROR_MESSAGE_PREFIX L"[push] Cannot recreate a dom root.");
				if (!domRoot) domRoot = ref;
				vint index = domStack.Add(ref);
				if (domCurrent) domCurrent->children.Add(ref);
				domCurrent = ref;
				return index;
			};

			auto popTo = [&](vint index)
			{
				if (index == -1)
				{
					CHECK_ERROR(domRoot, ERROR_MESSAGE_PREFIX L"[popTo] Cannot pop the dom root when it is not created yet.");
					domCurrent = nullptr;
					domStack.Clear();
				}
				else
				{
					CHECK_ERROR(0 <= index && index < domStack.Count(), ERROR_MESSAGE_PREFIX L"[popTo] Cannot pop to an invalid position.");
					CHECK_ERROR(index == domStack.Count() - 1, ERROR_MESSAGE_PREFIX L"[popTo] Cannot pop nothing.");
					CHECK_ERROR(domBoundaries.Count() == 0 || index >= domBoundaries[domBoundaries.Count() - 1], ERROR_MESSAGE_PREFIX L"[popTo] Cannot pop across a boundary.");
					while (domStack.Count() - 1> index)
					{
						domStack.RemoveAt(domStack.Count() - 1);
					}
					domCurrent = domStack[index];
				}
			};

			auto pop = [&]()
			{
				popTo(domStack.Count() - 2);
			};

			auto popBoundary = [&]()
			{
				CHECK_ERROR(domBoundaries.Count() > 0, ERROR_MESSAGE_PREFIX L"[popBoundary] Cannot pop a boundary when none is in the stack.");
				popTo(domBoundaries[domBoundaries.Count() - 1] - 1);
			};

			auto popByClipper = [&](Rect clipper)
			{
				CHECK_FAIL(L"Not Implemented!");
			};

			for (auto&& command : *commandListRef.Obj())
			{
				command.Apply(Overloading(
					[&](const UnitTestRenderingBeginBoundary& command)
					{
						auto& boundary = command.boundary;
						popByClipper(boundary.clipper);
						auto dom = Ptr(new UnitTestRenderingDom);
						dom->hitTestResult = boundary.hitTestResult;
						dom->bounds = boundary.bounds;
						domBoundaries.Add(push(dom));
					},
					[&](const UnitTestRenderingEndBoundary& command)
					{
						popBoundary();
					},
					[&](const UnitTestRenderingElement& command)
					{
						CHECK_FAIL(L"Not Implemented!");
					}));
			}

			CHECK_ERROR(domRoot, ERROR_MESSAGE_PREFIX L"Dom root is not created yet.");
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