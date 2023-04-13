#include "GuiInstanceLocalizedStrings.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace glr::xml;
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

		Ptr<GuiInstanceLocalizedStrings> GuiInstanceLocalizedStrings::LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlDocument> xml, GuiResourceError::List& errors)
		{
			auto ls = Ptr(new GuiInstanceLocalizedStrings);

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
			for (auto xmlStrings : XmlGetElements(xml->rootElement))
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
					auto lss = Ptr(new GuiInstanceLocalizedStrings::Strings);
					ls->strings.Add(lss);
					lss->tagPosition = { { resource },xmlStrings->name.codeRange.start };
					SplitBySemicolon(attLocales->value.value, lss->locales);

					for (auto locale : lss->locales)
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

					for (auto xmlString : XmlGetElements(xmlStrings))
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
								auto item = Ptr(new GuiInstanceLocalizedStrings::StringItem);
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

		Ptr<glr::xml::XmlElement> GuiInstanceLocalizedStrings::SaveToXml()
		{
			auto xml = Ptr(new XmlElement);
			xml->name.value = L"LocalizedStrings";
			{
				auto att = Ptr(new XmlAttribute);
				att->name.value = L"ref.Class";
				att->value.value = className;
				xml->attributes.Add(att);
			}
			{
				auto att = Ptr(new XmlAttribute);
				att->name.value = L"DefaultLocale";
				att->value.value = defaultLocale;
				xml->attributes.Add(att);
			}

			for (auto lss : strings)
			{
				auto xmlStrings = Ptr(new XmlElement);
				xml->subNodes.Add(xmlStrings);
				xmlStrings->name.value = L"Strings";
				{
					auto att = Ptr(new XmlAttribute);
					att->name.value = L"Strings";
					att->value.value = lss->GetLocalesName();
					xmlStrings->attributes.Add(att);
				}

				for (auto lssi : lss->items.Values())
				{
					auto xmlString = Ptr(new XmlElement);
					xmlStrings->subNodes.Add(xmlString);
					{
						auto att = Ptr(new XmlAttribute);
						att->name.value = L"Name";
						att->value.value = lssi->name;
						xmlString->attributes.Add(att);
					}
					{
						auto att = Ptr(new XmlAttribute);
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
			glr::ParsingTextPos formatPos(0, 0);
			auto textDesc = Ptr(new TextDesc);

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
					auto text = WString::CopyFrom(reading, vint(begin - reading));
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
							function = WString::CopyFrom(numberEnd + 1, (vint)(end - numberEnd - 1));
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
					textDesc->positions.Add(wtoi(WString::CopyFrom(number, (vint)(numberEnd - number))));
				}
				reading = end + 1;
			}

			if (*reading || textDesc->texts.Count() == 0)
			{
				textDesc->texts.Add(reading);
			}

			for (auto [i, index] : indexed(From(textDesc->positions).OrderBySelf()))
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
			for (auto lss : strings)
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

			for (auto lssi : defaultStrings->items.Values())
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
			for (auto lss : strings)
			{
				if (lss != defaultStrings)
				{
					auto localesName = lss->GetLocalesName();

					for (auto lssi : lss->items.Values())
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

		Ptr<workflow::WfFunctionDeclaration> GuiInstanceLocalizedStrings::GenerateFunction(Ptr<TextDesc> textDesc, const WString& functionName, workflow::WfFunctionKind functionKind)
		{
			auto func = Ptr(new WfFunctionDeclaration);
			func->functionKind = functionKind;
			func->anonymity = WfFunctionAnonymity::Named;
			func->name.value = functionName;
			func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<WString>::CreateTypeInfo().Obj());
			for (vint i = 0; i < textDesc->positions.Count(); i++)
			{
				auto type = textDesc->parameters[textDesc->positions[i]];

				auto argument = Ptr(new WfFunctionArgument);
				argument->name.value = L"<ls>" + itow(i);
				argument->type = GetTypeFromTypeInfo(type.key.Obj());
				func->arguments.Add(argument);
			}

			return func;
		}

		WString GuiInstanceLocalizedStrings::GenerateStringsCppName(Ptr<Strings> ls)
		{
			return From(ls->locales)
				.Aggregate(
					WString::Unmanaged(L"<ls>"),
					[](auto&& a, auto&& b)
					{
						return a + WString::Unmanaged(L"_") + b;
					});
		}

		Ptr<workflow::WfExpression> GuiInstanceLocalizedStrings::GenerateStrings(TextDescMap& textDescs, Ptr<Strings> ls)
		{
			auto lsExpr = Ptr(new WfNewInterfaceExpression);
			{
				auto refType = Ptr(new WfReferenceType);
				refType->name.value = GetInterfaceTypeName(false);

				auto refPointer = Ptr(new WfSharedPointerType);
				refPointer->element = refType;

				lsExpr->type = refPointer;
			}

			for (auto lss : ls->items.Values())
			{
				auto textDesc = textDescs[{ls, lss->name}];
				auto func = GenerateFunction(textDesc, lss->name, WfFunctionKind::Override);
				lsExpr->declarations.Add(func);

				auto block = Ptr(new WfBlockStatement);
				func->statement = block;

				Ptr<WfExpression> resultExpr;

				auto appendExpr = [&](Ptr<WfExpression> strExpr)
				{
					if (resultExpr)
					{
						auto binaryExpr = Ptr(new WfBinaryExpression);
						binaryExpr->op = WfBinaryOperator::FlagAnd;
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
					auto varDesc = Ptr(new WfVariableDeclaration);
					varDesc->name.value = L"<ls>_" + itow(i);

					auto varStat = Ptr(new WfVariableStatement);
					varStat->variable = varDesc;
					block->statements.Add(varStat);

					auto type = textDesc->parameters[i].key;
					auto function = textDesc->parameters[i].value;
					auto index = textDesc->positions[i];

					if (function == L"ShortDate" || function == L"LongDate" || function == L"YearMonthDate" || function == L"ShortTime" || function == L"LongTime")
					{
						auto refLoc = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<Localization>());

						auto refFormats = Ptr(new WfChildExpression);
						refFormats->parent = refLoc;
						refFormats->name.value = L"Get" + function + L"Formats";

						auto refLocale = Ptr(new WfReferenceExpression);
						refLocale->name.value = L"<ls>locale";

						auto callFormats = Ptr(new WfCallExpression);
						callFormats->function = refFormats;
						callFormats->arguments.Add(refLocale);

						auto refFirst = Ptr(new WfChildExpression);
						refFirst->parent = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<helper_types::LocalizedStrings>());
						refFirst->name.value = L"FirstOrEmpty";

						auto callFirst = Ptr(new WfCallExpression);
						{
							callFirst->function = refFirst;
							callFirst->arguments.Add(callFormats);
						}

						auto refLocale2 = Ptr(new WfReferenceExpression);
						refLocale2->name.value = L"<ls>locale";

						auto refParameter = Ptr(new WfReferenceExpression);
						refParameter->name.value = L"<ls>" + itow(index);

						auto refLoc2 = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<Localization>());

						auto refFD = Ptr(new WfChildExpression);
						refFD->parent = refLoc2;
						refFD->name.value = L"Format" + function.Right(4);

						auto callFD = Ptr(new WfCallExpression);
						callFD->function = refFD;
						callFD->arguments.Add(refLocale2);
						callFD->arguments.Add(callFirst);
						callFD->arguments.Add(refParameter);

						varDesc->expression = callFD;
					}
					else if (function.Length() >= 5 && (function.Left(5) == L"Date:" || function.Left(5) == L"Time:"))
					{
						auto refLocale = Ptr(new WfReferenceExpression);
						refLocale->name.value = L"<ls>locale";

						auto refFormat = Ptr(new WfStringExpression);
						refFormat->value.value = function.Right(function.Length() - 5);

						auto refParameter = Ptr(new WfReferenceExpression);
						refParameter->name.value = L"<ls>" + itow(index);

						auto refLoc2 = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<Localization>());

						auto refFD = Ptr(new WfChildExpression);
						refFD->parent = refLoc2;
						refFD->name.value = L"Format" + function.Left(4);

						auto callFD = Ptr(new WfCallExpression);
						callFD->function = refFD;
						callFD->arguments.Add(refLocale);
						callFD->arguments.Add(refFormat);
						callFD->arguments.Add(refParameter);

						varDesc->expression = callFD;
					}
					else if (function == L"Number" || function == L"Currency")
					{
						auto refLocale = Ptr(new WfReferenceExpression);
						refLocale->name.value = L"<ls>locale";

						auto refParameter = Ptr(new WfReferenceExpression);
						refParameter->name.value = L"<ls>" + itow(index);

						auto refLoc2 = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<Localization>());

						auto refFD = Ptr(new WfChildExpression);
						refFD->parent = refLoc2;
						refFD->name.value = L"Format" + function;

						auto callFD = Ptr(new WfCallExpression);
						callFD->function = refFD;
						callFD->arguments.Add(refLocale);
						callFD->arguments.Add(refParameter);

						varDesc->expression = callFD;
					}
					else
					{
						auto refParameter = Ptr(new WfReferenceExpression);
						refParameter->name.value = L"<ls>" + itow(index);

						varDesc->expression = refParameter;
					}
				}

				for (vint i = 0; i < textDesc->texts.Count(); i++)
				{
					if (textDesc->texts[i] != L"")
					{
						auto strExpr = Ptr(new WfStringExpression);
						strExpr->value.value = textDesc->texts[i];
						appendExpr(strExpr);
					}

					if (i < textDesc->parameters.Count())
					{
						auto refExpr = Ptr(new WfReferenceExpression);
						refExpr->name.value = L"<ls>_" + itow(i);
						appendExpr(refExpr);
					}
				}

				if (!resultExpr)
				{
					resultExpr = Ptr(new WfStringExpression);
				}

				auto returnStat = Ptr(new WfReturnStatement);
				returnStat->expression = resultExpr;

				block->statements.Add(returnStat);
			}

			return lsExpr;
		}

		Ptr<workflow::WfFunctionDeclaration> GuiInstanceLocalizedStrings::GenerateStringsFunction(const WString& name, TextDescMap& textDescs, Ptr<Strings> ls)
		{
			auto func = Ptr(new WfFunctionDeclaration);
			func->functionKind = WfFunctionKind::Static;
			func->anonymity = WfFunctionAnonymity::Named;
			func->name.value = name;
			{
				auto refType = Ptr(new WfReferenceType);
				refType->name.value = GetInterfaceTypeName(false);

				auto refPointer = Ptr(new WfSharedPointerType);
				refPointer->element = refType;

				func->returnType = refPointer;
			}
			{
				auto argument = Ptr(new WfFunctionArgument);
				argument->name.value = L"<ls>locale";
				argument->type = GetTypeFromTypeInfo(TypeInfoRetriver<Locale>::CreateTypeInfo().Obj());
				func->arguments.Add(argument);
			}

			auto block = Ptr(new WfBlockStatement);
			func->statement = block;
			
			auto returnStat = Ptr(new WfReturnStatement);
			returnStat->expression = GenerateStrings(textDescs, ls);
			block->statements.Add(returnStat);

			return func;
		}

		Ptr<workflow::WfFunctionDeclaration> GuiInstanceLocalizedStrings::GenerateGetFunction(TextDescMap& textDescs)
		{
			auto func = Ptr(new WfFunctionDeclaration);
			func->functionKind = WfFunctionKind::Static;
			func->anonymity = WfFunctionAnonymity::Named;
			func->name.value = L"Get";
			{
				auto refType = Ptr(new WfReferenceType);
				refType->name.value = GetInterfaceTypeName(false);

				auto refPointer = Ptr(new WfSharedPointerType);
				refPointer->element = refType;

				func->returnType = refPointer;
			}
			{
				auto argument = Ptr(new WfFunctionArgument);
				argument->name.value = L"<ls>locale";
				argument->type = GetTypeFromTypeInfo(TypeInfoRetriver<Locale>::CreateTypeInfo().Obj());
				func->arguments.Add(argument);
			}

			auto block = Ptr(new WfBlockStatement);
			func->statement = block;

			auto defaultStrings = GetDefaultStrings();
			for (auto ls : strings)
			{
				if (ls != defaultStrings)
				{
					auto refLocale = Ptr(new WfReferenceExpression);
					refLocale->name.value = L"<ls>locale";

					auto inferExpr = Ptr(new WfInferExpression);
					inferExpr->type = GetTypeFromTypeInfo(TypeInfoRetriver<WString>::CreateTypeInfo().Obj());
					inferExpr->expression = refLocale;

					auto ifStat = Ptr(new WfIfStatement);
					block->statements.Add(ifStat);

					if (ls->locales.Count() == 1)
					{
						auto strExpr = Ptr(new WfStringExpression);
						strExpr->value.value = ls->locales[0];

						auto eqExpr = Ptr(new WfBinaryExpression);
						eqExpr->op = WfBinaryOperator::EQ;
						eqExpr->first = inferExpr;
						eqExpr->second = strExpr;

						ifStat->expression = eqExpr;
					}
					else
					{
						auto listExpr = Ptr(new WfConstructorExpression);
						for (auto locale : ls->locales)
						{
							auto strExpr = Ptr(new WfStringExpression);
							strExpr->value.value = locale;

							auto item = Ptr(new WfConstructorArgument);
							item->key = strExpr;
							listExpr->arguments.Add(item);
						}

						auto inExpr = Ptr(new WfSetTestingExpression);
						inExpr->test = WfSetTesting::In;
						inExpr->element = inferExpr;
						inExpr->collection = listExpr;

						ifStat->expression = inExpr;
					}

					auto trueBlock = Ptr(new WfBlockStatement);
					ifStat->trueBranch = trueBlock;

					auto refStrings = Ptr(new WfReferenceExpression);
					refStrings->name.value = GenerateStringsCppName(ls);

					auto refLocale2 = Ptr(new WfReferenceExpression);
					refLocale2->name.value = L"<ls>locale";

					auto callExpr = Ptr(new WfCallExpression);
					callExpr->function = refStrings;
					callExpr->arguments.Add(refLocale2);

					auto returnStat = Ptr(new WfReturnStatement);
					returnStat->expression = callExpr;
					trueBlock->statements.Add(returnStat);
				}
			}
			auto returnStat = Ptr(new WfReturnStatement);
			{
				auto refStrings = Ptr(new WfReferenceExpression);
				refStrings->name.value = GenerateStringsCppName(defaultStrings);

				auto refDefaultLocale = Ptr(new WfStringExpression);
				refDefaultLocale->value.value = defaultLocale;

				auto inferExpr = Ptr(new WfInferExpression);
				inferExpr->type = GetTypeFromTypeInfo(TypeInfoRetriver<Locale>::CreateTypeInfo().Obj());
				inferExpr->expression = refDefaultLocale;

				auto callExpr = Ptr(new WfCallExpression);
				callExpr->function = refStrings;
				callExpr->arguments.Add(inferExpr);

				returnStat->expression = callExpr;
			}
			block->statements.Add(returnStat);

			return func;
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

			auto module = Ptr(new WfModule);
			module->moduleType = WfModuleType::Module;
			module->name.value = moduleName;
			{
				auto lsInterface = Workflow_InstallClass(GetInterfaceTypeName(true), module);
				lsInterface->kind = WfClassKind::Interface;
				lsInterface->constructorType = WfConstructorType::SharedPtr;

				auto defaultStrings = GetDefaultStrings();
				for (auto functionName : defaultStrings->items.Keys())
				{
					auto func = GenerateFunction(textDescs[{defaultStrings, functionName}], functionName, WfFunctionKind::Normal);
					lsInterface->declarations.Add(func);
				}
			}
			auto lsClass = Workflow_InstallClass(className, module);
			for (auto ls : strings)
			{
				auto cppName = GenerateStringsCppName(ls);
				lsClass->declarations.Add(GenerateStringsFunction(cppName, textDescs, ls));
			}
			lsClass->declarations.Add(GenerateGetFunction(textDescs));

			glr::ParsingTextPos pos(tagPosition.row, tagPosition.column);
			SetCodeRange(module, { pos,pos });
			return module;
		}
	}
}