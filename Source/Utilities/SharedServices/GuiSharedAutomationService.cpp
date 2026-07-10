#include "GuiSharedAutomationService.h"

namespace vl
{
	namespace presentation
	{
		using namespace remoteprotocol;

/***********************************************************************
DumpRemoteProtocolRenderingDom
***********************************************************************/

		Ptr<glr::json::JsonNode> DumpRemoteProtocolRenderingDom(
			const WString& title,
			const remoteprotocol::WindowSizingConfig& windowSizingConfig,
			Ptr<remoteprotocol::RenderingDom> renderingDom,
			collections::Dictionary<vint, collections::Pair<remoteprotocol::RendererType, Nullable<remoteprotocol::UnitTest_ElementDescVariant>>>& elementData
		)
		{
			auto dumpRoot = Ptr(new glr::json::JsonObject);
			ConvertCustomTypeToJsonField(dumpRoot, L"Title", title);
			ConvertCustomTypeToJsonField(dumpRoot, L"Window", windowSizingConfig);

			{
				auto field = Ptr(new glr::json::JsonObjectField);
				field->name.value = WString::Unmanaged(L"Elements");

				auto jsonArray = Ptr(new glr::json::JsonArray);
				field->value = jsonArray;
				dumpRoot->fields.Add(field);

				for (auto [id, data] : elementData)
				{
					auto jsonElement = Ptr(new glr::json::JsonObject);
					jsonArray->items.Add(jsonElement);

					ConvertCustomTypeToJsonField<vint>(jsonElement, L"Id", id);
					ConvertCustomTypeToJsonField(jsonElement, L"Type", data.key);
					ConvertCustomTypeToJsonField(jsonElement, L"Data", data.value);
				}
			}

			if (renderingDom)
			{
				ConvertCustomTypeToJsonField(dumpRoot, L"Dom", renderingDom);
			}
			return dumpRoot;
		}

		WString DumpJsonToString(Ptr<glr::json::JsonNode> json)
		{
			return stream::GenerateToStream([=](stream::TextWriter& writer)
			{
				glr::json::JsonFormatting formatting;
				formatting.spaceAfterColon = true;
				formatting.spaceAfterComma = true;
				formatting.crlf = true;
				formatting.compact = true;
				formatting.indentation = L"  ";
				return glr::json::JsonPrint(json, writer, formatting);
			});
		}

/***********************************************************************
AutomationServiceRenderer
***********************************************************************/

		Nullable<WString> AutomationServiceRenderer::CopyFatalError()
		{
			Nullable<WString> copiedFatalError;
			SPIN_LOCK(lockFatalError)
			{
				copiedFatalError = fatalError;
			}
			return copiedFatalError;
		}

		void AutomationServiceRenderer::SetFatalError(Nullable<WString> value)
		{
			SPIN_LOCK(lockFatalError)
			{
				fatalError = value;
			}
		}

		WString AutomationServiceRenderer::DumpDomTreeInternal()
		{
			auto copiedFatalError = CopyFatalError();
			auto dumpRoot = DumpRemoteProtocolRenderingDom(
				GetCurrentController()->WindowService()->GetMainWindow()->GetTitle(),
				renderer->windowSizingConfig,
				renderer->renderingDom,
				renderer->renderingElements);
			if (copiedFatalError)
			{
				ConvertCustomTypeToJsonField(dumpRoot.Cast<glr::json::JsonObject>(), L"fatalError", copiedFatalError.Value());
			}
			return DumpJsonToString(dumpRoot);
		}

		INativeAutomationService::IOCommandAvailability AutomationServiceRenderer::CanRunIOCommands()
		{
			return CopyFatalError() ? INativeAutomationService::IOCommandAvailability::ExitOnly : INativeAutomationService::IOCommandAvailability::Enabled;
		}

/***********************************************************************
RunIOCommandOnNativeWindow
***********************************************************************/

