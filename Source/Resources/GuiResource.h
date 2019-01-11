/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Resource

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_RESOURCES_GUIRESOURCE
#define VCZH_PRESENTATION_RESOURCES_GUIRESOURCE

#include "../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace workflow
	{
		class IWfCompilerCallback;
	}

	namespace presentation
	{
		class GuiResourceItem;
		class GuiResourceFolder;
		class GuiResource;

/***********************************************************************
Helper Functions
***********************************************************************/

		/// <summary>Get the folder path from a file path. The result folder path is ended with a separator.</summary>
		/// <returns>The folder path.</returns>
		/// <param name="filePath">The file path.</param>
		extern WString								GetFolderPath(const WString& filePath);
		/// <summary>Get the file name from a file path.</summary>
		/// <returns>The file name.</returns>
		/// <param name="filePath">The file path.</param>
		extern WString								GetFileName(const WString& filePath);
		/// <summary>Load a text file.</summary>
		/// <returns>Returns true if the operation succeeded.</returns>
		/// <param name="filePath">The text file path.</param>
		/// <param name="text">The text file content, if succeeded.</param>
		extern bool									LoadTextFile(const WString& filePath, WString& text);
		/// <summary>Test is a text a resource url and extract the protocol and the path.</summary>
		/// <returns>Returns true if the text is a resource url.</returns>
		/// <param name="text">The text.</param>
		/// <param name="protocol">The extracted protocol.</param>
		/// <param name="path">The extracted path.</param>
		extern bool									IsResourceUrl(const WString& text, WString& protocol, WString& path);

		extern void									HexToBinary(stream::IStream& stream, const WString& hexText);
		extern WString								BinaryToHex(stream::IStream& stream);

/***********************************************************************
Global String Key
***********************************************************************/

		struct GlobalStringKey
		{
		public:
			static GlobalStringKey					Empty;
			static GlobalStringKey					_InferType;
			static GlobalStringKey					_Set;
			static GlobalStringKey					_Ref;
			static GlobalStringKey					_Bind;
			static GlobalStringKey					_Format;
			static GlobalStringKey					_Str;
			static GlobalStringKey					_Eval;
			static GlobalStringKey					_Uri;
			static GlobalStringKey					_ControlTemplate;
			static GlobalStringKey					_ItemTemplate;

		private:
			vint									key = -1;

		public:
			static vint Compare(GlobalStringKey a, GlobalStringKey b){ return a.key - b.key; }
			bool operator==(GlobalStringKey g)const{ return key == g.key; }
			bool operator!=(GlobalStringKey g)const{ return key != g.key; }
			bool operator<(GlobalStringKey g)const{ return key < g.key; }
			bool operator<=(GlobalStringKey g)const{ return key <= g.key; }
			bool operator>(GlobalStringKey g)const{ return key > g.key; }
			bool operator>=(GlobalStringKey g)const{ return key >= g.key; }

			static GlobalStringKey					Get(const WString& string);
			vint									ToKey()const;
			WString									ToString()const;
		};

/***********************************************************************
Resource Image
***********************************************************************/
			
		/// <summary>
		/// Represnets an image to display.
		/// </summary>
		class GuiImageData : public Object, public Description<GuiImageData>
		{
		protected:
			Ptr<INativeImage>				image;
			vint							frameIndex;

		public:
			/// <summary>Create an empty image data.</summary>
			GuiImageData();
			/// <summary>Create an image data with a specified image and a frame index.</summary>
			/// <param name="_image">The specified image.</param>
			/// <param name="_frameIndex">The specified frame index.</param>
			GuiImageData(Ptr<INativeImage> _image, vint _frameIndex);
			~GuiImageData();

			/// <summary>Get the specified image.</summary>
			/// <returns>The specified image.</returns>
			Ptr<INativeImage>				GetImage();
			/// <summary>Get the specified frame index.</summary>
			/// <returns>The specified frame index.</returns>
			vint							GetFrameIndex();
		};

/***********************************************************************
Resource String
***********************************************************************/

		/// <summary>Represents a text resource.</summary>
		class GuiTextData : public Object, public Description<GuiTextData>
		{
		protected:
			WString							text;

		public:
			/// <summary>Create an empty text data.</summary>
			GuiTextData();
			/// <summary>Create a text data with a specified text.</summary>
			/// <param name="_text">The specified text.</param>
			GuiTextData(const WString& _text);
			
			/// <summary>Get the specified text.</summary>
			/// <returns>The specified text.</returns>
			WString							GetText();
		};

/***********************************************************************
Resource Structure
***********************************************************************/

		/// <summary>Resource node base.</summary>
		class GuiResourceNodeBase : public Object, public Description<GuiResourceNodeBase>
		{
			friend class GuiResourceFolder;
		protected:
			GuiResourceFolder*						parent;
			WString									name;
			WString									fileContentPath;
			WString									fileAbsolutePath;
			
		public:
			GuiResourceNodeBase();
			~GuiResourceNodeBase();

			/// <summary>Get the containing folder. Returns null means that this is the root resource node.</summary>
			/// <returns>The containing folder.</returns>
			GuiResourceFolder*						GetParent();
			/// <summary>Get the name of this resource node.</summary>
			/// <returns>The name of this resource node .</returns>
			const WString&							GetName();
			/// <summary>Get the resource path of this resource node</summary>
			/// <returns>The resource path of this resource node .</returns>
			WString									GetResourcePath();
			/// <summary>Get the file content path of this resource node. When saving the resource, if the path is not empty, the path will be serialized instead of the content.</summary>
			/// <returns>The file content path of this resource node .</returns>
			const WString&							GetFileContentPath();
			/// <summary>Get the absolute file content path of this resource node. This path points to an existing file containing the content.</summary>
			/// <returns>The file absolute path of this resource node .</returns>
			const WString&							GetFileAbsolutePath();
			/// <summary>Set the file content path of this resource node.</summary>
			/// <param name="content">The file content path of this resource node .</param>
			/// <param name="absolute">The file absolute path of this resource node .</param>
			void									SetFileContentPath(const WString& content, const WString& absolute);
		};

		struct GuiResourceLocation
		{
			WString									resourcePath;
			WString									filePath;

			GuiResourceLocation() = default;
			GuiResourceLocation(const WString& _resourcePath, const WString& _filePath);
			GuiResourceLocation(Ptr<GuiResourceNodeBase> node);

			bool operator==(const GuiResourceLocation& b)const { return resourcePath == b.resourcePath && filePath == b.filePath; }
			bool operator!=(const GuiResourceLocation& b)const { return !(*this == b); }
		};

		struct GuiResourceTextPos
		{
			GuiResourceLocation						originalLocation;
			vint									row = parsing::ParsingTextPos::UnknownValue;
			vint									column = parsing::ParsingTextPos::UnknownValue;

			GuiResourceTextPos() = default;
			GuiResourceTextPos(GuiResourceLocation location, parsing::ParsingTextPos position);

			bool operator==(const GuiResourceTextPos& b)const { return originalLocation == b.originalLocation && row == b.row && column == b.column; }
			bool operator!=(const GuiResourceTextPos& b)const { return !(*this == b); }
		};

		struct GuiResourceError
		{
		public:
			using List = collections::List<GuiResourceError>;

			GuiResourceLocation						location;
			GuiResourceTextPos						position;
			WString									message;

			GuiResourceError() = default;
			GuiResourceError(GuiResourceTextPos _position, const WString& _message);
			GuiResourceError(GuiResourceLocation _location, const WString& _message);
			GuiResourceError(GuiResourceLocation _location, GuiResourceTextPos _position, const WString& _message);

			bool operator==(const GuiResourceError& b)const { return location == b.location && position == b.position && message == b.message; }
			bool operator!=(const GuiResourceError& b)const { return !(*this == b); }

			static void								Transform(GuiResourceLocation _location, GuiResourceError::List& errors, collections::List<Ptr<parsing::ParsingError>>& parsingErrors);
			static void								Transform(GuiResourceLocation _location, GuiResourceError::List& errors, collections::List<Ptr<parsing::ParsingError>>& parsingErrors, parsing::ParsingTextPos offset);
			static void								Transform(GuiResourceLocation _location, GuiResourceError::List& errors, collections::List<Ptr<parsing::ParsingError>>& parsingErrors, GuiResourceTextPos offset);
			static void								SortAndLog(List& errors, collections::List<WString>& output, const WString& workingDirectory = WString::Empty);
		};

		class DocumentModel;
		class GuiResourcePathResolver;
		struct GuiResourcePrecompileContext;
		struct GuiResourceInitializeContext;
		class IGuiResourcePrecompileCallback;
		
		/// <summary>Resource item.</summary>
		class GuiResourceItem : public GuiResourceNodeBase, public Description<GuiResourceItem>
		{
			friend class GuiResourceFolder;
		protected:
			Ptr<DescriptableObject>					content;
			WString									typeName;
			
		public:
			/// <summary>Create a resource item.</summary>
			GuiResourceItem();
			~GuiResourceItem();

			/// <summary>Get the type of this resource item.</summary>
			/// <returns>The type name.</returns>
			const WString&							GetTypeName();
			
			/// <summary>Get the contained object for this resource item.</summary>
			/// <returns>The contained object.</returns>
			Ptr<DescriptableObject>					GetContent();
			/// <summary>Set the containd object for this resource item.</summary>
			/// <param name="_typeName">The type name of this contained object.</param>
			/// <param name="value">The contained object.</param>
			void									SetContent(const WString& _typeName, Ptr<DescriptableObject> value);

			/// <summary>Get the contained object as an image.</summary>
			/// <returns>The contained object.</returns>
			Ptr<GuiImageData>						AsImage();
			/// <summary>Get the contained object as an xml.</summary>
			/// <returns>The contained object.</returns>
			Ptr<parsing::xml::XmlDocument>			AsXml();
			/// <summary>Get the contained object as a string.</summary>
			/// <returns>The contained object.</returns>
			Ptr<GuiTextData>						AsString();
			/// <summary>Get the contained object as a document model.</summary>
			/// <returns>The contained object.</returns>
			Ptr<DocumentModel>						AsDocument();
		};
		
		/// <summary>Resource folder. A resource folder contains many sub folders and sub items.</summary>
		class GuiResourceFolder : public GuiResourceNodeBase, public Description<GuiResourceFolder>
		{
		protected:
			typedef collections::Dictionary<WString, Ptr<GuiResourceItem>>		ItemMap;
			typedef collections::Dictionary<WString, Ptr<GuiResourceFolder>>	FolderMap;
			typedef collections::List<Ptr<GuiResourceItem>>						ItemList;
			typedef collections::List<Ptr<GuiResourceFolder>>					FolderList;

			struct DelayLoading
			{
				WString								type;
				WString								workingDirectory;
				Ptr<GuiResourceItem>				preloadResource;
			};

			typedef collections::List<DelayLoading>								DelayLoadingList;

			WString									importUri;
			ItemMap									items;
			FolderMap								folders;

			void									LoadResourceFolderFromXml(DelayLoadingList& delayLoadings, const WString& containingFolder, Ptr<parsing::xml::XmlElement> folderXml, GuiResourceError::List& errors);
			void									SaveResourceFolderToXml(Ptr<parsing::xml::XmlElement> xmlParent);
			void									CollectTypeNames(collections::List<WString>& typeNames);
			void									LoadResourceFolderFromBinary(DelayLoadingList& delayLoadings, stream::internal::ContextFreeReader& reader, collections::List<WString>& typeNames, GuiResourceError::List& errors);
			void									SaveResourceFolderToBinary(stream::internal::ContextFreeWriter& writer, collections::List<WString>& typeNames);
			void									PrecompileResourceFolder(GuiResourcePrecompileContext& context, IGuiResourcePrecompileCallback* callback, GuiResourceError::List& errors);
			void									InitializeResourceFolder(GuiResourceInitializeContext& context, GuiResourceError::List& errors);
			void									ImportFromUri(const WString& uri, GuiResourceTextPos position, GuiResourceError::List& errors);
		public:
			/// <summary>Create a resource folder.</summary>
			GuiResourceFolder();
			~GuiResourceFolder();

			///<summary>Get the import uri for this folder.</summary>
			///<returns>The import uri for this folder. Returns an empty string for non-import folders</returns>
			const WString&							GetImportUri();
			///<summary>Set the import uri for this folder.</summary>
			///<param name="uri">The import uri for this folder. Set an empty string for non-import folders</param>
			void									SetImportUri(const WString& uri);

			/// <summary>Get all sub items.</summary>
			/// <returns>All sub items.</returns>
			const ItemList&							GetItems();
			/// <summary>Get the item of a specified name.</summary>
			/// <returns>The item of a specified name.</returns>
			/// <param name="name">The specified name.</param>
			Ptr<GuiResourceItem>					GetItem(const WString& name);
			/// <summary>Add a resource item.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="name">The name of this resource item.</param>
			/// <param name="item">The resource item.</param>
			bool									AddItem(const WString& name, Ptr<GuiResourceItem> item);
			/// <summary>Remove a resource item of a specified name.</summary>
			/// <returns>Returns the removed resource item if this operation succeeded.</returns>
			/// <param name="name">The name of this resource item.</param>
			Ptr<GuiResourceItem>					RemoveItem(const WString& name);
			/// <summary>Remove all resource item.</summary>
			void									ClearItems();
			
			/// <summary>Get all sub folders.</summary>
			/// <returns>All sub folders.</returns>
			const FolderList&						GetFolders();
			/// <summary>Get the folder of a specified name.</summary>
			/// <returns>The folder of a specified name.</returns>
			/// <param name="name">The specified name.</param>
			Ptr<GuiResourceFolder>					GetFolder(const WString& name);
			/// <summary>Add a resource folder.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="name">The name of this resource folder.</param>
			/// <param name="folder">The resource folder.</param>
			bool									AddFolder(const WString& name, Ptr<GuiResourceFolder> folder);
			/// <summary>Remove a resource folder of a specified name.</summary>
			/// <returns>Returns the removed resource folder if this operation succeeded.</returns>
			/// <param name="name">The name of this resource folder.</param>
			Ptr<GuiResourceFolder>					RemoveFolder(const WString& name);
			/// <summary>Remove all resource folders.</summary>
			void									ClearFolders();

			/// <summary>Get a contained resource object using a path like "Packages\Application\Name".</summary>
			/// <returns>The containd resource object.</returns>
			/// <param name="path">The path.</param>
			Ptr<DescriptableObject>					GetValueByPath(const WString& path);
			/// <summary>Get a resource folder using a path like "Packages\Application\Name\".</summary>
			/// <returns>The resource folder.</returns>
			/// <param name="path">The path.</param>
			Ptr<GuiResourceFolder>					GetFolderByPath(const WString& path);
			/// <summary>Create a contained resource object using a path like "Packages\Application\Name".</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="path">The path.</param>
			/// <param name="typeName">The type name of this contained object.</param>
			/// <param name="value">The contained object.</param>
			bool									CreateValueByPath(const WString& path, const WString& typeName, Ptr<DescriptableObject> value);
		};

/***********************************************************************
Resource
***********************************************************************/

		enum class GuiResourceUsage
		{
			DataOnly,
			InstanceClass,
		};

		/// <summary>Resource metadata.</summary>
		class GuiResourceMetadata : public Object
		{
		public:
			WString									name;
			WString									version;
			collections::List<WString>				dependencies;

			void									LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, GuiResourceLocation location, GuiResourceError::List& errors);
			Ptr<parsing::xml::XmlDocument>			SaveToXml();
		};
		
		/// <summary>Resource. A resource is a root resource folder that does not have a name.</summary>
		class GuiResource : public GuiResourceFolder, public Description<GuiResource>
		{
		protected:
			WString									workingDirectory;
			Ptr<GuiResourceMetadata>				metadata;

			static void								ProcessDelayLoading(Ptr<GuiResource> resource, DelayLoadingList& delayLoadings, GuiResourceError::List& errors);
		public:
			static const wchar_t*					CurrentVersionString;

			/// <summary>Create a resource.</summary>
			GuiResource();
			~GuiResource();

			/// <summary>Get the metadata of the resource.</summary>
			/// <returns>The metadata.</returns>
			Ptr<GuiResourceMetadata>				GetMetadata();

			/// <summary>Get the directory where the resource is load.</summary>
			/// <returns>The directory.</returns>
			WString									GetWorkingDirectory();

			/// <summary>Load a resource from an xml file. If the xml file refers other files, they will be loaded as well.</summary>
			/// <returns>The loaded resource.</returns>
			/// <param name="xml">The xml document.</param>
			/// <param name="filePath">The file path of the resource.</param>
			/// <param name="workingDirectory">The working directory for loading external resources.</param>
			/// <param name="errors">All collected errors during loading a resource.</param>
			static Ptr<GuiResource>					LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, const WString& filePath, const WString& workingDirectory, GuiResourceError::List& errors);

			/// <summary>Load a resource from an xml file. If the xml file refers other files, they will be loaded as well.</summary>
			/// <returns>The loaded resource.</returns>
			/// <param name="filePath">The file path of the xml file.</param>
			/// <param name="errors">All collected errors during loading a resource.</param>
			static Ptr<GuiResource>					LoadFromXml(const WString& filePath, GuiResourceError::List& errors);

			/// <summary>Save the resource to xml.</summary>
			/// <returns>The xml.</returns>
			Ptr<parsing::xml::XmlDocument>			SaveToXml();
			
			/// <summary>Load a precompiled resource from a stream.</summary>
			/// <returns>The loaded resource.</returns>
			/// <param name="stream">The stream.</param>
			/// <param name="errors">All collected errors during loading a resource.</param>
			static Ptr<GuiResource>					LoadPrecompiledBinary(stream::IStream& stream, GuiResourceError::List& errors);

			/// <summary>Load a precompiled resource from a stream. This function will hit an assert if there are errors.</summary>
			/// <returns>The loaded resource.</returns>
			/// <param name="stream">The stream.</param>
			static Ptr<GuiResource>					LoadPrecompiledBinary(stream::IStream& stream);
			
			/// <summary>Save the precompiled resource to a stream.</summary>
			/// <param name="stream">The stream.</param>
			void									SavePrecompiledBinary(stream::IStream& stream);

			/// <summary>Precompile this resource to improve performance.</summary>
			/// <returns>The resource folder contains all precompiled result. The folder will be added to the resource if there is no error.</returns>
			/// <param name="callback">A callback to receive progress.</param>
			/// <param name="errors">All collected errors during precompiling a resource.</param>
			Ptr<GuiResourceFolder>					Precompile(IGuiResourcePrecompileCallback* callback, GuiResourceError::List& errors);

			/// <summary>Initialize a precompiled resource.</summary>
			/// <param name="usage">In which role an application is initializing this resource.</param>
			/// <param name="errors">All collected errors during initializing a resource.</param>
			void									Initialize(GuiResourceUsage usage, GuiResourceError::List& errors);
			
			/// <summary>Get a contained document model using a path like "Packages\Application\Name". If the path does not exists or the type does not match, an exception will be thrown.</summary>
			/// <returns>The containd resource object.</returns>
			/// <param name="path">The path.</param>
			Ptr<DocumentModel>						GetDocumentByPath(const WString& path);
			/// <summary>Get a contained image using a path like "Packages\Application\Name". If the path does not exists or the type does not match, an exception will be thrown.</summary>
			/// <returns>The containd resource object.</returns>
			/// <param name="path">The path.</param>
			Ptr<GuiImageData>						GetImageByPath(const WString& path);
			/// <summary>Get a contained xml using a path like "Packages\Application\Name". If the path does not exists or the type does not match, an exception will be thrown.</summary>
			/// <returns>The containd resource object.</returns>
			/// <param name="path">The path.</param>
			Ptr<parsing::xml::XmlDocument>			GetXmlByPath(const WString& path);
			/// <summary>Get a contained string object using a path like "Packages\Application\Name". If the path does not exists or the type does not match, an exception will be thrown.</summary>
			/// <returns>The containd resource object.</returns>
			/// <param name="path">The path.</param>
			WString									GetStringByPath(const WString& path);
		};

