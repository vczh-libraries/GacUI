#include "GuiInstanceQuery.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
ExecuteQueryVisitor
***********************************************************************/

		class ExecuteQueryVisitor : public Object, public GuiIqQuery::IVisitor
		{
		public:
			Ptr<GuiInstanceContext>				context;
			List<Ptr<GuiConstructorRepr>>&		input;
			List<Ptr<GuiConstructorRepr>>&		output;

			ExecuteQueryVisitor(Ptr<GuiInstanceContext> _context, List<Ptr<GuiConstructorRepr>>& _input, List<Ptr<GuiConstructorRepr>>& _output)
				:context(_context), input(_input), output(_output)
			{
			}

			static bool TestCtor(GuiIqPrimaryQuery* node, GlobalStringKey attribute, Ptr<GuiConstructorRepr> ctor)
			{
				if (node->attributeNameOption == GuiIqNameOption::Specified && node->attributeName.value != attribute.ToString())
				{
					return false;
				}
				if (node->typeNameOption == GuiIqNameOption::Specified && node->typeName.value != ctor->typeName.ToString())
				{
					return false;
				}
				if (node->referenceName.value != L"")
				{
					bool instanceName = ctor->instanceName != GlobalStringKey::Empty && node->referenceName.value == ctor->instanceName.ToString();
					bool styleName = ctor->styleName && node->referenceName.value == ctor->styleName.Value();
					return instanceName || styleName;
				}
				return true;
			}

			void Traverse(GuiIqPrimaryQuery* node, Ptr<GuiAttSetterRepr> setter)
			{
				if (setter)
				{
					FOREACH_INDEXER(GlobalStringKey, attribute, index, setter->setters.Keys())
					{
						auto setterValue = setter->setters.Values()[index];
						FOREACH(Ptr<GuiValueRepr>, value, setterValue->values)
						{
							if (auto ctor = value.Cast<GuiConstructorRepr>())
							{
								if (TestCtor(node, attribute, ctor))
								{
									output.Add(ctor);
								}
							}
							if (node->childOption == GuiIqChildOption::Indirect)
							{
								if (auto setter = value.Cast<GuiAttSetterRepr>())
								{
									Traverse(node, setter);
								}
							}
						}
					}
				}
				else
				{
					if (TestCtor(node, GlobalStringKey::Empty, context->instance))
					{
						output.Add(context->instance);
					}
					if (node->childOption == GuiIqChildOption::Indirect)
					{
						Traverse(node, context->instance);
					}
				}
			}

			void Visit(GuiIqPrimaryQuery* node)override
			{
				auto inputExists = &input;
				if (inputExists)
				{
					FOREACH(Ptr<GuiConstructorRepr>, setter, input)
					{
						Traverse(node, setter);
					}
				}
				else
				{
					Traverse(node, 0);
				}
			}

			void Visit(GuiIqCascadeQuery* node)override
			{
				List<Ptr<GuiConstructorRepr>> temp;
				ExecuteQuery(node->parent, context, input, temp);
				ExecuteQuery(node->child, context, temp, output);
			}

			void Visit(GuiIqSetQuery* node)override
			{
				List<Ptr<GuiConstructorRepr>> first, second;
				ExecuteQuery(node->first, context, input, first);
				ExecuteQuery(node->second, context, input, second);

				switch (node->op)
				{
				case GuiIqBinaryOperator::ExclusiveOr:
					CopyFrom(output, From(first).Except(second).Union(From(second).Except(second)));
					break;
				case GuiIqBinaryOperator::Intersect:
					CopyFrom(output, From(first).Intersect(second));
					break;
				case GuiIqBinaryOperator::Union:
					CopyFrom(output, From(first).Union(second));
					break;
				case GuiIqBinaryOperator::Substract:
					CopyFrom(output, From(first).Except(second));
					break;
				}
			}
		};

/***********************************************************************
ExecuteQuery
***********************************************************************/

		void ExecuteQuery(Ptr<GuiIqQuery> query, Ptr<GuiInstanceContext> context, collections::List<Ptr<GuiConstructorRepr>>& input, collections::List<Ptr<GuiConstructorRepr>>& output)
		{
			ExecuteQueryVisitor visitor(context, input, output);
			query->Accept(&visitor);
		}

		void ExecuteQuery(Ptr<GuiIqQuery> query, Ptr<GuiInstanceContext> context, collections::List<Ptr<GuiConstructorRepr>>& output)
		{
#if defined(VCZH_GCC) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnull-dereference"
#endif
			ExecuteQuery(query, context, *(List<Ptr<GuiConstructorRepr>>*)0, output);
#if defined(VCZH_GCC) && defined(__clang__)
#pragma clang diagnostic pop
#endif
		}

