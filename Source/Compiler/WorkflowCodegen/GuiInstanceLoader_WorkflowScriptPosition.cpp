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
			ParsingTextPos									availableAfter;
			Ptr<types::ScriptPosition>						sp;

			WorkflowScriptPositionVisitor(GuiResourcePrecompileContext& _context, GuiResourceTextPos _position, ParsingTextPos _availableAfter)
				:context(_context)
				, position(_position)
				, availableAfter(_availableAfter)
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
					auto pos = node->codeRange.start;
					if (pos.row == availableAfter.row && pos.column > availableAfter.column)
					{
						pos.column -= availableAfter.column;
					}
					else if (pos.row > availableAfter.row)
					{
						pos.row -= availableAfter.row;
					}

					if (pos.row < 0 || pos.column < 0)
					{
						sp->nodePositions.Add(node, { position.originalLocation,pos });
					}
					else if (pos.row == 0)
					{
						sp->nodePositions.Add(node, { position.originalLocation,{position.row,position.column + pos.column} });
					}
					else
					{
						sp->nodePositions.Add(node, { position.originalLocation,{ position.row + pos.row,position.column } });
					}
				}
			}
		};

/***********************************************************************
WorkflowCompiler_ScriptPosition
***********************************************************************/

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfType> node, parsing::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter).VisitField(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfExpression> node, parsing::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter).VisitField(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfStatement> node, parsing::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter).VisitField(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfDeclaration> node, parsing::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter).VisitField(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfModule> node, parsing::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter).VisitField(node.Obj());
		}

		void Workflow_ClearScriptPosition(GuiResourcePrecompileContext& context)
		{
			context.additionalProperties.Remove(nullptr);
		}
	}
}