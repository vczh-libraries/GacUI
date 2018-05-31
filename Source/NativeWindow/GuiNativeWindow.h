/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window

Interfaces:
  INativeController						: Interface for Operating System abstraction

Renderers:
  GUI_GRAPHICS_RENDERER_GDI
  GUI_GRAPHICS_RENDERER_DIRECT2D
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUINATIVEWINDOW
#define VCZH_PRESENTATION_GUINATIVEWINDOW

#include "../GuiTypes.h"

namespace vl
{
	namespace presentation
	{
		using namespace reflection;

		class INativeWindow;
		class INativeWindowListener;
		class INativeController;
		class INativeControllerListener;

/***********************************************************************
System Object
***********************************************************************/

		/// <summary>
		/// Represents a screen.
		/// </summary>
		class INativeScreen : public virtual IDescriptable, Description<INativeScreen>
		{
		public:
			/// <summary>
			/// Get the bounds of the screen.
			/// </summary>
			/// <returns>The bounds of the screen.</returns>
			virtual Rect				GetBounds()=0;
			/// <summary>
			/// Get the bounds of the screen client area.
			/// </summary>
			/// <returns>The bounds of the screen client area.</returns>
			virtual Rect				GetClientBounds()=0;
			/// <summary>
			/// Get the name of the screen.
			/// </summary>
			/// <returns>The name of the screen.</returns>
			virtual WString				GetName()=0;
			/// <summary>
			/// Test is the screen is a primary screen.
			/// </summary>
			/// <returns>Returns true if the screen is a primary screen.</returns>
			virtual bool				IsPrimary()=0;
		};
		
		/// <summary>
		/// Represents a cursor.
		/// </summary>
		class INativeCursor : public virtual IDescriptable, Description<INativeCursor>
		{
		public:
			/// <summary>
			/// Represents a predefined cursor type.
			/// </summary>
			enum SystemCursorType
			{
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]Small waiting cursor.
				/// </summary>
				SmallWaiting,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]large waiting cursor.
				/// </summary>
				LargeWaiting,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]Arrow cursor.
				/// </summary>
				Arrow,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]Cross cursor.
				/// </summary>
				Cross,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]Hand cursor.
				/// </summary>
				Hand,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]Help cursor.
				/// </summary>
				Help,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]I beam cursor.
				/// </summary>
				IBeam,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]Sizing in all direction cursor.
				/// </summary>
				SizeAll,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]Sizing NE-SW cursor.
				/// </summary>
				SizeNESW,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]Sizing N-S cursor.
				/// </summary>
				SizeNS,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]Sizing NW-SE cursor.
				/// </summary>
				SizeNWSE,
				/// <summary>
				/// [T:vl.presentation.INativeCursor.SystemCursorType]Sizing W-E cursor.
				/// </summary>
				SizeWE,
				LastSystemCursor=SizeWE,
			};

			static const vint			SystemCursorCount=LastSystemCursor+1;
		public:
			/// <summary>
			/// Test is the cursor a system provided cursor.
			/// </summary>
			/// <returns>Returns true if the cursor a system provided cursor.</returns>
			virtual bool				IsSystemCursor()=0;
			/// <summary>
			/// Get the cursor type if the cursor a system provided cursor.
			/// </summary>
			/// <returns>The cursor type.</returns>
			virtual SystemCursorType	GetSystemCursorType()=0;
		};

/***********************************************************************
Image Object
***********************************************************************/

		class INativeImageService;
		class INativeImage;
		class INativeImageFrame;
		
		/// <summary>
		/// Represents a customized cache object for an image frame.
		/// </summary>
		class INativeImageFrameCache : public Interface
		{
		public:
			/// <summary>
			/// Called when this cache object is attached to an image frame.
			/// </summary>
			/// <param name="frame">The image frame that attached to.</param>
			virtual void						OnAttach(INativeImageFrame* frame)=0;
			/// <summary>
			/// Called when this cache object is detached to an image frame.
			/// </summary>
			/// <param name="frame">The image frame that detached from.</param>
			virtual void						OnDetach(INativeImageFrame* frame)=0;
		};

		/// <summary>
		/// Represents an image frame.
		/// </summary>
		class INativeImageFrame : public virtual IDescriptable, public Description<INativeImageFrame>
		{
		public:
			/// <summary>
			/// Get the image that owns this frame.
			/// </summary>
			/// <returns>The image that owns this frame.</returns>
			virtual INativeImage*				GetImage()=0;
			/// <summary>
			/// Get the size of this frame.
			/// </summary>
			/// <returns>The size of this frame.</returns>
			virtual Size						GetSize()=0;

			/// <summary>
			/// Attach a customized cache object to this image frame and bind to a key.
			/// </summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="key">The key binded with the customized cache object.</param>
			/// <param name="cache">The customized cache object.</param>
			virtual bool						SetCache(void* key, Ptr<INativeImageFrameCache> cache)=0;
			/// <summary>
			/// Get the attached customized cache object that is already binded to a key.
			/// </summary>
			/// <returns>The attached customized cache object.</returns>
			/// <param name="key">The key binded with the customized cache object.</param>
			virtual Ptr<INativeImageFrameCache>	GetCache(void* key)=0;
			/// <summary>
			/// Get the attached customized cache object that is already binded to a key, and then detach it.
			/// </summary>
			/// <returns>The detached customized cache object.</returns>
			/// <param name="key">The key binded with the customized cache object.</param>
			virtual Ptr<INativeImageFrameCache>	RemoveCache(void* key)=0;
		};
		
		/// <summary>
		/// Represents an image.
		/// </summary>
		class INativeImage : public virtual IDescriptable, public Description<INativeImage>
		{
		public:
			/// <summary>
			/// Represents an image format.
			/// </summary>
			enum FormatType
			{
				/// <summary>
				/// [T:vl.presentation.INativeImage.FormatType]Bitmap format.
				/// </summary>
				Bmp,
				/// <summary>
				/// [T:vl.presentation.INativeImage.FormatType]GIF format.
				/// </summary>
				Gif,
				/// <summary>
				/// [T:vl.presentation.INativeImage.FormatType]Icon format.
				/// </summary>
				Icon,
				/// <summary>
				/// [T:vl.presentation.INativeImage.FormatType]JPEG format.
				/// </summary>
				Jpeg,
				/// <summary>
				/// [T:vl.presentation.INativeImage.FormatType]PNG format.
				/// </summary>
				Png,
				/// <summary>
				/// [T:vl.presentation.INativeImage.FormatType]TIFF format.
				/// </summary>
				Tiff,
				/// <summary>
				/// [T:vl.presentation.INativeImage.FormatType]WMP format.
				/// </summary>
				Wmp,
				/// <summary>
				/// [T:vl.presentation.INativeImage.FormatType]Unknown format.
				/// </summary>
				Unknown,
			};
			
			/// <summary>
			/// Get the image service that creates this image.
			/// </summary>
			/// <returns>The image service that creates this image.</returns>
			virtual INativeImageService*		GetImageService()=0;
			/// <summary>
			/// Get the image format.
			/// </summary>
			/// <returns>The image format.</returns>
			virtual FormatType					GetFormat()=0;
			/// <summary>
			/// Get the number of frames in this image.
			/// </summary>
			/// <returns>The number of frames in this image.</returns>
			virtual vint						GetFrameCount()=0;
			/// <summary>
			/// Get the frame in this image by a specified frame index.
			/// </summary>
			/// <returns>The frame in this image by a specified frame index.</returns>
			/// <param name="index">The specified frame index.</param>
			virtual INativeImageFrame*			GetFrame(vint index)=0;
		};
		
		/// <summary>
		/// Image service. To access this service, use [M:vl.presentation.INativeController.ImageService].
		/// </summary>
		class INativeImageService : public virtual IDescriptable, public Description<INativeImageService>
		{
		public:
			/// <summary>
			/// Create an image from file.
			/// </summary>
			/// <returns>The created image.</returns>
			/// <param name="path">The file path.</param>
			virtual Ptr<INativeImage>			CreateImageFromFile(const WString& path)=0;

			/// <summary>
			/// Create an image from memory.
			/// </summary>
			/// <returns>The created image.</returns>
			/// <param name="buffer">The memory pointer.</param>
			/// <param name="length">The memory length.</param>
			virtual Ptr<INativeImage>			CreateImageFromMemory(void* buffer, vint length)=0;

			/// <summary>
			/// Create an image from stream.
			/// </summary>
			/// <returns>The created image.</returns>
			/// <param name="stream">The stream.</param>
			virtual Ptr<INativeImage>			CreateImageFromStream(stream::IStream& stream)=0;
		};

