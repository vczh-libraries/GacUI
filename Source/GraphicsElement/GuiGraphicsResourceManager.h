/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Element System and Infrastructure Interfaces

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSRESOURCEMANAGER
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSRESOURCEMANAGER

#include "GuiGraphicsElementInterfaces.h"
#include "GuiGraphicsDocumentInterfaces.h"
#include "../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
Resource Manager
***********************************************************************/

			/// <summary>
			/// This is a class for managing grpahics element factories and graphics renderer factories
			/// </summary>
			class GuiGraphicsResourceManager : public Object
			{
				typedef collections::Dictionary<WString, Ptr<IGuiGraphicsElementFactory>>		elementFactoryMap;
				typedef collections::Dictionary<WString, Ptr<IGuiGraphicsRendererFactory>>		rendererFactoryMap;
			protected:
				elementFactoryMap						elementFactories;
				rendererFactoryMap						rendererFactories;
			public:
				/// <summary>
				/// Create a graphics resource manager without any predefined factories
				/// </summary>
				GuiGraphicsResourceManager();
				~GuiGraphicsResourceManager();

				/// <summary>
				/// Register a <see cref="IGuiGraphicsElementFactory"></see> using the element type from <see cref="IGuiGraphicsElementFactory::GetElementTypeName"></see>.
				/// </summary>
				/// <param name="factory">The instance of the graphics element factory to register.</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool							RegisterElementFactory(IGuiGraphicsElementFactory* factory);
				/// <summary>
				/// Register a <see cref="IGuiGraphicsRendererFactory"></see> and bind it to a registered <see cref="IGuiGraphicsElementFactory"></see>.
				/// </summary>
				/// <param name="elementTypeName">The element type to represent a graphics element factory.</param>
				/// <param name="factory">The instance of the graphics renderer factory to register.</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool							RegisterRendererFactory(const WString& elementTypeName, IGuiGraphicsRendererFactory* factory);
				/// <summary>
				/// Get the instance of a registered <see cref="IGuiGraphicsElementFactory"></see> that is binded to a specified element type.
				/// </summary>
				/// <returns>Returns the element factory.</returns>
				/// <param name="elementTypeName">The element type to get a corresponding graphics element factory.</param>
				virtual IGuiGraphicsElementFactory*		GetElementFactory(const WString& elementTypeName);
				/// <summary>
				/// Get the instance of a registered <see cref="IGuiGraphicsRendererFactory"></see> that is binded to a specified element type.
				/// </summary>
				/// <returns>Returns the renderer factory.</returns>
				/// <param name="elementTypeName">The element type to get a corresponding graphics renderer factory.</param>
				virtual IGuiGraphicsRendererFactory*	GetRendererFactory(const WString& elementTypeName);
				/// <summary>
				/// Get the instance of a <see cref="IGuiGraphicsRenderTarget"></see> that is binded to an <see cref="INativeWindow"></see>.
				/// </summary>
				/// <param name="window">The specified window.</param>
				/// <returns>Returns the render target.</returns>
				virtual IGuiGraphicsRenderTarget*		GetRenderTarget(INativeWindow* window)=0;
				/// <summary>
				/// Recreate the render target for the specified window.
				/// </summary>
				/// <param name="window">The specified window.</param>
				virtual void							RecreateRenderTarget(INativeWindow* window) = 0;
				/// <summary>
				/// Resize the render target to fit the current window size.
				/// </summary>
				/// <param name="window">The specified window.</param>
				virtual void							ResizeRenderTarget(INativeWindow* window) = 0;
				/// <summary>
				/// Get the renderer awared rich text document layout engine provider object.
				/// </summary>
				/// <returns>Returns the layout provider.</returns>
				virtual IGuiGraphicsLayoutProvider*		GetLayoutProvider()=0;
			};

			/// <summary>
			/// Get the current <see cref="GuiGraphicsResourceManager"></see>.
			/// </summary>
			/// <returns>Returns the current resource manager.</returns>
			extern GuiGraphicsResourceManager*			GetGuiGraphicsResourceManager();
			/// <summary>
			/// Set the current <see cref="GuiGraphicsResourceManager"></see>.
			/// </summary>
			/// <param name="resourceManager">The resource manager to set.</param>
			extern void									SetGuiGraphicsResourceManager(GuiGraphicsResourceManager* resourceManager);
			/// <summary>
			/// Helper function to register a <see cref="IGuiGraphicsElementFactory"></see> with a <see cref="IGuiGraphicsRendererFactory"></see> and bind them together.
			/// </summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="elementFactory">The element factory to register.</param>
			/// <param name="rendererFactory">The renderer factory to register.</param>
			extern bool									RegisterFactories(IGuiGraphicsElementFactory* elementFactory, IGuiGraphicsRendererFactory* rendererFactory);