		namespace iocommands
		{
			const wchar_t* IO_COMMAND_SYNTAX =
				L"Syntax Error!\r\n"
				L"Predefined Commands:\r\n"
				L"!Type:<TEXT>\r\n"
				L"!Exit\r\n"
				L"!KeyDown:Key1+Key2+...+KeyN\r\n"
				L"!KeyUp:Key1+Key2+...+KeyN\r\n"
				L"!KeyPress:Key1+Key2+...+KeyN\r\n"
				L"!MouseMove:X,Y(,ctrl)?(,shift)?(,alt)?\r\n"
				L"!(Left|Middle|Right)(Down|Up|Click|DbClick):X,Y(,ctrl)?(,shift)?(,alt)?\r\n"
				L"!MouseWheel(Up|Down|Left|Right):ticks(,ctrl)?(,shift)?(,alt)?";

			struct IOCommandModifiers
			{
				bool ctrl = false;
				bool shift = false;
				bool alt = false;
			};

			struct TemporaryModifiers
			{
				bool ctrl = false;
				bool shift = false;
				bool alt = false;
			};

			struct MouseCommandArguments
			{
				Point position;
				IOCommandModifiers modifiers;
			};

			struct WheelCommandArguments
			{
				vint ticks = 0;
				IOCommandModifiers modifiers;
			};

			enum class MouseButton
			{
				Left,
				Middle,
				Right,
			};

			struct SyntaxErrorCommand
			{
			};

			struct ExitCommand
			{
			};

			struct TypeCommand
			{
				WString text;
			};

			enum class KeyOperation
			{
				Down,
				Up,
				Press,
			};

			struct KeyCommand
			{
				KeyOperation operation = KeyOperation::Press;
				collections::List<VKEY> keys;
			};

			struct MouseMoveCommand
			{
				MouseCommandArguments arguments;
			};

			struct MouseButtonCommand
			{
				MouseButton button = MouseButton::Left;
				WString operation;
				MouseCommandArguments arguments;
			};

			struct WheelCommand
			{
				vint direction = 0;
				bool horizontal = false;
				WheelCommandArguments arguments;
			};

			using IOCommand = Variant<
				SyntaxErrorCommand,
				ExitCommand,
				TypeCommand,
				KeyCommand,
				MouseMoveCommand,
				MouseButtonCommand,
				WheelCommand
				>;

			struct IOCommandHolder : Object
			{
				IOCommand command;

				IOCommandHolder(IOCommand&& _command)
					: command(std::move(_command))
				{
				}
			};

			bool StartsWith(const WString& text, const WString& prefix)
			{
				return text.Length() >= prefix.Length() && text.Left(prefix.Length()) == prefix;
			}

			WString Trim(const WString& text)
			{
				vint begin = 0;
				vint end = text.Length();
				while (begin < end && (text[begin] == L' ' || text[begin] == L'\t')) begin++;
				while (begin < end && (text[end - 1] == L' ' || text[end - 1] == L'\t')) end--;
				return text.Sub(begin, end - begin);
			}

			WString ToUpperToken(const WString& text)
			{
				WString result;
				for (vint i = 0; i < text.Length(); i++)
				{
					auto ch = text[i];
					if (L'a' <= ch && ch <= L'z') ch = ch - L'a' + L'A';
					result += WString::FromChar(ch);
				}
				return result;
			}

			WString NormalizeKeyName(const WString& text)
			{
				WString result;
				for (vint i = 0; i < text.Length(); i++)
				{
					auto ch = text[i];
					if (ch == L' ' || ch == L'\t') continue;
					if (L'a' <= ch && ch <= L'z') ch = ch - L'a' + L'A';
					result += WString::FromChar(ch);
				}
				return result;
			}