/***********************************************************************
Native Window
***********************************************************************/
		
		/// <summary>
		/// Represents a window.
		/// </summary>
		class INativeWindow : public Interface, public Description<INativeWindow>
		{
		public:
			/// <summary>
			/// Get the bounds of the window.
			/// </summary>
			/// <returns>The bounds of the window.</returns>
			virtual Rect				GetBounds()=0;
			/// <summary>
			/// Set the bounds of the window.
			/// </summary>
			/// <param name="bounds">The bounds of the window.</param>
			virtual void				SetBounds(const Rect& bounds)=0;
			/// <summary>
			/// Get the client size of the window.
			/// </summary>
			/// <returns>The client size of the window.</returns>
			virtual Size				GetClientSize()=0;
			/// <summary>
			/// Set the client size of the window.
			/// </summary>
			/// <param name="size">The client size of the window.</param>
			virtual void				SetClientSize(Size size)=0;
			/// <summary>
			/// Get the client bounds in screen space.
			/// </summary>
			/// <returns>The client bounds in screen space.</returns>
			virtual Rect				GetClientBoundsInScreen()=0;
			
			/// <summary>
			/// Get the title of the window. A title will be displayed as a name of this window.
			/// </summary>
			/// <returns>The title of the window.</returns>
			virtual WString				GetTitle()=0;
			/// <summary>
			/// Set the title of the window. A title will be displayed as a name of this window.
			/// </summary>
			/// <param name="title">The title of the window.</param>
			virtual void				SetTitle(WString title)=0;
			/// <summary>
			/// Get the mouse cursor of the window. When the mouse is on the window, the mouse cursor will be rendered.
			/// </summary>
			/// <returns>The mouse cursor of the window.</returns>
			virtual INativeCursor*		GetWindowCursor()=0;
			/// <summary>
			/// Set the mouse cursor of the window. When the mouse is on the window, the mouse cursor will be rendered.
			/// </summary>
			/// <param name="cursor">The mouse cursor of the window.</param>
			virtual void				SetWindowCursor(INativeCursor* cursor)=0;
			/// <summary>
			/// Get the caret point of the window. When an input method editor is opened, the input text box will be located to the caret point.
			/// </summary>
			/// <returns>The caret point of the window.</returns>
			virtual Point				GetCaretPoint()=0;
			/// <summary>
			/// Set the caret point of the window. When an input method editor is opened, the input text box will be located to the caret point.
			/// </summary>
			/// <param name="point">The caret point of the window.</param>
			virtual void				SetCaretPoint(Point point)=0;
			
			/// <summary>
			/// Get the parent window. A parent window doesn't contain a child window. It always displayed below the child windows. When a parent window is minimized or restored, so as its child windows.
			/// </summary>
			/// <returns>The parent window.</returns>
			virtual INativeWindow*		GetParent()=0;
			/// <summary>
			/// Set the parent window. A parent window doesn't contain a child window. It always displayed below the child windows. When a parent window is minimized or restored, so as its child windows.
			/// </summary>
			/// <param name="parent">The parent window.</param>
			virtual void				SetParent(INativeWindow* parent)=0;
			/// <summary>
			/// Test is the window always pass the focus to its parent window.
			/// </summary>
			/// <returns>Returns true if the window always pass the focus to its parent window.</returns>
			virtual bool				GetAlwaysPassFocusToParent()=0;
			/// <summary>
			/// Enable or disble always passing the focus to its parent window.
			/// </summary>
			/// <param name="value">True to enable always passing the focus to its parent window.</param>
			virtual void				SetAlwaysPassFocusToParent(bool value)=0;

			/// <summary>
			/// Enable the window customized frame mode.
			/// </summary>
			virtual void				EnableCustomFrameMode()=0;
			/// <summary>
			/// Disable the window customized frame mode.
			/// </summary>
			virtual void				DisableCustomFrameMode()=0;
			/// <summary>
			/// Test is the window customized frame mode enabled.
			/// </summary>
			/// <returns>Returns true if the window customized frame mode is enabled.</returns>
			virtual bool				IsCustomFrameModeEnabled()=0;

			/// <summary>Window size state.</summary>
			enum WindowSizeState
			{
				/// <summary>Minimized.</summary>
				Minimized,
				/// <summary>Restored.</summary>
				Restored,
				/// <summary>Maximized.</summary>
				Maximized,
			};

			/// <summary>
			/// Get the window size state.
			/// </summary>
			/// <returns>Returns the window size state.</returns>
			virtual WindowSizeState		GetSizeState()=0;
			/// <summary>
			/// Show the window.
			/// </summary>
			virtual void				Show()=0;
			/// <summary>
			/// Show the window without activation.
			/// </summary>
			virtual void				ShowDeactivated()=0;
			/// <summary>
			/// Restore the window.
			/// </summary>
			virtual void				ShowRestored()=0;
			/// <summary>
			/// Maximize the window.
			/// </summary>
			virtual void				ShowMaximized()=0;
			/// <summary>
			/// Minimize the window.
			/// </summary>
			virtual void				ShowMinimized()=0;
			/// <summary>
			/// Hide the window.
			/// </summary>
			virtual void				Hide()=0;
			/// <summary>
			/// Test is the window visible.
			/// </summary>
			/// <returns>Returns true if the window is visible.</returns>
			virtual bool				IsVisible()=0;

			/// <summary>
			/// Enable the window.
			/// </summary>
			virtual void				Enable()=0;
			/// <summary>
			/// Disable the window.
			/// </summary>
			virtual void				Disable()=0;
			/// <summary>
			/// Test is the window enabled.
			/// </summary>
			/// <returns>Returns true if the window is enabled.</returns>
			virtual bool				IsEnabled()=0;
			
			/// <summary>
			/// Set focus to the window.
			/// </summary>
			virtual void				SetFocus()=0;
			/// <summary>
			/// Test is the window focused.
			/// </summary>
			/// <returns>Returns true if the window is focused.</returns>
			virtual bool				IsFocused()=0;
			/// <summary>
			/// Activate to the window.
			/// </summary>
			virtual void				SetActivate()=0;
			/// <summary>
			/// Test is the window activated.
			/// </summary>
			/// <returns>Returns true if the window is activated.</returns>
			virtual bool				IsActivated()=0;
			
			/// <summary>
			/// Show the icon in the task bar.
			/// </summary>
			virtual void				ShowInTaskBar()=0;
			/// <summary>
			/// Hide the icon in the task bar.
			/// </summary>
			virtual void				HideInTaskBar()=0;
			/// <summary>
			/// Test is the window icon appeared in the task bar.
			/// </summary>
			/// <returns>Returns true if the window icon appears in the task bar.</returns>
			virtual bool				IsAppearedInTaskBar()=0;
			
			/// <summary>
			/// Enable activation to the window.
			/// </summary>
			virtual void				EnableActivate()=0;
			/// <summary>
			/// Disable activation to the window.
			/// </summary>
			virtual void				DisableActivate()=0;
			/// <summary>
			/// Test is the window allowed to be activated.
			/// </summary>
			/// <returns>Returns true if the window is allowed to be activated.</returns>
			virtual bool				IsEnabledActivate()=0;
			
			/// <summary>
			/// Require mouse message capturing to this window. If the capture is required, all mouse message will be send to this window.
			/// </summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			virtual bool				RequireCapture()=0;
			/// <summary>
			/// Release mouse message capturing to this window. If the capture is required, all mouse message will be send to this window.
			/// </summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			virtual bool				ReleaseCapture()=0;
			/// <summary>
			/// Test if the window is capturing mouse messages.
			/// </summary>
			/// <returns>Returns true if the window is capturing mouse messages.</returns>
			virtual bool				IsCapturing()=0;

			/// <summary>
			/// Test is the maximize box visible.
			/// </summary>
			/// <returns>Returns true if the maximize box is visible.</returns>
			virtual bool				GetMaximizedBox()=0;
			/// <summary>
			/// Make the maximize box visible or invisible.
			/// </summary>
			/// <param name="visible">True to make the maximize box visible.</param>
			virtual void				SetMaximizedBox(bool visible)=0;
			/// <summary>
			/// Test is the minimize box visible.
			/// </summary>
			/// <returns>Returns true if the minimize box is visible.</returns>
			virtual bool				GetMinimizedBox()=0;
			/// <summary>
			/// Make the minimize box visible or invisible.
			/// </summary>
			/// <param name="visible">True to make the minimize box visible.</param>
			virtual void				SetMinimizedBox(bool visible)=0;
			/// <summary>
			/// Test is the border visible.
			/// </summary>
			/// <returns>Returns true if the border is visible.</returns>
			virtual bool				GetBorder()=0;
			/// <summary>
			/// Make the border visible or invisible.
			/// </summary>
			/// <param name="visible">True to make the border visible.</param>
			virtual void				SetBorder(bool visible)=0;
			/// <summary>
			/// Test is the size box visible.
			/// </summary>
			/// <returns>Returns true if the size box is visible.</returns>
			virtual bool				GetSizeBox()=0;
			/// <summary>
			/// Make the size box visible or invisible.
			/// </summary>
			/// <param name="visible">True to make the size box visible.</param>
			virtual void				SetSizeBox(bool visible)=0;
			/// <summary>
			/// Test is the icon visible.
			/// </summary>
			/// <returns>Returns true if the icon is visible.</returns>
			virtual bool				GetIconVisible()=0;
			/// <summary>
			/// Make the icon visible or invisible.
			/// </summary>
			/// <param name="visible">True to make the icon visible.</param>
			virtual void				SetIconVisible(bool visible)=0;
			/// <summary>
			/// Test is the title bar visible.
			/// </summary>
			/// <returns>Returns true if the title bar is visible.</returns>
			virtual bool				GetTitleBar()=0;
			/// <summary>
			/// Make the title bar visible or invisible.
			/// </summary>
			/// <param name="visible">True to make the title bar visible.</param>
			virtual void				SetTitleBar(bool visible)=0;
			/// <summary>
			/// Test is the window always on top of the desktop.
			/// </summary>
			/// <returns>Returns true if the window is always on top of the desktop.</returns>
			virtual bool				GetTopMost()=0;
			/// <summary>
			/// Make the window always or never on top of the desktop.
			/// </summary>
			/// <param name="topmost">True to make the window always  on top of the desktop.</param>
			virtual void				SetTopMost(bool topmost)=0;

			/// <summary>
			/// Supress the system's Alt+X hot key
			/// </summary>
			virtual void				SupressAlt() = 0;
			
			/// <summary>
			/// Install an message listener.
			/// </summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="listener">The listener to install.</param>
			virtual bool				InstallListener(INativeWindowListener* listener)=0;
			/// <summary>
			/// Uninstall an message listener.
			/// </summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="listener">The listener to uninstall.</param>
			virtual bool				UninstallListener(INativeWindowListener* listener)=0;
			/// <summary>
			/// Redraw the content of the window.
			/// </summary>
			virtual void				RedrawContent()=0;
		};

		/// <summary>
		/// Mouse message information.
		/// </summary>
		struct NativeWindowMouseInfo
		{
			/// <summary>True if the control button is pressed.</summary>
			bool						ctrl;
			/// <summary>True if the shift button is pressed.</summary>
			bool						shift;
			/// <summary>True if the left mouse button is pressed.</summary>
			bool						left;
			/// <summary>True if the middle mouse button is pressed.</summary>
			bool						middle;
			/// <summary>True if the right mouse button is pressed.</summary>
			bool						right;
			/// <summary>The mouse position of x dimension.</summary>
			vint						x;
			/// <summary>The mouse position of y dimension.</summary>
			vint						y;
			/// <summary>The delta of the wheel.</summary>
			vint						wheel;
			/// <summary>True if the mouse is in the non-client area.</summary>
			bool						nonClient;
		};
		
		/// <summary>
		/// Key message information.
		/// </summary>
		struct NativeWindowKeyInfo
		{
			/// <summary>Key code of the key that sends this message, using VKEY_* macros.</summary>
			vint						code;
			/// <summary>True if the control button is pressed.</summary>
			bool						ctrl;
			/// <summary>True if the shift button is pressed.</summary>
			bool						shift;
			/// <summary>True if the alt button is pressed.</summary>
			bool						alt;
			/// <summary>True if the capslock button is pressed.</summary>
			bool						capslock;
		};
		
		/// <summary>
		/// Character message information.
		/// </summary>
		struct NativeWindowCharInfo
		{
			/// <summary>Character that sends this message.</summary>
			wchar_t						code;
			/// <summary>True if the control button is pressed.</summary>
			bool						ctrl;
			/// <summary>True if the shift button is pressed.</summary>
			bool						shift;
			/// <summary>True if the alt button is pressed.</summary>
			bool						alt;
			/// <summary>True if the capslock button is pressed.</summary>
			bool						capslock;
		};
		
		/// <summary>
		/// Represents a message listener to an <see cref="INativeWindow"/>.
		/// </summary>
		class INativeWindowListener : public Interface
		{
		public:
			/// <summary>Hit test result for a native window.</summary>
			enum HitTestResult
			{
				/// <summary>Border that doesn't contain sizing functionalitiy.</summary>
				BorderNoSizing,
				/// <summary>Left border.</summary>
				BorderLeft,
				/// <summary>Right border.</summary>
				BorderRight,
				/// <summary>Top border.</summary>
				BorderTop,
				/// <summary>Bottom border.</summary>
				BorderBottom,
				/// <summary>Left top border.</summary>
				BorderLeftTop,
				/// <summary>Right top border.</summary>
				BorderRightTop,
				/// <summary>Left bottom border.</summary>
				BorderLeftBottom,
				/// <summary>Right bottom border.</summary>
				BorderRightBottom,
				/// <summary>Title</summary>
				Title,
				/// <summary>Minimum button.</summary>
				ButtonMinimum,
				/// <summary>Maximum button.</summary>
				ButtonMaximum,
				/// <summary>Close button.</summary>
				ButtonClose,
				/// <summary>Client button.</summary>
				Client,
				/// <summary>Icon.</summary>
				Icon,
				/// <summary>Let the OS window layer decide.</summary>
				NoDecision,
			};

			/// <summary>
			/// Perform a hit test.
			/// </summary>
			/// <returns>Returns the hit test result. If "NoDecision" is returned, the native window provider should call the OS window layer to do the hit test.</returns>
			/// <param name="location">The location to do the hit test. This location is in the window space (not the client space).</param>
			virtual HitTestResult		HitTest(Point location);
			/// <summary>
			/// Called when the window is moving.
			/// </summary>
			/// <param name="bounds">The bounds. Message handler can change the bounds.</param>
			/// <param name="fixSizeOnly">True if the message raise only want the message handler to change the size.</param>
			virtual void				Moving(Rect& bounds, bool fixSizeOnly);
			/// <summary>
			/// Called when the window is moved.
			/// </summary>
			virtual void				Moved();
			/// <summary>
			/// Called when the window is enabled.
			/// </summary>
			virtual void				Enabled();
			/// <summary>
			/// Called when the window is disabled.
			/// </summary>
			virtual void				Disabled();
			/// <summary>
			/// Called when the window got the focus.
			/// </summary>
			virtual void				GotFocus();
			/// <summary>
			/// Called when the window lost the focus.
			/// </summary>
			virtual void				LostFocus();
			/// <summary>
			/// Called when the window is activated.
			/// </summary>
			virtual void				Activated();
			/// <summary>
			/// Called when the window is deactivated.
			/// </summary>
			virtual void				Deactivated();
			/// <summary>
			/// Called when the window is opened.
			/// </summary>
			virtual void				Opened();
			/// <summary>
			/// Called when the window is closing.
			/// </summary>
			/// <param name="cancel">Change the value to true to prevent the windows from being closed.</param>
			virtual void				Closing(bool& cancel);
			/// <summary>
			/// Called when the window is closed.
			/// </summary>
			virtual void				Closed();
			/// <summary>
			/// Called when the window is painting.
			/// </summary>
			virtual void				Paint();
			/// <summary>
			/// Called when the window is destroying.
			/// </summary>
			virtual void				Destroying();
			/// <summary>
			/// Called when the window is destroyed.
			/// </summary>
			virtual void				Destroyed();
			
			/// <summary>
			/// Called when the left mouse button is pressed.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				LeftButtonDown(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the left mouse button is released.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				LeftButtonUp(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the left mouse button performed a double click.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				LeftButtonDoubleClick(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the right mouse button is pressed.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				RightButtonDown(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the right mouse button is released.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				RightButtonUp(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the right mouse button performed a double click.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				RightButtonDoubleClick(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the middle mouse button is pressed.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				MiddleButtonDown(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the middle mouse button is released.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				MiddleButtonUp(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the middle mouse button performed a double click.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				MiddleButtonDoubleClick(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the horizontal mouse wheel scrolls.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				HorizontalWheel(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the horizontal vertical wheel scrolls.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				VerticalWheel(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the mouse is moving on the window.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				MouseMoving(const NativeWindowMouseInfo& info);
			/// <summary>
			/// Called when the mouse entered the window.
			/// </summary>
			virtual void				MouseEntered();
			/// <summary>
			/// Called when the mouse leaved the window.
			/// </summary>
			virtual void				MouseLeaved();
			
			/// <summary>
			/// Called a key is pressed.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				KeyDown(const NativeWindowKeyInfo& info);
			/// <summary>
			/// Called a key is released.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				KeyUp(const NativeWindowKeyInfo& info);
			/// <summary>
			/// Called a system key is pressed.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				SysKeyDown(const NativeWindowKeyInfo& info);
			/// <summary>
			/// Called a system key is released.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				SysKeyUp(const NativeWindowKeyInfo& info);
			/// <summary>
			/// Called an input character is generated.
			/// </summary>
			/// <param name="info">Detailed information to this message.</param>
			virtual void				Char(const NativeWindowCharInfo& info);
		};

