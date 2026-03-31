#pragma once

#include "DMGameplaySubsystem.generated.h"

UCLASS()
class UDMGameplaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
protected:
	// Override
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	// Gameplay Subsystem
	void OnLocalPlayerAddedEvent(ULocalPlayer* InLocalPlayer);
};