			void SplitByChar(const WString& text, wchar_t delimiter, collections::List<WString>& fragments)
			{
				vint begin = 0;
				for (vint i = 0; i <= text.Length(); i++)
				{
					if (i == text.Length() || text[i] == delimiter)
					{
						fragments.Add(Trim(text.Sub(begin, i - begin)));
						begin = i + 1;
					}
				}
			}

			bool TryParseInteger(const WString& text, vint& value)
			{
				auto normalized = Trim(text);
				if (normalized.Length() == 0) return false;
				value = wtoi(normalized);
				return itow(value) == normalized;
			}

			bool TryParseModifier(const WString& text, IOCommandModifiers& modifiers)
			{
				auto token = ToUpperToken(Trim(text));
				if (token == L"CTRL" || token == L"CONTROL")
				{
					modifiers.ctrl = true;
					return true;
				}
				else if (token == L"SHIFT")
				{
					modifiers.shift = true;
					return true;
				}
				else if (token == L"ALT" || token == L"MENU")
				{
					modifiers.alt = true;
					return true;
				}
				return false;
			}

			bool TryParseMouseArguments(const WString& text, MouseCommandArguments& arguments)
			{
				collections::List<WString> fragments;
				SplitByChar(text, L',', fragments);
				if (fragments.Count() < 2) return false;

				vint x = 0;
				vint y = 0;
				if (!TryParseInteger(fragments[0], x)) return false;
				if (!TryParseInteger(fragments[1], y)) return false;
				arguments.position = Point(x, y);

				for (vint i = 2; i < fragments.Count(); i++)
				{
					if (!TryParseModifier(fragments[i], arguments.modifiers)) return false;
				}
				return true;
			}

			bool TryParseWheelArguments(const WString& text, WheelCommandArguments& arguments)
			{
				collections::List<WString> fragments;
				SplitByChar(text, L',', fragments);
				if (fragments.Count() < 1) return false;
				if (!TryParseInteger(fragments[0], arguments.ticks)) return false;
				if (arguments.ticks < 0) return false;

				for (vint i = 1; i < fragments.Count(); i++)
				{
					if (!TryParseModifier(fragments[i], arguments.modifiers)) return false;
				}
				return true;
			}

			bool IsPressing(IoCommandState* state, VKEY key)
			{
				return state->pressingKeys.Contains(key);
			}

			bool IsCtrlPressing(IoCommandState* state)
			{
				return IsPressing(state, VKEY::KEY_CONTROL) || IsPressing(state, VKEY::KEY_LCONTROL) || IsPressing(state, VKEY::KEY_RCONTROL);
			}

			bool IsShiftPressing(IoCommandState* state)
			{
				return IsPressing(state, VKEY::KEY_SHIFT) || IsPressing(state, VKEY::KEY_LSHIFT) || IsPressing(state, VKEY::KEY_RSHIFT);
			}

			bool IsAltPressing(IoCommandState* state)
			{
				return IsPressing(state, VKEY::KEY_MENU) || IsPressing(state, VKEY::KEY_LMENU) || IsPressing(state, VKEY::KEY_RMENU);
			}

			NativeWindowMouseInfo MakeMouseInfo(IoCommandState* state)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::RunIOCommandOnNativeWindow(...)#"
				CHECK_ERROR(state->mousePosition, ERROR_MESSAGE_PREFIX L"The mouse position is not set.");
				NativeWindowMouseInfo info;
				info.ctrl = IsCtrlPressing(state);
				info.shift = IsShiftPressing(state);
				info.left = state->leftPressing;
				info.middle = state->middlePressing;
				info.right = state->rightPressing;
				info.x = state->mousePosition.Value().x;
				info.y = state->mousePosition.Value().y;
				info.wheel = 0;
				info.nonClient = false;
				return info;
#undef ERROR_MESSAGE_PREFIX
			}

