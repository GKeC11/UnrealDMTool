// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorStandaloneWindowSampleCommands.h"

#define LOCTEXT_NAMESPACE "FEditorStandaloneWindowSampleModule"

void FEditorStandaloneWindowSampleCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "EditorStandaloneWindowSample", "Bring up EditorStandaloneWindowSample window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
