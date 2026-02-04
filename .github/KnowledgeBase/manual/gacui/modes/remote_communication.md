# Implementing a Communication Protocol

**IGuiRemoteProtocol**has a lot of strong typed messages defined as functions. When the core application wants to send anything to a remote client, a member function is called. When the remote client sent anything to the core application, a member function in**IGuiRemoteProtocol::GetRemoteEventProcessor**is called. Some messages need to wait for a response from the remote client, the first argument of such message is always**vint id**. The core application will be blocked until the remote client sends back a expected response message with the same id.

Therefore**IGuiRemoteProtocol**represents a renderer. The core side implementation sends messages to client and receive responses from client. The client side implementation receives messages from core and send responses to core.

**IGuiRemoteProtocolChannel\<T\>**and**IGuiRemoteProtocolChannelReceiver\<T\>**works like**IGuiRemoteProtocol**and**IGuiRemoteEventProcessor**, but there is no strong typed messages anymore, everything becomes the**T**.

**GuiRemoteProtocolFromJsonChannel**(for core) and**GuiRemoteJsonChannelFromProtocol**(for client) handles message translation between**IGuiRemoteProtocol**and**IGuiRemoteProtocolChannel\<Ptr\<JsonObject\>\>**.

**GuiRemoteJsonChannelStringSerializer**(for core) and**GuiRemoteJsonChannelStringDeserializer**(for client) handles message translation between**IGuiRemoteProtocolChannel\<Ptr\<JsonObject\>\>**and**IGuiRemoteProtocolChannel\<WString\>**.

Although message are sent bidirectionally, but the naming is saying what is done from core to client.[Remote Protocol Client Application](../.././gacui/modes/remote_client.md)and[Implementing a Communication Protocol](../.././gacui/modes/remote_communication.md)demos how to setup the pipeline from**IGuiRemoteProtocol**all the way to sending and receiving strings. You can also implements your own pipeline using different message formats. JSON serialization is offered in GacUI by default, but you can also code generate your own serializer with other underlying format using metadata in[this folder](https://github.com/vczh-libraries/Release/tree/master/Import/Metadata).

