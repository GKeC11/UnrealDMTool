#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

namespace DMLogPrivate
{
	inline FString GetFileName(const ANSICHAR* FilePath)
	{
		if (!FilePath)
		{
			return FString();
		}

		const ANSICHAR* FileName = FilePath;
		for (const ANSICHAR* Char = FilePath; *Char != '\0'; ++Char)
		{
			if (*Char == '/' || *Char == '\\')
			{
				FileName = Char + 1;
			}
		}

		FString Result = ANSI_TO_TCHAR(FileName);
		int32 ExtensionIndex = INDEX_NONE;
		if (Result.FindLastChar(TEXT('.'), ExtensionIndex))
		{
			Result.LeftInline(ExtensionIndex, false);
		}

		return Result;
	}

	inline FString GetFunctionName(const ANSICHAR* FunctionName)
	{
		if (!FunctionName)
		{
			return FString();
		}

		FString Result = ANSI_TO_TCHAR(FunctionName);
		int32 ScopeIndex = INDEX_NONE;
		if (Result.FindLastChar(TEXT(':'), ScopeIndex))
		{
			Result.RightChopInline(ScopeIndex + 1, false);
		}

		return Result;
	}

	inline FString GetNetPrefix(const UObject* WorldContextObject)
	{
		const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
		if (!World)
		{
			return TEXT("[Unknown]");
		}

		switch (World->GetNetMode())
		{
		case NM_Client:
		{
			int32 ClientIndex = INDEX_NONE;

			if (const UPackage* WorldPackage = World->GetOutermost())
			{
				ClientIndex = WorldPackage->GetPIEInstanceID();
			}

			if (ClientIndex == INDEX_NONE)
			{
				if (const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController())
				{
					ClientIndex = LocalPlayer->GetLocalPlayerIndex() + 1;
				}
			}

			if (ClientIndex == INDEX_NONE)
			{
				ClientIndex = 1;
			}

			return FString::Printf(TEXT("[Client %d]"), ClientIndex);
		}
		case NM_Standalone:
		case NM_ListenServer:
		case NM_DedicatedServer:
		default:
			return TEXT("[Server]");
		}
	}
}

#if WITH_EDITOR
#define DM_LOG(WorldContextObject, CategoryName, Verbosity, Format, ...) \
	do \
	{ \
		const FString DMLogFileName = DMLogPrivate::GetFileName(__FILE__); \
		const FString DMLogFunctionName = DMLogPrivate::GetFunctionName(__FUNCTION__); \
		const FString DMLogPrefix = DMLogPrivate::GetNetPrefix(WorldContextObject); \
		UE_LOG(CategoryName, Verbosity, TEXT("%s[%s][%s] ") Format, *DMLogPrefix, *DMLogFileName, *DMLogFunctionName, ##__VA_ARGS__); \
	} while (false)
#else
#define DM_LOG(WorldContextObject, CategoryName, Verbosity, Format, ...) \
	do \
	{ \
		const FString DMLogFileName = DMLogPrivate::GetFileName(__FILE__); \
		const FString DMLogFunctionName = DMLogPrivate::GetFunctionName(__FUNCTION__); \
		UE_LOG(CategoryName, Verbosity, TEXT("[%s][%s] ") Format, *DMLogFileName, *DMLogFunctionName, ##__VA_ARGS__); \
	} while (false)
#endif
