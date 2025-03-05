#include "GuiRemoteProtocol_Filter.h"

namespace vl::presentation::remoteprotocol::repeatfiltering
{

/***********************************************************************
GuiRemoteEventFilter
***********************************************************************/

	GuiRemoteEventFilter::GuiRemoteEventFilter()
	{
	}

	GuiRemoteEventFilter::~GuiRemoteEventFilter()
	{
	}

	void GuiRemoteEventFilter::ProcessResponses()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::repeatfiltering::GuiRemoteProtocolFilter::ProcessResponses()#"
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
				CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognized response.");
			}
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES
		}

		CHECK_ERROR(responseIds.Count() == 0,  L"Messages sending to IGuiRemoteProtocol should be all responded.");
		filteredResponses.Clear();
#undef ERROR_MESSAGE_PREFIX
	}
	
	void GuiRemoteEventFilter::ProcessEvents()
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
	void GuiRemoteEventFilter::Respond ## NAME(vint id, const RESPONSE& arguments)\
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
	void GuiRemoteEventFilter::On ## NAME()\
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
	void GuiRemoteEventFilter::On ## NAME(const REQUEST& arguments)\
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

/***********************************************************************
GuiRemoteProtocolFilter
***********************************************************************/

	void GuiRemoteProtocolFilter::ProcessRequests()
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
	
		filteredRequests.Clear();
	}

	GuiRemoteProtocolFilter::GuiRemoteProtocolFilter(IGuiRemoteProtocol* _protocol)
		: GuiRemoteProtocolCombinator<GuiRemoteEventFilter>(_protocol)
	{
	}

	GuiRemoteProtocolFilter::~GuiRemoteProtocolFilter()
	{
	}
	
	// messages
	
#define MESSAGE_NODROP(NAME)
	
#define MESSAGE_DROPREP(NAME)\
		if (lastDropRepeatRequest ## NAME != -1)\
		{\
			filteredRequests[lastDropRepeatRequest ## NAME].dropped = true;\
		}\
		lastDropRepeatRequest ## NAME = filteredRequests.Count()\
	
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)\
	void GuiRemoteProtocolFilter::Request ## NAME()\
	{\
		MESSAGE_ ## DROPTAG(NAME);\
		FilteredRequest request;\
		request.name = FilteredRequestNames::NAME;\
		filteredRequests.Add(request);\
	}\
	
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)\
	void GuiRemoteProtocolFilter::Request ## NAME(vint id)\
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
	void GuiRemoteProtocolFilter::Request ## NAME(const REQUEST& arguments)\
	{\
		MESSAGE_ ## DROPTAG(NAME);\
		FilteredRequest request;\
		request.name = FilteredRequestNames::NAME;\
		request.arguments = arguments;\
		filteredRequests.Add(request);\
	}\
	
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)\
	void GuiRemoteProtocolFilter::Request ## NAME(vint id, const REQUEST& arguments)\
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
	
	void GuiRemoteProtocolFilter::Initialize(IGuiRemoteProtocolEvents* _events)
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
	
	void GuiRemoteProtocolFilter::Submit(bool& disconnected)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::repeatfiltering::GuiRemoteProtocolFilter::Submit()#"
		CHECK_ERROR(!eventCombinator.submitting, ERROR_MESSAGE_PREFIX L"This function is not allowed to be called recursively.");
		eventCombinator.submitting = true;
		ProcessRequests();
		GuiRemoteProtocolCombinator<GuiRemoteEventFilter>::Submit(disconnected);
		if (!disconnected)
		{
			eventCombinator.ProcessResponses();
		}
		eventCombinator.submitting = false;
		eventCombinator.ProcessEvents();
#undef ERROR_MESSAGE_PREFIX
	}
}