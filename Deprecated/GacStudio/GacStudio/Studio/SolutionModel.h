/***********************************************************************
Vczh Library++ 3.0
Developer: ³Âè÷å«(vczh)
GacStudio::SolutionModel

Interfaces:
***********************************************************************/

#ifndef GACSTUDIO_STUDIO_SOLUTIONMODEL
#define GACSTUDIO_STUDIO_SOLUTIONMODEL

#include "..\UI\_Source\GacStudioUI.h"

using namespace vl::collections;

namespace vm
{
	class StudioException : public Exception
	{
	protected:
		bool											nonConfigError;

	public:
		StudioException(const WString& message, bool _nonConfigError);
		~StudioException();

		bool											IsNonConfigError()const;
	};

	class FileMacroEnvironment : public Object, public virtual IMacroEnvironment
	{
	protected:
		IFileModel*										fileModel;
	public:
		FileMacroEnvironment(IFileModel* _fileModel);
		~FileMacroEnvironment();

		IMacroEnvironment*								GetParent()override;
		bool											HasMacro(WString name, bool inherit)override;
		WString											GetMacroValue(WString name, bool inherit)override;
	};

	class FileItem;
	class FolderItemBase;
	class FolderItem;
	class ProjectItem;
	class SolutionItem;

	extern ProjectItem*									GetOwnerProject(ISolutionItemModel* item);

	class FileItem : public Object, public virtual IFileModel, public Description<FileItem>
		, public virtual IRenameItemAction
		, public virtual IRemoveItemAction
		, public virtual IOpenInEditorItemAction
	{
		friend class FolderItemBase;
		friend class FolderItem;
		friend class ProjectItem;
	protected:
		IStudioModel*									studioModel;
		list::ObservableList<Ptr<ISolutionItemModel>>	children;
		Ptr<IFileFactoryModel>							fileFactory;
		WString											filePath;
		List<WString>									errors;
		bool											unsupported;
		ISolutionItemModel*								parent;

		Ptr<IEditorFileContentModel>					fileContent;
		List<Ptr<IEditorContentModel>>					supportedContents;
		List<Ptr<IEditorFactoryModel>>					supportedEditors;
		IEditorModel*									currentEditor;

		void											UpdateFilePath(const WString& newFilePath);
	public:
		FileItem(IStudioModel* _studioModel, Ptr<IFileFactoryModel> _fileFactory, WString _filePath, bool _unsupported = false);
		~FileItem();

		// --------------------------- action
		WString											GetRenameablePart()override;
		WString											PreviewRename(WString newName)override;
		LazyList<Ptr<ISaveItemAction>>					Rename(WString newName)override;
		LazyList<Ptr<ISaveItemAction>>					Remove()override;

		LazyList<Ptr<IEditorContentModel>>				GetSupportedContents()override;
		LazyList<Ptr<IEditorFactoryModel>>				GetSupportedEditors()override;
		IEditorModel*									GetCurrentEditor()override;
		Ptr<IEditorModel>								OpenEditor(Ptr<IEditorFactoryModel> editorFactory)override;
		void											CloseEditor()override;

		// --------------------------- feature
		Ptr<IFileFactoryModel>							GetFileFactory()override;
		void											InitializeFileAndSave()override;

		// --------------------------- solution item
		ISolutionItemModel*								GetParent()override;
		Ptr<GuiImageData>								GetImage()override;
		WString											GetName()override;
		Ptr<description::IValueObservableList>			GetChildren()override;
		WString											GetFilePath()override;
		WString											GetFileDirectory()override;
		vint											GetErrorCount()override;
		WString											GetErrorText(vint index)override;
	};

	class FolderItemBase : public Object
	{
	protected:
		list::ObservableList<Ptr<ISolutionItemModel>>	children;
		SortedList<WString>								folderNames;
		SortedList<WString>								fileNames;

		void											ClearInternal();
		void											FindFileItems(List<Ptr<FileItem>>& fileItems);
		void											AddFileItemInternal(const wchar_t* filePath, Ptr<IFileModel> fileItem);
		bool											RemoveFileItemInternal(const wchar_t* filePath, Ptr<IFileModel> fileItem);
	public:
		FolderItemBase();
		~FolderItemBase();
	};