/***********************************************************************
Native Window Services
***********************************************************************/

		/// <summary>
		/// System resource service. To access this service, use [M:vl.presentation.INativeController.ResourceService].
		/// </summary>
		class INativeResourceService : public virtual IDescriptable, public Description<INativeResourceService>
		{
		public:
			/// <summary>
			/// Get a cached cursor object using a predefined system cursor type;
			/// </summary>
			/// <returns>The cached cursor object.</returns>
			/// <param name="type">The predefined system cursor type.</param>
			virtual INativeCursor*			GetSystemCursor(INativeCursor::SystemCursorType type)=0;
			/// <summary>
			/// Get a cached cursor object using a default system cursor type;
			/// </summary>
			/// <returns>The cached cursor object.</returns>
			virtual INativeCursor*			GetDefaultSystemCursor()=0;

			/// <summary>
			/// Get the default font configuration of the system.
			/// </summary>
			/// <returns>The default font configuration of the system.</returns>
			virtual FontProperties			GetDefaultFont()=0;
			/// <summary>
			/// Override the default font configuration for the current process, only available GacUI library.
			/// </summary>
			/// <param name="value">The font configuration to override.</param>
			virtual void					SetDefaultFont(const FontProperties& value)=0;
		};

		/// <summary>
		/// Delay execution controller.
		/// </summary>
		class INativeDelay : public virtual IDescriptable, public Description<INativeDelay>
		{
		public:
			/// <summary>Delay execution controller status.</summary>
			enum ExecuteStatus
			{
				/// <summary>Pending.</summary>
				Pending,
				/// <summary>Executing.</summary>
				Executing,
				/// <summary>Executed.</summary>
				Executed,
				/// <summary>Canceled.</summary>
				Canceled,
			};

			/// <summary>Get the current status.</summary>
			/// <returns>The current status.</returns>
			virtual ExecuteStatus			GetStatus()=0;
			/// <summary>If the current task is pending, execute the task after a specified period.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="milliseconds">A specified period.</param>
			virtual bool					Delay(vint milliseconds)=0;
			/// <summary>If the current task is pending, cancel the task.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			virtual bool					Cancel()=0;
		};
		
		/// <summary>
		/// Asynchronized operation service. GacUI is not a thread safe library except for this service. To access this service, use [M:vl.presentation.INativeController.AsyncService].
		/// </summary>
		class INativeAsyncService : public virtual IDescriptable, public Description<INativeAsyncService>
		{
		public:

			/// <summary>
			/// Test is the current thread the main thread.
			/// </summary>
			/// <returns>Returns true if the current thread is the main thread.</returns>
			/// <param name="window">A window to access the corressponding main thread.</param>
			virtual bool					IsInMainThread(INativeWindow* window)=0;
			/// <summary>
			/// Invoke a specified function with an specified argument asynchronisly.
			/// </summary>
			/// <param name="proc">The specified function.</param>
			virtual void					InvokeAsync(const Func<void()>& proc)=0;
			/// <summary>
			/// Invoke a specified function with an specified argument in the main thread.
			/// </summary>
			/// <param name="window">A window to access the corressponding main thread.</param>
			/// <param name="proc">The specified function.</param>
			virtual void					InvokeInMainThread(INativeWindow* window, const Func<void()>& proc)=0;
			/// <summary>
			/// Invoke a specified function with an specified argument in the main thread and wait for the function to complete or timeout.
			/// </summary>
			/// <returns>Return true if the function complete. Return false if the function has not completed during a specified period of time.</returns>
			/// <param name="window">A window to access the corressponding main thread.</param>
			/// <param name="proc">The specified function.</param>
			/// <param name="milliseconds">The specified period of time to wait. Set to -1 (default value) to wait forever until the function completed.</param>
			virtual bool					InvokeInMainThreadAndWait(INativeWindow* window, const Func<void()>& proc, vint milliseconds=-1)=0;
			/// <summary>
			/// Delay execute a specified function with an specified argument asynchronisly.
			/// </summary>
			/// <returns>The Delay execution controller for this task.</returns>
			/// <param name="proc">The specified function.</param>
			/// <param name="milliseconds">Time to delay.</param>
			virtual Ptr<INativeDelay>		DelayExecute(const Func<void()>& proc, vint milliseconds)=0;
			/// <summary>
			/// Delay execute a specified function with an specified argument in the main thread.
			/// </summary>
			/// <returns>The Delay execution controller for this task.</returns>
			/// <param name="proc">The specified function.</param>
			/// <param name="milliseconds">Time to delay.</param>
			virtual Ptr<INativeDelay>		DelayExecuteInMainThread(const Func<void()>& proc, vint milliseconds)=0;
		};
		
		/// <summary>
		/// Clipboard service. To access this service, use [M:vl.presentation.INativeController.ClipboardService].
		/// </summary>
		class INativeClipboardService : public virtual IDescriptable, public Description<INativeClipboardService>
		{
		public:
			/// <summary>
			/// Test is there a text in the clipboard.
			/// </summary>
			/// <returns>Returns true if there is a text in the clipboard.</returns>
			virtual bool					ContainsText()=0;
			/// <summary>
			/// Get the text in the clipboard.
			/// </summary>
			/// <returns>The text in the clipboard.</returns>
			virtual WString					GetText()=0;
			/// <summary>
			/// Copy the text to the clipboard.
			/// </summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="value">The text to copy to the clipboard.</param>
			virtual bool					SetText(const WString& value)=0;
		};
		
		/// <summary>
		/// Screen information service. To access this service, use [M:vl.presentation.INativeController.ScreenService].
		/// </summary>
		class INativeScreenService : public virtual IDescriptable, public Description<INativeScreenService>
		{
		public:
			/// <summary>
			/// Get the number of all available screens.
			/// </summary>
			///  <returns>The number of all available screens.</returns>
			virtual vint					GetScreenCount()=0;
			/// <summary>
			/// Get the screen object by a specified screen index.
			/// </summary>
			/// <returns>The screen object.</returns>
			/// <param name="index">The specified screen index.</param>
			virtual INativeScreen*			GetScreen(vint index)=0;
			/// <summary>
			/// Get the screen object where the main part of the specified window is inside.
			/// </summary>
			/// <returns>The screen object.</returns>
			/// <param name="window">The specified window.</param>
			virtual INativeScreen*			GetScreen(INativeWindow* window)=0;
		};
		
		/// <summary>
		/// Window service. To access this service, use [M:vl.presentation.INativeController.WindowService].
		/// </summary>
		class INativeWindowService : public virtual Interface
		{
		public:
			/// <summary>
			/// Create a window.
			/// </summary>
			/// <returns>The created window.</returns>
			virtual INativeWindow*			CreateNativeWindow()=0;
			/// <summary>
			/// Destroy a window.
			/// </summary>
			/// <param name="window">The window to destroy.</param>
			virtual void					DestroyNativeWindow(INativeWindow* window)=0;
			/// <summary>
			/// Get the main window.
			/// </summary>
			/// <returns>The main window.</returns>
			virtual INativeWindow*			GetMainWindow()=0;
			/// <summary>
			/// Get the window that under a specified position in screen space.
			/// </summary>
			/// <returns>The window that under a specified position in screen space.</returns>
			/// <param name="location">The specified position in screen space.</param>
			virtual INativeWindow*			GetWindow(Point location)=0;
			/// <summary>
			/// Make the specified window a main window, show that window, and wait until the windows is closed.
			/// </summary>
			/// <param name="window">The specified window.</param>
			virtual void					Run(INativeWindow* window)=0;
		};
		
		/// <summary>
		/// User input service. To access this service, use [M:vl.presentation.INativeController.InputService].
		/// </summary>
		class INativeInputService : public virtual IDescriptable, public Description<INativeInputService>
		{
		public:
			/// <summary>
			/// Start to reveive global mouse message.
			/// </summary>
			virtual void					StartHookMouse()=0;
			/// <summary>
			/// Stop to receive global mouse message.
			/// </summary>
			virtual void					StopHookMouse()=0;
			/// <summary>
			/// Test is the global mouse message receiving enabled.
			/// </summary>
			/// <returns>Returns true if the global mouse message receiving is enabled.</returns>
			virtual bool					IsHookingMouse()=0;
			
			/// <summary>
			/// Start to reveive global timer message.
			/// </summary>
			virtual void					StartTimer()=0;
			/// <summary>
			/// Stop to receive global timer message.
			/// </summary>
			virtual void					StopTimer()=0;
			/// <summary>
			/// Test is the global timer message receiving enabled.
			/// </summary>
			/// <returns>Returns true if the global timer message receiving is enabled.</returns>
			virtual bool					IsTimerEnabled()=0;
			
			/// <summary>
			/// Test is the specified key pressing.
			/// </summary>
			/// <returns>Returns true if the specified key is pressing.</returns>
			/// <param name="code">The key code to test, using VKEY_* macros.</param>
			virtual bool					IsKeyPressing(vint code)=0;
			/// <summary>
			/// Test is the specified key toggled.
			/// </summary>
			/// <returns>Returns true if the specified key is toggled.</returns>
			/// <param name="code">The key code to test, using VKEY_* macros.</param>
			virtual bool					IsKeyToggled(vint code)=0;

			/// <summary>
			/// Get the name of a key.
			/// </summary>
			/// <returns>The name of a key.</returns>
			/// <param name="code">The key code, using VKEY_* macros.</param>
			virtual WString					GetKeyName(vint code)=0;
			/// <summary>
			/// Get the key from a name.
			/// </summary>
			/// <returns>The key, returns -1 if the key name doesn't exist.</returns>
			/// <param name="name">Key name</param>
			virtual vint					GetKey(const WString& name)=0;
		};
		
		/// <summary>
		/// Callback service. To access this service, use [M:vl.presentation.INativeController.CallbackService].
		/// </summary>
		class INativeCallbackService : public virtual Interface
		{
		public:
			/// <summary>
			/// Install a global message listener.
			/// </summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="listener">The global message listener to install.</param>
			virtual bool					InstallListener(INativeControllerListener* listener)=0;
			/// <summary>
			/// Uninstall a global message listener.
			/// </summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="listener">The global message listener to uninstall.</param>
			virtual bool					UninstallListener(INativeControllerListener* listener)=0;
		};


		/// <summary>
		/// Dialog service. To access this service, use [M:vl.presentation.INativeController.DialogService].
		/// </summary>
		class INativeDialogService : public virtual Interface
		{
		public:
			/// <summary>
			/// Message box button combination for displaying a message box.
			/// </summary>
			enum MessageBoxButtonsInput
			{
				/// <summary>Display OK.</summary>
				DisplayOK,
				/// <summary>Display OK, Cancel.</summary>
				DisplayOKCancel,
				/// <summary>Display Yes, No.</summary>
				DisplayYesNo,
				/// <summary>Display Yes, No, Cancel.</summary>
				DisplayYesNoCancel,
				/// <summary>Display Retry, Cancel.</summary>
				DisplayRetryCancel,
				/// <summary>Display Abort, Retry, Ignore.</summary>
				DisplayAbortRetryIgnore,
				/// <summary>Display Cancel, TryAgain, Continue.</summary>
				DisplayCancelTryAgainContinue,
			};

			/// <summary>
			/// Message box button to indicate what the user selected.
			/// </summary>
			enum MessageBoxButtonsOutput
			{
				/// <summary>Select OK.</summary>
				SelectOK,
				/// <summary>Select Cancel.</summary>
				SelectCancel,
				/// <summary>Select Yes.</summary>
				SelectYes,
				/// <summary>Select No.</summary>
				SelectNo,
				/// <summary>Select Retry.</summary>
				SelectRetry,
				/// <summary>Select Abort.</summary>
				SelectAbort,
				/// <summary>Select Ignore.</summary>
				SelectIgnore,
				/// <summary>Select TryAgain.</summary>
				SelectTryAgain,
				/// <summary>Select Continue.</summary>
				SelectContinue,
			};

			/// <summary>
			/// Message box default button.
			/// </summary>
			enum MessageBoxDefaultButton
			{
				/// <summary>First.</summary>
				DefaultFirst,
				/// <summary>Second.</summary>
				DefaultSecond,
				/// <summary>Third.</summary>
				DefaultThird,
			};

			/// <summary>
			/// Message box icons.
			/// </summary>
			enum MessageBoxIcons
			{
				/// <summary>No icon.</summary>
				IconNone,
				/// <summary>Error icon.</summary>
				IconError,
				/// <summary>Question icon.</summary>
				IconQuestion,
				/// <summary>Warning icon.</summary>
				IconWarning,
				/// <summary>Information icon.</summary>
				IconInformation,
			};

			/// <summary>
			/// Message box model options.
			/// </summary>
			enum MessageBoxModalOptions
			{
				/// <summary>Disable the current window.</summary>
				ModalWindow,
				/// <summary>Disable all windows in the application.</summary>
				ModalTask,
				/// <summary>Top most message box in the whole system.</summary>
				ModalSystem,
			};

			/// <summary>Show a message box.</summary>
			/// <returns>Returns the user selected button.</returns>
			/// <param name="window">The current window. This argument can be null.</param>
			/// <param name="text">The content of the message box.</param>
			/// <param name="title">The title of the message box.</param>
			/// <param name="buttons">The display button combination of the message box.</param>
			/// <param name="defaultButton">The default button of the message box.</param>
			/// <param name="icon">The icon of the message box.</param>
			/// <param name="modal">The modal option of the message box.</param>
			virtual MessageBoxButtonsOutput			ShowMessageBox(INativeWindow* window, const WString& text, const WString& title=L"", MessageBoxButtonsInput buttons=DisplayOK, MessageBoxDefaultButton defaultButton=DefaultFirst, MessageBoxIcons icon=IconNone, MessageBoxModalOptions modal=ModalWindow)=0;

			/// <summary>
			/// Color dialog custom color options
			/// </summary>
			enum ColorDialogCustomColorOptions
			{
				/// <summary>Disable the custom color panel.</summary>
				CustomColorDisabled,
				/// <summary>Enable the custom color panel.</summary>
				CustomColorEnabled,
				/// <summary>Open the custom color panel at the beginning.</summary>
				CustomColorOpened,
			};

			/// <summary>Show a color dialog.</summary>
			/// <returns>Returns true if the user selected the OK button.</returns>
			/// <param name="window">The current window.</param>
			/// <param name="selection">The color that the user selected.</param>
			/// <param name="selected">Make the color dialog selected the color specified in the "selection" parameter at the beginning.</param>
			/// <param name="customColorOptions">Custom color panel options.</param>
			/// <param name="customColors">The initial 16 colors in custom color boxes. This argument can be null.</param>
			virtual bool							ShowColorDialog(INativeWindow* window, Color& selection, bool selected=false, ColorDialogCustomColorOptions customColorOptions=CustomColorEnabled, Color* customColors=0)=0;

			/// <summary>Show a font dialog.</summary>
			/// <returns>Returns true if the user selected the OK button.</returns>
			/// <param name="window">The current window.</param>
			/// <param name="selectionFont">The font that the user selected.</param>
			/// <param name="selectionColor">The color that the user selected.</param>
			/// <param name="selected">Make the font dialog selected the font specified in the "selectionFont" and "selectionColor" parameters at the beginning.</param>
			/// <param name="showEffect">Enable the user to edit some extended font properties.</param>
			/// <param name="forceFontExist">Force the user to select existing font.</param>
			virtual bool							ShowFontDialog(INativeWindow* window, FontProperties& selectionFont, Color& selectionColor, bool selected=false, bool showEffect=true, bool forceFontExist=true)=0;
			
			/// <summary>
			/// File dialog type.
			/// </summary>
			enum FileDialogTypes
			{
				/// <summary>Open file dialog.</summary>
				FileDialogOpen,
				/// <summary>Open file dialog with preview.</summary>
				FileDialogOpenPreview,
				/// <summary>Save file dialog.</summary>
				FileDialogSave,
				/// <summary>Save file dialog with preview.</summary>
				FileDialogSavePreview,
			};

			/// <summary>
			/// File dialog options.
			/// </summary>
			enum FileDialogOptions
			{
				/// <summary>Allow multiple selection.</summary>
				FileDialogAllowMultipleSelection = 1,
				/// <summary>Prevent the user to select unexisting files.</summary>
				FileDialogFileMustExist = 2,
				/// <summary>Show the "Read Only" check box.</summary>
				FileDialogShowReadOnlyCheckBox = 4,
				/// <summary>Dereference link files.</summary>
				FileDialogDereferenceLinks = 8,
				/// <summary>Show the "Network" button.</summary>
				FileDialogShowNetworkButton = 16,
				/// <summary>Prompt if a new file is going to be created.</summary>
				FileDialogPromptCreateFile = 32,
				/// <summary>Promt if a existing file is going to be overwritten.</summary>
				FileDialogPromptOverwriteFile = 64,
				/// <summary>Prevent the user to select an unexisting directory.</summary>
				FileDialogDirectoryMustExist = 128,
				/// <summary>Add user selected files to "Recent" directory.</summary>
				FileDialogAddToRecent = 256,
			};

			/// <summary>Show a file dialog.</summary>
			/// <returns>Returns true if the user selected the OK button.</returns>
			/// <param name="window">The current window.</param>
			/// <param name="selectionFileNames">The file names that the user selected.</param>
			/// <param name="selectionFilterIndex">The filter that the user selected.</param>
			/// <param name="dialogType">The type of the file dialog.</param>
			/// <param name="title">The title of the file dialog.</param>
			/// <param name="initialFileName">The initial file name.</param>
			/// <param name="initialDirectory">The initial directory.</param>
			/// <param name="defaultExtension">The default file extension.</param>
			/// <param name="filter">The file name filter like L"Text Files|*.txt|All Files|*.*".</param>
			/// <param name="options">File dialog options. Multiple options can be combined using the "|" operator.</param>
			virtual bool							ShowFileDialog(INativeWindow* window, collections::List<WString>& selectionFileNames, vint& selectionFilterIndex, FileDialogTypes dialogType, const WString& title, const WString& initialFileName, const WString& initialDirectory, const WString& defaultExtension, const WString& filter, FileDialogOptions options)=0;
		};

		inline INativeDialogService::FileDialogOptions operator|(INativeDialogService::FileDialogOptions a, INativeDialogService::FileDialogOptions b)
		{
			return static_cast<INativeDialogService::FileDialogOptions>(static_cast<vuint64_t>(a) | static_cast<vuint64_t>(b));
		}

		inline INativeDialogService::FileDialogOptions operator&(INativeDialogService::FileDialogOptions a, INativeDialogService::FileDialogOptions b)
		{
			return static_cast<INativeDialogService::FileDialogOptions>(static_cast<vuint64_t>(a) & static_cast<vuint64_t>(b));
		}

