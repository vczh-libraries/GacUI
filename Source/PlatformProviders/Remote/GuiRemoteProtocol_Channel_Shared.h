/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocolChannel<T>

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL

#include "GuiRemoteProtocol_Shared.h"

namespace vl::presentation::remoteprotocol::channeling
{

/***********************************************************************
IGuiRemoteProtocolChannel<T>
***********************************************************************/

	template<typename TPackage>
	class IGuiRemoteProtocolChannelReceiver : public virtual Interface
	{
	public:
		virtual void											OnReceive(const TPackage& package) = 0;
	};

	template<typename TPackage>
	class IGuiRemoteProtocolChannel : public virtual Interface
	{
	public:
		virtual void											Initialize(IGuiRemoteProtocolChannelReceiver<TPackage>* receiver) = 0;
		virtual IGuiRemoteProtocolChannelReceiver<TPackage>*	GetReceiver() = 0;
		virtual void											Write(const TPackage& package) = 0;
		virtual WString											GetExecutablePath() = 0;
		virtual void											Submit(bool& disconnected) = 0;
		virtual void											ProcessRemoteEvents() = 0;
	};

/***********************************************************************
Serialization
***********************************************************************/

	template<typename TFrom, typename TTo>
	class GuiRemoteProtocolChannelTransformerBase
		: public Object
		, public virtual IGuiRemoteProtocolChannel<TFrom>
		, protected virtual IGuiRemoteProtocolChannelReceiver<TTo>
	{
	protected:
		IGuiRemoteProtocolChannel<TTo>*							channel = nullptr;
		IGuiRemoteProtocolChannelReceiver<TFrom>*				receiver = nullptr;

	public:
		GuiRemoteProtocolChannelTransformerBase(IGuiRemoteProtocolChannel<TTo>* _channel)
			: channel(_channel)
		{
		}

		void Initialize(IGuiRemoteProtocolChannelReceiver<TFrom>* _receiver) override
		{
			receiver = _receiver;
			channel->Initialize(this);
		}

		IGuiRemoteProtocolChannelReceiver<TFrom>* GetReceiver() override
		{
			return receiver;
		}

		WString GetExecutablePath() override
		{
			return channel->GetExecutablePath();
		}

		void Submit(bool& disconnected) override
		{
			channel->Submit(disconnected);
		}

		void ProcessRemoteEvents() override
		{
			channel->ProcessRemoteEvents();
		}
	};

	template<typename TSerialization>
	class GuiRemoteProtocolChannelSerializer
		: public GuiRemoteProtocolChannelTransformerBase<typename TSerialization::SourceType, typename TSerialization::DestType>
	{
	protected:
		typename TSerialization::ContextType					context;

		void OnReceive(const typename TSerialization::DestType& package) override
		{
			typename TSerialization::SourceType deserialized;
			TSerialization::Deserialize(context, package, deserialized);
			this->receiver->OnReceive(deserialized);
		}

	public:
		GuiRemoteProtocolChannelSerializer(IGuiRemoteProtocolChannel<typename TSerialization::DestType>* _channel, const typename TSerialization::ContextType& _context = {})
			: GuiRemoteProtocolChannelTransformerBase<typename TSerialization::SourceType, typename TSerialization::DestType>(_channel)
			, context(_context)
		{
		}

		void Write(const typename TSerialization::SourceType& package) override
		{
			typename TSerialization::DestType serialized;
			TSerialization::Serialize(context, package, serialized);
			this->channel->Write(serialized);
		}
	};

	template<typename TSerialization>
	class GuiRemoteProtocolChannelDeserializer
		: public GuiRemoteProtocolChannelTransformerBase<typename TSerialization::DestType, typename TSerialization::SourceType>
	{
	protected:
		typename TSerialization::ContextType					context;

		void OnReceive(const typename TSerialization::SourceType& package) override
		{
			typename TSerialization::DestType serialized;
			TSerialization::Serialize(context, package, serialized);
			this->receiver->OnReceive(serialized);
		}

	public:
		GuiRemoteProtocolChannelDeserializer(IGuiRemoteProtocolChannel<typename TSerialization::SourceType>* _channel, const typename TSerialization::ContextType& _context = {})
			: GuiRemoteProtocolChannelTransformerBase<typename TSerialization::DestType, typename TSerialization::SourceType>(_channel)
			, context(_context)
		{
		}

		void Write(const typename TSerialization::DestType& package) override
		{
			typename TSerialization::SourceType deserialized;
			TSerialization::Deserialize(context, package, deserialized);
			this->channel->Write(deserialized);
		}
	};

/***********************************************************************
String Transformation
***********************************************************************/

	template<typename TFrom, typename TTo>
	static void ConvertUtfString(const ObjectString<TFrom>& source, ObjectString<TTo>& dest)
	{
		vint len = _utftoutf<TFrom, TTo>(source.Buffer(), nullptr, 0);
		if (len < 1) dest = {};
		TTo* buffer = new TTo[len];
		memset(buffer, 0, len * sizeof(TTo));
		_utftoutf<TFrom, TTo>(source.Buffer(), buffer, len);
		dest = ObjectString<TTo>::TakeOver(buffer, len - 1);
	}

	template<typename TFrom, typename TTo>
	struct UtfStringSerializer
	{
		using SourceType = ObjectString<TFrom>;
		using DestType = ObjectString<TTo>;
		using ContextType = std::nullptr_t;

		static void Serialize(const ContextType&, const SourceType& source, DestType& dest)
		{
			ConvertUtfString(source, dest);
		}

		static void Deserialize(const ContextType&, const DestType& source, SourceType& dest)
		{
			ConvertUtfString(source, dest);
		}
	};

	template<typename TFrom, typename TTo>
	using GuiRemoteUtfStringChannelSerializer = GuiRemoteProtocolChannelSerializer<UtfStringSerializer<TFrom, TTo>>;

	template<typename TFrom, typename TTo>
	using GuiRemoteUtfStringChannelDeserializer = GuiRemoteProtocolChannelDeserializer<UtfStringSerializer<TFrom, TTo>>;
}

#endif