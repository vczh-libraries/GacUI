#include "GuiToolstripCommand.h"
#include "../GuiApplication.h"
#include "../../GraphicsHost/GuiGraphicsHost_ShortcutKey.h"
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
			using namespace parsing;

/***********************************************************************
GuiToolstripCommand
***********************************************************************/

			void GuiToolstripCommand::OnShortcutKeyItemExecuted(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				Executed.ExecuteWithNewSender(arguments, sender);
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

			void GuiToolstripCommand::ReplaceShortcut(compositions::IGuiShortcutKeyItem* value, Ptr<ShortcutBuilder> builder)
			{
				if (shortcutKeyItem != value)
				{
					if (shortcutKeyItem)
					{
						shortcutKeyItem->Executed.Detach(shortcutKeyItemExecutedHandler);
						if (shortcutBuilder)
						{
							auto manager = dynamic_cast<GuiShortcutKeyManager*>(shortcutOwner->GetShortcutKeyManager());
							if (manager)
							{
								manager->DestroyShortcut(shortcutBuilder->ctrl, shortcutBuilder->shift, shortcutBuilder->alt, shortcutBuilder->key);
							}
						}
					}
					shortcutKeyItem = nullptr;
					shortcutKeyItemExecutedHandler = nullptr;
					shortcutBuilder = value ? builder : nullptr;
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
				List<Ptr<ParsingError>> errors;
				if (auto parser = GetParserManager()->GetParser<ShortcutBuilder>(L"SHORTCUT"))
				{
					if (Ptr<ShortcutBuilder> builder = parser->ParseInternal(builderText, errors))
					{
						if (shortcutOwner)
						{
							if (!shortcutOwner->GetShortcutKeyManager())
							{
								shortcutOwner->SetShortcutKeyManager(new GuiShortcutKeyManager);
							}
							if (auto manager = dynamic_cast<GuiShortcutKeyManager*>(shortcutOwner->GetShortcutKeyManager()))
							{
								IGuiShortcutKeyItem* item = manager->TryGetShortcut(builder->ctrl, builder->shift, builder->alt, builder->key);
								if (!item)
								{
									item = manager->CreateShortcut(builder->ctrl, builder->shift, builder->alt, builder->key);
									if (item)
									{
										ReplaceShortcut(item, builder);
									}
								}
							}
						}
						else
						{
							shortcutBuilder = builder;
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

				if (shortcutOwner != host)
				{
					if (shortcutOwner)
					{
						ReplaceShortcut(nullptr, nullptr);
						shortcutOwner = nullptr;
					}
					shortcutOwner = host;
					if (shortcutBuilder && !shortcutKeyItem)
					{
						BuildShortcut(shortcutBuilder->text);
					}
				}
			}

			GuiToolstripCommand::GuiToolstripCommand()
			{
			}

			GuiToolstripCommand::~GuiToolstripCommand()
			{
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

			void GuiToolstripCommand::SetShortcut(compositions::IGuiShortcutKeyItem* value)
			{
				ReplaceShortcut(value, 0);
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

				GuiToolstripCommandShortcutParser()
					:regexShortcut(L"((<ctrl>Ctrl)/+|(<shift>Shift)/+|(<alt>Alt)/+)*(<key>/.+)")
				{
				}

				Ptr<ShortcutBuilder> ParseInternal(const WString& text, collections::List<Ptr<ParsingError>>& errors)override
				{
					Ptr<RegexMatch> match=regexShortcut.MatchHead(text);
					if (match && match->Result().Length() != text.Length())
					{
						errors.Add(new ParsingError(L"Failed to parse a shortcut \"" + text + L"\"."));
						return 0;
					}

					Ptr<ShortcutBuilder> builder = new ShortcutBuilder;
					builder->text = text;
					builder->ctrl = match->Groups().Contains(L"ctrl");
					builder->shift = match->Groups().Contains(L"shift");
					builder->alt = match->Groups().Contains(L"alt");

					WString name = match->Groups()[L"key"][0].Value();
					builder->key = GetCurrentController()->InputService()->GetKey(name);

					return builder->key == VKEY::_UNKNOWN ? nullptr : builder;
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
				
				void Load()override
				{
					IGuiParserManager* manager=GetParserManager();
					manager->SetParser(L"SHORTCUT", new GuiToolstripCommandShortcutParser);
				}

				void Unload()override
				{
				}
			};
			GUI_REGISTER_PLUGIN(GuiToolstripCommandPlugin)
		}
	}
}