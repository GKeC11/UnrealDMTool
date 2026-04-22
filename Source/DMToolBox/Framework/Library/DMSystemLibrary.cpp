#include "DMSystemLibrary.h"

#include "DMToolBox/Framework/Gameplay/Core/DMGameInstance.h"
#include "DMToolBox/Framework/Gameplay/Core/DMWorldSetting.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

UDMGameInstance* UDMSystemLibrary::ResolveDMGameInstance()
{
	if (!GEngine)
	{
		return nullptr;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* World = WorldContext.World();
		if (!IsValid(World) || !World->IsGameWorld())
		{
			continue;
		}

		if (UDMGameInstance* GameInstance = Cast<UDMGameInstance>(World->GetGameInstance()))
		{
			return GameInstance;
		}
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* World = WorldContext.World();
		if (!IsValid(World))
		{
			continue;
		}

		if (UDMGameInstance* GameInstance = Cast<UDMGameInstance>(World->GetGameInstance()))
		{
			return GameInstance;
		}
	}

	return nullptr;
}

UWorld* UDMSystemLibrary::ResolveWorldFromContext(const UObject* WorldContextObject)
{
	if (IsValid(WorldContextObject))
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			return World;
		}
	}

	if (!GEngine)
	{
		return nullptr;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* World = WorldContext.World();
		if (IsValid(World) && World->IsGameWorld())
		{
			return World;
		}
	}

	return nullptr;
}

int32 UDMSystemLibrary::ResolveClientIndex(const UWorld* World)
{
	if (!IsValid(World) || !GEngine)
	{
		return 0;
	}

	if (const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(World))
	{
		if (WorldContext->PIEInstance != INDEX_NONE)
		{
			return WorldContext->PIEInstance;
		}
	}

	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		const TArray<ULocalPlayer*>& LocalPlayers = GameInstance->GetLocalPlayers();
		if (LocalPlayers.Num() > 0 && IsValid(LocalPlayers[0]))
		{
			return LocalPlayers[0]->GetLocalPlayerIndex();
		}
	}

	return 0;
}

UDMLevelInitializationSetting* UDMSystemLibrary::GetLevelInitializationSetting(UWorld* InWorld)
{
	AWorldSettings* WorldSetting = InWorld->GetWorldSettings();
	if (ADMWorldSetting* DMWorldSetting = Cast<ADMWorldSetting>(WorldSetting))
	{
		if (UDMLevelInitializationSetting* LevelInitializationSetting = DMWorldSetting->LevelInitializationSetting)
		{
			return LevelInitializationSetting;
		}
	}

	return nullptr;
}
