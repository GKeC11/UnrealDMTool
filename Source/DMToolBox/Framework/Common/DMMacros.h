#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

namespace DMLogPrivate
{
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

#define DM_LOG(WorldContextObject, CategoryName, Verbosity, Format, ...) \
	do \
	{ \
		const FString DMLogPrefix = DMLogPrivate::GetNetPrefix(WorldContextObject); \
		UE_LOG(CategoryName, Verbosity, TEXT("%s ") Format, *DMLogPrefix, ##__VA_ARGS__); \
	} while (false)
