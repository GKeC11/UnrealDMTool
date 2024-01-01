// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"


class FSlateLabModule : public IModuleInterface
{
public:
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	void RegisterStyle();
	void UnregisterStyle();

	void RegisterCommands();
	void UnregisterCommands();
	
	void RegisterMenus();
	void OnRegisterMenus();
	void UnregisterMenus();
	
	void OpenSlateLabButtonClicked();
	
	void RegisterTabs();
	void UnregisterTabs();

	TSharedRef<SDockTab> OnSpawnViewportTab(const FSpawnTabArgs& Args);




public:

private:

	TSharedPtr<FUICommandList> CommandList;
};
