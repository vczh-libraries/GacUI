#include "GuiToolstripCommand.h"
#include "../../Application/Controls/GuiApplication.h"
#include "../../Application/Controls/GuiWindowControls.h"
#include "../../Application/GraphicsHost/GuiGraphicsHost_ShortcutKey.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace compositions;
			using namespace regex;

/***********************************************************************
GuiToolstripCommand
***********************************************************************/

			void GuiToolstripCommand::OnShortcutKeyItemExecuted(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (enabled)
				{
					Executed.ExecuteWithNewSender(arguments, sender);
				}
			}

			void GuiToolstripCommand::OnRenderTargetChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				UpdateShortcutOwner();
			}

			void GuiToolstripCommand::InvokeDescriptionChanged()
			{
				GuiEventArgs arguments;
				DescriptionChanged.Execute(arguments);
			}

			compositions::IGuiShortcutKeyManager* GuiToolstripCommand::GetShortcutManagerFromBuilder(Ptr<ShortcutBuilder> builder)
			{
				if (builder->global)
				{
					return GetApplication()->GetGlobalShortcutKeyManager();
				}
				else
				{
					if (attachedControlHost)
					{
						if (!attachedControlHost->GetShortcutKeyManager())
						{
							attachedControlHost->SetShortcutKeyManager(new GuiShortcutKeyManager());
						}
						return attachedControlHost->GetShortcutKeyManager();
					}
				}
				return nullptr;
			}

			void GuiToolstripCommand::RemoveShortcut()
			{
				if (shortcutKeyItem)
				{
					shortcutKeyItem->Executed.Detach(shortcutKeyItemExecutedHandler);
					shortcutKeyItem->GetManager()->DestroyShortcut(shortcutKeyItem);
				}
				shortcutKeyItem = nullptr;
				shortcutKeyItemExecutedHandler = nullptr;
			}

			void GuiToolstripCommand::ReplaceShortcut(compositions::IGuiShortcutKeyItem* value)
			{
				if (shortcutKeyItem != value)
				{
					RemoveShortcut();
					if (value)
					{
						shortcutKeyItem = value;
						shortcutKeyItemExecutedHandler = shortcutKeyItem->Executed.AttachMethod(this, &GuiToolstripCommand::OnShortcutKeyItemExecuted);
					}
					InvokeDescriptionChanged();
				}
			}

			void GuiToolstripCommand::BuildShortcut(const WString& builderText)
			{
				List<glr::ParsingError> errors;
				if (auto parser = GetParserManager()->GetParser<ShortcutBuilder>(L"SHORTCUT"))
				{
					if (auto builder = parser->ParseInternal(builderText, errors))
					{
						shortcutBuilder = builder;
						if (auto shortcutKeyManager = GetShortcutManagerFromBuilder(builder))
						{
							if (auto item = shortcutKeyManager->CreateShortcutIfNotExist(builder->ctrl, builder->shift, builder->alt, builder->key))
							{
								ReplaceShortcut(item);
							}
						}
					}
				}
			}

			void GuiToolstripCommand::UpdateShortcutOwner()
			{
				GuiControlHost* host = nullptr;
				if (auto control = dynamic_cast<GuiControl*>(attachedRootObject))
				{
					host = control->GetRelatedControlHost();
				}
				else if (auto composition = dynamic_cast<GuiGraphicsComposition*>(attachedRootObject))
				{
					host = composition->GetRelatedControlHost();
				}

				if (attachedControlHost != host)
				{
					attachedControlHost = host;
					if (shortcutBuilder && !shortcutBuilder->global)
					{
						if (shortcutKeyItem)
						{
							ReplaceShortcut(nullptr);
						}
						BuildShortcut(shortcutBuilder->text);
					}
				}
			}

			GuiToolstripCommand::GuiToolstripCommand()
			{
			}

			GuiToolstripCommand::~GuiToolstripCommand()
			{
				RemoveShortcut();
				shortcutBuilder = nullptr;
			}

			void GuiToolstripCommand::Attach(GuiInstanceRootObject* rootObject)
			{
				GuiGraphicsComposition* rootComposition = nullptr;

				if (attachedRootObject != rootObject)
				{
					if (attachedRootObject)
					{
						if (auto control = dynamic_cast<GuiControl*>(attachedRootObject))
						{
							control->ControlSignalTrigerred.Detach(renderTargetChangedHandler);
						}
						else if (auto composition = dynamic_cast<GuiGraphicsComposition*>(attachedRootObject))
						{
							composition->GetEventReceiver()->renderTargetChanged.Detach(renderTargetChangedHandler);
						}
						renderTargetChangedHandler = nullptr;
					}

					attachedRootObject = rootObject;
					if (attachedRootObject)
					{
						if (auto control = dynamic_cast<GuiControl*>(attachedRootObject))
						{
							renderTargetChangedHandler = control->ControlSignalTrigerred.AttachLambda(
								[=](GuiGraphicsComposition* sender, GuiControlSignalEventArgs& arguments)
								{
									OnRenderTargetChanged(sender, arguments);
								});
						}
						else if (auto composition = dynamic_cast<GuiGraphicsComposition*>(attachedRootObject))
						{
							renderTargetChangedHandler = composition->GetEventReceiver()->renderTargetChanged.AttachMethod(this, &GuiToolstripCommand::OnRenderTargetChanged);
						}
					}
					UpdateShortcutOwner();
				}
			}

			void GuiToolstripCommand::Detach(GuiInstanceRootObject* rootObject)
			{
				Attach(nullptr);
			}

			Ptr<GuiImageData> GuiToolstripCommand::GetLargeImage()
			{
				return largeImage;
			}

			void GuiToolstripCommand::SetLargeImage(Ptr<GuiImageData> value)
			{
				if (largeImage != value)
				{
					largeImage = value;
					InvokeDescriptionChanged();
				}
			}

			Ptr<GuiImageData> GuiToolstripCommand::GetImage()
			{
				return image;
			}

			void GuiToolstripCommand::SetImage(Ptr<GuiImageData> value)
			{
				if(image!=value)
				{
					image=value;
					InvokeDescriptionChanged();
				}
			}

			const WString& GuiToolstripCommand::GetText()
			{
				return text;
			}

			void GuiToolstripCommand::SetText(const WString& value)
			{
				if(text!=value)
				{
					text=value;
					InvokeDescriptionChanged();
				}
			}

			compositions::IGuiShortcutKeyItem* GuiToolstripCommand::GetShortcut()
			{
				return shortcutKeyItem;
			}

			WString GuiToolstripCommand::GetShortcutBuilder()
			{
				return shortcutBuilder ? shortcutBuilder->text : L"";
			}

			void GuiToolstripCommand::SetShortcutBuilder(const WString& value)
			{
				BuildShortcut(value);
			}

			bool GuiToolstripCommand::GetEnabled()
			{
				return enabled;
			}

			void GuiToolstripCommand::SetEnabled(bool value)
			{
				if(enabled!=value)
				{
					enabled=value;
					InvokeDescriptionChanged();
				}
			}

			bool GuiToolstripCommand::GetSelected()
			{
				return selected;
			}

			void GuiToolstripCommand::SetSelected(bool value)
			{
				if(selected!=value)
				{
					selected=value;
					InvokeDescriptionChanged();
				}
			}