			NativeWindowKeyInfo MakeKeyInfo(IoCommandState* state, VKEY key, bool autoRepeatKeyDown = false)
			{
				NativeWindowKeyInfo info;
				info.code = key;
				info.ctrl = IsCtrlPressing(state);
				info.shift = IsShiftPressing(state);
				info.alt = IsAltPressing(state);
				info.capslock = state->capslockToggled;
				info.autoRepeatKeyDown = autoRepeatKeyDown;
				return info;
			}

			NativeWindowCharInfo MakeCharInfo(IoCommandState* state, wchar_t ch)
			{
				NativeWindowCharInfo info;
				info.code = ch;
				info.ctrl = IsCtrlPressing(state);
				info.shift = IsShiftPressing(state);
				info.alt = IsAltPressing(state);
				info.capslock = state->capslockToggled;
				return info;
			}

			void KeyDown(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, VKEY key)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::RunIOCommandOnNativeWindow(...)#"
				CHECK_ERROR(!IsPressing(state, key), ERROR_MESSAGE_PREFIX L"The key is already being pressed.");
				state->pressingKeys.Add(key);
				if (key == VKEY::KEY_CAPITAL)
				{
					state->capslockToggled = !state->capslockToggled;
				}
				auto info = MakeKeyInfo(state, key);
				for (auto listener : listeners)
				{
					listener->KeyDown(info);
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void KeyUp(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, VKEY key)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::RunIOCommandOnNativeWindow(...)#"
				CHECK_ERROR(IsPressing(state, key), ERROR_MESSAGE_PREFIX L"The key is not being pressed.");
				state->pressingKeys.Remove(key);
				auto info = MakeKeyInfo(state, key);
				for (auto listener : listeners)
				{
					listener->KeyUp(info);
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void Char(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, wchar_t ch)
			{
				auto info = MakeCharInfo(state, ch);
				for (auto listener : listeners)
				{
					listener->Char(info);
				}
			}

			void PressTemporaryModifiers(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, const IOCommandModifiers& modifiers, TemporaryModifiers& temporary)
			{
				if (modifiers.ctrl && !IsCtrlPressing(state))
				{
					KeyDown(state, listeners, VKEY::KEY_CONTROL);
					temporary.ctrl = true;
				}
				if (modifiers.shift && !IsShiftPressing(state))
				{
					KeyDown(state, listeners, VKEY::KEY_SHIFT);
					temporary.shift = true;
				}
				if (modifiers.alt && !IsAltPressing(state))
				{
					KeyDown(state, listeners, VKEY::KEY_MENU);
					temporary.alt = true;
				}
			}

			void ReleaseTemporaryModifiers(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, const TemporaryModifiers& temporary)
			{
				if (temporary.alt)
				{
					KeyUp(state, listeners, VKEY::KEY_MENU);
				}
				if (temporary.shift)
				{
					KeyUp(state, listeners, VKEY::KEY_SHIFT);
				}
				if (temporary.ctrl)
				{
					KeyUp(state, listeners, VKEY::KEY_CONTROL);
				}
			}

			NativePoint ConvertGuiPointToNativePoint(INativeWindow* targetWindow, Point position)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::RunIOCommandOnNativeWindow(...)#"
				CHECK_ERROR(targetWindow, ERROR_MESSAGE_PREFIX L"Native target window is missing.");
				return targetWindow->Convert(position);
#undef ERROR_MESSAGE_PREFIX
			}

			void MouseMove(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, NativePoint position)
			{
				if (!state->mousePosition)
				{
					for (auto listener : listeners)
					{
						listener->MouseEntered();
					}
				}
				else if (state->mousePosition.Value() == position)
				{
					return;
				}

				state->mousePosition = position;
				auto info = MakeMouseInfo(state);
				for (auto listener : listeners)
				{
					listener->MouseMoving(info);
				}
			}

			void ButtonDown(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, MouseButton button, NativePoint position)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::RunIOCommandOnNativeWindow(...)#"
				MouseMove(state, listeners, position);
				switch (button)
				{
				case MouseButton::Left:
					CHECK_ERROR(!state->leftPressing, ERROR_MESSAGE_PREFIX L"The left button should not be being pressed.");
					state->leftPressing = true;
					for (auto listener : listeners) listener->LeftButtonDown(MakeMouseInfo(state));
					break;
				case MouseButton::Middle:
					CHECK_ERROR(!state->middlePressing, ERROR_MESSAGE_PREFIX L"The middle button should not be being pressed.");
					state->middlePressing = true;
					for (auto listener : listeners) listener->MiddleButtonDown(MakeMouseInfo(state));
					break;
				case MouseButton::Right:
					CHECK_ERROR(!state->rightPressing, ERROR_MESSAGE_PREFIX L"The right button should not be being pressed.");
					state->rightPressing = true;
					for (auto listener : listeners) listener->RightButtonDown(MakeMouseInfo(state));
					break;
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void ButtonUp(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, MouseButton button, NativePoint position)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::RunIOCommandOnNativeWindow(...)#"
				MouseMove(state, listeners, position);
				switch (button)
				{
				case MouseButton::Left:
					CHECK_ERROR(state->leftPressing, ERROR_MESSAGE_PREFIX L"The left button should be being pressed.");
					state->leftPressing = false;
					for (auto listener : listeners) listener->LeftButtonUp(MakeMouseInfo(state));
					break;
				case MouseButton::Middle:
					CHECK_ERROR(state->middlePressing, ERROR_MESSAGE_PREFIX L"The middle button should be being pressed.");
					state->middlePressing = false;
					for (auto listener : listeners) listener->MiddleButtonUp(MakeMouseInfo(state));
					break;
				case MouseButton::Right:
					CHECK_ERROR(state->rightPressing, ERROR_MESSAGE_PREFIX L"The right button should be being pressed.");
					state->rightPressing = false;
					for (auto listener : listeners) listener->RightButtonUp(MakeMouseInfo(state));
					break;
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void ButtonDoubleClick(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, MouseButton button, NativePoint position)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::RunIOCommandOnNativeWindow(...)#"
				MouseMove(state, listeners, position);
				switch (button)
				{
				case MouseButton::Left:
					CHECK_ERROR(!state->leftPressing, ERROR_MESSAGE_PREFIX L"The left button should not be being pressed.");
					state->leftPressing = true;
					for (auto listener : listeners) listener->LeftButtonDoubleClick(MakeMouseInfo(state));
					break;
				case MouseButton::Middle:
					CHECK_ERROR(!state->middlePressing, ERROR_MESSAGE_PREFIX L"The middle button should not be being pressed.");
					state->middlePressing = true;
					for (auto listener : listeners) listener->MiddleButtonDoubleClick(MakeMouseInfo(state));
					break;
				case MouseButton::Right:
					CHECK_ERROR(!state->rightPressing, ERROR_MESSAGE_PREFIX L"The right button should not be being pressed.");
					state->rightPressing = true;
					for (auto listener : listeners) listener->RightButtonDoubleClick(MakeMouseInfo(state));
					break;
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void MouseButtonOperation(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, MouseButton button, const WString& operation, NativePoint position)
			{
				if (operation == L"Down")
				{
					ButtonDown(state, listeners, button, position);
				}
				else if (operation == L"Up")
				{
					ButtonUp(state, listeners, button, position);
				}
				else if (operation == L"Click")
				{
					ButtonDown(state, listeners, button, position);
					ButtonUp(state, listeners, button, position);
				}
				else if (operation == L"DbClick")
				{
					ButtonDown(state, listeners, button, position);
					ButtonUp(state, listeners, button, position);
					ButtonDoubleClick(state, listeners, button, position);
					ButtonUp(state, listeners, button, position);
				}
			}

			void Wheel(IoCommandState* state, collections::List<INativeWindowListener*>& listeners, vint wheel, bool horizontal)
			{
				auto info = MakeMouseInfo(state);
				info.wheel = wheel;
				for (auto listener : listeners)
				{
					if (horizontal)
					{
						listener->HorizontalWheel(info);
					}
					else
					{
						listener->VerticalWheel(info);
					}
				}
			}

			bool TryParseKey(INativeController* nativeController, const WString& text, VKEY& key)
			{
				auto token = Trim(text);
				if (token.Length() == 0) return false;
				if (!nativeController) return false;

				auto inputService = nativeController->InputService();
				key = inputService->GetKey(token);
				if (key != VKEY::KEY_UNKNOWN) return true;

				auto normalizedToken = NormalizeKeyName(token);
				for (vint i = 0; i <= (vint)VKEY::KEY_MAXIMUM; i++)
				{
					auto candidate = (VKEY)i;
					auto candidateName = inputService->GetKeyName(candidate);
					if (candidateName != WString::Empty && candidateName != L"?" && NormalizeKeyName(candidateName) == normalizedToken)
					{
						key = candidate;
						return true;
					}
				}
				key = VKEY::KEY_UNKNOWN;
				return false;
			}

			bool TryParseKeyList(INativeController* nativeController, const WString& text, collections::List<VKEY>& keys)
			{
				collections::List<WString> fragments;
				SplitByChar(text, L'+', fragments);
				if (fragments.Count() == 0) return false;
				for (auto fragment : fragments)
				{
					VKEY key = VKEY::KEY_UNKNOWN;
					if (!TryParseKey(nativeController, fragment, key)) return false;
					keys.Add(key);
				}
				return true;
			}

			WString MouseButtonPrefix(MouseButton button)
			{
				switch (button)
				{
				case MouseButton::Left: return WString::Unmanaged(L"!Left");
				case MouseButton::Middle: return WString::Unmanaged(L"!Middle");
				default: return WString::Unmanaged(L"!Right");
				}
			}

			bool TryParseMouseButtonCommand(const WString& command, MouseButton button, const WString& operation, MouseButtonCommand& mouseCommand, bool& matched)
			{
				auto prefix = MouseButtonPrefix(button) + operation + WString::Unmanaged(L":");
				if (!StartsWith(command, prefix)) return false;
				matched = true;

				MouseCommandArguments arguments;
				if (!TryParseMouseArguments(command.Right(command.Length() - prefix.Length()), arguments)) return false;

				mouseCommand.button = button;
				mouseCommand.operation = operation;
				mouseCommand.arguments = arguments;
				return true;
			}

			bool TryParseWheelCommand(const WString& command, const WString& prefix, vint direction, bool horizontal, WheelCommand& wheelCommand, bool& matched)
			{
				if (!StartsWith(command, prefix)) return false;
				matched = true;

				WheelCommandArguments arguments;
				if (!TryParseWheelArguments(command.Right(command.Length() - prefix.Length()), arguments)) return false;

				wheelCommand.direction = direction;
				wheelCommand.horizontal = horizontal;
				wheelCommand.arguments = arguments;
				return true;
			}

			IOCommand ParseIOCommand(INativeController* nativeController, const WString& command)
			{
				if (command == L"!Exit")
				{
					return ExitCommand{};
				}

				if (StartsWith(command, L"!Type:"))
				{
					TypeCommand typeCommand;
					typeCommand.text = command.Right(command.Length() - 6);
					return typeCommand;
				}

				if (StartsWith(command, L"!KeyDown:") || StartsWith(command, L"!KeyUp:") || StartsWith(command, L"!KeyPress:"))
				{
					WString prefix;
					KeyOperation operation = KeyOperation::Press;
					if (StartsWith(command, L"!KeyDown:"))
					{
						prefix = WString::Unmanaged(L"!KeyDown:");
						operation = KeyOperation::Down;
					}
					else if (StartsWith(command, L"!KeyUp:"))
					{
						prefix = WString::Unmanaged(L"!KeyUp:");
						operation = KeyOperation::Up;
					}
					else
					{
						prefix = WString::Unmanaged(L"!KeyPress:");
					}

					collections::List<VKEY> keys;
					if (!TryParseKeyList(nativeController, command.Right(command.Length() - prefix.Length()), keys))
					{
						return SyntaxErrorCommand{};
					}

					KeyCommand keyCommand;
					keyCommand.operation = operation;
					CopyFrom(keyCommand.keys, keys);
					return keyCommand;
				}

				if (StartsWith(command, L"!MouseMove:"))
				{
					MouseMoveCommand mouseCommand;
					if (!TryParseMouseArguments(command.Right(command.Length() - 11), mouseCommand.arguments))
					{
						return SyntaxErrorCommand{};
					}
					return mouseCommand;
				}

				const WString operations[] =
				{
					WString::Unmanaged(L"Down"),
					WString::Unmanaged(L"Up"),
					WString::Unmanaged(L"Click"),
					WString::Unmanaged(L"DbClick"),
				};

				for (auto operation : operations)
				{
					MouseButtonCommand mouseCommand;
					bool matched = false;
					if (TryParseMouseButtonCommand(command, MouseButton::Left, operation, mouseCommand, matched)) return mouseCommand;
					if (matched) return SyntaxErrorCommand{};

					if (TryParseMouseButtonCommand(command, MouseButton::Middle, operation, mouseCommand, matched)) return mouseCommand;
					if (matched) return SyntaxErrorCommand{};

					if (TryParseMouseButtonCommand(command, MouseButton::Right, operation, mouseCommand, matched)) return mouseCommand;
					if (matched) return SyntaxErrorCommand{};
				}

				{
					WheelCommand wheelCommand;
					bool matched = false;
					if (TryParseWheelCommand(command, L"!MouseWheelUp:", 1, false, wheelCommand, matched)) return wheelCommand;
					if (matched) return SyntaxErrorCommand{};
					if (TryParseWheelCommand(command, L"!MouseWheelDown:", -1, false, wheelCommand, matched)) return wheelCommand;
					if (matched) return SyntaxErrorCommand{};
					if (TryParseWheelCommand(command, L"!MouseWheelRight:", 1, true, wheelCommand, matched)) return wheelCommand;
					if (matched) return SyntaxErrorCommand{};
					if (TryParseWheelCommand(command, L"!MouseWheelLeft:", -1, true, wheelCommand, matched)) return wheelCommand;
					if (matched) return SyntaxErrorCommand{};
				}

				return SyntaxErrorCommand{};
			}

			void ExecuteIOCommand(IoCommandState* state, INativeController* nativeController, INativeWindow* targetWindow, collections::List<INativeWindowListener*>& listeners, IOCommand&& ioCommand)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::RunIOCommandOnNativeWindow(...)#"
				CHECK_ERROR(nativeController, ERROR_MESSAGE_PREFIX L"Native controller is missing.");
				CHECK_ERROR(targetWindow, ERROR_MESSAGE_PREFIX L"Native target window is missing.");
				auto listenerList = &listeners;
				auto commandHolder = Ptr(new IOCommandHolder(std::move(ioCommand)));
				nativeController->AsyncService()->InvokeInMainThread(targetWindow, [=]() mutable
				{
					auto activateTargetWindow = [=]()
					{
						if (!targetWindow->IsActivated())
						{
							targetWindow->SetActivate();
						}
					};

					commandHolder->command.Apply(Overloading(
						[](const SyntaxErrorCommand&)
						{
							CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Syntax error command should not be executed.");
						},
						[=](const ExitCommand&)
						{
							targetWindow->Hide(true);
						},
						[=](const TypeCommand& typeCommand)
						{
							activateTargetWindow();
							for (vint i = 0; i < typeCommand.text.Length(); i++)
							{
								auto ch = typeCommand.text[i];
								if (ch == L'\r') continue;
								if (ch == L'\n')
								{
									Char(state, *listenerList, L'\r');
									Char(state, *listenerList, L'\n');
								}
								else
								{
									Char(state, *listenerList, ch);
								}
							}
						},
						[=](const KeyCommand& keyCommand)
						{
							activateTargetWindow();
							switch (keyCommand.operation)
							{
							case KeyOperation::Down:
								for (auto key : keyCommand.keys)
								{
									KeyDown(state, *listenerList, key);
								}
								break;
							case KeyOperation::Up:
								for (vint i = keyCommand.keys.Count() - 1; i >= 0; i--)
								{
									KeyUp(state, *listenerList, keyCommand.keys[i]);
								}
								break;
							case KeyOperation::Press:
								for (auto key : keyCommand.keys)
								{
									KeyDown(state, *listenerList, key);
								}
								for (vint i = keyCommand.keys.Count() - 1; i >= 0; i--)
								{
									KeyUp(state, *listenerList, keyCommand.keys[i]);
								}
								break;
							}
						},
						[=](const MouseMoveCommand& mouseCommand)
						{
							activateTargetWindow();
							TemporaryModifiers temporary;
							PressTemporaryModifiers(state, *listenerList, mouseCommand.arguments.modifiers, temporary);
							MouseMove(state, *listenerList, ConvertGuiPointToNativePoint(targetWindow, mouseCommand.arguments.position));
							ReleaseTemporaryModifiers(state, *listenerList, temporary);
						},
						[=](const MouseButtonCommand& mouseCommand)
						{
							activateTargetWindow();
							TemporaryModifiers temporary;
							PressTemporaryModifiers(state, *listenerList, mouseCommand.arguments.modifiers, temporary);
							MouseButtonOperation(state, *listenerList, mouseCommand.button, mouseCommand.operation, ConvertGuiPointToNativePoint(targetWindow, mouseCommand.arguments.position));
							ReleaseTemporaryModifiers(state, *listenerList, temporary);
						},
						[=](const WheelCommand& wheelCommand)
						{
							activateTargetWindow();
							TemporaryModifiers temporary;
							PressTemporaryModifiers(state, *listenerList, wheelCommand.arguments.modifiers, temporary);
							Wheel(state, *listenerList, wheelCommand.arguments.ticks * 120 * wheelCommand.direction, wheelCommand.horizontal);
							ReleaseTemporaryModifiers(state, *listenerList, temporary);
						}
						));
				});
#undef ERROR_MESSAGE_PREFIX
			}
		}

		WString RunIOCommandOnNativeWindow(
			IoCommandState* state,
			INativeController* nativeController,
			INativeWindow* nativeWindow,
			collections::List<INativeWindowListener*>& listeners,
			WString command
		)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::RunIOCommandOnNativeWindow(...)#"
			CHECK_ERROR(state, ERROR_MESSAGE_PREFIX L"IO command state is missing.");
			CHECK_ERROR(nativeController, ERROR_MESSAGE_PREFIX L"Native controller is missing.");

			auto targetWindow = nativeWindow;
			if (!targetWindow)
			{
				targetWindow = nativeController->WindowService()->GetMainWindow();
			}
			if (!targetWindow)
			{
				return WString::Unmanaged(iocommands::IO_COMMAND_SYNTAX);
			}

			auto ioCommand = iocommands::ParseIOCommand(nativeController, command);
			if (ioCommand.TryGet<iocommands::SyntaxErrorCommand>())
			{
				return WString::Unmanaged(iocommands::IO_COMMAND_SYNTAX);
			}

			iocommands::ExecuteIOCommand(state, nativeController, targetWindow, listeners, std::move(ioCommand));
			return WString::Unmanaged(L"Queued");
#undef ERROR_MESSAGE_PREFIX
		}
	}
}
