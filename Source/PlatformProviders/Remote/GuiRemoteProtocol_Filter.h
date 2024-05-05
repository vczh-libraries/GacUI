/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocol

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_FILTER
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_FILTER

#include "GuiRemoteProtocol_FilterVerifier.h"

namespace vl::presentation::remoteprotocol::repeatfiltering
{
	using FilteredRequestTypes = Variant<std::nullptr_t
#define FILTERED_VARIANT_ELEMENT(TYPE) ,TYPE
		GACUI_REMOTEPROTOCOL_MESSAGE_REQUEST_TYPES(FILTERED_VARIANT_ELEMENT)
#undef FILTERED_VARIANT_ELEMENT
		>;
	
	using FilteredResponseTypes = Variant<std::nullptr_t
#define FILTERED_VARIANT_ELEMENT(TYPE) ,TYPE
		GACUI_REMOTEPROTOCOL_MESSAGE_RESPONSE_TYPES(FILTERED_VARIANT_ELEMENT)
#undef FILTERED_VARIANT_ELEMENT
		>;
	
	using FilteredEventTypes = Variant<std::nullptr_t
#define FILTERED_VARIANT_ELEMENT(TYPE) ,TYPE
		GACUI_REMOTEPROTOCOL_EVENT_REQUEST_TYPES(FILTERED_VARIANT_ELEMENT)
#undef FILTERED_VARIANT_ELEMENT
		>;

	enum class FilteredRequestNames
	{
		Unknown,
#define FILTERED_ENUM_ITEM(NAME, ...) NAME,
		GACUI_REMOTEPROTOCOL_MESSAGES(FILTERED_ENUM_ITEM)
#undef FILTERED_ENUM_ITEM
	};

	enum class FilteredResponseNames
	{
		Unknown,
#define FILTERED_ENUM_ITEM_NORES(NAME)
#define FILTERED_ENUM_ITEM_RES(NAME) NAME,
#define FILTERED_ENUN_ITEM(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...) FILTERED_ENUM_ITEM_ ## RESTAG(NAME)
		GACUI_REMOTEPROTOCOL_MESSAGES(FILTERED_ENUN_ITEM)
#undef FILTERED_ENUM_ITEM
#undef FILTERED_ENUM_ITEM_RES
#undef FILTERED_ENUM_ITEM_NORES
	};

	enum class FilteredEventNames
	{
		Unknown,
#define FILTERED_ENUM_ITEM(NAME, ...) NAME,
		GACUI_REMOTEPROTOCOL_EVENTS(FILTERED_ENUM_ITEM)
#undef FILTERED_ENUM_ITEM
	};

	struct FilteredRequest
	{
		bool					dropped = false;
		vint					id = -1;
		FilteredRequestNames	name = FilteredRequestNames::Unknown;
		FilteredRequestTypes	arguments;
	};

	struct FilteredResponse
	{
		vint					id = -1;
		FilteredResponseNames	name = FilteredResponseNames::Unknown;
		FilteredResponseTypes	arguments;
	};

	struct FilteredEvent
	{
		bool					dropped = false;
		vint					id = -1;
		FilteredEventNames		name = FilteredEventNames::Unknown;
		FilteredEventTypes		arguments;
	};

/***********************************************************************
GuiRemoteEventFilter
***********************************************************************/

	class GuiRemoteEventFilter : public GuiRemoteEventCombinator
	{
	protected:
		collections::List<FilteredResponse>						filteredResponses;
		collections::List<FilteredEvent>						filteredEvents;

#define EVENT_NODROP(NAME)
#define EVENT_DROPREP(NAME)										vint lastDropRepeatEvent ## NAME = -1;
#define EVENT_DROPCON(NAME)										vint lastDropConsecutiveEvent ## NAME = -1;
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)		EVENT_ ## DROPTAG(NAME)
			GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_DROPCON
#undef EVENT_DROPREP
#undef EVENT_NODROP

	public:
		bool													submitting = false;
		collections::Dictionary<vint, FilteredResponseNames>	responseIds;
	
		void ProcessResponses()
		{
			for (auto&& response : filteredResponses)
			{
#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)\
				case FilteredResponseNames::NAME:\
					targetEvents->Respond ## NAME(response.id, response.arguments.Get<RESPONSE>());\
					break;\
	
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
				switch (response.name)
				{
				GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
				default:
					CHECK_FAIL(L"vl::presentation::remoteprotocol::GuiRemoteEventFilter::ProcessResponses()#Unrecognized response.");
				}
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES
			}
	
			filteredResponses.Clear();
		}
	
