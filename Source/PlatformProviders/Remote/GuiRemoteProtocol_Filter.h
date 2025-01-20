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

		GuiRemoteEventFilter();
		~GuiRemoteEventFilter();
	
		void													ProcessResponses();	
		void													ProcessEvents();

		// responses

#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE)								void Respond ## NAME(vint id, const RESPONSE& arguments) override;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## RESTAG(NAME, RESPONSE)
			GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES

		// events

#define EVENT_NOREQ(NAME, REQUEST, DROPTAG)			void On ## NAME() override;
#define EVENT_REQ(NAME, REQUEST, DROPTAG)			void On ## NAME(const REQUEST& arguments) override;
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, DROPTAG, ...)	EVENT_ ## REQTAG(NAME, REQUEST, DROPTAG)
		GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ
	};

/***********************************************************************
GuiRemoteProtocolFilter
***********************************************************************/
	
	class GuiRemoteProtocolFilter : public GuiRemoteProtocolCombinator<GuiRemoteEventFilter>
	{
	protected:
		vint																lastRequestId = -1;
		collections::List<FilteredRequest>									filteredRequests;
	
#define MESSAGE_NODROP(NAME)
#define MESSAGE_DROPREP(NAME)												vint lastDropRepeatRequest ## NAME = -1;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG)	MESSAGE_ ## DROPTAG(NAME)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_DROPREP
#undef MESSAGE_NODROP

		void																ProcessRequests();
	public:
		GuiRemoteProtocolFilter(IGuiRemoteProtocol* _protocol);
		~GuiRemoteProtocolFilter();
	
	protected:
	
	public:
	
		// messages


#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)				void Request ## NAME() override;
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)					void Request ## NAME(vint id) override;
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE, DROPTAG)					void Request ## NAME(const REQUEST& arguments) override;
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE, DROPTAG)					void Request ## NAME(vint id, const REQUEST& arguments) override;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, DROPTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE, DROPTAG)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

		// protocol

		void																Initialize(IGuiRemoteProtocolEvents* _events) override;
		void																Submit() override;
	};
}

#endif