/***********************************************************************
Native Window Controller
***********************************************************************/

		/// <summary>
		/// Global native system service controller. Use [M:vl.presentation.GetCurrentController] to access this controller.
		/// </summary>
		class INativeController : public virtual IDescriptable, public Description<INativeController>
		{
		public:
			/// <summary>
			/// Get the callback service.
			/// </summary>
			/// <returns>The callback service</returns>
			virtual INativeCallbackService*			CallbackService()=0;
			/// <summary>
			/// Get the system resource service.
			/// </summary>
			/// <returns>The system resource service</returns>
			virtual INativeResourceService*			ResourceService()=0;
			/// <summary>
			/// Get the asynchronized operation service.
			/// </summary>
			/// <returns>The asynchronized operation service</returns>
			virtual INativeAsyncService*			AsyncService()=0;
			/// <summary>
			/// Get the clipboard service.
			/// </summary>
			/// <returns>The clipboard service</returns>
			virtual INativeClipboardService*		ClipboardService()=0;
			/// <summary>
			/// Get the image service.
			/// </summary>
			/// <returns>The image service</returns>
			virtual INativeImageService*			ImageService()=0;
			/// <summary>
			/// Get the screen information service.
			/// </summary>
			/// <returns>The screen information service</returns>
			virtual INativeScreenService*			ScreenService()=0;
			/// <summary>
			/// Get the window service.
			/// </summary>
			/// <returns>The window service</returns>
			virtual INativeWindowService*			WindowService()=0;
			/// <summary>
			/// Get the user input service.
			/// </summary>
			/// <returns>The user input service</returns>
			virtual INativeInputService*			InputService()=0;
			/// <summary>
			/// Get the dialog service.
			/// </summary>
			/// <returns>The user dialog service</returns>
			virtual INativeDialogService*			DialogService()=0;
			/// <summary>
			/// Get the file path of the current executable.
			/// </summary>
			/// <returns>The file path of the current executable.</returns>
			virtual WString							GetExecutablePath()=0;
		};
		
		/// <summary>
		/// Represents a global message listener to an <see cref="INativeController"/>.
		/// </summary>
		class INativeControllerListener : public Interface
		{
		public:
			/// <summary>
			/// Called when the left mouse button is pressed. To receive or not receive this message, use <see cref="INativeInputService::StartHookMouse"/> or <see cref="INativeInputService::StopHookMouse"/>.
			/// </summary>
			/// <param name="position">The mouse position in the screen space.</param>
			virtual void					LeftButtonDown(Point position);
			/// <summary>
			/// Called when the left mouse button is released. To receive or not receive this message, use <see cref="INativeInputService::StartHookMouse"/> or <see cref="INativeInputService::StopHookMouse"/>
			/// </summary>
			/// <param name="position">The mouse position in the screen space.</param>
			virtual void					LeftButtonUp(Point position);
			/// <summary>
			/// Called when the right mouse button is pressed. To receive or not receive this message, use <see cref="INativeInputService::StartHookMouse"/> or <see cref="INativeInputService::StopHookMouse"/>
			/// </summary>
			/// <param name="position">The mouse position in the screen space.</param>
			virtual void					RightButtonDown(Point position);
			/// <summary>
			/// Called when the right mouse button is released. To receive or not receive this message, use <see cref="INativeInputService::StartHookMouse"/> or <see cref="INativeInputService::StopHookMouse"/>
			/// </summary>
			/// <param name="position">The mouse position in the screen space.</param>
			virtual void					RightButtonUp(Point position);
			/// <summary>
			/// Called when the mouse is moving. To receive or not receive this message, use <see cref="INativeInputService::StartHookMouse"/> or <see cref="INativeInputService::StopHookMouse"/>
			/// </summary>
			/// <param name="position">The mouse position in the screen space.</param>
			virtual void					MouseMoving(Point position);
			/// <summary>
			/// Called when the global timer message raised. To receive or not receive this message, use <see cref="INativeInputService::StartTimer"/> or <see cref="INativeInputService::StopTimer"/>
			/// </summary>
			virtual void					GlobalTimer();
			/// <summary>
			/// Called when the content of the clipboard is updated.
			/// </summary>
			virtual void					ClipboardUpdated();
			/// <summary>
			/// Called when a window is created.
			/// </summary>
			/// <param name="window">The created window.</param>
			virtual void					NativeWindowCreated(INativeWindow* window);
			/// <summary>
			/// Called when a window is destroying.
			/// </summary>
			/// <param name="window">The destroying window.</param>
			virtual void					NativeWindowDestroying(INativeWindow* window);
		};

		/// <summary>
		/// Get the global native system service controller.
		/// </summary>
		/// <returns>The global native system service controller.</returns>
		extern								INativeController* GetCurrentController();
		/// <summary>
		/// Set the global native system service controller.
		/// </summary>
		/// <param name="controller">The global native system service controller.</param>
		extern void							SetCurrentController(INativeController* controller);
	}
}

