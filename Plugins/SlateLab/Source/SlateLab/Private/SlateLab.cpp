// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlateLab.h"

#include "SlateLabCommands.h"
#include "ToolMenus.h"
#include "Widgets/SViewport.h"

#define LOCTEXT_NAMESPACE "FSlateLabModule"

void FSlateLabModule::StartupModule()
{
	RegisterStyle();
	
	RegisterCommands();
	
	RegisterMenus();

	RegisterTabs();
}

void FSlateLabModule::ShutdownModule()
{
	UnregisterStyle();
	
	UnregisterCommands();

	UnregisterMenus();

	UnregisterTabs();
}

void FSlateLabModule::RegisterStyle()
{
	FSlateLabStyle::Initialize();

	FSlateLabStyle::ReloadTextures();
}

void FSlateLabModule::UnregisterStyle()
{
	FSlateLabStyle::Shutdown();
}

void FSlateLabModule::RegisterCommands()
{
	FSlateLabCommands::Register();
	
	CommandList = MakeShareable(new FUICommandList());
	
	CommandList->MapAction(FSlateLabCommands::Get().OpenSlateLab,
		FExecuteAction::CreateRaw(this, &FSlateLabModule::OpenSlateLabButtonClicked),
		FCanExecuteAction());
}

void FSlateLabModule::UnregisterCommands()
{
	FSlateLabCommands::Get().Unregister();
}

void FSlateLabModule::RegisterMenus()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(
		this, &FSlateLabModule::OnRegisterMenus));
}

void FSlateLabModule::OnRegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	
	UToolMenu* UserToolBar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& SlateLabSection =  UserToolBar->AddSection("SlateLab");
	FToolMenuEntry& SlateLabEntry = SlateLabSection.AddEntry(FToolMenuEntry::InitToolBarButton(
		FSlateLabCommands::Get().OpenSlateLab));
	SlateLabEntry.SetCommandList(CommandList);
}

void FSlateLabModule::UnregisterMenus()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

static const FName ViewportTabName = "SlateLapViewportTab";

void FSlateLabModule::RegisterTabs()
{
	FGlobalTabmanager::Get()->RegisterTabSpawner(ViewportTabName, FOnSpawnTab::CreateRaw(
		this, &FSlateLabModule::OnSpawnViewportTab));
}

void FSlateLabModule::UnregisterTabs()
{
	FGlobalTabmanager::Get()->UnregisterTabSpawner(ViewportTabName);
}

TSharedRef<SDockTab> FSlateLabModule::OnSpawnViewportTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
	.TabRole(NomadTab)
	[
		SNew(SViewport)
	];
}

void FSlateLabModule::OpenSlateLabButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ViewportTabName);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSlateLabModule, SlateLab)