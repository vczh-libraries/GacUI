#ifndef GACUI_UNITTEST_TESTPOINTINPUT
#define GACUI_UNITTEST_TESTPOINTINPUT

#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol_Shared.h"

using namespace vl::presentation::remoteprotocol;

class PointProtocolInput
{
public:
	IGuiRemoteProtocolEvents*	events;
	IOPointAction				action;
	vuint64_t					sequence = 0;
	vuint64_t					timestamp = 1000000;
	bool						modifiers;

	PointProtocolInput(IGuiRemoteProtocolEvents* _events, bool _modifiers = true) : events(_events), modifiers(_modifiers) {}

	void						Start(NativePoint position)
	{
		NativeWindowPointInfo sample;
		sample.sequenceId = ++sequence;
		sample.timestamp = timestamp;
		sample.position = position;
		sample.activePointCount = 1;
		sample.ctrl = sample.shift = sample.alt = sample.osSuper = modifiers;
		action.info = EncodePointInfo(sample);
		action.eventIndex = 0;
		events->OnIOPointStarted(action);
		action.eventIndex++;
		events->OnIOPointDown(action);
	}

	void						Move(NativePoint position, vint pointId = 0)
	{
		action.info.pointId = pointId;
		action.info.position = position;
		action.eventIndex++;
		events->OnIOPointUpdate(action);
	}

	void						Down(vint pointId, NativePoint position)
	{
		action.info.pointId = pointId;
		action.info.position = position;
		action.info.activePointCount++;
		action.eventIndex++;
		events->OnIOPointDown(action);
	}

	void						Up(vint pointId, NativePoint position)
	{
		action.info.pointId = pointId;
		action.info.position = position;
		action.info.activePointCount--;
		action.eventIndex++;
		events->OnIOPointUp(action);
	}

	void						End(vuint64_t elapsed = 0, PointSequenceEndReason reason = PointSequenceEndReason::Succeeded)
	{
		IOPointEnd end = {};
		end.sequenceId = action.info.sequenceId;
		end.eventIndex = ++action.eventIndex;
		end.timestamp = u64tow(timestamp + elapsed);
		end.reason = reason;
		events->OnIOPointEnded(end);
		timestamp += elapsed + 1;
	}
};

#endif
