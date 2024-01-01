// Fill out your copyright notice in the Description page of Project Settings.

#include "SlateLabCommands.h"

#define LOCTEXT_NAMESPACE "FSlateLabModule"

void FSlateLabCommands::RegisterCommands()
{
	UI_COMMAND(OpenSlateLab, "OpenSlateLab", "Open Slate Lab Windows", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