/***********************************************************************
Resource Path Resolver
***********************************************************************/

		/// <summary>Represents a symbol resolver for loading a resource of a certain protocol.</summary>
		class IGuiResourcePathResolver : public IDescriptable, public Description<IGuiResourcePathResolver>
		{
		public:
			/// <summary>Load a resource when the descriptor is something like a protocol-prefixed uri.</summary>
			/// <returns>The loaded resource. Returns null if failed to load.</returns>
			/// <param name="path">The path.</param>
			virtual Ptr<DescriptableObject>					ResolveResource(const WString& path)=0;
		};

		/// <summary>Represents an <see cref="IGuiResourcePathResolver"/> factory.</summary>
		class IGuiResourcePathResolverFactory : public IDescriptable, public Description<IGuiResourcePathResolverFactory>
		{
		public:
			/// <summary>Get the protocol for this resolver.</summary>
			/// <returns>The protocol.</returns>
			virtual WString									GetProtocol()=0;

			/// <summary>Create an <see cref="IGuiResourcePathResolver"/> object.</summary>
			/// <returns>The created resolver.</returns>
			/// <param name="resource">The resource context.</param>
			/// <param name="workingDirectory">The working directory context.</param>
			virtual Ptr<IGuiResourcePathResolver>			CreateResolver(Ptr<GuiResource> resource, const WString& workingDirectory)=0;
		};
		
		/// <summary>Represents a symbol resolver for loading a resource.</summary>
		class GuiResourcePathResolver : public Object, public Description<GuiResourcePathResolver>
		{
			typedef collections::Dictionary<WString, Ptr<IGuiResourcePathResolver>>		ResolverMap;
		protected:
			ResolverMap										resolvers;
			Ptr<GuiResource>								resource;
			WString											workingDirectory;

		public:
			/// <summary>Create a resolver.</summary>
			/// <param name="_resource">The resource context.</param>
			/// <param name="_workingDirectory">The working directory context.</param>
			GuiResourcePathResolver(Ptr<GuiResource> _resource, const WString& _workingDirectory);
			~GuiResourcePathResolver();

			/// <summary>Load a resource when the descriptor is something like a protocol-prefixed uri.</summary>
			/// <returns>The loaded resource. Returns null if failed to load.</returns>
			/// <param name="protocol">The protocol.</param>
			/// <param name="path">The path.</param>
			Ptr<DescriptableObject>							ResolveResource(const WString& protocol, const WString& path);
		};

