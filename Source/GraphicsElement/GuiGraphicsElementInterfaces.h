/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Element System and Infrastructure Interfaces

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSELEMENTINTERFACES
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSELEMENTINTERFACES

#include "../GuiTypes.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			class GuiGraphicsComposition;

			extern void											InvokeOnCompositionStateChanged(compositions::GuiGraphicsComposition* composition);
		}

		namespace elements
		{
			class IGuiGraphicsElement;
			class IGuiGraphicsElementFactory;
			class IGuiGraphicsRenderer;
			class IGuiGraphicsRendererFactory;
			class IGuiGraphicsRenderTarget;

/***********************************************************************
Basic Construction
***********************************************************************/

			/// <summary>
			/// This is the interface for graphics elements.
			/// Graphics elements usually contains some information and helper functions for visible things.
			/// An graphics elements should be created using ElementType::Create.
			/// </summary>
			class IGuiGraphicsElement : public virtual IDescriptable, public Description<IGuiGraphicsElement>
			{
				friend class compositions::GuiGraphicsComposition;
			protected:

				virtual void									SetOwnerComposition(compositions::GuiGraphicsComposition* composition) = 0;
			public:
				/// <summary>
				/// Access the <see cref="IGuiGraphicsElementFactory"></see> that is used to create this graphics elements.
				/// </summary>
				/// <returns>Returns the related factory.</returns>
				virtual IGuiGraphicsElementFactory*				GetFactory() = 0;
				/// <summary>
				/// Access the associated <see cref="IGuiGraphicsRenderer"></see> for this graphics element.
				/// </summary>
				/// <returns>Returns the related renderer.</returns>
				virtual IGuiGraphicsRenderer*					GetRenderer() = 0;
				/// <summary>
				/// Get the owner composition.
				/// </summary>
				/// <returns>The owner composition.</returns>
				virtual compositions::GuiGraphicsComposition*	GetOwnerComposition() = 0;
			};

			/// <summary>
			/// This is the interface for graphics element factories.
			/// Graphics element factories should be registered using [M:vl.presentation.elements.GuiGraphicsResourceManager.RegisterElementFactory].
			/// </summary>
			class IGuiGraphicsElementFactory : public Interface
			{
			public:
				/// <summary>
				/// Get the name representing the kind of graphics element to be created.
				/// </summary>
				/// <returns>Returns the name of graphics elements.</returns>
				virtual WString							GetElementTypeName()=0;
				/// <summary>
				/// Create a <see cref="IGuiGraphicsElement"></see>.
				/// </summary>
				/// <returns>Returns the created graphics elements.</returns>
				virtual IGuiGraphicsElement*			Create()=0;
			};

			/// <summary>
			/// This is the interface for graphics renderers.
			/// </summary>
			class IGuiGraphicsRenderer : public Interface
			{
			public:
				/// <summary>
				/// Access the graphics <see cref="IGuiGraphicsRendererFactory"></see> that is used to create this graphics renderer.
				/// </summary>
				/// <returns>Returns the related factory.</returns>
				virtual IGuiGraphicsRendererFactory*	GetFactory()=0;

				/// <summary>
				/// Initialize the grpahics renderer by binding a <see cref="IGuiGraphicsElement"></see> to it.
				/// </summary>
				/// <param name="element">The graphics element to bind.</param>
				virtual void							Initialize(IGuiGraphicsElement* element)=0;
				/// <summary>
				/// Release all resources that used by this renderer.
				/// </summary>
				virtual void							Finalize()=0;
				/// <summary>
				/// Set a <see cref="IGuiGraphicsRenderTarget"></see> to this element.
				/// </summary>
				/// <param name="renderTarget">The graphics render target. It can be NULL.</param>
				virtual void							SetRenderTarget(IGuiGraphicsRenderTarget* renderTarget)=0;
				/// <summary>
				/// Render the graphics element using a specified bounds.
				/// </summary>
				/// <param name="bounds">Bounds to decide the size and position of the binded graphics element.</param>
				virtual void							Render(Rect bounds)=0;
				/// <summary>
				/// Notify that the state in the binded graphics element is changed. This function is usually called by the element itself.
				/// </summary>
				virtual void							OnElementStateChanged()=0;
				/// <summary>
				/// Calculate the minimum size using the binded graphics element and its state.
				/// </summary>
				/// <returns>The minimum size.</returns>
				virtual Size							GetMinSize()=0;
			};

			/// <summary>
			/// This is the interface for graphics renderer factories.
			/// Graphics renderers should be registered using [M:vl.presentation.elements.GuiGraphicsResourceManager.RegisterRendererFactory]. 
			/// </summary>
			class IGuiGraphicsRendererFactory : public Interface
			{
			public:
				/// <summary>
				/// Create a <see cref="IGuiGraphicsRenderer"></see>.
				/// </summary>
				/// <returns>Returns the created graphics renderer.</returns>
				virtual IGuiGraphicsRenderer*			Create()=0;
			};

			enum RenderTargetFailure
			{
				None,
				ResizeWhileRendering,
				LostDevice,
			};

			/// <summary>
			/// This is the interface for graphics renderer targets.
			/// </summary>
			class IGuiGraphicsRenderTarget : public Interface
			{
			public:
				/// <summary>
				/// Notify the target to prepare for rendering.
				/// </summary>
				virtual void							StartRendering()=0;
				/// <summary>
				/// Notify the target to stop rendering.
				/// </summary>
				/// <returns>Returns false to recreate render target.</returns>
				virtual RenderTargetFailure				StopRendering()=0;
				/// <summary>
				/// Apply a clipper to the render target.
				/// The result clipper is combined by all clippers in the clipper stack maintained by the render target.
				/// </summary>
				/// <param name="clipper">The clipper to push.</param>
				virtual void							PushClipper(Rect clipper)=0;
				/// <summary>
				/// Remove the last pushed clipper from the clipper stack.
				/// </summary>
				virtual void							PopClipper()=0;
				/// <summary>
				/// Get the combined clipper
				/// </summary>
				/// <returns>The combined clipper</returns>
				virtual Rect							GetClipper()=0;
				/// <summary>
				/// Test is the combined clipper is as large as the render target.
				/// </summary>
				/// <returns>Return true if the combined clipper is as large as the render target.</returns>
				virtual bool							IsClipperCoverWholeTarget()=0;
			};
		}
	}
}

#endif