// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeDebugGameInstanceSubSystem.h"

void URuntimeDebugGameInstanceSubSystem::AddLogEntryData(TSharedPtr<FRuntimeDebugConsoleEntryData>& Data)
{
	LogEntryDataArray.Add(Data);
}