/***********************************************************************
Native Window Provider
***********************************************************************/

/*
 * Virtual Keys, Standard Set
 */
#define VKEY_LBUTTON        0x01
#define VKEY_RBUTTON        0x02
#define VKEY_CANCEL         0x03
#define VKEY_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

#define VKEY_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#define VKEY_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */

/*
 * 0x07 : unassigned
 */

#define VKEY_BACK           0x08
#define VKEY_TAB            0x09

/*
 * 0x0A - 0x0B : reserved
 */

#define VKEY_CLEAR          0x0C
#define VKEY_RETURN         0x0D

#define VKEY_SHIFT          0x10
#define VKEY_CONTROL        0x11
#define VKEY_MENU           0x12
#define VKEY_PAUSE          0x13
#define VKEY_CAPITAL        0x14

#define VKEY_KANA           0x15
#define VKEY_HANGEUL        0x15  /* old name - should be here for compatibility */
#define VKEY_HANGUL         0x15
#define VKEY_JUNJA          0x17
#define VKEY_FINAL          0x18
#define VKEY_HANJA          0x19
#define VKEY_KANJI          0x19

#define VKEY_ESCAPE         0x1B

#define VKEY_CONVERT        0x1C
#define VKEY_NONCONVERT     0x1D
#define VKEY_ACCEPT         0x1E
#define VKEY_MODECHANGE     0x1F

