# OS Provider

OS provider is the key to make GacUI cross-platform.

If you want to port GacUI to a new platform, this is what you should do:

## Implement INativeWindowService and INativeWindow

These two interfaces is the key of any OS provider.

Typically, an instance of **INativeWindow** controls a window in the target OS. If multiple graphics API is usable (like GDI and Direct2D on Windows), they usually share the same **INativeWindow** implementation.

## Implement Element Renderers in Your Favorite Graphics API

All elements have their own renderer object. When an element is put in a window, a renderer object will be associated to this element. Instances of the same element type have their own renderer object, they don't share renderer objects. So it is very easy to cache necessary system resources in the renderer.

By using **DEFINE_GUI_GRAPHICS_RENDERER** in your renderer class, all boilerplate code will be ready, and you just need to call the static **Register** function.

When an element needs to be renderered, it will find the registered renderer class, create an instance and associate it to the element itself.

## Implement GuiGraphicsResourceManager and IGuiGraphicsRenderTarget

**GuiGraphicsResourceManager** is the core object for rendering. It is a class, but it has a few abstract functions to override. **SetGuiGraphicsResourceManager** is required to register your implementation of **GuiGraphicsResourceManager**.

**IGuiGraphicsRenderTarget** is part of the element interfaces to give all renderer objects access to a window's system resource.

Usually you will also need to create an instance of **INativeControllerListener**, it is a convenient way to track the creating and destroying of **INativeWindow** instances and keep your code decoupled. When a **INativeWindow** is created, usually an implementation of **IGuiGraphicsRenderTarget** will also be created and associated to the window.

**IGuiGraphicsRenderTarget* GuiGraphicsResourceManager::GetRenderTarget(INativeWindow*)** is one of the important function to implement. When a window needs to refresh it self, this function will be called to retrive an implementation of **IGuiGraphicsRenderTarget** from an **INativeWindow**. And the render target will be send to all instances of renderer objects for all elements.

Since in **DEFINE_GUI_GRAPHICS_RENDERER** you have already specify the underlying type that is also created by you to implement **IGuiGraphicsRenderTarget**, whenever a render target is offered to a element renderers, **RenderTargetChangedInternal** will be called with the real render target type. This is a good chance to recreate system resources that depend on this particular render target. After that, the render target will be stored in the protected **renderTarget** field.

For graphics API like DirectX, sometimes the device context will be invalid and the whole thing must be restarted. In this case, a new render target will be created and set to all renderer objects by calling **RenderTargetChangedInternal** again.

## Implement INativeController

**INativeController** consists of multiple services: - **INativeCallbackService**: Managing **INativeControllerListener** objects so that different components can receive system messages and keep the code decoupled. - **INativeAsyncService**: Managing and executing task queues in multiple threads. - **INativeResourceService**: Accessing cursors and fonts from the OS. - **INativeClipboardService**: Accessing clipboard from the OS. - **INativeScreenService**: Accessing physical monitors from the OS. - **INativeDialogService**: Accessing default dialogs from the OS. - **INativeImageService**: Loading image from files or memories. - **INativeWindowService**: Managing windows using OS's window manager. - **INativeInputService**: Accessing global keyboard status and timer.

## Development Note

Implementing **INativeController** is a super complex topic. In order to port GacUI to a new platform, you must know everything about how GacUI interact with the OS. It is highly recommended to read the [Windows](https://github.com/vczh-libraries/GacUI/tree/master/Source/NativeWindow/Windows), [macOS](https://github.com/vczh-libraries/iGac) and [Linux](https://github.com/vczh-libraries/gGac) implementation for **INativeController** before creating yours. Especially there are GDI and Direct2D support in Windows, it is a good example to know how to allow multiple rendering techniques in one OS.

In the future, **INativeHostedController** and **INativeHostedWindow** will be created for scenarios that all GacUI windows are rendered in one OS native window, like CLI, UWP or browsers.