/***********************************************************************
ApplyStyle
***********************************************************************/

		void ApplyStyleInternal(Ptr<GuiAttSetterRepr> src, Ptr<GuiAttSetterRepr> dst)
		{
			FOREACH_INDEXER(GlobalStringKey, attribute, srcIndex, src->setters.Keys())
			{
				auto srcValue = src->setters.Values()[srcIndex];
				vint dstIndex = dst->setters.Keys().IndexOf(attribute);
				if (dstIndex == -1)
				{
					dst->setters.Add(attribute, srcValue);
				}
				else
				{
					auto dstValue = dst->setters.Values()[dstIndex];
					if (srcValue->binding == dstValue->binding)
					{
						if (srcValue->binding == GlobalStringKey::_Set)
						{
							ApplyStyleInternal(srcValue->values[0].Cast<GuiAttSetterRepr>(), dstValue->values[0].Cast<GuiAttSetterRepr>());
						}
						else
						{
							CopyFrom(dstValue->values, srcValue->values, true);
						}
					}
				}
			}

			FOREACH_INDEXER(GlobalStringKey, eventName, srcIndex, src->eventHandlers.Keys())
			{
				if (!dst->eventHandlers.Keys().Contains(eventName))
				{
					auto srcValue = src->eventHandlers.Values()[srcIndex];
					dst->eventHandlers.Add(eventName, srcValue);
				}
			}

			FOREACH_INDEXER(GlobalStringKey, varName, srcIndex, src->environmentVariables.Keys())
			{
				if (!dst->environmentVariables.Keys().Contains(varName))
				{
					auto srcValue = src->environmentVariables.Values()[srcIndex];
					dst->environmentVariables.Add(varName, srcValue);
				}
			}
		}

		void ApplyStyle(Ptr<GuiInstanceStyle> style, Ptr<GuiConstructorRepr> ctor)
		{
			ApplyStyleInternal(style->setter->Clone().Cast<GuiAttSetterRepr>(), ctor);
		}

/***********************************************************************
GuiIqPrint
***********************************************************************/

		class GuiIqPrintVisitor : public Object, public GuiIqQuery::IVisitor
		{
		public:
			stream::StreamWriter&				writer;

			GuiIqPrintVisitor(stream::StreamWriter& _writer)
				:writer(_writer)
			{
			}

			void Visit(GuiIqPrimaryQuery* node)override
			{
				switch (node->childOption)
				{
				case GuiIqChildOption::Direct:
					writer.WriteString(L"/");
					break;
				case GuiIqChildOption::Indirect:
					writer.WriteString(L"//");
					break;
				}

				if (node->attributeNameOption == GuiIqNameOption::Specified)
				{
					writer.WriteChar(L'@');
					writer.WriteString(node->attributeName.value);
					writer.WriteChar(L':');
				}

				if (node->typeNameOption == GuiIqNameOption::Specified)
				{
					writer.WriteString(node->typeName.value);
				}
				else
				{
					writer.WriteChar(L'*');
				}

				if (node->referenceName.value != L"")
				{
					writer.WriteChar(L'.');
					writer.WriteString(node->referenceName.value);
				}
			}

			void Visit(GuiIqCascadeQuery* node)override
			{
				node->parent->Accept(this);
				node->child->Accept(this);
			}

			void Visit(GuiIqSetQuery* node)override
			{
				writer.WriteChar(L'(');
				node->first->Accept(this);
				switch (node->op)
				{
				case GuiIqBinaryOperator::ExclusiveOr:
					writer.WriteString(L" ^ ");
					break;
				case GuiIqBinaryOperator::Intersect:
					writer.WriteString(L" * ");
					break;
				case GuiIqBinaryOperator::Union:
					writer.WriteString(L" + ");
					break;
				case GuiIqBinaryOperator::Substract:
					writer.WriteString(L" - ");
					break;
				}
				node->second->Accept(this);
				writer.WriteChar(L')');
			}
		};

		void GuiIqPrint(Ptr<GuiIqQuery> query, stream::StreamWriter& writer)
		{
			GuiIqPrintVisitor visitor(writer);
			query->Accept(&visitor);
		}
	}
}
