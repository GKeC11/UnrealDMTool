#include "DMDedicatedServerSubsystem.h"

#include "DMToolBox/Framework/Common/DMMacros.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMisc.h"

bool UDMDedicatedServerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UGameInstance* GameInstance = Cast<UGameInstance>(Outer);
	return Super::ShouldCreateSubsystem(Outer) && GameInstance && GameInstance->IsDedicatedServerInstance();
}

void UDMDedicatedServerSubsystem::Deinitialize()
{
	DM_LOG(this, LogTemp, Log, TEXT("Deinitialize: Cancel pending empty-server shutdown."));
	CancelShutdown();
	PendingShutdownGameMode.Reset();

	Super::Deinitialize();
}

void UDMDedicatedServerSubsystem::NotifyPlayerLoggedIn(APlayerController* NewPlayer)
{
	if (!IsValid(NewPlayer))
	{
		DM_LOG(this, LogTemp, Warning, TEXT("NotifyPlayerLoggedIn skipped: PlayerController is invalid."));
		return;
	}

	bHadPlayerConnection = true;
	CancelShutdown();
	DM_LOG(this, LogTemp, Log, TEXT("NotifyPlayerLoggedIn: Player=%s"), *GetNameSafe(NewPlayer));
}

void UDMDedicatedServerSubsystem::NotifyPlayerLoggedOut(AGameModeBase* GameMode)
{
	DM_LOG(this, LogTemp, Log, TEXT("NotifyPlayerLoggedOut: GameMode=%s"), *GetNameSafe(GameMode));
	ScheduleShutdownIfEmpty(GameMode);
}

void UDMDedicatedServerSubsystem::SetEmptyServerShutdownDelay(const float InDelaySeconds)
{
	EmptyServerShutdownDelay = FMath::Max(0.0f, InDelaySeconds);
	DM_LOG(this, LogTemp, Log, TEXT("SetEmptyServerShutdownDelay: DelaySeconds=%.2f"), EmptyServerShutdownDelay);
}

void UDMDedicatedServerSubsystem::ScheduleShutdownIfEmpty(AGameModeBase* GameMode)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		DM_LOG(this, LogTemp, Warning, TEXT("ScheduleShutdownIfEmpty skipped: World is invalid."));
		return;
	}

	if (!IsValid(GameMode))
	{
		DM_LOG(this, LogTemp, Warning, TEXT("ScheduleShutdownIfEmpty skipped: GameMode is invalid."));
		return;
	}

	if (GameMode->GetNetMode() != NM_DedicatedServer)
	{
		DM_LOG(this, LogTemp, Log, TEXT("ScheduleShutdownIfEmpty skipped: NetMode is not dedicated server. GameMode=%s"), *GetNameSafe(GameMode));
		return;
	}

	// Avoid shutting down a freshly booted server while matchmaking has not connected anyone yet.
	if (!bHadPlayerConnection)
	{
		DM_LOG(this, LogTemp, Log, TEXT("ScheduleShutdownIfEmpty skipped: No player has connected yet. GameMode=%s"), *GetNameSafe(GameMode));
		return;
	}

	if (GameMode->GetNumPlayers() > 0)
	{
		DM_LOG(this, LogTemp, Log, TEXT("ScheduleShutdownIfEmpty skipped: Server still has players. GameMode=%s, PlayerCount=%d"),
			*GetNameSafe(GameMode),
			GameMode->GetNumPlayers());
		return;
	}

	PendingShutdownGameMode = GameMode;
	if (EmptyServerShutdownDelay <= 0.0f)
	{
		DM_LOG(this, LogTemp, Log, TEXT("ScheduleShutdownIfEmpty: Shutdown immediately. GameMode=%s"), *GetNameSafe(GameMode));
		ShutdownIfStillEmpty();
		return;
	}

	DM_LOG(this, LogTemp, Log, TEXT("ScheduleShutdownIfEmpty: Shutdown scheduled. GameMode=%s, DelaySeconds=%.2f"),
		*GetNameSafe(GameMode),
		EmptyServerShutdownDelay);
	World->GetTimerManager().SetTimer(
		EmptyServerShutdownTimerHandle,
		this,
		&ThisClass::ShutdownIfStillEmpty,
		EmptyServerShutdownDelay,
		false);
}

void UDMDedicatedServerSubsystem::CancelShutdown()
{
	if (UWorld* World = GetWorld())
	{
		const bool bWasShutdownScheduled = World->GetTimerManager().IsTimerActive(EmptyServerShutdownTimerHandle);
		World->GetTimerManager().ClearTimer(EmptyServerShutdownTimerHandle);
		if (bWasShutdownScheduled)
		{
			DM_LOG(this, LogTemp, Log, TEXT("CancelShutdown: Pending empty-server shutdown cancelled."));
		}
	}

	PendingShutdownGameMode.Reset();
}

void UDMDedicatedServerSubsystem::ShutdownIfStillEmpty()
{
	AGameModeBase* GameMode = PendingShutdownGameMode.Get();
	if (!IsValid(GameMode))
	{
		DM_LOG(this, LogTemp, Warning, TEXT("ShutdownIfStillEmpty skipped: GameMode is invalid."));
		return;
	}

	if (GameMode->GetNetMode() != NM_DedicatedServer)
	{
		DM_LOG(this, LogTemp, Log, TEXT("ShutdownIfStillEmpty skipped: NetMode is not dedicated server. GameMode=%s"), *GetNameSafe(GameMode));
		return;
	}

	if (GameMode->GetNumPlayers() > 0)
	{
		DM_LOG(this, LogTemp, Log, TEXT("ShutdownIfStillEmpty skipped: Server has players again. GameMode=%s, PlayerCount=%d"),
			*GetNameSafe(GameMode),
			GameMode->GetNumPlayers());
		return;
	}

	DM_LOG(this, LogTemp, Log, TEXT("ShutdownIfStillEmpty: Requesting dedicated server exit. GameMode=%s"), *GetNameSafe(GameMode));
	FPlatformMisc::RequestExit(false);
}
