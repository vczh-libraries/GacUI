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
		// both hitTestResult and element could be nullptr
		Nullable<INativeWindowListener::HitTestResult>		hitTestResult;
		Nullable<ElementDescVariant>						element;
		Rect												bounds;
		Rect												validArea;
		DomList												children;

		Ptr<glr::json::JsonObject> AsJson()
		{
			auto jsonDom = Ptr(new glr::json::JsonObject);
			if (hitTestResult)
			{
				auto fieldHtr = Ptr(new glr::json::JsonObjectField);
				fieldHtr->name.value = WString::Unmanaged(L"HitTestResult");
				fieldHtr->value = remoteprotocol::ConvertCustomTypeToJson(hitTestResult.Value());
				jsonDom->fields.Add(fieldHtr);
			}
			if (element)
			{
				auto fieldElement = Ptr(new glr::json::JsonObjectField);
				fieldElement->name.value = WString::Unmanaged(L"Element");
				element.Value().Apply([&](auto&& desc)
				{
					fieldElement->value = remoteprotocol::ConvertCustomTypeToJson(desc);
				});
				jsonDom->fields.Add(fieldElement);
			}
			{
				auto fieldBounds = Ptr(new glr::json::JsonObjectField);
				fieldBounds->name.value = WString::Unmanaged(L"Bounds");
				fieldBounds->value = remoteprotocol::ConvertCustomTypeToJson(bounds);
				jsonDom->fields.Add(fieldBounds);
			}
			{
				auto fieldValidArea = Ptr(new glr::json::JsonObjectField);
				fieldValidArea->name.value = WString::Unmanaged(L"ValidArea");
				fieldValidArea->value = remoteprotocol::ConvertCustomTypeToJson(validArea);
				jsonDom->fields.Add(fieldValidArea);
			}
			if (children.Count() > 0)
			{
				auto arrayChildren = Ptr(new glr::json::JsonArray);
				for (auto&& child : children)
				{
					arrayChildren->items.Add(child->AsJson());
				}

				auto fieldChildren = Ptr(new glr::json::JsonObjectField);
				fieldChildren->name.value = WString::Unmanaged(L"Children");
				fieldChildren->value = arrayChildren;
				jsonDom->fields.Add(fieldChildren);
			}
			return jsonDom;
		}
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

			auto getCurrentBoundary = [&]()
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
				domCurrent->children.Add(ref);
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

			for (auto&& command : *commandListRef.Obj())
			{
				command.Apply(Overloading(
					[&](const UnitTestRenderingBeginBoundary& command)
					{
						auto& boundary = command.boundary;
						Ptr<UnitTestRenderingDom> dom;
						{
							vint min = getCurrentBoundary();
							for (vint i = domStack.Count() - 1; i > min; i--)
							{
								auto parent = domStack[i];
								if (parent->bounds == boundary.bounds)
								{
									dom = parent;
									popTo(i);
									break;
								}
							}
						}

						if (dom)
						{
							CHECK_ERROR(dom->validArea == boundary.areaClippedBySelf, ERROR_MESSAGE_PREFIX L"Incorrect valid area of dom.");
							dom->hitTestResult = boundary.hitTestResult;
						}
						else
						{
							auto dom = Ptr(new UnitTestRenderingDom);
							dom->hitTestResult = boundary.hitTestResult;
							dom->bounds = boundary.bounds;
							dom->validArea = boundary.areaClippedBySelf;
							domBoundaries.Add(push(dom));
						}
					},
					[&](const UnitTestRenderingEndBoundary& command)
					{
						popBoundary();
					},
					[&](const UnitTestRenderingElement& command)
					{
						{
							vint min = getCurrentBoundary();
							vint index = min;
							for (vint i = domStack.Count() - 1; i > min; i--)
							{
								auto parent = domStack[i];
								if (parent->validArea.Contains(command.rendering.areaClippedByParent))
								{
									index = i;
									break;
								}
							}

							if (index == min && index != 0)
							{
								CHECK_ERROR(domStack[index]->validArea.Contains(command.rendering.areaClippedByParent), L"Incorrect valid area of element in current boundary.");
							}
							popTo(index);
						}
						auto dom = Ptr(new UnitTestRenderingDom);
						dom->element = command.desc;
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

		const auto& GetLoggedCreatedImages()
		{
			return this->createdImages;
		}

		const auto& GetLoggedCreatedElements()
		{
			return this->createdElements;
		}

		const auto& GetLoggedRenderingResults()
		{
			return loggedRenderingResults;
		}

		Ptr<glr::json::JsonObject> GetLogAsJson()
		{
			auto log = Ptr(new glr::json::JsonObject);
			{
				auto arrayImages = Ptr(new glr::json::JsonArray);
				for (auto&& image : GetLoggedCreatedImages().Values())
				{
					auto nodeImage = remoteprotocol::ConvertCustomTypeToJson(image);
					arrayImages->items.Add(nodeImage);
				}

				auto fieldImages = Ptr(new glr::json::JsonObjectField);
				fieldImages->name.value = WString::Unmanaged(L"Images");
				fieldImages->value = arrayImages;
				log->fields.Add(fieldImages);
			}
			{
				auto arrayElements = Ptr(new glr::json::JsonArray);
				for (auto [id, typeDescPair] : GetLoggedCreatedElements())
				{
					auto nodeElement = Ptr(new glr::json::JsonObject);
					{
						auto fieldId = Ptr(new glr::json::JsonObjectField);
						fieldId->name.value = WString::Unmanaged(L"Id");
						fieldId->value = remoteprotocol::ConvertJsonToCustomType(id);
						nodeElement->fields.Add(fieldId);
					}
					{
						auto fieldType = Ptr(new glr::json::JsonObjectField);
						fieldType->name.value = WString::Unmanaged(L"Type");
						fieldType->value = remoteprotocol::ConvertJsonToCustomType(typeDescPair.key);
						nodeElement->fields.Add(fieldType);
					}
					arrayElements->items.Add(nodeElement);
				}

				auto fieldElements = Ptr(new glr::json::JsonObjectField);
				fieldElements->name.value = WString::Unmanaged(L"Elements");
				fieldElements->value = arrayElements;
				log->fields.Add(fieldElements);
			}
			{
				auto arrayFrames = Ptr(new glr::json::JsonArray);
				for (auto&& frame : GetLoggedRenderingResults())
				{
					auto nodeFrame = frame->AsJson();
					arrayFrames->items.Add(nodeFrame);
				}

				auto fieldFrames = Ptr(new glr::json::JsonObjectField);
				fieldFrames->name.value = WString::Unmanaged(L"Frames");
				fieldFrames->value = arrayFrames;
				log->fields.Add(fieldFrames);
			}
			return log;
		}
	};
}

#endif