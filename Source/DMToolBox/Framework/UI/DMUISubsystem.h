#pragma once

#include "DMUIScreen.h"
#include "Engine/World.h"

#include "DMUISubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FDMLevelTravelCompletedDelegate);

UCLASS()
class UDMUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDMUIScreen* GetCurrentScreen() { return CurrentScreen; }

	bool IsLevelTravelCompleted() const { return bLevelTravelCompleted; }

	FDMLevelTravelCompletedDelegate OnLevelTravelCompleted;

	// Override
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	// UI Subsystem
	void OnLocalPlayerAddedEvent(ULocalPlayer* InLocalPlayer);

private:
	void HandlePostWorldInitialization(UWorld* InitializedWorld, const UWorld::InitializationValues InitializationValues);
	void HandleWorldCleanup(UWorld* CleanupWorld, bool bSessionEnded, bool bCleanupResources);
	void MarkLevelTravelCompleted(UWorld* LoadedWorld);

protected:
	// UI
	UPROPERTY()
	UDMUIScreen* CurrentScreen = nullptr;

	FDelegateHandle PostWorldInitializationDelegateHandle;
	FDelegateHandle WorldCleanupDelegateHandle;

	bool bLevelTravelCompleted = false;
};
