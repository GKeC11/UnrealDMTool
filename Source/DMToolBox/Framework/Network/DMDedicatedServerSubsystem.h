#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "DMDedicatedServerSubsystem.generated.h"

class AGameModeBase;
class APlayerController;

UCLASS()
class DMTOOLBOX_API UDMDedicatedServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|DedicatedServer")
	void NotifyPlayerLoggedIn(APlayerController* NewPlayer);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|DedicatedServer")
	void NotifyPlayerLoggedOut(AGameModeBase* GameMode);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|DedicatedServer")
	void SetEmptyServerShutdownDelay(float InDelaySeconds);

	UFUNCTION(BlueprintPure, Category = "DMToolBox|DedicatedServer")
	float GetEmptyServerShutdownDelay() const { return EmptyServerShutdownDelay; }

protected:
	void ScheduleShutdownIfEmpty(AGameModeBase* GameMode);
	void CancelShutdown();
	void ShutdownIfStillEmpty();

protected:
	UPROPERTY(EditAnywhere, Category = "DMToolBox|DedicatedServer")
	float EmptyServerShutdownDelay = 10.0f;

	FTimerHandle EmptyServerShutdownTimerHandle;
	TWeakObjectPtr<AGameModeBase> PendingShutdownGameMode;
	bool bHadPlayerConnection = false;
};
