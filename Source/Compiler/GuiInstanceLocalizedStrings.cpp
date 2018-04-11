#include "GuiInstanceLocalizedStrings.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace parsing;
		using namespace parsing::xml;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace reflection::description;

/***********************************************************************
GuiInstanceLocalizedStrings
***********************************************************************/

		WString GuiInstanceLocalizedStrings::Strings::GetLocalesName()
		{
			return From(locales).Aggregate(WString(L""), [](const WString& a, const WString& b)
			{
				return a == L"" ? b : a + L";" + b;
			});
		}

		Ptr<GuiInstanceLocalizedStrings> GuiInstanceLocalizedStrings::LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors)
		{
			auto ls = MakePtr<GuiInstanceLocalizedStrings>();

			if (xml->rootElement->name.value!=L"LocalizedStrings")
			{
				errors.Add(GuiResourceError({ { resource },xml->rootElement->codeRange.start }, L"Precompile: The root element of localized strings should be \"LocalizedStrings\"."));
				return nullptr;
			}
			ls->tagPosition = { {resource},xml->rootElement->name.codeRange.start };

			auto attClassName = XmlGetAttribute(xml->rootElement, L"ref.Class");
			if (!attClassName)
			{
				errors.Add(GuiResourceError({ { resource },xml->rootElement->codeRange.start }, L"Precompile: Missing attribute \"ref.Class\" in \"LocalizedStrings\"."));
			}
			else
			{
				ls->className = attClassName->value.value;
			}

			auto attDefaultLocale = XmlGetAttribute(xml->rootElement, L"DefaultLocale");
			if (!attDefaultLocale)
			{
				errors.Add(GuiResourceError({ { resource },xml->rootElement->codeRange.start }, L"Precompile: Missing attribute \"DefaultLocale\" in \"LocalizedStrings\"."));
			}
			else
			{
				ls->defaultLocale = attDefaultLocale->value.value;
			}

			if (!attClassName || !attDefaultLocale)
			{
				return nullptr;
			}

			SortedList<WString> existingLocales;
			FOREACH(Ptr<XmlElement>, xmlStrings, XmlGetElements(xml->rootElement))
			{
				if (xmlStrings->name.value != L"Strings")
				{
					errors.Add(GuiResourceError({ { resource },xmlStrings->codeRange.start }, L"Precompile: Unknown element \"" + xmlStrings->name.value + L"\", it should be \"Strings\"."));
					continue;
				}

				auto attLocales = XmlGetAttribute(xmlStrings, L"Locales");
				if (!attLocales)
				{
					errors.Add(GuiResourceError({ { resource },xmlStrings->codeRange.start }, L"Precompile: Missing attribute \"Locales\" in \"Strings\"."));
				}
				else
				{
					auto lss = MakePtr<GuiInstanceLocalizedStrings::Strings>();
					ls->strings.Add(lss);
					lss->tagPosition = { { resource },xmlStrings->name.codeRange.start };
					SplitBySemicolon(attLocales->value.value, lss->locales);

					FOREACH(WString, locale, lss->locales)
					{
						if (!existingLocales.Contains(locale))
						{
							existingLocales.Add(locale);
						}
						else
						{
							errors.Add(GuiResourceError({ { resource },attLocales->codeRange.start }, L"Precompile: Locale \"" + locale + L"\" already exists."));
						}
					}

					FOREACH(Ptr<XmlElement>, xmlString, XmlGetElements(xmlStrings))
					{
						if (xmlString->name.value != L"String")
						{
							errors.Add(GuiResourceError({ { resource },xmlString->codeRange.start }, L"Precompile: Unknown element \"" + xmlString->name.value + L"\", it should be \"String\"."));
							continue;
						}

						auto attName = XmlGetAttribute(xmlString, L"Name");
						auto attText = XmlGetAttribute(xmlString, L"Text");

						if (!attName)
						{
							errors.Add(GuiResourceError({ { resource },xmlString->codeRange.start }, L"Precompile: Missing attribute \"Name\" in \"String\"."));
						}
						if (!attText)
						{
							errors.Add(GuiResourceError({ { resource },xmlString->codeRange.start }, L"Precompile: Missing attribute \"Text\" in \"String\"."));
						}

						if (attName && attText)
						{
							if (lss->items.Keys().Contains(attName->value.value))
							{
								errors.Add(GuiResourceError({ { resource },xmlString->codeRange.start }, L"Precompile: String \"" + attName->value.value + L"\" already exists."));
							}
							else
							{
								auto item = MakePtr<GuiInstanceLocalizedStrings::StringItem>();
								item->name = attName->value.value;
								item->text = attText->value.value;
								item->textPosition = { {resource},attText->value.codeRange.start };
								item->textPosition.column += 1;
								lss->items.Add(item->name, item);
							}
						}
					}
				}
			}

			if (!existingLocales.Contains(ls->defaultLocale))
			{
				errors.Add(GuiResourceError({ { resource },xml->rootElement->codeRange.start }, L"Precompile: Strings for the default locale \"" + ls->defaultLocale + L"\" is not defined."));
			}

			return ls;
		}

		Ptr<parsing::xml::XmlElement> GuiInstanceLocalizedStrings::SaveToXml()
		{
			auto xml = MakePtr<XmlElement>();
			xml->name.value = L"LocalizedStrings";
			{
				auto att = MakePtr<XmlAttribute>();
				att->name.value = L"ref.Class";
				att->value.value = className;
				xml->attributes.Add(att);
			}
			{
				auto att = MakePtr<XmlAttribute>();
				att->name.value = L"DefaultLocale";
				att->value.value = defaultLocale;
				xml->attributes.Add(att);
			}

			FOREACH(Ptr<GuiInstanceLocalizedStrings::Strings>, lss, strings)
			{
				auto xmlStrings = MakePtr<XmlElement>();
				xml->subNodes.Add(xmlStrings);
				xmlStrings->name.value = L"Strings";
				{
					auto att = MakePtr<XmlAttribute>();
					att->name.value = L"Strings";
					att->value.value = lss->GetLocalesName();
					xmlStrings->attributes.Add(att);
				}

				FOREACH(Ptr<GuiInstanceLocalizedStrings::StringItem>, lssi, lss->items.Values())
				{
					auto xmlString = MakePtr<XmlElement>();
					xmlStrings->subNodes.Add(xmlString);
					{
						auto att = MakePtr<XmlAttribute>();
						att->name.value = L"Name";
						att->value.value = lssi->name;
						xmlString->attributes.Add(att);
					}
					{
						auto att = MakePtr<XmlAttribute>();
						att->name.value = L"Text";
						att->value.value = lssi->text;
						xmlString->attributes.Add(att);
					}
				}
			}

			return xml;
		}

		Ptr<GuiInstanceLocalizedStrings::Strings> GuiInstanceLocalizedStrings::GetDefaultStrings()
		{
			return From(strings)
				.Where([=](Ptr<Strings> strings)
				{
					return strings->locales.Contains(defaultLocale);
				})
				.First();
		}

		WString GuiInstanceLocalizedStrings::GetInterfaceTypeName(bool hasNamespace)
		{
			auto pair = INVLOC.FindLast(className, L"::", Locale::None);
			if (pair.key == -1)
			{
				return L"I" + className + L"Strings";
			}
			else
			{
				auto ns = className.Left(pair.key + 2);
				auto name = className.Right(className.Length() - ns.Length());
				return(hasNamespace ? ns : L"") + L"I" + name + L"Strings";
			}
		}

		Ptr<GuiInstanceLocalizedStrings::TextDesc> GuiInstanceLocalizedStrings::ParseLocalizedText(const WString& text, GuiResourceTextPos pos, GuiResourceError::List& errors)
		{
			const wchar_t* reading = text.Buffer();
			const wchar_t* textPosCounter = reading;
			ParsingTextPos formatPos(0, 0);
			auto textDesc = MakePtr<TextDesc>();

			auto addError = [&](const WString& message)
			{
				auto errorPos = pos;
				errorPos.row += formatPos.row;
				errorPos.column = (formatPos.row == 0 ? errorPos.column : 0) + formatPos.column;
				errors.Add({ errorPos,message });
			};

			bool addedParameter = true;
			while (*reading)
			{
				const wchar_t* begin = wcsstr(reading, L"$(");
				if (begin)
				{
					auto text = WString(reading, vint(begin - reading));
					if (addedParameter)
					{
						textDesc->texts.Add(text);
					}
					else
					{
						textDesc->texts[textDesc->texts.Count() - 1] += text;
					}
				}
				else
				{
					break;
				}

				const wchar_t* end = wcsstr(begin, L")");
				if (!end)
				{
					addError(L"Precompile: Does not find matched close bracket.");
					return nullptr;
				}

				while (textPosCounter++ < begin + 2)
				{
					switch (textPosCounter[-1])
					{
					case '\n':
						formatPos.row++;
						formatPos.column = 0;
						break;
					default:
						formatPos.column++;
						break;
					}
				}

				if (end - begin == 3 && wcsncmp(begin, L"$($)", 4) == 0)
				{
					addedParameter = false;
					textDesc->texts[textDesc->texts.Count() - 1] += L"$";
				}
				else
				{
					addedParameter = true;
					const wchar_t* number = begin + 2;
					const wchar_t* numberEnd = number;
					while (L'0' <= *numberEnd && *numberEnd < L'9')
					{
						numberEnd++;
					}

					if (number == numberEnd)
					{
						addError(L"Precompile: Unexpected character, the correct format is $(index) or $(index:function).");
						return nullptr;
					}

					Ptr<ITypeInfo> type;
					WString function;
					if (*numberEnd == L':')
					{
						if (end - numberEnd > 1)
						{
							function = WString(numberEnd + 1, (vint)(end - numberEnd - 1));
							if (function == L"ShortDate" || function == L"LongDate" || function == L"YearMonthDate" || function == L"ShortTime" || function == L"LongTime")
							{
								type = TypeInfoRetriver<DateTime>::CreateTypeInfo();
							}
							else if (function.Length() >= 5 && (function.Left(5) == L"Date:" || function.Left(5) == L"Time:"))
							{
								type = TypeInfoRetriver<DateTime>::CreateTypeInfo();
							}
							else if (function == L"Number" || function == L"Currency")
							{
								type = TypeInfoRetriver<WString>::CreateTypeInfo();
							}
							else
							{
								addError(L"Precompile: Unknown formatting function name \"" + function + L"\".");
								return nullptr;
							}
						}
						else
						{
							addError(L"Precompile: Unexpected character, the correct format is $(index) or $(index:function).");
							return nullptr;
						}
					}
					else if (numberEnd != end)
					{
						addError(L"Precompile: Unexpected character, the correct format is $(index) or $(index:function).");
						return nullptr;
					}

					if (!type)
					{
						type = TypeInfoRetriver<WString>::CreateTypeInfo();
					}
					textDesc->parameters.Add({ type,function });
					textDesc->positions.Add(wtoi(WString(number, (vint)(numberEnd - number))));
				}
				reading = end + 1;
			}

			if (*reading || textDesc->texts.Count() == 0)
			{
				textDesc->texts.Add(reading);
			}

			FOREACH_INDEXER(vint, i, index, From(textDesc->positions).OrderBy([](vint a, vint b) {return a - b; }))
			{
				if (i != index)
				{
					errors.Add({ pos,L"Precompile: Missing parameter \"" + itow(index) + L"\"." });
					return nullptr;
				}
			}
			return textDesc;
		}

		void GuiInstanceLocalizedStrings::Validate(TextDescMap& textDescs, GuiResourcePrecompileContext& precompileContext, GuiResourceError::List& errors)
		{
			auto defaultStrings = GetDefaultStrings();

			vint errorCount = errors.Count();
			FOREACH(Ptr<Strings>, lss, strings)
			{
				if (lss != defaultStrings)
				{
					auto localesName = lss->GetLocalesName();

					auto missing = From(defaultStrings->items.Keys())
						.Except(lss->items.Keys())
						.Aggregate(WString(L""), [](const WString& a, const WString& b)
						{
							return a == L"" ? b : a + L", " + b;
						});
					
					auto extra = From(lss->items.Keys())
						.Except(defaultStrings->items.Keys())
						.Aggregate(WString(L""), [](const WString& a, const WString& b)
						{
							return a == L"" ? b : a + L", " + b;
						});

					if (missing != L"")
					{
						errors.Add({ lss->tagPosition,L"Precompile: Missing strings for locale \"" + localesName + L"\": " + missing + L"." });
					}

					if (extra != L"")
					{
						errors.Add({ lss->tagPosition,L"Precompile: Unnecessary strings for locale \"" + localesName + L"\": " + extra + L"." });
					}
				}
			}
			if (errors.Count() != errorCount)
			{
				return;
			}

			FOREACH(Ptr<StringItem>, lssi, defaultStrings->items.Values())
			{
				if (auto textDesc = ParseLocalizedText(lssi->text, lssi->textPosition, errors))
				{
					textDescs.Add({ defaultStrings,lssi->name }, textDesc);
				}
			}
			if (errors.Count() != errorCount)
			{
				return;
			}

			auto defaultLocalesName = defaultStrings->GetLocalesName();
			FOREACH(Ptr<Strings>, lss, strings)
			{
				if (lss != defaultStrings)
				{
					auto localesName = lss->GetLocalesName();

					FOREACH(Ptr<StringItem>, lssi, lss->items.Values())
					{
						if (auto textDesc = ParseLocalizedText(lssi->text, lssi->textPosition, errors))
						{
							textDescs.Add({ lss,lssi->name }, textDesc);
							auto defaultDesc = textDescs[{defaultStrings, lssi->name}];
							if (defaultDesc->parameters.Count() != textDesc->parameters.Count())
							{
								errors.Add({ lss->tagPosition,L"String \"" + lssi->name + L"\" in locales \"" + defaultLocalesName + L"\" and \"" + localesName + L"\" have different numbers of parameters." });
							}
							else
							{
								for (vint i = 0; i < textDesc->parameters.Count(); i++)
								{
									auto defaultParameter = defaultDesc->parameters[defaultDesc->positions[i]];
									auto parameter = textDesc->parameters[textDesc->positions[i]];

									if (defaultParameter.key->GetTypeDescriptor()->GetTypeName() != parameter.key->GetTypeDescriptor()->GetTypeName())
									{
										errors.Add({ lss->tagPosition,L"Parameter \"" + itow(i) + L"\" in String \"" + lssi->name + L"\" in locales \"" + defaultLocalesName + L"\" and \"" + localesName + L"\" are in different types \"" + defaultParameter.key->GetTypeFriendlyName() + L"\" and \"" + parameter.key->GetTypeFriendlyName() + L"\"." });
									}
								}
							}
						}
					}
				}
			}
			if (errors.Count() != errorCount)
			{
				return;
			}
		}

		Ptr<workflow::WfFunctionDeclaration> GuiInstanceLocalizedStrings::GenerateFunction(Ptr<TextDesc> textDesc, const WString& functionName, workflow::WfClassMemberKind classMemberKind)
		{
			auto func = MakePtr<WfFunctionDeclaration>();
			func->anonymity = WfFunctionAnonymity::Named;
			func->name.value = functionName;
			func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<WString>::CreateTypeInfo().Obj());
			{
				auto member = MakePtr<WfClassMember>();
				member->kind = classMemberKind;
				func->classMember = member;
			}
			for (vint i = 0; i < textDesc->positions.Count(); i++)
			{
				auto type = textDesc->parameters[textDesc->positions[i]];

				auto argument = MakePtr<WfFunctionArgument>();
				argument->name.value = L"<ls>" + itow(i);
				argument->type = GetTypeFromTypeInfo(type.key.Obj());
				func->arguments.Add(argument);
			}

			return func;
		}

		Ptr<workflow::WfExpression> GuiInstanceLocalizedStrings::GenerateStrings(TextDescMap& textDescs, Ptr<Strings> ls)
		{
			auto lsExpr = MakePtr<WfNewInterfaceExpression>();
			{
				auto refType = MakePtr<WfReferenceType>();
				refType->name.value = GetInterfaceTypeName(false);

				auto refPointer = MakePtr<WfSharedPointerType>();
				refPointer->element = refType;

				lsExpr->type = refPointer;
			}

			FOREACH(Ptr<StringItem>, lss, ls->items.Values())
			{
				auto textDesc = textDescs[{ls, lss->name}];
				auto func = GenerateFunction(textDesc, lss->name, WfClassMemberKind::Override);
				lsExpr->declarations.Add(func);

				auto block = MakePtr<WfBlockStatement>();
				func->statement = block;

				Ptr<WfExpression> resultExpr;

				auto appendExpr = [&](Ptr<WfExpression> strExpr)
				{
					if (resultExpr)
					{
						auto binaryExpr = MakePtr<WfBinaryExpression>();
						binaryExpr->op = WfBinaryOperator::Union;
						binaryExpr->first = resultExpr;
						binaryExpr->second = strExpr;

						resultExpr = binaryExpr;
					}
					else
					{
						resultExpr = strExpr;
					}
				};

				for (vint i = 0; i < textDesc->parameters.Count(); i++)
				{
					auto varDesc = MakePtr<WfVariableDeclaration>();
					varDesc->name.value = L"<ls>_" + itow(i);

					auto varStat = MakePtr<WfVariableStatement>();
					varStat->variable = varDesc;
					block->statements.Add(varStat);

					auto type = textDesc->parameters[i].key;
					auto function = textDesc->parameters[i].value;
					auto index = textDesc->positions[i];

					if (function == L"ShortDate" || function == L"LongDate" || function == L"YearMonthDate" || function == L"ShortTime" || function == L"LongTime")
					{
						auto refLoc = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<Localization>());

						auto refFormats = MakePtr<WfChildExpression>();
						refFormats->parent = refLoc;
						refFormats->name.value = L"Get" + function + L"Formats";

						auto refLocale = MakePtr<WfReferenceExpression>();
						refLocale->name.value = L"<ls>locale";

						auto callFormats = MakePtr<WfCallExpression>();
						callFormats->function = refFormats;
						callFormats->arguments.Add(refLocale);

						auto refFirst = MakePtr<WfReferenceExpression>();
						refFirst->name.value = L"<ls>First";

						auto callFirst = MakePtr<WfCallExpression>();
						{
							callFirst->function = refFirst;
							callFirst->arguments.Add(callFormats);
						}

						auto refLocale2 = MakePtr<WfReferenceExpression>();
						refLocale2->name.value = L"<ls>locale";

						auto refParameter = MakePtr<WfReferenceExpression>();
						refParameter->name.value = L"<ls>" + itow(index);

						auto refLoc2 = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<Localization>());

						auto refFD = MakePtr<WfChildExpression>();
						refFD->parent = refLoc2;
						refFD->name.value = L"Format" + function.Right(4);

						auto callFD = MakePtr<WfCallExpression>();
						callFD->function = refFD;
						callFD->arguments.Add(refLocale2);
						callFD->arguments.Add(callFirst);
						callFD->arguments.Add(refParameter);

						varDesc->expression = callFD;
					}
					else if (function.Length() >= 5 && (function.Left(5) == L"Date:" || function.Left(5) == L"Time:"))
					{
						auto refLocale = MakePtr<WfReferenceExpression>();
						refLocale->name.value = L"<ls>locale";

						auto refFormat = MakePtr<WfStringExpression>();
						refFormat->value.value = function.Right(function.Length() - 5);

						auto refParameter = MakePtr<WfReferenceExpression>();
						refParameter->name.value = L"<ls>" + itow(index);

						auto refLoc2 = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<Localization>());

						auto refFD = MakePtr<WfChildExpression>();
						refFD->parent = refLoc2;
						refFD->name.value = L"Format" + function.Left(4);

						auto callFD = MakePtr<WfCallExpression>();
						callFD->function = refFD;
						callFD->arguments.Add(refLocale);
						callFD->arguments.Add(refFormat);
						callFD->arguments.Add(refParameter);

						varDesc->expression = callFD;
					}
					else if (function == L"Number" || function == L"Currency")
					{
						auto refLocale = MakePtr<WfReferenceExpression>();
						refLocale->name.value = L"<ls>locale";

						auto refParameter = MakePtr<WfReferenceExpression>();
						refParameter->name.value = L"<ls>" + itow(index);

						auto refLoc2 = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<Localization>());

						auto refFD = MakePtr<WfChildExpression>();
						refFD->parent = refLoc2;
						refFD->name.value = L"Format" + function;

						auto callFD = MakePtr<WfCallExpression>();
						callFD->function = refFD;
						callFD->arguments.Add(refLocale);
						callFD->arguments.Add(refParameter);

						varDesc->expression = callFD;
					}
					else
					{
						auto refParameter = MakePtr<WfReferenceExpression>();
						refParameter->name.value = L"<ls>" + itow(index);

						varDesc->expression = refParameter;
					}
				}

				for (vint i = 0; i < textDesc->texts.Count(); i++)
				{
					if (textDesc->texts[i] != L"")
					{
						auto strExpr = MakePtr<WfStringExpression>();
						strExpr->value.value = textDesc->texts[i];
						appendExpr(strExpr);
					}

					if (i < textDesc->parameters.Count())
					{
						auto refExpr = MakePtr<WfReferenceExpression>();
						refExpr->name.value = L"<ls>_" + itow(i);
						appendExpr(refExpr);
					}
				}

				if (!resultExpr)
				{
					resultExpr = MakePtr<WfStringExpression>();
				}

				auto returnStat = MakePtr<WfReturnStatement>();
				returnStat->expression = resultExpr;

				block->statements.Add(returnStat);
			}

			return lsExpr;
		}

		Ptr<workflow::WfModule> GuiInstanceLocalizedStrings::Compile(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, GuiResourceError::List& errors)
		{
			vint errorCount = errors.Count();
			TextDescMap textDescs;
			Validate(textDescs, precompileContext, errors);
			if (errors.Count() != errorCount)
			{
				return nullptr;
			}

			auto module = MakePtr<WfModule>();
			module->name.value = moduleName;
			{
				auto lsInterface = Workflow_InstallClass(GetInterfaceTypeName(true), module);
				lsInterface->kind = WfClassKind::Interface;
				lsInterface->constructorType = WfConstructorType::SharedPtr;

				auto defaultStrings = GetDefaultStrings();
				FOREACH(WString, functionName, defaultStrings->items.Keys())
				{
					auto func = GenerateFunction(textDescs[{defaultStrings, functionName}], functionName, WfClassMemberKind::Normal);
					lsInterface->declarations.Add(func);
				}
			}
			auto lsClass = Workflow_InstallClass(className, module);
			{
				auto func = MakePtr<WfFunctionDeclaration>();
				lsClass->declarations.Add(func);
				func->anonymity = WfFunctionAnonymity::Named;
				func->name.value = L"<ls>First";
				func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<WString>::CreateTypeInfo().Obj());
				{
					auto argument = MakePtr<WfFunctionArgument>();
					argument->type = GetTypeFromTypeInfo(TypeInfoRetriver<LazyList<WString>>::CreateTypeInfo().Obj());
					argument->name.value = L"<ls>formats";
					func->arguments.Add(argument);
				}
				{
					auto member = MakePtr<WfClassMember>();
					member->kind = WfClassMemberKind::Static;
					func->classMember = member;
				}
				auto block = MakePtr<WfBlockStatement>();
				func->statement = block;

				{
					auto forStat = MakePtr<WfForEachStatement>();
					block->statements.Add(forStat);
					forStat->name.value = L"<ls>format";
					forStat->direction = WfForEachDirection::Normal;

					auto refArgument = MakePtr<WfReferenceExpression>();
					refArgument->name.value = L"<ls>formats";
					forStat->collection = refArgument;

					auto forBlock = MakePtr<WfBlockStatement>();
					forStat->statement = forBlock;
					{
						auto refFormat = MakePtr<WfReferenceExpression>();
						refFormat->name.value = L"<ls>format";

						auto returnStat = MakePtr<WfReturnStatement>();
						returnStat->expression = refFormat;
						forBlock->statements.Add(returnStat);
					}
				}
				{
					auto returnStat = MakePtr<WfReturnStatement>();
					returnStat->expression = MakePtr<WfStringExpression>();
					block->statements.Add(returnStat);
				}
			}
			{
				auto func = MakePtr<WfFunctionDeclaration>();
				lsClass->declarations.Add(func);

				func->anonymity = WfFunctionAnonymity::Named;
				func->name.value = L"Get";
				{
					auto refType = MakePtr<WfReferenceType>();
					refType->name.value = GetInterfaceTypeName(false);

					auto refPointer = MakePtr<WfSharedPointerType>();
					refPointer->element = refType;

					func->returnType = refPointer;
				}
				{
					auto member = MakePtr<WfClassMember>();
					member->kind = WfClassMemberKind::Static;
					func->classMember = member;
				}
				{
					auto argument = MakePtr<WfFunctionArgument>();
					argument->name.value = L"<ls>locale";
					argument->type = GetTypeFromTypeInfo(TypeInfoRetriver<Locale>::CreateTypeInfo().Obj());
					func->arguments.Add(argument);
				}

				auto block = MakePtr<WfBlockStatement>();
				func->statement = block;

				auto defaultStrings = GetDefaultStrings();
				FOREACH(Ptr<Strings>, ls, strings)
				{
					if (ls != defaultStrings)
					{
						auto listExpr = MakePtr<WfConstructorExpression>();
						FOREACH(WString, locale, ls->locales)
						{
							auto strExpr = MakePtr<WfStringExpression>();
							strExpr->value.value = locale;

							auto item = MakePtr<WfConstructorArgument>();
							item->key = strExpr;
							listExpr->arguments.Add(item);
						}

						auto refLocale = MakePtr<WfReferenceExpression>();
						refLocale->name.value = L"<ls>locale";

						auto inferExpr = MakePtr<WfInferExpression>();
						inferExpr->type = GetTypeFromTypeInfo(TypeInfoRetriver<WString>::CreateTypeInfo().Obj());
						inferExpr->expression = refLocale;

						auto inExpr = MakePtr<WfSetTestingExpression>();
						inExpr->test = WfSetTesting::In;
						inExpr->element = inferExpr;
						inExpr->collection = listExpr;

						auto ifStat = MakePtr<WfIfStatement>();
						block->statements.Add(ifStat);
						ifStat->expression = inExpr;

						auto trueBlock = MakePtr<WfBlockStatement>();
						ifStat->trueBranch = trueBlock;

						auto returnStat = MakePtr<WfReturnStatement>();
						returnStat->expression = GenerateStrings(textDescs, ls);
						trueBlock->statements.Add(returnStat);
					}
				}
				auto returnStat = MakePtr<WfReturnStatement>();
				returnStat->expression = GenerateStrings(textDescs, defaultStrings);
				block->statements.Add(returnStat);
			}

			ParsingTextPos pos(tagPosition.row, tagPosition.column);
			SetCodeRange(module, { pos,pos });
			return module;
		}
	}
}