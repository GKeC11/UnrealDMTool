﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeDebugConsoleEntryData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RuntimeDebugGameInstanceSubSystem.generated.h"

/**
 * 
 */
UCLASS()
class DMTOOL_API URuntimeDebugGameInstanceSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void AddLogEntryData(FRuntimeDebugConsoleEntryData* Data);

private:
	TArray<TSharedPtr<FRuntimeDebugConsoleEntryData>> LogEntryDataArray;
};
