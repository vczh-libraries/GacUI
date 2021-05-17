#include "EditorModel.h"
#include "SolutionModel.h"

namespace vm
{

/***********************************************************************
EditorModelBase
***********************************************************************/

	EditorModelBase::EditorModelBase(Ptr<IEditorFactoryModel> _factory, Ptr<IOpenInEditorItemAction> _action)
		:factory(_factory)
		, action(_action)
	{
	}

	EditorModelBase::~EditorModelBase()
	{
		SafeDeleteControl(editorControl);
	}

	Ptr<IEditorFactoryModel> EditorModelBase::GetEditorFactory()
	{
		return factory;
	}

	Ptr<IOpenInEditorItemAction> EditorModelBase::GetEditorAction()
	{
		return action;
	}

	GuiControl* EditorModelBase::GetEditorControl()
	{
		if (!editorControl)
		{
			editorControl = CreateEditorControl();
		}
		return editorControl;
	}

	void EditorModelBase::Open(Ptr<IEditorContentModel> content)
	{
		if (currentContent)
		{
			throw StudioException(L"Internal error: An editor can only edit one item during its life cycle.", true);
		}
		currentContent = content;
		currentContent->BeginEdit(this);
	}

	void EditorModelBase::Save()
	{
		if (currentContent)
		{
			currentContent->Persist(GetEditingContent());
		}
	}

	void EditorModelBase::Close()
	{
		if (currentContent)
		{
			currentContent->EndEdit();
			currentContent = nullptr;
		}
	}
}