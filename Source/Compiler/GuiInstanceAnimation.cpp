#include "GuiInstanceAnimation.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace reflection::description;
		using namespace collections;
		using namespace parsing::xml;
		using namespace workflow;
		using namespace workflow::analyzer;

/***********************************************************************
GuiInstanceGradientAnimation
***********************************************************************/

		Ptr<GuiInstanceGradientAnimation> GuiInstanceGradientAnimation::LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors)
		{
			auto animation = MakePtr<GuiInstanceGradientAnimation>();
			animation->tagPosition = { {resource},xml->rootElement->codeRange.start };

			if (auto classAttr = XmlGetAttribute(xml->rootElement, L"ref.Class"))
			{
				animation->className = classAttr->value.value;
				animation->classPosition = { {resource},classAttr->value.codeRange.start };
				animation->classPosition.column += 1;
			}
			else
			{
				errors.Add({
					{{resource},xml->rootElement->codeRange.start},
					L"Precompile: Missing required attribute: \"ref.Class\"."
					});
			}

			if (auto typeAttr = XmlGetAttribute(xml->rootElement, L"Type"))
			{
				animation->typeName = typeAttr->value.value;
				animation->typePosition = { { resource },typeAttr->value.codeRange.start };
				animation->typePosition.column += 1;
			}
			else
			{
				errors.Add({
					{ { resource },xml->rootElement->codeRange.start },
					L"Precompile: Missing required attribute: \"Type\"."
					});
			}

			if (auto interpolationElement = XmlGetElement(xml->rootElement, L"Interpolation"))
			{
				if (auto cdata = interpolationElement->subNodes[0].Cast<XmlCData>())
				{
					animation->interpolation = cdata->content.value;
					animation->interpolationPosition = { { resource },cdata->codeRange.start };
					animation->interpolationPosition.column += 9; // <![CDATA[
				}
				else
				{
					errors.Add({
						{ { resource },interpolationElement->codeRange.start },
						L"Precompile: Interpolation function should be contained in a CDATA section."
						});
				}
			}

			if (auto targetsElement = XmlGetElement(xml->rootElement, L"Targets"))
			{
				FOREACH(Ptr<XmlElement>, targetElement, XmlGetElements(targetsElement, L"Target"))
				{
					Target target;

					if (auto nameAttr = XmlGetAttribute(targetElement, L"Name"))
					{
						target.name = nameAttr->value.value;
						target.namePosition = { { resource },nameAttr->value.codeRange.start };
						target.namePosition.column += 1;
					}
					else
					{
						errors.Add({
							{ { resource },targetElement->codeRange.start },
							L"Precompile: Missing required attribute: \"Name\"."
							});
					}

					if (auto interpolationElement = XmlGetElement(targetElement, L"Interpolation"))
					{
						if (auto cdata = interpolationElement->subNodes[0].Cast<XmlCData>())
						{
							target.interpolation = cdata->content.value;
							target.interpolationPosition = { { resource },cdata->codeRange.start };
							target.interpolationPosition.column += 9; // <![CDATA[
						}
						else
						{
							errors.Add({
								{ { resource },interpolationElement->codeRange.start },
								L"Precompile: Interpolation function should be contained in a CDATA section."
								});
						}
					}

					animation->targets.Add(target);
				}

				if (animation->targets.Count() == 0)
				{
					errors.Add({
						{ { resource },targetsElement->codeRange.start },
						L"Precompile: Missing required element: \"Target\" in \"Targets\"."
						});
				}
			}
			else
			{
				errors.Add({
					{ { resource },xml->rootElement->codeRange.start },
					L"Precompile: Missing required element: \"Targets\"."
					});
			}

			return animation;
		}

		Ptr<parsing::xml::XmlElement> GuiInstanceGradientAnimation::SaveToXml()
		{
			auto gradientElement = MakePtr<XmlElement>();
			{
				auto classAttr = MakePtr<XmlAttribute>();
				classAttr->name.value = L"ref.Class";
				classAttr->value.value = className;
				gradientElement->attributes.Add(classAttr);
			}
			{
				auto typeAttr = MakePtr<XmlAttribute>();
				typeAttr->name.value = L"Type";
				typeAttr->value.value = typeName;
				gradientElement->attributes.Add(typeAttr);
			}
			if (interpolation != L"")
			{
				auto interpolationElement = MakePtr<XmlElement>();
				interpolationElement->name.value = L"Interpolation";
				gradientElement->subNodes.Add(interpolationElement);

				auto cdata = MakePtr<XmlCData>();
				cdata->content.value = interpolation;
				interpolationElement->subNodes.Add(cdata);
			}
			{
				auto targetsElement = MakePtr<XmlElement>();
				targetsElement->name.value = L"Targets";
				gradientElement->subNodes.Add(targetsElement);

				FOREACH(Target, target, targets)
				{
					auto targetElement = MakePtr<XmlElement>();
					targetElement->name.value = L"Target";
					targetsElement->subNodes.Add(targetElement);
					{
						auto nameAttr = MakePtr<XmlAttribute>();
						nameAttr->name.value = L"Name";
						nameAttr->value.value = target.name;
						targetElement->attributes.Add(nameAttr);
					}
					if (target.interpolation != L"")
					{
						auto interpolationElement = MakePtr<XmlElement>();
						interpolationElement->name.value = L"Interpolation";
						targetElement->subNodes.Add(interpolationElement);

						auto cdata = MakePtr<XmlCData>();
						cdata->content.value = target.interpolation;
						interpolationElement->subNodes.Add(cdata);
					}
				}
			}
			return gradientElement;
		}

		bool GuiInstanceGradientAnimation::IsSupportedPrimitiveType(description::ITypeDescriptor* td)
		{
			if (td == description::GetTypeDescriptor<vint8_t>()) return true;
			if (td == description::GetTypeDescriptor<vint16_t>()) return true;
			if (td == description::GetTypeDescriptor<vint32_t>()) return true;
			if (td == description::GetTypeDescriptor<vint64_t>()) return true;
			if (td == description::GetTypeDescriptor<vuint8_t>()) return true;
			if (td == description::GetTypeDescriptor<vuint16_t>()) return true;
			if (td == description::GetTypeDescriptor<vuint32_t>()) return true;
			if (td == description::GetTypeDescriptor<vuint64_t>()) return true;
			if (td == description::GetTypeDescriptor<float>()) return true;
			if (td == description::GetTypeDescriptor<double>()) return true;
			return false;
		}

		vint GuiInstanceGradientAnimation::ValidateStructMembers(GuiResourceTextPos namePosition, description::ITypeDescriptor* td, const WString& prefix, GuiResourceError::List& errors)
		{
			vint members = 0;
			vint count = td->GetPropertyCount();
			for (vint i = 0; i < count; i++)
			{
				auto propInfo = td->GetProperty(i);
				members += ValidatePropertyType(namePosition, propInfo->GetReturn(), prefix + L"." + propInfo->GetName(), errors);
			}

			count = td->GetBaseTypeDescriptorCount();
			for (vint i = 0; i<count; i++)
			{
				members += ValidateStructMembers(namePosition, td->GetBaseTypeDescriptor(i), prefix, errors);
			}

			return members;
		}

		vint GuiInstanceGradientAnimation::ValidatePropertyType(GuiResourceTextPos namePosition, description::ITypeInfo* typeInfo, const WString& prefix, GuiResourceError::List& errors, bool rootValue)
		{
			auto td = typeInfo->GetTypeDescriptor();
			switch (td->GetTypeDescriptorFlags())
			{
			case TypeDescriptorFlags::Primitive:
				if (IsSupportedPrimitiveType(td))
				{
					return 1;
				}
				break;
			case TypeDescriptorFlags::Struct:
				{
					vint members = ValidateStructMembers(namePosition, td, prefix, errors);
					if (rootValue && members == 0)
					{
						errors.Add({ namePosition,L"Precompile: Property \"" + prefix + L"\" of type \"" + typeInfo->GetTypeFriendlyName() + L"\" in class \"" + typeName + L"\" is not supported. A struct should at least has one numeric primitive member to perform gradual changing." });
					}
					return members;
				}
			}
			errors.Add({ namePosition,L"Precompile: Property \"" + prefix + L"\" of type \"" + typeInfo->GetTypeFriendlyName() + L"\" in class \"" + typeName + L"\" is not supported. Only numeric types and structs are able to perform gradual changing." });
			return 0;
		}

		void GuiInstanceGradientAnimation::EnumerateMembers(EnumerateMemberCallback callback, EnumerateMemberAccessor accessor, description::IPropertyInfo* propInfo)
		{
			auto td = propInfo->GetReturn()->GetTypeDescriptor();
			switch (td->GetTypeDescriptorFlags())
			{
			case TypeDescriptorFlags::Primitive:
				callback([=](Ptr<WfExpression> expression)
				{
					auto member = MakePtr<WfMemberExpression>();
					member->parent = accessor(expression);
					member->name.value = propInfo->GetName();
					return member;
				});
				break;
			case TypeDescriptorFlags::Struct:
				EnumerateMembers(callback, accessor, td);
				break;
			}
		}

		void GuiInstanceGradientAnimation::EnumerateMembers(EnumerateMemberCallback callback, EnumerateMemberAccessor accessor, description::ITypeDescriptor* td)
		{
			vint count = td->GetPropertyCount();
			for (vint i = 0; i < count; i++)
			{
				auto propInfo = td->GetProperty(i);
				EnumerateMembers(callback, accessor, propInfo);
			}

			count = td->GetBaseTypeDescriptorCount();
			for (vint i = 0; i<count; i++)
			{
				EnumerateMembers(callback, accessor, td->GetBaseTypeDescriptor(i));
			}
		}

		void GuiInstanceGradientAnimation::EnumerateProperties(EnumerateMemberCallback callback, description::ITypeDescriptor* td)
		{
			FOREACH(Target, target, targets)
			{
				auto propInfo = td->GetPropertyByName(target.name, true);
				EnumerateMembers(callback, [](auto x) {return x; }, propInfo);
			}
		}

		Ptr<workflow::WfModule> GuiInstanceGradientAnimation::Compile(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, bool generateImpl, GuiResourceError::List& errors)
		{
			if (auto td = description::GetTypeDescriptor(typeName))
			{
				if (td->GetTypeDescriptorFlags() == TypeDescriptorFlags::Class)
				{
					auto module = MakePtr<WfModule>();
					module->name.value = moduleName;
					auto animationClass = Workflow_InstallClass(className, module);

					auto typeInfo = MakePtr<SharedPtrTypeInfo>(MakePtr<TypeDescriptorTypeInfo>(td, TypeInfoHint::Normal));
					auto typeInfoDouble = CreateTypeInfoFromTypeFlag(TypeFlag::F8);

					auto addDecl = [=](Ptr<WfDeclaration> decl)
					{
						decl->classMember = MakePtr<WfClassMember>();
						decl->classMember->kind = WfClassMemberKind::Normal;
						animationClass->declarations.Add(decl);
					};

					auto notImplemented = []()
					{
						auto block = MakePtr<WfBlockStatement>();

						auto stringExpr = MakePtr<WfStringExpression>();
						stringExpr->value.value = L"Not Implemented";

						auto raiseStat = MakePtr<WfRaiseExceptionStatement>();
						raiseStat->expression = stringExpr;

						block->statements.Add(raiseStat);
						return block;
					};

					{
						// prop Begin : <TYPE> = <DEFAULT> {}
						auto prop = MakePtr<WfAutoPropertyDeclaration>();
						addDecl(prop);

						prop->name.value = L"Begin";
						prop->type = GetTypeFromTypeInfo(typeInfo.Obj());
						prop->expression = CreateDefaultValue(typeInfo.Obj());
						prop->configConst = WfAPConst::Writable;
						prop->configObserve = WfAPObserve::Observable;
					}
					{
						// prop End : <TYPE> = <DEFAULT> {}
						auto prop = MakePtr<WfAutoPropertyDeclaration>();
						addDecl(prop);

						prop->name.value = L"End";
						prop->type = GetTypeFromTypeInfo(typeInfo.Obj());
						prop->expression = CreateDefaultValue(typeInfo.Obj());
						prop->configConst = WfAPConst::Writable;
						prop->configObserve = WfAPObserve::Observable;
					}
					{
						// prop Current : <TYPE> = <DEFAULT> {}
						auto prop = MakePtr<WfAutoPropertyDeclaration>();
						addDecl(prop);

						prop->name.value = L"Current";
						prop->type = GetTypeFromTypeInfo(typeInfo.Obj());
						prop->expression = CreateDefaultValue(typeInfo.Obj());
						prop->configConst = WfAPConst::Writable;
						prop->configObserve = WfAPObserve::Observable;
					}
					{
						// prop Interpolation : (func(double):double) = <VALUE> {const, not observe}
						auto var = MakePtr<WfVariableDeclaration>();
						addDecl(var);

						auto att = MakePtr<WfAttribute>();
						att->category.value = L"cpp";
						att->name.value = L"Private";
						var->attributes.Add(att);

						var->name.value = L"<animation>interpolation";
						var->type = GetTypeFromTypeInfo(TypeInfoRetriver<Func<double(double)>>::CreateTypeInfo().Obj());
						if (interpolation == L"" || !generateImpl)
						{
							auto ref = MakePtr<WfOrderedNameExpression>();
							ref->name.value = L"$1";

							auto lambda = MakePtr<WfOrderedLambdaExpression>();
							lambda->body = ref;

							var->expression = lambda;
						}
						else
						{
							var->expression = Workflow_ParseExpression(precompileContext, interpolationPosition.originalLocation, interpolation, interpolationPosition, errors);
						}
					}

					List<IPropertyInfo*> props;
					List<Ptr<WfExpression>> interpolations;
					FOREACH(Target, target, targets)
					{
						if (auto propInfo = td->GetPropertyByName(target.name, true))
						{
							if (!propInfo->GetGetter())
							{
								errors.Add({ target.namePosition,L"Precompile: Variable \"" + target.name + L"\" is not supported. An writable property with event is expected." });
							}
							else if (!propInfo->GetSetter())
							{
								errors.Add({ target.namePosition,L"Precompile: Readonly property \"" + target.name + L"\" is not supported. An writable property with event is expected." });
							}
							else if (!propInfo->GetValueChangedEvent())
							{
								errors.Add({ target.namePosition,L"Precompile: Property \"" + target.name + L"\" is not supported. An writable property with event is expected." });
							}
							ValidatePropertyType(target.namePosition, propInfo->GetReturn(), propInfo->GetName(), errors, true);

							Ptr<WfExpression> interpolation;
							if (target.interpolation != L"" && generateImpl)
							{
								interpolation = Workflow_ParseExpression(precompileContext, target.interpolationPosition.originalLocation, target.interpolation, target.interpolationPosition, errors);
							}

							props.Add(propInfo);
							interpolations.Add(interpolation);
						}
						else
						{
							errors.Add({ target.namePosition,L"Precompile: Property \"" + target.name + L"\" does not exist." });
						}
					}

					{
						// func GetTimeScale(<animation>begin : <TYPE>, <animation>end : <TYPE>, <animation>current : <TYPE>) : double
						auto func = MakePtr<WfFunctionDeclaration>();
						addDecl(func);

						func->anonymity = WfFunctionAnonymity::Named;
						func->name.value = L"GetTimeScale";
						{
							auto argument = MakePtr<WfFunctionArgument>();
							argument->name.value = L"<animation>begin";
							argument->type = GetTypeFromTypeInfo(typeInfo.Obj());
							func->arguments.Add(argument);
						}
						{
							auto argument = MakePtr<WfFunctionArgument>();
							argument->name.value = L"<animation>end";
							argument->type = GetTypeFromTypeInfo(typeInfo.Obj());
							func->arguments.Add(argument);
						}
						func->returnType = GetTypeFromTypeInfo(typeInfoDouble.Obj());

						func->statement = notImplemented();
					}
					{
						// func Interpolate(<animation>begin : <TYPE>, <animation>end : <TYPE>, <animation>current : <TYPE>, <animation>ratio : double) : void
						auto func = MakePtr<WfFunctionDeclaration>();
						addDecl(func);

						func->anonymity = WfFunctionAnonymity::Named;
						func->name.value = L"Interpolate";
						{
							auto argument = MakePtr<WfFunctionArgument>();
							argument->name.value = L"<animation>begin";
							argument->type = GetTypeFromTypeInfo(typeInfo.Obj());
							func->arguments.Add(argument);
						}
						{
							auto argument = MakePtr<WfFunctionArgument>();
							argument->name.value = L"<animation>end";
							argument->type = GetTypeFromTypeInfo(typeInfo.Obj());
							func->arguments.Add(argument);
						}
						{
							auto argument = MakePtr<WfFunctionArgument>();
							argument->name.value = L"<animation>current";
							argument->type = GetTypeFromTypeInfo(typeInfo.Obj());
							func->arguments.Add(argument);
						}
						{
							auto argument = MakePtr<WfFunctionArgument>();
							argument->name.value = L"<animation>ratio";
							argument->type = GetTypeFromTypeInfo(typeInfoDouble.Obj());
							func->arguments.Add(argument);
						}
						func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());

						func->statement = notImplemented();
					}
					{
						// func Interpolate(<animation>ratio : double) : void
						auto func = MakePtr<WfFunctionDeclaration>();
						addDecl(func);

						func->anonymity = WfFunctionAnonymity::Named;
						func->name.value = L"Interpolate";
						{
							auto argument = MakePtr<WfFunctionArgument>();
							argument->name.value = L"<animation>ratio";
							argument->type = GetTypeFromTypeInfo(typeInfoDouble.Obj());
							func->arguments.Add(argument);
						}
						func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());

						auto block = MakePtr<WfBlockStatement>();
						func->statement = block;

						auto refBegin = MakePtr<WfReferenceExpression>();
						refBegin->name.value = L"Begin";

						auto refEnd = MakePtr<WfReferenceExpression>();
						refEnd->name.value = L"End";

						auto refCurrent = MakePtr<WfReferenceExpression>();
						refCurrent->name.value = L"Begin";

						auto refRatio = MakePtr<WfReferenceExpression>();
						refRatio->name.value = L"<animation>ratio";

						auto refFunc = MakePtr<WfReferenceExpression>();
						refFunc->name.value = L"Interpolate";

						auto callExpr = MakePtr<WfCallExpression>();
						callExpr->function = refFunc;
						callExpr->arguments.Add(refBegin);
						callExpr->arguments.Add(refEnd);
						callExpr->arguments.Add(refCurrent);
						callExpr->arguments.Add(refRatio);

						auto exprStat = MakePtr<WfExpressionStatement>();
						exprStat->expression = callExpr;

						block->statements.Add(exprStat);
					}

					return module;
				}
				else
				{
					errors.Add({ typePosition, L"Precompile: Type \"" + typeName + L"\" is not a class." });
				}
			}
			else
			{
				errors.Add({ typePosition, L"Precompile: Type \"" + typeName + L"\" does not exist." });
			}

			return nullptr;
		}
	}
}