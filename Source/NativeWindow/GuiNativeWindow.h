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
		class GuiImageData;
		class DocumentModel;
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
		class INativeScreen : public virtual IDescriptable, public Description<INativeScreen>
		{
		public:
			/// <summary>
			/// Get the bounds of the screen.
			/// </summary>
			/// <returns>The bounds of the screen.</returns>
			virtual NativeRect			GetBounds()=0;
			/// <summary>
			/// Get the bounds of the screen client area.
			/// </summary>
			/// <returns>The bounds of the screen client area.</returns>
			virtual NativeRect			GetClientBounds()=0;
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
			/// <summary>
			/// Get the scaling for the screen's horizontal edge. For example, in Windows when you have a 96 DPI, this function returns 1.0.
			/// </summary>
			virtual double				GetScalingX() = 0;
			/// <summary>
			/// Get the scaling for the screen's vertical edge. For example, in Windows when you have a 96 DPI, this function returns 1.0.
			/// </summary>
			virtual double				GetScalingY() = 0;
		};
		
		/// <summary>
		/// Represents a cursor.
		/// </summary>
		class INativeCursor : public virtual IDescriptable, public Description<INativeCursor>
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
			/// <summary>
			/// Save the image to a stream.
			/// </summary>
			/// <param name="stream">The stream.</param>
			/// <param name="formatType">The format of the image.</param>
			virtual void						SaveToStream(stream::IStream& stream, FormatType formatType = FormatType::Unknown) = 0;
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
			/// Convert point from native coordinate to GUI coordinate.
			/// </summary>
			/// <returns>The converted result.</returns>
			/// <param name="value">The coordinate to convert.</param>
			virtual Point				Convert(NativePoint value) = 0;
			/// <summary>
			/// Convert point from GUI coordinate to native coordinate.
			/// </summary>
			/// <returns>The converted result.</returns>
			/// <param name="value">The coordinate to convert.</param>
			virtual NativePoint			Convert(Point value) = 0;
			/// <summary>
			/// Convert size from native coordinate to GUI coordinate.
			/// </summary>
			/// <returns>The converted result.</returns>
			/// <param name="value">The coordinate to convert.</param>
			virtual Size				Convert(NativeSize value) = 0;
			/// <summary>
			/// Convert size from GUI coordinate to native coordinate.
			/// </summary>
			/// <returns>The converted result.</returns>
			/// <param name="value">The coordinate to convert.</param>
			virtual NativeSize			Convert(Size value) = 0;
			/// <summary>
			/// Convert margin from native coordinate to GUI coordinate.
			/// </summary>
			/// <returns>The converted result.</returns>
			/// <param name="value">The coordinate to convert.</param>
			virtual Margin				Convert(NativeMargin value) = 0;
			/// <summary>
			/// Convert margin from GUI coordinate to native coordinate.
			/// </summary>
			/// <returns>The converted result.</returns>
			/// <param name="value">The coordinate to convert.</param>
			virtual NativeMargin		Convert(Margin value) = 0;

			/// <summary>
			/// Get the bounds of the window.
			/// </summary>
			/// <returns>The bounds of the window.</returns>
			virtual NativeRect			GetBounds()=0;
			/// <summary>
			/// Set the bounds of the window.
			/// </summary>
			/// <param name="bounds">The bounds of the window.</param>
			virtual void				SetBounds(const NativeRect& bounds)=0;
			/// <summary>
			/// Get the client size of the window.
			/// </summary>
			/// <returns>The client size of the window.</returns>
			virtual NativeSize			GetClientSize()=0;
			/// <summary>
			/// Set the client size of the window.
			/// </summary>
			/// <param name="size">The client size of the window.</param>
			virtual void				SetClientSize(NativeSize size)=0;
			/// <summary>
			/// Get the client bounds in screen space.
			/// </summary>
			/// <returns>The client bounds in screen space.</returns>
			virtual NativeRect			GetClientBoundsInScreen()=0;
			
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
			virtual NativePoint			GetCaretPoint()=0;
			/// <summary>
			/// Set the caret point of the window. When an input method editor is opened, the input text box will be located to the caret point.
			/// </summary>
			/// <param name="point">The caret point of the window.</param>
			virtual void				SetCaretPoint(NativePoint point)=0;
			
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
			/// <summary>
			/// Get the amount of the border. The window template may need this value to calculate where to put the client area.
			/// </summary>
			/// <returns>Returns the amount of the border.</returns>
			virtual NativeMargin		GetCustomFramePadding() = 0;

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
			/// Get the icon.
			/// </summary>
			/// <returns>Returns the icon.</returns>
			virtual Ptr<GuiImageData>	GetIcon()=0;
			/// <summary>
			/// Set the icon.
			/// </summary>
			/// <param name="icon">The icon. Set to null to use the default icon.</param>
			virtual void				SetIcon(Ptr<GuiImageData> icon)=0;

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
			/// <param name="closeWindow">Set to true to really close the window. Or the window will just be hidden. This parameter only affect the main window.</param>
			virtual void				Hide(bool closeWindow)=0;
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
		/// <typeparam name="T">Type of the coordinate.</typeparam>
		template<typename T>
		struct WindowMouseInfo_
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
			T							x;
			/// <summary>The mouse position of y dimension.</summary>
			T							y;
			/// <summary>The delta of the wheel.</summary>
			vint						wheel;
			/// <summary>True if the mouse is in the non-client area.</summary>
			bool						nonClient;
		};

		using WindowMouseInfo = WindowMouseInfo_<GuiCoordinate>;
		using NativeWindowMouseInfo = WindowMouseInfo_<NativeCoordinate>;
		
		/// <summary>
		/// Key message information.
		/// </summary>
		struct NativeWindowKeyInfo
		{
			/// <summary>Key code of the key that sends this message.</summary>
			VKEY						code;
			/// <summary>True if the control button is pressed.</summary>
			bool						ctrl;
			/// <summary>True if the shift button is pressed.</summary>
			bool						shift;
			/// <summary>True if the alt button is pressed.</summary>
			bool						alt;
			/// <summary>True if the capslock button is pressed.</summary>
			bool						capslock;
			/// <summary>True if this repeated event is generated because a key is holding down.</summary>
			bool						autoRepeatKeyDown;
		};

		using WindowKeyInfo = NativeWindowKeyInfo;
		
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

		using WindowCharInfo = NativeWindowCharInfo;
		
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
			virtual HitTestResult		HitTest(NativePoint location);
			/// <summary>
			/// Called when the window is moving.
			/// </summary>
			/// <param name="bounds">The bounds. Message handler can change the bounds.</param>
			/// <param name="fixSizeOnly">True if the message raise only want the message handler to change the size.</param>
			virtual void				Moving(NativeRect& bounds, bool fixSizeOnly);
			/// <summary>
			/// Called when the window is moved.
			/// </summary>
			virtual void				Moved();
			/// <summary>
			/// Called when the dpi associated with this window is changed.
			/// </summary>
			virtual void				DpiChanged();
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
		/// Clipboard reader.
		/// </summary>
		class INativeClipboardReader : public virtual IDescriptable, public Description<INativeClipboardReader>
		{
		public:
			/// <summary>Test is there a text in the clipboard.</summary>
			/// <returns>Returns true if there is a text in the clipboard.</returns>
			virtual bool					ContainsText() = 0;

			/// <summary>Get the text from the clipboard.</summary>
			/// <returns>The text.</returns>
			virtual WString					GetText() = 0;

			/// <summary>Test is there a document in the clipboard.</summary>
			/// <returns>Returns true if there is a document in the clipboard.</returns>
			virtual bool					ContainsDocument() = 0;

			/// <summary>Get the document from the clipboard.</summary>
			/// <returns>The document.</returns>
			virtual Ptr<DocumentModel>		GetDocument() = 0;

			/// <summary>Test is there an image in the clipboard.</summary>
			/// <returns>Returns true if there is an image in the clipboard.</returns>
			virtual bool					ContainsImage() = 0;

			/// <summary>Get the image from the clipboard.</summary>
			/// <returns>The image.</returns>
			virtual Ptr<INativeImage>		GetImage() = 0;
		};

		/// <summary>
		/// Clipboard writer.
		/// </summary>
		class INativeClipboardWriter : public virtual IDescriptable, public Description<INativeClipboardWriter>
		{
		public:
			/// <summary>Prepare a text for the clipboard.</summary>
			/// <param name="value">The text.</param>
			virtual void					SetText(const WString& value) = 0;

			/// <summary>Prepare a document for the clipboard.</summary>
			/// <param name="value">The document.</param>
			virtual void					SetDocument(Ptr<DocumentModel> value) = 0;

			/// <summary>Prepare an image for the clipboard.</summary>
			/// <param name="value">The image.</param>
			virtual void					SetImage(Ptr<INativeImage> value) = 0;

			/// <summary>Send all data to the clipboard.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			virtual bool					Submit() = 0;
		};
		
		/// <summary>
		/// Clipboard service. To access this service, use [M:vl.presentation.INativeController.ClipboardService].
		/// </summary>
		class INativeClipboardService : public virtual IDescriptable, public Description<INativeClipboardService>
		{
		public:
			/// <summary>Read clipboard.</summary>
			/// <returns>The clipboard reader.</returns>
			virtual Ptr<INativeClipboardReader>		ReadClipboard() = 0;
			/// <summary>Write clipboard.</summary>
			/// <returns>The clipboard writer.</returns>
			virtual Ptr<INativeClipboardWriter>		WriteClipboard() = 0;
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
			virtual INativeWindow*			CreateNativeWindow() = 0;
			/// <summary>
			/// Destroy a window.
			/// </summary>
			/// <param name="window">The window to destroy.</param>
			virtual void					DestroyNativeWindow(INativeWindow* window) = 0;
			/// <summary>
			/// Get the main window.
			/// </summary>
			/// <returns>The main window.</returns>
			virtual INativeWindow*			GetMainWindow() = 0;
			/// <summary>
			/// Get the window that under a specified position in screen space.
			/// </summary>
			/// <returns>The window that under a specified position in screen space.</returns>
			/// <param name="location">The specified position in screen space.</param>
			virtual INativeWindow*			GetWindow(NativePoint location) = 0;
			/// <summary>
			/// Make the specified window a main window, show that window, and wait until the windows is closed.
			/// </summary>
			/// <param name="window">The specified window.</param>
			virtual void					Run(INativeWindow* window) = 0;
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
			/// <param name="code">The key code to test.</param>
			virtual bool					IsKeyPressing(VKEY code)=0;
			/// <summary>
			/// Test is the specified key toggled.
			/// </summary>
			/// <returns>Returns true if the specified key is toggled.</returns>
			/// <param name="code">The key code to test.</param>
			virtual bool					IsKeyToggled(VKEY code)=0;

			/// <summary>
			/// Get the name of a key.
			/// </summary>
			/// <returns>The name of a key.</returns>
			/// <param name="code">The key code.</param>
			virtual WString					GetKeyName(VKEY code)=0;
			/// <summary>
			/// Get the key from a name.
			/// </summary>
			/// <returns>The key, returns -1 if the key name doesn't exist.</returns>
			/// <param name="name">Key name</param>
			virtual VKEY					GetKey(const WString& name)=0;
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
			virtual void					LeftButtonDown(NativePoint position);
			/// <summary>
			/// Called when the left mouse button is released. To receive or not receive this message, use <see cref="INativeInputService::StartHookMouse"/> or <see cref="INativeInputService::StopHookMouse"/>
			/// </summary>
			/// <param name="position">The mouse position in the screen space.</param>
			virtual void					LeftButtonUp(NativePoint position);
			/// <summary>
			/// Called when the right mouse button is pressed. To receive or not receive this message, use <see cref="INativeInputService::StartHookMouse"/> or <see cref="INativeInputService::StopHookMouse"/>
			/// </summary>
			/// <param name="position">The mouse position in the screen space.</param>
			virtual void					RightButtonDown(NativePoint position);
			/// <summary>
			/// Called when the right mouse button is released. To receive or not receive this message, use <see cref="INativeInputService::StartHookMouse"/> or <see cref="INativeInputService::StopHookMouse"/>
			/// </summary>
			/// <param name="position">The mouse position in the screen space.</param>
			virtual void					RightButtonUp(NativePoint position);
			/// <summary>
			/// Called when the mouse is moving. To receive or not receive this message, use <see cref="INativeInputService::StartHookMouse"/> or <see cref="INativeInputService::StopHookMouse"/>
			/// </summary>
			/// <param name="position">The mouse position in the screen space.</param>
			virtual void					MouseMoving(NativePoint position);
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

#endif