		void ProcessEvents()
		{
#define EVENT_NODROP(NAME)
#define EVENT_DROPREP(NAME)									lastDropRepeatEvent ## NAME = -1;
#define EVENT_DROPCON(NAME)									lastDropConsecutiveEvent ## NAME = -1;
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)	EVENT_ ## DROPTAG(NAME)
			GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_DROPCON
#undef EVENT_DROPREP
#undef EVENT_NODROP
	
			collections::List<FilteredEvent> events(std::move(filteredEvents));
	
			for (auto&& event : events)
			{
				if (event.dropped)
				{
					continue;
				}
	
#define EVENT_NOREQ(NAME, REQUEST)\
				case FilteredEventNames::NAME:\
					targetEvents->On ## NAME();\
					break;\
	
#define EVENT_REQ(NAME, REQUEST)\
				case FilteredEventNames::NAME:\
					targetEvents->On ## NAME(event.arguments.Get<REQUEST>());\
					break;\
	
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST)
				switch (event.name)
				{
				GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
				default:
					CHECK_FAIL(L"vl::presentation::remoteprotocol::GuiRemoteEventFilter::ProcessEvents()#Unrecognized event.");
				}
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ
			}
		}

		// responses

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)\
		void Respond ## NAME(vint id, const RESPONSE& arguments) override\
		{\
			CHECK_ERROR(\
				responseIds[id] == FilteredResponseNames::NAME,\
				L"vl::presentation::remoteprotocol::GuiRemoteEventFilter::"\
				L"Respond" L ## #NAME L"()#"\
				L"Messages sending to IGuiRemoteProtocol should be responded by calling the correct function.");\
			responseIds.Remove(id);\
			FilteredResponse response;\
			response.id = id;\
			response.name = FilteredResponseNames::NAME;\
			response.arguments = arguments;\
			filteredResponses.Add(response);\
		}\
	
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

		// events
	
#define EVENT_NODROP(NAME)
	
#define EVENT_DROPREP(NAME)\
			if (lastDropRepeatEvent ## NAME != -1)\
			{\
				filteredEvents[lastDropRepeatEvent ## NAME].dropped = true;\
			}\
			lastDropRepeatEvent ## NAME = filteredEvents.Count() - 1\
	
#define EVENT_DROPCON(NAME)\
			if (lastDropConsecutiveEvent ## NAME != -1 && lastDropConsecutiveEvent ## NAME == filteredEvents.Count() - 1)\
			{\
				filteredEvents[lastDropConsecutiveEvent ## NAME].dropped = true;\
			}\
			lastDropConsecutiveEvent ## NAME = filteredEvents.Count() - 1\
	
#define EVENT_NOREQ(NAME, REQUEST, DROPTAG)\
		void On ## NAME() override\
		{\
			if (submitting)\
			{\
				EVENT_ ## DROPTAG(NAME);\
				FilteredEvent event;\
				event.name = FilteredEventNames::NAME;\
				filteredEvents.Add(event);\
			}\
			else\
			{\
				targetEvents->On ## NAME();\
			}\
		}\
	
#define EVENT_REQ(NAME, REQUEST, DROPTAG)\
		void On ## NAME(const REQUEST& arguments) override\
		{\
			if (submitting)\
			{\
				EVENT_ ## DROPTAG(NAME);\
				FilteredEvent event;\
				event.name = FilteredEventNames::NAME;\
				event.arguments = arguments;\
				filteredEvents.Add(event);\
			}\
			else\
			{\
				targetEvents->On ## NAME(arguments);\
			}\
		}\
	
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST, DROPTAG)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ
#undef EVENT_DROPCON
#undef EVENT_DROPREP
#undef EVENT_NOREP
	};

/***********************************************************************
GuiRemoteProtocolFilter
***********************************************************************/
	
	class GuiRemoteProtocolFilter : public GuiRemoteProtocolCombinator<GuiRemoteEventFilter>
	{
	protected:
		vint													lastRequestId = -1;
		collections::List<FilteredRequest>						filteredRequests;
	
#define MESSAGE_NODROP(NAME)
#define MESSAGE_DROPREP(NAME)												vint lastDropRepeatRequest ## NAME = -1;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)	MESSAGE_ ## DROPTAG(NAME)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP
	
		void ProcessRequests()
		{
#define MESSAGE_NODROP(NAME)
#define MESSAGE_DROPREP(NAME)												lastDropRepeatRequest ## NAME = -1;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)	MESSAGE_ ## DROPTAG(NAME)
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP
	
			for (auto&& request : filteredRequests)
			{
				CHECK_ERROR(\
					!request.dropped || request.id == -1,\
					L"vl::presentation::remoteprotocol::GuiRemoteProtocolFilter::ProcessRequests()#"\
					L"Messages with id cannot be dropped.");\
				if (request.dropped)
				{
					continue;
				}
	
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)\
				case FilteredRequestNames::NAME:\
					targetProtocol->Request ## NAME();\
					break;\
	
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)\
				case FilteredRequestNames::NAME:\
					targetProtocol->Request ## NAME(request.id);\
					break;\
	
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)\
				case FilteredRequestNames::NAME:\
					targetProtocol->Request ## NAME(request.arguments.Get<REQUEST>());\
					break;\
	
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)\
				case FilteredRequestNames::NAME:\
					targetProtocol->Request ## NAME(request.id, request.arguments.Get<REQUEST>());\
					break;\
	
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
				switch (request.name)
				{
				GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
				default:
					CHECK_FAIL(L"vl::presentation::remoteprotocol::GuiRemoteProtocolFilter::ProcessRequests()#Unrecognized request.");
				}
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
			}
	
			CHECK_ERROR(eventCombinator.responseIds.Count() == 0, L"Messages sending to IGuiRemoteProtocol should be all responded.");
			filteredRequests.Clear();
		}
	public:
		GuiRemoteProtocolFilter(IGuiRemoteProtocol* _protocol)
			: GuiRemoteProtocolCombinator<GuiRemoteEventFilter>(_protocol)
		{
		}
	
	protected:
	
	public:
	
		// messages
	