/***********************************************************************
Helpers
***********************************************************************/

			template<typename TElement>
			class GuiElementBase : public Object, public IGuiGraphicsElement, public Description<TElement>
			{
			public:
				class Factory : public Object, public IGuiGraphicsElementFactory
				{
				public:
					WString GetElementTypeName()
					{
						return TElement::GetElementTypeName();
					}
					IGuiGraphicsElement* Create()
					{
						auto element = new TElement;
						element->factory = this;
						IGuiGraphicsRendererFactory* rendererFactory = GetGuiGraphicsResourceManager()->GetRendererFactory(GetElementTypeName());
						if (rendererFactory)
						{
							element->renderer = rendererFactory->Create();
							element->renderer->Initialize(element);
						}
						return element;
					}
				};
			protected:
				IGuiGraphicsElementFactory*				factory = nullptr;
				Ptr<IGuiGraphicsRenderer>				renderer;
				compositions::GuiGraphicsComposition*	ownerComposition = nullptr;

				void SetOwnerComposition(compositions::GuiGraphicsComposition* composition)override
				{
					ownerComposition = composition;
				}

				void InvokeOnCompositionStateChanged()
				{
					if (ownerComposition)
					{
						compositions::InvokeOnCompositionStateChanged(ownerComposition);
					}
				}

				void InvokeOnElementStateChanged()
				{
					if (renderer)
					{
						renderer->OnElementStateChanged();
					}
					InvokeOnCompositionStateChanged();
				}
			public:
				static TElement* Create()
				{
					return dynamic_cast<TElement*>(GetGuiGraphicsResourceManager()->GetElementFactory(TElement::GetElementTypeName())->Create());
				}

				~GuiElementBase()
				{
					if (renderer)
					{
						renderer->Finalize();
					}
				}

				IGuiGraphicsElementFactory* GetFactory()override
				{
					return factory;
				}

				IGuiGraphicsRenderer* GetRenderer()override
				{
					return renderer.Obj();
				}

				compositions::GuiGraphicsComposition* GetOwnerComposition()override
				{
					return ownerComposition;
				}
			};

#define DEFINE_GUI_GRAPHICS_ELEMENT(TELEMENT, ELEMENT_TYPE_NAME)\
				friend class GuiElementBase<TELEMENT>;\
			public:\
				static WString GetElementTypeName()\
				{\
					return ELEMENT_TYPE_NAME;\
				}\

#define DEFINE_GUI_GRAPHICS_RENDERER(TELEMENT, TRENDERER, TTARGET)\
			public:\
				class Factory : public Object, public IGuiGraphicsRendererFactory\
				{\
				public:\
					IGuiGraphicsRenderer* Create()\
					{\
						TRENDERER* renderer=new TRENDERER;\
						renderer->factory=this;\
						renderer->element=0;\
						renderer->renderTarget=0;\
						return renderer;\
					}\
				};\
			protected:\
				IGuiGraphicsRendererFactory*	factory;\
				TELEMENT*						element;\
				TTARGET*						renderTarget;\
				Size							minSize;\
			public:\
				static void Register()\
				{\
					RegisterFactories(new TELEMENT::Factory, new TRENDERER::Factory);\
				}\
				IGuiGraphicsRendererFactory* GetFactory()override\
				{\
					return factory;\
				}\
				void Initialize(IGuiGraphicsElement* _element)override\
				{\
					element=dynamic_cast<TELEMENT*>(_element);\
					InitializeInternal();\
				}\
				void Finalize()override\
				{\
					FinalizeInternal();\
				}\
				void SetRenderTarget(IGuiGraphicsRenderTarget* _renderTarget)override\
				{\
					TTARGET* oldRenderTarget=renderTarget;\
					renderTarget=dynamic_cast<TTARGET*>(_renderTarget);\
					RenderTargetChangedInternal(oldRenderTarget, renderTarget);\
				}\
				Size GetMinSize()override\
				{\
					return minSize;\
				}\

#define DEFINE_CACHED_RESOURCE_ALLOCATOR(TKEY, TVALUE)\
			public:\
				static const vint DeadPackageMax=32;\
				struct Package\
				{\
					TVALUE							resource;\
					vint								counter;\
					bool operator==(const Package& package)const{return false;}\
					bool operator!=(const Package& package)const{return true;}\
				};\
				struct DeadPackage\
				{\
					TKEY							key;\
					TVALUE							value;\
					bool operator==(const DeadPackage& package)const{return false;}\
					bool operator!=(const DeadPackage& package)const{return true;}\
				};\
				Dictionary<TKEY, Package>			aliveResources;\
				List<DeadPackage>					deadResources;\
			public:\
				TVALUE Create(const TKEY& key)\
				{\
					vint index=aliveResources.Keys().IndexOf(key);\
					if(index!=-1)\
					{\
						Package package=aliveResources.Values().Get(index);\
						package.counter++;\
						aliveResources.Set(key, package);\
						return package.resource;\
					}\
					TVALUE resource;\
					for(vint i=0;i<deadResources.Count();i++)\
					{\
						if(deadResources[i].key==key)\
						{\
							DeadPackage deadPackage=deadResources[i];\
							deadResources.RemoveAt(i);\
							resource=deadPackage.value;\
							break;\
						}\
					}\
					if(!resource)\
					{\
						resource=CreateInternal(key);\
					}\
					Package package;\
					package.resource=resource;\
					package.counter=1;\
					aliveResources.Add(key, package);\
					return package.resource;\
				}\
				void Destroy(const TKEY& key)\
				{\
					vint index=aliveResources.Keys().IndexOf(key);\
					if(index!=-1)\
					{\
						Package package=aliveResources.Values().Get(index);\
						package.counter--;\
						if(package.counter==0)\
						{\
							aliveResources.Remove(key);\
							if(deadResources.Count()==DeadPackageMax)\
							{\
								deadResources.RemoveAt(DeadPackageMax-1);\
							}\
							DeadPackage deadPackage;\
							deadPackage.key=key;\
							deadPackage.value=package.resource;\
							deadResources.Insert(0, deadPackage);\
						}\
						else\
						{\
							aliveResources.Set(key, package);\
						}\
					}\
				}
		}
	}
}

#endif