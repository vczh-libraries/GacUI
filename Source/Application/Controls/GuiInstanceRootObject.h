/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUIINSTANCEROOTOBJECT
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUIINSTANCEROOTOBJECT

#include "../../Resources/GuiResource.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{
			class GuiTemplate;
		}

		namespace controls
		{
			class GuiControlHost;
			class GuiCustomControl;

/***********************************************************************
Component
***********************************************************************/

			class GuiInstanceRootObject;

			/// <summary>
			/// Represnets a component.
			/// </summary>
			class GuiComponent : public Object, public Description<GuiComponent>
			{
			public:
				GuiComponent();
				~GuiComponent();

				virtual void							Attach(GuiInstanceRootObject* rootObject);
				virtual void							Detach(GuiInstanceRootObject* rootObject);
			};

/***********************************************************************
Animation
***********************************************************************/

			/// <summary>Animation.</summary>
			class IGuiAnimation abstract : public virtual IDescriptable, public Description<IGuiAnimation>
			{
			public:
				/// <summary>Called when the animation is about to play the first frame.</summary>
				virtual void							Start() = 0;

				/// <summary>Called when the animation is about to pause.</summary>
				virtual void							Pause() = 0;

				/// <summary>Called when the animation is about to resume.</summary>
				virtual void							Resume() = 0;

				/// <summary>Play the animation. The animation should calculate the time itself to determine the content of the current state of animating objects.</summary>
				virtual void							Run() = 0;

				/// <summary>Test if the animation has ended.</summary>
				/// <returns>Returns true if the animation has ended.</returns>
				virtual bool							GetStopped() = 0;

				/// <summary>Create a finite animation.</summary>
				/// <returns>Returns the created animation.</returns>
				/// <param name="run">The animation callback for each frame.</param>
				/// <param name="milliseconds">The length of the animation.</param>
				static Ptr<IGuiAnimation>				CreateAnimation(const Func<void(vuint64_t)>& run, vuint64_t milliseconds);

				/// <summary>Create an infinite animation.</summary>
				/// <returns>Returns the created animation.</returns>
				/// <param name="run">The animation callback for each frame.</param>
				static Ptr<IGuiAnimation>				CreateAnimation(const Func<void(vuint64_t)>& run);
			};

/***********************************************************************
Root Object
***********************************************************************/

			class RootObjectTimerCallback;

			/// <summary>Represnets a root GUI object.</summary>
			class GuiInstanceRootObject abstract : public Description<GuiInstanceRootObject>
			{
				friend class RootObjectTimerCallback;
				using SubscriptionList = collections::List<Ptr<description::IValueSubscription>>;
				using ObjectMap = collections::Dictionary<WString, reflection::description::Value>;
			protected:
				Ptr<GuiResourcePathResolver>					resourceResolver;
				ObjectMap										namedObjects;
				SubscriptionList								subscriptions;
				collections::SortedList<GuiComponent*>			components;
				Ptr<RootObjectTimerCallback>					timerCallback;
				collections::SortedList<Ptr<IGuiAnimation>>		runningAnimations;
				collections::SortedList<Ptr<IGuiAnimation>>		pendingAnimations;
				bool											finalized = false;

				virtual controls::GuiControlHost*				GetControlHostForInstance() = 0;
				void											InstallTimerCallback(controls::GuiControlHost* controlHost);
				bool											UninstallTimerCallback(controls::GuiControlHost* controlHost);
				void											OnControlHostForInstanceChanged();
				void											StartPendingAnimations();
			public:
				GuiInstanceRootObject();
				~GuiInstanceRootObject();

				/// <summary>Clear all subscriptions and components.</summary>
				void											FinalizeInstance();

				/// <summary>Test has the object been finalized.</summary>
				/// <returns>Returns true if this object has been finalized.</returns>
				bool											IsFinalized();

				void											FinalizeInstanceRecursively(templates::GuiTemplate* thisObject);
				void											FinalizeInstanceRecursively(GuiCustomControl* thisObject);
				void											FinalizeInstanceRecursively(GuiControlHost* thisObject);
				void											FinalizeGeneralInstance(GuiInstanceRootObject* thisObject);

				/// <summary>Set the resource resolver to connect the current root object to the resource creating it.</summary>
				/// <param name="resolver">The resource resolver</param>
				void											SetResourceResolver(Ptr<GuiResourcePathResolver> resolver);
				/// <summary>Resolve a resource using the current resource resolver.</summary>
				/// <returns>The loaded resource. Returns null if failed to load.</returns>
				/// <param name="protocol">The protocol.</param>
				/// <param name="path">The path.</param>
				/// <param name="ensureExist">Set to true and it will throw an exception if the resource doesn't exist.</param>
				Ptr<DescriptableObject>							ResolveResource(const WString& protocol, const WString& path, bool ensureExist);

				/// <summary>Add a subscription. When this control host is disposing, all attached subscriptions will be deleted.</summary>
				/// <returns>Returns null if this operation failed.</returns>
				/// <param name="subscription">The subscription to test.</param>
				Ptr<description::IValueSubscription>			AddSubscription(Ptr<description::IValueSubscription> subscription);
				/// <summary>Clear all subscriptions.</summary>
				void											UpdateSubscriptions();

				/// <summary>Add a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="component">The component to add.</param>
				bool											AddComponent(GuiComponent* component);

				/// <summary>Add a control host as a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="controlHost">The controlHost to add.</param>
				bool											AddControlHostComponent(GuiControlHost* controlHost);

				/// <summary>Add an animation. The animation will be paused if the root object is removed from a window.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="animation">The animation.</param>
				bool											AddAnimation(Ptr<IGuiAnimation> animation);

				/// <summary>Kill an animation.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="animation">The animation.</param>
				bool											KillAnimation(Ptr<IGuiAnimation> animation);

				/// <summary>
				/// Get the object by name, which is set by <see cref="SetNamedObject"/>.
				/// </summary>
				/// <param name="name">The name of the object.</param>
				/// <returns>The object. Returns null if the name is not taken.</returns>
				reflection::description::Value					GetNamedObject(const WString& name);

				/// <summary>
				/// Set an object with a name. If the name has been taken, the previous object will be replaced.
				/// </summary>
				/// <param name="name">The name of the object.</param>
				/// <param name="namedObject">The object.</param>
				void											SetNamedObject(const WString& name, const reflection::description::Value& namedObject);
			};
		}
	}
}

#endif