#define MESSAGE_NODROP(NAME)
	
#define MESSAGE_DROPREP(NAME)\
			if (lastDropRepeatRequest ## NAME != -1)\
			{\
				filteredRequests[lastDropRepeatRequest ## NAME].dropped = true;\
			}\
			lastDropRepeatRequest ## NAME = filteredRequests.Count()\
	
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME() override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			FilteredRequest request;\
			request.name = FilteredRequestNames::NAME;\
			filteredRequests.Add(request);\
		}\
	
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME(vint id) override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			CHECK_ERROR(\
				lastRequestId < id,\
				L"vl::presentation::remoteprotocol::GuiRemoteProtocolFilter::"\
				L"Request" L ## #NAME L"()#"\
				L"Id of a message sending to IGuiRemoteProtocol should be increasing.");\
			lastRequestId = id;\
			FilteredRequest request;\
			request.id = id;\
			request.name = FilteredRequestNames::NAME;\
			filteredRequests.Add(request);\
			eventCombinator.responseIds.Add(id, FilteredResponseNames::NAME);\
		}\
	
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME(const REQUEST& arguments) override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			FilteredRequest request;\
			request.name = FilteredRequestNames::NAME;\
			request.arguments = arguments;\
			filteredRequests.Add(request);\
		}\
	
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)\
		void Request ## NAME(vint id, const REQUEST& arguments) override\
		{\
			MESSAGE_ ## DROPTAG(NAME);\
			CHECK_ERROR(\
				lastRequestId < id,\
				L"vl::presentation::remoteprotocol::GuiRemoteProtocolFilter::"\
				L"Request" L ## #NAME L"()#"\
				L"Id of a message sending to IGuiRemoteProtocol should be increasing.");\
			lastRequestId = id;\
			FilteredRequest request;\
			request.id = id;\
			request.name = FilteredRequestNames::NAME;\
			request.arguments = arguments;\
			filteredRequests.Add(request);\
			eventCombinator.responseIds.Add(id, FilteredResponseNames::NAME);\
		}\
	
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE, DROPTAG)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP
	
		// protocol
	
		void Initialize(IGuiRemoteProtocolEvents* _events) override
		{
			if (auto verifierProtocol = dynamic_cast<GuiRemoteProtocolFilterVerifier*>(targetProtocol))
			{
				verifierProtocol->targetProtocol->Initialize(&eventCombinator);
				eventCombinator.targetEvents = &verifierProtocol->eventCombinator;
				verifierProtocol->eventCombinator.targetEvents = _events;
			}
			else
			{
				GuiRemoteProtocolCombinator<GuiRemoteEventFilter>::Initialize(_events);
			}
		}
	
		void Submit() override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::repeatfiltering::GuiRemoteProtocolFilter::Submit()#"
			CHECK_ERROR(!eventCombinator.submitting, ERROR_MESSAGE_PREFIX L"This function is not allowed to be called recursively.");
			eventCombinator.submitting = true;
			ProcessRequests();
			eventCombinator.ProcessResponses();
			GuiRemoteProtocolCombinator<GuiRemoteEventFilter>::Submit();
			eventCombinator.submitting = false;
			eventCombinator.ProcessEvents();
#undef ERROR_MESSAGE_PREFIX
		}
	};
}

#endif