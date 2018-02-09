#include "GuiInstanceAnimation.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace reflection::description;
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
			return gradientElement;
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
						auto prop = MakePtr<WfAutoPropertyDeclaration>();
						addDecl(prop);

						prop->name.value = L"Interpolation";
						prop->type = GetTypeFromTypeInfo(TypeInfoRetriver<Func<double(double)>>::CreateTypeInfo().Obj());
						if (interpolation == L"" || !generateImpl)
						{
							auto ref = MakePtr<WfOrderedNameExpression>();
							ref->name.value = L"$1";

							auto lambda = MakePtr<WfOrderedLambdaExpression>();
							lambda->body = ref;

							prop->expression = lambda;
						}
						else
						{
							prop->expression = Workflow_ParseExpression(precompileContext, interpolationPosition.originalLocation, interpolation, interpolationPosition, errors);
						}
						prop->configConst = WfAPConst::Readonly;
						prop->configObserve = WfAPObserve::NotObservable;
					}
					{
						// func GetDistance(<animation>begin : <TYPE>, <animation>end : <TYPE>) : double
						auto func = MakePtr<WfFunctionDeclaration>();
						addDecl(func);

						func->anonymity = WfFunctionAnonymity::Named;
						func->name.value = L"GetDistance";
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
						// func Interpolate(<animation>begin : <TYPE>, <animation>end : <TYPE>, <animation>out : <TYPE>, <animation>ratio : double) : void
						auto func = MakePtr<WfFunctionDeclaration>();
						addDecl(func);

						func->anonymity = WfFunctionAnonymity::Named;
						func->name.value = L"GetDistance";
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
							argument->name.value = L"<animation>out";
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