#define VKEY_SPACE          0x20
#define VKEY_PRIOR          0x21
#define VKEY_NEXT           0x22
#define VKEY_END            0x23
#define VKEY_HOME           0x24
#define VKEY_LEFT           0x25
#define VKEY_UP             0x26
#define VKEY_RIGHT          0x27
#define VKEY_DOWN           0x28
#define VKEY_SELECT         0x29
#define VKEY_PRINT          0x2A
#define VKEY_EXECUTE        0x2B
#define VKEY_SNAPSHOT       0x2C
#define VKEY_INSERT         0x2D
#define VKEY_DELETE         0x2E
#define VKEY_HELP           0x2F

/*
 * VKEY_0 - VKEY_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x40 : unassigned
 * VKEY_A - VKEY_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */

#define VKEY_0              0x30
#define VKEY_1              0x31
#define VKEY_2              0x32
#define VKEY_3              0x33
#define VKEY_4              0x34
#define VKEY_5              0x35
#define VKEY_6              0x36
#define VKEY_7              0x37
#define VKEY_8              0x38
#define VKEY_9              0x39

#define VKEY_A              0x41
#define VKEY_B              0x42
#define VKEY_C              0x43
#define VKEY_D              0x44
#define VKEY_E              0x45
#define VKEY_F              0x46
#define VKEY_G              0x47
#define VKEY_H              0x48
#define VKEY_I              0x49
#define VKEY_J              0x4A
#define VKEY_K              0x4B
#define VKEY_L              0x4C
#define VKEY_M              0x4D
#define VKEY_N              0x4E
#define VKEY_O              0x4F
#define VKEY_P              0x50
#define VKEY_Q              0x51
#define VKEY_R              0x52
#define VKEY_S              0x53
#define VKEY_T              0x54
#define VKEY_U              0x55
#define VKEY_V              0x56
#define VKEY_W              0x57
#define VKEY_X              0x58
#define VKEY_Y              0x59
#define VKEY_Z              0x5A

