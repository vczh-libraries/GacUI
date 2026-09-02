# Creating Remote Protocol and Foreign View Model

A coding agent can assemble the repetitive transport, generated-code, and test setup for a GacUI application that uses the remote protocol. It can also build a code generator and runtime support for implementing a Workflow RPC view model in a language other than C++. Give the agent the maintained repository guides below instead of describing the protocols from memory.

## Choose the Relevant Guides

- [GacUI Remoting Test Helpers](https://github.com/vczh-libraries/Release/blob/master/Import-Test/README.md) explains the reusable setup for a remote Core and renderer, and for applications with either a local or foreign view model. Use it for application bring-up, transport selection, process roles, connection order, and shutdown order.
- [Working with Workflow RPC](https://github.com/vczh-libraries/GacJS/blob/master/doc/rpc/README.md) is the entry point for implementing view models in another programming language. Use it when the agent must create the target-language RPC library, metadata-driven code generator, wrappers, or cross-language verification.

A remote renderer with a C++ view model normally needs only the GacUI guide. A GacUI application with a foreign view model needs both guides, whether the UI is rendered locally or through the remote protocol.

## Give the Agent Enough Repository Context

Clone or otherwise make the referenced GacUI, GacJS, and Workflow repositories available beside the application when their examples or test assets are needed. Ask the agent to read the selected guides first, then follow the reference projects and documents linked from them. Also provide the application's `Project.md`, resource XML, target platforms, process topology, transport choice, and the programming language that will own the view model.

Treat the bundled remoting helpers as bring-up and test support. If the result will be shipped, explicitly ask the agent to replace the test transport with an implementation that satisfies the product's security, validation, reliability, and deployment requirements.

