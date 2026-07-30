#include "GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace workflow;

/***********************************************************************
WorkflowScriptPositionVisitor
***********************************************************************/

		class WorkflowScriptPositionVisitor : public workflow::traverse_visitor::AstVisitor
		{
		public:
			GuiResourcePrecompileContext&					context;
			GuiResourceTextPos								position;
			glr::ParsingTextPos								availableAfter;
			Ptr<types::ScriptPosition>						sp;
			bool											overwriteExisting;

			WorkflowScriptPositionVisitor(GuiResourcePrecompileContext& _context, GuiResourceTextPos _position, glr::ParsingTextPos _availableAfter, bool _overwriteExisting)
				:context(_context)
				, position(_position)
				, availableAfter(_availableAfter)
				, overwriteExisting(_overwriteExisting)
			{
				sp = Workflow_EnsureScriptPosition(context);
			}

			void Traverse(glr::ParsingAstBase* node) override
			{
				if (overwriteExisting || !sp->nodePositions.Keys().Contains(node))
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

					types::ScriptPositionRecord record;
					record.position = position;
					record.availableAfter = availableAfter;
					if (pos.row < 0 || pos.column < 0)
					{
						record.computedPosition = position;
					}
					else if (pos.row == 0)
					{
						record.computedPosition = { position.originalLocation,{position.row,position.column + pos.column} };
					}
					else
					{
						record.computedPosition = { position.originalLocation,{ position.row + pos.row,pos.column } };
					}

					if (overwriteExisting)
					{
						sp->nodePositions.Set(Ptr(node), record);
					}
					else
					{
						sp->nodePositions.Add(Ptr(node), record);
					}
				}
			}
		};

/***********************************************************************
WorkflowCompiler_ScriptPosition
***********************************************************************/

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfType> node, glr::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter, false).InspectInto(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfExpression> node, glr::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter, false).InspectInto(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfStatement> node, glr::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter, false).InspectInto(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfDeclaration> node, glr::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter, false).InspectInto(node.Obj());
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfModule> node, glr::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter, false).InspectInto(node.Obj());
		}

		void Workflow_RecordScriptPositionOverwrite(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfExpression> node, glr::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter, true).InspectInto(node.Obj());
		}

		void Workflow_RecordScriptPositionOverwrite(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfStatement> node, glr::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter, true).InspectInto(node.Obj());
		}

		void Workflow_RecordScriptPositionOverwrite(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfDeclaration> node, glr::ParsingTextPos availableAfter)
		{
			WorkflowScriptPositionVisitor(context, position, availableAfter, true).InspectInto(node.Obj());
		}

		Ptr<types::ScriptPosition> Workflow_EnsureScriptPosition(GuiResourcePrecompileContext& context)
		{
			vint index = context.additionalProperties.Keys().IndexOf(nullptr);
			if (index == -1)
			{
				auto sp = Ptr(new types::ScriptPosition);
				context.additionalProperties.Add(nullptr, sp);
				return sp;
			}
			return context.additionalProperties.Values()[index].Cast<types::ScriptPosition>();
		}

		Ptr<types::ScriptPosition> Workflow_GetScriptPosition(GuiResourcePrecompileContext& context)
		{
			vint index = context.additionalProperties.Keys().IndexOf(nullptr);
			if (index == -1) return nullptr;
			return context.additionalProperties.Values()[index].Cast<types::ScriptPosition>();
		}

		void Workflow_ClearScriptPosition(GuiResourcePrecompileContext& context)
		{
			context.additionalProperties.Remove(nullptr);
		}
	}
}