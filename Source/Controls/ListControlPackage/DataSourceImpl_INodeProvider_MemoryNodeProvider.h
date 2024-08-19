/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_DATASOURCEIMPL_INODEPROVIDER_MEMORYNODEPROVIDER
#define VCZH_PRESENTATION_CONTROLS_DATASOURCEIMPL_INODEPROVIDER_MEMORYNODEPROVIDER

#include "DataSource_INodeProvider.h"

namespace vl::presentation::controls::tree
{
/***********************************************************************
MemoryNodeProvider
***********************************************************************/

	/// <summary>An in-memory <see cref="INodeProvider"/> implementation.</summary>
	class MemoryNodeProvider
		: public Object
		, public virtual INodeProvider
		, public Description<MemoryNodeProvider>
	{
		typedef collections::List<Ptr<MemoryNodeProvider>> ChildList;
		typedef collections::IEnumerator<Ptr<MemoryNodeProvider>> ChildListEnumerator;

	public:
		class NodeCollection : public collections::ObservableListBase<Ptr<MemoryNodeProvider>>
		{
			friend class MemoryNodeProvider;
		protected:
			vint						offsetBeforeChildModified = 0;
			MemoryNodeProvider*			ownerProvider;

			void						OnBeforeChildModified(vint start, vint count, vint newCount);
			void						OnAfterChildModified(vint start, vint count, vint newCount);
			bool						QueryInsert(vint index, Ptr<MemoryNodeProvider> const& child)override;
			bool						QueryRemove(vint index, Ptr<MemoryNodeProvider> const& child)override;
			void						BeforeInsert(vint index, Ptr<MemoryNodeProvider> const& child)override;
			void						BeforeRemove(vint index, Ptr<MemoryNodeProvider> const& child)override;
			void						AfterInsert(vint index, Ptr<MemoryNodeProvider> const& child)override;
			void						AfterRemove(vint index, vint count)override;

			NodeCollection();
		public:
		};

	protected:
		MemoryNodeProvider*				parent = nullptr;
		bool							expanding = false;
		vint							childCount = 0;
		vint							totalVisibleNodeCount = 1;
		Ptr<DescriptableObject>			data;
		NodeCollection					children;

		virtual INodeProviderCallback*	GetCallbackProxyInternal();
		void							OnChildTotalVisibleNodesChanged(vint offset);
	public:
		/// <summary>Create a node provider with a data object.</summary>
		/// <param name="_data">The data object.</param>
		MemoryNodeProvider(Ptr<DescriptableObject> _data = nullptr);
		~MemoryNodeProvider();

		/// <summary>Get the data object.</summary>
		/// <returns>The data object.</returns>
		Ptr<DescriptableObject>			GetData();
		/// <summary>Set the data object.</summary>
		/// <param name="value">The data object.</param>
		void							SetData(const Ptr<DescriptableObject>& value);
		/// <summary>Get all sub nodes.</summary>
		/// <returns>All sub nodes.</returns>
		NodeCollection&					Children();

		bool							GetExpanding()override;
		void							SetExpanding(bool value)override;
		vint							CalculateTotalVisibleNodes()override;
		void							NotifyDataModified()override;

		vint							GetChildCount()override;
		Ptr<INodeProvider>				GetParent()override;
		Ptr<INodeProvider>				GetChild(vint index)override;
	};

/***********************************************************************
NodeRootProviderBase
***********************************************************************/

	/// <summary>A general implementation for <see cref="INodeRootProvider"/>.</summary>
	class NodeRootProviderBase : public virtual INodeRootProvider, protected virtual INodeProviderCallback, public Description<NodeRootProviderBase>
	{
		collections::List<INodeProviderCallback*>			callbacks;
	protected:
		void							OnAttached(INodeRootProvider* provider)override;
		void							OnBeforeItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
		void							OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
		void							OnItemExpanded(INodeProvider* node)override;
		void							OnItemCollapsed(INodeProvider* node)override;
	public:
		/// <summary>Create a node root provider.</summary>
		NodeRootProviderBase();
		~NodeRootProviderBase();
		
		bool							CanGetNodeByVisibleIndex()override;
		Ptr<INodeProvider>				GetNodeByVisibleIndex(vint index)override;
		bool							AttachCallback(INodeProviderCallback* value)override;
		bool							DetachCallback(INodeProviderCallback* value)override;
		IDescriptable*					RequestView(const WString& identifier)override;
	};

/***********************************************************************
MemoryNodeRootProvider
***********************************************************************/
				
	/// <summary>An in-memory <see cref="INodeRootProvider"/> implementation.</summary>
	class MemoryNodeRootProvider
		: public MemoryNodeProvider
		, public NodeRootProviderBase
		, public Description<MemoryNodeRootProvider>
	{
	protected:
		INodeProviderCallback*			GetCallbackProxyInternal()override;
	public:
		/// <summary>Create a node root provider.</summary>
		MemoryNodeRootProvider();
		~MemoryNodeRootProvider();

		Ptr<INodeProvider>				GetRootNode()override;
		/// <summary>Get the <see cref="MemoryNodeProvider"/> object from an <see cref="INodeProvider"/> object.</summary>
		/// <returns>The corresponding <see cref="MemoryNodeProvider"/> object.</returns>
		/// <param name="node">The node to get the memory node.</param>
		MemoryNodeProvider*				GetMemoryNode(INodeProvider* node);
	};
}

#endif