/***********************************************************************
GuiToolstripCommand::ShortcutBuilder Parser
***********************************************************************/

			class GuiToolstripCommandShortcutParser : public Object, public IGuiParser<GuiToolstripCommand::ShortcutBuilder>
			{
				typedef GuiToolstripCommand::ShortcutBuilder			ShortcutBuilder;
			public:
				Regex						regexShortcut;
				const vint					_global;
				const vint					_ctrl;
				const vint					_shift;
				const vint					_alt;
				const vint					_key;

				GuiToolstripCommandShortcutParser()
					: regexShortcut(L"((<global>global:))?((<ctrl>Ctrl)/+|(<shift>Shift)/+|(<alt>Alt)/+)*(<key>/.+)")
					, _global(regexShortcut.CaptureNames().IndexOf(L"global"))
					, _ctrl(regexShortcut.CaptureNames().IndexOf(L"ctrl"))
					, _shift(regexShortcut.CaptureNames().IndexOf(L"shift"))
					, _alt(regexShortcut.CaptureNames().IndexOf(L"alt"))
					, _key(regexShortcut.CaptureNames().IndexOf(L"key"))
				{
				}

				Ptr<ShortcutBuilder> ParseInternal(const WString& text, collections::List<glr::ParsingError>& errors)override
				{
					Ptr<RegexMatch> match=regexShortcut.MatchHead(text);
					if (match && match->Result().Length() != text.Length())
					{
						glr::ParsingError error;
						error.message = L"Failed to parse a shortcut \"" + text + L"\".";
						errors.Add(error);
						return nullptr;
					}

					auto builder = Ptr(new ShortcutBuilder);
					builder->text = text;
					builder->global = match->Groups().Contains(_global);
					builder->ctrl = match->Groups().Contains(_ctrl);
					builder->shift = match->Groups().Contains(_shift);
					builder->alt = match->Groups().Contains(_alt);

					WString name = match->Groups()[_key][0].Value();
					builder->key = GetCurrentController()->InputService()->GetKey(name);

					return builder->key == VKEY::KEY_UNKNOWN ? nullptr : builder;
				}
			};

/***********************************************************************
GuiToolstripCommandPlugin
***********************************************************************/

			class GuiToolstripCommandPlugin : public Object, public IGuiPlugin
			{
			public:

				GUI_PLUGIN_NAME(GacUI_Compiler_ShortcutParser)
				{
					GUI_PLUGIN_DEPEND(GacUI_Parser);
				}
				
				void Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
				{
					if (controllerUnrelatedPlugins)
					{
						IGuiParserManager* manager = GetParserManager();
						manager->SetParser(L"SHORTCUT", Ptr(new GuiToolstripCommandShortcutParser));
					}
				}

				void Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
				{
				}
			};
			GUI_REGISTER_PLUGIN(GuiToolstripCommandPlugin)
		}
	}
}