/***********************************************************************
Resource Type Resolver
***********************************************************************/

		class IGuiResourceTypeResolver_Precompile;
		class IGuiResourceTypeResolver_Initialize;
		class IGuiResourceTypeResolver_DirectLoadXml;
		class IGuiResourceTypeResolver_DirectLoadStream;
		class IGuiResourceTypeResolver_IndirectLoad;

		/// <summary>Represents a symbol type for loading a resource.</summary>
		class IGuiResourceTypeResolver : public virtual IDescriptable, public Description<IGuiResourceTypeResolver>
		{
		public:
			/// <summary>Get the type of the resource that load by this resolver.</summary>
			/// <returns>The type.</returns>
			virtual WString										GetType() = 0;
			/// <summary>Test is this resource able to serialize in an XML resource or not.</summary>
			/// <returns>Returns true if this resource is able to serialize in an XML resource.</returns>
			virtual bool										XmlSerializable() = 0;
			/// <summary>Test is this resource able to serialize in a precompiled binary resource or not.</summary>
			/// <returns>Returns true if this resource is able to serialize in a precompiled binary resource.</returns>
			virtual bool										StreamSerializable() = 0;
			
			/// <summary>Get the precompiler for the type resolver.</summary>
			/// <returns>Returns null if the type resolve does not support precompiling.</returns>
			virtual IGuiResourceTypeResolver_Precompile*		Precompile(){ return 0; }
			/// <summary>Get the initializer for the type resolver.</summary>
			/// <returns>Returns null if the type resolve does not support initializing.</returns>
			virtual IGuiResourceTypeResolver_Initialize*		Initialize(){ return 0; }
			/// <summary>Get the object for convert the resource between xml and object.</summary>
			/// <returns>Returns null if the type resolver does not have this ability.</returns>
			virtual IGuiResourceTypeResolver_DirectLoadXml*		DirectLoadXml(){ return 0; }
			/// <summary>Get the object for convert the resource between stream and object.</summary>
			/// <returns>Returns null if the type resolver does not have this ability.</returns>
			virtual IGuiResourceTypeResolver_DirectLoadStream*	DirectLoadStream(){ return 0; }
			/// <summary>Get the object for convert the resource between the preload type and the current type.</summary>
			/// <returns>Returns null if the type resolver does not have this ability.</returns>
			virtual IGuiResourceTypeResolver_IndirectLoad*		IndirectLoad(){ return 0; }
		};

		/// <summary>Provide a context for resource precompiling</summary>
		struct GuiResourcePrecompileContext
		{
			typedef collections::Dictionary<Ptr<DescriptableObject>, Ptr<DescriptableObject>>	PropertyMap;

			/// <summary>Progress callback.</summary>
			workflow::IWfCompilerCallback*						compilerCallback = nullptr;
			/// <summary>The folder to contain compiled objects.</summary>
			Ptr<GuiResourceFolder>								targetFolder;
			/// <summary>The root resource object.</summary>
			GuiResource*										rootResource = nullptr;
			/// <summary>Indicate the pass index of this precompiling pass.</summary>
			vint												passIndex = -1;
			/// <summary>The path resolver. This is only for delay load resource.</summary>
			Ptr<GuiResourcePathResolver>						resolver;
			/// <summary>Additional properties for resource item contents</summary>
			PropertyMap											additionalProperties;
		};

		/// <summary>
		///		Represents a precompiler for resources of a specified type.
		///		Current resources that needs precompiling:
		///		Workflow:
		///			Pass  0: Collect workflow scripts / Compile localized strings / Generate ClassNameRecord
		///			Pass  1: Compile workflow scripts
		///		Instance:
		///			Pass  2: Collect instance types													/ Compile animation types
		///			Pass  3: Compile
		///			Pass  4: Generate instance types with event handler functions to TemporaryClass	/ Compile animation types
		///			Pass  5: Compile
		///			Pass  6: Generate instance types with everything to InstanceCtor				/ Compile animation types
		///			Pass  7: Compile
		/// </summary>
		class IGuiResourceTypeResolver_Precompile : public virtual IDescriptable, public Description<IGuiResourceTypeResolver_Precompile>
		{
		public:
			enum PassNames
			{
				Workflow_Collect					= 0,
				Workflow_Compile					= 1,
				Workflow_Max						= Workflow_Compile,

				Instance_CollectInstanceTypes		= 2,
				Instance_CompileInstanceTypes		= 3,
				Instance_CollectEventHandlers		= 4,
				Instance_CompileEventHandlers		= 5,
				Instance_GenerateInstanceClass		= 6,
				Instance_CompileInstanceClass		= 7,
				Instance_Max						= Instance_CompileInstanceClass,
			};

			enum PassSupport
			{
				NotSupported,
				PerResource,
				PerPass,
			};

			/// <summary>Get the maximum pass index that the precompiler needs.</summary>
			/// <returns>Returns the maximum pass index. The precompiler doesn't not need to response to every pass.</returns>
			virtual vint										GetMaxPassIndex() = 0;
			/// <summary>Get how this resolver supports precompiling.</summary>
			/// <param name="passIndex">The pass index.</param>
			/// <returns>Returns how this resolver supports precompiling.</returns>
			virtual PassSupport									GetPassSupport(vint passIndex) = 0;
			/// <summary>Precompile the resource item.</summary>
			/// <param name="resource">The resource to precompile.</param>
			/// <param name="context">The context for precompiling.</param>
			/// <param name="errors">All collected errors during loading a resource.</param>
			virtual void										PerResourcePrecompile(Ptr<GuiResourceItem> resource, GuiResourcePrecompileContext& context, GuiResourceError::List& errors) = 0;
			/// <summary>Precompile for a pass.</summary>
			/// <param name="context">The context for precompiling.</param>
			/// <param name="errors">All collected errors during loading a resource.</param>
			virtual void										PerPassPrecompile(GuiResourcePrecompileContext& context, GuiResourceError::List& errors) = 0;
		};

		class IGuiResourcePrecompileCallback : public virtual IDescriptable, public Description<IGuiResourcePrecompileCallback>
		{
		public:
			virtual workflow::IWfCompilerCallback*				GetCompilerCallback() = 0;
			virtual void										OnPerPass(vint passIndex) = 0;
			virtual void										OnPerResource(vint passIndex, Ptr<GuiResourceItem> resource) = 0;
		};

		/// <summary>Provide a context for resource initializing</summary>
		struct GuiResourceInitializeContext : GuiResourcePrecompileContext
		{
			GuiResourceUsage									usage;
		};

		/// <summary>
		///		Represents a precompiler for resources of a specified type.
		///		Current resources that needs precompiling:
		///			Pass 0: Script		(initialize view model scripts)
		///			Pass 1: Script		(initialize shared scripts)
		///			Pass 2: Script		(initialize instance scripts)
		/// </summary>
		class IGuiResourceTypeResolver_Initialize : public virtual IDescriptable, public Description<IGuiResourceTypeResolver_Initialize>
		{
		public:
			/// <summary>Get the maximum pass index that the initializer needs.</summary>
			/// <returns>Returns the maximum pass index. The initializer doesn't not need to response to every pass.</returns>
			virtual vint										GetMaxPassIndex() = 0;
			/// <summary>Initialize the resource item.</summary>
			/// <param name="resource">The resource to initializer.</param>
			/// <param name="context">The context for initializing.</param>
			/// <param name="errors">All collected errors during initializing a resource.</param>
			virtual void										Initialize(Ptr<GuiResourceItem> resource, GuiResourceInitializeContext& context, GuiResourceError::List& errors) = 0;
		};

		/// <summary>Represents a symbol type for loading a resource without a preload type.</summary>
		class IGuiResourceTypeResolver_DirectLoadXml : public virtual IDescriptable, public Description<IGuiResourceTypeResolver_DirectLoadXml>
		{
		public:
			/// <summary>Serialize a resource to an xml element. This function is called if this type resolver does not have a preload type.</summary>
			/// <returns>The serialized xml element.</returns>
			/// <param name="resource">The resource item containing the resource.</param>
			/// <param name="content">The object to serialize.</param>
			virtual Ptr<parsing::xml::XmlElement>				Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content) = 0;

			/// <summary>Load a resource for a type inside an xml element.</summary>
			/// <returns>The resource.</returns>
			/// <param name="resource">The resource item containing the resource.</param>
			/// <param name="element">The xml element.</param>
			/// <param name="errors">All collected errors during loading a resource.</param>
			virtual Ptr<DescriptableObject>						ResolveResource(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlElement> element, GuiResourceError::List& errors) = 0;

			/// <summary>Load a resource for a type from a file.</summary>
			/// <returns>The resource.</returns>
			/// <param name="resource">The resource item containing the resource.</param>
			/// <param name="path">The file path.</param>
			/// <param name="errors">All collected errors during loading a resource.</param>
			virtual Ptr<DescriptableObject>						ResolveResource(Ptr<GuiResourceItem> resource, const WString& path, GuiResourceError::List& errors) = 0;
		};

		/// <summary>Represents a symbol type for loading a resource without a preload type.</summary>
		class IGuiResourceTypeResolver_DirectLoadStream : public virtual IDescriptable, public Description<IGuiResourceTypeResolver_DirectLoadStream>
		{
		public:
			/// <summary>Serialize a precompiled resource to a stream.</summary>
			/// <param name="resource">The resource item containing the resource.</param>
			/// <param name="content">The content to serialize.</param>
			/// <param name="stream">The stream.</param>
			virtual void										SerializePrecompiled(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content, stream::IStream& stream) = 0;

			/// <summary>Load a precompiled resource from a stream.</summary>
			/// <returns>The resource.</returns>
			/// <param name="resource">The resource item containing the resource.</param>
			/// <param name="stream">The stream.</param>
			/// <param name="errors">All collected errors during loading a resource.</param>
			virtual Ptr<DescriptableObject>						ResolveResourcePrecompiled(Ptr<GuiResourceItem> resource, stream::IStream& stream, GuiResourceError::List& errors) = 0;
		};

		/// <summary>Represents a symbol type for loading a resource with a preload type.</summary>
		class IGuiResourceTypeResolver_IndirectLoad : public virtual IDescriptable, public Description<IGuiResourceTypeResolver_IndirectLoad>
		{
		public:
			/// <summary>Get the preload type to load the resource before loading itself.</summary>
			/// <returns>The preload type. Returns an empty string to indicate that there is no preload type for this resolver.</returns>
			virtual WString										GetPreloadType() = 0;
			/// <summary>Get the delay load feature for this resolver.</summary>
			/// <returns>Returns true if this type need to delay load.</returns>
			virtual bool										IsDelayLoad() = 0;

			/// <summary>Serialize a resource to a resource in preload type.</summary>
			/// <returns>The serialized resource.</returns>
			/// <param name="resource">The resource item containing the resource.</param>
			/// <param name="content">The object to serialize.</param>
			virtual Ptr<DescriptableObject>						Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content) = 0;

			/// <summary>Load a resource for a type from a resource loaded by the preload type resolver.</summary>
			/// <returns>The resource.</returns>
			/// <param name="resource">The resource item containing the resource.</param>
			/// <param name="resolver">The path resolver. This is only for delay load resource.</param>
			/// <param name="errors">All collected errors during loading a resource.</param>
			virtual Ptr<DescriptableObject>						ResolveResource(Ptr<GuiResourceItem> resource, Ptr<GuiResourcePathResolver> resolver, GuiResourceError::List& errors) = 0;
		};

