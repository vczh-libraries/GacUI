/***********************************************************************
Vczh Library++ 3.0
Developer: ³Âè÷å«(vczh)
GacStudio::EditorModel

Interfaces:
***********************************************************************/

#include "..\UI\_Source\GacStudioUI.h"

using namespace vl::collections;

namespace vm
{

/***********************************************************************
Basic IEditorModel Implementations
***********************************************************************/

	class EditorModelBase : public Object, public virtual IEditorModel
	{
	protected:
		Ptr<IEditorFactoryModel>			factory;
		Ptr<IOpenInEditorItemAction>		action;
		GuiControl*							editorControl;
		Ptr<IEditorContentModel>			currentContent;

		virtual GuiControl*					CreateEditorControl() = 0;
	public:
		EditorModelBase(Ptr<IEditorFactoryModel> _factory, Ptr<IOpenInEditorItemAction> _action);
		~EditorModelBase();

		Ptr<IEditorFactoryModel>			GetEditorFactory()override;
		Ptr<IOpenInEditorItemAction>		GetEditorAction()override;
		GuiControl*							GetEditorControl()override;
		void								Open(Ptr<IEditorContentModel> content)override;
		void								Save()override;
		void								Close()override;
	};
}