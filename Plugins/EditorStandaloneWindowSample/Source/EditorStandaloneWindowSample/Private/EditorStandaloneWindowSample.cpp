// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorStandaloneWindowSample.h"
#include "EditorStandaloneWindowSampleStyle.h"
#include "EditorStandaloneWindowSampleCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName EditorStandaloneWindowSampleTabName("EditorStandaloneWindowSample");

#define LOCTEXT_NAMESPACE "FEditorStandaloneWindowSampleModule"

void FEditorStandaloneWindowSampleModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FEditorStandaloneWindowSampleStyle::Initialize();
	FEditorStandaloneWindowSampleStyle::ReloadTextures();

	FEditorStandaloneWindowSampleCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FEditorStandaloneWindowSampleCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FEditorStandaloneWindowSampleModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FEditorStandaloneWindowSampleModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(EditorStandaloneWindowSampleTabName, FOnSpawnTab::CreateRaw(this, &FEditorStandaloneWindowSampleModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FEditorStandaloneWindowSampleTabTitle", "EditorStandaloneWindowSample"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FEditorStandaloneWindowSampleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FEditorStandaloneWindowSampleStyle::Shutdown();

	FEditorStandaloneWindowSampleCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(EditorStandaloneWindowSampleTabName);
}

TSharedRef<SDockTab> FEditorStandaloneWindowSampleModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FEditorStandaloneWindowSampleModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("EditorStandaloneWindowSample.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FEditorStandaloneWindowSampleModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(EditorStandaloneWindowSampleTabName);
}

void FEditorStandaloneWindowSampleModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FEditorStandaloneWindowSampleCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FEditorStandaloneWindowSampleCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEditorStandaloneWindowSampleModule, EditorStandaloneWindowSample)