#define VKEY_LWIN           0x5B
#define VKEY_RWIN           0x5C
#define VKEY_APPS           0x5D

/*
 * 0x5E : reserved
 */

#define VKEY_SLEEP          0x5F

#define VKEY_NUMPAD0        0x60
#define VKEY_NUMPAD1        0x61
#define VKEY_NUMPAD2        0x62
#define VKEY_NUMPAD3        0x63
#define VKEY_NUMPAD4        0x64
#define VKEY_NUMPAD5        0x65
#define VKEY_NUMPAD6        0x66
#define VKEY_NUMPAD7        0x67
#define VKEY_NUMPAD8        0x68
#define VKEY_NUMPAD9        0x69
#define VKEY_MULTIPLY       0x6A
#define VKEY_ADD            0x6B
#define VKEY_SEPARATOR      0x6C
#define VKEY_SUBTRACT       0x6D
#define VKEY_DECIMAL        0x6E
#define VKEY_DIVIDE         0x6F
#define VKEY_F1             0x70
#define VKEY_F2             0x71
#define VKEY_F3             0x72
#define VKEY_F4             0x73
#define VKEY_F5             0x74
#define VKEY_F6             0x75
#define VKEY_F7             0x76
#define VKEY_F8             0x77
#define VKEY_F9             0x78
#define VKEY_F10            0x79
#define VKEY_F11            0x7A
#define VKEY_F12            0x7B
#define VKEY_F13            0x7C
#define VKEY_F14            0x7D
#define VKEY_F15            0x7E
#define VKEY_F16            0x7F
#define VKEY_F17            0x80
#define VKEY_F18            0x81
#define VKEY_F19            0x82
#define VKEY_F20            0x83
#define VKEY_F21            0x84
#define VKEY_F22            0x85
#define VKEY_F23            0x86
#define VKEY_F24            0x87