/***********************************************************************
Resource Resolver Manager
***********************************************************************/

		/// <summary>A resource resolver manager.</summary>
		class IGuiResourceResolverManager : public IDescriptable, public Description<IGuiResourceResolverManager>
		{
		public:
			/// <summary>Get the <see cref="IGuiResourcePathResolverFactory"/> for a protocol.</summary>
			/// <returns>The factory.</returns>
			/// <param name="protocol">The protocol.</param>
			virtual IGuiResourcePathResolverFactory*			GetPathResolverFactory(const WString& protocol) = 0;
			/// <summary>Set the <see cref="IGuiResourcePathResolverFactory"/> for a protocol.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="factory">The factory.</param>
			virtual bool										SetPathResolverFactory(Ptr<IGuiResourcePathResolverFactory> factory) = 0;
			/// <summary>Get the <see cref="IGuiResourceTypeResolver"/> for a resource type.</summary>
			/// <returns>The resolver.</returns>
			/// <param name="type">The resource type.</param>
			virtual IGuiResourceTypeResolver*					GetTypeResolver(const WString& type) = 0;
			/// <summary>Set the <see cref="IGuiResourceTypeResolver"/> for a resource type.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <param name="resolver">The resolver.</param>
			virtual bool										SetTypeResolver(Ptr<IGuiResourceTypeResolver> resolver) = 0;
			/// <summary>Get the maximum precompiling pass index.</summary>
			/// <returns>The maximum precompiling pass index.</returns>
			virtual vint										GetMaxPrecompilePassIndex() = 0;
			/// <summary>Get the maximum initializing pass index.</summary>
			/// <returns>The maximum initializing pass index.</returns>
			virtual vint										GetMaxInitializePassIndex() = 0;
			/// <summary>Get names of all per resource resolvers for a pass.</summary>
			/// <param name="passIndex">The pass index.</param>
			/// <param name="names">Names of resolvers</param>
			virtual void										GetPerResourceResolverNames(vint passIndex, collections::List<WString>& names) = 0;
			/// <summary>Get names of all per pass resolvers for a pass.</summary>
			/// <param name="passIndex">The pass index.</param>
			/// <param name="names">Names of resolvers</param>
			virtual void										GetPerPassResolverNames(vint passIndex, collections::List<WString>& names) = 0;
		};
		
		extern IGuiResourceResolverManager*						GetResourceResolverManager();
		extern void												DecompressStream(const char** buffer, bool compress, vint rows, vint block, vint remain, stream::IStream& outputStream);
	}
}

#endif