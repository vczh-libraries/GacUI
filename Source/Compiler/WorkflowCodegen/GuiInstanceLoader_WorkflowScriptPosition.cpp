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

		class WorkflowScriptPositionVisitor
			:public Object
			, WfType::IVisitor
			, WfExpression::IVisitor
			, WfStatement::IVisitor
			, WfDeclaration::IVisitor
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

			void Record(ParsingTreeCustomBase* node)
			{
				sp->nodePositions.Add(node, position);
			}

			void Call(Ptr<WfType> node)
			{
				if (!node) return;
				Record(node.Obj());
				node->Accept(this);
			}

			void Call(Ptr<WfExpression> node)
			{
				if (!node) return;
				Record(node.Obj());
				node->Accept(this);
			}

			void Call(Ptr<WfStatement> node)
			{
				if (!node) return;
				Record(node.Obj());
				node->Accept(this);
			}

			void Call(Ptr<WfDeclaration> node)
			{
				if (!node) return;
				Record(node.Obj());
				Call(node->attributes);
				node->Accept(this);
			}

			void Call(Ptr<WfClassMember> node)
			{
				if (!node) return;
				Record(node.Obj());
				Call(node->declaration);
			}

			void Call(Ptr<WfAttribute> node)
			{
				if (!node) return;
				Record(node.Obj());
				Call(node->value);
			}

			template<typename T>
			void Call(List<Ptr<T>>& nodes)
			{
				FOREACH(Ptr<T>, node, nodes)
				{
					Call(node);
				}
			}

			//-----------------------------------------------------------------

			void Visit(WfPredefinedType* node)override
			{
			}

			void Visit(WfTopQualifiedType* node)override
			{
			}

			void Visit(WfReferenceType* node)override
			{
			}

			void Visit(WfRawPointerType* node)override
			{
				Call(node->element);
			}

			void Visit(WfSharedPointerType* node)override
			{
				Call(node->element);
			}

			void Visit(WfNullableType* node)override
			{
				Call(node->element);
			}

			void Visit(WfEnumerableType* node)override
			{
				Call(node->element);
			}

			void Visit(WfMapType* node)override
			{
				Call(node->key);
				Call(node->value);
			}

			void Visit(WfFunctionType* node)override
			{
				Call(node->result);
				Call(node->arguments);
			}

			void Visit(WfChildType* node)override
			{
				Call(node->parent);
			}

			//-----------------------------------------------------------------

			void Visit(WfThisExpression* node)override
			{
			}

			void Visit(WfTopQualifiedExpression* node)override
			{
			}

			void Visit(WfReferenceExpression* node)override
			{
			}

			void Visit(WfOrderedNameExpression* node)override
			{
			}

			void Visit(WfOrderedLambdaExpression* node)override
			{
				Call(node->body);
			}

			void Visit(WfMemberExpression* node)override
			{
				Call(node->parent);
			}

			void Visit(WfChildExpression* node)override
			{
				Call(node->parent);
			}

			void Visit(WfLiteralExpression* node)override
			{
			}

			void Visit(WfFloatingExpression* node)override
			{
			}

			void Visit(WfIntegerExpression* node)override
			{
			}

			void Visit(WfStringExpression* node)override
			{
			}

			void Visit(WfFormatExpression* node)override
			{
				Call(node->expandedExpression);
			}

			void Visit(WfUnaryExpression* node)override
			{
				Call(node->operand);
			}

			void Visit(WfBinaryExpression* node)override
			{
				Call(node->first);
				Call(node->second);
			}

			void Visit(WfLetExpression* node)override
			{
				FOREACH(Ptr<WfLetVariable>, var, node->variables)
				{
					Record(var.Obj());
					Call(var->value);
				}
				Call(node->expression);
			}

			void Visit(WfIfExpression* node)override
			{
				Call(node->condition);
				Call(node->trueBranch);
				Call(node->falseBranch);
			}

			void Visit(WfRangeExpression* node)override
			{
				Call(node->begin);
				Call(node->end);
			}

			void Visit(WfSetTestingExpression* node)override
			{
				Call(node->element);
				Call(node->collection);
			}

			void Visit(WfConstructorExpression* node)override
			{
				FOREACH(Ptr<WfConstructorArgument>, argument, node->arguments)
				{
					Record(argument.Obj());
					Call(argument->key);
					Call(argument->value);
				}
			}

			void Visit(WfInferExpression* node)override
			{
				Call(node->expression);
				Call(node->type);
			}

			void Visit(WfTypeCastingExpression* node)override
			{
				Call(node->expression);
				Call(node->type);
			}

			void Visit(WfTypeTestingExpression* node)override
			{
				Call(node->expression);
				Call(node->type);
			}

			void Visit(WfTypeOfTypeExpression* node)override
			{
				Call(node->type);
			}

			void Visit(WfTypeOfExpressionExpression* node)override
			{
				Call(node->expression);
			}

			void Visit(WfAttachEventExpression* node)override
			{
				Call(node->event);
				Call(node->function);
			}

			void Visit(WfDetachEventExpression* node)override
			{
				Call(node->event);
				Call(node->handler);
			}

			void Visit(WfBindExpression* node)override
			{
				Call(node->expression);
				Call(node->expandedExpression);
			}

			void Visit(WfObserveExpression* node)override
			{
				Call(node->parent);
				Call(node->expression);
				Call(node->events);
			}

			void Visit(WfCallExpression* node)override
			{
				Call(node->function);
				Call(node->arguments);
			}

			void Visit(WfFunctionExpression* node)override
			{
				Call(node->function.Obj());
			}

			void Visit(WfNewClassExpression* node)override
			{
				Call(node->type);
				Call(node->arguments);
			}

			void Visit(WfNewInterfaceExpression* node)override
			{
				Call(node->type);
				Call(node->members);
			}

			//-----------------------------------------------------------------

			void Visit(WfBreakStatement* node)override
			{
			}

			void Visit(WfContinueStatement* node)override
			{
			}

			void Visit(WfReturnStatement* node)override
			{
				Call(node->expression);
			}

			void Visit(WfDeleteStatement* node)override
			{
				Call(node->expression);
			}

			void Visit(WfRaiseExceptionStatement* node)override
			{
				Call(node->expression);
			}

			void Visit(WfIfStatement* node)override
			{
				Call(node->type);
				Call(node->expression);
				Call(node->trueBranch);
				Call(node->falseBranch);
			}

			void Visit(WfSwitchStatement* node)override
			{
				FOREACH(Ptr<WfSwitchCase>, switchCase, node->caseBranches)
				{
					Record(switchCase.Obj());
					Call(switchCase->expression);
					Call(switchCase->statement);
				}
				Call(node->expression);
				Call(node->defaultBranch);
			}

			void Visit(WfWhileStatement* node)override
			{
				Call(node->condition);
				Call(node->statement);
			}

			void Visit(WfForEachStatement* node)override
			{
				Call(node->collection);
				Call(node->statement);
			}

			void Visit(WfTryStatement* node)override
			{
				Call(node->protectedStatement);
				Call(node->catchStatement);
				Call(node->finallyStatement);
			}

			void Visit(WfBlockStatement* node)override
			{
				Call(node->statements);
			}

			void Visit(WfExpressionStatement* node)override
			{
				Call(node->expression);
			}

			void Visit(WfVariableStatement* node)override
			{
				Call(node->variable.Obj());
			}

			//-----------------------------------------------------------------

			void Visit(WfNamespaceDeclaration* node)override
			{
				Call(node->declarations);
			}

			void Visit(WfFunctionDeclaration* node)override
			{
				FOREACH(Ptr<WfFunctionArgument>, argument, node->arguments)
				{
					Record(argument.Obj());
					Call(argument->attributes);
					Call(argument->type);
				}
				Call(node->returnType);
				Call(node->statement);
			}

			void Visit(WfVariableDeclaration* node)override
			{
				Call(node->type);
				Call(node->expression);
			}

			void Visit(WfEventDeclaration* node)override
			{
				Call(node->arguments);
			}

			void Visit(WfPropertyDeclaration* node)override
			{
				Call(node->type);
			}

			void Visit(WfConstructorDeclaration* node)override
			{
				FOREACH(Ptr<WfBaseConstructorCall>, call, node->baseConstructorCalls)
				{
					Record(call.Obj());
					Call(call->type);
					Call(call->arguments);
				}
				FOREACH(Ptr<WfFunctionArgument>, argument, node->arguments)
				{
					Record(argument.Obj());
					Call(argument->attributes);
					Call(argument->type);
				}
				Call(node->statement);
			}

			void Visit(WfDestructorDeclaration* node)override
			{
				Call(node->statement);
			}

			void Visit(WfClassDeclaration* node)override
			{
				Call(node->baseTypes);
				Call(node->members);
			}

			void Visit(WfEnumDeclaration* node)override
			{
				FOREACH(Ptr<WfEnumItem>, item, node->items)
				{
					Record(item.Obj());
					Call(item->attributes);
					FOREACH(Ptr<WfEnumItemIntersection>, itemInt, item->intersections)
					{
						Record(itemInt.Obj());
					}
				}
			}

			void Visit(WfStructDeclaration* node)override
			{
				FOREACH(Ptr<WfStructMember>, member, node->members)
				{
					Record(member.Obj());
					Call(member->attributes);
					Call(member->type);
				}
			}
		};

/***********************************************************************
WorkflowCompiler_ScriptPosition
***********************************************************************/

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfType> node)
		{
			WorkflowScriptPositionVisitor(context, position).Call(node);
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfExpression> node)
		{
			WorkflowScriptPositionVisitor(context, position).Call(node);
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfStatement> node)
		{
			WorkflowScriptPositionVisitor(context, position).Call(node);
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfDeclaration> node)
		{
			WorkflowScriptPositionVisitor(context, position).Call(node);
		}

		void Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfModule> node)
		{
			WorkflowScriptPositionVisitor visitor(context, position);
			FOREACH(Ptr<WfModuleUsingPath>, path, node->paths)
			{
				visitor.Record(path.Obj());
				FOREACH(Ptr<WfModuleUsingItem>, item, path->items)
				{
					visitor.Record(item.Obj());
					FOREACH(Ptr<WfModuleUsingFragment>, fragment, item->fragments)
					{
						visitor.Record(fragment.Obj());
					}
				}
			}
			visitor.Call(node->declarations);
		}
	}
}