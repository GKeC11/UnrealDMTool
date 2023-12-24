// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorStandaloneWindowSampleStyle.h"

class FEditorStandaloneWindowSampleCommands : public TCommands<FEditorStandaloneWindowSampleCommands>
{
public:

	FEditorStandaloneWindowSampleCommands()
		: TCommands<FEditorStandaloneWindowSampleCommands>(TEXT("EditorStandaloneWindowSample"), NSLOCTEXT("Contexts", "EditorStandaloneWindowSample", "EditorStandaloneWindowSample Plugin"), NAME_None, FEditorStandaloneWindowSampleStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};