	class FolderItem : public FolderItemBase, public virtual IFolderModel, public Description<FolderItem>
		, public virtual IAddFileItemAction
		, public virtual IRenameItemAction
		, public virtual IRemoveItemAction
	{
	protected:
		ISolutionItemModel*								parent;
		Ptr<GuiImageData>								image;
		WString											filePath;

	public:
		FolderItem(ISolutionItemModel* _parent, WString _filePath);
		~FolderItem();
		
		// --------------------------- action
		bool											HasFile(WString fileName)override;
		LazyList<Ptr<ISaveItemAction>>					AddFile(Ptr<IFileModel> file)override;
		WString											GetRenameablePart()override;
		WString											PreviewRename(WString newName)override;
		LazyList<Ptr<ISaveItemAction>>					Rename(WString newName)override;
		LazyList<Ptr<ISaveItemAction>>					Remove()override;
		
		// --------------------------- solution item
		ISolutionItemModel*								GetParent()override;
		Ptr<GuiImageData>								GetImage()override;
		WString											GetName()override;
		Ptr<description::IValueObservableList>			GetChildren()override;
		WString											GetFilePath()override;
		WString											GetFileDirectory()override;
		vint											GetErrorCount()override;
		WString											GetErrorText(vint index)override;
	};

	class ProjectItem : public FolderItemBase, public virtual IProjectModel, public Description<ProjectItem>
		, public virtual IAddFileItemAction
		, public virtual IRenameItemAction
		, public virtual IRemoveItemAction
		, public virtual ISaveItemAction
	{
		friend class FileItem;
		friend class FolderItem;
	protected:
		IStudioModel*									studioModel;
		Ptr<IProjectFactoryModel>						projectFactory;
		WString											filePath;
		List<Ptr<IFileModel>>							fileItems;
		List<WString>									errors;
		bool											unsupported;

		WString											GetNormalizedRelativePath(Ptr<IFileModel> fileItem);
		void											AddFileItem(Ptr<IFileModel> fileItem);
		void											RemoveFileItem(Ptr<IFileModel> fileItem);
		void											RenameFileItem(Ptr<IFileModel> fileItem, const WString& oldPath);
	public:
		ProjectItem(IStudioModel* _studioModel, Ptr<IProjectFactoryModel> _projectFactory, WString _filePath, bool _unsupported = false);
		~ProjectItem();
		
		// --------------------------- action
		bool											HasFile(WString fileName)override;
		LazyList<Ptr<ISaveItemAction>>					AddFile(Ptr<IFileModel> file)override;
		WString											GetRenameablePart()override;
		WString											PreviewRename(WString newName)override;
		LazyList<Ptr<ISaveItemAction>>					Rename(WString newName)override;
		LazyList<Ptr<ISaveItemAction>>					Remove()override;
		void											Save()override;
		
		// --------------------------- feature
		Ptr<IProjectFactoryModel>						GetProjectFactory()override;
		void											OpenProject()override;
		void											InitializeProjectAndSave()override;
		
		// --------------------------- solution item
		ISolutionItemModel*								GetParent()override;
		Ptr<GuiImageData>								GetImage()override;
		WString											GetName()override;
		Ptr<description::IValueObservableList>			GetChildren()override;
		WString											GetFilePath()override;
		WString											GetFileDirectory()override;
		vint											GetErrorCount()override;
		WString											GetErrorText(vint index)override;
	};

	class SolutionItem : public Object, public virtual ISolutionModel, public Description<SolutionItem>
		, public virtual ISaveItemAction
	{
	protected:
		IStudioModel*									studioModel;
		list::ObservableList<Ptr<IProjectModel>>		projects;
		Ptr<IProjectFactoryModel>						projectFactory;
		WString											filePath;
		List<WString>									errors;

	public:
		SolutionItem(IStudioModel* _studioModel, Ptr<IProjectFactoryModel> _projectFactory, WString _filePath);
		~SolutionItem();
		
		// --------------------------- action
		void											Save()override;

		// --------------------------- feature
		void											OpenSolution()override;
		void											NewSolution()override;
		void											AddProject(Ptr<IProjectModel> project)override;
		void											RemoveProject(Ptr<IProjectModel> project)override;
		
		// --------------------------- solution item
		ISolutionItemModel*								GetParent()override;
		Ptr<GuiImageData>								GetImage()override;
		WString											GetName()override;
		Ptr<description::IValueObservableList>			GetChildren()override;
		WString											GetFilePath()override;
		WString											GetFileDirectory()override;
		vint											GetErrorCount()override;
		WString											GetErrorText(vint index)override;
	};
}

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			DECL_TYPE_INFO(vm::FileItem)
			DECL_TYPE_INFO(vm::FolderItem)
			DECL_TYPE_INFO(vm::ProjectItem)
			DECL_TYPE_INFO(vm::SolutionItem)
		}
	}
}

#endif