/*
 * 0x88 - 0x8F : unassigned
 */

#define VKEY_NUMLOCK        0x90
#define VKEY_SCROLL         0x91

/*
 * NEC PC-9800 kbd definitions
 */
#define VKEY_OEM_NEC_EQUAL  0x92   // '=' key on numpad

/*
 * Fujitsu/OASYS kbd definitions
 */
#define VKEY_OEM_FJ_JISHO   0x92   // 'Dictionary' key
#define VKEY_OEM_FJ_MASSHOU 0x93   // 'Unregister word' key
#define VKEY_OEM_FJ_TOUROKU 0x94   // 'Register word' key
#define VKEY_OEM_FJ_LOYA    0x95   // 'Left OYAYUBI' key
#define VKEY_OEM_FJ_ROYA    0x96   // 'Right OYAYUBI' key

/*
 * 0x97 - 0x9F : unassigned
 */

/*
 * VKEY_L* & VKEY_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
#define VKEY_LSHIFT         0xA0
#define VKEY_RSHIFT         0xA1
#define VKEY_LCONTROL       0xA2
#define VKEY_RCONTROL       0xA3
#define VKEY_LMENU          0xA4
#define VKEY_RMENU          0xA5

#define VKEY_BROWSER_BACK        0xA6
#define VKEY_BROWSER_FORWARD     0xA7
#define VKEY_BROWSER_REFRESH     0xA8
#define VKEY_BROWSER_STOP        0xA9
#define VKEY_BROWSER_SEARCH      0xAA
#define VKEY_BROWSER_FAVORITES   0xAB
#define VKEY_BROWSER_HOME        0xAC

#define VKEY_VOLUME_MUTE         0xAD
#define VKEY_VOLUME_DOWN         0xAE
#define VKEY_VOLUME_UP           0xAF
#define VKEY_MEDIA_NEXT_TRACK    0xB0
#define VKEY_MEDIA_PREV_TRACK    0xB1
#define VKEY_MEDIA_STOP          0xB2
#define VKEY_MEDIA_PLAY_PAUSE    0xB3
#define VKEY_LAUNCH_MAIL         0xB4
#define VKEY_LAUNCH_MEDIA_SELECT 0xB5
#define VKEY_LAUNCH_APP1         0xB6
#define VKEY_LAUNCH_APP2         0xB7

/*
 * 0xB8 - 0xB9 : reserved
 */

#define VKEY_OEM_1          0xBA   // ';:' for US
#define VKEY_OEM_PLUS       0xBB   // '+' any country
#define VKEY_OEM_COMMA      0xBC   // ',' any country
#define VKEY_OEM_MINUS      0xBD   // '-' any country
#define VKEY_OEM_PERIOD     0xBE   // '.' any country
#define VKEY_OEM_2          0xBF   // '/?' for US
#define VKEY_OEM_3          0xC0   // '`~' for US

/*
 * 0xC1 - 0xD7 : reserved
 */

/*
 * 0xD8 - 0xDA : unassigned
 */

#define VKEY_OEM_4          0xDB  //  '[{' for US
#define VKEY_OEM_5          0xDC  //  '\|' for US
#define VKEY_OEM_6          0xDD  //  ']}' for US
#define VKEY_OEM_7          0xDE  //  ''"' for US
#define VKEY_OEM_8          0xDF

/*
 * 0xE0 : reserved
 */

/*
 * Various extended or enhanced keyboards
 */

#define VKEY_OEM_AX         0xE1  //  'AX' key on Japanese AX kbd
#define VKEY_OEM_102        0xE2  //  "<>" or "\|" on RT 102-key kbd.
#define VKEY_ICO_HELP       0xE3  //  Help key on ICO
#define VKEY_ICO_00         0xE4  //  00 key on ICO

#define VKEY_PROCESSKEY     0xE5
#define VKEY_ICO_CLEAR      0xE6
#define VKEY_PACKET         0xE7

/*
 * 0xE8 : unassigned
 */

/*
 * Nokia/Ericsson definitions
 */
#define VKEY_OEM_RESET      0xE9
#define VKEY_OEM_JUMP       0xEA
#define VKEY_OEM_PA1        0xEB
#define VKEY_OEM_PA2        0xEC
#define VKEY_OEM_PA3        0xED
#define VKEY_OEM_WSCTRL     0xEE
#define VKEY_OEM_CUSEL      0xEF
#define VKEY_OEM_ATTN       0xF0
#define VKEY_OEM_FINISH     0xF1
#define VKEY_OEM_COPY       0xF2
#define VKEY_OEM_AUTO       0xF3
#define VKEY_OEM_ENLW       0xF4
#define VKEY_OEM_BACKTAB    0xF5

#define VKEY_ATTN           0xF6
#define VKEY_CRSEL          0xF7
#define VKEY_EXSEL          0xF8
#define VKEY_EREOF          0xF9
#define VKEY_PLAY           0xFA
#define VKEY_ZOOM           0xFB
#define VKEY_NONAME         0xFC
#define VKEY_PA1            0xFD
#define VKEY_OEM_CLEAR      0xFE

/*
 * Friendly names for common keys (US)
 */
#define VKEY_SEMICOLON		VKEY_OEM_1
#define VKEY_SLASH			VKEY_OEM_2
#define VKEY_GRAVE_ACCENT	VKEY_OEM_3
#define VKEY_RIGHT_BRACKET	VKEY_OEM_4
#define VKEY_BACKSLASH		VKEY_OEM_5
#define VKEY_LEFT_BRACKET	VKEY_OEM_6
#define VKEY_APOSTROPHE		VKEY_OEM_7


#endif