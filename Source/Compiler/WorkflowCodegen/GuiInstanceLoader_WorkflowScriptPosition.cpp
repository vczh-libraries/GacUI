#include "GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace parsing;
		using namespace workflow;

/***********************************************************************
WorkflowScriptPositionVisitor
***********************************************************************/

		class WorkflowScriptPositionVisitor : public traverse_visitor::ModuleVisitor
		{
		public:
			GuiResourcePrecompileContext&					context;
			GuiResourceTextPos								position;
			Ptr<types::ScriptPosition>						sp;

			WorkflowScriptPositionVisitor(GuiResourcePrecompileContext& _context, GuiResourceTextPos _position)
				:context(_context)
				, position(_position)
			{
				vint index = context.additionalProperties.Keys().IndexOf(nullptr);
				if (index == -1)
				{
					context.additionalProperties.Add(nullptr, MakePtr<types::ScriptPosition>());
				}
				sp = context.additionalProperties[nullptr].Cast<types::ScriptPosition>();
			}

			virtual void Traverse(ParsingTreeCustomBase* node)override
			{
				if (!sp->nodePositions.Keys().Contains(node))
				{
					sp->nodePositions.Add(node, position);
				}
			}
		};

/***********************************************************************
WorkflowCompiler_ScriptPosition
***********************************************************************/

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfType> node)
		{
			WorkflowScriptPositionVisitor(context, position).VisitField(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfExpression> node)
		{
			WorkflowScriptPositionVisitor(context, position).VisitField(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfStatement> node)
		{
			WorkflowScriptPositionVisitor(context, position).VisitField(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfDeclaration> node)
		{
			WorkflowScriptPositionVisitor(context, position).VisitField(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfModule> node)
		{
			WorkflowScriptPositionVisitor(context, position).VisitField(node.Obj());
		}

		void Workflow_ClearScriptPosition(GuiResourcePrecompileContext& context)
		{
			context.additionalProperties.Remove(nullptr);
		}
	}
}