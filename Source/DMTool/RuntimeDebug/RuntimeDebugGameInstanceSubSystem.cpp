// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeDebugGameInstanceSubSystem.h"

void URuntimeDebugGameInstanceSubSystem::AddLogEntryData(FRuntimeDebugConsoleEntryData* Data)
{
	LogEntryDataArray.Add(MakeShareable(Data));
}
